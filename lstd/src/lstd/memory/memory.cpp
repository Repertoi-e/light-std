module;

#include "../common.h"

module lstd.memory;

import lstd.basic;
import lstd.path;
import lstd.fmt;
import lstd.os;

LSTD_BEGIN_NAMESPACE

#if defined DEBUG_MEMORY

/*
 * The five following functions handle the low-order mark bit that indicates
 * whether a node is logically deleted (1) or not (0).
 *  - is_marked_ref returns whether it is marked, 
 *  - (un)set_marked changes the mark,
 *  - get_(un)marked_ref sets the mark before returning the node.
 */

bool is_marked_ref(void *i) { return ((u64) i) & 0x1LL; }
void *get_unmarked_ref(void *w) { return (void *) (((u64) w) & ~0x1LL); }
void *get_marked_ref(void *w) { return (void *) (((u64) w) | 0x1LL); }
void *unset_mark(void *i) { return (void *) (((u64) i) & ~0x1LL); }
void *set_mark(void *i) { return (void *) (((u64) i) | 0x1LL); }

void debug_memory::init_list() {
    // @Cleanup @Platform @TODO @Memory Don't use the os allocator. We should have a seperate allocator for debug info.
    auto [sentinel1, sentinel2, _] = os_allocate_packed<node, node>(1);

    sentinel1->Header = (allocation_header *) 0;
    sentinel1->Header = (allocation_header *) U64_MAX;

    Head       = sentinel1;
    Tail       = sentinel2;
    Head->Next = Tail;
}

debug_memory::node *debug_memory::new_node(allocation_header *header, node *next) {
    // @Cleanup @Platform @TODO @Memory Don't use the os allocator. We should have a seperate allocator for debug info.
    auto *result   = (node *) os_allocate_block(sizeof(node));
    result->Header = header;
    result->Next   = next;
    return result;
}

debug_memory::list_search_result debug_memory::list_search(allocation_header *header) {
    node *leftNodeNext = null;
    node *leftNode = null, *rightNode = null;

    while (true) {
        node *t     = Head;
        node *tNext = Head->Next;
        while (is_marked_ref(tNext) || (t->Header < header)) {
            if (!is_marked_ref(tNext)) {
                leftNode     = t;
                leftNodeNext = tNext;
            }
            t = (node *) get_unmarked_ref(tNext);
            if (t == Tail) break;
            tNext = t->Next;
        }
        rightNode = t;

        if (leftNodeNext == rightNode) {
            if (!is_marked_ref(rightNode->Next)) {
                return {leftNode, rightNode};
            }
        } else {
            if (atomic_compare_and_swap(&leftNode->Next, leftNodeNext, rightNode) == leftNodeNext) {
                if (!is_marked_ref(rightNode->Next)) {
                    return {leftNode, rightNode};
                }
            }
        }
    }
}

bool debug_memory::list_contains(allocation_header *header) {
    auto *it = (node *) get_unmarked_ref(Head->Next);
    while (it != Tail) {
        if (!is_marked_ref(it->Next) && it->Header >= header) {
            return it->Header == header;
        }
        it = (node *) get_unmarked_ref(it->Next);
    }
    return false;
}

debug_memory::list_add_result debug_memory::list_add(allocation_header *header) {
    node *newNode = null;
    while (true) {
        auto [left, right] = list_search(header);
        if (right != Tail && right->Header == header) {
            return {right, true};
        }

        if (!newNode) {
            newNode = new_node(header, right);
        } else {
            newNode->Next = right;
        }

        if (atomic_compare_and_swap(&left->Next, right, newNode) == right) {
            return {newNode, false};
        }
    }
}

bool debug_memory::list_remove(allocation_header *header) {
    while (true) {
        auto [left, right] = list_search(header);

        // Check if we found our node
        if (right == Tail || right->Header != header) return false;

        auto *rightNext = right->Next;
        if (!is_marked_ref(rightNext)) {
            if (atomic_compare_and_swap(&(right->Next), rightNext, (node *) get_marked_ref(rightNext)) == rightNext) return true;
        }
    }
}

// Copied from test.h
//
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      /home/.../game/src/some_dir/a/string.cpp ---> some_dir/a/localization.cpp
//      /home/.../game/some_dir/string.cpp       ---> localization.cpp
//
constexpr string get_short_file_name(string str) {
    char srcData[] = {'s', 'r', 'c', OS_PATH_SEPARATOR, '\0'};
    string src     = srcData;

    s64 findResult = string_find(str, src, string_length(str), true);
    if (findResult == -1) {
        findResult = string_find(str, OS_PATH_SEPARATOR, string_length(str), true);
        assert(findResult != string_length(str) - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        findResult += string_length(src);
    }

    string result = str;
    return substring(result, findResult, string_length(result));
}

void debug_memory::report_leaks() {
    maybe_verify_heap();

    s64 leaksCount = 0;

    // @Cleanup: Factor this into a macro
    auto *it = (node *) get_unmarked_ref(Head->Next);
    while (it != Tail) {
        if (!is_marked_ref(it->Next) && !it->MarkedAsFreed && !it->MarkedAsLeak) {
            ++leaksCount;
        }
        it = (node *) get_unmarked_ref(it->Next);
    }

    // @Cleanup @Platform @TODO @Memory Don't use the platform allocator. We should have a seperate allocator for debug info.
    node **leaks = malloc<node *>({.Count = leaksCount, .Alloc = platform_get_persistent_allocator(), .Options = LEAK});
    defer(free(leaks));

    auto *p = leaks;

    // @Cleanup: Factor this into a macro
    it = (node *) get_unmarked_ref(Head->Next);
    while (it != Tail) {
        if (!is_marked_ref(it->Next) && !it->MarkedAsFreed && !it->MarkedAsLeak) {
            *p++ = it;
        }
        it = (node *) get_unmarked_ref(it->Next);
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

file_scope void verify_header_integrity(allocation_header *header) {
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // If an assert fires here it means that memory was messed up in some way.
    //
    // We check for several problems here:
    //   * Accessing headers which were freed.
    //   * Alignment should not be 0, should be more than POINTER_SIZE (8 bytes) and should be a power of 2.
    //     If any of these is not true, then the header was definitely corrupted.
    //   * We store a pointer to the memory block at the end of the header, any valid header will have this pointer point after itself.
    //     Otherwise the header was definitely corrupted.
    //   * No man's land was modified. This means that you wrote before or after the allocated block.
    //     This catches buffer underflows/overflows errors.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    char freedHeader[sizeof(allocation_header)];
    fill_memory(freedHeader, DEAD_LAND_FILL, sizeof(allocation_header));
    if (compare_memory((char *) header, freedHeader, sizeof(allocation_header)) == 0) {
        assert(false && "Trying to access freed memory!");
    }

    assert(header->Alignment && "Stored alignment is zero. Definitely corrupted.");
    assert(header->Alignment >= POINTER_SIZE && "Stored alignment smaller than pointer size (8 bytes). Definitely corrupted.");
    assert(is_pow_of_2(header->Alignment) && "Stored alignment not a power of 2. Definitely corrupted.");

    assert(header->DEBUG_Pointer == header + 1 && "Debug pointer doesn't match. They should always match.");

    auto *user = (char *) header + sizeof(allocation_header);

    char noMansLand[NO_MANS_LAND_SIZE];
    fill_memory(noMansLand, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
    assert(compare_memory((char *) user - NO_MANS_LAND_SIZE, noMansLand, NO_MANS_LAND_SIZE) == 0 &&
           "No man's land was modified. This means that you wrote before the allocated block.");

    assert(compare_memory((char *) header->DEBUG_Pointer + header->Size, noMansLand, NO_MANS_LAND_SIZE) == 0 &&
           "No man's land was modified. This means that you wrote after the allocated block.");

    //
    // If one of these asserts was triggered in _maybe_verify_heap()_, this can also mean that the linked list is messed up in some way.
    //
}

void debug_memory::verify_heap() {
    auto *it = (node *) get_unmarked_ref(Head->Next);
    while (it != Tail) {
        if (!is_marked_ref(it->Next) && !it->MarkedAsFreed) {
            verify_header_integrity(it->Header);
        }
        it = (node *) get_unmarked_ref(it->Next);
    }
}

void debug_memory::maybe_verify_heap() {
    if (AllocationCount % MemoryVerifyHeapFrequency) return;
    verify_heap();
}

void check_for_overlapping_blocks(debug_memory::node *left, debug_memory::node *right, allocation_header *header) {
    // Check for overlapping memory blocks.
    // We can do this because we keep the linked list sorted by the memory address
    // of individual allocated blocks and we have info about their size.
    // This might catch bugs in the allocator implementation/two allocators using the same pool.

    while (right->MarkedAsFreed) {
        right = (debug_memory::node *) get_unmarked_ref(right->Next);
    }

    /*
       @TODO: Because we need to skip nodes that are marked as freed, currently we don't check for overlapping blocks below.
       In order to do this without abysmal performance we need a doubly-linked list. Figure out how to do this lock-free.

    // Check below
    if (left != DEBUG_memory->Head) {
        s64 size = left->Header->Size;
#if defined DEBUG_MEMORY
        size += NO_MANS_LAND_SIZE;
#endif
        if (((byte *) left->Header + size) > (byte *) header) {
            assert(false && "Allocator implementation returned a pointer which overlaps with another allocated block (below). This can be due to a bug in the allocator code or because two allocators use the same pool.");
        }
    }*/

    // Check above
    if (right != DEBUG_memory->Tail) {
        s64 size = header->Size;
#if defined DEBUG_MEMORY
        size += NO_MANS_LAND_SIZE;
#endif

        if (((byte *) header + size) < ((byte *) right->Header - right->Header->AlignmentPadding)) {
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
    // Before I wrote the fix the program was crashing because I was using SIMD types,
    // which require to be aligned, on memory not 16-aligned.
    // I tried allocating with specified alignment but it wasn't taking into
    // account the size of the allocation header (accounting happened before bumping
    // the resulting pointer here).
    //
    // Since I had to redo how alignment was handled I decided to remove ALLOCATE_ALIGNED
    // and REALLOCATE_ALIGNED and drastically simplify allocator implementations.
    // What we do now is request a block of memory with a size
    // that was calculated with alignment in mind.
    //                                                                              - 5.04.2020
    //
    // Since then we do this again differently because there was a bug where reallocating was having
    // issues with _AlignmentPadding_. Now we require allocators to implement RESIZE instead of REALLOCATE
    // which mustn't move the block but instead return null if resizing failed to tell us we need to allocate a new one.
    // This moves handling reallocation entirely on our side, which, again is even cleaner.
    //                                                                              - 18.05.2020
    //
    p = result + 1;
    assert((((u64) p & ~((s64) align - 1)) == (u64) p) && "Pointer wasn't properly aligned.");

#if defined DEBUG_MEMORY
    fill_memory(p, CLEAN_LAND_FILL, userSize);

    fill_memory((char *) p - NO_MANS_LAND_SIZE, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
    fill_memory((char *) p + userSize, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);

    result->DEBUG_Pointer = result + 1;
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
    DEBUG_memory->maybe_verify_heap();
    s64 id = DEBUG_memory->AllocationCount;

    if (id == 75) {
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

    auto [left, right] = DEBUG_memory->list_search(header);
    assert(right != DEBUG_memory->Tail && "?????");  // The Tail is a sentinel value with Header == max address. This should NEVER trip.

    debug_memory::node *nodeToEncode = null;

    if (right->Header == header) {
        // Maybe we caught a bug in the allocator implementation,
        // or maybe two different allocators use the same pool.
        assert(right->MarkedAsFreed && "Allocator implementation returning a pointer which is still live and wasn't freed yet");

        // Overwrite node which was freed.
        right->Header = header;
        nodeToEncode  = right;
        right         = right->Next;  // We need the two adjacent nodes in order to check for overlapping blocks below
    }

    check_for_overlapping_blocks(left, right, header);

    if (!nodeToEncode) {
        auto [addedNode, alreadyPresent] = DEBUG_memory->list_add(header);
        assert(!alreadyPresent);
        nodeToEncode = addedNode;
    }

    nodeToEncode->ID = DEBUG_memory->AllocationCount;
    atomic_inc(&DEBUG_memory->AllocationCount);

    nodeToEncode->AllocatedAt = loc;

    nodeToEncode->RID          = 0;
    nodeToEncode->MarkedAsLeak = options & LEAK;

    nodeToEncode->MarkedAsFreed = false;

    nodeToEncode->FreedAt = {};
#endif

    return result;
}

void *general_reallocate(void *ptr, s64 newUserSize, u64 options, source_location loc) {
    options |= Context.AllocOptions;

#if defined DEBUG_MEMORY
    DEBUG_memory->maybe_verify_heap();
#endif

    auto *header = (allocation_header *) ptr - 1;

    auto [left, right] = DEBUG_memory->list_search(header);
    assert(right != DEBUG_memory->Tail && "?????");  // The Tail is a sentinel value with Header == max address. This should NEVER trip.

    if (right->Header != header) {
        // @TODO: Callstack
        panic(tprint("{!RED}Attempting to reallocate a memory block which was not allocated in the heap.{!} This happened at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!}).", loc.File, loc.Line, loc.Function));
        return null;
    }

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

    if (right->MarkedAsFreed) {
        // @TODO: Callstack
        panic(tprint("{!RED}Attempting to reallocate a memory block which was freed.{!} The free happened at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!}).", right->FreedAt.File, right->FreedAt.Line, right->FreedAt.Function));
        return null;
    }

    void *result = ptr;

    // Try to resize the block, this returns null if the block can't be resized and we need to move it.
    void *newBlock = alloc.Function(allocator_mode::RESIZE, alloc.Context, newSize, block, oldSize, options);
    if (!newBlock) {
        // Memory needs to be moved
        void *newBlock = alloc.Function(allocator_mode::ALLOCATE, alloc.Context, newSize, null, 0, options);
        assert(newBlock);

        result = encode_header(newBlock, newUserSize, header->Alignment, alloc, options);

        header        = (allocation_header *) result - 1;
        right->Header = header;

        // Copy old stuff and free
        copy_memory(result, ptr, oldUserSize);
        alloc.Function(allocator_mode::FREE, alloc.Context, 0, block, oldSize, options);
    } else {
        //
        // The block was resized sucessfully and it doesn't need moving
        //

        assert(block == newBlock);  // Sanity

        header->Size = newUserSize;
    }

#if defined DEBUG_MEMORY
    check_for_overlapping_blocks(left, (debug_memory::node *) get_unmarked_ref(right->Next), header);
    right->RID += 1;
    right->AllocatedAt = loc;

    if (oldSize < newSize) {
        // If we are expanding the memory, fill the new stuff with CLEAN_LAND_FILL
        fill_memory((char *) result + oldUserSize, CLEAN_LAND_FILL, newSize - oldSize);
    } else {
        // If we are shrinking the memory, fill the old stuff with DEAD_LAND_FILL
        fill_memory((char *) header + oldSize, DEAD_LAND_FILL, oldSize - newSize);
    }

    fill_memory((char *) result + newUserSize, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
#endif

    return result;
}

void general_free(void *ptr, u64 options, source_location loc) {
    if (!ptr) return;

    options |= Context.AllocOptions;

#if defined DEBUG_MEMORY
    DEBUG_memory->maybe_verify_heap();
#endif

    auto *header = (allocation_header *) ptr - 1;

    auto [left, right] = DEBUG_memory->list_search(header);
    assert(right != DEBUG_memory->Tail && "?????");  // The Tail is a sentinel value with Header == max address. This should NEVER trip.

    if (right->Header != header) {
        // @TODO: Callstack
        panic(tprint("Attempting to free a memory block which was not allocated in the heap."));
        return;
    }

    if (right->MarkedAsFreed) {
        panic(tprint("{!RED}Attempting to free a memory block which was already freed.{!} The previous free happened at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!})", right->FreedAt.File, right->FreedAt.Line, right->FreedAt.Function));
        return;
    }

    auto alloc  = header->Alloc;
    void *block = (byte *) header - header->AlignmentPadding;

    s64 extra = header->Alignment + sizeof(allocation_header) + sizeof(allocation_header) % header->Alignment;
#if defined DEBUG_MEMORY
    extra += NO_MANS_LAND_SIZE;
#endif

    s64 size = header->Size + extra;

#if defined DEBUG_MEMORY
    right->MarkedAsFreed = true;
    right->FreedAt       = loc;

    fill_memory(block, DEAD_LAND_FILL, size);

    auto id = right->ID;
#endif

    alloc.Function(allocator_mode::FREE, alloc.Context, 0, block, size, options);
}

void free_all(allocator alloc, u64 options) {
#if defined DEBUG_MEMORY
    // Remove allocations made with the allocator from the the linked list so we don't corrupt the heap
    auto *it = (debug_memory::node *) get_unmarked_ref(DEBUG_memory->Head->Next);
    while (it != DEBUG_memory->Tail) {
        if (!is_marked_ref(it->Next) && !it->MarkedAsFreed) {
            if (it->Header->Alloc == alloc) {
                it->MarkedAsFreed = true;
                it->FreedAt       = source_location::current();
            }
        }
        it = (debug_memory::node *) get_unmarked_ref(it->Next);
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
    zero_memory(block, num * size);
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
