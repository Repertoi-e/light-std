#pragma once

/**
 * @file platform.h
 * @brief Detects the OS, CPU architecture, endianness, and the compiler.
 *
 * Example usage:
 * @code
 * #if OS == WINDOWS
 * ...
 * #endif
 *
 * #if BITS == 64 && ENDIAN == LITTLE_ENDIAN
 * ...
 * #endif
 *
 * #if COMPILER == MSVC
 * ...
 * #endif
 * @endcode
 */

// OS constants
#define WINDOWS 1
#define MACOS 2
#define LINUX 3
#define NO_OS 5

#if defined LSTD_NO_OS
#define OS NO_OS
#endif

#ifndef OS
#if defined linux || defined __linux || defined __linux__ || \
    defined __GNU__ || defined __GLIBC__
#define OS LINUX
#define OS_STRING "Linux"
#elif defined _WIN32 || defined __WIN32__ || defined WIN32
#define OS WINDOWS
#define OS_STRING "Windows"
#elif defined macintosh || defined __APPLE__ || defined __APPLE_CC__
#define OS MACOS
#define OS_STRING "MacOS"
#else
#define OS NO_OS
#endif
#endif

#if OS == NO_OS
#ifndef OS_STRING
#define OS_STRING "NoOS"
#endif
#endif

// Architecture defines
#define VM 1
#define X86 2
#define ARM 3
#define MIPS 4
#define PPC 5

#if defined __pnacl__ || defined __CLR_VER
#define ARCH VM
#elif defined _M_X64 || defined __x86_64__ || defined _M_IX86 || \
    defined __i386__
#define ARCH X86
#elif defined __arm__ || defined _M_ARM || __aarch64__
#define ARCH ARM
#elif defined __mips__ || defined __mips64
#define ARCH MIPS
#elif defined __powerpc__
#define ARCH PPC
#endif

#if ARCH == X86
#define X86_AES defined __AES__
#define X86_F16C defined __F16C__
#define X86_BMI defined __BMI__
#define X86_BMI2 defined __BMI2__
#define X86_SSE (defined __SSE__ || (_M_IX86_FP >= 1))
#define X86_SSE2 (defined __SSE2__ || (_M_IX86_FP >= 2))
#define X86_SSE3 defined __SSE3__
#define X86_SSSE3 defined __SSSE3__
#define X86_SSE4_1 defined __SSE4_1__
#define X86_SSE4_2 defined __SSE4_2__
#define X86_AVX defined __AVX__
#define x86_AVX2 defined __AVX2__
#elif ARCH == ARM
#define ANY_ARM_NEON defined __ARM_NEON__)
#elif ARCH == MIPS
#define MIPS_MSA defined __mips_msa)
#endif

#if defined _M_X64 || defined __x86_64__ || defined __aarch64__ || \
    defined __mips64 || defined __powerpc64__ || defined __ppc64__
#define BITS 64
#else
#define BITS 32
#endif

#define POINTER_SIZE (BITS / 8)

// Detect endianness
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#endif

#if OS == LINUX
#include <endian.h>
#else
#if OS == MACOS
#include <machine/endian.h>
#else
#endif
#endif

#if defined __BYTE_ORDER
#if defined __BIG_ENDIAN && (__BYTE_ORDER == __BIG_ENDIAN)
#define ENDIAN BIG_ENDIAN
#endif
#if defined __LITTLE_ENDIAN && (__BYTE_ORDER == __LITTLE_ENDIAN)
#define ENDIAN LITTLE_ENDIAN
#endif
#endif
#if !defined __BYTE_ORDER && defined _BYTE_ORDER
#if defined _BIG_ENDIAN && (_BYTE_ORDER == _BIG_ENDIAN)
#define ENDIAN BIG_ENDIAN
#endif
#if defined _LITTLE_ENDIAN && (_BYTE_ORDER == _LITTLE_ENDIAN)
#define ENDIAN LITTLE_ENDIAN
#endif
#endif

// Windows is always little-endian.
#if !defined ENDIAN
#if OS == WINDOWS
#define ENDIAN LITTLE_ENDIAN
#endif
#endif  

#if !defined ENDIAN
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#define ENDIAN BIG_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#define ENDIAN LITTLE_ENDIAN
#endif
#endif

#if !defined ENDIAN
#error Endianness not detected
#endif

// Compiler constants
#define MSVC 1
#define CLANG 2
#define GCC 3

#if defined __clang__
#define COMPILER CLANG
#define COMPILER_STRING "Clang/LLVM"
#elif defined __GNUC__ || defined __GNUG__
#define COMPILER GCC
#define COMPILER_STRING "GCC"
#elif defined _MSC_VER
#define COMPILER MSVC
#define COMPILER_STRING "MSVC"
#else
#warning Compiler not detected
#endif

#if COMPILER == MSVC
// These macros are used to aid the compiler at certain optimizations.
#define always_inline __forceinline
#define never_inline __declspec(noinline)
#define no_vtable __declspec(novtable)
#define no_alias __declspec(noalias)
#define restrict __declspec(restrict)
#else
#define always_inline inline
#define never_inline __attribute__((noinline))
#define no_vtable __attribute__((__type__(no_table)))
#define no_alias __restrict
#define restrict __restrict
#endif
