#pragma once

#include "math_types.h"

//
// The following integral types are defined: s8, s16, s32, s64 (and corresponding unsigned types: u8, u16, u32, u64)
//		as well as: f32 (float), f64 (double), null (nullptr), utf8 (char), byte (unsigned char)
//      as well as: null (same as nullptr)
//      as well as: initializer_list (if not included from the std)
//
// Note: We don't support long doubles (lf64) or operations with them. That's why we don't even provide type info.
//

// :AvoidSTDs:
// Normally initializer_list would be included but if we avoid using headers from the C++ STD we define our own implementation here.
// Note: You must tell us with a macro: LSTD_DONT_DEFINE_STD.
// 
// By default we avoid STDs (like in real life) but if e.g. a library relies on it we would get definition errors.
// In general this library can work WITH or WITHOUT the normal standard library.
#if defined LSTD_DONT_DEFINE_STD
#include <initializer_list>
#else
// Note: If you get many compile errors (but you have defined LSTD_DONT_DEFINE_STD).
// You probably need to define it globally, because not all headers from this library see the macro.

namespace std {
template <typename T>
struct initializer_list {
    const T *First = null;
    const T *Last = null;

    using value_type = T;
    using reference = const T &;
    using const_reference = const T &;
    using size_type = size_t;

    constexpr initializer_list() noexcept {}
    constexpr initializer_list(const T *first, const T *last) noexcept : First(first), Last(last) {}

    using iterator = const T *;
    using const_iterator = const T *;

    constexpr const T *begin() const noexcept { return First; }
    constexpr const T *end() const noexcept { return Last; }

    constexpr size_t size() const noexcept { return static_cast<size_t>(Last - First); }
};
}  // namespace std
#endif

LSTD_BEGIN_NAMESPACE

//
// Fundamental types:
//
using s8 = char;
using s16 = short;
using s32 = int;
// using s64 = long long; Defined in math_types.h.. to avoid circular dependencies

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned;
using u64 = unsigned long long;

using utf16 = wchar_t;  // There is char16_t but since utf16 is only used on Windows (or you should die in hell) and windows headers use wchar_t
using utf32 = char32_t;

// We use utf8 for bytes which are supposed to be encoded in utf8
// instead of using just char (which generally has the meaning of byte).
// We also define byte which is an unsigned type and can store [0-255].
// Strings in C++ are char* which makes them compatible with utf8*.
using utf8 = char;
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

#define WCHAR_MIN 0x0000
#define WCHAR_MAX 0xffff

using f32 = float;
using f64 = double;

#define F64_DECIMAL_DIG 17                    // # of decimal digits of rounding precision
#define F64_DIG 15                            // # of decimal digits of precision
#define F64_EPSILON 2.2204460492503131e-016   // smallest such that 1.0+F64_EPSILON != 1.0
#define F64_HAS_SUBNORM 1                     // type does support subnormal numbers
#define F64_MANT_DIG 53                       // # of bits in mantissa
#define F64_MAX 1.7976931348623158e+308       // max value
#define F64_MAX_10_EXP 308                    // max decimal exponent
#define F64_MAX_EXP 1024                      // max binary exponent
#define F64_MIN 2.2250738585072014e-308       // min positive value
#define F64_MIN_10_EXP (-307)                 // min decimal exponent
#define F64_MIN_EXP (-1021)                   // min binary exponent
#define F64_RADIX 2                           // exponent radix
#define F64_TRUE_MIN 4.9406564584124654e-324  // min positive value

#define F32_DECIMAL_DIG 9             // # of decimal digits of rounding precision
#define F32_DIG 6                     // # of decimal digits of precision
#define F32_EPSILON 1.192092896e-07F  // smallest such that 1.0+F32_EPSILON != 1.0
#define F32_HAS_SUBNORM 1             // type does support subnormal numbers
#define F32_GUARD 0
#define F32_MANT_DIG 24           // # of bits in mantissa
#define F32_MAX 3.402823466e+38F  // max value
#define F32_MAX_10_EXP 38         // max decimal exponent
#define F32_MAX_EXP 128           // max binary exponent
#define F32_MIN 1.175494351e-38F  // min normalized positive value
#define F32_MIN_10_EXP (-37)      // min decimal exponent
#define F32_MIN_EXP (-125)        // min binary exponent
#define F32_NORMALIZE 0
#define F32_RADIX 2                    // exponent radix
#define F32_TRUE_MIN 1.401298464e-45F  // min positive value

//
// Extra types:
//

// Personal preference
// I prefer to type null over nullptr but they are exactly the same
constexpr auto null = nullptr;

using null_t = decltype(nullptr);

template <typename T>
using initializer_list = std::initializer_list<T>;

LSTD_END_NAMESPACE
