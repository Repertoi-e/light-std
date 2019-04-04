#pragma once

/// A header which detects OS, cpu architecture and compiler

// OS constants
#define WINDOWS 1
#define MACOS 2
#define LINUX 3
#define ANDROID 4

#if defined linux || defined __linux || defined __linux__ || defined __GNU__ || defined __GLIBC__
#define OS LINUX
#define OS_STRING "Linux"
#elif defined _WIN32 || defined __WIN32__ || defined WIN32
#define OS WINDOWS
#define OS_STRING "Windows"
#elif defined macintosh || defined __APPLE__ || defined __APPLE_CC__
#define OS MACOS
#define OS_STRING "MacOS"
#elif defined __ANDROID__
#define OS ANDROID
#define OS_STRING "Android"
#else
#error Unrecognized platform
#endif


// Architecture defines
#if defined __pnacl__ || defined __CLR_VER
#define ARCH_VM
#endif
#if (defined _M_IX86 || defined __i386__) && !defined ARCH_VM
#define ARCH_X86_32
#endif
#if (defined _M_X64 || defined __x86_64__) && !defined ARCH_VM
#define ARCH_X86_64
#endif
#if defined ARCH_X86_32 || defined ARCH_X86_64
#define ARCH_X86
#endif
#if defined __arm__ || defined _M_ARM
#define ARCH_ARM
#endif
#if defined __aarch64__
#define ARCH_AARCH64
#endif
#if defined ARCH_AARCH64 || defined ARCH_ARM
#define ARCH_ANY_ARM
#endif
#if defined __mips64
#define ARCH_MIPS64
#endif
#if defined __mips__ && !defined __mips64  // mips64 also declares __mips__
#define ARCH_MIPS32
#endif
#if defined ARCH_MIPS32 || defined ARCH_MIPS64
#define ARCH_MIPS
#endif
#if defined __powerpc__
#define ARCH_PPC
#endif

#if defined ARCH_X86
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
#endif

#if defined ARCH_ANY_ARM
#define ANY_ARM_NEON defined __ARM_NEON__)
#endif
#if defined ARCH_MIPS
#define MIPS_MSA defined __mips_msa)
#endif

#if defined ARCH_X86_64 || defined ARCH_AARCH64 || defined ARCH_MIPS64 || defined __powerpc64__ || defined __ppc64__ 
#define BITS 64
#else
#define BITS 32
#endif

// Detect endianness
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321

#if defined __GLIBC__ || defined __GNU_LIBRARY__
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
