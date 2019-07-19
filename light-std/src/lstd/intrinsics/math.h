#pragma once

/// Provides common useful short functons (my definition for instrinsics) that work with numbers

#include <intrin.h>
#include "float_spec.h"

#define PI 3.1415926535897932384626433832795
#define PI_OVER_2 1.57079632679489661923   // pi/2
#define PI_OVER_4 0.785398163397448309616  // pi/4

#define LN_BASE 2.71828182845904523536  // e

#define TAU 6.283185307179586476925286766559

#define LOG2 0.69314718055994530941723212145818
#define LOG10 2.30258509299404568402

#define LOG2E 1.44269504088896340736    // log2(e)
#define LOG10E 0.434294481903251827651  // log10(e)

#define SQRT2 1.41421356237309504880       // sqrt(2)
#define INV_SQRT2 0.707106781186547524401  // 1/sqrt(2)

#define POWERS_OF_10(factor)                                                                                        \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, \
        factor * 100000000, factor * 1000000000

constexpr u32 POWERS_OF_10_32[] = {1, POWERS_OF_10(1)};
constexpr u64 POWERS_OF_10_64[] = {1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

constexpr u32 ZERO_OR_POWERS_OF_10_32[] = {0, POWERS_OF_10(1)};
constexpr u64 ZERO_OR_POWERS_OF_10_64[] = {0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};
#undef POWERS_OF_10

#if COMPILER == MSVC
#pragma intrinsic(_BitScanReverse)
inline u32 MSB(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}

#pragma intrinsic(_BitScanReverse64)
inline u32 MSB_64(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse64(&r, x);
    return 63 - r;
}

#pragma intrinsic(_BitScanForward)
inline u32 LSB(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanForward(&r, x);
    return r;
}

#pragma intrinsic(_BitScanForward64)
inline u32 LSB_64(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanForward64(&r, x);
    return r;
}
#else
#define MSB(n) __builtin_clz(n)
#define MSB_64(n) __builtin_clzll(n)
#define LSB(n) __builtin_ctz(n)
#define LSB_64(n) __builtin_ctzll(n)
#endif

constexpr u32 ROTATE_LEFT_32(u32 x, u32 bits) { return (x << bits) | (x >> (32 - bits)); }
constexpr u64 ROTATE_LEFT_64(u64 x, u32 bits) { return (x << bits) | (x >> (64 - bits)); }

constexpr u32 ROTATE_RIGHT_32(u32 x, u32 bits) { return (x >> bits) | (x << (32 - bits)); }
constexpr u64 ROTATE_RIGHT_64(u64 x, u32 bits) { return (x >> bits) | (x << (64 - bits)); }

#define INTEGRAL_FUNCTION_CONSTEXPR(return_type) \
    template <typename T>                        \
    constexpr enable_if_t<is_integral_v<T>, return_type>

INTEGRAL_FUNCTION_CONSTEXPR(bool) IS_POW_OF_2(T number) { return (number & (number - 1)) == 0; }

INTEGRAL_FUNCTION_CONSTEXPR(T) CEIL_TO_POWER_OF_2(T number, T pow2) {
    static_assert(IS_POW_OF_2(pow2), "Argument is not a power of 2");
    return (number + pow2 - 1) & ~(pow2 - 1);
}

INTEGRAL_FUNCTION_CONSTEXPR(T) ABS(T number) {
    auto s = number >> (sizeof(T) * 8 - 1);
    return (number ^ s) - s;
}

#undef INTEGRAL_FUNCTION_CONSTEXPR

constexpr f32 ABS(f32 number) {
    ieee754_f32 format = {number};
    format.ieee.S = 0;
    return format.F;
}

constexpr f64 ABS(f64 number) {
    ieee754_f64 format = {number};
    format.ieee.S = 0;
    return format.F;
}

constexpr bool IS_INF(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.E == 0xff && format.ieee.M == 0;
}

constexpr bool IS_INF(f64 number) {
    ieee754_f64 format = {number};
#if ENDIAN == BIG_ENDIAN
    // @Wrong
    // Haven't tested this
    return ((u32) format.U & 0xffffff7f) == 0x0000f07f && ((u32)(format.U >> 32) == 0);
#else
    return ((u32)(format.U >> 32) & 0x7fffffff) == 0x7ff00000 && ((u32) format.U == 0);
#endif
}

constexpr bool IS_NAN(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.E == 0xff && format.ieee.M != 0;
}

constexpr bool IS_NAN(f64 number) {
    ieee754_f64 format = {number};
#if ENDIAN == BIG_ENDIAN
    // @Wrong
    // Haven't tested this
    return ((u32) format.U & 0xffffff7f) + ((u32)(format.U >> 32) != 0) > 0x0000f07f;
#else
    return ((u32)(format.U >> 32) & 0x7fffffff) + ((u32) format.U != 0) > 0x7ff00000;
#endif
}

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
constexpr enable_if_t<is_integral_v<T> && numeric_info<T>::is_signed, bool> IS_NEG(T value) {
    return value < 0;
}

template <typename T>
constexpr enable_if_t<is_integral_v<T> && !numeric_info<T>::is_signed, bool> IS_NEG(T) {
    return false;
}

constexpr bool IS_NEG(f32 number) {
    ieee754_f32 format = {number};
    return format.ieee.S;
}

constexpr bool IS_NEG(f64 number) {
    ieee754_f64 format = {number};
    return format.ieee.S;
}

template <typename T>
T MAX(T x, T y) {
    return x > y ? x : y;
}

template <typename T>
T MIN(T x, T y) {
    return x < y ? x : y;
}

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline u32 COUNT_DIGITS(u64 n) {
    s32 t = (64 - MSB_64(n | 1)) * 1233 >> 12;
    return (u32) t - (n < ZERO_OR_POWERS_OF_10_64[t]) + 1;
}

template <u32 Bits, typename T>
constexpr u32 COUNT_DIGITS(T value) {
    T n = value;
    u32 numDigits = 0;
    do {
        ++numDigits;
    } while ((n >>= Bits) != 0);
    return numDigits;
}
