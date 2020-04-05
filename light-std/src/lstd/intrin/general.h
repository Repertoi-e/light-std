#pragma once

/// Provides common useful short functons (my definition for instrinsics) that work with numbers

#include <intrin.h>
#include <math.h>  // For any other standard functions

#include "../common.h"

LSTD_BEGIN_NAMESPACE

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
    u32 W;

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
    u64 W;

    struct {
#if ENDIAN == BIG_ENDIAN
        u32 MSW;
        u32 LSW;
#else
        u32 LSW;
        u32 MSW;
#endif
    };

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
    u64 W;

    struct {
#if ENDIAN == BIG_ENDIAN
        u32 MSW;
        u32 LSW;
#else
        u32 LSW;
        u32 MSW;
#endif
    };

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
inline u32 msb(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}

#pragma intrinsic(_BitScanReverse64)
inline u32 msb_64(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse64(&r, x);
    return 63 - r;
}

#pragma intrinsic(_BitScanForward)
inline u32 lsb(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanForward(&r, x);
    return r;
}

#pragma intrinsic(_BitScanForward64)
inline u32 lsb_64(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanForward64(&r, x);
    return r;
}
#else
#define msb(n) __builtin_clz(n)
#define msb_64(n) __builtin_clzll(n)
#define lsb(n) __builtin_ctz(n)
#define lsb_64(n) __builtin_ctzll(n)
#endif

constexpr u32 rotate_left_32(u32 x, u32 bits) { return (x << bits) | (x >> (32 - bits)); }
constexpr u64 rotate_left_64(u64 x, u32 bits) { return (x << bits) | (x >> (64 - bits)); }

constexpr u32 rotate_right_32(u32 x, u32 bits) { return (x >> bits) | (x << (32 - bits)); }
constexpr u64 rotate_right_64(u64 x, u32 bits) { return (x >> bits) | (x << (64 - bits)); }

// Useful: http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign

#define u32_has_zero(v) (((v) -0x01010101UL) & ~(v) &0x80808080UL)
#define u32_has_value(x, n) (u32_has_zero((x) ^ (~0UL / 255 * (u8)(n))))

#define u32_has_less(x, n) (((x) - ~0UL / 255 * (u8)(n)) & ~(x) & ~0UL / 255 * 128)
#define u32_count_less(x, n) \
    (((~0UL / 255 * (127 + (n)) - ((x) & ~0UL / 255 * 127)) & ~(x) & ~0UL / 255 * 128) / 128 % 255)

#define u32_has_more(x, n) (((x) + ~0UL / 255 * (127 - (u8)(n)) | (x)) & ~0UL / 255 * 128)
#define u32_count_more(x, n) \
    (((((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(n)) | (x)) & ~0UL / 255 * 128) / 128 % 255)

#define u32_likely_has_between(x, m, n) \
    ((((x) - ~0UL / 255 * (u8)(n)) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(m))) & ~0UL / 255 * 128)
#define u32_has_between(x, m, n)                                       \
    ((~0UL / 255 * (127 + (u8)(n)) - ((x) & ~0UL / 255 * 127) & ~(x) & \
      ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(m))) &       \
     ~0UL / 255 * 128)
#define u32_count_between(x, m, n) (u32_has_between(x, m, n) / 128 % 255)

#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

template <typename T>
constexpr enable_if_t<is_integer_v<T>> set_bit(T *number, T bit, bool value) {
    auto enabled = (make_unsigned_t<T>) value;
    *number ^= (-enabled ^ *number) & bit;
}

#if COMPILER == MSVC
#pragma warning(pop)
#endif

#define INTEGRAL_FUNCTION_CONSTEXPR(return_type) \
    template <typename T>                        \
    constexpr enable_if_t<is_integral_v<T>, return_type>

INTEGRAL_FUNCTION_CONSTEXPR(bool) is_pow_of_2(T number) { return (number & (number - 1)) == 0; }

INTEGRAL_FUNCTION_CONSTEXPR(T) const_abs(T number) {
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

template <typename T>
constexpr enable_if_t<is_integral_v<T> && numeric_info<T>::is_signed, bool> sign_bit(T number) {
    return number < 0;
}

template <typename T>
constexpr enable_if_t<is_integral_v<T> && !numeric_info<T>::is_signed, bool> sign_bit(T) {
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

// Handles zero as well
template <typename T>
constexpr s32 sign(T number) {
    if (number == T(0)) return 0;
    return sign_bit(number) ? -1 : 1;
}

template <typename T>
constexpr s32 sign_no_zero(T number) {
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

template <typename T>
constexpr T const_min(T x, T y) {
    return x < y ? x : y;
}

template <typename T>
constexpr T const_max(T x, T y) {
    return x > y ? x : y;
}

template <typename T>
constexpr T min(T x, T y) {
    return x < y ? x : y;
}

template <typename T>
constexpr T max(T x, T y) {
    return x > y ? x : y;
}

inline f32 min(f32 x, f32 y) { return fminf(x, y); }
inline f32 max(f32 x, f32 y) { return fmaxf(x, y); }
inline f64 min(f64 x, f64 y) { return fmin(x, y); }
inline f64 max(f64 x, f64 y) { return fmax(x, y); }

template <typename T>
enable_if_t<is_integral_v<T> && is_unsigned_v<T>, T> ceil_pow_of_2(T v) {
    v--;
    for (size_t i = 1; i < sizeof(T) * 8; i *= 2) v |= v >> i;
    return ++v;
}

#undef INTEGRAL_FUNCTION_CONSTEXPR

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline u32 count_digits(u64 n) {
    s32 t = (64 - msb_64(n | 1)) * 1233 >> 12;
    return (u32) t - (n < ZERO_OR_POWERS_OF_10_64[t]) + 1;
}

template <u32 Bits, typename T>
constexpr u32 count_digits(T value) {
    T n = value;
    u32 numDigits = 0;
    do {
        ++numDigits;
    } while ((n >>= Bits) != 0);
    return numDigits;
}

// All of these return the value after the operation
#if COMPILER == MSVC
#define atomic_inc(ptr) _InterlockedIncrement((ptr))
#define atomic_inc_64(ptr) _InterlockedIncrement64((ptr))
#define atomic_add(ptr, value) _InterlockedAdd((ptr), value)
#define atomic_add_64(ptr, value) _InterlockedAdd64((ptr), value)
#else
#define atomic_inc(ptr) __sync_add_and_fetch((ptr), 1)
#define atomic_inc_64(ptr) __sync_add_and_fetch((ptr), 1)
#error atomic_add and atomic_add_64
#endif

LSTD_END_NAMESPACE
