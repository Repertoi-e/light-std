#pragma once

//
// A header with common definitions and helper macros and functions
//

// By default everything is outside namespace, but if that's a problem for the
// user, provide a way to specify a namespace in which everything gets wrapped.
#if !defined GU_NAMESPACE_NAME
#define GU_BEGIN_NAMESPACE
#define GU_END_NAMESPACE
#else
#define GU_BEGIN_NAMESPACE namespace GU_NAMESPACE_NAME {
#define GU_END_NAMESPACE }
#endif

#include <stddef.h>

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
// This constant is used to represent an invalid index
// (e.g. the result of a search)
static constexpr size_t npos = (size_t) -1;

constexpr s32 _X64 = 1;
constexpr s32 _X86 = 2;

// Determine x86 or x64 architecture
#if defined _WIN64 || defined __x86_64__ || defined __ppc64__
constexpr s32 PROCESSOR = _X64;
#define PROCESSOR_X64

using ptr_t = s64;
using uptr_t = u64;
#else
constexpr s32 PROCESSOR = _X86;

using ptr_t = s32;
using uptr_t = u32;
#endif

#include <limits>

constexpr s32 _MSVC = 1;
constexpr s32 _CLANG = 2;
constexpr s32 _GCC = 3;

// Determine compiler, at the moment only MSVC, Clang or GCC are detected
#if defined(__clang__)
constexpr s32 COMPILER = _CLANG;
#define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
constexpr s32 COMPILER = _GCC;
#define COMPILER_GCC
#elif defined(_MSC_VER)
constexpr s32 COMPILER = _MSVC;
#define COMPILER_MSVC
#else
#warning Compiler not detected
#endif

constexpr s32 _WINDOWS = 1;
constexpr s32 _LINUX = 2;
constexpr s32 _MAC = 3;

#if defined __linux__
constexpr s32 OS = _LINUX;
#define OS_LINUX
#elif defined __APPLE__
constexpr s32 OS = _MAC;
#define OS_MAC
#elif defined _WIN32 || defined _WIN64
constexpr s32 OS = _WINDOWS;
#define OS_WINDOWS
#endif

// A type-safe compile-time function that returns the number of elements in an array
//
//    int arr[25];
//    constexpr size_t len = array_count(arr); // 25
//
GU_BEGIN_NAMESPACE
template <typename T, size_t n>
constexpr size_t array_count(const T (&)[n]) {
    return n;
}
GU_END_NAMESPACE

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

#if defined COMPILER_MSVC && defined GU_NO_CRT && !defined GU_FLTUSED_DEF
#define GU_FLTUSED_DEF
extern "C" {
int _fltused;
}
#endif

// Go-style defer macro
// Use:
//
//  defer {
//      statements; // Gets called on scope exit
//  };
//
GU_BEGIN_NAMESPACE

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
#define DEFER_INTERNAL_(LINE) __game_utils_defer_lambda_##LINE
#define DEFER_INTERNAL(LINE) DEFER_INTERNAL_(LINE)
#define defer auto DEFER_INTERNAL(__LINE__) = Defer_Dummy{} *[&]()
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

struct string;
void print_string_to_console(const string &str);

GU_END_NAMESPACE