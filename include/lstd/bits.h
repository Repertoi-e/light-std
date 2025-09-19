#pragma once

#include "common.h"

#if COMPILER == MSVC

#if BITS == 64
#pragma intrinsic(_BitScanForward64)
extern "C" unsigned char __cdecl _BitScanForward64(unsigned long *_Index,
                                                   unsigned __int64 _Mask);
#endif
#pragma intrinsic(_BitScanForward)
extern "C" unsigned char __cdecl _BitScanForward(unsigned long *_Index,
                                                 unsigned long _Mask);

#if BITS == 64
#pragma intrinsic(_BitScanReverse64)
extern "C" unsigned char __cdecl _BitScanReverse64(unsigned long *_Index,
                                                   unsigned __int64 _Mask);
#endif
#pragma intrinsic(_BitScanReverse)
extern "C" unsigned char __cdecl _BitScanReverse(unsigned long *_Index,
                                                 unsigned long _Mask);
#endif

//
// Defines:
//      msb, lsb, rotate_left/right_32/64,
//      byte_swap_2/4/8, count_digits
//
// These bit hacks may be useful:
// http://graphics.stanford.edu/~seander/bithacks.html#CopyIntegerSign
// We used to include them here, but never used them.
//

LSTD_BEGIN_NAMESPACE

#if COMPILER == MSVC

#endif

// Returns the index of the most significant set bit.
// The index always starts at the LSB.
//   e.g msb(12) (binary - 1100) -> returns 3
// If x is 0, returned value is -1 (no set bits).
template <typename T>
inline s32 msb(T x) {
  // We can't use a concept here because we need the msb forward declaration in
  // u128.h, but that file can't include "type_info.h". C++ is bullshit.
  static_assert(is_unsigned_integral<T>);

  if constexpr (sizeof(T) == 16) {
    // 128 bit integers
    if (x.hi != 0) return 64 + msb(x.hi);
    return msb(x.lo);
  } else {
#if COMPILER == MSVC
    if constexpr (sizeof(T) == 8) {
#if BITS == 64
      unsigned long r = 0;
      return _BitScanReverse64(&r, x) ? ((s32)r) : -1;
#else
      assert(false &&
             "Trying to call msb with "
             "64 bit integer on 32 bit "
             "platform.");
#endif
    } else {
      unsigned long r = 0;
      return _BitScanReverse(&r, x) ? ((s32)r) : -1;
    }
#else
    if constexpr (sizeof(T) == 8) {
      return x == 0 ? -1 : 63 - __builtin_clzll(x);
    } else {
      return x == 0 ? -1 : 31 - __builtin_clz(x);
    }
#endif
  }
}

// Returns the index of the least significant set bit.
// The index always starts at the LSB.
//   e.g lsb(12) (binary - 1100) -> returns 2
// If x is 0, returned value is -1 (no set bits).
inline s32 lsb(is_unsigned_integral auto x) {
  if constexpr (sizeof(x) == 16) {
    // 128 bit integers
    if (x.lo == 0) return 64 + lsb(x.hi);
    return lsb(x.lo);
  } else {
#if COMPILER == MSVC
    if constexpr (sizeof(x) == 8) {
#if BITS == 64
      unsigned long r = 0;
      return _BitScanForward64(&r, x) ? ((s32)r) : -1;
#else
      assert(false &&
             "Trying to call lsb with "
             "64 bit integer on 32 bit "
             "platform.");
#endif
    } else {
      unsigned long r = 0;
      return _BitScanForward(&r, x) ? ((s32)r) : -1;
    }
#else
    if constexpr (sizeof(x) == 8) {
      return x == 0 ? -1 : __builtin_ctzll(x);
    } else {
      return x == 0 ? -1 : __builtin_ctz(x);
    }
#endif
  }
}

inline u32 rotate_left_32(u32 x, u32 bits) {
  return (x << bits) | (x >> (32 - bits));
}
inline u64 rotate_left_64(u64 x, u32 bits) {
  return (x << bits) | (x >> (64 - bits));
}

inline u32 rotate_right_32(u32 x, u32 bits) {
  return (x >> bits) | (x << (32 - bits));
}

inline u64 rotate_right_64(u64 x, u32 bits) {
  return (x >> bits) | (x << (64 - bits));
}

// Functions for swapping endianness. You can check for the endianness by using
// #if ENDIAN = LITTLE_ENDIAN, etc.
#if COMPILER == MSVC
static inline u16 byte_swap_16(u16 x) { return _byteswap_ushort(x); }
static inline u32 byte_swap_32(u32 x) { return _byteswap_ulong(x); }
static inline u64 byte_swap_64(u64 x) { return _byteswap_uint64(x); }
#elif COMPILER == GCC || COMPILER == CLANG
static inline u16 byte_swap_16(u16 x) { return __builtin_bswap16(x); }
static inline u32 byte_swap_32(u32 x) { return __builtin_bswap32(x); }
static inline u64 byte_swap_64(u64 x) { return __builtin_bswap64(x); }
#endif

template <typename T>
void swap(T &a, T &b) {
  T c = a;
  a = b;
  b = c;
}

template <typename T, s64 N>
void swap(T (&a)[N], T (&b)[N]) {
  For(range(N)) swap(a[it], b[it]);
}

#define POWERS_OF_10(factor)                                                 \
  factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, \
      factor * 1000000, factor * 10000000, factor * 100000000,               \
      factor * 1000000000

// These are just look up tables for powers of ten.
// Used in the fmt module when printing arithmetic types, for example
// and here in count_digits.

inline const u64 POWERS_OF_10_64[] = {
    1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline u32 count_digits(is_unsigned_integral auto n) {
  s32 integerLog2 =
      msb(n | 1);  // log_2(n) == msb(n) (@Speed Not the fastest way)
  // We also | 1 (if n is 0, we treat is as 1)

  // Divide by log_2(10), which is approx. 1233 / 4096
  u32 t = ((u32)integerLog2 + 1) * 1233 >>
          12;  // We add 1 to integerLog2 because it rounds down.

  u32 integerLog10 =
      t - (n < POWERS_OF_10_64[t]);  // t may be off by 1, correct it.

  return integerLog10 + 1;  // Number of digits in 'n' is [log_10(n)] + 1
}

template <u32 Bits>
inline u32 count_digits(is_integral auto value) {
  decltype(value) n = value;

  u32 numDigits = 0;
  do {
    ++numDigits;
  } while ((n >>= Bits) != 0);
  return numDigits;
}

LSTD_END_NAMESPACE
