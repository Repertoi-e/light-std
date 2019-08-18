#pragma once

/// Defines the structure of allocators in this library.
/// Provides a default thread-safe global allocator and thread local temporary allocator.

#include "../common.h"
#include "../intrin.h"

#include <new>

LSTD_BEGIN_NAMESPACE

//
// Allocators:
//

enum class allocator_mode { ALLOCATE = 0, ALIGNED_ALLOCATE, REALLOCATE, ALIGNED_REALLOCATE, FREE, FREE_ALL };

// We need strong typing in order to allow user flags in operator new() overloads
enum class alignment : size_t;

// This is a user flag when allocating.
// When specified, the allocated memory is initialized.
// This is handled internally, so allocator implementations don't need to pay attention to it.
constexpr u64 DO_INIT_FLAG = BIT(31);

//
// OS allocator:
//
void *os_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, alignment align,
                   u64);

// This specifies what the signature of each allocation function should look like.
//
// _mode_ is what we are doing currently, allocating, resizing,
//      freeing a block or freeing everything (*not every allocator should support freeing everything)
// _context_ is used as a pointer to any data the allocator needs (e.g. state)
// _size_ is the size of the allocation
// _oldMemory_ is used when resizing
// _oldSize_ is the old size of memory block, used only when resizing
// _align_ is used when calling with ALIGNED_ALLOCATE or ALIGNED_REALLOCATE
//
// the last u64 is reserved for user flags
//
// !!! When called with FREE_ALL, a return value of null means success!
//     To signify that the allocator doesn't support FREE_ALL (or the operation failed) return: (void*) -1
using allocator_func_t = add_pointer_t<void *(allocator_mode mode, void *context, size_t size, void *oldMemory,
                                              size_t oldSize, alignment align, u64)>;

struct allocation_header {
    // Useful for debugging (better than file and line, in my opinion, because you can set a breakpoint with the ID
    // in allocate() below). Every allocation has an unique ID equal to the ID of the previous + 1 (starting at 0)
    // This is useful for debugging bugs related to allocations because (assuming your program isn't multithreaded)
    // each time you run your program the ID of each allocation is easily reproducible (assuming no randomness).
    size_t ID;

    // The allocator used when allocating the memory
    allocator_func_t Function;
    void *Context;

    // The size of the allocation (NOT including the size of the header)
    size_t Size;

    // The pointer allocated (this is used to test if the header exists at all)
    void *Pointer;
};

struct allocator {
    allocator_func_t Function = null;
    void *Context = null;

    inline static size_t AllocationCount = 0;

    void *allocate(size_t size, u64 userFlags = 0) const {
        return general_allocate(size, false, alignment(0), userFlags);
    }

    void *allocate_aligned(size_t size, alignment align, u64 userFlags = 0) const {
        return general_allocate(size, true, align, userFlags);
    }

    static void *reallocate(void *ptr, size_t newSize, u64 userFlags = 0) {
        return general_reallocate(ptr, newSize, false, alignment(0), userFlags);
    }

    static void *reallocate_aligned(void *ptr, size_t newSize, alignment align, u64 userFlags = 0) {
        return general_reallocate(ptr, newSize, true, align, userFlags);
    }

    // This is static, because it doesn't depend on the allocator object you call it from.
    // Each pointer has a header which has information about the allocator it was allocated with
    // Calling free on a null pointer doesn't do anything
    static void free(void *ptr, u64 userFlags = 0) {
        if (!ptr) return;

        auto *header = (allocation_header *) ptr - 1;
        assert(header->Pointer == ptr &&
               "Calling free on a pointer that doesn't have a header (probably it isn't dynamic memory or"
               "it wasn't allocated with an allocator from this library)");

        header->Function(allocator_mode::FREE, header->Context, 0, header, header->Size + sizeof(allocation_header),
                         alignment(0), userFlags);
    }

    // Note that not all allocators must support this.
    // Returns true if the operation was completed successfully.
    bool free_all(u64 userFlags = 0) const {
        return Function(allocator_mode::FREE_ALL, Context, 0, 0, 0, alignment(0), userFlags) == null;
    }

    bool operator==(allocator other) const { return Function == other.Function && Context == other.Context; }
    bool operator!=(allocator other) const { return Function != other.Function || Context != other.Context; }

    operator bool() const { return Function; }

   private:
    // @Redundant
    // Currently, temporary allocator doesn't have any use for the header, since it doesn't support freeing.
    // Maybe make it optional for allocator implementations?
    static void *encode_header(void *ptr, size_t size, allocator_func_t function, void *context) {
        auto *result = (allocation_header *) ptr;

        result->ID = AllocationCount;
        ATOMIC_INC_64((s64 *) &AllocationCount);

        result->Function = function;
        result->Context = context;
        result->Size = size;
        result->Pointer = result + 1;
        return (void *) result->Pointer;
    }

    // The main reason for having a combined function is to help debugging because the source of an allocation
    // can be one of the two functions (allocate() and allocate_aligned() below)
    void *general_allocate(size_t size, bool aligned, alignment align, u64 userFlags = 0) const {
        void *result;
        if (!aligned) {
            result = Function(allocator_mode::ALLOCATE, Context, size + sizeof(allocation_header), null, 0,
                              alignment(0), userFlags);
        } else {
            assert((size_t) align > 0 && IS_POW_OF_2((size_t) align));
            result = Function(allocator_mode::ALIGNED_ALLOCATE, Context, size + sizeof(allocation_header), null, 0,
                              align, userFlags);
            assert((((uptr_t) result & ~((size_t) align - 1)) == (uptr_t) result) &&
                   "Pointer wasn't properly aligned.");
        }
        if (userFlags & DO_INIT_FLAG) {
            zero_memory((char *) result + sizeof(allocation_header), size);
        }
        return encode_header(result, size, Function, Context);
    }

    static void *general_reallocate(void *ptr, size_t newSize, bool aligned, alignment align, u64 userFlags = 0) {
        auto *header = (allocation_header *) ptr - 1;
        assert(header->Pointer == ptr &&
               "Calling reallocate on a pointer that doesn't have a header (probably it isn't dynamic memory or"
               "it wasn't allocated with an allocator from this library)");

        auto oldSize = header->Size;  // The header stores the size without sizeof(allocation_header)
        if (oldSize > newSize) return ptr;

        auto *allocFunc = header->Function;
        auto *allocContext = header->Context;

        void *result;
        if (!aligned) {
            result = allocFunc(allocator_mode::REALLOCATE, allocContext, newSize + sizeof(allocation_header), header,
                               oldSize + sizeof(allocation_header), alignment(0), userFlags);
        } else {
            assert((size_t) align > 0 && IS_POW_OF_2((size_t) align));
            result = allocFunc(allocator_mode::ALIGNED_REALLOCATE, allocContext, newSize + sizeof(allocation_header),
                               header, oldSize + sizeof(allocation_header), align, userFlags);
            assert((((uptr_t) result & ~((size_t) align - 1)) == (uptr_t) result) &&
                   "Pointer wasn't properly aligned.");
        }
        if (userFlags & DO_INIT_FLAG) {
            zero_memory((char *) result + sizeof(allocation_header) + oldSize, newSize - oldSize);
        }
        return encode_header(result, newSize, allocFunc, allocContext);
    }
};

// Returns an aligned pointer
// This is used in allocator implementations for supporting aligned allocations
// If alignment is 4 bytes on 64 bit system, it get set to 8 bytes
inline void *get_aligned_pointer(void *ptr, size_t alignment) {
    assert(alignment > 0 && IS_POW_OF_2(alignment));

    alignment = alignment < POINTER_SIZE ? POINTER_SIZE : alignment;
    return (void *) (((uptr_t) ptr + alignment - 1) & ~(alignment - 1));
}

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
void *default_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                        alignment align, u64);

inline allocator Malloc = {default_allocator, null};

//
// Temporary allocator:
//

struct temporary_allocator_data {
    struct overflow_page {
        void *Storage = null;
        size_t Reserved = 0;
        size_t Used = 0;

        overflow_page *Next = null;
    };

    void *Storage = null;
    size_t Reserved = 0;
    size_t Used = 0;

    // Including overflow pages
    size_t OverallUsed = 0;

    overflow_page *OverflowPageList;
};

// This allocator works like an arena allocator.
// It's super fast because it basically bumps a pointer.
// It can be used globally to allocate memory that is not meant to last long
// (e.g. return value of a function that converts utf8 to utf16 to pass to a windows call)
// With this allocator you don't free individual allocations, but instead FREE_ALL the entire thing
// when you are use nobody uses the "temporary memory".
// Example use case: if you are programming a game and you need to calculate a mesh for a frame,
//     using this allocators means having the freedom of calling new/delete without performance implications.
//     At the end of the frame when the memory is no longer needed you FREE_ALL and start the next frame.
void *temporary_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                          alignment align, u64);

LSTD_END_NAMESPACE