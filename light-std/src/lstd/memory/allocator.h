#pragma once

/// Defines the structure of allocators in this library.
/// Provides a default thread-safe global allocator and thread local temporary allocator.

#include <new>

#include "../common.h"
#include "../intrin.h"

LSTD_BEGIN_NAMESPACE

//
// Allocators:
//

enum class allocator_mode { ALLOCATE = 0, REALLOCATE, FREE, FREE_ALL };

// We need strong typing in order to allow user flags in operator new() overloads
// Alignment is handled internally, so allocator implementations don't need to pay attention to it.
enum class alignment : u32;

// This is a user flag when allocating.
// When specified, the allocated memory is initialized.
// This is handled internally, so allocator implementations don't need to pay attention to it.
constexpr u64 DO_INIT_FLAG = 1ull << 31;

//
// OS allocator:
//
void *os_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64);

// This specifies what the signature of each allocation function should look like.
//
// _mode_ is what we are doing currently, allocating, resizing,
//      freeing a block or freeing everything
//      (*implementing FREE_ALL everything is not a requirement, the user decides when free_all is called)
// _context_ is used as a pointer to any data the allocator needs (e.g. state)
// _size_ is the size of the allocation
// _oldMemory_ is used when resizing
// _oldSize_ is the old size of memory block, used only when resizing
//
// the last u64 is reserved for user flags
//
// !!! When called with FREE_ALL, a return value of null means success!
//     To signify that the allocator doesn't support FREE_ALL (or the operation failed) return: (void*) -1
using allocator_func_t =
    add_pointer_t<void *(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64)>;

struct allocation_header {
    // Useful for debugging (better than file and line, in my opinion, because you can set a breakpoint with the ID
    // in allocate() below). Every allocation has an unique ID equal to the ID of the previous allocation + 1.
    // This is useful for debugging bugs related to allocations because (assuming your program isn't multithreaded)
    // each time you run your program the ID of each allocation is easily reproducible
    // (assuming no randomness from the user side).
    s64 ID;

    // The allocator used when allocating the memory
    allocator_func_t Function;
    void *Context;

    // The size of the allocation (NOT including the size of the header and padding)
    size_t Size;

    // The padding (in bytes) which was added to the pointer after allocating to make sure the resulting pointer is
    // aligned. The structure of an allocation is basically this:
    //
    // User requests allocation of _size_. The underlying allocator is called with
    //     _size_ + sizeof(allocation_header) + (sizeof(allocation_header) % alignment)
    //
    // The result:
    //   ...[..Alignment padding..][............Header............]............
    //      ^ The pointer returned by the allocator                ^ The resulting pointer (aligned)
    //
    u32 Alignment;  // The alignment which was specified when allocating
    u32 AlignmentPadding;

    // Note: This brings up the total size of the structure to 64 bytes, so we'll rarely have much alignment padding
    void *UserData1, *UserData2;

    // The pointer allocated (this is used to test if the header exists at all).
    // I mean... I guess it's not perfect...
    void *Pointer;
};

// Calculates the required padding in bytes which needs to be added to _ptr_ in order to be aligned.
inline size_t calculate_padding(void *ptr, u32 alignment) {
    assert(alignment > 0 && is_pow_of_2(alignment));
    return ((uptr_t) ptr / alignment + 1) * alignment - (uptr_t) ptr;
}

inline size_t calculate_padding_with_header(void *ptr, u32 alignment, size_t headerSize) {
    size_t padding = calculate_padding(ptr, alignment);

    if (padding < headerSize) {
        headerSize -= padding;
        if (headerSize % alignment > 0) {
            padding += alignment * (1 + (headerSize / alignment));
        } else {
            padding += alignment * (headerSize / alignment);
        }
    }
    return padding;
}

// Returns an aligned pointer.
inline void *get_aligned_pointer(void *ptr, size_t alignment) {
    assert(alignment > 0 && is_pow_of_2(alignment));
    return (void *) (((uptr_t) ptr + alignment - 1) & ~(alignment - 1));
}

struct allocator {
    allocator_func_t Function = null;
    void *Context = null;

    inline static s64 AllocationCount = 0;

    void *allocate(size_t size, u64 userFlags = 0) const { return general_allocate(size, 0, userFlags); }

    void *allocate_aligned(size_t size, alignment align, u64 userFlags = 0) const {
        return general_allocate(size, (size_t) align, userFlags);
    }

    static void *reallocate(void *ptr, size_t newSize, u64 userFlags = 0) {
        return general_reallocate(ptr, newSize, 0, userFlags);
    }

    static void *reallocate_aligned(void *ptr, size_t newSize, alignment align, u64 userFlags = 0) {
        return general_reallocate(ptr, newSize, (size_t) align, userFlags);
    }

    // This is static, because it doesn't depend on the allocator object you call it from.
    // Each pointer has a header which has information about the allocator it was allocated with.
    // Calling free on a null pointer doesn't do anything.
    static void free(void *ptr, u64 userFlags = 0) {
        if (!ptr) return;

        auto *header = (allocation_header *) ptr - 1;
        assert(header->Pointer == ptr &&
               "Calling free on a pointer that doesn't have a header (probably isn't dynamic memory or"
               "it wasn't allocated with an allocator from this library)");

        size_t size = header->Size + sizeof(allocation_header) + header->AlignmentPadding;

        void *memory = (char *) header - header->AlignmentPadding;
        header->Function(allocator_mode::FREE, header->Context, 0, memory, size, userFlags);
    }

    // Note that not all allocators must support this.
    // Returns true if the operation was completed successfully.
    bool free_all(u64 userFlags = 0) const {
        return Function(allocator_mode::FREE_ALL, Context, 0, 0, 0, userFlags) == null;
    }

    bool operator==(allocator other) const { return Function == other.Function && Context == other.Context; }
    bool operator!=(allocator other) const { return Function != other.Function || Context != other.Context; }

    operator bool() const { return Function; }

   private:
    // @Redundant
    // Currently, temporary allocator doesn't have any use for the header, since it doesn't support freeing.
    // Maybe make it optional for allocator implementations (to preserve memory)?
    //
    // ^ I'm not sure this is relevant. The header contains other useful information that doesn't have to do with
    // freeing.
    static void *encode_header(void *ptr, size_t size, u32 alignment, allocator_func_t function, void *context,
                               void *userData1, void *userData2) {
        size_t padding = calculate_padding_with_header(ptr, alignment, sizeof(allocation_header));
        size_t alignmentPadding = padding - sizeof(allocation_header);

        auto *result = (allocation_header *) ((char *) ptr + alignmentPadding);

        result->ID = AllocationCount;
        atomic_inc_64(&AllocationCount);

        result->Function = function;
        result->Context = context;
        result->Size = size;

        result->Alignment = (u32) alignment;
        result->AlignmentPadding = (u32) alignmentPadding;

        result->UserData1 = userData1;
        result->UserData2 = userData2;

        //
        // This is now safe since we handle alignment here (and not in general_(re)allocate).
        // Before I wrote the fix the program was crashing because I was using SIMD types,
        // which require to be aligned, on memory not 16-aligned.
        // I tried allocating with specified alignment but it wasn't taking into
        // account the size of the allocation header (accounting happened before bumping
        // the resulting pointer here).
        //
        // Since I had to redid how alignment was handled I decided to remove ALLOCATE_ALIGNED
        // and REALLOCATE_ALIGNED and drastically simplify allocator implementations.
        // What we do now is request a block of memory with a size
        // that was calculated with alignment in mind.
        //                                                                              - 5.04.2020
        result->Pointer = result + 1;

        auto *p = result->Pointer;
        assert((((uptr_t) p & ~((size_t) alignment - 1)) == (uptr_t) p) && "Pointer wasn't properly aligned.");
        return p;
    }

    // The main reason for having a combined function is to help debugging because the source of an allocation
    // can be one of the two functions (allocate() and allocate_aligned() below)
    void *general_allocate(size_t size, u32 alignment, u64 userFlags = 0) const {
        alignment = alignment < POINTER_SIZE ? POINTER_SIZE : alignment;
        assert(is_pow_of_2(alignment));

        size_t required = size + alignment + sizeof(allocation_header) + (sizeof(allocation_header) % alignment);
        void *result = Function(allocator_mode::ALLOCATE, Context, required, null, 0, userFlags);
        if (userFlags & DO_INIT_FLAG) {
            zero_memory((char *) result + sizeof(allocation_header), size);
        }
        return encode_header(result, size, alignment, Function, Context, null, null);
    }

    static void *general_reallocate(void *ptr, size_t newSize, u32 alignment, u64 userFlags = 0) {
        assert(ptr);

        alignment = alignment < POINTER_SIZE ? POINTER_SIZE : alignment;
        assert(is_pow_of_2(alignment));

        auto *header = (allocation_header *) ptr - 1;
        assert(header->Pointer == ptr &&
               "Calling reallocate on a pointer that doesn't have a header (probably it's not dynamic memory or"
               "it wasn't allocated with an allocator from this library)");
        assert(
            header->Alignment == alignment &&
            "Calling reallocate with different alignment. I may try to implement this but screw you - be consistent.");

        // The header stores the size of the requested allocation
        // (so the user code can look at the header and not be confused with garbage)
        size_t oldSize = header->Size + sizeof(allocation_header) + header->AlignmentPadding;
        size_t requiredSize = newSize + sizeof(allocation_header) + (sizeof(allocation_header) % alignment);

        void *oldUserData1 = header->UserData1;
        void *oldUserData2 = header->UserData2;

        void *oldMemory = (char *) header - header->AlignmentPadding;

        auto *allocFunc = header->Function;
        auto *allocContext = header->Context;

        void *result = allocFunc(allocator_mode::REALLOCATE, allocContext, requiredSize, oldMemory, oldSize, userFlags);
        if (userFlags & DO_INIT_FLAG) {
            zero_memory((char *) result + oldSize, newSize - header->Size);
        }
        return encode_header(result, newSize, alignment, allocFunc, allocContext, oldUserData1, oldUserData2);
    }
};

// Ensures that _alloc_ contains the most appropriate allocator. 
// If _alloc_ is null, we set it to the context's allocator.
// If _alloc_ points to an allocator that doesn't have a function, we set it to the context's allocator.
void get_an_allocator(allocator **alloc);

//
// Operators
//
void *operator new(size_t size);
void *operator new[](size_t size);

// If _alloc_ points to a null allocator, use a default one and store it in _alloc_
void *operator new(size_t size, allocator *alloc, u64 userFlags = 0) noexcept;
void *operator new[](size_t size, allocator *alloc, u64 userFlags = 0) noexcept;

void *operator new(size_t size, allocator alloc, u64 userFlags = 0) noexcept;
void *operator new[](size_t size, allocator alloc, u64 userFlags = 0) noexcept;

// If _alloc_ points to a null allocator, use a default one and store it in _alloc_
void *operator new(size_t size, alignment align, allocator *alloc = null, u64 userFlags = 0) noexcept;
void *operator new[](size_t size, alignment align, allocator *alloc = null, u64 userFlags = 0) noexcept;

void *operator new(size_t size, alignment align, allocator alloc, u64 userFlags = 0) noexcept;
void *operator new[](size_t size, alignment align, allocator alloc, u64 userFlags = 0) noexcept;

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;

//
// Default allocator:
//

// General purpose allocator (like malloc)
void *default_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64);

inline allocator Malloc = {default_allocator, null};

//
// Temporary allocator:
//

struct temporary_allocator_data {
    struct page {
        void *Storage = null;
        size_t Reserved = 0;
        size_t Used = 0;

        page *Next = null;
    };

    page Base;
    size_t TotalUsed = 0;
};

// This allocator works like an arena allocator.
// It's super fast because it basically bumps a pointer.
// It can be used globally to allocate memory that is not meant to last long
// (e.g. return value of a function that converts utf8 to utf16 to pass to a windows call)
//
// With this allocator you don't free individual allocations, but instead FREE_ALL the entire thing
// when you are use nobody uses the "temporary memory" anymore.
//
// It initializes itself the first time you allocate with it, the available space is always a multiple of 8 KiB,
// when we run out of space we allocate "overflow pages" and keep a list of them. The next time you FREE_ALL,
// these pages are merged and the default buffer is resized (new size is the combined size of all allocated buffers).
//
// Example use case: if you are programming a game and you need to calculate a mesh for a frame,
//     using this allocators means having the freedom of calling new/delete without performance implications.
//     At the end of the frame when the memory is no longer needed you FREE_ALL and start the next frame.
void *temporary_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64);

LSTD_END_NAMESPACE
