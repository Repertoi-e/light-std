#pragma once

#include "memory/allocator.h"

LSTD_BEGIN_NAMESPACE

namespace io {
struct writer;
}

namespace internal {
extern io::writer *g_ConsoleLog;
}  // namespace internal

struct implicit_context {
    // When allocating you should use the context's allocator
    // This makes it so when users call your functions they
    // can specify an allocator beforehand by pushing a new context variable,
    // without having to pass you anything as a parameter for example.
    //
    // The idea for this comes from the implicit context in Jai.
    allocator Alloc = Malloc;

    // This needs to be initialized by calling init_temporary_allocator() before using it in a thread.
    // It's thread local to prevent data races and to remain fast (thread safety implies overhead)
    allocator TemporaryAlloc = {temporary_allocator, null};

    // _storageSize_ specifies how many bytes of memory to reserve for the allocator
    // This function always uses the global malloc allocator (and not the context's one)
    void init_temporary_allocator(size_t storageSize) const;

    // Frees the memory held by the temporary allocator
    void release_temporary_allocator() const;

    // This variable is useful when you redirect all logging output
    // (provided that the code that logs stuff uses the context!).
    // By default it points to io::cout
    io::writer *Log = internal::g_ConsoleLog;

    //
    // @Thread
    // thread::id ThreadID;
};

// Immutable context available everywhere
// The current state gets copied from parent thread to the new thread when creating a thread
inline thread_local const implicit_context Context;

LSTD_END_NAMESPACE

// This is a helper macro to safely modify a variable in the implicit context in a block of code.
// Usage:
//    PUSH_CONTEXT(variable, newVariableValue) {
//        ... code with new context variable ...
//    }
//    ... old context variable value is restored ...
//
#define PUSH_CONTEXT(var, newValue)                                                    \
    auto LINE_NAME(oldVar) = Context.var;                                \
    auto LINE_NAME(restored) = false;                                      \
    auto LINE_NAME(context) = const_cast<implicit_context *>(&Context);    \
    defer({                                                                            \
        if (!LINE_NAME(restored)) {                                        \
            LINE_NAME(context)->##var = LINE_NAME(oldVar);     \
        }                                                                              \
    });                                                                                \
    if (true) {                                                                        \
        LINE_NAME(context)->##var = newValue;                              \
        goto body;                                                                     \
    } else                                                                             \
        while (true)                                                                   \
            if (true) {                                                                \
                LINE_NAME(context)->##var = LINE_NAME(oldVar); \
                LINE_NAME(restored) = true;                                \
                break;                                                                 \
            } else                                                                     \
            body:
