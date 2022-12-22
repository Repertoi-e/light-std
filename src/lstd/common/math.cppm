module;

#include "namespace.h"

//
// This file defines some common math functions:
//    sign_bit, sign_no_zero, sign, copy_sign,
//    is_nan, is_signaling_nan, is_infinite, is_finite,
//    cast_numeric_safe,
//    min, max, clamp,
//    is_pow_of_2, ceil_pow_of_2, const_exp10
//    abs
//

export module lstd.math;

export import lstd.type_info;
export import lstd.ieee;

LSTD_BEGIN_NAMESPACE

export {
    bool sign_bit(is_signed_integral auto x) { return x < 0; }
    bool sign_bit(is_unsigned_integral auto) { return false; }

    bool sign_bit(f32 x) { return ieee754_f32{ x }.ieee.S; }
    bool sign_bit(f64 x) { return ieee754_f64{ x }.ieee.S; }

    // Returns -1 if x is negative, 1 otherwise
    s32 sign_no_zero(is_scalar auto x) { return sign_bit(x) ? -1 : 1; }

    // Returns -1 if x is negative, 1 if positive, 0 otherwise
    s32 sign(is_scalar auto x) {
        if (x == decltype(x)(0)) return 0;
        return sign_no_zero(x);
    }

    template <is_floating_point T>
    T copy_sign(T x, T y) {
        if constexpr (sizeof x == sizeof f32) {
            ieee754_f32 formatx = { x }, formaty = { y };
            formatx.ieee.S = formaty.ieee.S;
            return formatx.F;
        }
        else {
            ieee754_f64 formatx = { x }, formaty = { y };
            formatx.ieee.S = formaty.ieee.S;
            return formatx.F;
        }
    }

    bool is_nan(is_floating_point auto x) {
        if constexpr (sizeof x == sizeof f32) {
            ieee754_f32 format = { x };
            return format.ieee.E == 0xFF && format.ieee.M != 0;
        }
        else {
            ieee754_f64 format = { x };
            return format.ieee.E == 0x7FF && (format.ieee.M0 != 0 || format.ieee.M1 != 0);
        }
    }

    bool is_signaling_nan(is_floating_point auto x) {
        if constexpr (sizeof x == sizeof f32)
            return is_nan(x) && ieee754_f32 { x }.ieee_nan.N == 0;
        else
            return is_nan(x) && ieee754_f64 { x }.ieee_nan.N == 0;
    }

    bool is_infinite(is_floating_point auto x) {
        if constexpr (sizeof x == sizeof f32) {
            ieee754_f32 format = { x };
            return format.ieee.E == 0xFF && format.ieee.M == 0;
        }
        else {
            ieee754_f64 format = { x };
            return format.ieee.E == 0x7FF && format.ieee.M0 == 0 && format.ieee.M1 == 0;
        }
    }

    bool is_finite(is_floating_point auto x) {
        if constexpr (sizeof x == sizeof f32)
            return ieee754_f32{ x }.ieee.E != 0xFF;
        else
            return ieee754_f64{ x }.ieee.E != 0x7FF;
    }
}

template <typename T, typename U>
concept are_same_signage = is_signed_integral<T> && is_signed_integral<U> || is_unsigned_integral<T> && is_unsigned_integral<U>;

template <typename T, typename U>
requires(is_scalar<T> && is_scalar<U>) auto cast_numeric_safe(U y) {
    if constexpr (is_floating_point<T>) {
        static_assert(is_integral<U> || sizeof(T) >= sizeof(U), "T is a float. U must be a float of the same size or smaller, or an integer. Otherwise information may be lost when casting.");
    } else if constexpr (is_integral<T> && is_integral<U>) {
        static_assert(sizeof(T) > sizeof(U) || sizeof(T) == sizeof(U) && are_same_signage<T, U>, "Both T and U are integers. T must be larger than U, or if they have the same size, they must have the same signage. Otherwise information may be lost when casting.");
    } else {
		// XXX TEMP static_assert(false, "T was an integer, but U was a floating point. Information may be lost when casting.");
    }
    return (T) y;
}

auto min_(auto x, auto y) {
    auto y_casted = cast_numeric_safe<decltype(x)>(y);
    if constexpr (is_floating_point<decltype(x)>) {
        if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x < y_casted ? x : y_casted;
}

auto max_(auto x, auto y) {
    auto y_casted = cast_numeric_safe<decltype(x)>(y);
    if constexpr (is_floating_point<decltype(x)>) {
        if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x > y_casted ? x : y_casted;
}

export {

    template <is_scalar... Args>
    auto min(is_scalar auto x, Args... rest) {
        auto result = x;
        ((void)(result = min_(result, rest)), ...);
        return result;
    }

    template <is_scalar... Args>
    auto max(is_scalar auto x, Args... rest) {
        auto result = x;
        ((void)(result = max_(result, rest)), ...);
        return result;
    }

    // Returns lower if x < lower, return upper if x > upper, returns x otherwise
    auto clamp(auto x, auto lower, auto upper) { return max(lower, min(upper, x)); }

    // Checks if x is a power of 2
    bool is_pow_of_2(is_integral auto x) { return (x & x - 1) == 0; }

    // Returns the smallest power of 2 bigger or equal to x.
    template <is_integral T>
    auto ceil_pow_of_2(T x) {
        if (x <= 1) return T(1);

        T power = 2;
        --x;
        while (x >>= 1) power <<= 1;
        return power;
    }

    // Returns 10 ** exp at compile-time. Uses recursion.
    template <typename T>
    T const_exp10(s32 exp) { return exp == 0 ? T(1) : T(10) * const_exp10<T>(exp - 1); }

    auto abs(is_scalar auto x) {
        if constexpr (is_floating_point<decltype(x)>) {
            if constexpr (sizeof x == sizeof f32) {
                ieee754_f32 u = { x };
                u.ieee.S = 0;
                return u.F;
            }
            else {
                ieee754_f64 u = { x };
                u.ieee.S = 0;
                return u.F;
            }
        }
        else {
            if constexpr (is_unsigned_integral<decltype(x)>) {
                return x;  // Unsigned integrals are always positive
            }
            else {
                return x < 0 ? -x : x;
            }
        }
    }
}

LSTD_END_NAMESPACE
