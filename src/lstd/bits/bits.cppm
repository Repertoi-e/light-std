module;

#include "../common/namespace.h"

#include <intrin.h>

#if COMPILER == MSVC
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanReverse64)

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanForward64)
#endif

export module lstd.bits;

export import lstd.math;

LSTD_BEGIN_NAMESPACE

export {
    // Returns the index of the most significant set bit.
    // The index always starts at the LSB.
    //   e.g msb(12) (binary - 1100) -> returns 3
    // If x is 0, returned value is -1 (no set bits).
    template <typename T>
    s32 msb(T x) {
        // We can't use a concept here because we need the msb forward declaration in u128.h,
        // but that file can't include "type_info.h". C++ is bullshit.
        static_assert(is_unsigned_integral<T>);

        if constexpr (sizeof(T) == 16) {
            // 128 bit integers
            if (x.hi != 0) return 64 + msb(x.hi);
            return msb(x.lo);
        }
        else {
#if COMPILER == MSVC
            if constexpr (sizeof(T) == 8) {
                unsigned long r = 0;
                return _BitScanReverse64(&r, x) ? ((s32)r) : -1;
            }
            else {
                unsigned long r = 0;
                return _BitScanReverse(&r, x) ? ((s32)r) : -1;
            }
#else
#error Implement.
#endif
        }
    }

    // Returns the index of the least significant set bit.
    // The index always starts at the LSB.
    //   e.g lsb(12) (binary - 1100) -> returns 2
    // If x is 0, returned value is -1 (no set bits).
    s32 lsb(is_unsigned_integral auto x) {
        if constexpr (sizeof(x) == 16) {
            // 128 bit integers
            if (x.lo == 0) return 64 + lsb(x.hi);
            return lsb(x.lo);
        }
        else {
#if COMPILER == MSVC
            if constexpr (sizeof(x) == 8) {
                unsigned long r = 0;
                return _BitScanForward64(&r, x) ? ((s32)r) : -1;
            }
            else {
                unsigned long r = 0;
                return _BitScanForward(&r, x) ? ((s32)r) : -1;
            }
#else
#error Implement.
#endif
        }
    }

    u32 rotate_left_32(u32 x, u32 bits) { return (x << bits) | (x >> (32 - bits)); }
    u64 rotate_left_64(u64 x, u32 bits) { return (x << bits) | (x >> (64 - bits)); }

    u32 rotate_right_32(u32 x, u32 bits) { return (x >> bits) | (x << (32 - bits)); }
    u64 rotate_right_64(u64 x, u32 bits) { return (x >> bits) | (x << (64 - bits)); }

    // Functions for swapping endianness. You can check for the endianness by using #if ENDIAN = LITTLE_ENDIAN, etc.
    void byte_swap_2(void* ptr) {
        u16 x = *(u16*)ptr;
        *(u16*)ptr = x << 8 & 0xFF00 | x >> 8 & 0x00FF;
    }

    void byte_swap_4(void* ptr) {
        u32 x = *(u32*)ptr;
        *(u32*)ptr = x << 24 & 0xFF000000 | x << 8 & 0x00FF0000 | x >> 8 & 0x0000FF00 | x >> 24 & 0x000000FF;
    }

    void byte_swap_8(void* ptr) {
        u64 x = *(u64*)ptr;
        x = ((x << 8) & 0xFF00FF00FF00FF00ULL) | ((x >> 8) & 0x00FF00FF00FF00FFULL);
        x = ((x << 16) & 0xFFFF0000FFFF0000ULL) | ((x >> 16) & 0x0000FFFF0000FFFFULL);
        *(u64*)ptr = (x << 32) | (x >> 32);
    }

    //
    // Useful: http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign
    //

    bool has_zero_byte(u32 v) {
        // Uses 4 operations
        return (((v)-0x01010101UL) & ~(v) & 0x80808080UL);
    }

    bool has_byte(u32 x, u8 value) {
        // Uses 5 operations when value is constant
        return (has_zero_byte((x) ^ (~0UL / 255 * value)));
    }

    bool has_byte_less_than(u32 x, u8 value) {
        // Uses 4 operations when value is constant
        return (((x)-~0UL / 255 * value) & ~(x) & ~0UL / 255 * 128);
    }

    bool has_byte_greater_than(u32 x, u8 value) {
        // Uses 3 operations when value is constant
        return (((x)+~0UL / 255 * (127 - value) | (x)) & ~0UL / 255 * 128);
    }

    s32 count_bytes_less_than(u32 x, u8 value) {
        // Uses 7 operations when value is constant
        return (((~0UL / 255 * (127 + (value)) - ((x) & ~0UL / 255 * 127)) & ~(x) & ~0UL / 255 * 128) / 128 % 255);
    }

    s32 count_bytes_greater_than(u32 x, u8 value) {
        // Uses 6 operations when value is constant
        return (((((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(value)) | (x)) & ~0UL / 255 * 128) / 128 % 255);
    }

    // Sometimes it reports false positives.
    // Use has_byte_between for an exact answer.
    // Use this as a fast pretest:
    bool has_likely_byte_between(u32 x, u8 low, u8 high) {
        // Uses 7 operations when values are constant
        return ((((x)-~0UL / 255 * high) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - low)) & ~0UL / 255 * 128);
    }

    bool has_byte_between(u32 x, u8 low, u8 high) {
        // Uses 7 operations when values are constant
        return ((~0UL / 255 * (127 + (u8)(high)) - ((x) & ~0UL / 255 * 127) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(low))) & ~0UL / 255 * 128);
    }

    s32 count_bytes_between(u32 x, u8 low, u8 high) {
        // Uses 10 operations when values are constant
        return ((~0UL / 255 * (127 + (u8)(high)) - ((x) & ~0UL / 255 * 127) & ~(x) & ((x) & ~0UL / 255 * 127) + ~0UL / 255 * (127 - (u8)(low))) & ~0UL / 255 * 128) / 128 % 255;
    }

#define POWERS_OF_10(factor) \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, factor * 100000000, factor * 1000000000

	// These are just look up tables for powers of ten. 
	// Used in the fmt module when printing arithmetic types, for example.

    const u32 POWERS_OF_10_32[] = { 1, POWERS_OF_10(1) };
    const u64 POWERS_OF_10_64[] = { 1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull };

    const u32 ZERO_OR_POWERS_OF_10_32[] = { 0, POWERS_OF_10(1) };
	const u64 ZERO_OR_POWERS_OF_10_64[] = { 0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull };

    // Returns the number of bits (base 2 digits) needed to represent n. Leading zeroes
    // are not counted, except for n == 0, in which case count_digits_base_2 returns 1.
    u32 count_digits_base_2(is_unsigned_integral auto n) {
        s32 integerLog2 = msb(n | 1);  // log_2(n) == msb(n) (@Speed Not the fastest way)
        // We also | 1 (if n is 0, we treat is as 1)

        return (u32)(integerLog2 + 1);  // Number of bits in 'n' is [log_2(n)] + 1
    }

    // Returns the number of decimal digits in n. Leading zeros are not counted
    // except for n == 0 in which case count_digits returns 1.
    u32 count_digits(is_unsigned_integral auto n) {
        s32 integerLog2 = msb(n | 1);  // log_2(n) == msb(n) (@Speed Not the fastest way)
        // We also | 1 (if n is 0, we treat is as 1)

        // Divide by log_2(10), which is approx. 1233 / 4096
        u32 t = ((u32)integerLog2 + 1) * 1233 >> 12;  // We add 1 to integerLog2 because it rounds down.

        u32 integerLog10 = t - (n < POWERS_OF_10_64[t]);  // t may be off by 1, correct it.

        return integerLog10 + 1;  // Number of digits in 'n' is [log_10(n)] + 1
    }

    template <u32 Bits>
    u32 count_digits(is_integral auto value) {
        decltype(value) n = value;

        u32 numDigits = 0;
        do {
            ++numDigits;
        } while ((n >>= Bits) != 0);
        return numDigits;
    }
}

LSTD_END_NAMESPACE
