#pragma once

#include "../common/namespace.h"
#include "../platform.h"

LSTD_BEGIN_NAMESPACE
// Replacement for the std::is_constant_evaluated
#if COMPILER == MSVC
[[nodiscard]] constexpr bool is_constant_evaluated() noexcept { return __builtin_is_constant_evaluated(); }
#else
#error Implement.
#endif
LSTD_END_NAMESPACE

//
// The following integral types are defined: s8, s16, s32, s64 (and corresponding unsigned types: u8, u16, u32, u64)
//		f32 (float), f64 (double), utf8 (char), byte (unsigned char)
// This file also defines vector types (to be used with streaming SIMD extensions).
//
// Note: We don't support long doubles (lf64) or operations with them. That's why we don't even provide type info.
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

// We use utf8 for bytes which are supposed to be encoded in utf8
// instead of using just char (which generally has the meaning of byte).
// We also define byte which is an unsigned type and can store [0-255].
// Strings in C++ are char* which makes them compatible with utf8*.
using utf8 = char;

using utf16 = wchar_t; // There is char16_t but since utf16 is only used on Windows (or you should die in hell) and windows headers use wchar_t
using utf32 = char32_t;

using byte = unsigned char;

#define S64_C(c) c##L
#define U64_C(c) c##UL

#define S8_MIN (-128)
#define S16_MIN (-32767 - 1)
#define S32_MIN (-2147483647 - 1)
#define S64_MIN (-S64_C(9223372036854775807L) - 1)

#define S8_MAX (127)
#define S16_MAX (32767)
#define S32_MAX (2147483647)
#define S64_MAX (S64_C(9223372036854775807))

#define U8_MAX (255)
#define U16_MAX (65535)
#define U32_MAX (4294967295U)
#define U64_MAX (U64_C(18446744073709551615))

#define S128_MIN s128(S64_MIN, 0)
#define S128_MAX s128(S64_MAX, U64_MAX)
#define U128_MAX u128(U64_MAX, U64_MAX)

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

using f32 = float;
using f64 = double;

constexpr s32 F64_DECIMAL_DIG = 17;                      // # of decimal digits of rounding precision
constexpr s32 F64_DIG         = 15;                      // # of decimal digits of precision
constexpr f64 F64_EPSILON     = 2.2204460492503131e-016; // smallest such that 1.0 + F64_EPSILON != 1.0

constexpr s32 F64_MANT_BITS = 52; // # of bits in mantissa, excluding the hidden bit (which is always interpreted as 1 for normal numbers)

constexpr s32 F64_EXP_BITS   = 11;    // # of bits in exponent
constexpr s32 F64_EXP_BIAS   = 1023;  // also called zero offset
constexpr s32 F64_MAX_EXP    = 1023;  // max binary exponent
constexpr s32 F64_MIN_EXP    = -1022; // min binary exponent
constexpr s32 F64_MAX_10_EXP = 308;   // max decimal exponent
constexpr s32 F64_MIN_10_EXP = -307;  // min decimal exponent

constexpr f64 F64_MAX = 1.7976931348623158e+308; // max value
constexpr f64 F64_MIN = 2.2250738585072014e-308; // min positive value

constexpr f64 F64_TRUE_MIN = 4.9406564584124654e-324; // min positive value, denormal

constexpr s32 F32_DECIMAL_DIG = 9;                // # of decimal digits of rounding precision
constexpr s32 F32_DIG         = 6;                // # of decimal digits of precision
constexpr f64 F32_EPSILON     = 1.192092896e-07F; // smallest such that 1.0 + F64_EPSILON != 1.0

constexpr s32 F32_MANT_BITS = 23; // # of bits in mantissa, excluding the hidden bit (which is always interpreted as 1 for normal numbers)

constexpr s32 F32_EXP_BITS   = 8;    // # of bits in exponent
constexpr s32 F32_EXP_BIAS   = 127;  // also called zero offset
constexpr s32 F32_MAX_EXP    = 127;  // max binary exponent
constexpr s32 F32_MIN_EXP    = -126; // min binary exponent
constexpr s32 F32_MAX_10_EXP = 38;   // max decimal exponent
constexpr s32 F32_MIN_10_EXP = -37;  // min decimal exponent

constexpr f64 F32_MAX = 3.402823466e+38F; // max value
constexpr f64 F32_MIN = 1.175494351e-38F; // min positive value

constexpr f64 F32_TRUE_MIN = 1.401298464e-45F; // min positive value, denormal

//
// Vector types (aligned on 16 byte boundaries for SIMDs)
//

// @TODO: When we do the mangled name shit, use this.
// template <typename T, s64 Count>
// union alignas(16) base_vector_type { T Values[Count]; };

union alignas(16) u8v16 {
    u8 Values[16];
};

union alignas(16) u16v8 {
    u16 Values[8];
};

union alignas(16) u32v4 {
    u32 Values[4];
};

union alignas(16) u64v2 {
    u64 Values[2];
};

union alignas(16) s8v16 {
    s8 Values[16];
};

union alignas(16) s16v8 {
    s16 Values[8];
};

union alignas(16) s32v4 {
    s32 Values[4];
};

union alignas(16) s64v2 {
    s64 Values[2];
};

// using u8v16 = base_vector_type<u8, 16>;
// using u16v8 = base_vector_type<u16, 8>;
// using u32v4 = base_vector_type<u32, 4>;
// using u64v2 = base_vector_type<u64, 2>;
//
// using s8v16 = base_vector_type<s8, 16>;
// using s16v8 = base_vector_type<s16, 8>;
// using s32v4 = base_vector_type<s32, 4>;
// using s64v2 = base_vector_type<s64, 2>;

union alignas(8) f32v2 {
    f32 Values[2];
};

union alignas(16) f32v4 {
    f32 Values[4];
};

union alignas(16) f64v2 {
    f64 Values[2];
};

// using f32v4 = base_vector_type<f32, 4>;
// using f64v2 = base_vector_type<f64, 2>;
