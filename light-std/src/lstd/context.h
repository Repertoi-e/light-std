#pragma once

#include "memory/allocator.h"

struct Implicit_Context {
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

    // @TODO
    // This variable is useful when you redirect all logging output
    // (provided that the code that logs stuff uses the context!).
    // By default the writer outputs to the console.
    // io::writer *Log = internal::g_ConsoleLog;
    //
    // @Thread
    // thread::id ThreadID;
};

// Immutable context available everywhere
// The current state gets copied from parent thread to the new thread when creating a thread
inline thread_local const Implicit_Context Context;

#define LSTD_PC_VAR_(x, LINE) LSTD_NAMESPACE_NAME##_lstd_push_context##x##LINE
#define LSTD_PC_VAR(x, LINE) LSTD_PC_VAR_(x, LINE)

// This is a helper macro to safely modify a variable in the implicit context in a block of code.
// Usage:
//    PUSH_CONTEXT(variable, newVariableValue) {
//        ... code with new context variable ...
//    }
//    ... old context variable value is restored ...
//
#define PUSH_CONTEXT(var, newValue)                                                    \
    auto LSTD_PC_VAR(oldVar, __LINE__) = Context.##var;                                \
    auto LSTD_PC_VAR(restored, __LINE__) = false;                                      \
    auto LSTD_PC_VAR(context, __LINE__) = const_cast<Implicit_Context *>(&Context);    \
    defer {                                                                            \
        if (!LSTD_PC_VAR(restored, __LINE__)) {                                        \
            LSTD_PC_VAR(context, __LINE__)->##var = LSTD_PC_VAR(oldVar, __LINE__);     \
        }                                                                              \
    };                                                                                 \
    if (true) {                                                                        \
        LSTD_PC_VAR(context, __LINE__)->##var = newValue;                              \
        goto body;                                                                     \
    } else                                                                             \
        while (true)                                                                   \
            if (true) {                                                                \
                LSTD_PC_VAR(context, __LINE__)->##var = LSTD_PC_VAR(oldVar, __LINE__); \
                LSTD_PC_VAR(restored, __LINE__) = true;                                \
                break;                                                                 \
            } else                                                                     \
            body:
