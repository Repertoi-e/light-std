#pragma once

#include "common.hpp"
#include "memory/allocator.hpp"

#define SHARED_MEMORY_NO_ASSERT
#include "memory/delegate.hpp"
#undef SHARED_MEMORY_NO_ASSERT

LSTD_BEGIN_NAMESPACE

// A function that gets called when an assert in the program is called.
// If you don't specify one in the context a default one is provided,
// which on failure prints the information to the log writer and stops the program.
//
// The message is the condition in string form. This allows the assert user to
// add extra information that will get printed, for example:
//
//      assert(index < size && "Index out of bounds.");
//
using Assert_Function = delegate<void(const byte *file, s32 line, const byte *condition)>;

namespace io {
struct writer;
}

namespace internal {
extern io::writer *g_ConsoleLog;
}

struct Implicit_Context {
    // When allocating you should use the context's allocator
    // This makes it so when users call your functions, they
    // can specify an allocator beforehand by pushing a new context,
    // without having to pass you anything as a parameter for example.
    //
    // The idea for this comes from the implicit context in Jai.
    allocator_closure Allocator = MALLOC;

    // This variable is useful when you redirect all logging output
    // (provided that the code that logs stuff uses the context!).
    // By default the writer outputs to the console.
    io::writer *Log = internal::g_ConsoleLog;

    // The delegate that gets called when an assert fails
    Assert_Function AssertFailed = os_assert_failed;
};

// Immutable context available everywhere
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

#define CONTEXT_ALLOC Context.Allocator

#undef assert

#if defined LSTD_DEBUG
#define assert(condition) (!!(condition)) ? (void) 0 : Context.AssertFailed(__FILE__, __LINE__, u8## #condition)
#define LSTD_ASSERT
#else
#define assert(condition) ((void) 0)
#define LSTD_ASSERT
#endif

LSTD_END_NAMESPACE
