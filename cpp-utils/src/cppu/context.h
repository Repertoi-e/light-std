#pragma once

#include "common.h"
#include "memory/allocator.h"

#include <functional>

CPPU_BEGIN_NAMESPACE

// Note that this function isn't meant for
// formatting (that would be print in string/print.h!) but just for
// outputting any given text. This is useful for example when your game
// has a custom in game console. So then you can have an easy way to
// redirect all output to it (provided that the code that logs stuff uses
// our context!).
//
// If you don't specify a default logger function, the program uses the
// console to print the message.

struct string;
using Log_Function = std::function<void(const string &str)>;

// A function that gets called when an assert in the program is called.
// If you don't specify one in the context a default one is provided,
// which on failure prints the information to the console and stops the program.
//
// The message is the condition in string form. This allows the assert user to
// add extra information that will get printed, for example:
//
//      assert(index < size && "Index out of bounds.");
//
using Assert_Function = std::function<void(const char *file, int line, const char *condition)>;

// When allocating you should use the context's allocator
// This makes it so when users call your functions, they
// can specify an allocator beforehand by pushing a new context,
// without having to pass you anything as a parameter for example.
//
// The idea for this comes from the implicit context in Jai.
struct Implicit_Context {
    Allocator_Closure Allocator = MALLOC;

    Log_Function Log = print_string_to_console;
    Assert_Function AssertFailed = default_assert_failed;
};

// Immutable context available everywhere
// TODO: This should be thread local
inline Implicit_Context __context;

#define OLD_CONTEXT_VAR_GEN_(LINE) __game_utils_old_context_var##LINE
#define OLD_CONTEXT_VAR_GEN(LINE) OLD_CONTEXT_VAR_GEN_(LINE)

// This is a helper macro to safely modify the implicit context in a block of code.
// Usage:
//    PUSH_CONTEXT {
//        ... code with new context ...
//    }
//    ... old context is restored ...
//
// Don't pass a pointer as a parameter!
#define PUSH_CONTEXT(newContext)                                \
    Implicit_Context OLD_CONTEXT_VAR_GEN(__LINE__) = __context; \
    if (true) {                                                 \
        __context = newContext;                                 \
        goto body;                                              \
    } else                                                      \
        while (true)                                            \
            if (true) {                                         \
                __context = OLD_CONTEXT_VAR_GEN(__LINE__);      \
                break;                                          \
            } else                                              \
            body:

#define CONTEXT_ALLOC __context.Allocator

CPPU_END_NAMESPACE
