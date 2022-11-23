#pragma once

#include "namespace.h"

LSTD_BEGIN_NAMESPACE

// LINE_NAME appends the line number to x, used in macros to get "unique" variable names.
#define LINE_NAME(name) _MACRO_CONCAT(name, __LINE__)
#define _MACRO_DO_CONCAT(s1, s2) s1##s2
#define _MACRO_CONCAT(s1, s2) _MACRO_DO_CONCAT(s1, s2)

//
// Go-style defer statement
//
//  defer(...);
//  defer({
//      ...;
//  });
//
// The statements inside get called on scope exit.
//
#undef defer

struct Defer_Dummy {};
template <typename F>
struct Deferrer {
    F Func;

    // We don't call destructors in free() (take a look at context.h), but they still work.
    // In this case to rely on them to implement defer. This gets called on a stack variable anyway.
    ~Deferrer() { Func(); }
};
template <typename F>
Deferrer<F> operator*(Defer_Dummy, F func) {
    return {func};
}

#define defer(x) auto LINE_NAME(LSTD_defer) = LSTD_NAMESPACE::Defer_Dummy{} * [&]() { x; }

LSTD_END_NAMESPACE
