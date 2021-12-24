module;

#include "../common.h"
#include "vendor/tlsf/tlsf.h"

export module lstd.memory;

///
/// Defines the structure of allocators in this library.
///

LSTD_BEGIN_NAMESPACE

//
// By default we do some extra work when allocating to make it easier to catch memory related bugs.
// That work is measureable in performance so we don't want to do it in Release configuration.
// If you want to disable it for Debug/DebugOptimized as well, define FORCE_NO_DEBUG_MEMORY.
// You can read the comments in this file (around where DEBUG_MEMORY is mentioned)
// and see what extra safety stuff we do.
//

export {
    enum class allocator_mode {
        ALLOCATE,
        RESIZE,
        FREE,
        FREE_ALL
    };

    // :AllocationFlags:
    // Allocations marked explicitly as leaks don't get reported when calling debug_memory_report_leaks().
    // This is handled internally when passed, so allocator implementations needn't pay attention to it.
    constexpr u64 LEAK = 1ull << 63;

    //
    // This specifies what the signature of each allocation function should look like.
    //
    // _mode_ is what we are doing currently: allocating, resizing, freeing a block or freeing everything
    //      Note: * Implementing FREE_ALL is not a requirement, some allocators can't support this by design.
    //
    // _context_ is used as a pointer to any data the allocator needs as state
    // _size_ is the size of the allocation
    // _oldMemory_ is used when resizing or freeing a block
    // _oldSize_ is used only when resizing, the old size of the memory block
    //
    // The last pointer to u64 is reserved for options. The allocator implementation decides how to treat those.
    // We just provide a way to pass those. Note: The LEAK option means that bit number 64 is reserved for special use.
    // That means that the maximum integer you have to work with is the bottom 63 bits of the options.
    // I expect people to use this as a bit field anyway.
    //
    //
    // !!! When called with RESIZE, this doesn't mean "reallocate"!
    //     Only valid return here is _oldMemory_ (memory was grown/shrank in place)
    //     or null - memory can't be resized and needs to be moved.
    //
    //     In the second case we automatically handle allocating a new block and copying the old data there
    //     (in general_reallocate) so allocator implementations don't need to pay attention to this.
    //
    // !!! Alignment is handled internally. Allocator implementations don't need to pay attention to it.
    //     When an aligned allocation is being made, we send a request at least _alignment_ bytes larger,
    //     so when the allocator function returns an unaligned pointer we can freely bump it.
    //     The information about the alignment is saved in the header and that's how we know what
    //     the original returned pointer was (so we pass it properly when freeing or reallocating).
    //
    // We do this so custom allocator implementations can be kept as simple as possible.
    //
    using allocator_func_t = void *(*) (allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options);

    // This is a hack that's used to get around Context's and TemporaryAllocator's constructors
    // getting fired and overriding its allocator (which may have potentially been set by other global constructors).
    struct allocator_dont_init_t {};

    struct allocator {
        allocator_func_t Function;
        void *Context;

        allocator() : Function(null), Context(null) {}
        allocator(allocator_dont_init_t) {}
        allocator(allocator_func_t function, void *context) : Function(function), Context(context) {}

        bool operator==(allocator other) const { return Function == other.Function && Context == other.Context; }
        bool operator!=(allocator other) const { return Function != other.Function || Context != other.Context; }

        operator bool() const { return Function; }
    };

    // Note: Not all allocators must support this.
    void free_all(allocator alloc, u64 options = 0);

    //
    // Here we define malloc/calloc/realloc/free.
    //

    template <typename T>
    concept non_void = !types::is_same<T, void>;

    template <non_void T>
    T *lstd_allocate_impl(s64 count, allocator alloc, u32 alignment, u64 options, source_location loc);

    template <non_void T>
    requires(!types::is_const<T>) T *lstd_reallocate_impl(T * block, s64 newCount, u64 options, source_location loc);

    template <non_void T>
    requires(!types::is_const<T>) void lstd_free_impl(T * block, u64 options, source_location loc);

    //
    // :BigPhilosophyTime: Some more stuff..
    //
    // We don't use new and delete.
    // 1) The syntax is ugly in my opinion.
    // 2) You have to be careful not to mix "new" with "delete[]"/"new[]" and "delete".
    // 3) It's an operator and can be overriden by structs/classes.
    // 4) Modern C++ people say not to use new/delete as well, so ..
    //
    // Now seriously, there are two ways to allocate memory in C++: malloc and new.
    // For the sake of not introducing a THIRD way, we override malloc.
    // We do that because :STANDARDLIBRARYISBANNED: (see tagged comment).
    //
    // Since we don't link the CRT malloc is undefined, so we need to
    // provide a replacement anyway (or modify code which is annoying and
    // not always possible, e.g. a prebuilt library).
    //
    // *** Caveat: A DLL may already have linked with the CRT, which means that in
    // that case problems occur. There are two options: prebuild your DLLs
    // to not use the standard library (ideally), or we could do some hacks
    // and redirect calls to malloc to our replacement (@TODO It may actually be possible).
    //
    //
    // new and delete actually have some useful semantics (new - initializing the values,
    // delete - calling a destructor if defined). So we provide templated versions of malloc/free.
    //
    // malloc<T>:
    // - Calls constructors on non-scalar values.
    // - Returns T* so you don't have to cast from void*
    // - Can't call with T == void (use non-templated malloc in that case!)
    //
    // free<T>:
    // - Doesn't call destructors. We are strongly against destructors. Explicit code is better
    //   so to implement "destructor functionality" (unitializing the type, freeing members, etc.)
    //   you provide an overload of free that takes your type as an argument.
    //
    //   e.g.   free(string *s) {
    //              free(s.Data);
    //              atomic_add(&GlobalStringCount, -1);
    //          }
    //
    //   You may say that this is just a renamed destructor. But this doesn't run hiddenly
    //   (when exiting a scope or a function). Sometimes you actually want to do that,
    //   in that case you can do    defer(free(s));    Which is a useful macro that emulates
    //   a destructor (calls any statements at the end of a scope);
    //
    // For more philosophy (like why we don't like exceptions, copy or move constructors)
    // you can look at the type policy (:TypePolicy: in common.h).
    //
    //
    // We allocate a bit of space before a block to store a header with information (the size of the allocation, the alignment,
    // the allocator with which it was allocated, and debugging info if DEBUG_MEMORY is defined - see comments in allocator.h).
    //              (* this overhead should become smaller, right now we haven't bothered yet! @TODO)
    //
    // There is one big assumption we make:
    //   Your types are "trivially copyable" which means that they can be copied byte by byte to another place and still work.
    //   We don't call copy/move constructors when e.g. reallocating.
    //
    // We are following a data oriented design with this library.
    // e.g. our array<> type also expects type to be simple data. If you need extra logic when copying memory,
    // implement that explicitly (not with a copy constructor). We do that to avoid C++ complicated shit that
    // drives sane people insane. Remember: here we don't do C++ style exceptions, we don't do copy/move constructors,
    // we don't do destructors, we don't do any of that.
    //
    // Note: I said we don't do destructors but we still call them. That's because
    // external code may still rely on them and we don't want memory leaks.
    //
    // However since arrays and even the basic allocation functions copy byte by byte, that means that types that own
    // memory (like string) must be implemented differently. They can't rely on copy constructors to do the dirty work.
    // In that case destructors may create a situation where memory is freed before it needs to, and the new shallow
    // copy gets invalidated. Since we are explicit with free() that problem vanishes. (:TypePolicy: in common.h).
    // In the end, you don't have to write bloaty giant bug-prone copy/move constructors which are also
    // sometimes a reason for slow code (e.g. to make sure you don't do unnecessary expensive copies you have
    // to put const& everywhere). For these reasons I think our policy is better.
    //
    //
    // The functions also allow certain options to be specified. Using C++20 syntax, here are a few examples on how calls look:
    //
    //     auto *node = malloc<ast_binop>();
    //     auto *node = malloc<ast_binop>({.Alloc = AstNodeAllocator});
    //     auto *node = malloc<ast_binop>({.Options = MY_SPECIAL_FLAG | LEAK});
    //
    //     auto *simdType = malloc<f32v4>({.Alloc = TemporaryAllocator, .Alignment = 16});
    //
    //     auto *memory = malloc<byte>({.Count = 200});
    //     auto *memory = malloc<byte>({.Count = 200, .Alloc = my_special_allocator, .Alignment = 64, .Options = LEAK});
    //
    //
    // The functions take source_location (C++20) as a final parameter.
    // This info (file, line number and parent function) is saved to the header when DEBUG_MEMORY is defined
    // and helps track down allocations.
    //

    struct allocate_options {
        // How many items to allocate, historically we provided a seperate allocate_array routine, but we merged it.
        s64 Count = 1;

        allocator Alloc = {};
        u32 Alignment   = 0;
        u64 Options     = 0;
    };

    // T is used to initialize the resulting memory (uses placement new to call the constructor).
    //
    // In C++20 we can do constexpr allocations.
    template <non_void T>
    T *malloc(allocate_options options = {}, source_location loc = source_location::current()) {
        return lstd_allocate_impl<T>(options.Count, options.Alloc, options.Alignment, options.Options, loc);
    }

    // We have just 2 parameters but we need to have different parameter type than regular realloc to avoid overload issues.
    struct reallocate_options {
        s64 NewCount;
        s64 Options = 0;
    };

    // We don't provide a templated calloc, because it doesn't make much sense.
    // There is no reason to initialize everything to zero after malloc has called T's constructors.
    // Either modify your type to initialize it's members to zero in the constructor or call the standard
    // calloc routine which is outside of this namespace.
    // template <non_void T>
    // T *calloc(allocate_options options = {}, source_location loc = source_location::current()) { }

    // If DEBUG_MEMORY is defined, calling reallocate on a block that is already freed panics the program and gives information about the site.
    //
    // Note: We don't support "non-trivially copyable" types (types that can have logic in the copy constructor).
    // We assume your type can be copied to another place in memory and just work.
    // We assume that the destructor of the old copy doesn't invalidate the new copy.
    // We don't do destructors in this library but we still call them here because external code may have types that leak otherwise.
    //
    // In C++20 we can do constexpr allocations.
    template <non_void T>
    T *realloc(T * block, reallocate_options options, source_location loc = source_location::current()) {
        return lstd_reallocate_impl<T>(block, options.NewCount, options.Options, loc);
    }

    // If DEBUG_MEMORY is defined, calling free on a block that is already freed panics the program and gives information about the site.
    //
    // We don't do destructors in this library but we still call them here because external code may have types that leak otherwise.
    // If T is non-scalar we call the destructors on the objects in the memory block (the destructor is determined by T, like
    // in the C++ delete, so make sure you pass a correct pointer type).
    //
    // The allocation header contains the size of the allocation, that's how we know how many destructors to call (assuming the T is valid).
    // new[] and delete[] are implemented the same way (they save an integer before the returned block).
    // That's why it's dangerous to mix new and delete[] and new[] and delete.
    // However we only have one type of free here.
    //
    // In C++20 we can do constexpr allocations.
    template <non_void T>
    requires(!types::is_const<T>) void free(T * block, u64 options = 0, source_location loc = source_location::current()) {
        lstd_free_impl(block, options, loc);
    }

    //
    // In DEBUG we put extra stuff to make bugs more obvious. These constants are like the ones MSVC's debug CRT model uses.
    // Like them, we use specific values for bytes outside the allocated range, for freed memory and for uninitialized memory.
    //
    // The values of the constants and the following comment is taken from the MSVC implementation:
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
    constexpr s64 NO_MANS_LAND_SIZE = 4;

    // _NO_MANS_LAND_SIZE_ (4) extra bytes with this value before and after the allocation block
    // which help detect reading out of range errors
    constexpr u8 NO_MANS_LAND_FILL = 0xFD;

    // When freeing we fill the block with this value (detects bugs when accessing memory that's freed)
    constexpr u8 DEAD_LAND_FILL = 0xDD;

    // When allocating a new block we fill it with this value
    // (detects bugs when accessing memory before initializing it)
    constexpr u8 CLEAN_LAND_FILL = 0xCD;

    //
    // Each allocation contains this header before the returned pointer.
    // The returned pointer is guaranteed to be aligned to the specified alignment,
    // we do that by padding this structure. Info about that is saved in the header itself.
    // Right now this uses 32 bytes when DEBUG_MEMORY is not defined, 96 bytes when storing debug info.
    //
    // @TODO: This may be too big of an overhead. We can make this smaller!
    //        We can split allocations into small/medium/large, and use different headers (like https://nothings.org/stb/stb_malloc.h).
    //
    struct allocation_header {
        // The allocator used when allocating the memory. We need this when resizing/freeing
        // in order to call the right allocator procedure. By design, we can't get rid of this.
        allocator Alloc;

        // The size of the allocation (NOT including the size of the header and padding).
        // By design, we can't get rid of this (but we can make this smaller for medium
        // sized allocations by packing a different header).
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
        u16 Alignment;         // We allow a maximum of 65535 byte alignment
        u16 AlignmentPadding;  // Offset from the block that needs to be there in order for the result to be aligned

#if defined DEBUG_MEMORY
        // This is used to detect buffer underruns.
        // There may be padding after this member, but we treat this region as "(allocation_header *) p + 1 - 4 bytes".
        // This doesn't matter since we just need AT LEAST 4 bytes free.
        char DEBUG_NoMansLand[NO_MANS_LAND_SIZE];

        // After the whole memory block we follow again by NO_MANS_LAND_SIZE bytes of no man's land again.
        // We use that to detect buffer overruns.
#endif
    };

// #if'd so programs don't compile when debug memory info shouldn't be used.
#if defined DEBUG_MEMORY
    thread_local s64 AllocationCount;

    struct debug_memory_node {
        debug_memory_node *Next;
        debug_memory_node *Prev;

        allocation_header *Header;

        //
        // The thread which allocated this block. We use this to check if memory
        // is freed by another thread. By default we treat that as an error.
        // However sometimes that can be intentional, so we provide a way
        // to disable this treatment explicitly (with the ALLOW_CROSSTHREAD_FREE allocation flag).
        // See :AllocationFlags:
        //
        // @TODO: Currently :ThreadIDsAreRandom:
        u32 TID;

        // @TODO: We don't support cross-thread freeing right now because
        // our debug list of nodes is not thread safe. We can make a thread-safe queue
        // for handling these cases. For now we ban it.
        // bool MarkedAllowCrossThreadFree;

        //
        // The ID is useful for debugging bugs related to allocations.
        // Every allocation has an unique number == to the ID of the previous allocation + 1.
        // The allocation count is unique to each thread in order to ensure reproducibility.
        //
        // Each time you run your program with the same set of inputs the
        // ID of each allocation is the same (assuming no randomness).
        //
        // You can set a breakpoint with the ID in general_allocate()
        // to catch the allocation before it happens.
        //
        // Currently :ThreadIDsAreRandom:, so you may need to skip a few breaks,
        // @TODO: We shouldn't use the OS thread id but keep our own state with sequential IDs.
        //
        s64 ID;

        //
        // This ID keeps track how many times this block has been reallocated.
        // When realloc() is called we check if the block can be directly resized in place (using
        // allocation_mode::RESIZE). If not, we allocate a new block and copy the information there.
        // In both cases the ID above stays the same and RID gets incremented. This always starts at 0.
        //
        // This helps debug memory allocations that are reallocated often (strings/arrays, etc.)
        //
        s64 RID;

        //
        // We mark the source of the (re)allocation if such information was provided.
        // On subsequent reallocations we overwrite this.
        //
        // When allocating with the default malloc/calloc/realloc (the non-templated
        // extern "C" versions which are replacements for the standard library functions)
        // we don't get a very useful FileName and FileLine (we get the ones from the wrapper).
        // This also applies when an allocation is made with C++ new.
        //
        // @TODO Remove this and instead save a call stack (optional!, that's a lot of info).
        //
        // To debug allocations you can try to use the _ID_ and set a breakpoint. See info above.
        //
        source_location AllocatedAt;

        //
        // When calling general_free() we free the block with the allocator implementation
        // but keep the node live in the list. We do this in order to detect freeing the same pointer twice.
        // If the program requests a new block and the allocator implementation returns the same memory
        // address then we reuse this node and clear the flag.
        //
        // Not freeing nodes might accummulate some memory overhead. @TODO Provide a clear_freed_nodes() routine.
        // Fragmentation is not a problem because we allocate nodes from a pool allocator.
        //
        bool Freed;

        // See note above about not getting the right info from the extern "C" versions and C++ operators new/delete.
        // @TODO Remove this and instead save a call stack (optional!, that's a lot of info).
        source_location FreedAt;

        //
        // When allocating a block the caller can mark it as a leak (using the LEAK allocation flag).
        // See :AllocationFlags:
        //
        // That means that it doesn't get freed before the end of the program (since the OS claims back the memory anyway).
        // When DEBUG_memory->CheckForLeaksAtTermination is set to true we log a list of unfreed allocations
        // at termination. You can also call debug_memory_report_leaks().
        //
        // Blocks which have this flag don't get reported (they are "meant" to be "leaks").
        //
        bool MarkedAsLeak;
    };

    // We store a per-thread list of allocations made, in order to look for leaks and check memory integrity
    // (detect buffer under/overruns, modifying freed memory, freeing the same pointer twice, etc.).
    //
    // The list is doubly-linked and sorted by the value of the pointer of the allocation (in increasing order).
    //
    // We also detect if allocator implementations return overlapping blocks,
    // which may happen if two allocators use the same pool, or the implementation itself has a bug.
    //
    //
    // Overall we have a pretty robust and helpful debug memory model which not a lot of
    // runtimes care to provide although its trivial to do these checks.
    thread_local debug_memory_node *DebugMemoryHead, *DebugMemoryTail;

    // Called when creating a new thread (e.g. in os.win32.common.cppm).
    // Allocates starting sentinel values for the list and the pool for futher nodes.
    void debug_memory_init();

    // Called on thread exit. Calls debug_memory_report_leaks
    // and deallocates any left-over memory.
    void debug_memory_uninit();

    // Returns true if the allocation is in this thread's list.
    // e.g. this is used in array<T> to check for crossthread access
    // which may result in errors because array<T> is not thread-safe.
    bool debug_memory_list_contains(allocation_header * header);

    // Assuming that the heap is not corrupted, this reports any unfreed allocations.
    // Yes, the OS claims back all the memory the program has allocated anyway; we are not promoting
    // C++ style RAII which make EVEN program termination slow. We give this information to the
    // programmer because they might find that useful.
    void debug_memory_report_leaks();

    // Verifies the integrity of headers in all allocations.
    void debug_memory_verify_heap();

    // See :MemoryVerifyHeapFrequency:
    void debug_memory_maybe_verify_heap();
#endif

    //
    // :BigPhilosophyTime: Read here.
    //
    // 2007, "What Every Programmer Should Know About Memory", Ulrich Drepper
    // https://people.freebsd.org/~lstewart/articles/cpumemory.pdf
    //
    // We don't provide a traditional general purpose "malloc" function.
    //
    // I think the programmer should be very aware of the memory the program is using.
    // That's why we require allocators to be initted with an initial (or several) large blocks (pools)
    // from which smaller blocks are used for allocations. This forces you to think about the memory layout.
    // The hope is that faster software is produced because modern computers suffer a lot from cache misses.
    //
    // C++ is a low-level language (was high-level in old days when ASM was low-level, but now we consider it low-level).
    // Usually modern high-level languages put much of the memory management behind walls of abstraction.
    // Hardware has gotten blazingly fast, but software has deteriorated. Modern CPUs can make billions
    // of calculations per second, but reading even ONE byte from RAM can take hundreds of clock cycles
    // (if the memory is not in the cache). You MUST think about the cache if you want to write fast software.
    //
    // Once you start thinking about the cache you start programming in a data oriented way.
    // You don't try to model the real world in the program, but instead structure the program in the way that
    // the computer will work with the data. Data that is processes together should be close together in memory.
    //
    // And that's why we should remove some of the abstraction. We should think about the computer's memory.
    // We should write fast software. We can slow down global warming by not wasting CPU clock cycles.
    //
    // Caveat: Of course, writing abstractions which allows more rapid programming is the rational thing to do.
    // After all we can do so much more stuff instead of micro-optimizing everything. But being a bit too careless
    // results in the modern mess of software that wastes most of CPU time doing nothing, because people decided to
    // abstract too much stuff.
    //
    // I am writing this in 2021 - in the midst of the COVID-19 crisis. We have been using online video conferencing
    // software for school and work for a year now. The worst example is MS Teams. Clicking a button takes a good second
    // to open a chat. It lags constantly, bugs everywhere. No. Your computer is not slow. Your computer is a
    // super-computer compared to what people had 30-40 years ago.
    //
    // Not only you waste electricity by being a careless programmer, you also waste USER'S TIME!
    // If your program is used by millions of PC, 1 second to click a SIMPLE BUTTON quickly becomes hours and then days.
    //

    //
    // Generally malloc implementations do the following:
    // - Have seperate heaps for different sized allocations
    // - Call OS functions for very large allocations
    // - Different algorithms for allocation, e.g. stb_malloc implements the TLSF algorithm for O(1) allocation
    //
    // Here we provide a wrapper around the TLSF algorithm.
    // Here is how you can get a general purpose allocator:
    // - Allocate a large block with the OS allocator
    // - Call tlsf_allocator_add_pool on your TLSF.
    // - Don't take this as boilerplate which you need to write so you can then use this general purpose
    //   allocator for everything. Once you go beyond the quick and dirty sketching of your code, look for
    //   places which can benefit from a more specialized allocator (arena allocator, pool allocator, etc.)
    //

    struct tlsf_allocator_data {
        tlsf_t State = null;  // We use a vendor library that implements the algorithm.
    };

    //
    // Two-Level Segregated Fit memory allocator implementation. Wrapper around tlsf.h/cpp (in vendor folder),
    // written by Matthew Conte (matt@baisoku.org). Released under the BSD license.
    //
    // * O(1) cost for alloc, free, resize
    // * Extremely low overhead per allocation (4 bytes)
    // * Low overhead per TLSF management of pools (~3kB)
    // * Low fragmentation
    //
    void *tlsf_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
        assert(context);

        auto *data = (tlsf_allocator_data *) context;

        if (!data->State) {
            assert(false && "No pools have been added yet! Add the first one with tlsf_allocator_add_pool().");
            return null;
        }

        switch (mode) {
            case allocator_mode::ALLOCATE:
                return tlsf_malloc(data->State, size);
            case allocator_mode::RESIZE:
                return tlsf_resize(data->State, oldMemory, size);
            case allocator_mode::FREE: {
                tlsf_free(data->State, oldMemory);
                return null;
            }
            case allocator_mode::FREE_ALL: {
                assert(false);  // Some allocators can't support this by design
                return null;
            }
        }
        return null;
    }

    void tlsf_allocator_add_pool(tlsf_allocator_data * data, void *block, s64 size) {
        if (!data->State) {
            data->State = tlsf_create_with_pool(block, (u64) size);
        } else {
            tlsf_add_pool(data->State, block, (u64) size);
        }
    }

    // Assumes the block exists
    void tlsf_allocator_remove_pool(tlsf_allocator_data * data, void *block) {
        tlsf_remove_pool(data->State, block);
    }

    //
    // void *default_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 *);
    // allocator Malloc;
    //
    // We don't provide a default allocator anymore, as explained above.
    //

    struct arena_allocator_data {
        void *Block = null;  // This should be supplied before using the allocator
        s64 Size    = 0;

        s64 Used = 0;
    };

    //
    // Arena allocator.
    //
    // This type of allocator super fast because it basically bumps a pointer.
    // With this allocator you don't free individual allocations, but instead free
    // the entire thing (with FREE_ALL) when you are sure nobody uses the memory anymore.
    // Note that free_all doesn't free the added pools, but instead resets their
    // pointers to the beginning of the buffer.
    //
    // The arena allocator doesn't handle overflows (when the block doesn't have enough space for an allocation).
    // When out of memory, you should provide another block.
    //
    // You should avoid adding many pools with this allocator because when we searh for empty
    // space we walk the entire linked list (we stop at the first pool which has empty space).
    // This is the simplest but not the best behaviour in some cases.
    //
    // Be wary that if you have many pools performance will not be optimal.
    // In that case I suggest writing a specialized allocator.
    void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
        auto *data = (arena_allocator_data *) context;

        switch (mode) {
            case allocator_mode::ALLOCATE: {
                if (data->Used + size >= data->Size) return null;  // Not enough space

                void *result = (byte *) data->Block + data->Used;
                data->Used += size;
                return result;
            }
            case allocator_mode::RESIZE: {
                void *p = (byte *) data->Block + data->Used - oldSize;
                if (oldMemory == p) {
                    // We can resize only if it's the last allocation
                    data->Used += size - oldSize;
                    return oldMemory;
                }
                return null;
            }
            case allocator_mode::FREE: {
                // We don't free individual allocations in the arena allocator
                return null;
            }
            case allocator_mode::FREE_ALL: {
                data->Used = 0;
                return null;
            }
        }
        return null;
    }

    // Hack, the default constructor would otherwise zero init the debug memory pool's members,
    // which is set before global constructors run. Similar thing happens with context.
    struct pool_allocator_dont_init_t {};

    //
    // Pool allocator.
    //
    // A variation of the fast bump arena allocator.
    // Allows O(1) allocation and freeing of individual elements.
    //
    // The limitation is that each allocation must have the same predefined size.
    //
    // This allocator is useful for managing a bunch of objects of the same type.
    //
    struct pool_allocator_data {
        s64 ElementSize;  // You must set this before using the allocator

        struct block {
            block *Next;
            s64 Size;
        };

        block *Base;

        struct chunk {
            chunk *Next;
        };
        chunk *FreeList;

        pool_allocator_data() : ElementSize(0), Base(null), FreeList(null) {}
        pool_allocator_data(pool_allocator_dont_init_t) {}
    };

#if defined DEBUG_MEMORY
    thread_local pool_allocator_data DebugMemoryNodesPool = pool_allocator_data(pool_allocator_dont_init_t{});
#endif

    void pool_allocator_add_free_chunks(pool_allocator_data * data, void *block, s64 size) {
        auto *c = (pool_allocator_data::chunk *) block;

        auto *oldFreeList = data->FreeList;
        data->FreeList    = c;

        For(range(size / data->ElementSize - 1)) {
            c->Next = (pool_allocator_data::chunk *) ((byte *) c + data->ElementSize);
            c       = c->Next;
        }

        c->Next = oldFreeList;
    }

    // Use this to provide more space in the pool allocator. Also inits the first block.
    // Size needs to be multiple of _ElementSize_ + sizeof(allocator_pool_data::block).
    // We avoid allocating a seperate linked list but use the first few bytes of _block_ as a header.
    void pool_allocator_provide_block(pool_allocator_data * data, void *block, s64 size) {
        assert(size >= (s64) sizeof(pool_allocator_data::block) + data->ElementSize);
        assert(data->ElementSize > 0);

        auto *b = (pool_allocator_data::block *) block;
        b->Size = size - sizeof(pool_allocator_data::block);
        assert(b->Size % data->ElementSize == 0);

        b->Next    = data->Base;
        data->Base = b;

        pool_allocator_add_free_chunks(data, b + 1, b->Size);
    }

    void *pool_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
        auto *data = (pool_allocator_data *) context;

        switch (mode) {
            case allocator_mode::ALLOCATE: {
                assert(size == data->ElementSize);

                if (data->FreeList) {
                    auto *block    = data->FreeList;
                    data->FreeList = block->Next;
                    return block;
                }
                return null;
            }
            case allocator_mode::RESIZE: {
                assert(false && "Can't do that");
                return null;
            }
            case allocator_mode::FREE: {
                auto *c        = (pool_allocator_data::chunk *) oldMemory;
                c->Next        = data->FreeList;
                data->FreeList = c;
                return null;
            }
            case allocator_mode::FREE_ALL: {
                data->FreeList = null;

                auto *b = data->Base;
                while (b) {
                    pool_allocator_add_free_chunks(data, b + 1, b->Size);
                    b = b->Next;
                }
                return null;
            }
        }
        return null;
    }

    // These handle alignment, populating the allocation header and debug memory stuff.
    void *general_allocate(allocator alloc, s64 userSize, u32 alignment, u64 options, source_location loc);

    // If DEBUG_MEMORY is defined, calling reallocate on a block that is already freed panics the program and gives information about the site.
    void *general_reallocate(void *ptr, s64 newSize, u64 options, source_location loc);

    // Calling free on a null pointer doesn't do anything.
    // If DEBUG_MEMORY is defined, calling free on a block that is already freed panics the program and gives information about the site.
    void general_free(void *ptr, u64 options, source_location loc);

    // Calculates the required padding in bytes which needs to be added to _ptr_ in order to be aligned
    u16 calculate_padding_for_pointer(void *ptr, s32 alignment) {
        assert(alignment > 0 && is_pow_of_2(alignment));
        return (u16) (((u64) ptr + (alignment - 1) & -alignment) - (u64) ptr);
    }

    // Like calculate_padding_for_pointer but padding must be at least the header size
    u16 calculate_padding_for_pointer_with_header(void *ptr, s32 alignment, u32 headerSize) {
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
}

template <non_void T>
requires(!types::is_const<T>) T *lstd_reallocate_impl(T *block, s64 newCount, u64 options, source_location loc) {
    if (!block) return null;

    // I think the standard implementation frees in this case but we need to decide
    // what _options_ should go there (no options or the ones passed to reallocate?),
    // so we leave that up to the call site.
    assert(newCount != 0);

    auto *header = (allocation_header *) block - 1;
    s64 oldCount = header->Size / sizeof(T);

    if constexpr (!types::is_scalar<T>) {
        if (newCount < oldCount) {
            auto *p   = block + newCount;
            auto *end = block + oldCount;
            while (p != end) {
                p->~T();
                ++p;
            }
        }
    }

    s64 newSize  = newCount * sizeof(T);
    auto *result = (T *) general_reallocate(block, newSize, options, loc);

    if constexpr (!types::is_scalar<T>) {
        if (oldCount < newCount) {
            auto *p   = result + oldCount;
            auto *end = result + newCount;
            while (p != end) {
                new (p) T;
                ++p;
            }
        }
    }
    return result;
}

template <non_void T>
T *lstd_allocate_impl(s64 count, allocator alloc, u32 alignment, u64 options, source_location loc) {
    s64 size = count * sizeof(T);

    auto *result = (T *) general_allocate(alloc, size, alignment, options, loc);

    if constexpr (!types::is_scalar<T>) {
        auto *p   = result;
        auto *end = result + count;
        while (p != end) {
            new (p) T;
            ++p;
        }
    }
    return result;
}

template <non_void T>
requires(!types::is_const<T>) void lstd_free_impl(T *block, u64 options, source_location loc) {
    if (!block) return;

    auto *header = (allocation_header *) block - 1;
    s64 count    = header->Size / sizeof(T);

    if constexpr (!types::is_scalar<T>) {
        auto *p = block;
        while (count--) {
            p->~T();
            ++p;
        }
    }

    // @TODO
    //if constexpr (is_constant_evaluated()) {
    //    // Constexpr allocations in C++20 seem to just look for the magic symbol "delete".
    //    // Doesn't care if it's defined or not.
    //    delete block;
    //} else {
    general_free(block, options, loc);
    // }
}

LSTD_END_NAMESPACE
