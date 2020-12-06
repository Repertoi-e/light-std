#pragma once

//
// This file defines common useful functions that work on scalars (integrals or floating point types)
//

#include "floating_point.h"

// @NoCRT @DependencyCleanup Replace with our own math trig/sqrt etc. functions in order to not depend on the runtime lib

LSTD_BEGIN_NAMESPACE

template <types::is_integral T>
constexpr T is_pow_of_2(T number) { return (number & (number - 1)) == 0; }

template <types::is_integral T>
constexpr T const_abs(T number) {
    auto s = number >> (sizeof(T) * 8 - 1);
    return (number ^ s) - s;
}

constexpr f32 const_abs(f32 number) {
    ieee754_f32 format = {number};
    format.ieee.S = 0;
    return format.F;
}

constexpr f64 const_abs(f64 number) {
    ieee754_f64 format = {number};
    format.ieee.S = 0;
    return format.F;
}

template <types::is_signed_integral T>
constexpr bool sign_bit(T number) {
    return number < 0;
}

template <types::is_unsigned_integral T>
constexpr bool sign_bit(T) {
    return false;
}

constexpr bool sign_bit(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.S;
}

constexpr bool sign_bit(f64 number) {
    ieee754_f64 format = {number};
    return format.ieee.S;
}

// Returns -1 if number is negative, 1 otherwise
template <typename T>
constexpr s32 sign_no_zero(T number) {
    return sign_bit(number) ? -1 : 1;
}

// Handles zero as well
template <typename T>
constexpr s32 sign(T number) {
    if (number == T(0)) return 0;
    return sign_bit(number) ? -1 : 1;
}

constexpr bool is_inf(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.E == 0xff && format.ieee.M == 0;
}

constexpr bool is_inf(f64 number) {
    ieee754_f64 format = {number};
#if ENDIAN == BIG_ENDIAN
    // @Wrong
    // Haven't tested this
    return ((u32) format.W & 0xffffff7f) == 0x0000f07f && ((u32)(format.W >> 32) == 0);
#else
    return ((u32)(format.W >> 32) & 0x7fffffff) == 0x7ff00000 && ((u32) format.W == 0);
#endif
}

constexpr bool is_nan(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.E == 0xff && format.ieee.M != 0;
}

constexpr bool is_nan(f64 number) {
    ieee754_f64 format = {number};
#if ENDIAN == BIG_ENDIAN
    // @Wrong
    // Haven't tested this
    return ((u32) format.W & 0xffffff7f) + ((u32)(format.W >> 32) != 0) > 0x0000f07f;
#else
    return ((u32)(format.W >> 32) & 0x7fffffff) + ((u32) format.W != 0) > 0x7ff00000;
#endif
}

// rad / tau * 360
// deg * tau / 360
// @Robustness Accurate enough?
#define TAU 6.2831853f
#define PI 3.1415926f
#define EULER 2.7182818f
#define SQRT2 1.4142135f

// Returns 10 ** exponent at compile-time.
template <typename T>
constexpr T const_exp10(s32 exponent) {
    return exponent == 0 ? T(1) : T(10) * const_exp10<T>(exponent - 1);
}

template <typename T>
always_inline constexpr T const_min(T x, T y) {
    return x < y ? x : y;
}

template <typename T>
always_inline constexpr T const_max(T x, T y) {
    return x > y ? x : y;
}

template <typename T>
always_inline constexpr T min(T x, T y) {
    return x < y ? x : y;
}

template <typename T>
always_inline constexpr T max(T x, T y) {
    return x > y ? x : y;
}

// Defined in internal.cpp
f32 min(f32 x, f32 y);
f32 max(f32 x, f32 y);
f64 min(f64 x, f64 y);
f64 max(f64 x, f64 y);

template <typename T>
always_inline T clamp(T value, T lower, T upper) {
    return max(lower, min(upper, value));
}

template <types::is_integral T>
T ceil_pow_of_2(T v) {
    if (v <= 1) return 1;
    T power = 2;
    --v;
    while (v >>= 1) power <<= 1;
    return power;
}

LSTD_END_NAMESPACE
