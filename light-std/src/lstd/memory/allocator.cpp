#include "allocator.h"

#include "../file.h"
#include "../internal/context.h"
#include "../io/fmt.h"
#include "../math.h"
#include "../os.h"

LSTD_BEGIN_NAMESPACE

#if defined DEBUG_MEMORY
void allocator::DEBUG_unlink_header(allocation_header *header) {
    thread::scoped_lock<thread::mutex> _(&DEBUG_Mutex);

    assert(header);
    assert(DEBUG_Head);

    if (header == DEBUG_Head) {
        DEBUG_Head = header->DEBUG_Next;
    }

    if (header->DEBUG_Next) {
        header->DEBUG_Next->DEBUG_Previous = header->DEBUG_Previous;
    }

    if (header->DEBUG_Previous) {
        header->DEBUG_Previous->DEBUG_Next = header->DEBUG_Next;
    }
}
void allocator::DEBUG_add_header(allocation_header *header) {
    thread::scoped_lock<thread::mutex> _(&DEBUG_Mutex);

    header->DEBUG_Next = DEBUG_Head;
    if (DEBUG_Head) {
        DEBUG_Head->DEBUG_Previous = header;
    }
    DEBUG_Head = header;
}
void allocator::DEBUG_swap_header(allocation_header *oldHeader, allocation_header *newHeader) {
    auto *prev = oldHeader->DEBUG_Previous;
    auto *next = oldHeader->DEBUG_Next;

    assert(DEBUG_Head);  // ?

    thread::scoped_lock<thread::mutex> _(&DEBUG_Mutex);
    if (prev) {
        prev->DEBUG_Next = newHeader;
        newHeader->DEBUG_Previous = prev;
    } else {
        DEBUG_Head = newHeader;
    }

    if (next) {
        next->DEBUG_Previous = newHeader;
        newHeader->DEBUG_Next = next;
    }
}
#endif

void allocator::verify_header(allocation_header *header) {
#if defined DEBUG_MEMORY
    char freedHeader[sizeof(allocation_header)];
    fill_memory(freedHeader, DEAD_LAND_FILL, sizeof(allocation_header));
    if (compare_memory(header, freedHeader, sizeof(allocation_header)) == -1) {
        assert(false && "Trying to access freed memory!");
    }

    assert(header->Alignment && "Alignment is zero. Definitely corrupted.");
    assert(header->Alignment >= POINTER_SIZE && "Alignment smaller than pointer size. Definitely corrupted.");
    assert(is_pow_of_2(header->Alignment) && "Alignment not a power of 2. Definitely corrupted.");

    assert(header->DEBUG_Pointer == header + 1 && "Debug pointer doesn't match. They should always match.");

    auto *user = (char *) header + sizeof(allocation_header);

    char noMansLand[NO_MANS_LAND_SIZE];
    fill_memory(noMansLand, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
    assert(compare_memory((char *) user - NO_MANS_LAND_SIZE, noMansLand, NO_MANS_LAND_SIZE) == -1 &&
           "No man's land was modified. This means that you wrote before the allocated block.");

    assert(compare_memory((char *) header->DEBUG_Pointer + header->Size, noMansLand, NO_MANS_LAND_SIZE) == -1 &&
           "No man's land was modified. This means that you wrote after the allocated block.");

    //
    // If one of these asserts was triggered in _verify_heap()_, this can also mean that the linked list is messed up
    // (possibly by modifying the pointers in the header).
    //
#endif
}

void allocator::verify_heap() {
#if defined DEBUG_MEMORY
    // We need to lock here because another thread can free a header while we are reading from it.
    thread::scoped_lock<thread::mutex> _(&DEBUG_Mutex);

    auto *it = DEBUG_Head;
    while (it) {
        verify_header(it);
        it = it->DEBUG_Next;
    }
#endif
}

void *allocator::encode_header(void *p, s64 userSize, u32 align, allocator_func_t f, void *c, u64 flags) {
    u32 padding = calculate_padding_for_pointer_with_header(p, align, sizeof(allocation_header));
    u32 alignmentPadding = padding - sizeof(allocation_header);

    auto *result = (allocation_header *) ((char *) p + alignmentPadding);

#if defined DEBUG_MEMORY
    result->DEBUG_Next = null;
    result->DEBUG_Previous = null;

    result->ID = (u32) AllocationCount;
    atomic_inc_64(&AllocationCount);

    result->RID = 0;
#endif

    result->Function = f;
    result->Context = c;
    result->Size = userSize;

    result->Alignment = align;
    result->AlignmentPadding = alignmentPadding;

    result->Owner = null;

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

    if (flags & DO_INIT_0) {
        zero_memory(p, userSize);
    }
#if defined DEBUG_MEMORY
    else {
        fill_memory(p, CLEAN_LAND_FILL, userSize);
    }

    fill_memory((char *) p - NO_MANS_LAND_SIZE, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
    fill_memory((char *) p + userSize, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);

    result->DEBUG_Pointer = result + 1;

    result->MarkedAsLeak = flags & LEAK;
#endif

    return p;
}

void *allocator::general_allocate(s64 userSize, u32 align, u64 userFlags) const {
    if (align == 0) {
        auto contextAlignment = ::Context.AllocAlignment;
        assert(is_pow_of_2(contextAlignment));
        align = contextAlignment;
    }

#if defined DEBUG_MEMORY
    s64 id = AllocationCount;

    if (id == 7) {
        s32 k = 42;
    }
#endif

    align = align < POINTER_SIZE ? POINTER_SIZE : align;
    assert(is_pow_of_2(align));

    s64 required = userSize + align + sizeof(allocation_header) + (sizeof(allocation_header) % align);
#if defined DEBUG_MEMORY
    required += NO_MANS_LAND_SIZE;  // This is for the bytes after the requested block
#endif

    void *block = Function(allocator_mode::ALLOCATE, Context, required, null, 0, userFlags);
    auto *result = encode_header(block, userSize, align, Function, Context, userFlags);

#if defined DEBUG_MEMORY
    auto *header = (allocation_header *) result - 1;
    DEBUG_add_header(header);
#endif

    verify_heap();

    return result;
}

void *allocator::general_reallocate(void *ptr, s64 newUserSize, u64 userFlags) {
    auto *header = (allocation_header *) ptr - 1;
    verify_header(header);

    if (header->Size == newUserSize) return ptr;

#if defined DEBUG_MEMORY
    auto id = header->ID;
#endif

    // The header stores the size of the requested allocation
    // (so the user code can look at the header and not be confused with garbage)
    s64 extra = sizeof(allocation_header) + header->Alignment + (sizeof(allocation_header) % header->Alignment);

    s64 oldUserSize = header->Size;

    s64 oldSize = oldUserSize + extra;
    s64 newSize = newUserSize + extra;

#if defined DEBUG_MEMORY
    oldSize += NO_MANS_LAND_SIZE;
    newSize += NO_MANS_LAND_SIZE;
#endif

    auto *func = header->Function;
    auto *context = header->Context;

    void *block = (char *) header - header->AlignmentPadding;

    void *p;

    // Try to resize the block, this returns null if the block can't be resized and we need to move it.
    void *newBlock = func(allocator_mode::RESIZE, context, newSize, block, oldSize, userFlags);
    if (!newBlock) {
        // Memory needs to be moved
        void *newBlock = func(allocator_mode::ALLOCATE, context, newSize, null, 0, userFlags);
        auto *newPointer = encode_header(newBlock, newUserSize, header->Alignment, func, context, userFlags);

        auto *newHeader = (allocation_header *) newPointer - 1;

        newHeader->Owner = header->Owner;
        copy_memory(newPointer, ptr, header->Size);

#if defined DEBUG_MEMORY
        newHeader->ID = id;
        newHeader->RID = header->RID + 1;

        DEBUG_swap_header(header, newHeader);
        fill_memory(block, DEAD_LAND_FILL, oldSize);

        newHeader->MarkedAsLeak = header->MarkedAsLeak;
#endif
        func(allocator_mode::FREE, context, 0, block, oldSize, userFlags);

        p = (void *) (newHeader + 1);
    } else {
        // The block was resized sucessfully and it doesn't need moving
        assert(block == newBlock);  // Sanity

#if defined DEBUG_MEMORY
        ++header->RID;
#endif
        header->Size = newUserSize;

        p = (void *) (header + 1);
    }

    if (oldSize < newSize) {
        if (userFlags & DO_INIT_0) {
            fill_memory((char *) p + oldUserSize, 0, newSize - oldSize);
        }
#if defined DEBUG_MEMORY
        else {
            fill_memory((char *) p + oldUserSize, CLEAN_LAND_FILL, newSize - oldSize);
        }
#endif
    }
#if defined DEBUG_MEMORY
    else {
        // If we are shrinking the memory, fill the old stuff with DEAD_LAND_FILL
        fill_memory((char *) header + oldSize, DEAD_LAND_FILL, oldSize - newSize);
    }
#endif

#if defined DEBUG_MEMORY
    fill_memory((char *) p + newUserSize, NO_MANS_LAND_FILL, NO_MANS_LAND_SIZE);
#endif

    verify_heap();

    return p;
}

void allocator::free(void *ptr, u64 userFlags) {
    if (!ptr) return;

    auto *header = (allocation_header *) ptr - 1;
    verify_header(header);

#if defined DEBUG_MEMORY
    auto id = header->ID;
#endif

    s64 extra = header->Alignment + sizeof(allocation_header) + (sizeof(allocation_header) % header->Alignment);
    s64 size = header->Size + extra;
#if defined DEBUG_MEMORY
    size += NO_MANS_LAND_SIZE;
#endif

    auto *func = header->Function;
    auto *context = header->Context;

    void *block = (char *) header - header->AlignmentPadding;

#if defined DEBUG_MEMORY
    DEBUG_unlink_header(header);
    fill_memory(block, DEAD_LAND_FILL, size);
#endif

    func(allocator_mode::FREE, context, 0, block, size, userFlags);

    verify_heap();
}

void allocator::free_all(u64 userFlags) const {
    auto result = Function(allocator_mode::FREE_ALL, Context, 0, 0, 0, userFlags);
    assert((result != (void *) -1) && "Allocator doesn't support FREE_ALL");
}

LSTD_END_NAMESPACE

void *operator new(size_t size) { return operator new(size, Context.Alloc, 0); }
void *operator new[](size_t size) { return operator new(size, Context.Alloc, 0); }

void *operator new(size_t size, allocator alloc, u64 userFlags) noexcept {
    assert(alloc);
    return alloc.allocate(size, userFlags);
}

void *operator new[](size_t size, allocator alloc, u64 userFlags) noexcept {
    return operator new(size, alloc, userFlags);
}

void *operator new(size_t size, alignment align, u64 userFlags) noexcept {
    return Context.Alloc.allocate_aligned(size, align, userFlags);
}

void *operator new[](size_t size, alignment align, u64 userFlags) noexcept {
    return operator new(size, align, userFlags);
}

void *operator new(size_t size, alignment align, allocator alloc, u64 userFlags) noexcept {
    assert(alloc);
    return alloc.allocate_aligned(size, align, userFlags);
}

void *operator new[](size_t size, alignment align, allocator alloc, u64 userFlags) noexcept {
    return operator new(size, align, alloc, userFlags);
}

void operator delete(void *ptr) noexcept { allocator::free(ptr); }
void operator delete[](void *ptr) noexcept { allocator::free(ptr); }

void operator delete(void *ptr, allocator alloc, u64 userFlags) noexcept { allocator::free(ptr); }
void operator delete[](void *ptr, allocator alloc, u64 userFlags) noexcept { allocator::free(ptr); }

void operator delete(void *ptr, alignment align, u64 userFlags) noexcept { allocator::free(ptr); }
void operator delete[](void *ptr, alignment align, u64 userFlags) noexcept { allocator::free(ptr); }

void operator delete(void *ptr, alignment align, allocator alloc, u64 userFlags) noexcept { allocator::free(ptr); }
void operator delete[](void *ptr, alignment align, allocator alloc, u64 userFlags) noexcept { allocator::free(ptr); }
