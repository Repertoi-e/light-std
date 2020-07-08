#pragma once

/// Defines the structure of allocators in this library.
/// Provides a default thread-safe global allocator and thread local temporary allocator.

#include <new>

#include "../internal/common.h"
#include "../thread.h"

LSTD_BEGIN_NAMESPACE

// By default we do some extra work when allocating to make it easier to catch memory related bugs.
// That work is measureable in performance so we don't want to do it in Dist configuration.
// If you want to disable it for debug/release as well, define FORCE_NO_DEBUG_MEMORY.
// You can read the comments in this file (around where DEBUG_MEMORY is mentioned)
// and see what extra stuff we do.

// XXX There is a bug in the debug memory routines that I can't be bothered to hunt down at the moment
// because I have a dead line for the project I need to finish.
#define FORCE_NO_DEBUG_MEMORY

#if defined DEBUG || defined RELEASE
#if !defined DEBUG_MEMORY && !defined FORCE_NO_DEBUG_MEMORY
#define DEBUG_MEMORY
#endif
#else
// Don't enable extra info when in Dist configuration unless predefined
#endif

//
// Allocators:
//

// Maximum size of an allocation we will attemp to request
#define MAX_ALLOCATION_REQUEST 0xFFFFFFFFFFFFFFE0  // Around 16384 PiB

enum class allocator_mode { ALLOCATE = 0, RESIZE, FREE, FREE_ALL };

// We need strong typing in order to allow user flags in operator new() overloads.
// Alignment is handled internally, so allocator implementations don't need to pay attention to it.
enum class alignment : u32;

// This is a user flag when allocating.
// When specified, the allocated memory is initialized to 0.
// This is handled internally, so allocator implementations don't need to pay attention to it.
constexpr u64 DO_INIT_0 = 1ull << 31;

// This specifies what the signature of each allocation function should look like.
//
// _mode_ is what we are doing currently, allocating, resizing,
//      freeing a block or freeing everything
//      (*implementing FREE_ALL is NOT a requirement, depends on the use case of the user)
// _context_ is used as a pointer to any data the allocator needs (state)
// _size_ is the size of the allocation
// _oldMemory_ is used when resizing or freeing a block
// _oldSize_ is the old size of memory block, used only when resizing
//
// the last u64 is reserved for user flags
//
// !!! When called with FREE_ALL, a return value of null means success!
//     To signify that the allocator doesn't support FREE_ALL (or the operation failed) return: (void*) -1.
//
// !!! When called with RESIZE, this doesn't mean "reallocate"!
//     Only valid return here is _oldMemory_ (memory was grown/shrank in place)
//     or null - memory can't be resized and needs to be moved.
//     In the second case we allocate a new block and copy the old data there.
using allocator_func_t =
    add_pointer_t<void *(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64)>;

#if defined DEBUG_MEMORY
//
// In DEBUG we put extra stuff to make bugs more obvious. These constants are literally taken from MSVC's debug CRT
// model. Like them, we use specific values for bytes outside the allocated range, for freed memory and for
// uninitialized memory.
//
// The following values are non-zero, constant, odd, large, and atypical.
// - Non-zero values help find bugs that assume zero-filled data
// - Constant values are good so that memory filling is deterministic (to help
//   make bugs reproducible). Of course, it is bad if the constant filling of
//   weird values masks a bug.
// - Mathematically odd numbers are good for finding bugs assuming a cleared
//   lower bit (e.g. properly aligned pointers to types other than char are not odd).
// - Large byte values are less typical and are useful for finding bad addresses.
// - Atypical values are good because they typically cause early detection in code.
// - For the case of the no-man's land and free blocks, if you store to any of
//   these locations, the memory integrity checker will detect it.
//

inline constexpr s64 NO_MANS_LAND_SIZE = 4;

// _NO_MANS_LAND_SIZE_ (4) extra bytes with this value before and after the allocation block
// which help detect reading out of range errors
inline constexpr u8 NO_MANS_LAND_FILL = 0xFD;

// When freeing we fill the block with this value (detects bugs when accessing memory that's freed)
inline constexpr u8 DEAD_LAND_FILL = 0xDD;

// When allocating a new block and DO_INIT_0 was not specified we fill it with this value
// (detects bugs when accessing memory before initializing it)
inline constexpr u8 CLEAN_LAND_FILL = 0xCD;
#endif

struct allocation_header {
#if defined DEBUG_MEMORY
    allocation_header *DEBUG_Next, *DEBUG_Previous;
#endif

    // Useful for debugging (better than file and line, in my opinion, because you can set a breakpoint with the ID
    // in allocate() below). Every allocation has an unique ID equal to the ID of the previous allocation + 1.
    // This is useful for debugging bugs related to allocations because (assuming your program isn't multithreaded)
    // each time you run your program the ID of each allocation is easily reproducible
    // (assuming no randomness from the user side).
    u32 ID;

    // This is used to keep track of how many times this block has been reallocated.
    // When reallocate is called we check if the block can be directly resized in place (using allocation_mode::RESIZE).
    // If not, we allocate a new block and transfer all the information to it there.
    // In both cases the ID above stays the same and this "local" ID is incremented.
    u32 RID;

    // The allocator used when allocating the memory
    allocator_func_t Function;
    void *Context;

    // The size of the allocation (NOT including the size of the header and padding)
    s64 Size;

    void *Owner;  // Points to the object that owns the block. Manage this with functions from "owner_pointers.h".

#if defined DEBUG_MEMORY
    // The pointer allocated (this is used to verify if the header exists at all).
    // I mean... I guess it's not perfectly reproducible but good enough to catch bugs in dev...
    void *DEBUG_Pointer;
#endif

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
    u16 Alignment;         // We allow a maximum of 65536 bit (8192 byte) alignment
    u16 AlignmentPadding;  // Offset from the block that needs to be there in order for the result to be aligned

    void *UserData;

#if defined DEBUG_MEMORY
    // There may be padding after this (because we have modified this struct before but forgot)
    // but it's ok, we just need at least 4 bytes free. We always set the last 4 bytes of the header.
    char DEBUG_NoMansLand[NO_MANS_LAND_SIZE];
#endif

    // This header is followed by:
    // char Data[Size];
    // char NoMansLand[NO_MANS_LAND_SIZE]; (only in DEBUG)
};

// Calculates the required padding in bytes which needs to be added to _ptr_ in order to be aligned
inline u16 calculate_padding_for_pointer(void *ptr, s32 alignment) {
    assert(alignment > 0 && is_pow_of_2(alignment));
    return (u16)((((u64) ptr + (alignment - 1)) & -alignment) - (u64) ptr);
}

// Like calculate_padding_for_pointer but padding must be at least the header size
inline u16 calculate_padding_for_pointer_with_header(void *ptr, s32 alignment, u32 headerSize) {
    u16 padding = calculate_padding_for_pointer(ptr, alignment);
    if (padding < headerSize) {
        headerSize -= padding;
        if (headerSize % alignment) {
            padding += alignment * (1 + (headerSize / alignment));
        } else {
            padding += alignment * (headerSize / alignment);
        }
    }
    return padding;
}

struct allocator {
    allocator_func_t Function = null;
    void *Context = null;

    inline static s64 AllocationCount = 0;

#if defined DEBUG_MEMORY
    // We keep a linked list of all allocations. You can use this to visualize them.
    inline static allocation_header *DEBUG_Head = null;

    inline static thread::mutex DEBUG_Mutex;

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   DEBUG_MEMORY
    // Use _DEBUG_unlink_header_ in your allocator implementation to make sure we don't corrupt the heap
    // (e.g. by freeing the entire allocator, but the headers still being in the linked list).
    // An example on how to use this properly in FREE_ALL is in temporary_allocator.cpp.
    // Note that it's not required for your allocator to implement FREE_ALL at all.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Removes a header from the list (thread-safe)
    static void DEBUG_unlink_header(allocation_header *header);

    // This adds the header to the front - making it the new DEBUG_Head (thread-safe)
    static void DEBUG_add_header(allocation_header *header);

    // Replaces _oldHeader_ with _newHeader_ in the list. (thread-safe)
    static void DEBUG_swap_header(allocation_header *oldHeader, allocation_header *newHeader);
#endif

    void *allocate(s64 size, u64 userFlags = 0) const { return general_allocate(size, 0, userFlags); }

    void *allocate_aligned(s64 size, alignment align, u64 userFlags = 0) const {
        return general_allocate(size, (s64) align, userFlags);
    }

    // Resizes the memory block. If it can't do that it allocates a new one and copies the old memory.
    // This function takes into account aligned blocks.
    // Calling reallocate with newSize == 0 frees the memory.
    // Calling reallocate on a null pointer doesn't do anything.
    // _userFlags_ gets passed to allocate/free if this function calls them
    static void *reallocate(void *ptr, s64 newSize, u64 userFlags = 0) {
        if (!ptr) return null;
        if (newSize == 0) free(ptr, userFlags);
        return general_reallocate(ptr, newSize, userFlags);
    }

    // This is static, because it doesn't depend on the allocator object you call it from.
    // Each pointer has a header which has information about the allocator it was allocated with.
    // Calling free on a null pointer doesn't do anything.
    static void free(void *ptr, u64 userFlags = 0);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //   DEBUG_MEMORY
    //     See the comment in DEBUG_unlink_header.
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // Note: Not all allocators must support this (check your use case!)
    void free_all(u64 userFlags = 0) const;

    bool operator==(allocator other) const { return Function == other.Function && Context == other.Context; }
    bool operator!=(allocator other) const { return Function != other.Function || Context != other.Context; }

    operator bool() const { return Function; }

    // Verifies the integrity of headers in all allocations (only if DEBUG_MEMORY is on).
    // You can call this anytime even outside the class.
    static void verify_heap();

    // Verifies the integrity of a single header (only if DEBUG_MEMORY is on).
    // You can call this anytime even outside the class.
    static void verify_header(allocation_header *header);

   private:
    static void *encode_header(void *p, s64 userSize, u32 align, allocator_func_t f, void *c, bool initToZero);

    // The main reason for having a combined function is to help debugging because the source of an allocation
    // can be one of the two functions (allocate() and allocate_aligned() above)
    void *general_allocate(s64 userSize, u32 align, u64 userFlags = 0) const;
    static void *general_reallocate(void *ptr, s64 newSize, u64 userFlags = 0);
};

//
// Default allocator:
//

// General purpose allocator (like malloc)
void *default_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64);

inline allocator Malloc = {default_allocator, null};

//
// Temporary allocator:
//

struct temporary_allocator_data {
    struct page {
        void *Storage = null;
        s64 Reserved = 0;
        s64 Used = 0;

        page *Next = null;
    };

    page Base;
    s64 TotalUsed = 0;
};

// This allocator works like an arena allocator.
// It's super fast because it basically bumps a pointer.
// It can be used globally to allocate memory that is not meant to last long
// (e.g. return value of a function that converts utf8 to utf16 to pass to a windows call)
//
// With this allocator you don't free individual allocations, but instead free the entire thing (with FREE_ALL)
// when you are sure nobody uses memory the "temporary memory" anymore.
//
// It initializes itself the first time you allocate with it, the available space is always a multiple of 8 KiB,
// when we run out of space we allocate "overflow pages" and keep a list of them. The next time you FREE_ALL,
// these pages are merged and the default buffer is resized (new size is the combined size of all allocated buffers).
//
// Example use case: if you are programming a game and you need to calculate some stuff for a mesh for a given frame,
//     using this allocator means having the freedom of calling new/delete without performance implications.
//     At the end of the frame when the memory is no longer used you FREE_ALL and start the next frame.
void *temporary_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64);

LSTD_END_NAMESPACE

//
// Operators
//
void *operator new(size_t size);
void *operator new[](size_t size);

void *operator new(size_t size, allocator alloc, u64 userFlags = 0) noexcept;
void *operator new[](size_t size, allocator alloc, u64 userFlags = 0) noexcept;

void *operator new(size_t size, alignment align, allocator alloc, u64 userFlags = 0) noexcept;
void *operator new[](size_t size, alignment align, allocator alloc, u64 userFlags = 0) noexcept;

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;

void operator delete(void *ptr, allocator alloc, u64 userFlags) noexcept;
void operator delete[](void *ptr, allocator alloc, u64 userFlags) noexcept;

void operator delete(void *ptr, alignment align, allocator alloc, u64 userFlags) noexcept;
void operator delete[](void *ptr, alignment align, allocator alloc, u64 userFlags) noexcept;
