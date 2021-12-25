#pragma once

//
// A header which defines common types, numeric info,
// including bits operations, atomic operations, common math functions,
//     definitions for the macros: assert, defer, For, For_enumerate ...
//     static_for, range,
// Also:
//     copy_memory (memcpy), copy_memory_const, fill_memory (memset), zero_memory
//
//

#include "common/atomic.h"
#include "common/bits.h"
#include "common/context.h"
#include "common/debug_break.h"
#include "common/defer_assert_for.h"
#include "common/fmt.h"
#include "common/math.h"
#include "common/memory.h"
#include "common/namespace.h"
#include "common/numeric_info.h"
#include "common/sequence.h"
#include "common/u128.h"

//
// Provides replacements for the math functions found in virtually all standard libraries.
// Also provides functions for extended precision arithmetic, statistical functions, physics, astronomy, etc.
// https://www.netlib.org/cephes/
// Note: We don't include everything, just cmath for now.
//       Statistics is a thing we will most definitely include as well in the future.
//       Everything else you can include on your own in your project (we don't want to be bloat-y).
//
// Note: Important difference,
// atan2's return range is 0 to 2PI, and not -PI to PI (as per normal in the C standard library).
//
//
// Parts of the source code that we modified are marked with :WEMODIFIEDCEPHES:
//

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1995, 2000 by Stephen L. Moshier
*/
#include "vendor/cephes/maths_cephes.h"

#define PI 3.1415926535897932384626433832795
#define TAU 6.283185307179586476925286766559

// Convenience storage literal operators, allows for specifying sizes like this:
//  s64 a = 10_MiB;

// _B For completeness
constexpr u64 operator"" _B(u64 i) { return i; }
constexpr u64 operator"" _KiB(u64 i) { return i << 10; }
constexpr u64 operator"" _MiB(u64 i) { return i << 20; }
constexpr u64 operator"" _GiB(u64 i) { return i << 30; }

constexpr u64 operator"" _thousand(u64 i) { return i * 1000; }
constexpr u64 operator"" _million(u64 i) { return i * 1000000; }
constexpr u64 operator"" _billion(u64 i) { return i * 1000000000; }

LSTD_BEGIN_NAMESPACE

// Loop that gets unrolled at compile-time
template <s64 First, s64 Last, typename Lambda>
void static_for(Lambda ref f) {
    if constexpr (First < Last) {
        f(types::integral_constant<s64, First>{});
        static_for<First + 1, Last>(f);
    }
}

// @Volatile: README.md
// :TypePolicy:
//
// Ideal: keep it simple.
//
// - Keep it data oriented.
//   Programs work with data. Design your data so it makes the solution straightforward and minimize abstraction layers.
// - Use struct instead of class, keep everything public.
// - Always provide a default constructor (implicit or by "T() {}")
// - copy/move constructors and destructors are banned. No excuse.
// - No throwing of exceptions, .. ever, .. anywhere. No excuse.
//   They make your code complicated. When you can't handle an error and need to exit from a function, return multiple values.
//   C++ doesn't really help with this, but you can use C++17 structured bindings, e.g.:
//          auto [content, success] = path_read_entire_file("data/hello.txt");
//
//
// Some examples:
//
// _array_ is this library implemented the following way:
//     _array_ is a struct that contains 2 fields (Data and Count).
//
//     It has no sense of ownership. That is determined explictly in code and by the programmer.
//
//     By default arrays are views, to make them dynamic, call     make_dynamic(&arr).
//     After that you can modify them (add/remove elements etc.)
//
//     You can safely pass around copies and return arrays from functions because
//     there is no hidden destructor which will free the memory.
//
//     When a dynamic array is no longer needed call    free(arr.Data);
//
//
//     We provide a defer macro which runs at the end of the scope (like a destructor),
//     you can use this for functions which return from multiple places,
//     so you are absolutely sure    free(arr.Data) is ran and there were no leaks.
//
//
//     All of this allows to skip writing copy/move constructors/assignment operators.
//
// _string_s are just array<char>. All of this applies to them as well.
// They are not null-terminated, which means that taking substrings doesn't allocate memory.
//
//
//     // Constructed from a zero-terminated string buffer. Doesn't allocate memory.
//     // Like arrays, strings are views by default.
//     string path = "./data/";
//     make_dynamic(&path);         // Allocates a buffer and copies the string it was pointing to
//     defer(free(path.Data));
//
//     append(&path, "output.txt");
//
//     string pathWithoutDot = substring(path, 2, -1);
//
// To make a deep copy of an array use clone().
// e.g.         string newPath = clone(path); // Allocates a new buffer and copies contents in _path_
//

template <typename T>
constexpr void swap(T &a, T &b) {
    T c = a;
    a   = b;
    b   = c;
}

template <typename T, s64 N>
constexpr void swap(T (&a)[N], T (&b)[N]) {
    For(range(N)) swap(a[it], b[it]);
}

//
// Optimized implementations of copy_memory, fill_memory, compare_memory even on Debug.
// See context/internal.cpp
//
// SSE implementations are available when on x86 architecture.
//

extern void *(*copy_memory_fast)(void *dst, const void *src, u64 size);

template <typename T>
constexpr T *copy_elements(T *dst, auto *src, s64 n) {
    assert(sizeof(*dst) == sizeof(*src));

    u64 to   = (u64) dst / sizeof(*dst);
    u64 from = (u64) src / sizeof(*src);

    if (to > from && (s64) (to - from) < n) {
        // If overlapping in this way:
        //   [from......]
        //         [to........]
        // copy in reverse.
        For(range(n - 1, -1, -1)) dst[it] = src[it];
    } else {
        // Otherwise copy forwards..
        For(range(n)) dst[it] = src[it];
    }
    return dst;
}

template <typename T>
constexpr T *copy_memory(T *dst, auto *src, s64 bytes) {
    // This applies to runtime code.
    // Because is_constant_evaluated() is not if constexpr, code in that branch
    // still tries to compile (even though it may never run in constexpr), which
    // causes errors when trying to do sizeof(void)... so we have to impose
    // this limitation to all callers.
    static_assert(!types::is_same<T, void>, "C++ doesn't allow type pruning in constexpr context. Call with char* instead of void* to fix this.");
    // You can also call copy_memory_fast to bypass casting...

    if (is_constant_evaluated()) {
        return copy_elements(dst, src, bytes / sizeof(T));
    } else {
        return (T *) copy_memory_fast(dst, src, bytes);
    }
}

extern void *(*fill_memory_fast)(void *dst, char value, u64 size);

template <typename T>
constexpr T *fill_elements(T *dst, T value, u64 n) {
    For(range(n)) {
        dst[it] = value;
    }
    return dst;
}

constexpr char *fill_memory(char *dst, char value, u64 bytes) {
    if (is_constant_evaluated()) {
        return fill_elements(dst, value, bytes);
    } else {
        return (char *) fill_memory_fast(dst, value, bytes);
    }
}

// Constexpr doesn't work with void* ...
inline void *fill_memory(void *dst, char value, u64 bytes) { return fill_memory_fast(dst, value, bytes); }

constexpr char *zero_memory(char *dst, u64 n) { return fill_memory(dst, 0, n); }

// Constexpr doesn't work with void* ...
inline void *zero_memory(void *dst, u64 n) { return fill_memory(dst, 0, n); }

extern s32 (*compare_memory_fast)(const void *s1, const void *s2, u64 size);

template <typename T>
constexpr s32 compare_elements(const T *s1, const T *s2, s64 n) {
    For(range(n)) {
        if (*s1 != *s2) return *s1 - *s2;
        ++s1, ++s2;
    }
    return 0;
}

template <typename T>
constexpr s32 compare_memory(const T *s1, const T *s2, s64 bytes) {
    // This applies to runtime code.
    // Because is_constant_evaluated() is not if constexpr, code in that branch
    // still tries to compile (even though it may never run in constexpr), which
    // causes errors when trying to do sizeof(void)... so we have to impose
    // this limitation to all callers.
    static_assert(!types::is_same<T, void>, "C++ doesn't allow type pruning in constexpr context. Call with char* instead of void* to fix this.");
    // You can also call compare_memory_fast to bypass casting...

    if (is_constant_evaluated() && !types::is_same<T, void>) {
        return compare_elements(s1, s2, bytes / sizeof(T));
    } else {
        // If constexpr fails here, don't use void* for s1 and s2.
        // C++ doesn't allow type pruning in constexpr context...

        return compare_memory_fast(s1, s2, bytes);
    }
}

LSTD_END_NAMESPACE
