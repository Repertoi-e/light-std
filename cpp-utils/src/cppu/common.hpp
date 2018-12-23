#pragma once

//
// A header with common definitions and helper macros and functions
//

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

using b32 = s32;

constexpr auto null = nullptr;

// Note that we assume we are on x64 architecture.
// For now we don't support 32 bit.
using ptr_t = s64;
using uptr_t = u64;
using size_t = uptr_t;

// This constant is used to represent an invalid index
// (e.g. the result of a search)
static constexpr size_t npos = (size_t) -1;

constexpr s32 MSVC = 1;
constexpr s32 CLANG = 2;
constexpr s32 GCC = 3;

// Determine compiler, at the moment only MSVC, Clang or GCC are detected
#if defined(__clang__)
constexpr s32 COMPILER = CLANG;
#define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
constexpr s32 COMPILER = GCC;
#define COMPILER_GCC
#elif defined(_MSC_VER)
constexpr s32 COMPILER = MSVC;
#define COMPILER_MSVC
#else
#warning Compiler not detected
#endif

constexpr s32 WINDOWS = 1;
constexpr s32 LINUX = 2;
constexpr s32 MAC = 3;

#if defined CPPU_PLATFORM_LINUX
constexpr s32 OS = LINUX;
#define OS_LINUX
#elif defined CPPU_PLATFORM_MAC
constexpr s32 OS = MAC;
#define OS_MAC
#elif CPPU_PLATFORM_WINDOWS
constexpr s32 OS = WINDOWS;
#define OS_WINDOWS
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
#define defer_internal_(LINE) __game_utils_defer_lambda_##LINE
#define defer_internal(LINE) defer_internal_(LINE)
#define defer auto defer_internal(__LINE__) = Defer_Dummy{} *[&]()
#endif

#undef assert

#ifdef NDEBUG
#define assert(condition) ((void) 0)
#else
#define assert(condition) (!!(condition)) ? (void) 0 : __context.AssertFailed(__FILE__, __LINE__, u8## #condition)
#endif

template <typename T>
constexpr const T &Min(const T &a, const T &b) {
    return (b < a) ? b : a;
}

template <typename T>
constexpr const T &Max(const T &a, const T &b) {
    return (a < b) ? b : a;
}

// Platform specific utily functions:
// Implementations in *platform*.cpp

// Returns the elapsed real time in seconds, platform-specific implementation
f64 get_wallclock_in_seconds();

// Pauses the program and waits for a user key press.
void wait_for_input(b32 message = true);

extern void exit_program(int code);

// A default failed assert callback that logs a message and stops the program
void default_assert_failed(const char *file, int line, const char *condition);

CPPU_END_NAMESPACE