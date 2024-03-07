#pragma once

#include "../namespace.h"
#include "128.h"
#include "ieee.h"
#include "types.h"

// Define numeric<T>, a way to get info about numbers, e.g. min/max,
// max digits, for floats: exponent, mantissa bits, etc.
//
// numeric<T> is useful when writing template functions and you don't know
// the specific integral type, so you cannot just assume the max size for
// 32 bit integer for example. In that case you can use numeric<T>::max()

LSTD_BEGIN_NAMESPACE

namespace internal {
struct numeric_base {
  static constexpr bool is_integral = false;
  static constexpr s32 digits = 0;
  static constexpr s32 digits10 = 0;
  static constexpr s32 max_digits10 = 0;
};

struct numeric_integer_base : numeric_base {
  static constexpr bool is_integral = true;
};
}  // namespace internal

template <typename T>
struct numeric : public internal::numeric_base {
  using value_t = T;
};

// Const/volatile variations of numeric.
template <typename T>
struct numeric<const T> : public numeric<T> {};
template <typename T>
struct numeric<volatile T> : public numeric<T> {};
template <typename T>
struct numeric<const volatile T> : public numeric<T> {};

template <>
struct numeric<char> : public internal::numeric_integer_base {
  static constexpr char min() { return (-128); }
  static constexpr char max() { return (127); }

  static constexpr s32 digits = 8;
  static constexpr s32 digits10 = 2;
};

template <>
struct numeric<wchar_t> : public internal::numeric_integer_base {
  static constexpr wchar_t min() { return 0x0000; }
  static constexpr wchar_t max() { return 0xffff; }

  static constexpr s32 digits = 16;
  static constexpr s32 digits10 = 4;
};

template <>
struct numeric<bool> : public internal::numeric_integer_base {
  // limits for type bool
  static constexpr bool min() { return false; }
  static constexpr bool max() { return true; }

  static constexpr s32 digits = 1;
};

template <>
struct numeric<u8> : public internal::numeric_integer_base {
  static constexpr u8 min() { return 0; }
  static constexpr u8 max() { return (255); }

  static constexpr s32 digits = 8;
  static constexpr s32 digits10 = 2;
};

template <>
struct numeric<s16> : public internal::numeric_integer_base {
  static constexpr s16 min() { return (-32768); }
  static constexpr s16 max() { return (32767); }

  static constexpr s32 digits = 15;
  static constexpr s32 digits10 = 4;
};

#ifdef _NATIVE_WCHAR_T_DEFINED
template <>
struct numeric<u16> : public internal::numeric_integer_base {
  static constexpr u16 min() { return 0; }
  static constexpr u16 max() { return (65535); }

  static constexpr s32 digits = 16;
  static constexpr s32 digits10 = 4;
};
#endif

template <>
struct numeric<unsigned short> : public internal::numeric_integer_base {
  static constexpr u16 min() { return 0; }
  static constexpr u16 max() { return (65535); }

  static constexpr s32 digits = 16;
  static constexpr s32 digits10 = 4;
};

template <>
struct numeric<char8_t> : public internal::numeric_integer_base {
  static constexpr char8_t min() { return 0; }
  static constexpr char8_t max() { return (255); }

  static constexpr s32 digits = 8;
  static constexpr s32 digits10 = 2;
};

template <>
struct numeric<char16_t> : public internal::numeric_integer_base {
  static constexpr char16_t min() { return 0; }
  static constexpr char16_t max() { return (65535); }

  static constexpr s32 digits = 16;
  static constexpr s32 digits10 = 4;
};

template <>
struct numeric<s32> : public internal::numeric_integer_base {
  static constexpr s32 min() { return (-2147483647 - 1); }
  static constexpr s32 max() { return (2147483647); }

  static constexpr s32 digits = 31;
  static constexpr s32 digits10 = 9;
};

template <>
struct numeric<u32> : public internal::numeric_integer_base {
  static constexpr u32 min() { return 0; }
  static constexpr u32 max() { return (4294967295U); }

  static constexpr s32 digits = 32;
  static constexpr s32 digits10 = 9;
};

template <>
struct numeric<char32_t> : public internal::numeric_integer_base {
 public:
  static constexpr char32_t min() { return 0; }
  static constexpr char32_t max() { return (4294967295U); }

  static constexpr s32 digits = 32;
  static constexpr s32 digits10 = 9;
};

template <>
struct numeric<s64> : public internal::numeric_integer_base {
  static constexpr s64 min() { return (-9223372036854775807L - 1); }
  static constexpr s64 max() { return (9223372036854775807L); }

  static constexpr s32 digits = 63;
  static constexpr s32 digits10 = 18;
};

template <>
struct numeric<long> : public internal::numeric_integer_base {
  static constexpr s64 min() { return (-9223372036854775807L - 1); }
  static constexpr s64 max() { return (9223372036854775807L); }

  static constexpr s32 digits = 63;
  static constexpr s32 digits10 = 18;
};

template <>
struct numeric<u64> : public internal::numeric_integer_base {
 public:
  static constexpr u64 min() { return 0; }
  static constexpr u64 max() { return (18446744073709551615ULL); }

  static constexpr s32 digits = 64;
  static constexpr s32 digits10 = 19;
};

template <>
struct numeric<unsigned long> : public internal::numeric_integer_base {
 public:
  static constexpr u64 min() { return 0; }
  static constexpr u64 max() { return (18446744073709551615ULL); }

  static constexpr s32 digits = 64;
  static constexpr s32 digits10 = 19;
};

template <>
struct numeric<f32> {
 public:
  static constexpr f32 min() { return 1.175494351e-38F; }
  static constexpr f32 max() { return 3.402823466e+38F; }
  static constexpr f32 epsilon() {
    return 1.192092896e-07F;
  }  // smallest suchthat 1.0 + epsilon != 1.0
  static constexpr f32 round_error() { return 0.5F; }
  static constexpr f32 denorm_min() { return 1.401298464e-45F; }
  static constexpr f32 infinity() { return __builtin_huge_valf(); }
  static constexpr f32 quiet_NaN() { return __builtin_nanf("0"); }
  static constexpr f32 signaling_NaN() { return __builtin_nansf("1"); }

  static constexpr s32 digits = 23 + 1;   // including the hidden bit
  static constexpr s32 digits10 = 6;      // # of decimal digits of precision
  static constexpr s32 max_digits10 = 9;  // # of decimal digits of precision
  static constexpr s32 max_exponent = 127;
  static constexpr s32 max_exponent10 = 38;
  static constexpr s32 min_exponent = -126;
  static constexpr s32 min_exponent10 = -37;
  static constexpr s32 bits_mantissa =
      23;  // # of bits in mantissa, excluding the hidden bit (which is always
           // interpreted as 1 for normal numbers)
  static constexpr s32 bits_exponent = 8;
  static constexpr s32 exponent_bias = 127;
};

template <>
struct numeric<f64> {
 public:
  static constexpr f64 min() { return 2.2250738585072014e-308; }
  static constexpr f64 max() { return 1.7976931348623158e+308; }
  static constexpr f64 epsilon() {
    return 2.2204460492503131e-016;
  }  // smalles such that 1.0 + epsilon != 1.0
  static constexpr f64 round_error() { return 0.5; }
  static constexpr f64 denorm_min() { return 4.9406564584124654e-324; }
  static constexpr f64 infinity() { return __builtin_huge_val(); }
  static constexpr f64 quiet_NaN() { return __builtin_nan("0"); }
  static constexpr f64 signaling_NaN() { return __builtin_nans("1"); }

  static constexpr s32 digits = 52 + 1;    // including the hidden bit
  static constexpr s32 digits10 = 15;      // # of decimal digits of precision
  static constexpr s32 max_digits10 = 17;  // # of decimal digits of precision
  static constexpr s32 max_exponent = 1023;
  static constexpr s32 max_exponent10 = 308;
  static constexpr s32 min_exponent = -1022;
  static constexpr s32 min_exponent10 = -307;
  static constexpr s32 bits_mantissa =
      52;  // number of bits in mantissa,excluding the hidden bit (which is
           // always interpreted as 1 for normalnumbers)
  static constexpr s32 bits_exponent = 11;
  static constexpr s32 exponent_bias = 1023;
};

template <>
struct numeric<u128> : public internal::numeric_integer_base {
 public:
  static constexpr u128 min() { return 0; }
  static constexpr u128 max() { return u128(U64_MAX, U64_MAX); }

  static constexpr s32 digits = 128;
  static constexpr s32 digits10 = 38;
};

template <>
struct numeric<s128> : public internal::numeric_integer_base {
  static constexpr s128 min() { return s128(numeric<s64>::min(), 0); }
  static constexpr s128 max() { return s128(numeric<s64>::max(), U64_MAX); }

  static constexpr s32 digits = 127;
  static constexpr s32 digits10 = 38;
};

LSTD_END_NAMESPACE