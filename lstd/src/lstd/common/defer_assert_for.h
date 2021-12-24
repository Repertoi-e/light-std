#pragma once

#include "type_info.h"

LSTD_BEGIN_NAMESPACE

#define ref const &
#define ref_volatile &&

// Used in macros to get "unique" variable names
#define LINE_NAME(name) _MACRO_CONCAT(name, __LINE__)
#define _MACRO_DO_CONCAT(s1, s2) s1##s2
#define _MACRO_CONCAT(s1, s2) _MACRO_DO_CONCAT(s1, s2)

// Go-style defer
//
//  defer(...);
//  defer({
//      ...;
//  });
//
// The statements inside get called on scope exit
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

#undef assert

#if !defined NDEBUG
#define assert(condition) (!!(condition)) ? (void) 0 : debug_break()
#else
#define assert(condition) ((void) 0)
#endif

template <typename T, typename TIter = decltype(types::declval<T>().begin()), typename = decltype(types::declval<T>().end())>
constexpr auto enumerate_impl(T ref in) {
    struct iterator {
        s64 I;
        TIter Iter;

        bool operator!=(const iterator &other) const { return Iter != other.Iter; }
        void operator++() { ++I, ++Iter; }

        struct dereference_result {
            s64 Index;
            decltype(*types::declval<TIter>()) Value;
        };

        auto operator*() const {
            return dereference_result{I, *Iter};
        }
    };

    struct iterable_wrapper {
        T Iterable;

        auto begin() { return iterator{0, Iterable.begin()}; }
        auto end() { return iterator{0, Iterable.end()}; }
    };

    return iterable_wrapper{(T &&) in};
}

// Shortcut macros for "for each" loops (really up to personal style if you want to use this)
//
//  For(array) print(it);
//
#define For_as(x, in) for (auto ref_volatile x : in)
#define For(in) For_as(it, in)

//
// Inspired from Python's enumerate().
// Example usage:
//
//    For_enumerate(data) {
//        other_data[it_index] = it + 1;
//    }
//
// .. which is the same as:
//
//    For(range(data.Count)) {
//        other_data[it] = data[it] + 1;
//    }
//
// Might not look much shorter but you don't a separate
// variable if you use data[it] more than once.
// It's just a convenience.
//
// You can change the names of the internal
// variables by using _For_enumerate_as_.
//
#define For_enumerate_as(it_index, it, in) for (auto [it_index, it] : LSTD_NAMESPACE::enumerate_impl(in))
#define For_enumerate(in) For_enumerate_as(it_index, it, in)

LSTD_END_NAMESPACE
