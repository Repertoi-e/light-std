#pragma once

//
// Define LSTD_NAMESPACE as a preprocessor definition to the value you want the
// namespace to be called. (By default the libary has the namespace "lstd").
//
// * If you want to use this library without a namespace,
// define LSTD_NO_NAMESPACE when building
//

#if not defined LSTD_NO_NAMESPACE && defined LSTD_NAMESPACE
#define LSTD_BEGIN_NAMESPACE \
    namespace LSTD_NAMESPACE \
    {
#define LSTD_END_NAMESPACE }
#define LSTD_USING_NAMESPACE using namespace LSTD_NAMESPACE
#else
#define LSTD_NAMESPACE
#define LSTD_BEGIN_NAMESPACE
#define LSTD_END_NAMESPACE
#define LSTD_USING_NAMESPACE
#endif

/**
 * Detects the OS, CPU architecture, endianness, and the compiler.
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
    defined(__BIG_ENDIAN__) ||                               \
    defined(__ARMEB__) ||                                    \
    defined(__THUMBEB__) ||                                  \
    defined(__AARCH64EB__) ||                                \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#define ENDIAN BIG_ENDIAN
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) ||                                 \
    defined(__ARMEL__) ||                                         \
    defined(__THUMBEL__) ||                                       \
    defined(__AARCH64EL__) ||                                     \
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

//
// Defines the debug_break() function for various platforms.
//

/* Copyright (c) 2011-2018, Scott Tsai
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef _MSC_VER

#define debug_break __debugbreak

#else

#ifdef __cplusplus
extern "C"
{
#endif

#define DEBUG_BREAK_USE_TRAP_INSTRUCTION 1
#define DEBUG_BREAK_USE_BULTIN_TRAP 2
#define DEBUG_BREAK_USE_SIGTRAP 3

#if defined(__i386__) || defined(__x86_64__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
    __inline__ static void trap_instruction(void)
    {
        __asm__ volatile("int $0x03");
    }
#elif defined(__thumb__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
/* FIXME: handle __THUMB_INTERWORK__ */
__inline__ static void trap_instruction(void)
{
    /* See 'arm-linux-tdep.c' in GDB source.
     * Both instruction sequences below work. */
#if 1
    /* 'eabi_linux_thumb_le_breakpoint' */
    __asm__ volatile(".inst 0xde01");
#else
    /* 'eabi_linux_thumb2_le_breakpoint' */
    __asm__ volatile(".inst.w 0xf7f0a000");
#endif

    /* Known problem:
     * After a breakpoint hit, can't 'stepi', 'step', or 'continue' in GDB.
     * 'step' would keep getting stuck on the same instruction.
     *
     * Workaround: use the new GDB commands 'debugbreak-step' and
     * 'debugbreak-continue' that become available
     * after you source the script from GDB:
     *
     * $ gdb -x debugbreak-gdb.py <... USUAL ARGUMENTS ...>
     *
     * 'debugbreak-step' would jump over the breakpoint instruction with
     * roughly equivalent of:
     * (gdb) set $instruction_len = 2
     * (gdb) tbreak *($pc + $instruction_len)
     * (gdb) jump   *($pc + $instruction_len)
     */
}
#elif defined(__arm__) && !defined(__thumb__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__inline__ static void trap_instruction(void)
{
    /* See 'arm-linux-tdep.c' in GDB source,
     * 'eabi_linux_arm_le_breakpoint' */
    __asm__ volatile(".inst 0xe7f001f0");
    /* Known problem:
     * Same problem and workaround as Thumb mode */
}
#elif defined(__aarch64__) && defined(__APPLE__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_BULTIN_DEBUGTRAP
#elif defined(__aarch64__)
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__inline__ static void trap_instruction(void)
{
    /* See 'aarch64-tdep.c' in GDB source,
     * 'aarch64_default_breakpoint' */
    __asm__ volatile(".inst 0xd4200000");
}
#elif defined(__powerpc__)
/* PPC 32 or 64-bit, big or little endian */
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__inline__ static void trap_instruction(void)
{
    /* See 'rs6000-tdep.c' in GDB source,
     * 'rs6000_breakpoint' */
    __asm__ volatile(".4byte 0x7d821008");

    /* Known problem:
     * After a breakpoint hit, can't 'stepi', 'step', or 'continue' in GDB.
     * 'step' stuck on the same instruction ("twge r2,r2").
     *
     * The workaround is the same as ARM Thumb mode: use debugbreak-gdb.py
     * or manually jump over the instruction. */
}
#elif defined(__riscv)
/* RISC-V 32 or 64-bit, whether the "C" extension
 * for compressed, 16-bit instructions are supported or not */
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_TRAP_INSTRUCTION
__inline__ static void trap_instruction(void)
{
    /* See 'riscv-tdep.c' in GDB source,
     * 'riscv_sw_breakpoint_from_kind' */
    __asm__ volatile(".4byte 0x00100073");
}
#else
#define DEBUG_BREAK_IMPL DEBUG_BREAK_USE_SIGTRAP
#endif

#ifndef DEBUG_BREAK_IMPL
#error "debugbreak.h is not supported on this target"
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_TRAP_INSTRUCTION
__inline__ static void debug_break(void)
{
    trap_instruction();
}
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_BULTIN_DEBUGTRAP
__inline__ static void debug_break(void)
{
    __builtin_debugtrap();
}
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_BULTIN_TRAP
__inline__ static void debug_break(void)
{
    __builtin_trap();
}
#elif DEBUG_BREAK_IMPL == DEBUG_BREAK_USE_SIGTRAP
#include <signal.h>
__inline__ static void debug_break(void)
{
    raise(SIGTRAP);
}
#else
#error "invalid DEBUG_BREAK_IMPL value"
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifdef _MSC_VER */

//
// Defines the normal debug assert you'd expect to see.
//
#undef assert

#if !defined NDEBUG
#define assert(condition) (!!(condition)) ? (void)0 : debug_break()
#else
#define assert(condition) ((void)0)
#endif

//
// Some personal preferences:
//
// I prefer to type null over null but they are exactly the same
using null_t = decltype(nullptr);
inline const null_t null = nullptr;

#ifndef NULL
#define NULL 0
#endif

#ifndef U64_MAX
#define U64_MAX 18446744073709551615ULL
#endif

//
// The following integral types are defined here:
//      s8, s16, s32, s64, s128,
//      ... and corresponding unsigned types: u8, u16, u32, u64, u128,
//      vector types (aligned on 16 byte boundaries for SIMDs):
//          u8v16, u16v8, u32v4, u64v2, s8v16, s16v8, s32v4, s64v2, f32v4, f64v2
//		f32 (float), f64 (double), wchar (for Windows),
//      code_point (for the integer value of a Unicode code point),
//      and byte (unsigned char)
//
// Note: We don't support long doubles (lf64) or operations with them throughout
// the library.
//

//
// Fundamental types:
//
using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;

using wchar = wchar_t; // Only useful for Windows calls. Please don't use
                       // utf-16 in your programs...

using code_point =
    char32_t; // Holds the integer value of a Unicode code point.

using byte = unsigned char;

using f32 = float;
using f64 = double;

//
// Vector types (aligned on 16 byte boundaries for SIMDs)
//
template <typename T, s64 Count>
union alignas(16) base_vector_type
{
    T Values[Count];
};

using u8v16 = base_vector_type<u8, 16>;
using u16v8 = base_vector_type<u16, 8>;
using u32v4 = base_vector_type<u32, 4>;
using u64v2 = base_vector_type<u64, 2>;

using s8v16 = base_vector_type<s8, 16>;
using s16v8 = base_vector_type<s16, 8>;
using s32v4 = base_vector_type<s32, 4>;
using s64v2 = base_vector_type<s64, 2>;
using f32v4 = base_vector_type<f32, 4>;
using f64v2 = base_vector_type<f64, 2>;

//
// Convenience storage literal operators, allows for specifying sizes like this:
//  s64 a = 10_MiB;
//  s64 a = 20_billion;
//
// The International Electronic Commission established the term kibibyte
// for 1024 bytes, because the metric system already has a use for the
// prefix "kilo" meaning a thousand. So 1 KB = 1000 bytes.
//
// In practice, however, when buying storage or downloading files or
// looking in Windows explorer, KB has the meaning of 1024 bytes.
// Suddenly switching to KiB for no particular reason (except having
// the feeling of being correct or superior that you know a term
// which others don't) would cause confusion to users.
//
// However, we are programmers. And in general the decision of the IEC
// does sound logical. So for the sake of being exact we will name
// these literals with the proper term.
//
//
// _B is for completeness, really useless though
constexpr u64 operator"" _B(u64 i) { return i; }
constexpr u64 operator"" _KiB(u64 i) { return i << 10; }
constexpr u64 operator"" _MiB(u64 i) { return i << 20; }
constexpr u64 operator"" _GiB(u64 i) { return i << 30; }

constexpr u64 operator"" _thousand(u64 i) { return i * 1000; }
constexpr u64 operator"" _million(u64 i) { return i * 1000000; }
constexpr u64 operator"" _billion(u64 i) { return i * 1000000000; }

//
// Intrinsic-like s128 and u128 types:
//

struct s128;

//
// u128
//
// An unsigned 128-bit integer type. The API is meant to mimic an intrinsic
// type as closely as is practical, including exhibiting undefined behavior in
// analogous cases (e.g. division by zero). This type is intended to be a
// drop-in replacement once C++ supports an intrinsic `uint128_t` type; when
// that occurs, existing well-behaved uses of `u128` will continue to work
// using that new type.
//
// However, a `u128` differs from intrinsic integral types in the following
// ways:
//
//   * Errors on implicit conversions that do not preserve value (such as
//     loss of precision when converting to float values).
//   * Requires explicit construction from and conversion to floating point
//     types.
//   * Conversion to integral types requires an explicit static_cast() to
//     mimic use of the `-Wnarrowing` compiler flag.
//   * The alignment requirement of `u128` may differ from that of an
//     intrinsic 128-bit integer type depending on platform and build
//     configuration.
//
// Example:
//
//     f32 y = u128(17);  // Error. u128 cannot be implicitly
//                        // converted to float.
//
//     u128 v;
//     u64 i = v;        // Error
//     u64 i = (u64) v;  // OK
//
struct alignas(16) u128
{
#if ENDIAN == LITTLE_ENDIAN
    u64 lo;
    u64 hi;
#elif ENDIAN == BIG_ENDIAN
    u64 hi;
    u64 lo;
#else
#error "Unsupported byte order: must be little or big endian."
#endif
    constexpr u128() {}

    //
    // Constructors from arithmetic types
    //
#if ENDIAN == LITTLE_ENDIAN
    constexpr u128(u64 high, u64 low) : lo{low}, hi{high} {}
    constexpr u128(s32 v) : lo{(u64)(v)}, hi{v < 0 ? U64_MAX : 0} {}
    constexpr u128(s64 v) : lo{(u64)(v)}, hi{v < 0 ? U64_MAX : 0} {}
    constexpr u128(u32 v) : lo{v}, hi{0} {}
    constexpr u128(u64 v) : lo{v}, hi{0} {}
#elif ENDIAN == BIG_ENDIAN
    constexpr u128(u64 high, u64 low) : hi{high}, lo{low} {}
    constexpr u128(s32 v) : hi{v < 0 ? U64_MAX : 0}, lo{(u64)(v)} {}
    constexpr u128(s64 v) : hi{v < 0 ? U64_MAX : 0}, lo{(u64)(v)} {}
    constexpr u128(u32 v) : hi{0}, lo{v} {}
    constexpr u128(u64 v) : hi{0}, lo{v} {}
#else
#error "Unsupported byte order: must be little or big endian."
#endif
    constexpr u128(s128 v);
    constexpr explicit u128(float v);
    constexpr explicit u128(double v);

    //
    // Assignment operators from arithmetic types
    //
    constexpr u128 &operator=(s32 v) { return *this = u128(v); }
    constexpr u128 &operator=(u32 v) { return *this = u128(v); }
    constexpr u128 &operator=(s64 v) { return *this = u128(v); }
    constexpr u128 &operator=(u64 v) { return *this = u128(v); }
    constexpr u128 &operator=(s128 v);

    //
    // Conversion operators to other arithmetic types
    //
    constexpr explicit operator bool() const { return lo || hi; }
    constexpr explicit operator s8() const { return (s8)lo; }
    constexpr explicit operator u8() const { return (u8)lo; }
    constexpr explicit operator code_point() const { return (code_point)lo; }
    constexpr explicit operator wchar() const { return (wchar)lo; }
    constexpr explicit operator s16() const { return (s16)lo; }
    constexpr explicit operator u16() const { return (u16)lo; }
    constexpr explicit operator s32() const { return (s32)lo; }
    constexpr explicit operator u32() const { return (u32)lo; }
    constexpr explicit operator s64() const { return (s64)lo; }
    constexpr explicit operator u64() const { return (u64)lo; }
    explicit operator float() const;
    explicit operator double() const;

    //
    // Arithmetic operators
    //
    constexpr u128 &operator+=(u128 other);
    constexpr u128 &operator-=(u128 other);
    constexpr u128 &operator*=(u128 other);
    constexpr u128 &operator/=(u128 other);
    constexpr u128 &operator%=(u128 other);
    constexpr u128 operator++(s32);
    constexpr u128 operator--(s32);
    constexpr u128 &operator<<=(s32);
    constexpr u128 &operator>>=(s32);
    constexpr u128 &operator&=(u128 other);
    constexpr u128 &operator|=(u128 other);
    constexpr u128 &operator^=(u128 other);
    constexpr u128 &operator++();
    constexpr u128 &operator--();
};

// Casts from unsigned to signed while preserving the underlying binary
// representation.
inline constexpr s64 s64_bit_cast_to_u64(u64 v)
{
    // Casting an unsigned integer to a signed integer of the same
    // width is implementation defined behavior if the source value would not fit
    // in the destination type. We step around it with a roundtrip bitwise not
    // operation to make sure this function remains constexpr. Clang, GCC, and
    // MSVC optimize this to a no-op on x86-64.
    return v & (u64{1} << 63) ? ~((s64)(~v)) : ((s64)(v));
}

//
// s128
//
// A signed 128-bit integer type. See docs for u128.
//
// The design goal for `s128` is that it will be compatible with a future
// `int128_t`, if that type becomes a part of the standard.
//
// However, an `s128` differs from intrinsic integral types in the following
// ways:
//
//   * It is not implicitly convertible to other integral types.
//   * Requires explicit construction from and conversion to floating point
//     types.
//
// Example:
//
//     float y = s128(17);  // Error. s128 cannot be implicitly
//                          // converted to float.
//
//     s128 v;
//     s64 i = v;        // Error
//     s64 i = (s64) v;  // OK
//
struct alignas(16) s128
{
#if ENDIAN == LITTLE_ENDIAN
    u64 lo;
    s64 hi;
#elif ENDIAN == BIG_ENDIAN
    s64 hi;
    u64 lo;
#else
#error "Unsupported byte order: must be little or big endian."
#endif
    constexpr s128() {}

    //
    // Constructors from arithmetic types
    //

#if ENDIAN == LITTLE_ENDIAN
    constexpr s128(s64 high, u64 low) : lo(low), hi(high) {}
    constexpr s128(s32 v) : lo{(u64)v}, hi{v < 0 ? ~s64{0} : 0} {}
    constexpr s128(s64 v) : lo{(u64)v}, hi{v < 0 ? ~s64{0} : 0} {}
    constexpr s128(u32 v) : lo{v}, hi{0} {}
    constexpr s128(u64 v) : lo{v}, hi{0} {}
    explicit constexpr s128(u128 v) : lo{v.lo}, hi{(s64)v.hi} {}
#elif ENDIAN == BIG_ENDIAN
    constexpr s128(s64 high, u64 low) : hi{high}, lo{low} {}
    constexpr s128(s32 v) : hi{v < 0 ? ~s64{0} : 0}, lo{(u64)v} {}
    constexpr s128(s64 v) : hi{v < 0 ? ~s64{0} : 0}, lo{(u64)v} {}
    constexpr s128(u32 v) : hi{0}, lo{v} {}
    constexpr s128(u64 v) : hi{0}, lo{v} {}
    explicit constexpr s128(u128 v) : hi{(s64)v.hi}, lo{v.lo} {}
#else
#error "Unsupported byte order: must be little or big endian."
#endif
    explicit s128(float v);
    explicit s128(double v);

    //
    // Assignment operators from arithmetic types
    //
    constexpr s128 &operator=(s32 v) { return *this = s128(v); }
    constexpr s128 &operator=(u32 v) { return *this = s128(v); }
    constexpr s128 &operator=(s64 v) { return *this = s128(v); }
    constexpr s128 &operator=(u64 v) { return *this = s128(v); }

    //
    // Conversion operators to other arithmetic types
    //
    constexpr explicit operator bool() const { return lo || hi; }
    constexpr explicit operator s8() const { return (s8)(s64)(*this); }
    constexpr explicit operator u8() const { return (u8)lo; }
    constexpr explicit operator wchar() const { return (wchar)(s64)(*this); }
    constexpr explicit operator code_point() const { return (code_point)lo; }
    constexpr explicit operator s16() const { return (s16)(s64)(*this); }
    constexpr explicit operator u16() const { return (u16)lo; }
    constexpr explicit operator s32() const { return (s32)(s64)(*this); }
    constexpr explicit operator u32() const { return (u32)lo; }

    constexpr explicit operator s64() const
    {
        // We don't bother checking the value of hi. If *this < 0, lo's high bit
        // must be set in order for the value to fit into a s64. Conversely, if
        // lo's high bit is set, *this must be < 0 for the value to fit.
        return s64_bit_cast_to_u64(lo);
    }

    constexpr explicit operator u64() const { return lo; }
    explicit operator float() const;
    explicit operator double() const;

    //
    // Arithmetic operators
    //
    constexpr s128 &operator+=(s128 other);
    constexpr s128 &operator-=(s128 other);
    constexpr s128 &operator*=(s128 other);
    constexpr s128 &operator/=(s128 other);
    constexpr s128 &operator%=(s128 other);
    constexpr s128 operator++(s32);
    constexpr s128 operator--(s32);
    constexpr s128 &operator++();
    constexpr s128 &operator--();
    constexpr s128 &operator&=(s128 other);
    constexpr s128 &operator|=(s128 other);
    constexpr s128 &operator^=(s128 other);
    constexpr s128 &operator<<=(s32 amount);
    constexpr s128 &operator>>=(s32 amount);
};

constexpr u128 &u128::operator=(s128 v) { return *this = u128(v); }

#if ENDIAN == LITTLE_ENDIAN
constexpr u128::u128(s128 v) : lo{v.lo}, hi{(u64)(v.hi)} {}
#elif ENDIAN == BIG_ENDIAN
constexpr u128::u128(s128 v) : hi{(u64)(v.hi)}, lo{v.lo} {}
#else
#error "Unsupported byte order: must be little or big endian."
#endif

extern "C"
{
#if !defined LSTD_NO_CRT && COMPILER == MSVC
    __declspec(dllimport) double ldexp(double, s32); // Sigh...
#else
    double ldexp(double, s32);
#endif
} // TODO: Constexpr

inline u128::operator float() const
{
    return (float)lo + (float)ldexp((double)hi, 64);
}
inline u128::operator double() const
{
    return (double)lo + ldexp((double)hi, 64);
}

/*
inline s128::operator float() const {
    // We must convert the absolute value and then negate as needed, because
    // floating point types are typically sign-magnitude. Otherwise, the
    // difference between the high and low 64 bits when interpreted as two's
    // complement overwhelms the precision of the mantissa.
    //
    // Also check to make sure we don't negate Int128Min()
    return hi < 0 && *this != Int128Min() ? -static_cast<float>(-*this) :
(float) lo + ldexp((float) hi, 64);
}
inline s128::operator double() const {  // See comment in s128::operator
float() above. return hi < 0 && *this != Int128Min() ?
-static_cast<double>(-*this) : (double) lo + ldexp((double) hi, 64);
}
*/

constexpr bool operator==(u128 lhs, u128 rhs)
{
    return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}
constexpr bool operator!=(u128 lhs, u128 rhs) { return !(lhs == rhs); }
constexpr bool operator<(u128 lhs, u128 rhs)
{
    return (lhs.hi == rhs.hi) ? (lhs.lo < rhs.lo) : (lhs.hi < rhs.hi);
}
constexpr bool operator>(u128 lhs, u128 rhs) { return rhs < lhs; }
constexpr bool operator<=(u128 lhs, u128 rhs) { return !(rhs < lhs); }
constexpr bool operator>=(u128 lhs, u128 rhs) { return !(lhs < rhs); }

constexpr bool operator==(s128 lhs, s128 rhs)
{
    return (lhs.lo == rhs.lo && lhs.hi == rhs.hi);
}
constexpr bool operator!=(s128 lhs, s128 rhs) { return !(lhs == rhs); }
constexpr bool operator<(s128 lhs, s128 rhs)
{
    return (lhs.hi == rhs.hi) ? (lhs.lo < rhs.lo) : (lhs.hi < rhs.hi);
}
constexpr bool operator>(s128 lhs, s128 rhs)
{
    return (lhs.hi == rhs.hi) ? (lhs.lo > rhs.lo) : (lhs.hi > rhs.hi);
}
constexpr bool operator<=(s128 lhs, s128 rhs) { return !(lhs > rhs); }
constexpr bool operator>=(s128 lhs, s128 rhs) { return !(lhs < rhs); }

constexpr u128 operator-(u128 val)
{
    u64 hi = ~val.hi;
    u64 lo = ~val.lo;
    if (lo == 0)
        ++hi; // carry
    return u128(hi, lo);
}

constexpr bool operator!(u128 val) { return !val.hi && !val.lo; }
constexpr u128 operator~(u128 val) { return u128(~val.hi, ~val.lo); }
constexpr u128 operator|(u128 lhs, u128 rhs)
{
    return u128(lhs.hi | rhs.hi, lhs.lo | rhs.lo);
}
constexpr u128 operator&(u128 lhs, u128 rhs)
{
    return u128(lhs.hi & rhs.hi, lhs.lo & rhs.lo);
}
constexpr u128 operator^(u128 lhs, u128 rhs)
{
    return u128(lhs.hi ^ rhs.hi, lhs.lo ^ rhs.lo);
}

constexpr u128 &u128::operator|=(u128 other)
{
    hi |= other.hi;
    lo |= other.lo;
    return *this;
}

constexpr u128 &u128::operator&=(u128 other)
{
    hi &= other.hi;
    lo &= other.lo;
    return *this;
}

constexpr u128 &u128::operator^=(u128 other)
{
    hi ^= other.hi;
    lo ^= other.lo;
    return *this;
}

constexpr u128 operator<<(u128 lhs, s32 amount)
{
    // u64 shifts of >= 64 are undefined, so we will need to check some special
    // cases
    if (amount < 64)
    {
        if (amount != 0)
            return u128((lhs.hi << amount) | (lhs.lo >> (64 - amount)),
                        lhs.lo << amount);
        return lhs;
    }
    return u128(lhs.lo << (amount - 64), 0);
}

constexpr u128 operator>>(u128 lhs, s32 amount)
{
    // u64 shifts of >= 64 are undefined, so we will need to check some special
    // cases
    if (amount < 64)
    {
        if (amount != 0)
            return u128(lhs.hi >> amount,
                        (lhs.lo >> amount) | (lhs.hi << (64 - amount)));
        return lhs;
    }
    return u128(0, lhs.hi >> (amount - 64));
}

constexpr u128 operator+(u128 lhs, u128 rhs)
{
    u128 result = u128(lhs.hi + rhs.hi, lhs.lo + rhs.lo);
    if (result.lo < lhs.lo)
        return u128(result.hi + 1, result.lo);
    return result;
}

constexpr u128 operator-(u128 lhs, u128 rhs)
{
    u128 result = u128(lhs.hi - rhs.hi, lhs.lo - rhs.lo);
    if (lhs.lo < rhs.lo)
        return u128(result.hi - 1, result.lo);
    return result;
}

constexpr u128 operator*(u128 lhs, u128 rhs)
{
    u64 a32 = lhs.lo >> 32;
    u64 a00 = lhs.lo & 0xffffffff;
    u64 b32 = rhs.lo >> 32;
    u64 b00 = rhs.lo & 0xffffffff;
    u128 result = u128(lhs.hi * rhs.lo + lhs.lo * rhs.hi + a32 * b32, a00 * b00);
    result += u128(a32 * b00) << 32;
    result += u128(a00 * b32) << 32;
    return result;
}

constexpr void div_mod(u128 dividend, u128 divisor, u128 *quotient_ret, u128 *remainder_ret);

constexpr u128 operator/(u128 lhs, u128 rhs)
{
    u128 quotient = 0, remainder = 0;
    div_mod(lhs, rhs, &quotient, &remainder);
    return quotient;
}

constexpr u128 operator%(u128 lhs, u128 rhs)
{
    u128 quotient = 0, remainder = 0;
    div_mod(lhs, rhs, &quotient, &remainder);
    return remainder;
}

constexpr u128 u128::operator++(s32)
{
    u128 tmp(*this);
    *this += 1;
    return tmp;
}

constexpr u128 u128::operator--(s32)
{
    u128 tmp(*this);
    *this -= 1;
    return tmp;
}

constexpr u128 &u128::operator++()
{
    *this += 1;
    return *this;
}

constexpr u128 &u128::operator--()
{
    *this -= 1;
    return *this;
}

constexpr s128 operator-(s128 v)
{
    s64 hi = ~v.hi;
    u64 lo = ~v.lo + 1;
    if (lo == 0)
        ++hi; // carry
    return s128(hi, lo);
}

constexpr bool operator!(s128 v) { return !v.lo && !v.hi; }

constexpr s128 operator~(s128 val) { return s128(~val.hi, ~val.lo); }

constexpr s128 operator+(s128 lhs, s128 rhs)
{
    s128 result = s128(lhs.hi + rhs.hi, lhs.lo + rhs.lo);
    if (result.lo < lhs.lo)
    { // check for carry
        return s128(result.hi + 1, result.lo);
    }
    return result;
}

constexpr s128 operator-(s128 lhs, s128 rhs)
{
    s128 result = s128(lhs.hi - rhs.hi, lhs.lo - rhs.lo);
    if (lhs.lo < rhs.lo)
    { // check for carry
        return s128(result.hi - 1, result.lo);
    }
    return result;
}

constexpr s128 operator*(s128 lhs, s128 rhs)
{
    u128 result = u128(lhs) * rhs;
    return s128(s64_bit_cast_to_u64(result.hi), result.lo);
}

constexpr u128 unsigned_absolute_value(s128 v)
{
    return v.hi < 0 ? -u128(v)
                    : u128(v); // Cast to uint128 before possibly negating
                               // because -Int128Min() is undefined.
}

constexpr s128 operator/(s128 lhs, s128 rhs)
{
    // assert(lhs != Int128Min() || rhs != -1);  // UB on two's complement.

    u128 quotient = 0, remainder = 0;
    div_mod(unsigned_absolute_value(lhs), unsigned_absolute_value(rhs), &quotient, &remainder);
    if ((lhs.hi < 0) != (rhs.hi < 0))
        quotient = -quotient;
    return s128(s64_bit_cast_to_u64(quotient.hi), quotient.lo);
}

constexpr s128 operator%(s128 lhs, s128 rhs)
{
    // assert(lhs != Int128Min() || rhs != -1);  // UB on two's complement.

    u128 quotient = 0, remainder = 0;
    div_mod(unsigned_absolute_value(lhs), unsigned_absolute_value(rhs), &quotient,
            &remainder);
    if (lhs.hi < 0)
        remainder = -remainder;
    return s128(s64_bit_cast_to_u64(remainder.hi), remainder.lo);
}

constexpr s128 s128::operator++(s32)
{
    s128 tmp(*this);
    *this += 1;
    return tmp;
}

constexpr s128 s128::operator--(s32)
{
    s128 tmp(*this);
    *this -= 1;
    return tmp;
}

constexpr s128 &s128::operator++()
{
    *this += 1;
    return *this;
}

constexpr s128 &s128::operator--()
{
    *this -= 1;
    return *this;
}

constexpr s128 operator|(s128 lhs, s128 rhs)
{
    return s128(lhs.hi | rhs.hi, lhs.lo | rhs.lo);
}
constexpr s128 operator&(s128 lhs, s128 rhs)
{
    return s128(lhs.hi & rhs.hi, lhs.lo & rhs.lo);
}
constexpr s128 operator^(s128 lhs, s128 rhs)
{
    return s128(lhs.hi ^ rhs.hi, lhs.lo ^ rhs.lo);
}

constexpr s128 operator<<(s128 lhs, s32 amount)
{
    // u64 shifts of >= 64 are undefined, so we will need to check some special
    // cases
    if (amount < 64)
    {
        if (amount != 0)
            return s128((lhs.hi << amount) | (s64)(lhs.lo >> (64 - amount)),
                        lhs.lo << amount);
        return lhs;
    }
    return s128((s64)(lhs.lo << (amount - 64)), 0);
}

constexpr s128 operator>>(s128 lhs, s32 amount)
{
    // u64 shifts of >= 64 are undefined, so we will need to check some special
    // cases
    if (amount < 64)
    {
        if (amount != 0)
            return s128(lhs.hi >> amount,
                        (lhs.lo >> amount) | ((u64)lhs.hi << (64 - amount)));
        return lhs;
    }
    return s128(0, (u64)(lhs.hi >> (amount - 64)));
}

constexpr u128 &u128::operator<<=(s32 amount)
{
    return (*this = *this << amount), *this;
}
constexpr u128 &u128::operator>>=(s32 amount)
{
    return (*this = *this >> amount), *this;
}
constexpr u128 &u128::operator+=(u128 other)
{
    return (*this = *this + other), *this;
}
constexpr u128 &u128::operator-=(u128 other)
{
    return (*this = *this - other), *this;
}
constexpr u128 &u128::operator*=(u128 other)
{
    return (*this = *this * other), *this;
}
constexpr u128 &u128::operator/=(u128 other)
{
    return (*this = *this / other), *this;
}
constexpr u128 &u128::operator%=(u128 other)
{
    return (*this = *this % other), *this;
}
constexpr s128 &s128::operator+=(s128 other)
{
    return (*this = *this + other), *this;
}
constexpr s128 &s128::operator-=(s128 other)
{
    return (*this = *this - other), *this;
}
constexpr s128 &s128::operator*=(s128 other)
{
    return (*this = *this * other), *this;
}
constexpr s128 &s128::operator/=(s128 other)
{
    return (*this = *this / other), *this;
}
constexpr s128 &s128::operator%=(s128 other)
{
    return (*this = *this % other), *this;
}
constexpr s128 &s128::operator|=(s128 other)
{
    return (*this = *this | other), *this;
}
constexpr s128 &s128::operator&=(s128 other)
{
    return (*this = *this & other), *this;
}
constexpr s128 &s128::operator^=(s128 other)
{
    return (*this = *this ^ other), *this;
}
constexpr s128(&s128::operator<<=(s32 amount))
{
    return (*this = *this << amount), *this;
}
constexpr s128 &s128::operator>>=(s32 amount)
{
    return (*this = *this >> amount), *this;
}

namespace internal
{
#if COMPILER == MSVC
#pragma intrinsic(_BitScanReverse64)

    extern "C" unsigned char __cdecl _BitScanReverse64(unsigned long *_Index,
                                                       unsigned __int64 _Mask);
#endif

#if OS == WINDOWS
    inline s32 msb(u64 x)
    {
        unsigned long r = 0;
        return _BitScanReverse64(&r, x) ? ((s32)r) : -1;
    }
#elif OS == LINUX || OS == MACOS
    inline s32 msb(u64 x)
    {
        if (x == 0)
        {
            return -1;
        }
        // __builtin_clzll counts the leading zeros in the 64-bit integer x
        // Since we want the index of the most significant bit, we subtract the
        // count of leading zeros from 63 (the index of the highest bit in a 64-bit value)
        return 63 - __builtin_clzll(x);
    }
#endif

    inline s32 msb(u128 x)
    {
        if (x.hi != 0)
            return 64 + msb(x.hi);
        return msb(x.lo);
    }
} // namespace internal

// Long division/modulo for u128 implemented using the
// shift subtract division algorithm adapted from:
// https://stackoverflow.com/questions/5386377/division-without-using
constexpr void div_mod(u128 dividend, u128 divisor, u128 *quotient_ret, u128 *remainder_ret)
{
    if (divisor == 0)
        return;

    if (divisor > dividend)
    {
        *quotient_ret = 0;
        *remainder_ret = dividend;
        return;
    }

    if (divisor == dividend)
    {
        *quotient_ret = 1;
        *remainder_ret = 0;
        return;
    }

    u128 denominator = divisor;
    u128 quotient = 0;

    // Left aligns the MSB of the denominator and the dividend.
    s32 shift = internal::msb(dividend) - internal::msb(denominator);
    denominator <<= shift;

    // Uses shift-subtract algorithm to divide dividend by denominator. The
    // remainder will be left in dividend.
    for (s32 i = 0; i <= shift; ++i)
    {
        quotient <<= 1;
        if (dividend >= denominator)
        {
            dividend -= denominator;
            quotient |= 1;
        }
        denominator >>= 1;
    }

    *quotient_ret = quotient;
    *remainder_ret = dividend;
}

//
// Defines unions to access different bits of a
// floating point number according to the IEEE 754 standard.
//

LSTD_BEGIN_NAMESPACE

union ieee754_f32
{
    f32 F;  // FLOAT
    u32 W;  // WORD
    s32 SW; // Signed WORD

    // This is the IEEE 754 single-precision format.
    struct
    {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 8;
        u32 M : 23;
#else
        u32 M : 23;
        u32 E : 8;
        u32 S : 1;
#endif
    } ieee;

    // This format makes it easier to see if a NaN is a signalling NaN.
    struct
    {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 8;
        u32 N : 1;
        u32 M : 22;
#else
        u32 M : 22;
        u32 N : 1;
        u32 E : 8;
        u32 S : 1;
#endif
    } ieee_nan;
};

union ieee754_f64
{
    f64 F;   // FLOAT
    u64 DW;  // DWORD
    s64 SDW; // Signed DWORD

    struct
    {
#if ENDIAN == BIG_ENDIAN
        u32 MSW;
        u32 LSW;
#else
        u32 LSW;
        u32 MSW;
#endif
    };

    // This is the IEEE 754 single-precision format.
    struct
    {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 11;
        u32 M0 : 20;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 20;
        u32 E : 11;
        u32 S : 1;
#endif
    } ieee;

    // This format makes it easier to see if a NaN is a signalling NaN.
    struct
    {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 11;
        u32 N : 1;
        u32 M0 : 19;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 19;
        u32 N : 1;
        u32 E : 11;
        u32 S : 1;
#endif
    } ieee_nan;
};

LSTD_END_NAMESPACE

// Define numeric<T>, a way to get info about numbers, e.g. min/max,
// max digits, for floats: exponent, mantissa bits, etc.
//
// numeric<T> is useful when writing template functions and you don't know
// the specific integral type, so you cannot just assume the max size for
// 32 bit integer for example. In that case you can use numeric<T>::max()

LSTD_BEGIN_NAMESPACE

namespace internal
{
    struct numeric_base
    {
        static constexpr bool is_integral = false;
        static constexpr s32 digits = 0;
        static constexpr s32 digits10 = 0;
        static constexpr s32 max_digits10 = 0;
    };

    struct numeric_integer_base : numeric_base
    {
        static constexpr bool is_integral = true;
    };
} // namespace internal

template <typename T>
struct numeric : public internal::numeric_base
{
    using value_t = T;
};

// Const/volatile variations of numeric.
template <typename T>
struct numeric<const T> : public numeric<T>
{
};
template <typename T>
struct numeric<volatile T> : public numeric<T>
{
};
template <typename T>
struct numeric<const volatile T> : public numeric<T>
{
};

template <>
struct numeric<char> : public internal::numeric_integer_base
{
    static constexpr char min() { return (-128); }
    static constexpr char max() { return (127); }

    static constexpr s32 digits = 8;
    static constexpr s32 digits10 = 2;
};

template <>
struct numeric<wchar_t> : public internal::numeric_integer_base
{
    static constexpr wchar_t min() { return 0x0000; }
    static constexpr wchar_t max() { return 0xffff; }

    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};

template <>
struct numeric<bool> : public internal::numeric_integer_base
{
    // limits for type bool
    static constexpr bool min() { return false; }
    static constexpr bool max() { return true; }

    static constexpr s32 digits = 1;
};

template <>
struct numeric<u8> : public internal::numeric_integer_base
{
    static constexpr u8 min() { return 0; }
    static constexpr u8 max() { return (255); }

    static constexpr s32 digits = 8;
    static constexpr s32 digits10 = 2;
};

template <>
struct numeric<s16> : public internal::numeric_integer_base
{
    static constexpr s16 min() { return (-32768); }
    static constexpr s16 max() { return (32767); }

    static constexpr s32 digits = 15;
    static constexpr s32 digits10 = 4;
};

#ifdef _NATIVE_WCHAR_T_DEFINED
template <>
struct numeric<u16> : public internal::numeric_integer_base
{
    static constexpr u16 min() { return 0; }
    static constexpr u16 max() { return (65535); }

    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};
#endif

template <>
struct numeric<unsigned short> : public internal::numeric_integer_base
{
    static constexpr u16 min() { return 0; }
    static constexpr u16 max() { return (65535); }

    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};

template <>
struct numeric<char8_t> : public internal::numeric_integer_base
{
    static constexpr char8_t min() { return 0; }
    static constexpr char8_t max() { return (255); }

    static constexpr s32 digits = 8;
    static constexpr s32 digits10 = 2;
};

template <>
struct numeric<char16_t> : public internal::numeric_integer_base
{
    static constexpr char16_t min() { return 0; }
    static constexpr char16_t max() { return (65535); }

    static constexpr s32 digits = 16;
    static constexpr s32 digits10 = 4;
};

template <>
struct numeric<s32> : public internal::numeric_integer_base
{
    static constexpr s32 min() { return (-2147483647 - 1); }
    static constexpr s32 max() { return (2147483647); }

    static constexpr s32 digits = 31;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric<u32> : public internal::numeric_integer_base
{
    static constexpr u32 min() { return 0; }
    static constexpr u32 max() { return (4294967295U); }

    static constexpr s32 digits = 32;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric<char32_t> : public internal::numeric_integer_base
{
public:
    static constexpr char32_t min() { return 0; }
    static constexpr char32_t max() { return (4294967295U); }

    static constexpr s32 digits = 32;
    static constexpr s32 digits10 = 9;
};

template <>
struct numeric<s64> : public internal::numeric_integer_base
{
    static constexpr s64 min() { return (-9223372036854775807L - 1); }
    static constexpr s64 max() { return (9223372036854775807L); }

    static constexpr s32 digits = 63;
    static constexpr s32 digits10 = 18;
};

template <>
struct numeric<long> : public internal::numeric_integer_base
{
    static constexpr s64 min() { return (-9223372036854775807L - 1); }
    static constexpr s64 max() { return (9223372036854775807L); }

    static constexpr s32 digits = 63;
    static constexpr s32 digits10 = 18;
};

template <>
struct numeric<u64> : public internal::numeric_integer_base
{
public:
    static constexpr u64 min() { return 0; }
    static constexpr u64 max() { return (18446744073709551615ULL); }

    static constexpr s32 digits = 64;
    static constexpr s32 digits10 = 19;
};

template <>
struct numeric<unsigned long> : public internal::numeric_integer_base
{
public:
    static constexpr u64 min() { return 0; }
    static constexpr u64 max() { return (18446744073709551615ULL); }

    static constexpr s32 digits = 64;
    static constexpr s32 digits10 = 19;
};

template <>
struct numeric<f32>
{
public:
    static constexpr f32 min() { return 1.175494351e-38F; }
    static constexpr f32 max() { return 3.402823466e+38F; }
    static constexpr f32 epsilon()
    {
        return 1.192092896e-07F;
    } // smallest suchthat 1.0 + epsilon != 1.0
    static constexpr f32 round_error() { return 0.5F; }
    static constexpr f32 denorm_min() { return 1.401298464e-45F; }
    static constexpr f32 infinity() { return __builtin_huge_valf(); }
    static constexpr f32 quiet_NaN() { return __builtin_nanf("0"); }
    static constexpr f32 signaling_NaN() { return __builtin_nansf("1"); }

    static constexpr s32 digits = 23 + 1;  // including the hidden bit
    static constexpr s32 digits10 = 6;     // # of decimal digits of precision
    static constexpr s32 max_digits10 = 9; // # of decimal digits of precision
    static constexpr s32 max_exponent = 127;
    static constexpr s32 max_exponent10 = 38;
    static constexpr s32 min_exponent = -126;
    static constexpr s32 min_exponent10 = -37;
    static constexpr s32 bits_mantissa =
        23; // # of bits in mantissa, excluding the hidden bit (which is always
            // interpreted as 1 for normal numbers)
    static constexpr s32 bits_exponent = 8;
    static constexpr s32 exponent_bias = 127;
};

template <>
struct numeric<f64>
{
public:
    static constexpr f64 min() { return 2.2250738585072014e-308; }
    static constexpr f64 max() { return 1.7976931348623158e+308; }
    static constexpr f64 epsilon()
    {
        return 2.2204460492503131e-016;
    } // smalles such that 1.0 + epsilon != 1.0
    static constexpr f64 round_error() { return 0.5; }
    static constexpr f64 denorm_min() { return 4.9406564584124654e-324; }
    static constexpr f64 infinity() { return __builtin_huge_val(); }
    static constexpr f64 quiet_NaN() { return __builtin_nan("0"); }
    static constexpr f64 signaling_NaN() { return __builtin_nans("1"); }

    static constexpr s32 digits = 52 + 1;   // including the hidden bit
    static constexpr s32 digits10 = 15;     // # of decimal digits of precision
    static constexpr s32 max_digits10 = 17; // # of decimal digits of precision
    static constexpr s32 max_exponent = 1023;
    static constexpr s32 max_exponent10 = 308;
    static constexpr s32 min_exponent = -1022;
    static constexpr s32 min_exponent10 = -307;
    static constexpr s32 bits_mantissa =
        52; // number of bits in mantissa,excluding the hidden bit (which is
            // always interpreted as 1 for normalnumbers)
    static constexpr s32 bits_exponent = 11;
    static constexpr s32 exponent_bias = 1023;
};

template <>
struct numeric<u128> : public internal::numeric_integer_base
{
public:
    static constexpr u128 min() { return 0; }
    static constexpr u128 max() { return u128(U64_MAX, U64_MAX); }

    static constexpr s32 digits = 128;
    static constexpr s32 digits10 = 38;
};

template <>
struct numeric<s128> : public internal::numeric_integer_base
{
    static constexpr s128 min() { return s128(numeric<s64>::min(), 0); }
    static constexpr s128 max() { return s128(numeric<s64>::max(), U64_MAX); }

    static constexpr s32 digits = 127;
    static constexpr s32 digits10 = 38;
};

LSTD_END_NAMESPACE

//
// Define va_list and related macros if the C runtime is not being used
//
#if defined LSTD_NO_CRT
#define va_start __crt_va_start
#define va_arg __crt_va_arg
#define va_end __crt_va_end
#define va_copy(destination, source) ((destination) = (source))
#endif

//
// This defines the necessary types for the spaceship <=> operator in C++20
// to work. partial_ordering, weak_ordering, strong_ordering, comparison_category_of.
//

#if defined LSTD_NO_CRT
namespace std
{
    using literal_zero = decltype(null);

    // These "pretty" enumerator names are safe since they reuse names of
    // user-facing entities.
    enum class compare_result_eq : char
    {
        EQUAL = 0
    }; // -0.0 is equal to +0.0

    enum class compare_result_ord : char
    {
        LESS = -1,
        GREATER = 1
    };

    enum class compare_result_unord : char
    {
        UNORDERED = -127
    };

    struct partial_ordering
    {
        char Value;

        explicit partial_ordering(const compare_result_eq value)
            : Value((char)value) {}
        explicit partial_ordering(const compare_result_ord value)
            : Value((char)value) {}
        explicit partial_ordering(const compare_result_unord value)
            : Value((char)value) {}

        static const partial_ordering less;
        static const partial_ordering equivalent;
        static const partial_ordering greater;
        static const partial_ordering unordered;

        friend bool operator==(const partial_ordering &,
                               const partial_ordering &) = default;

        bool is_ordered() const
        {
            return Value != (char)compare_result_unord::UNORDERED;
        }
    };

    inline const partial_ordering partial_ordering::less(compare_result_ord::LESS);
    inline const partial_ordering partial_ordering::equivalent(
        compare_result_eq::EQUAL);
    inline const partial_ordering partial_ordering::greater(
        compare_result_ord::GREATER);
    inline const partial_ordering partial_ordering::unordered(
        compare_result_unord::UNORDERED);

    inline bool operator==(const partial_ordering value, literal_zero)
    {
        return value.Value == 0;
    }

    inline bool operator<(const partial_ordering value, literal_zero)
    {
        return value.Value == (char)compare_result_ord::LESS;
    }
    inline bool operator<(literal_zero, const partial_ordering value)
    {
        return 0 < value.Value;
    }

    inline bool operator>(const partial_ordering value, literal_zero)
    {
        return value.Value > 0;
    }
    inline bool operator>(literal_zero, const partial_ordering value)
    {
        return 0 > value.Value && value.is_ordered();
    }

    inline bool operator<=(const partial_ordering value, literal_zero)
    {
        return value.Value <= 0 && value.is_ordered();
    }
    inline bool operator<=(literal_zero, const partial_ordering value)
    {
        return 0 <= value.Value;
    }

    inline bool operator>=(const partial_ordering value, literal_zero)
    {
        return value.Value >= 0;
    }
    inline bool operator>=(literal_zero, const partial_ordering value)
    {
        return 0 >= value.Value && value.is_ordered();
    }

    inline partial_ordering operator<=>(const partial_ordering value,
                                        literal_zero)
    {
        return value;
    }
    inline partial_ordering operator<=>(literal_zero,
                                        const partial_ordering value)
    {
        return partial_ordering{(compare_result_ord)-value.Value};
    }

    struct weak_ordering
    {
        char Value;

        explicit weak_ordering(const compare_result_eq value) : Value((char)value) {}
        explicit weak_ordering(const compare_result_ord value) : Value((char)value) {}

        static const weak_ordering less;
        static const weak_ordering equivalent;
        static const weak_ordering greater;

        operator partial_ordering() const
        {
            return partial_ordering{(compare_result_ord)Value};
        }

        friend bool operator==(const weak_ordering &,
                               const weak_ordering &) = default;
    };

    inline const weak_ordering weak_ordering::less(compare_result_ord::LESS);
    inline const weak_ordering weak_ordering::equivalent(compare_result_eq::EQUAL);
    inline const weak_ordering weak_ordering::greater(compare_result_ord::GREATER);

    inline bool operator==(const weak_ordering value, literal_zero)
    {
        return value.Value == 0;
    }

    inline bool operator<(const weak_ordering value, literal_zero)
    {
        return value.Value < 0;
    }
    inline bool operator<(literal_zero, const weak_ordering value)
    {
        return 0 < value.Value;
    }

    inline bool operator>(const weak_ordering value, literal_zero)
    {
        return value.Value > 0;
    }
    inline bool operator>(literal_zero, const weak_ordering value)
    {
        return 0 > value.Value;
    }

    inline bool operator<=(const weak_ordering value, literal_zero)
    {
        return value.Value <= 0;
    }
    inline bool operator<=(literal_zero, const weak_ordering value)
    {
        return 0 <= value.Value;
    }

    inline bool operator>=(const weak_ordering value, literal_zero)
    {
        return value.Value >= 0;
    }
    inline bool operator>=(literal_zero, const weak_ordering value)
    {
        return 0 >= value.Value;
    }

    inline weak_ordering operator<=>(const weak_ordering value, literal_zero)
    {
        return value;
    }
    inline weak_ordering operator<=>(literal_zero, const weak_ordering value)
    {
        return weak_ordering{(compare_result_ord)-value.Value};
    }

    struct strong_ordering
    {
        char Value;

        explicit strong_ordering(const compare_result_eq value)
            : Value((char)value) {}
        explicit strong_ordering(const compare_result_ord value)
            : Value((char)value) {}

        static const strong_ordering less;
        static const strong_ordering equal;
        static const strong_ordering equivalent;
        static const strong_ordering greater;

        operator weak_ordering() const
        {
            return weak_ordering{(compare_result_ord)Value};
        }
        operator partial_ordering() const
        {
            return partial_ordering{(compare_result_ord)Value};
        }

        friend bool operator==(const strong_ordering &,
                               const strong_ordering &) = default;
    };

    inline const strong_ordering strong_ordering::less(compare_result_ord::LESS);
    inline const strong_ordering strong_ordering::equal(compare_result_eq::EQUAL);
    inline const strong_ordering strong_ordering::equivalent(
        compare_result_eq::EQUAL);
    inline const strong_ordering strong_ordering::greater(
        compare_result_ord::GREATER);

    inline bool operator==(const strong_ordering value, literal_zero)
    {
        return value.Value == 0;
    }

    inline bool operator<(const strong_ordering value, literal_zero)
    {
        return value.Value < 0;
    }
    inline bool operator<(literal_zero, const strong_ordering value)
    {
        return 0 < value.Value;
    }

    inline bool operator>(const strong_ordering value, literal_zero)
    {
        return value.Value > 0;
    }
    inline bool operator>(literal_zero, const strong_ordering value)
    {
        return 0 > value.Value;
    }

    inline bool operator<=(const strong_ordering value, literal_zero)
    {
        return value.Value <= 0;
    }
    inline bool operator<=(literal_zero, const strong_ordering value)
    {
        return 0 <= value.Value;
    }

    inline bool operator>=(const strong_ordering value, literal_zero)
    {
        return value.Value >= 0;
    }
    inline bool operator>=(literal_zero, const strong_ordering value)
    {
        return 0 >= value.Value;
    }

    inline strong_ordering operator<=>(const strong_ordering value, literal_zero)
    {
        return value;
    }
    inline strong_ordering operator<=>(literal_zero, const strong_ordering value)
    {
        return strong_ordering{(compare_result_ord)(-value.Value)};
    }
} // namespace std
#else
#include <compare>
#endif

LSTD_BEGIN_NAMESPACE

using partial_ordering = std::partial_ordering;
using weak_ordering = std::weak_ordering;
using strong_ordering = std::strong_ordering;

enum comparison_category : char
{
    COMPARISON_CATEGORY_NONE = 1,
    COMPARISON_CATEGORY_PARTIAL = 2,
    COMPARISON_CATEGORY_WEAK = 4,
    COMPARISON_CATEGORY_STRONG = 0,
};

// template <typename... Types>
// inline unsigned char comparison_category_of =
// get_comparison_category{(get_comparison_category<Types> | ... |
// COMPARISON_CATEGORY_STRONG)};

template <typename T>
inline unsigned char comparison_category_of = COMPARISON_CATEGORY_NONE;

template <>
inline unsigned char comparison_category_of<partial_ordering> =
    COMPARISON_CATEGORY_PARTIAL;

template <>
inline unsigned char comparison_category_of<weak_ordering> =
    COMPARISON_CATEGORY_WEAK;

template <>
inline unsigned char comparison_category_of<strong_ordering> =
    COMPARISON_CATEGORY_STRONG;

LSTD_END_NAMESPACE

//
// Minimal implementation of std::initializer_list if the C runtime is not being used
//
#if defined LSTD_NO_CRT

namespace std
{
    template <typename T>
    struct initializer_list
    {
        using value_type = T;
        using reference = const T &;
        using const_reference = const T &;
        using size_type = size_t;

        initializer_list() noexcept {}

        initializer_list(const T *first, const T *last) noexcept
            : First(first), Last(last) {}

        using iterator = const T *;
        using const_iterator = const T *;

        const T *begin() const noexcept { return First; }
        const T *end() const noexcept { return Last; }

        size_t size() const noexcept { return static_cast<size_t>(Last - First); }

        // private in order to be compatible with std::
    private:
        const T *First = null;
        const T *Last = null;
    };
} // namespace std
#else
#include <initializer_list>
#endif

LSTD_BEGIN_NAMESPACE
template <typename T>
using initializer_list = std::initializer_list<T>;
LSTD_END_NAMESPACE

//
// Support for std::source_location if the C runtime is not being used.
// Use this to get the location where a function
// was called without using macros.
//

#if defined LSTD_NO_CRT

// @Platform Compiles on MSVC only. @Robustness Not fully compatible with
// std::source_location (missing column info).
struct source_location
{
    const char *File = "Unknown";
    const char *Function = "Unknown";
    int Line = 0;

    constexpr source_location() {}

    // Uses built-in compiler functions.
    static consteval source_location current(
        const char *file = __builtin_FILE(),
        const char *func = __builtin_FUNCTION(), int line = __builtin_LINE())
    {
        source_location loc;
        loc.File = file;
        loc.Function = func;
        loc.Line = line;
        return loc;
    }
};
#else
#include <source_location>

LSTD_BEGIN_NAMESPACE
using source_location = std::source_location;
LSTD_END_NAMESPACE

#endif

LSTD_BEGIN_NAMESPACE

// LINE_NAME appends the line number to x, used in macros to get "unique"
// variable names.
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

struct Defer_Dummy
{
};
template <typename F>
struct Deferrer
{
    F Func;

    // We don't call destructors in free() (take a look at context.h), but they
    // still work. In this case to rely on them to implement defer. This gets
    // called on a stack variable anyway.
    ~Deferrer() { Func(); }
};
template <typename F>
Deferrer<F> operator*(Defer_Dummy, F func)
{
    return {func};
}

#define defer(x) \
    auto LINE_NAME(LSTD_defer) = LSTD_NAMESPACE::Defer_Dummy{} * [&]() { x; }

#define defer_to_exit(x) atexit([]() { x; })

LSTD_END_NAMESPACE

// Semantics to avoid the use of & when the symbol is not used as a unary or
// binary operator.
//
// e.g.
//      void array_process(array<u8> no_copy bytes) { ... }
//      void array_modify(array<u8> ref bytes) { ... }
//
//      array<s32> a;
//      modify_array(mut a);
//
#define no_copy const &
#define ref &
#define mut

// Used to mark functions for which the caller is supposed to free the result.
// This at leasts makes the compiler warn the caller if they've decided to
// discard the result.
//
// e.g.
//		mark_as_leak string make_string(...) { ... }
//
#define mark_as_leak [[nodiscard("Leak")]]

//
// e.g. cast(int) 5.0
//
#define cast(x) (x)

// Helper macro for, e.g flag enums
//
// enum flags {
//	Flag_1 = BIT(0),
//  Flag_1 = BIT(1)
//  Flag_1 = BIT(2)
//  ...
// };
//
#define BIT(x) (1 << (x))

// Gives the offset of a member in a struct (in bytes)
#define offset_of(s, field) ((u64) & ((s *)(0))->field)

#include <typeinfo>

LSTD_BEGIN_NAMESPACE

//
// This one is inspired from Python's enumerate().
// For each loop which also gives the index of the element.
//
// Example usage:
//
//    For_enumerate(a) {
//        b[it_index] = it + 1;  // Here _it_ is the object in the iterable and
//        _it_index_ is the index.
//    }
//
// .. which is the same as:
//
//    For(range(a.Count)) {
//        b[it] = a[it] + 1;    // Here _it_ is the iterator value of the range
//        between 0 and count.
//    }
//
// You can change the names of the internal
// variables by using _For_enumerate_as_.
//
#define For_enumerate_as(it_index, it, in) \
    for (auto [it_index, it] : LSTD_NAMESPACE::enumerate_impl(in))
#define For_enumerate(in) For_enumerate_as(it_index, it, in)

template <typename T, typename TIter = decltype(T().begin()),
          typename = decltype(T().end())>
auto enumerate_impl(T no_copy in)
{
    struct iterator
    {
        s64 I;
        TIter Iter;

        bool operator!=(iterator no_copy other) const { return Iter != other.Iter; }
        void operator++() { ++I, ++Iter; }

        struct dereference_result
        {
            s64 Index;
            decltype(*(TIter())) Value;
        };

        auto operator*() const { return dereference_result{I, *Iter}; }
    };

    struct iterable_wrapper
    {
        T Iterable;

        auto begin() { return iterator{0, Iterable.begin()}; }
        auto end() { return iterator{0, Iterable.end()}; }
    };

    return iterable_wrapper{in};
}

LSTD_END_NAMESPACE

LSTD_BEGIN_NAMESPACE

/**
 * @brief A utility base struct for defining properties as both
 * structconstants and as types.
 *
 * @tparam T The type of the integral constant.
 * @tparam Value The value of the integral constant.
 */
template <typename T, T Value>
struct integral_constant
{
    static const T value = Value;

    using value_t = T;
    using type = integral_constant<T, Value>;

    operator value_t() const { return value; }
    value_t operator()() const { return value; }
};

/// @brief A type alias equivalent to integral_constant<bool, true>
using true_t = integral_constant<bool, true>;
/// @brief A type alias equivalent to integral_constant<bool, false>
using false_t = integral_constant<bool, false>;

namespace internal
{
    template <typename T>
    struct is_const_helper_1 : false_t
    {
    };
    template <typename T>
    struct is_const_helper_1<volatile T *> : true_t
    {
    };
    template <typename T>
    struct is_const_helper_1<const volatile T *> : true_t
    {
    };
    template <typename T>
    struct is_const_helper_2 : is_const_helper_1<T *>
    {
    };
    template <typename T>
    struct is_const_helper_2<T &> : false_t
    {
    }; // Note here that Tis const, not the reference to T. So is_const is false.

    template <typename, typename>
    bool is_same_template_helper = false;
    template <template <typename...> typename T, typename... A, typename... B>
    bool is_same_template_helper<T<A...>, T<B...>> = true;

    template <typename T, typename U>
    struct same_helper : false_t
    {
    };
    template <typename T>
    struct same_helper<T, T> : true_t
    {
    };

    template <typename T>
    struct type_identity
    {
        using type = T;
    };

    template <typename T>
    auto try_add_lvalue_reference(int) -> type_identity<T &>;
    template <typename T>
    auto try_add_lvalue_reference(...) -> type_identity<T>;

    template <typename>
    struct is_function_helper : false_t
    {
    };
    template <typename R, typename... Args>
    struct is_function_helper<R(Args...)> : true_t
    {
    };
    template <typename R, typename... Args>
    struct is_function_helper<R(Args..., ...)> : true_t
    {
    };
    template <typename T>
    struct is_pointer_helper : false_t
    {
    };
    template <typename T>
    struct is_pointer_helper<T *> : true_t
    {
    };

    template <typename T>
    struct is_member_pointer_helper : false_t
    {
    };
    template <typename T, typename U>
    struct is_member_pointer_helper<T U::*> : true_t
    {
    };

    template <typename T>
    struct is_array_helper : false_t
    {
    };
    template <typename T>
    struct is_array_helper<T[]> : true_t
    {
    };
    template <typename T, int N>
    struct is_array_helper<T[N]> : true_t
    {
    };
} // namespace internal

/**
 * @brief A struct used to denote a special template argument that means it's an
 * unused argument.
 */
struct unused
{
};

template <bool Condition, typename ConditionIsTrueType,
          typename ConditionIsFalseType>
struct type_select
{
    using type = ConditionIsTrueType;
};

template <typename ConditionIsTrueType, typename ConditionIsFalseType>
struct type_select<false, ConditionIsTrueType, ConditionIsFalseType>
{
    using type = ConditionIsFalseType;
};

/**
 * @brief A type alias that selects one of two type options based on a boolean
 * condition.
 *
 * This type alias is used to declare a type from one of two type options. The
 * result is based on the value of the `Condition` template parameter.
 *
 * Example usage:
 * @code
 *    using T = type_select_t<Condition, ChoiceAType, ChoiceBType>;
 * @endcode
 *
 * @tparam Condition A boolean value determining the selected type.
 * @tparam ConditionIsTrueType The type to select if `Condition` is true.
 * @tparam ConditionIsFalseType The type to select if `Condition` is false.
 */
template <bool Condition, typename ConditionIsTrueType,
          typename ConditionIsFalseType>
using type_select_t = typename type_select<Condition, ConditionIsTrueType,
                                           ConditionIsFalseType>::type;

template <typename T, typename = unused, typename = unused>
struct first_type_select
{
    using type = T;
};

/**
 * @brief A type alias that unilaterally selects the first type.
 *
 * @tparam T The first type to select.
 * @tparam unused A placeholder unused type.
 * @tparam unused Another placeholder unused type.
 */
template <typename T, typename = unused, typename = unused>
using first_type_select_t = typename first_type_select<T>::type;

/**
 * @brief Concept to check if two types are the same.
 *
 * @tparam T The first type to compare.
 * @tparam U The second type to compare.
 */
template <typename T, typename U>
concept is_same = internal::same_helper<T, U>::value;

/**
 * @brief Concept to check if a type is the same as any of a list of given
 * types.
 *
 * @tparam T The type to compare against the list of types.
 * @tparam Types The list of types to compare with `T`.
 */
template <typename T, typename... Types>
concept is_same_to_one_of = (is_same<T, Types> || ...);

/**
 * @brief Concept to check if two types are the same, regardless of their
 * template parameters.
 *
 * This concept checks if two types have the same template but does not compare
 * their template parameters.
 *
 * Example usage:
 * @code
 *   is_same_template<array<int, 32>, array<float, 16>> // true
 * @endcode
 *
 * Note that it doesn't work if you mix types and typenames:
 * @code
 *   is_same_template<array, array<float, 16>> // false
 * @endcode
 *
 * @tparam T The first type to compare.
 * @tparam U The second type to compare.
 */
template <typename T, typename U>
concept is_same_template = internal::is_same_template_helper<T, U>;

/// @brief Concept to check if T has const-qualification.
template <typename T>
concept is_const = internal::is_const_helper_2<T>::value;

template <typename T>
struct remove_cv
{
    using type = T;
};
template <typename T>
struct remove_cv<const T>
{
    using type = T;
};
template <typename T>
struct remove_cv<const T[]>
{
    using type = T[];
};
template <typename T, s64 N>
struct remove_cv<const T[N]>
{
    using type = T[N];
};
template <typename T>
struct remove_cv<volatile T>
{
    using type = T;
};
template <typename T>
struct remove_cv<volatile T[]>
{
    using type = T[];
};
template <typename T, s64 N>
struct remove_cv<volatile T[N]>
{
    using type = T[N];
};
template <typename T>
struct remove_cv<const volatile T>
{
    using type = T;
};
template <typename T>
struct remove_cv<const volatile T[]>
{
    using type = T[];
};
template <typename T, s64 N>
struct remove_cv<const volatile T[N]>
{
    using type = T[N];
};

/// @brief Type alias to remove top-level const/volatile qualification from T.
template <typename T>
using remove_cv_t = typename remove_cv<T>::type;

template <typename T>
struct remove_ref
{
    using type = T;
};
template <typename T>
struct remove_ref<T &>
{
    using type = T;
};
template <typename T>
struct remove_ref<T &&>
{
    using type = T;
};

/**
 * @brief Type alias that removes top-level indirection by reference (if any)
 * from T.
 *
 * For a given type T, remove_reference_t<T&> is equivalent to T.
 */
template <typename T>
using remove_ref_t = typename remove_ref<T>::type;

template <typename T>
struct add_lvalue_reference
    : decltype(internal::try_add_lvalue_reference<T>(0))
{
};

/**
 * @brief Type alias to add an l-value reference to T.
 *
 * Follows the rules (8.3.2 p6):
 * @code
 *      void + &  -> void
 *      T    + &  -> T&
 *      T&   + &  -> T&
 *      T&&  + &  -> T&
 * @endcode
 */
template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

template <typename T>
struct add_rvalue_reference
{
    using type = T &&;
};
template <typename T>
struct add_rvalue_reference<T &>
{
    using type = T &;
};
template <>
struct add_rvalue_reference<void>
{
    using type = void;
};
template <>
struct add_rvalue_reference<const void>
{
    using type = const void;
};
template <>
struct add_rvalue_reference<volatile void>
{
    using type = volatile void;
};
template <>
struct add_rvalue_reference<const volatile void>
{
    using type = const volatile void;
};

/**
 * @brief Type alias to add an r-value reference to T.
 *
 * Follows the rules (8.3.2 p6):
 * @code
 *      void + &&  -> void
 *      T    + &&  -> T&&
 *      T&   + &&  -> T&
 *      T&&  + &&  -> T&&
 * @endcode
 */
template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

/**
 * @brief Function template that converts any type T to a reference type.
 *
 * This function template makes it possible to use member functions in decltype
 * expressions without specifying constructors. It has no use outside decltype
 * expressions.
 *
 * @tparam T The type to convert to a reference type.
 * @return A reference to T.
 */
template <typename T>
typename add_rvalue_reference<T>::type
declval() noexcept; // It works with compiler magic I guess.

/// @brief Concept to check if T is an integral type.
template <typename T>
concept is_integral = numeric<T>::is_integral;

/// @brief Concept to check if T is a signed integral type.
template <typename T>
concept is_signed_integral = is_integral<T> && T(-1) < T(0);

/// @brief Concept to check if T is an unsigned integral type.
template <typename T>
concept is_unsigned_integral = is_integral<T> && !is_signed_integral<T>;

/// @brief Concept to check if T is a floating point type.
template <typename T>
concept is_floating_point = is_same_to_one_of<remove_cv_t<T>, f32, f64>;

/// @brief Concept to check if T is an arithmetic type (integral or floating
/// point type).
template <typename T>
concept is_arithmetic = is_integral<T> || is_floating_point<T>;

/// @brief Concept to check if T is an enum type.
template <typename T>
concept is_enum = __is_enum(remove_cv_t<T>);

/// @brief Concept to check if T is a pointer to an object or a function, but
/// not to member objects/functions.
template <typename T>
concept is_function = internal::is_function_helper<T>::value;

/// @brief Tests whether T is a pointer to an object, to a function, but not to
/// member objects/functions.
template <typename T>
concept is_pointer = internal::is_pointer_helper<remove_cv_t<T>>::value;

/// @brief Concept to check if T is a pointer to a member object or a member
/// function.
template <typename T>
concept is_member_pointer =
    internal::is_member_pointer_helper<remove_cv_t<T>>::value;

/**
 * @brief Concept to check if T is a scalar type.
 *
 * A scalar is an integer type, a floating point type, an enum type, a pointer,
 * a member function pointer, or a null pointer type.
 */
template <typename T>
concept is_scalar = is_arithmetic<T> || is_enum<T> || is_pointer<T> ||
                    is_member_pointer<T> || is_same<remove_cv_t<T>, decltype(null)>;

/**
 * @brief Concept to check if a type is convertible to another type.
 *
 * This concept checks if an object of type `From` can be implicitly converted
 * to an object of type `To` through a valid standard conversion sequence,
 * user-defined conversion, or an ellipsis conversion.
 *
 * @tparam From The type to be converted from.
 * @tparam To The type to be converted to.
 */
template <typename From, typename To>
concept is_convertible = __is_convertible_to(From, To);

/**
 * @brief Concept to check if a type can be constructed with the given set of
 * arguments.
 *
 * This concept checks if an object of type `T` can be constructed with a set of
 * arguments of types `Args...`. It checks if a constructor for type `T` exists
 * that accepts the provided argument types.
 *
 * @tparam T The type of the object to be constructed.
 * @tparam Args The types of arguments to be used for constructing the object.
 */
template <typename T, typename... Args>
concept is_constructible = __is_constructible(T, Args...);

template <typename T>
struct underlying_type
{
    using type = __underlying_type(T);
};

/// @brief Alias template for the underlying type of an enum.
template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

template <typename T>
struct remove_cvref
{
    using type = remove_cv_t<remove_ref_t<T>>;
};

/**
 * @brief Alias template for the type obtained by removing top-level const
 * and/or volatile qualification and reference from the given type.
 */
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

/// @brief Concept to check if the decayed versions of "T" and "U" are the same
/// template type.
template <typename T, typename U>
concept is_same_template_decayed =
    is_same_template<remove_cvref_t<T>, remove_cvref_t<U>>;

template <typename T>
struct remove_pointer
{
    using type = T;
};
template <typename T>
struct remove_pointer<T *>
{
    using type = T;
};
template <typename T>
struct remove_pointer<T *const>
{
    using type = T;
};
template <typename T>
struct remove_pointer<T *volatile>
{
    using type = T;
};
template <typename T>
struct remove_pointer<T *const volatile>
{
    using type = T;
};

/// @brief Removes the first level of pointers from T.
template <typename T>
using remove_pointer_t = typename remove_pointer<T>::type;

template <typename T>
struct add_pointer
{
    using type = remove_ref_t<T> *;
};

// Adds a level of pointers to T
template <typename T>
using add_pointer_t = typename add_pointer<T>::type;

template <typename T>
concept is_array = internal::is_array_helper<T>::value;

template <typename T>
struct remove_extent
{
    using type = T;
};
template <typename T>
struct remove_extent<T[]>
{
    using type = T;
};
template <typename T, s64 N>
struct remove_extent<T[N]>
{
    using type = T;
};

/**
 * @brief The remove_extent transformation trait removes a dimension from an
 * array.
 *
 * This type trait is used to remove one level of array nesting from the input
 * type T. If the input type T is not an array, then remove_extent<T>::type is
 * equivalent to T. If the input type T is an array type T[N], then
 * remove_extent<T[N]>::type is equivalent to T. If the input type T is a
 * const-qualified array type const T[N], then remove_extent<const T[N]>::type
 * is equivalent to const T.
 *
 * For example, given a multi-dimensional array type T[M][N],
 * remove_extent<T[M][N]>::type is equivalent to T[N].
 */
template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

template <typename T>
struct decay
{
    using U = remove_ref_t<T>;

    using type = type_select_t<
        is_array<U>, remove_extent_t<U> *,
        type_select_t<is_function<U>, add_pointer_t<U>, remove_cv_t<U>>>;
};

/**
 * @brief Converts the type T to its decayed equivalent.
 *
 * This function template performs several type conversions to arrive at the
 * decayed equivalent of the input type T. These conversions include lvalue to
 * rvalue, array to pointer, function to pointer conversions, and removal of
 * const and volatile.
 *
 * The resulting type is the type conversion that's actually silently applied
 * by the compiler to all function arguments when passed by value.
 */
template <typename T>
using decay_t = typename decay<T>::type;

template <typename... T>
struct common_type;
template <typename T>
struct common_type<T>
{
    using type = decay_t<T>;
};

template <typename T, typename U>
struct common_type<T, U>
{
    using type = decay_t<decltype(true ? declval<T>() : declval<U>())>;
};

template <typename T, typename U, typename... V>
struct common_type<T, U, V...>
{
    using type =
        typename common_type<typename common_type<T, U>::type, V...>::type;
};

/**
 * @brief Computes the common type of a set of input types.
 * The common_type type trait computes the common type of a set of input types,
 * that is, the type to which every type in the parameter pack can be implicitly
 * converted to.
 *
 * It does so by using recursion and the common_type_helper struct to repeatedly
 * compute the common type of pairs of input types, until the common type of all
 * input types has been determined.
 */
template <typename... Ts>
using common_type_t = typename common_type<Ts...>::type;

/**
 * @brief Concept to check if the decayed versions of "T" and "U" are the same
 * basic type.
 *
 * This concept gets around the fact that `is_same` will treat `bool` and
 * `bool&` as different types.
 *
 * @tparam T The first type to compare.
 * @tparam U The second type to compare.
 */
template <typename T, typename U>
concept is_same_decayed = internal::same_helper<decay_t<T>, decay_t<U>>::value;

/**
 * @brief Safely converts between unrelated types that have a binary
 * equivalency.
 *
 * This approach is required by strictly conforming C++ compilers because
 * directly using a C or C++ cast between unrelated types is fraught with
 * the possibility of undefined runtime behavior due to type aliasing.
 *
 * @tparam DestType The destination type for the conversion.
 * @tparam SourceType The source type for the conversion.
 * @param sourceValue The source value to be converted.
 * @return The converted value of type DestType.
 *
 * Example usage:
 * @code
 *    f32 f = 1.234f;
 *    u32 br = bit_cast<u32>(f);
 * @endcode
 */
template <typename DestType, typename SourceType>
DestType bit_cast(SourceType const &sourceValue)
{
    static_assert(sizeof(DestType) == sizeof(SourceType));
    return __builtin_bit_cast(DestType, sourceValue);
}

//
// Type name extraction using typeid
//

/**
 * @brief Get the name of a type as a string.
 *
 * This function uses typeid to get the type name. On some compilers
 * this will be mangled, but it's still useful for debugging.
 *
 * @tparam T The type whose name to extract.
 * @return A string containing the type name (may be mangled).
 *
 * Example usage:
 * @code
 *   const char* name = type_name<int>();  // Returns type name
 * @endcode
 */
template <typename T>
const char *type_name()
{
    return typeid(T).name();
}

LSTD_END_NAMESPACE

//
// Shortcut macros for "for each" loops (really up to personal style if you want
// to use this)
//
//  For(array) print(it);
//
#define For_as(x, in) for (auto &&x : in)
#define For(in) For_as(it, in)

//
// Loop that gets unrolled at compile-time, this avoids copy-pasta
// or using macros in order to be sure the code gets unrolled properly.
//
template <s64 First, s64 Last, typename Lambda>
void static_for(Lambda f)
{
    if constexpr (First < Last)
    {
        f(integral_constant<s64, First>{});
        static_for<First + 1, Last>(f);
    }
}

/**
 * @brief Provides Python-like range functionality for iterating over a range of
 * integers.
 *
 * Example usage:
 *
 *  - `For(range(12)) { ...use the variable 'it' here... }`      // [0, 12)
 *  - `For(range(3, 10, 2)) {}`                                  // every second
 * integer (step 2) in [3, 10)
 *  - `For(range(10, 0, -1)) {}`                                 // reverse [10,
 * 0)
 *
 *  which is equivalent to:
 *
 *  - `For_as(it, range(12)) {}`
 *  - `For_as(it, range(3, 10, 2)) {}`
 *  - `For_as(it, range(10, 0, -1)) {}`
 *
 *  which is equivalent to:
 *
 *  - `for (int it = 0; it < 12; it++) {}`
 *  - `for (int it = 3; it < 10; it += 2) {}`
 *  - `for (int it = 10; it > 0; it += -1) {}`
 *
 * In release, it gets optimized to have no overhead.
 */
struct range
{
    struct iterator
    {
        s64 I, Step;

        iterator(s64 i, s64 step = 1) : I(i), Step(step) {}

        operator s32() const { return (s32)I; }
        operator s64() const { return I; }

        s64 operator*() const { return I; }
        iterator operator++() { return I += Step, *this; }

        iterator operator++(s32)
        {
            iterator temp(*this);
            return I += Step, temp;
        }

        bool operator==(iterator other) const
        {
            return Step < 0 ? (I <= other.I) : (I >= other.I);
        }
        bool operator!=(iterator other) const
        {
            return Step < 0 ? (I > other.I) : (I < other.I);
        }
    };

    iterator Begin;
    iterator End;

    range(s64 start, s64 stop, s64 step) : Begin(start, step), End(stop) {}
    range(s64 start, s64 stop) : range(start, stop, 1) {}
    range(u64 stop) : range(0, stop, 1) {}

    // Checks if a value is inside the given range.
    // This also accounts for stepping.
    bool has(s64 value) const
    {
        if (Begin.Step > 0 ? (value >= Begin.I && value < End.I)
                           : (value > End.I && value <= Begin.I))
        {
            s64 diff = value - Begin.I;
            if (diff % Begin.Step == 0)
            {
                return true;
            }
        }
        return false;
    }

    iterator begin() const { return Begin; }
    iterator end() const { return End; }
};

#if !defined(_SIZE_T)
#if BITS == 32
using size_t = u32;
using ptrdiff_t = s32;
using intptr_t = s32;
using time_t = s32;
#else
using size_t = u64;
using ptrdiff_t = s64;
using intptr_t = s64;
using time_t = s64;
#endif
#endif

using usize = size_t;

//
// Define some common math functions:
//    min, max, clamp,
//    abs,
//    cast_numeric_safe,
//    is_pow_of_2, ceil_pow_of_2, const_exp10,
//    is_nan, is_signaling_nan, is_infinite, is_finite,
//    sign_bit, sign_no_zero, sign, copy_sign
//

// https://tauday.com/tau-manifesto
#define TAU 6.283185307179586476925286766559
#define PI (TAU / 2)

//
// If we aren't building with CRT then:
//
// Cephes provides our replacement for the math functions found in virtually all
// standard libraries. Also provides functions for extended precision
// arithmetic, statistical functions, physics, astronomy, etc.
// https://www.netlib.org/cephes/
// Note: We don't include everything from it, just cmath for now.
//       Statistics is a thing we will most definitely include as well in the
//       future. Everything else you can include on your own in your project (we
//       don't want to be bloat-y).
//
// Note: Important difference,
// atan2's return range is 0 to 2PI, and not -PI to PI (as per normal in the C
// standard library).
//
// Parts of the source code that we modified are marked with :WEMODIFIEDCEPHES:
//
// @TODO: We should always have our own math functions
// because otherwise they'd differ from compiler to compiler.
// This is a horrendous mistake the C++ committee has allowed to happen.

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1995, 2000 by Stephen L. Moshier
*/
#if defined LSTD_NO_CRT
#include "vendor/cephes/maths_cephes.h"
#else
#include <math.h>
#endif

LSTD_BEGIN_NAMESPACE

inline bool sign_bit(is_signed_integral auto x) { return x < 0; }
inline bool sign_bit(is_unsigned_integral auto) { return false; }

inline bool sign_bit(f32 x) { return ieee754_f32{x}.ieee.S; }
inline bool sign_bit(f64 x) { return ieee754_f64{x}.ieee.S; }

// Returns -1 if x is negative, 1 otherwise
inline s32 sign_no_zero(is_scalar auto x) { return sign_bit(x) ? -1 : 1; }

// Returns -1 if x is negative, 1 if positive, 0 otherwise
inline s32 sign(is_scalar auto x)
{
    if (x == decltype(x)(0))
        return 0;
    return sign_no_zero(x);
}

template <is_floating_point T>
inline T copy_sign(T x, T y)
{
    if constexpr (sizeof(x) == sizeof(f32))
    {
        ieee754_f32 formatx = {x}, formaty = {y};
        formatx.ieee.S = formaty.ieee.S;
        return formatx.F;
    }
    else
    {
        ieee754_f64 formatx = {x}, formaty = {y};
        formatx.ieee.S = formaty.ieee.S;
        return formatx.F;
    }
}

inline bool is_nan(is_floating_point auto x)
{
    if constexpr (sizeof(x) == sizeof(f32))
    {
        ieee754_f32 format = {x};
        return format.ieee.E == 0xFF && format.ieee.M != 0;
    }
    else
    {
        ieee754_f64 format = {x};
        return format.ieee.E == 0x7FF &&
               (format.ieee.M0 != 0 || format.ieee.M1 != 0);
    }
}

inline bool is_signaling_nan(is_floating_point auto x)
{
    if constexpr (sizeof(x) == sizeof(f32))
        return is_nan(x) && ieee754_f32{x}.ieee_nan.N == 0;
    else
        return is_nan(x) && ieee754_f64{x}.ieee_nan.N == 0;
}

inline bool is_infinite(is_floating_point auto x)
{
    if constexpr (sizeof(x) == sizeof(f32))
    {
        ieee754_f32 format = {x};
        return format.ieee.E == 0xFF && format.ieee.M == 0;
    }
    else
    {
        ieee754_f64 format = {x};
        return format.ieee.E == 0x7FF && format.ieee.M0 == 0 && format.ieee.M1 == 0;
    }
}

inline bool is_finite(is_floating_point auto x)
{
    if constexpr (sizeof(x) == sizeof(f32))
        return ieee754_f32{x}.ieee.E != 0xFF;
    else
        return ieee754_f64{x}.ieee.E != 0x7FF;
}

/**
 * @brief Safely casts a numeric value of type U to a numeric value of type T.
 *
 * When LSTD_NUMERIC_CAST_CHECK is defined, runtime checks are performed
 * to detect potential overflows when casting between integer types. If an
 * overflow occurs, an error message is reported and an assertion is triggered.
 *
 * @tparam T The target numeric type to cast the value to.
 * @tparam U The source numeric type of the value being cast.
 * @param y The value of type U to be cast to type T.
 * @return The value y cast to type T.
 *
 * @note The template requires both T and U to be scalar types (i.e., integral
 * or floating-point types).
 */
template <typename T, typename U>
inline constexpr auto cast_numeric(U y)
    requires(is_scalar<T> && is_scalar<U>)
{
#if defined(LSTD_NUMERIC_CAST_CHECK)
    if constexpr (is_integral<T> && is_integral<U>)
    {
        if constexpr (is_signed_integral<T> && is_unsigned_integral<U>)
        {
            if (y > static_cast<U>(numeric<T>::max()))
            {
                // Report error and assert
                assert(false && "Overflow: unsigned to signed integer cast.");
            }
        }
        else if constexpr (is_unsigned_integral<T> && is_signed_integral<U>)
        {
            if (y < 0 || static_cast<u128>(y) > numeric<T>::max())
            {
                // Report error and assert
                assert(false && "Overflow: signed to unsigned integer cast.");
            }
        }
        else if constexpr (is_signed_integral<T> && is_signed_integral<U>)
        {
            if (y > numeric<T>::max() || y < numeric<T>::min())
            {
                // Report error and assert
                assert(false && "Overflow: signed integer to signed integer cast.");
            }
        }
        else if constexpr (is_unsigned_integral<T> && is_unsigned_integral<U>)
        {
            if (y > numeric<T>::max())
            {
                // Report error and assert
                assert(false && "Overflow: unsigned integer to unsigned integer cast.");
            }
        }
    }
#endif
    return (T)y;
}

namespace internal
{
    constexpr auto min_(auto x, auto y)
    {
        auto y_casted = cast_numeric<decltype(x)>(y);
        if constexpr (is_floating_point<decltype(x)>)
        {
            if (is_nan(x) || is_nan(y_casted))
                return x + y_casted;
        }
        return x < y_casted ? x : y_casted;
    }

    constexpr auto max_(auto x, auto y)
    {
        auto y_casted = cast_numeric<decltype(x)>(y);
        if constexpr (is_floating_point<decltype(x)>)
        {
            if (is_nan(x) || is_nan(y_casted))
                return x + y_casted;
        }
        return x > y_casted ? x : y_casted;
    }
} // namespace internal

template <is_scalar... Args>
inline constexpr auto min(is_scalar auto x, Args... rest)
{
    auto result = x;
    ((void)(result = internal::min_(result, rest)), ...);
    return result;
}

template <is_scalar... Args>
inline constexpr auto max(is_scalar auto x, Args... rest)
{
    auto result = x;
    ((void)(result = internal::max_(result, rest)), ...);
    return result;
}

// Returns lower if x < lower, return upper if x > upper, returns x otherwise
inline auto clamp(auto x, auto lower, auto upper)
{
    return max(lower, min(upper, x));
}

// Checks if x is a power of 2
inline bool is_pow_of_2(is_integral auto x) { return (x & x - 1) == 0; }

// Returns the smallest power of 2 bigger or equal to x.
inline auto ceil_pow_of_2(is_integral auto x)
{
    using T = decltype(x);

    if (x <= 1)
        return (T)1;

    T power = 2;
    --x;
    while (x >>= 1)
        power <<= 1;
    return power;
}

// Returns 10 ** exp at compile-time. Uses recursion.
template <typename T>
inline T const_exp10(s32 exp)
{
    return exp == 0 ? T(1) : T(10) * const_exp10<T>(exp - 1);
}

inline auto abs(is_scalar auto x)
{
    if constexpr (is_floating_point<decltype(x)>)
    {
        if constexpr (sizeof(x) == sizeof(f32))
        {
            ieee754_f32 u = {x};
            u.ieee.S = 0;
            return u.F;
        }
        else
        {
            ieee754_f64 u = {x};
            u.ieee.S = 0;
            return u.F;
        }
    }
    else
    {
        if constexpr (is_unsigned_integral<decltype(x)>)
        {
            return x; // Unsigned integrals are always positive
        }
        else
        {
            return x < 0 ? -x : x;
        }
    }
}
