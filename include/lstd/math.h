#pragma once

//
// This file defines some common math functions:
//    min, max, clamp,
//    abs,
//    cast_numeric_safe,
//    is_pow_of_2, ceil_pow_of_2, const_exp10,
//    is_nan, is_signaling_nan, is_infinite, is_finite,
//    sign_bit, sign_no_zero, sign, copy_sign
//

#include "common.h"
#include "type_info.h"

// Tau supremacy https://tauday.com/tau-manifesto
#define TAU 6.283185307179586476925286766559
#define PI (TAU / 2)

//
// If we aren't building with CRT then:
//
// Cephes provides our replacement for the math functions found in virtually all
// standard libraries. Also provides functions for extended precision
// arithmetic, statistical functions, physics, astronomy, etc.
// https://www.netlib.org/cephes/
// Note: We don't include everything from it, just cmath for now.
//       Statistics is a thing we will most definitely include as well in the
//       future. Everything else you can include on your own in your project (we
//       don't want to be bloat-y).
//
// Note: Important difference,
// atan2's return range is 0 to 2PI, and not -PI to PI (as per normal in the C
// standard library).
//
// Parts of the source code that we modified are marked with :WEMODIFIEDCEPHES:
//
// @TODO: We should always have our own math functions
// because otherwise they'd differ from compiler to compiler.
// This is a horrendous mistake the C++ committee has allowed to happen.

/*
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1995, 2000 by Stephen L. Moshier
*/
#if defined LSTD_NO_CRT
#include "vendor/cephes/maths_cephes.h"
#else
#include <math.h>
#endif

LSTD_BEGIN_NAMESPACE

inline bool sign_bit(is_signed_integral auto x) { return x < 0; }
inline bool sign_bit(is_unsigned_integral auto) { return false; }

inline bool sign_bit(f32 x) { return ieee754_f32{x}.ieee.S; }
inline bool sign_bit(f64 x) { return ieee754_f64{x}.ieee.S; }

// Returns -1 if x is negative, 1 otherwise
inline s32 sign_no_zero(is_scalar auto x) { return sign_bit(x) ? -1 : 1; }

// Returns -1 if x is negative, 1 if positive, 0 otherwise
inline s32 sign(is_scalar auto x) {
  if (x == decltype(x)(0)) return 0;
  return sign_no_zero(x);
}

template <is_floating_point T>
inline T copy_sign(T x, T y) {
  if constexpr (sizeof(x) == sizeof(f32)) {
    ieee754_f32 formatx = {x}, formaty = {y};
    formatx.ieee.S = formaty.ieee.S;
    return formatx.F;
  } else {
    ieee754_f64 formatx = {x}, formaty = {y};
    formatx.ieee.S = formaty.ieee.S;
    return formatx.F;
  }
}

inline bool is_nan(is_floating_point auto x) {
  if constexpr (sizeof(x) == sizeof(f32)) {
    ieee754_f32 format = {x};
    return format.ieee.E == 0xFF && format.ieee.M != 0;
  } else {
    ieee754_f64 format = {x};
    return format.ieee.E == 0x7FF &&
           (format.ieee.M0 != 0 || format.ieee.M1 != 0);
  }
}

inline bool is_signaling_nan(is_floating_point auto x) {
  if constexpr (sizeof(x) == sizeof(f32))
    return is_nan(x) && ieee754_f32{x}.ieee_nan.N == 0;
  else
    return is_nan(x) && ieee754_f64{x}.ieee_nan.N == 0;
}

inline bool is_infinite(is_floating_point auto x) {
  if constexpr (sizeof(x) == sizeof(f32)) {
    ieee754_f32 format = {x};
    return format.ieee.E == 0xFF && format.ieee.M == 0;
  } else {
    ieee754_f64 format = {x};
    return format.ieee.E == 0x7FF && format.ieee.M0 == 0 && format.ieee.M1 == 0;
  }
}

inline bool is_finite(is_floating_point auto x) {
  if constexpr (sizeof(x) == sizeof(f32))
    return ieee754_f32{x}.ieee.E != 0xFF;
  else
    return ieee754_f64{x}.ieee.E != 0x7FF;
}

/**
 * @brief Safely casts a numeric value of type U to a numeric value of type T.
 *
 * When LSTD_NUMERIC_CAST_CHECK is defined, runtime checks are performed
 * to detect potential overflows when casting between integer types. If an
 * overflow occurs, an error message is reported and an assertion is triggered.
 *
 * @tparam T The target numeric type to cast the value to.
 * @tparam U The source numeric type of the value being cast.
 * @param y The value of type U to be cast to type T.
 * @return The value y cast to type T.
 *
 * @note The template requires both T and U to be scalar types (i.e., integral
 * or floating-point types).
 */
template <typename T, typename U>
inline constexpr auto cast_numeric(U y) requires(is_scalar<T>&& is_scalar<U>) {
#if defined(LSTD_NUMERIC_CAST_CHECK)
  if constexpr (is_integral<T> && is_integral<U>) {
    if constexpr (is_signed_integral<T> && is_unsigned_integral<U>) {
      if (y > static_cast<U>(numeric<T>::max())) {
        // Report error and assert
        assert(false && "Overflow: unsigned to signed integer cast.");
      }
    } else if constexpr (is_unsigned_integral<T> && is_signed_integral<U>) {
      if (y < 0 || static_cast<u128>(y) > numeric<T>::max()) {
        // Report error and assert
        assert(false && "Overflow: signed to unsigned integer cast.");
      }
    } else if constexpr (is_signed_integral<T> && is_signed_integral<U>) {
      if (y > numeric<T>::max() || y < numeric<T>::min()) {
        // Report error and assert
        assert(false && "Overflow: signed integer to signed integer cast.");
      }
    } else if constexpr (is_unsigned_integral<T> && is_unsigned_integral<U>) {
      if (y > numeric<T>::max()) {
        // Report error and assert
        assert(false && "Overflow: unsigned integer to unsigned integer cast.");
      }
    }
  }
#endif
  return (T)y;
}

namespace internal {
  constexpr auto min_(auto x, auto y) {
    auto y_casted = cast_numeric<decltype(x)>(y);
    if constexpr (is_floating_point<decltype(x)>) {
      if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x < y_casted ? x : y_casted;
  }

  constexpr auto max_(auto x, auto y) {
    auto y_casted = cast_numeric<decltype(x)>(y);
    if constexpr (is_floating_point<decltype(x)>) {
      if (is_nan(x) || is_nan(y_casted)) return x + y_casted;
    }
    return x > y_casted ? x : y_casted;
  }
}  // namespace internal

template <is_scalar... Args>
inline constexpr auto min(is_scalar auto x, Args... rest) {
  auto result = x;
  ((void)(result = internal::min_(result, rest)), ...);
  return result;
}

template <is_scalar... Args>
inline constexpr auto max(is_scalar auto x, Args... rest) {
  auto result = x;
  ((void)(result = internal::max_(result, rest)), ...);
  return result;
}

// Returns lower if x < lower, return upper if x > upper, returns x otherwise
inline auto clamp(auto x, auto lower, auto upper) {
  return max(lower, min(upper, x));
}

// Checks if x is a power of 2
inline bool is_pow_of_2(is_integral auto x) { return (x & x - 1) == 0; }

// Returns the smallest power of 2 bigger or equal to x.
inline auto ceil_pow_of_2(is_integral auto x) {
  using T = decltype(x);

  if (x <= 1) return (T)1;

  T power = 2;
  --x;
  while (x >>= 1) power <<= 1;
  return power;
}

// Returns 10 ** exp at compile-time. Uses recursion.
template <typename T>
inline T const_exp10(s32 exp) {
  return exp == 0 ? T(1) : T(10) * const_exp10<T>(exp - 1);
}

inline auto abs(is_scalar auto x) {
  if constexpr (is_floating_point<decltype(x)>) {
    if constexpr (sizeof(x) == sizeof(f32)) {
      ieee754_f32 u = {x};
      u.ieee.S = 0;
      return u.F;
    } else {
      ieee754_f64 u = {x};
      u.ieee.S = 0;
      return u.F;
    }
  } else {
    if constexpr (is_unsigned_integral<decltype(x)>) {
      return x;  // Unsigned integrals are always positive
    } else {
      return x < 0 ? -x : x;
    }
  }
}

LSTD_END_NAMESPACE
