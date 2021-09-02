#pragma once

/// Defines the structure of allocators in this library.
/// Provides a default thread-safe global allocator and thread local temporary allocator.

#include "../common/common.h"
#include "../thread.h"
#include "vendor/tlsf/tlsf.h"

LSTD_BEGIN_NAMESPACE

// By default we do some extra work when allocating to make it easier to catch memory related bugs.
// That work is measureable in performance so we don't want to do it in Dist configuration.
// If you want to disable it for debug/release as well, define FORCE_NO_DEBUG_MEMORY.
// You can read the comments in this file (around where DEBUG_MEMORY is mentioned)
// and see what extra safety stuff we do.

#if not defined NDEBUG
#if !defined DEBUG_MEMORY && !defined FORCE_NO_DEBUG_MEMORY
#define DEBUG_MEMORY 1
#endif
#else
// Don't enable extra info when in Dist configuration unless predefined
// Note: "#define DEBUG_MEMORY 0" is WRONG. We check with "#if defined DEBUG_MEMORY",
// so that means in order to disable DEBUG_MEMORY you mustn't set DEBUG_MEMORY to anything at all.
#endif

//
// Allocators:
//

// Maximum size of an allocation we will attemp to request
#define MAX_ALLOCATION_REQUEST 0xFFFFFFFFFFFFFFE0  // Around 16384 PiB

enum class allocator_mode {
    ADD_POOL = 0,
    REMOVE_POOL,
    ALLOCATE,
    RESIZE,
    FREE,
    FREE_ALL
};

// This is an option when allocating.
// Allocations marked explicitly as leaks don't get reported with DEBUG_memory->report_leaks().
// This is handled internally when passed, so allocator implementations needn't pay attention to it.
constexpr u64 LEAK = 1ull << 63;

// This specifies what the signature of each allocation function should look like.
//
// _mode_ is what we are doing currently: adding a pool, allocating, resizing, freeing a block or freeing everything
//      Note: * Implementing FREE_ALL is NOT a requirement, some allocators by design can't implement this
//            * Allocators shouldn't request memory from the OS in any way. They should use the pools added by the user
//              of the allocator. See :BigPhilosophyTime: near the bottom of this file for the reasoning behind this.
//              So... before using an allocator, call os_allocate_block() to get a pool of appropriate size.
//              And call the allocator with ADD_POOL (the wrapper you should use is allocator_add_pool()).
//
// _context_ is used as a pointer to any data the allocator needs (state)
// _size_ is the size of the allocation
// _oldMemory_ is used when resizing or freeing a block
// _oldSize_ is the old size of memory block, used only when resizing
//
// The last pointer to u64 is reserved for options. The allocator implementation decides how to treat those.
// We just provide a way to pass those. Note: The LEAK option means that bit number 64 is reserved for special use.
// That means that the maximum integer you have to work with is the bottom 63 bits of the options.
// I expect people to this as a bit field anyway.
//
//
// !!! When called with FREE_ALL, a return value of null means success!
//     To signify that the allocator doesn't support FREE_ALL (or the operation failed) return: (void*) -1.
//
// !!! When called with RESIZE, this doesn't mean "reallocate"!
//     Only valid return here is _oldMemory_ (memory was grown/shrank in place)
//     or null - memory can't be resized and needs to be moved.
//     In the second case we allocate a new block and copy the old data there (in general_reallocate).
//
// !!! Alignment is handled internally. Allocator implementations needn't pay attention to it.
//     When an aligned allocation is being made, we send a request at least _alignment_ bytes larger,
//     so when the allocator function returns an unaligned pointer we can freely bump it.
//     The information about the alignment is saved in the header, that's how we know what
//     the old pointer was when freeing or reallocating.
//
// We do this so custom allocator implementations can be kept as simple as possible.
// For examples see how we implement the arena allocator, pool allocator, tlsf (two-level segmented fit), etc.
using allocator_func_t = void *(*)(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

struct allocator {
    allocator_func_t Function;
    void *Context;

    allocator(allocator_func_t function = null, void *context = null)
        : Function(function),
          Context(context) {
    }

    bool operator==(allocator other) const { return Function == other.Function && Context == other.Context; }
    bool operator!=(allocator other) const { return Function != other.Function || Context != other.Context; }

    operator bool() const { return Function; }
};

#if defined DEBUG_MEMORY
//
// In DEBUG we put extra stuff to make bugs more obvious. These constants are like the ones MSVC's debug CRT model uses.
// Like them, we use specific values for bytes outside the allocated range, for freed memory and for uninitialized memory.
//
// The values of the constants and the following comment is taken from MSVC:
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

// When allocating a new block we fill it with this value
// (detects bugs when accessing memory before initializing it)
inline constexpr u8 CLEAN_LAND_FILL = 0xCD;
#endif

//
// Each allocation contains this header before the returned pointer.
// The returned pointer is guaranteed to be aligned to the specified alignment,
// we do that by padding this structure. Info about that is saved in the header itself.
// Right now this uses 32 bytes when DEBUG_MEMORY is not defined, 96 bytes when storing debug info.
//
// @TODO: This is way too bug of an overhead. We CAN make this smaller!
//        We should be careful in aranging this header. We can split allocations into medium/large,
//        etc. and use a smaller header for medium allocations to reduce overhead (like https://nothings.org/stb/stb_malloc.h).
//
struct allocation_header {
#if defined DEBUG_MEMORY
    // We store a linked list of all allocations made, in order to look for leaks and check integrity of all allocations at once.
    // @TODO: In the future I plan to build a very sophisticated tool which shows all allocations in a program visually,
    //        we can even save a call stack for each allocation (not in the header of course :D), so it's easier to debug where
    //        an allocation came from (rather than using the ID and a breakpoint which is the current way of doing this).
    //
    allocation_header *DEBUG_Next, *DEBUG_Previous;

    // Useful for debugging (you can set a breakpoint with the ID in general_allocate() in allocator.cpp).
    // Every allocation has an unique ID == to the ID of the previous allocation + 1.
    // This is useful for debugging bugs related to allocations because (assuming your program isn't multithreaded)
    // each time you run your program the ID of each allocation is easily reproducible (assuming no randomness from the user side).
    //
    // In the future we will have a more sophisticated way to debug allocations. See comment above.
    s64 ID;

    // This ID is used to keep track of how many times this block has been reallocated.
    // When reallocate_array() is called we check if the block can be directly resized in place (using
    // allocation_mode::RESIZE). If not, we allocate a new block and transfer all the information to it there. In both
    // cases the ID above stays the same and this local ID is incremented. This always starts at 0.
    //
    // This helps debug memory allocations that are reallocated often (strings/arrays, etc.)
    s64 RID;

    // We mark the source of the allocation if such information was provided.
    // On reallocation we overwrite these with the source provided then.
    const utf8 *FileName;
    s64 FileLine;
#endif

    // The allocator used when allocating the memory. We read this when resizing/freeing to call the right allocator procedure.
    // By design, we can't get rid of this.
    allocator Alloc;

    // The size of the allocation (NOT including the size of the header and padding).
    // By design, we can't get rid of this (but we can make this smaller for medium sized allocations by packing a different header).
    s64 Size;

#if defined DEBUG_MEMORY
    // This is another guard to check that the header is valid.
    // This points to "(allocation_header *) p + 1" (the pointer we return after the allocation).
    void *DEBUG_Pointer;
#endif

    // The padding (in bytes) which was added to the pointer after allocating to make sure the resulting pointer is
    // aligned. The structure of an allocation is basically this:
    //
    // User requests allocation of _size_. The underlying allocator is called with
    //     _size_ + sizeof(allocation_header) + (sizeof(allocation_header) % alignment)
    //
    // The result:
    //   ...[..Alignment padding..][............Header..................]............
    //      ^ The pointer returned by the allocator implementation       ^ The resulting pointer (aligned)
    //
    u16 Alignment;        // We allow a maximum of 65535 bit (8191 byte) alignment
    u16 AlignmentPadding; // Offset from the block that needs to be there in order for the result to be aligned

#if defined DEBUG_MEMORY
    // When allocating we can mark the next allocation as a leak.
    // That means that it's irrelevant if we don't free it before the end of the program (since the OS claims back the memory anyway).
    // When DEBUG_memory->CheckForLeaksAtTermination is set to true we log a list of unfreed allocations at the end of the program.
    // Headers with this bool set to true get skipped.
    bool MarkedAsLeak;

    // This is used to detect buffer underruns.
    // There may be padding after this member, but we treat this region as "(allocation_header *) p + 1 - 4 bytes".
    // This doesn't matter since we just need AT LEAST 4 bytes free.
    char DEBUG_NoMansLand[NO_MANS_LAND_SIZE];

    // After the returned memory block we follow by NO_MANS_LAND_SIZE bytes of no man's land again.
    // We use that to detect buffer overruns.
#endif
};

// 32, 96
// constexpr s64 a = sizeof(allocation_header);

// Calculates the required padding in bytes which needs to be added to _ptr_ in order to be aligned
inline u16 calculate_padding_for_pointer(void *ptr, s32 alignment) {
    assert(alignment > 0 && is_pow_of_2(alignment));
    return (u16) (((u64) ptr + (alignment - 1) & -alignment) - (u64) ptr);
}

// Like calculate_padding_for_pointer but padding must be at least the header size
inline u16 calculate_padding_for_pointer_with_header(void *ptr, s32 alignment, u32 headerSize) {
    u16 padding = calculate_padding_for_pointer(ptr, alignment);
    if (padding < headerSize) {
        headerSize -= padding;
        if (headerSize % alignment) {
            padding += alignment * (1 + headerSize / alignment);
        } else {
            padding += alignment * (headerSize / alignment);
        }
    }
    return padding;
}

// #if'd so programs don't compile when debug info shouldn't be used.
#if defined DEBUG_MEMORY
struct debug_memory {
    s64 AllocationCount = 0;

    // We keep a linked list of all allocations. You can use this list to visualize them.
    // _Head_ is the last allocation done.
    allocation_header *Head = null;

    // Currently this mutex should be released in the OS implementations (e.g. windows_common.cpp).
    // We need to lock it before modifying the linked list for example.
    //
    // @TODO: @Speed: Lock-free linked list!
    thread::mutex Mutex;

    // After every allocation we check the heap for corruption.
    // The problem is that this involves iterating over a (possibly) large linked list of every allocation made.
    // We use the frequency variable below to specify how often we perform that expensive operation.
    // By default we check the heap every 255 allocations, but if a problem is found you may want to decrease
    // this to 1 so you catch the corruption at just the right time.
    u8 MemoryVerifyHeapFrequency = 255;

    // Set this to true to print a list of unfreed memory blocks when the library uninitializes.
    // Yes, the OS claims back all the memory the program has allocated anyway, and we are not promoting C++ style RAII
    // which make EVEN program termination slow, we are just providing this information to the programmer because they might
    // want to debug crashes/bugs related to memory. (I had to debug a bug with loading/unloading DLLs during runtime).
    bool CheckForLeaksAtTermination = false;

    void unlink_header(allocation_header *header);                                // Removes a header from the list
    void add_header(allocation_header *header);                                   // This adds the header to the front - making it the new head
    void swap_header(allocation_header *oldHeader, allocation_header *newHeader); // Replaces _oldHeader_ with _newHeader_ in the list

    // Assuming that the heap is not corrupted, this reports any unfreed allocations.
    // Yes, the OS claims back all the memory the program has allocated anyway, and we are not promoting C++ style RAII
    // which make EVEN program termination slow, we are just providing this information to the programmer because they might
    // want to debug crashes/bugs related to memory. (I had to debug a bug with loading/unloading DLLs during runtime).
    void report_leaks();

    // Verifies the integrity of headers in all allocations (only if DEBUG_MEMORY is on).
    //
    // We call this function when a new allocation is made.
    // The problem is that it involves iterating over a linked list of every allocation made.
    // We use the frequency variable in the _Context_ to specify how often we perform that expensive operation.
    // By default we check the heap every 255 allocations, but if a problem is found you may want to decrease it to 1 so
    // your program runs way slower but you catch the corruption at just the right time.
    void maybe_verify_heap();

    // Verifies the integrity of a single header (only if DEBUG_MEMORY is on).
    void verify_header(allocation_header *header);
};

inline debug_memory *DEBUG_memory;
#endif

void *general_allocate(allocator alloc, s64 userSize, u32 alignment, u64 options = 0, source_location loc = {});

// Each pointer has a header which has information about the allocator it was allocated with.
void *general_reallocate(void *ptr, s64 newSize, u64 options = 0, source_location loc = {});

// Each pointer has a header which has information about the allocator it was allocated with.
// Calling free on a null pointer doesn't do anything.
//
// @TODO: source_location? Idea for the future: Turn on a switch to remember every freed allocation and have a way to
// display all that information in a visual way. This will help the programmer see what the program is doing with memory exactly.
void general_free(void *ptr, u64 options = 0);

// Note: Not all allocators must support this.
void free_all(allocator alloc, u64 options = 0);

//
// Allocators don't allocate with os_allocate_block() but instead should require the programmer to have already passed
// a block of memory (a pool) which they divide into smaller allocations. The pool may be allocated by another allocator
// or os_allocate_block(). There is a reason we are not abstracting this away and we want to be explicit about this.
// See comment beginning with :BigPhilosophyTime: a bit further down in this file.
//
inline void allocator_add_pool(allocator alloc, void *block, s64 size, u64 options = 0) {
    auto *pool = alloc.Function(allocator_mode::ADD_POOL, alloc.Context, size, block, 0, options);
    assert(pool == block && "Add pool failed");
}

// Should trip an assert if the block doesn't exist in the allocator's pool
inline void allocator_remove_pool(allocator alloc, void *block, u64 options = 0) {
    auto *result = alloc.Function(allocator_mode::REMOVE_POOL, alloc.Context, 0, block, 0, options);
    assert(result == block && "Remove pool failed");
}

// When calling allocator_add_page() we store this piece of metadata in the beginning of the block
// (in order to avoid allocating a separate linked list). So each block you add to an allocator should be larger than this structure.
struct allocator_pool {
    allocator_pool *Next;
    s64 Size;
    s64 Used;
};

// These are helper routines which implement the basic logic of a linked list of pools.
// See example usage in arena_allocator.cpp
//
// Don't forget that you can do something entirely different in your custom allocator!
// We just provide this as a good, basic, robust, working solution so you don't have to write this manually every time.

// This stores an initialized pool in the beginning of the block
inline bool allocator_pool_initialize(void *block, s64 size) {
    if (size <= sizeof(allocator_pool)) {
        assert(false && "Pool is too small");
        return false;
    }

    auto *pool = (allocator_pool *) block;
    pool->Next = null;
    pool->Size = size - sizeof(allocator_pool);
    pool->Used = 0;
    return true;
}

inline void allocator_pool_add_to_linked_list(allocator_pool **base, allocator_pool *pool) {
    // @Cleanup Make macros for linked lists (don't make a data structure, that's over-abstraction).
    // Then we can get rid of this function as well.
    if (!*base) {
        *base = pool;
    } else {
        auto *it = *base;
        while (it->Next) it = it->Next;
        it->Next = pool;
    }
}

inline void *allocator_pool_remove_from_linked_list(allocator_pool **base, allocator_pool *pool) {
    // @Cleanup Make macros for linked lists (don't make a data structure, that's over-abstraction).
    // Then we can get rid of this function as well.

    if (!*base) {
        assert(false && "No pools have been added yet");
        return null;
    }

    allocator_pool *it = *base, *prev = null;
    while (it != pool && it->Next) {
        prev = it;
        it   = it->Next;
    }

    if (it != pool) {
        assert(false && "Pool with this address was not found in this allocator's pool list");
        return null;
    }

    if (prev) {
        prev->Next = it->Next;
    } else {
        *base = (*base)->Next;
    }

    return it;
}

//
// Here is a list of allocators we provide:
//

//
// :BigPhilosophyTime: Read here.
//
// We don't provide a traditional general purpose "malloc" function.
//
// I think the programmer should be very aware of the memory the program is using.
// That's why we require allocators to be initted with an initial (or several) large blocks (pools)
// from which smaller blocks are used for allocations. This forces you to think and write
// faster software, because modern comptures suffer a lot from improper memory layout (cache misses, etc.)
// Forcing you to think about your allocator algorithm, as well as the pool management, instead
// of providing a one fit all solution, results (hypothesis) in better software overall.
//
// C++ is a low-level language (was high-level in old days when ASM was low-level, but now we consider it low-level).
// Usually modern high-level languages put much of the memory management behind walls of abstraction.
// Somewhere in all that progress of abstraction we began writing very very slow software (take a look at a
// complicated modern piece of software (games are an exception, but games/engines are high quality software,
// game programmers know all this stuff) - it's SLOW. Hardware has gotten blazingly fast, but software has deteriorated.
// Modern CPUs can make a million billion calculations per second, but reading even ONE byte from RAM can take hundreds of clock cycles
// (if the memory is not in the cache). You MUST think about the cache if you want to write fast software.
//
// Once you start thinking about the cache you start programming in a data oriented way.
// You don't try to model the real world in the program, but instead structure the program in the way that
// the computer will work with the data. Data that is processes together should be close together in memory.
//
// And that's why we should remove some of the abstraction. We should think about the computer's memory.
// We should write fast software. We can slow down global warming by not wasting CPU clock cycles.
//
// Note:
// Of course, writing abstractions which allows rapid programming is the rational thing to do. After all we can do so much more stuff
// instead of micro-optimizing malloc. But being a bit too careless results in the modern mess of software that wastes most of CPU time
// doing nothing, because people decided to abstract stuff.
//
// I am writing this in 2021 - in the midst of the COVID-19 crisis. We have been using online video conferencing software for school
// and work for a year now. The worst example is MS Teams. Clicking a button takes a good second to open a chat. It lags constantly,
// bugs everywhere. No. Your computer is not slow. Your computer is a super-computer compared to what people had 30-40 years ago.
//
// Not only you waste electricity by being a careless programmer, you also waste USER'S TIME!
// If your program is used by millions of PC, 1 second to click a SIMPLE BUTTON quickly becomes hours and then days.
//
// This library is a call out and an attempt to bring people's attention to these problems.
// It also attemps to provide an expressive and high-level *way of programming*, while also necessarily being low-level in some cases.
//
//
//
// So...
// Generally malloc implementations do the following:
// - Have seperate heaps for different sized allocations
// - Call OS functions for very large allocations
// - Different algorithms for allocation, e.g. stb_malloc implements the TLSF algorithm for O(1) allocation
//
// Here we provide a wrapper around the TLSF algorithm. Here is how you should use it:
// Allocate a large block with the OS allocator (that's usually how everything starts).
// Call allocator_add_pool on the TLSF
//

struct tlsf_allocator_data {
    tlsf_t State = null; // We use a vendor library that implements the algorithm.
};

// Two-Level Segregated Fit memory allocator implementation. Wrapper around tlsf.h/cpp (in vendor folder),
// written by Matthew Conte (matt@baisoku.org). Released under the BSD license.
//
// * O(1) cost for alloc, free, resize
// * Extremely low overhead per allocation (4 bytes)
// * Low overhead per TLSF management of pools (~3kB)
// * Low fragmentation
//
void *tlsf_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

//
// General purpose allocator.
//
// void *default_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 *);
// inline allocator Malloc;
//
// We don't provide this, as explained above.
//

struct arena_allocator_data {
    allocator_pool *Base = null; // Linked list of pools, see arena_allocator.cpp for example usage of the helper routines we provide to manage this.
    // Of course, you can implement an entirely different way to store pools in your custom allocator!
    s64 PoolsCount = 0;
    s64 TotalUsed  = 0;
};

//
// Arena allocator.
//
// This type of allocator super fast because it basically bumps a pointer.
// With this allocator you don't free individual allocations, but instead free
// the entire thing (with FREE_ALL) when you are sure nobody uses the memory anymore.
// Note that free_all doesn't free the pools, but instead sets their pointers to 0.
//
// The arena allocator doesn't handle overflows (when no pool has enough space for an allocation).
// When out of memory, you should add another pool (with allocator_add_pool()) or provide a larger starting pool.
// See :BigPhilosophyTime: a bit higher up in this file.
//
// You should avoid adding many pools with this allocator because when we searh for empty
// space we walk the entire linked list (we stop at the first pool which has empty space).
// This is the simplest but not the best behaviour in some cases.
// Be wary that if you have many pools performance will not be optimal. In that case I suggest
// writing a specialized allocator (by taking arena_allocator as an example - implemented in arena_allocator.cpp).
void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

//
// :TemporaryAllocator: See context.h
//
// This is an extension to the arena allocator, things that are different:
// * This allocator is not initialized by default, but the first allocation you do with it adds a starting pool (of size 8_KiB).
//   You can initialize it yourself in a given thread by calling allocator_add_pool() yourself.
// * When you try to allocate a block but there is no available space, this automatically adds another pool (and prints a warning to the console).
//
// One good example use case for the temporary allocator: if you are programming a game and you need to calculate
//   some mesh stuff for a given frame, using this allocator means having the freedom of dynamically allocating
//   without compromising performance. At the end of the frame when the memory is no longer used you FREE_ALL and
//   start the next frame.
//
// We print warnings when allocating new pools. Use that as a guide to see where you need to pay more attention 
// - perhaps increase the pool size or call free_all() more often.
//
void *default_temp_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

LSTD_END_NAMESPACE
