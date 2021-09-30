#pragma once

#include <intrin.h>

#include "type_info.h"

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
#define offset_of(s, field) ((u64) & ((s *) (0))->field)

#if COMPILER == MSVC
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanReverse64)

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)
#endif

LSTD_BEGIN_NAMESPACE

// Returns the index of the most significant set bit.
// The index always starts at the LSB.
//   e.g msb(12) (binary - 1100) -> returns 3
// If x is 0, returned value is -1 (no set bits).
template <typename T>
constexpr always_inline s32 msb(T x) {
    // We can't use a concept here because we need the msb forward declaration in u128.h,
    // but that file can't include "type_info.h". C++ is bullshit.
    static_assert(types::is_unsigned_integral<T>);

    if constexpr (sizeof(T) == 16) {
        // 128 bit integers
        if (x.hi != 0) return 64 + msb(x.hi);
        return msb(x.lo);
    } else {
        if (is_constant_evaluated()) {
            s32 r = 0;
            while (x >>= 1) ++r;
            return r;
        } else {
#if COMPILER == MSVC
            if constexpr (sizeof(T) == 8) {
                unsigned long r = 0;
                return _BitScanReverse64(&r, x) ? ((s32) r) : -1;
            } else {
                unsigned long r = 0;
                return _BitScanReverse(&r, x) ? ((s32) r) : -1;
            }
#endif
        }
    }
}

// Returns the index of the least significant set bit.
// The index always starts at the LSB.
//   e.g lsb(12) (binary - 1100) -> returns 2
// If x is 0, returned value is -1 (no set bits).
constexpr always_inline s32 lsb(types::is_unsigned_integral auto x) {
    if constexpr (sizeof(x) == 16) {
        // 128 bit integers
        if (x.lo == 0) return 64 + lsb(x.hi);
        return lsb(x.lo);
    } else {
        if (is_constant_evaluated()) {
            s32 r = 0;
            while (!(x & 1)) ++r, x >>= 1;
            return r;
        } else {
#if COMPILER == MSVC
            if constexpr (sizeof(x) == 8) {
                unsigned long r = 0;
                return _BitScanForward64(&r, x) ? ((s32) r) : -1;
            } else {
                unsigned long r = 0;
                return _BitScanForward(&r, x) ? ((s32) r) : -1;
            }
#endif
        }
    }
}

constexpr u32 rotate_left_32(u32 x, u32 bits) { return (x << bits) | (x >> (32 - bits)); }
constexpr u64 rotate_left_64(u64 x, u32 bits) { return (x << bits) | (x >> (64 - bits)); }

constexpr u32 rotate_right_32(u32 x, u32 bits) { return (x >> bits) | (x << (32 - bits)); }
constexpr u64 rotate_right_64(u64 x, u32 bits) { return (x >> bits) | (x << (64 - bits)); }

//
// Useful: http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign
//

// Uses 4 operations:
#define U32_HAS_ZERO_BYTE(v) (((v) -0x01010101UL) & ~(v) &0x80808080UL)

// Uses 5 operations when n is constant:
#define U32_HAS_BYTE(x, n) (U32_HAS_ZERO_BYTE((x) ^ (~0UL / 255 * (u8) (n))))

// Uses 4 operations when n is constant:
#define U32_HAS_BYTE_LESS_THAN(x, n) (((x) - ~0UL / 255 * (u8) (n)) & ~(x) & ~0UL / 255 * 128)

// Uses 7 operations when n is constant:
#define U32_COUNT_BYTES_LESS_THAN(x, n) (((~0UL / 255 * (127 + (n)) - ((x) & ~0UL / 255 * 127)) & ~(x) & ~0UL / 255 * 128) / 128 % 255)

// Uses 3 operations when n is constant:
#define U32_HAS_BYTE_GREATER_THAN(x, n) (((x) + ~0UL / 255 * (127 - (u8) (n)) | (x)) & ~0UL / 255 * 128)

// Uses 6 operations when n is constant:
#define U32_COUNT_BYTES_GREATER_THAN(x, n) (((((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8) (n)) | (x)) & ~0UL / 255 * 128) / 128 % 255)

// Uses 7 operations when n is constant.
// Sometimes it reports false positives. Use U32_HAS_BYTE_BETWEEN for an exact answer.
// Use this as a fast pretest:
#define U32_LIKELY_HAS_BYTE_BETWEEN(x, m, n) ((((x) - ~0UL / 255 * (u8) (n)) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8) (m))) & ~0UL / 255 * 128)

// Uses 8 operations when n is constant:
#define U32_HAS_BYTE_BETWEEN(x, m, n) ((~0UL / 255 * (127 + (u8) (n)) - ((x) & ~0UL / 255 * 127) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8) (m))) & ~0UL / 255 * 128)

// Uses 10 operations when n is constant:
#define U32_COUNT_BYTES_BETWEEN(x, m, n) (U32_HAS_BYTE_BETWEEN(x, m, n) / 128 % 255)

#define POWERS_OF_10(factor) \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, factor * 100000000, factor * 1000000000

// These are just look up tables for powers of ten. Used in the fmt module when printing arithmetic types, for example.
constexpr u32 POWERS_OF_10_32[] = {1, POWERS_OF_10(1)};
constexpr u64 POWERS_OF_10_64[] = {1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

constexpr u32 ZERO_OR_POWERS_OF_10_32[] = {0, POWERS_OF_10(1)};
constexpr u64 ZERO_OR_POWERS_OF_10_64[] = {0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};
#undef POWERS_OF_10

// Returns the number of bits (base 2 digits) needed to represent n. Leading zeroes
// are not counted, except for n == 0, in which case count_digits_base_2 returns 1.
//
// Source: Bit-Twiddling hacks
always_inline u32 count_digits_base_2(types::is_unsigned_integral auto n) {
    s32 integerLog2 = msb(n | 1);  // log_2(n) == msb(n) (@Speed Not the fastest way)
                                   // We also | 1 (if n is 0, we treat is as 1)

    return (u32) (integerLog2 + 1);  // Number of bits in 'n' is [log_2(n)] + 1
}

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
//
// Source: Bit-Twiddling hacks
always_inline u32 count_digits(types::is_unsigned_integral auto n) {
    s32 integerLog2 = msb(n | 1);  // log_2(n) == msb(n) (@Speed Not the fastest way)
                                   // We also | 1 (if n is 0, we treat is as 1)

    // Divide by log_2(10), which is approx. 1233 / 4096
    u32 t = ((u32) integerLog2 + 1) * 1233 >> 12;  // We add 1 to integerLog2 because it rounds down.

    u32 integerLog10 = t - (n < POWERS_OF_10_64[t]);  // t may be off by 1, correct it.

    return integerLog10 + 1;  // Number of digits in 'n' is [log_10(n)] + 1
}

template <u32 Bits, types::is_integral T>
constexpr u32 count_digits(T value) {
    T n = value;

    u32 numDigits = 0;
    do {
        ++numDigits;
    } while ((n >>= Bits) != 0);
    return numDigits;
}

LSTD_END_NAMESPACE
