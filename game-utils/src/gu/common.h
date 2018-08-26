#pragma once

//
// A header with common definitions and helper macros and functions
//

// By default everything is outside namespace, but if that's a problem for the
// user, provide a way to specify a namespace in which everything gets wrapped.
#if !defined GU_BEGIN_NAMESPACE
#define GU_BEGIN_NAMESPACE 
#define GU_END_NAMESPACE   
#else
#if !defined GU_END_NAMESPACE
#warning GU_BEGIN_NAMESPACE defined, but GU_END_NAMESPACE is not
#endif
#endif

#include <stddef.h>

using s8  = char;
using s16 = short;
using s32 = int;
using s64 = long long;

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;

using byte = u8;

using f32 = float;
using f64 = double;

using b32 = s32;

// Determine x86 or x64 architecture
#if defined _WIN64 || defined __x86_64__ || defined __ppc64__
constexpr b32 PROCESSOR_x64 = true;
constexpr b32 PROCESSOR_x86 = false;

using ptr_t  = s64;
using uptr_t = u64;
#else
constexpr b32 PROCESSOR_x64 = false;
constexpr b32 PROCESSOR_x86 = true;

using ptr_t  = s32;
using uptr_t = u32;
#endif

#include <limits>

// Determine compiler, at the moment only MSVC, Clang or GCC are detected
#if defined(__clang__)
constexpr b32 COMPILER_MSVC  = false;
constexpr b32 COMPILER_CLANG = true;
constexpr b32 COMPILER_GCC   = false;
#define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
constexpr b32 COMPILER_MSVC  = false;
constexpr b32 COMPILER_CLANG = false
constexpr b32 COMPILER_GCC   = true;
#define COMPILER_GCC
#elif defined(_MSC_VER)
constexpr b32 COMPILER_MSVC  = true;
constexpr b32 COMPILER_CLANG = false;
constexpr b32 COMPILER_GCC   = false;
#define COMPILER_MSVC
#else
#warning Compiler not detected
#endif


#if defined __linux__
constexpr b32 OS_LINUX   = true;
constexpr b32 OS_MAC     = false;
constexpr b32 OS_WINDOWS = false;
#define OS_LINUX
#elif defined __APPLE__
constexpr b32 OS_LINUX   = false;
constexpr b32 OS_MAC     = true;
constexpr b32 OS_WINDOWS = false;
#define OS_MAC
#elif defined _WIN32 || defined _WIN64
constexpr b32 OS_LINUX   = false;
constexpr b32 OS_MAC     = false;
constexpr b32 OS_WINDOWS = true;
#define OS_WINDOWS
#endif


// A type-safe compile-time function that returns the number of elements in an array
//
//    int arr[25];
//    constexpr size_t len = array_count(arr); // 25
//
GU_BEGIN_NAMESPACE
template <typename T, size_t n> 
constexpr size_t array_count(const T(&)[n]) { 
    return n; 
}
GU_END_NAMESPACE

#if defined COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wno-literal-suffix"
#elif defined COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuser-defined-literals"
#else
#pragma warning(push)
#pragma warning(disable: 4455)
#endif

// Convenience storage literal operators, allows for specifying sizes like this:
//    size_t mySize = 10MiB;
inline constexpr size_t operator"" _B(u64 i) {
    return (size_t) (i); // For completeness 
}

inline constexpr size_t operator"" _KiB(u64 i) {
    return (size_t) (i) << 10;
}

inline constexpr size_t operator"" _MiB(u64 i) {
    return (size_t) (i) << 20;
}

inline constexpr size_t operator"" _GiB(u64 i) {
    return (size_t) (i) << 30;
}

#if COMPILER_GCC
#pragma GCC diagnostic pop
#elif defined COMPILER_CLANG
#pragma clang diagnostic pop
#else
#pragma warning(pop)
#endif

#undef assert

#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) ((condition) ? (void)0 : __context->AssertFailed(__FILE__, __LINE__, #condition))
#endif 


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
template <class F> struct Deferrer { F func; ~Deferrer() { func(); } };
template <class F> Deferrer<F> operator*(Defer_Dummy, F func) { return { func }; }
#define DEFER_(LINE) __game_utils_defer_lambda_##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = Defer_Dummy{} * [&]()
#endif

template <typename T>
inline T const& Min(T const &a, T const &b) {
    return (b < a) ? b : a;
}

template <typename T>
inline T const& Max(T const &a, T const &b) {
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
void default_failed_assert(const char *file, int line, const char *failedCondition);

GU_END_NAMESPACE