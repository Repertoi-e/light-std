#pragma once

#include "../memory/allocator.h"
#include "../thread.h"

LSTD_BEGIN_NAMESPACE

struct writer;

namespace internal {
extern writer *g_ConsoleLog;
}  // namespace internal

template <typename T>
struct array;

struct os_function_call;
struct string;

typedef void os_panic_handler_t(const string &message, const array<os_function_call> &callStack);

void default_panic_handler(const string &message, const array<os_function_call> &callStack);

//
// Thread local global variable to control certain behaviours of the program.
// A way to store options that are valid in a certain scope or for certain threads.
//
// Gets initialized when the program runs for the main thread and tls_init (take a look at windows_common.cpp) initializes allocators.
// Options get copied to new threads (take a look at thread_wrapper in windows_thread.cpp).
//
struct context {
    // The current thread's ID
    thread::id ThreadID;

    // This allocator gets initialized the first time it gets used in a thread.
    // Each thread gets a unique temporary allocator to prevent data races and to remain fast.
    temporary_allocator_data TempAllocData{};  // Initialized the first time it is used
    allocator Temp = {temporary_allocator, &TempAllocData};

    ///////////////////////////////////////////////////////////////////////////////////////
    //
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Stuff that is unique to every thread.
    //
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv Stuff that gets copied from the parent when a new thread is spawned.
    //
    // Layout is important because we copy everything after the _Temp_ member.
    //
    // .. We do it that way because tls_init (windows_common.cpp) gets called automatically but it can't have possibly enough info about
    // the parent context, but we still initialize the default allocator and the temporary allocator there.
    // 
    // If we did everything in the thread wrapper (our thread module is the one that copies the context variables)
    // then threads that were not created with this library would not get the same allocator treatment.
    //
    // The result is that we provide a default malloc implementation in the maximum number of cases that are valid
    // and the allocator is used as long as you call the allocate functions from this library.
    //
    // We also ensure that every thread gets it's own version of a temporary allocator of 8 KiB by default so
    // it can do fast allocations without worrying about thread-safety (malloc needs to do that).
    //
    ///////////////////////////////////////////////////////////////////////////////////////

    // When allocating you should use the context's allocator
    // This makes it so when users call your functions they
    // can specify an allocator beforehand by pushing a new context variable,
    // without having to pass you anything as a parameter for example.
    //
    // The idea for this comes from the implicit context in Jai.
    allocator Alloc;  // = Malloc; by default. Initialized in *platform*_common.cpp in _initialize_context_

    u16 AllocAlignment = POINTER_SIZE;  // By default

    // Any options that get OR'd with the options in any allocation (options are implemented as flags).
    // e.g. use this to mark some allocation a function does (in which you have no control of) as a LEAK.
    // Currently there are three allocator options:
    //   - LEAK:                Marks the allocation as a known leak (doesn't get reported when calling allocator::DEBUG_report_leaks())
    u64 AllocOptions = 0;

    // Set this to true to print a list of unfreed memory blocks when the library uninitializes.
    // Yes, the OS claims back all the memory the program has allocated anyway, and we are not promoting C++ style RAII
    // which make even program termination slow, we are just providing this information to the user because they might
    // want to load/unload DLLs during the runtime of the application, and those DLLs might use all kinds of complex
    // cross-boundary memory stuff things, etc. This is useful for debugging crashes related to that.
    bool CheckForLeaksAtTermination = false;

    // Used for debugging. Every time an allocation is made, logs info about it.
    bool LogAllAllocations = false;
    bool LoggingAnAllocation = false;  // Used to avoid infinite looping when the above bool is true.

    // When DEBUG_MEMORY is defined we check the heap for corruption, we do that when a new allocation is made.
    // The problem is that it involves iterating over a linked list of every allocation made.
    // We use the frequency variable below to specify how often we perform that expensive operation.
    // By default we check the heap every 255 allocations, but if a problem is found you may want to decrease it to 1 so
    // your program runs way slower but you catch the corruption at just the right time.
    u8 DebugMemoryVerifyHeapFrequency = 255;

    // Gets called when the program encounters an unhandled expection.
    // This can be used to view the stack trace before the program terminates.
    // The default handler prints the crash message and stack trace to _Log_.
    os_panic_handler_t *PanicHandler = default_panic_handler;
    bool HandlingPanic;  // Used to avoid infinite looping when handling panics

    // When printing you should use this variable.
    // This makes it so users can redirect logging output.
    // By default it points to io::cout (the console).
    writer *Log = internal::g_ConsoleLog;

    // Disable stylized text output (colors, background colors, and bold/italic/strikethrough/underline text).
    // This is useful when logging to a file and not a console. The ansi escape codes look like garbage in files.
    bool FmtDisableAnsiCodes = false;
};

// Immutable context available everywhere
// The current state gets copied from parent thread to the new thread when creating a thread
//
// This used to be const, but with casting and stuff in C++ I don't it's worth the hassle honestly.
// Const just fights the programmer more than it helps him.
// Now that allows us to modify the context cleanly without ugly casting without restricting to scope.
// Even though I really really recommend using WITH_CONTEXT_VAR, WITH_ALLOC, WITH_ALIGNMENT
// since these restore the old value at the end of the scope and in most cases that's what you want.
inline thread_local context Context;

LSTD_END_NAMESPACE

// This is a helper macro to safely modify a variable in the implicit context in a block of code.
// Usage:
//    WITH_CONTEXT_VAR(variable, newVariableValue) {
//        ... code with new context variable ...
//    }
//    ... old context variable value is restored ...
//
#define WITH_CONTEXT_VAR(var, newValue)            \
    auto LINE_NAME(oldVar) = Context.var;          \
    auto LINE_NAME(restored) = false;              \
    defer({                                        \
        if (!LINE_NAME(restored)) {                \
            Context.##var = LINE_NAME(oldVar);     \
        }                                          \
    });                                            \
    if (true) {                                    \
        Context.##var = newValue;                  \
        goto LINE_NAME(body);                      \
    } else                                         \
        while (true)                               \
            if (true) {                            \
                Context.##var = LINE_NAME(oldVar); \
                LINE_NAME(restored) = true;        \
                break;                             \
            } else                                 \
                LINE_NAME(body) :

// Shortcuts for allocations
#define WITH_ALLOC(newAlloc) WITH_CONTEXT_VAR(Alloc, newAlloc)
#define WITH_ALIGNMENT(newAlignment) WITH_CONTEXT_VAR(AllocAlignment, newAlignment)

// These were moved from allocator.h where they made sense to be, but we need to access Context.Alloc here.
// In the future we hopefully find a way to structure the library so these problems are avoided.
// We can't make them non-templates because we need the type info...

template <non_void T>
T *lstd_allocate_impl(s64 count, u32 alignment, allocator alloc, u64 options, const utf8 *file = "", s64 fileLine = -1) {
    s64 size = count * sizeof(T);

    if (!alloc) alloc = Context.Alloc;
    auto *result = (T *) general_allocate(alloc, size, alignment, options, file, fileLine);

    if constexpr (!types::is_scalar<T>) {
        auto *p = result;
        auto *end = result + count;
        while (p != end) {
            new (p) T;
            ++p;
        }
    }
    return result;
}

template <non_void T>
T *lstd_allocate_impl(s64 count, u32 alignment, allocator alloc, const utf8 *file = "", s64 fileLine = -1) {
    return lstd_allocate_impl<T>(count, alignment, alloc, 0, file, fileLine);
}

template <non_void T>
T *lstd_allocate_impl(s64 count, u32 alignment, u64 options, const utf8 *file = "", s64 fileLine = -1) {
    return lstd_allocate_impl<T>(count, alignment, Context.Alloc, options, file, fileLine);
}

template <non_void T>
T *lstd_allocate_impl(s64 count, u32 alignment, const utf8 *file = "", s64 fileLine = -1) {
    return lstd_allocate_impl<T>(count, alignment, Context.Alloc, 0, file, fileLine);
}

// Note: We don't support "non-trivially copyable" types (types that can have logic in the copy constructor).
// We assume your type can be copied to another place in memory and just work.
// We assume that the destructor of the old copy doesn't invalidate the new copy.
template <non_void T>
requires(!types::is_const<T>) T *lstd_reallocate_array_impl(T *block, s64 newCount, u64 options, const utf8 *file = "", s64 fileLine = -1) {
    if (!block) return null;

    // I think the standard implementation frees in this case but we need to decide
    // what _options_ should go there (no options or the ones passed to reallocate?),
    // so we leave that up to the call site.
    assert(newCount != 0);

    auto *header = (allocation_header *) block - 1;
    s64 oldCount = header->Size / sizeof(T);

    if constexpr (!types::is_scalar<T>) {
        if (newCount < oldCount) {
            auto *p = block + newCount;
            auto *end = block + oldCount;
            while (p != end) {
                p->~T();
                ++p;
            }
        }
    }

    s64 newSize = newCount * sizeof(T);
    auto *result = (T *) general_reallocate(block, newSize, options, file, fileLine);

    if constexpr (!types::is_scalar<T>) {
        if (oldCount < newCount) {
            auto *p = result + oldCount;
            auto *end = result + newCount;
            while (p != end) {
                new (p) T;
                ++p;
            }
        }
    }
    return result;
}

// We assume your type can be copied to another place in memory and just work.
// We assume that the destructor of the old copy doesn't invalidate the new copy.
template <non_void T>
requires(!types::is_const<T>) T *lstd_reallocate_array_impl(T *block, s64 newCount, const utf8 *file = "", s64 fileLine = -1) {
    return lstd_reallocate_array_impl(block, newCount, 0, file, fileLine);
}

// If T is non-scalar we call the destructors on _block_ (completely determined by T, so make sure you pass that correctly!)
template <typename T>
requires(!types::is_const<T>) void lstd_free_impl(T *block, u64 options = 0) {
    if (!block) return;

    s64 sizeT = 1;
    if constexpr (!types::is_same<T, void>) {
        sizeT = sizeof(T);
    }

    auto *header = (allocation_header *) block - 1;
    s64 count = header->Size / sizeT;

    if constexpr (!types::is_same<T, void> && !types::is_scalar<T>) {
        auto *p = block;
        while (count--) {
            p->~T();
            ++p;
        }
    }

    general_free(block, options);
}

// T is used to initialize the resulting memory (uses placement new).
// When you pass DO_INIT_0 we initialize the memory with zeroes before initializing T.
#if defined DEBUG_MEMORY
#define allocate(T, ...) lstd_allocate_impl<T>(1, 0, __VA_ARGS__, __FILE__, __LINE__)
#define allocate_aligned(T, alignment, ...) lstd_allocate_impl<T>(1, alignment, __VA_ARGS__, __FILE__, __LINE__)
#define allocate_array(T, count, ...) lstd_allocate_impl<T>(count, 0, __VA_ARGS__, __FILE__, __LINE__)
#define allocate_array_aligned(T, count, alignment, ...) lstd_allocate_impl<T>(count, alignment, __VA_ARGS__, __FILE__, __LINE__)

#define reallocate_array(block, newCount, ...) lstd_reallocate_array_impl(block, newCount, __VA_ARGS__, __FILE__, __LINE__)

#define free lstd_free_impl
#else
#define allocate(T, ...) lstd_allocate_impl<T>(1, 0, __VA_ARGS__)
#define allocate_aligned(T, alignment, ...) lstd_allocate_impl<T>(1, alignment, __VA_ARGS__)
#define allocate_array(T, count, ...) lstd_allocate_impl<T>(count, 0, __VA_ARGS__)
#define allocate_array_aligned(T, count, alignment, ...) lstd_allocate_impl<T>(count, alignment, __VA_ARGS__)

#define reallocate_array(block, newCount, ...) lstd_reallocate_array_impl(block, newCount, __VA_ARGS__)

#define free lstd_free_impl
#endif

//
// We overload the new/delete operators so we handle the allocations. The allocator used is the one specified in the Context.
//
void *operator new(size_t size);
void *operator new[](size_t size);

void *operator new(size_t size, align_val_t alignment);
void *operator new[](size_t size, align_val_t alignment);

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;

void operator delete(void *ptr, align_val_t alignment) noexcept;
void operator delete[](void *ptr, align_val_t alignment) noexcept;
