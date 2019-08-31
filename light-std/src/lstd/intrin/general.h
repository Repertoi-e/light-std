#pragma once

/// Provides common useful short functons (my definition for instrinsics) that work with numbers

#include <intrin.h>

#include "../common.h"

#define POWERS_OF_10(factor)                                                                                        \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, \
        factor * 100000000, factor * 1000000000

constexpr u32 POWERS_OF_10_32[] = {1, POWERS_OF_10(1)};
constexpr u64 POWERS_OF_10_64[] = {1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

constexpr u32 ZERO_OR_POWERS_OF_10_32[] = {0, POWERS_OF_10(1)};
constexpr u64 ZERO_OR_POWERS_OF_10_64[] = {0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};
#undef POWERS_OF_10

union ieee754_f32 {
    f32 F;
    u32 U;

    // This is the IEEE 754 single-precision format.
    struct {
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
    struct {
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

union ieee754_f64 {
    f64 F;
    u64 U;

    // This is the IEEE 754 single-precision format.
    struct {
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
    struct {
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

// @Wrong
// This seems wrong, not sure.
// sizeof(ieee854_lf64) is 16 but sizeof(long double) in MSVC is 8
union ieee854_lf64 {
    lf64 F;
    u64 U;

    // This is the IEEE 854 double-extended-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 15;
        u32 Empty : 16;
        u32 M0 : 32;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 32;
        u32 E : 15;
        u32 S : 1;
        u32 Empty : 16;
#endif
    } ieee;

    // This is for NaNs in the IEEE 854 double-extended-precision format.
    struct {
#if ENDIAN == BIG_ENDIAN
        u32 S : 1;
        u32 E : 15;
        u32 Empty : 16;
        u32 One : 1;
        u32 N : 1;
        u32 M0 : 30;
        u32 M1 : 32;
#else
        u32 M1 : 32;
        u32 M0 : 30;
        u32 N : 1;
        u32 One : 1;
        u32 E : 15;
        u32 S : 1;
        u32 Empty : 16;
#endif
    } ieee_nan;
};

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

#define U32_HAS_ZERO(v) (((v) -0x01010101UL) & ~(v) &0x80808080UL)
#define U32_HAS_VALUE(x, n) (U32_HAS_ZERO((x) ^ (~0UL / 255 * (u8)(n))))

#define U32_HAS_LESS(x, n) (((x) - ~0UL / 255 * (u8)(n)) & ~(x) & ~0UL / 255 * 128)
#define U32_COUNT_LESS(x, n) \
    (((~0UL / 255 * (127 + (n)) - ((x) & ~0UL / 255 * 127)) & ~(x) & ~0UL / 255 * 128) / 128 % 255)

#define U32_HAS_MORE(x, n) (((x) + ~0UL / 255 * (127 - (u8)(n)) | (x)) & ~0UL / 255 * 128)
#define U32_COUNT_MORE(x, n) \
    (((((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(n)) | (x)) & ~0UL / 255 * 128) / 128 % 255)

#define U32_LIKELY_HAS_BETWEEN(x, m, n) \
    ((((x) - ~0UL / 255 * (u8)(n)) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(m))) & ~0UL / 255 * 128)
#define U32_HAS_BETWEEN(x, m, n)                                       \
    ((~0UL / 255 * (127 + (u8)(n)) - ((x) & ~0UL / 255 * 127) & ~(x) & \
      ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(m))) &       \
     ~0UL / 255 * 128)
#define U32_COUNT_BETWEEN(x, m, n) (hasbetween(x, m, n) / 128 % 255)

#define INTEGRAL_FUNCTION_CONSTEXPR(return_type) \
    template <typename T>                        \
    constexpr enable_if_t<is_integral_v<T>, return_type>

INTEGRAL_FUNCTION_CONSTEXPR(bool) IS_POW_OF_2(T number) { return (number & (number - 1)) == 0; }

INTEGRAL_FUNCTION_CONSTEXPR(T) ABS(T number) {
    auto s = number >> (sizeof(T) * 8 - 1);
    return (number ^ s) - s;
}

template <typename T>
enable_if_t<is_integral_v<T> && is_unsigned_v<T>, T> CEIL_POW_OF_2(T v) {
    v--;
    for (size_t i = 1; i < sizeof(T) * 8; i *= 2) v |= v >> i;
    return ++v;
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

inline s32 FLOOR(f32 x) { return (s32)(x + (f32) numeric_info<s32>::max()) - numeric_info<s32>::max(); }
inline s32 FLOOR(f64 x) { return (s32)(x + (f64) numeric_info<s32>::max()) - numeric_info<s32>::max(); }

inline s32 CEIL(f32 x) { return numeric_info<s32>::max() - (s32)((f32) numeric_info<s32>::max() - x); }
inline s32 CEIL(f64 x) { return numeric_info<s32>::max() - (s32)((f64) numeric_info<s32>::max() - x); }

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

// All of these return the value after the operation
#if COMPILER == MSVC
#define ATOMIC_INC(ptr) _InterlockedIncrement((ptr))
#define ATOMIC_INC_64(ptr) _InterlockedIncrement64((ptr))
#define ATOMIC_ADD(ptr, value) _InterlockedAdd((ptr), value)
#define ATOMIC_ADD_64(ptr, value) _InterlockedAdd64((ptr), value)
#else
#define ATOMIC_INC(ptr) __sync_add_and_fetch((ptr), 1)
#define ATOMIC_INC_64(ptr) __sync_add_and_fetch((ptr), 1)
#error ATOMIC_ADD and ATOMIC_ADD_64
#endif
