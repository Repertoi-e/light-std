#pragma once

#include "../internal/floating_point.h"
#include "../types/numeric_info.h"
#include "constants.h"

LSTD_BEGIN_NAMESPACE

//
// This file defines common functions that work on scalars (integrals or floating point types).
//
// Includes functions that normally require the C runtime library. There is a point to be made on why that's not very good -
// different platforms have different implementations of the standard library, that means that your sin function suddenly changes
// when you switch from Windows to Linux... We implement our own functions in order to not rely on the runtime library at all,
// but this also means that you have one implementation for all platforms.
//
// !!!
// The implementations of some functions are based on the Cephes library.
// The changes are documented in the source files of each function respectively.
//
// Cephes Math Library Release 2.8: June, 2000
// Copyright 1985, 1995, 2000 by Stephen L. Moshier
// !!!
//
//
// Here is the list of functions included by this header:
//
// *return type* | *name* (equivalent in CRT - if it exists) | -> *note*
// --------------|-------------------------------------------|-----------------
// bool                    sign_bit            (signbit)       -> works for both integers and floats
// s32                     sign_no_zero                        -> returns -1 if x is negative, 1 otherwise
// s32                     sign                                -> returns -1 if x is negative, 1 if positive, 0 otherwise
//
// f64                     copy_sign                           -> copies the sign of y to x; only portable way to manipulate NaNs
//
// bool                    is_nan,             (isnan)
// bool                    is_signaling_nan
// bool                    is_infinite,        (isinf)
// bool                    is_finite,          (isfinite)      -> !is_infinite(x) && !is_nan(x)
//
// T                       min *                               -> also supports variable number of arguments
// T                       max *                               -> also supports variable number of arguments
// T                       clamp *                             -> clamps x between two values
// T                       abs *
//
// bool                    is_pow_of_2                         -> checks if x is a power of 2
// T                       ceil_pow_of_2                       -> returns the smallest power of 2 bigger or equal to x
//
// T                       const_exp10                         -> evaluates 10 ** exp at compile-time, using recursion
//
// f64                     pow *
// f64                     sqrt *
// f64                     ln (log)                            -> returns the natural logarithm of x
//                         ^^^^^^^^ Function is called ln instead of just log, because that name
//                                  is more logical to me (and follows math notation).
//
// f64                     log2 *
// f64                     log10 *
// f64                     sin *
// f64                     cos *
// f64                     asin *
// f64                     acos *
//
// f64                     ceil *
// f64                     floor *
// f64                     round *
//
// decompose_float_result  fraction_exponent                   -> decomposes a float into a normalized fraction and an integer exponent
// f64                     load_exponent                       -> multiplies a float by 2 ** exp
//                         ^^^^^^^^^^^^^^^^^
//                         These two functions are inverses of each other and are used to modify floats without messing with the bits.
//
//
// * These functions share names with the C++ standard library.
//   We are using a namespace by default so this should not cause unresolvable name conflicts.
//
//
// THIS IS NOT A COMPLETE REPLACEMENT FOR ALL FUNCTIONS IN THE STANDARD LIBRARY.
// When we need a certain function, we implement is as we go.
// If we attempt to make a super general math library that has all the functions in the world, we will never complete it.
// This also forces us to think which functions are *very* useful and get used a lot and which are just boilerplate that
// may get used once in one remote corner in a program in a very specific case.
//

//
// Note: A quick reminder on how IEEE 754 works.
//
// Here is how a 32 bit float looks in memory:
//
//      31
//      |
//      | 30    23 22                    0
//      | |      | |                     |
// -----+-+------+-+---------------------+
// qnan 0 11111111 10000000000000000000000
// snan 0 11111111 01000000000000000000000
//  inf 0 11111111 00000000000000000000000
// -inf 1 11111111 00000000000000000000000
// -----+-+------+-+---------------------+
//      | |      | |                     |
//      | +------+ +---------------------+
//      |    |               |
//      |    v               v
//      | exponent        mantissa
//      |
//      v
//      sign
//
// Inf is defined to be any x with exponent == 0xFF and mantissa == 0
// NaN is defined to be any x with exponent == 0xFF and mantissa != 0
//
// "SNaNs (signaling NaNs) are typically used to trap or invoke an exception handler.
//  They must be inserted by software; that is, the processor never generates an SNaN as a result of a floating-point operation."
//                                                          - Intel manual, 4.8.3.4
//
// QNans (quiet NaNs) and SNanS are differentiated by the 22th bit. For this particular example the 23th bit
// for sNaN is set because otherwise the mantissa would be 0 (and that is interpreted as infinity by definition).
//

constexpr bool sign_bit(types::is_signed_integral auto x) { return x < 0; }
constexpr bool sign_bit(types::is_unsigned_integral auto) { return false; }

constexpr bool sign_bit(f32 x) { return ieee754_f32{x}.ieee.S; }
constexpr bool sign_bit(f64 x) { return ieee754_f64{x}.ieee.S; }

// Returns -1 if x is negative, 1 otherwise
constexpr s32 sign_no_zero(types::is_scalar auto x) { return sign_bit(x) ? -1 : 1; }

// Returns -1 if x is negative, 1 if positive, 0 otherwise
constexpr s32 sign(types::is_scalar auto x) {
    if (x == decltype(x)(0)) return 0;
    return sign_no_zero(x);
}

template <types::is_floating_point T>
constexpr T copy_sign(T x, T y) {
    if constexpr (sizeof(x) == 4) {
        ieee754_f32 formatx = {x}, formaty = {y};
        formatx.ieee.S = formaty.ieee.S;
        return formatx.F;
    } else {
        ieee754_f64 formatx = {x}, formaty = {y};
        formatx.ieee.S = formaty.ieee.S;
        return formatx.F;
    }
}

constexpr bool is_nan(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == 4) {
        ieee754_f32 format = {x};
        return format.ieee.E == F32_MAX_EXP && format.ieee.M != 0;
    } else {
        ieee754_f64 format = {x};
        return format.ieee.E == F64_MAX_EXP && (format.ieee.M0 != 0 || format.ieee.M1 != 0);
    }
}

constexpr bool is_signaling_nan(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == 4)
        return is_nan(x) && ieee754_f32{x}.ieee_nan.N == 0;
    else
        return is_nan(x) && ieee754_f64{x}.ieee_nan.N == 0;
}

constexpr bool is_infinite(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == 4) {
        ieee754_f32 format = {x};
        return format.ieee.E == F32_MAX_EXP && format.ieee.M == 0;
    } else {
        ieee754_f64 format = {x};
        return format.ieee.E == F64_MAX_EXP && format.ieee.M0 == 0 && format.ieee.M1 == 0;
    }
}

constexpr bool is_finite(types::is_floating_point auto x) {
    if constexpr (sizeof(x) == 4)
        return ieee754_f32{x}.ieee.E != F32_MAX_EXP;
    else
        return ieee754_f64{x}.ieee.E != F64_MAX_EXP;
}

template <typename T, typename U>
concept are_same_signage = (types::is_signed_integral<T> && types::is_signed_integral<U>) || (types::is_unsigned_integral<T> && types::is_unsigned_integral<U>);

template <typename T, typename U>
requires(types::is_scalar<T> &&types::is_scalar<U>) constexpr T cast_numeric_safe(U y) {
    if constexpr (types::is_floating_point<T>) {
        static_assert((types::is_integral<U> || sizeof(T) >= sizeof(U)), "T is a float. U must be a float of the same size or smaller, or an integer. Otherwise information may be lost when casting.");
    } else if constexpr (types::is_integral<T> && types::is_integral<U>) {
        static_assert(sizeof(T) > sizeof(U) || (sizeof(T) == sizeof(U) && are_same_signage<T, U>), "Both T and U are integers. T must be larger than U, or if they have the same size, they must have the same signage. Otherwise information may be lost when casting.");
    } else {
        static_assert(false, "T was an integer, but U was a floating point. Information may be lost when casting.");
    }
    return (T) y;
}

constexpr auto min_(auto x, auto y) {
    auto y_casted = cast_numeric_safe<decltype(x)>(y);
    if constexpr (types::is_floating_point<decltype(x)>) {
        if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x < y_casted ? x : y_casted;
}

constexpr auto max_(auto x, auto y) {
    auto y_casted = cast_numeric_safe<decltype(x)>(y);
    if constexpr (types::is_floating_point<decltype(x)>) {
        if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x > y_casted ? x : y_casted;
}

template <types::is_scalar... Args>
constexpr auto min(types::is_scalar auto x, Args... rest) {
    auto result = x;
    ((void) (result = min_(result, rest)), ...);
    return result;
}

template <types::is_scalar... Args>
constexpr auto max(types::is_scalar auto x, Args... rest) {
    auto result = x;
    ((void) (result = max_(result, rest)), ...);
    return result;
}

// Returns lower if x < lower, return upper if x > upper, returns x otherwise
constexpr always_inline auto clamp(auto x, auto lower, auto upper) { return max(lower, min(upper, x)); }

constexpr auto abs(types::is_scalar auto x) {
    if constexpr (types::is_floating_point<decltype(x)>) {
        if constexpr (sizeof(x) == 4) {
            ieee754_f32 u = {x};
            u.ieee.S = 0;
            return u.F;
        } else {
            ieee754_f64 u = {x};
            u.ieee.S = 0;
            return u.F;
        }
    } else {
        if constexpr (types::is_unsigned_integral<decltype(x)>) {
            return x;  // Unsigned integrals are always positive
        } else {
            return x < 0 ? -x : x;
        }
    }
}

// Checks if x is a power of 2
constexpr bool is_pow_of_2(types::is_integral auto x) { return (x & (x - 1)) == 0; }

// Returns the smallest power of 2 bigger or equal to x.
template <types::is_integral T>
constexpr auto ceil_pow_of_2(T x) {
    if (x <= 1) return T(1);

    T power = 2;
    --x;
    while (x >>= 1) power <<= 1;
    return power;
}

// Returns 10 ** exp at compile-time. Uses recursion.
template <typename T>
constexpr T const_exp10(s32 exp) { return exp == 0 ? T(1) : T(10) * const_exp10<T>(exp - 1); }

//
// Math functions
//

constexpr f64 pow(f64 a, f64 x);                                   // Returns a ** x
constexpr f64 pow(f64 a, types::is_integral auto x) { return 0; }  // Returns a ** x (where x is an integer)
constexpr f64 exp(f64 x) { return 0; }                             // Returns e ** x
constexpr f64 exp(types::is_integral auto x);                      // Returns e ** x (where x is an integer)
constexpr f64 sqrt(f64 x) { return 0; }                            // Returns the square root of x
constexpr f64 ln(f64 x) { return 0; }                              // Returns the natural logarithm (log_e)  of x
constexpr f64 log2(f64 x) { return 0; }                            // Returns the log_2 of x
constexpr f64 log10(f64 x) { return 0; }                           // Returns the log_10 of x

always_inline constexpr f32 pow(f32 a, f32 x) { return (f32) pow((f64) a, (f64) x); }
always_inline constexpr f32 pow(f32 a, types::is_integral auto x) { return (f32) pow((f64) a, x); }
always_inline constexpr f32 exp(f32 x) { return (f32) exp((f64) x); }
always_inline constexpr f32 exp(types::is_integral auto x) { return (f32) exp(x); }
always_inline constexpr f32 sqrt(f32 x) { return (f32) sqrt((f64) x); }
always_inline constexpr f32 log2(f32 x) { return (f32) log2((f64) x); }
always_inline constexpr f32 log10(f32 x) { return (f32) log10((f64) x); }

constexpr f64 sin(f64 x);
constexpr f64 cos(f64 x);
constexpr f64 asin(f64 x) { return 0; }
constexpr f64 acos(f64 x) { return 0; }

always_inline constexpr f32 sin(f32 x) { return (f32) sin((f64) x); }
always_inline constexpr f32 cos(f32 x) { return (f32) cos((f64) x); }
always_inline constexpr f32 asin(f32 x) { return (f32) asin((f64) x); }
always_inline constexpr f32 acos(f32 x) { return (f32) acos((f64) x); }

// Returns the smallest integer greater than or equal to x.
// It truncates toward +Inf.
constexpr f64 ceil(f64 x);
always_inline constexpr f32 ceil(f32 x) { return (f32) ceil((f64) x); }

// Returns the largest integer less than or equal to x.
// It truncates toward -Inf.
constexpr f64 floor(f64 x);
always_inline constexpr f32 floor(f32 x) { return (f32) floor((f64) x); }

// Returns the nearest integer to x
constexpr f64 round(f64 x);
always_inline constexpr f32 round(f32 x) { return (f32) round((f64) x); }

//
// Defined in frexp_ldexp.cpp:
//

// Decomposes given floating point value into a normalized fraction
// and an integral power of two such that x = _Fraction_ * 2 ** _Exponent_.
// _Fraction_ is in the range (-1; -0.5] or [0.5; 1).
//
// If the arg was 0, both returned values are 0.
// If the arg was not finite, it is returned in _Fraction_ and _Exponent_ is unspecified.
struct decompose_float_result {
    f64 Fraction;
    s32 Exponent;
};

// (frexp) The inverse of load_exponent.
// Decomposes a float into a fraction and an exponent. See note above _decompose_float_result_.
//
// These two functions are used to modify floats without messing with the bits.
constexpr decompose_float_result fraction_exponent(f64 x);

// (ldexp) The reverse of _fraction_exponent_.
// Multiplies x by 2 to the power of _exp_.
//
// These two functions are used to modify floats without messing with the bits.
constexpr f64 load_exponent(f64 x, s32 n);

// This template function unrolls a loop at compile-time.
// The lambda should take "auto it" as a parameter and
// that can be used as a compile-time constant index.
//
// This is useful for when you can just write a for-loop
// instead of using template functional recursive programming.
//
// @Cleanup: This was in common.h but we use this here (but this file is included in common.h).
template <s64 First, s64 Last, typename Lambda>
void static_for(Lambda &&f) {
    if constexpr (First < Last) {
        f(types::integral_constant<s64, First>{});
        static_for<First + 1, Last>(f);
    }
}

// @Cleanup: This is used frequently. Where is the best place to put it?
namespace scalar_math_internal {
template <s64 N>
constexpr f64 poleval(f64 x, const f64 c[]) {
    f64 result = c[0];
    // Unrolls loop at compile-time
    static_for<1, N>([&](auto i) {
        result = result * x + c[i];
    });
    return result;
}

// Same as poleval but treats the coeff in front of x^n as 1.
template <s64 N>
constexpr f64 poleval_1(f64 x, const f64 c[]) {
    f64 result = x + c[1];
    // Unrolls loop at compile-time
    static_for<2, N>([&](auto i) {
        result = result * x + c[i];
    });
    return result;
}
}  // namespace scalar_math_internal

LSTD_END_NAMESPACE

#include "ceil_floor_round.h"
#include "frexp_ldexp.h"
#include "pow_exp.h"
#include "sin_cos.h"
