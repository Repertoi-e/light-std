module;

#include "../common.h"

module lstd.memory;

import lstd.basic;
import lstd.path;
import lstd.fmt;
import lstd.os;
import lstd.atomic;

LSTD_BEGIN_NAMESPACE

// Copied from test.h
//
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      /home/.../game/src/some_dir/a/string.cpp ---> some_dir/a/localization.cpp
//      /home/.../game/some_dir/string.cpp       ---> localization.cpp
//
string get_short_file_name(string str) {
    char srcData[] = {'s', 'r', 'c', OS_PATH_SEPARATOR, '\0'};
	string src = srcData;

    s64 findResult = search(str, src, search_options{ .Start = -1, .Reversed = true });
    if (findResult == -1) {
        findResult = search(str, OS_PATH_SEPARATOR, search_options{ .Start = -1, .Reversed = true });
        assert(findResult != length(str) - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        findResult += length(src);
    }

    string result = str;
    return slice(result, findResult, length(result));
}

#if defined DEBUG_MEMORY
debug_memory_node *new_node(allocation_header *header) {
    auto *node = (debug_memory_node *) pool_allocator(allocator_mode::ALLOCATE, &DebugMemoryNodesPool, sizeof(debug_memory_node), null, 0, 0);
    assert(node);

    memset0((byte *) node, sizeof(debug_memory_node));

    node->Header = header;

    // Leave invalid for now, filled out later
    node->ID = (u64) -1;

    return node;
}

void debug_memory_init() {
    AllocationCount = 0;

    DebugMemoryNodesPool.ElementSize = sizeof(debug_memory_node);

    s64 startingPoolSize = 5000 * sizeof(debug_memory_node) + sizeof(pool_allocator_data::block);

    void *pool = os_allocate_block(startingPoolSize);
    pool_allocator_provide_block(&DebugMemoryNodesPool, pool, startingPoolSize);

    // We allocate sentinels to simplify linked list management code
    auto sentinel1 = new_node((allocation_header *) 0);
    auto sentinel2 = new_node((allocation_header *) numeric<u64>::max());

    sentinel1->Next = sentinel2;
    sentinel2->Prev = sentinel1;
    DebugMemoryHead = sentinel1;
    DebugMemoryTail = sentinel2;
}

void debug_memory_uninit() {
    if (Context.DebugMemoryPrintListOfUnfreedAllocationsAtThreadExitOrProgramTermination) {
        debug_memory_report_leaks();
    }

    auto *b = DebugMemoryNodesPool.Base;
    while (b) {
        auto *next = b->Next;
        os_free_block(b);
        b = next;
    }
}

file_scope auto *list_search(allocation_header *header) {
    debug_memory_node *t = DebugMemoryHead;
    while (t != DebugMemoryTail && t->Header < header) t = t->Next;
    return t;
}

file_scope debug_memory_node *list_add(allocation_header *header) {
    auto *n = list_search(header);
    assert(n->Header != header);

    auto *node = new_node(header);

    node->Next    = n;
    node->Prev    = n->Prev;
    n->Prev->Next = node;
    n->Prev       = node;

    return node;
}

file_scope debug_memory_node *list_remove(allocation_header *header) {
    auto *n = list_search(header);
    if (n->Header != header) return null;

    n->Prev->Next = n->Next;
    n->Next->Prev = n->Prev;

    return n;
}

bool debug_memory_list_contains(allocation_header *header) {
    return list_search(header)->Header == header;
}

void debug_memory_report_leaks() {
    debug_memory_maybe_verify_heap();

    s64 leaksCount = 0;

    // @Cleanup: Factor this into a macro
    auto *it = DebugMemoryHead->Next;
    while (it != DebugMemoryTail) {
        if (!it->Freed && !it->MarkedAsLeak) ++leaksCount;
        it = it->Next;
    }

    // @Cleanup @Platform @TODO @Memory Don't use the platform allocator. We should have a seperate allocator for debug info.
    auto **leaks = malloc<debug_memory_node *>({.Count = leaksCount, .Alloc = platform_get_persistent_allocator(), .Options = LEAK});
    defer(free(leaks));

    auto *p = leaks;

    it = DebugMemoryHead->Next;
    while (it != DebugMemoryTail) {
        if (!it->Freed && !it->MarkedAsLeak) *p++ = it;
        it = it->Next;
    }

    if (leaksCount) {
        print(">>> Warning: The module {!YELLOW}\"{}\"{!} terminated but it still had {!YELLOW}{}{!} allocations which were unfreed. Here they are:\n", os_get_current_module(), leaksCount);
    }

    For_as(i, range(leaksCount)) {
        auto *it = leaks[i];

        string file = "Unknown";

        //
        // @Cleanup D I R T Y @Cleanup @Cleanup @Cleanup
        //
        if (compare_string(it->AllocatedAt.File, "") != -1) {
            file = get_short_file_name(it->AllocatedAt.File);
        }

        print("    * {}:{} requested {!GRAY}{}{!} bytes, {{ID: {}, RID: {}}}\n", file, it->AllocatedAt.Line, it->Header->Size, it->ID, it->RID);
    }
}

file_scope void verify_node_integrity(debug_memory_node *node) {
    auto *header = node->Header;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // If an assert fires here it means that memory was messed up in some way.
    //
    // We check for several problems here:
    //   * No man's land was modified. This means that you wrote before or after the allocated block.
    //     This catches buffer underflows/overflows errors.
    //   * Alignment should not be 0, should be more than POINTER_SIZE (8 bytes) and should be a power of 2.
    //     If any of these is not true, then the header was definitely corrupted.
    //   * We store a pointer to the memory block at the end of the header, any valid header will have this
    //     pointer point after itself. Otherwise the header was definitely corrupted.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if (node->Freed) {
        // We can't verify the memory was not modified because
        // it could be given back to the OS and to another program.

        return;
    }

    // The ID of the allocation to debug.
    auto id = node->ID;

    byte noMansLand[NO_MANS_LAND_SIZE];
    memset(noMansLand, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);

    auto *user = (char *) header + sizeof(allocation_header);
    assert(memcmp((byte *) user - NO_MANS_LAND_SIZE, noMansLand, NO_MANS_LAND_SIZE) == 0 &&
           "No man's land was modified. This means that you wrote before the allocated block.");

    assert(header->DEBUG_Pointer == user && "Debug pointer doesn't match. They should always match.");

    assert(memcmp((byte *) header->DEBUG_Pointer + header->Size, noMansLand, NO_MANS_LAND_SIZE) == 0 &&
           "No man's land was modified. This means that you wrote after the allocated block.");

    assert(header->Alignment && "Stored alignment is zero. Definitely corrupted.");
    assert(header->Alignment >= POINTER_SIZE && "Stored alignment smaller than pointer size (8 bytes). Definitely corrupted.");
    assert(is_pow_of_2(header->Alignment) && "Stored alignment not a power of 2. Definitely corrupted.");
}

void debug_memory_verify_heap() {
    auto *it = DebugMemoryHead->Next;
    while (it != DebugMemoryTail) {
        verify_node_integrity(it);
        it = it->Next;
    }
}

void debug_memory_maybe_verify_heap() {
    if (AllocationCount % Context.DebugMemoryHeapVerifyFrequency) return;
    debug_memory_verify_heap();
}

void check_for_overlapping_blocks(debug_memory_node *node) {
    // Check for overlapping memory blocks.
    // We can do this because we keep the linked list sorted by the memory address
    // of individual allocated blocks and we have info about their size.
    // This might catch bugs in the allocator implementation/two allocators using the same pool.

    auto *left = node->Prev;
    while (left->Freed) left = left->Prev;

    auto *right = node->Next;
    while (right->Freed) right = right->Next;

    if (left != DebugMemoryHead) {
        // Check below
        s64 size = left->Header->Size + sizeof(allocation_header);
#if defined DEBUG_MEMORY
        size += NO_MANS_LAND_SIZE;
#endif
        if (((byte *) left->Header + size) > ((byte *) node->Header - node->Header->AlignmentPadding)) {
            assert(false && "Allocator implementation returned a pointer which overlaps with another allocated block (below). This can be due to a bug in the allocator code or because two allocators use the same pool.");
        }
    }

    if (right != DebugMemoryTail) {
        // Check above
        s64 size = node->Header->Size + sizeof(allocation_header);
#if defined DEBUG_MEMORY
        size += NO_MANS_LAND_SIZE;
#endif

        if (((byte *) node->Header + size) >= ((byte *) right->Header - right->Header->AlignmentPadding)) {
            assert(false && "Allocator implementation returned a pointer which overlaps with another allocated block (above). This can be due to a bug in the allocator code or because two allocators share the same pool.");
        }
    }
}
#endif

file_scope void *encode_header(void *p, s64 userSize, u32 align, allocator alloc, u64 flags) {
    u32 padding          = calculate_padding_for_pointer_with_header(p, align, sizeof(allocation_header));
    u32 alignmentPadding = padding - sizeof(allocation_header);

    auto *result = (allocation_header *) ((char *) p + alignmentPadding);

    result->Alloc = alloc;
    result->Size  = userSize;

    result->Alignment        = align;
    result->AlignmentPadding = alignmentPadding;

    //
    // This is now safe since we handle alignment here (and not in general_(re)allocate).
    // Before I wrote the fix the program was crashing because of SIMD types,
    // which require memory to be 16 byte aligned. I tried allocating with specified
    // alignment but it wasn't taking into account the size of the allocation header
    // (accounting happened before bumping the resulting pointer here).
    //
    // Since I had to redo how alignment was handled I decided to remove ALLOCATE_ALIGNED
    // and REALLOCATE_ALIGNED and drastically simplify allocator implementations.
    // What we do now is request a block of memory with extra size
    // that takes into account possible padding for alignment.
    //                                                                              - 5.04.2020
    //
    // Now we do this differently (again) because there was a bug where reallocating was having
    // issues with _AlignmentPadding_. Now we require allocators to implement RESIZE instead of REALLOCATE
    // which mustn't move the block but instead return null if resizing failed to tell us we need to allocate a new one.
    // This moves handling reallocation entirely on our side, which, again is even cleaner.
    //                                                                              - 18.05.2020
    //
    p = result + 1;
    assert((((u64) p & ~((s64) align - 1)) == (u64) p) && "Pointer wasn't properly aligned.");

#if defined DEBUG_MEMORY
    memset((byte *) p, CLEAN_LAND_FILL, userSize);

    memset((byte *) p - NO_MANS_LAND_SIZE, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
    memset((byte *) p + userSize, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);

    result->DEBUG_Pointer = p;
#endif

    return p;
}

// Without using the lstd.fmt module, i.e. without allocations.
file_scope void log_file_and_line(source_location loc) {
    write(Context.Log, loc.File);
    write(Context.Log, ":");

    char number[20];

    auto line = loc.Line;

    auto *numberP  = number + 19;
    s64 numberSize = 0;
    {
        while (line) {
            *numberP-- = line % 10 + '0';
            line /= 10;
            ++numberSize;
        }
    }
    write(Context.Log, numberP + 1, numberSize);
}

void *general_allocate(allocator alloc, s64 userSize, u32 alignment, u64 options, source_location loc) {
    if (!alloc) alloc = Context.Alloc;
    assert(alloc && "Context allocator was null. The programmer should set it before calling allocate functions.");

    options |= Context.AllocOptions;

    if (alignment == 0) {
        auto contextAlignment = Context.AllocAlignment;
        assert(is_pow_of_2(contextAlignment));
        alignment = contextAlignment;
    }

#if defined DEBUG_MEMORY
    debug_memory_maybe_verify_heap();
    s64 id = AllocationCount;

    if (id == 723) {
        s32 k = 42;
    }
#endif

    if (Context.LogAllAllocations && !Context._LoggingAnAllocation) {
        auto newContext                 = Context;
        newContext._LoggingAnAllocation = true;

        PUSH_CONTEXT(newContext) {
            write(Context.Log, ">>> Starting allocation at: ");
            log_file_and_line(loc);
            write(Context.Log, "\n");
        }
    }

    alignment = alignment < POINTER_SIZE ? POINTER_SIZE : alignment;
    assert(is_pow_of_2(alignment));

    s64 required = userSize + alignment + sizeof(allocation_header) + sizeof(allocation_header) % alignment;
#if defined DEBUG_MEMORY
    required += NO_MANS_LAND_SIZE;  // This is for the safety bytes after the requested block
#endif

    void *block = alloc.Function(allocator_mode::ALLOCATE, alloc.Context, required, null, 0, options);
    assert(block);

    auto *result = encode_header(block, userSize, alignment, alloc, options);

#if defined DEBUG_MEMORY
    auto *header = (allocation_header *) result - 1;

    auto *node = list_search(header);

    debug_memory_node *nodeToEncode = null;
    if (node->Header == header) {
        if (!node->Freed) {
            // Maybe this is a bug in the allocator implementation,
            // or maybe two different allocators use the same pool.
            assert(false && "Allocator implementation returning a pointer which is still live and wasn't freed yet");
            return null;
        }

        // Overwrite node which was marked as freed.
        node->Header = header;
        nodeToEncode = node;
    }

    if (!nodeToEncode) {
        nodeToEncode = list_add(header);
    }

    check_for_overlapping_blocks(nodeToEncode);

    nodeToEncode->ID = AllocationCount;
    atomic_inc(&AllocationCount);

    nodeToEncode->AllocatedAt = loc;

    nodeToEncode->RID          = 0;
    nodeToEncode->MarkedAsLeak = options & LEAK;

    nodeToEncode->Freed = false;

    nodeToEncode->FreedAt = {};
#endif

    return result;
}

void *general_reallocate(void *ptr, s64 newUserSize, u64 options, source_location loc) {
    options |= Context.AllocOptions;

    auto *header = (allocation_header *) ptr - 1;

#if defined DEBUG_MEMORY
    debug_memory_maybe_verify_heap();

    auto *node = list_search(header);
    if (node->Header != header) {
        // @TODO: Callstack
        panic(tprint("{!RED}Attempting to reallocate a memory block which was not allocated in the heap.{!} This happened at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!}).", loc.File, loc.Line, loc.Function));
        return null;
    }

    if (node->Freed) {
        // @TODO: Callstack
        panic(tprint("{!RED}Attempting to reallocate a memory block which was freed.{!} The free happened at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!}).", node->FreedAt.File, node->FreedAt.Line, node->FreedAt.Function));
        return null;
    }
#endif

    if (header->Size == newUserSize) [[unlikely]] {
        return ptr;
    }

    if (Context.LogAllAllocations && !Context._LoggingAnAllocation) [[unlikely]] {
        auto newContext                 = Context;
        newContext._LoggingAnAllocation = true;

        PUSH_CONTEXT(newContext) {
            write(Context.Log, ">>> Starting reallocation at: ");
            log_file_and_line(loc);
            write(Context.Log, "\n");
        }
    }

    // The header stores just the size of the requested allocation
    // (so the user code can look at the header and not be confused with garbage)
    s64 extra = sizeof(allocation_header) + header->Alignment + sizeof(allocation_header) % header->Alignment;
#if defined DEBUG_MEMORY
    extra += NO_MANS_LAND_SIZE;
#endif

    s64 oldUserSize = header->Size;
    s64 oldSize     = oldUserSize + extra;
    s64 newSize     = newUserSize + extra;

    auto alloc = header->Alloc;

    void *block = (byte *) header - header->AlignmentPadding;

    void *result = ptr;

    // Try to resize the block, this returns null if the block can't be resized and we need to move it.
    void *newBlock = alloc.Function(allocator_mode::RESIZE, alloc.Context, newSize, block, oldSize, options);
    if (!newBlock) {
        // Memory needs to be moved
        void *newBlock = alloc.Function(allocator_mode::ALLOCATE, alloc.Context, newSize, null, 0, options);
        assert(newBlock);

        result = encode_header(newBlock, newUserSize, header->Alignment, alloc, options);

        // We can't just override the header cause we need to keep the list sorted by the header address
        header = (allocation_header *) result - 1;

#if defined DEBUG_MEMORY
        // See note in _general_free()_
        node->Freed   = true;
        node->FreedAt = loc;

        // @Volatile
        auto id              = node->ID;
        auto rid             = node->RID;
        bool wasMarkedAsLeak = node->MarkedAsLeak;

        node = list_add(header);
#endif

        // Copy old state
#if defined DEBUG_MEMORY
        node->ID           = id;
        node->RID          = rid;
        node->MarkedAsLeak = wasMarkedAsLeak;
#endif

        // Copy old stuff and free
        memcpy((char *) result, (char *) ptr, oldUserSize);
        alloc.Function(allocator_mode::FREE, alloc.Context, 0, block, oldSize, options);
    } else {
        //
        // The block was resized sucessfully and it doesn't need moving
        //

        assert(block == newBlock);  // Sanity

        header->Size = newUserSize;
    }

#if defined DEBUG_MEMORY
    check_for_overlapping_blocks(node);

    node->RID += 1;
    node->AllocatedAt = loc;

    if (oldSize < newSize) {
        // If we are expanding the memory, fill the new stuff with CLEAN_LAND_FILL
        memset((byte *) result + oldUserSize, CLEAN_LAND_FILL, newSize - oldSize);
    } else {
        // If we are shrinking the memory, fill the old stuff with DEAD_LAND_FILL
        memset((byte *) header + oldSize, DEAD_LAND_FILL, oldSize - newSize);
    }

    memset((byte *) result + newUserSize, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
#endif

    return result;
}

void general_free(void *ptr, u64 options, source_location loc) {
    if (!ptr) return;

    options |= Context.AllocOptions;

    auto *header = (allocation_header *) ptr - 1;

#if defined DEBUG_MEMORY
    debug_memory_maybe_verify_heap();

    auto *node = list_search(header);
    if (node->Header != header) {
        // @TODO: Callstack
        panic(tprint("Attempting to free a memory block which was not heap allocated (in this thread)."));

        // Note: We don't support cross-thread freeing yet.

        return;
    }

    if (node->Freed) {
        panic(tprint("{!RED}Attempting to free a memory block which was already freed.{!} The previous free happened at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!})", node->FreedAt.File, node->FreedAt.Line, node->FreedAt.Function));
        return;
    }
#endif


    auto alloc  = header->Alloc;
    void *block = (byte *) header - header->AlignmentPadding;

    s64 extra = header->Alignment + sizeof(allocation_header) + sizeof(allocation_header) % header->Alignment;
#if defined DEBUG_MEMORY
    extra += NO_MANS_LAND_SIZE;
#endif

    s64 size = header->Size + extra;

#if defined DEBUG_MEMORY
    // If DEBUG_MEMORY we keep freed notes in the list
    // but mark them as freed. This allows debugging double freeing the same memory block.

    node->Freed   = true;
    node->FreedAt = loc;

    memset((byte *) block, DEAD_LAND_FILL, size);

    auto id = node->ID;
#endif

    alloc.Function(allocator_mode::FREE, alloc.Context, 0, block, size, options);
}

void free_all(allocator alloc, u64 options) {
#if defined DEBUG_MEMORY
    // Remove allocations made with the allocator from the the linked list so we don't corrupt the heap
    auto *it = DebugMemoryHead->Next;
    while (it != DebugMemoryTail) {
        if (!it->Freed) {
            if (it->Header->Alloc == alloc) {
                it->Freed   = true;
                it->FreedAt = source_location::current();
            }
        }
        it = it->Next;
    }
#endif

    options |= Context.AllocOptions;
    alloc.Function(allocator_mode::FREE_ALL, alloc.Context, 0, 0, 0, options);
}

LSTD_END_NAMESPACE

LSTD_USING_NAMESPACE;

extern "C" {
void *malloc(size_t size) { return (void *) malloc<byte>({.Count = (s64) size}); }

void *calloc(size_t num, size_t size) {
    void *block = malloc(num * size);
    memset0((byte *) block, num * size);
    return block;
}

void *realloc(void *block, size_t newSize) {
    if (!block) {
        return malloc(newSize);
    }
    return (void *) realloc((byte *) block, {.NewCount = (s64) newSize});
}

// No need to define this global function if the library was built without a namespace
void free(void *block) { free((byte *) block); }
}

[[nodiscard]] void *operator new(size_t size) { return general_allocate(Context.Alloc, size, 0, 0, source_location::current()); }
[[nodiscard]] void *operator new[](size_t size) { return general_allocate(Context.Alloc, size, 0, 0, source_location::current()); }

[[nodiscard]] void *operator new(size_t size, align_val_t alignment) { return general_allocate(Context.Alloc, size, (u32) alignment, 0, source_location::current()); }
[[nodiscard]] void *operator new[](size_t size, align_val_t alignment) { return general_allocate(Context.Alloc, size, (u32) alignment, 0, source_location::current()); }

void operator delete(void *ptr, align_val_t alignment) noexcept { general_free(ptr, 0, source_location::current()); }
void operator delete[](void *ptr, align_val_t alignment) noexcept { general_free(ptr, 0, source_location::current()); }
