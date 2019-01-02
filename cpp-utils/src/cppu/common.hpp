#pragma once

//
// A header with common definitions and helper macros and functions
//

// @Temp
#define CPPU_NAMESPACE_NAME cppu

// By default everything is outside namespace, but if that's a problem for the
// user, provide a way to specify a namespace in which everything gets wrapped.
#if !defined CPPU_NAMESPACE_NAME
#define CPPU_BEGIN_NAMESPACE
#define CPPU_END_NAMESPACE
#define CPPU_NAMESPACE_NAME
#else
#define CPPU_BEGIN_NAMESPACE namespace CPPU_NAMESPACE_NAME {
#define CPPU_END_NAMESPACE }
#endif

#include <stddef.h>
#include <algorithm>
#include <limits>
#include <utility>

using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;

using byte = u8;

using f32 = float;
using f64 = double;

constexpr auto null = nullptr;

// Note that we assume we are on x64 architecture.
// For now we don't support 32 bit.
using ptr_t = s64;
using uptr_t = u64;
using size_t = uptr_t;

// This constant is used to represent an invalid index
// (e.g. the result of a search)
static constexpr size_t npos = (size_t) -1;

#define MSVC 1
#define CLANG 2
#define GCC 3

// Determine compiler, at the moment only MSVC, Clang or GCC are detected
#if defined(__clang__)
#define COMPILER CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER GCC
#elif defined(_MSC_VER)
#define COMPILER MSVC
#else
#warning Compiler not detected
#endif

#define WINDOWS 1
#define LINUX 2
#define MAC 3

#if defined CPPU_PLATFORM_LINUX
#define OS LINUX
#elif defined CPPU_PLATFORM_MAC
#define OS MAC
#elif CPPU_PLATFORM_WINDOWS
#define OS WINDOWS
#else
#error Platform not set
#endif

// A type-safe compile-time function that returns the number of elements in an array
//
//    int arr[25];
//    constexpr size_t len = array_count(arr); // 25
//
CPPU_BEGIN_NAMESPACE
template <typename T, size_t n>
constexpr size_t array_count(const T (&)[n]) {
    return n;
}
CPPU_END_NAMESPACE

// Convenience storage literal operators, allows for specifying sizes like this:
//
//    size_t mySize = 10MiB;
//    mySize = 10GiB;
//
inline constexpr size_t operator"" _B(u64 i) {
    return (size_t)(i);  // For completeness
}

inline constexpr size_t operator"" _KiB(u64 i) { return (size_t)(i) << 10; }
inline constexpr size_t operator"" _MiB(u64 i) { return (size_t)(i) << 20; }
inline constexpr size_t operator"" _GiB(u64 i) { return (size_t)(i) << 30; }

// Go-style defer macro
// Use:
//
//  defer {
//      statements; // Gets called on scope exit
//  };
//
CPPU_BEGIN_NAMESPACE

#ifndef defer
struct Defer_Dummy {};
template <typename F>
struct Deferrer {
    F func;
    ~Deferrer() { func(); }
};
template <typename F>
Deferrer<F> operator*(Defer_Dummy, F func) {
    return {func};
}
#define DEFER_INTERNAL_(LINE) CPPU_NAMESPACE_NAME##_cppu_defer##LINE
#define DEFER_INTERNAL(LINE) DEFER_INTERNAL_(LINE)
#define defer auto DEFER_INTERNAL(__LINE__) = Defer_Dummy{} *[&]()
#endif

#undef assert

#ifdef CPPU_DEBUG
#define assert(condition) (!!(condition)) ? (void) 0 : Context.AssertFailed(__FILE__, __LINE__, u8## #condition)
#else
#define assert(condition) ((void) 0)
#endif

using std::max;
using std::min;

// Platform specific utily functions:
// Implementations in *platform*.cpp

// Returns the elapsed real time in seconds, platform-specific implementation
f64 os_get_wallclock_in_seconds();

extern void os_exit_program(int code);

// A default failed assert callback that logs a message and stops the program
void os_assert_failed(const char *file, int line, const char *condition);

class NonCopyable {
   protected:
    NonCopyable() {}
    ~NonCopyable() {}

    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

class NonMovable {
   protected:
    NonMovable() {}
    ~NonMovable() {}

    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
};

// Shortcut macros for foreach loops (really up to personal style if you use this)
#define For_as(x, in) for (const auto &x : in)
#define For(in) For_as(it, in)

CPPU_END_NAMESPACE