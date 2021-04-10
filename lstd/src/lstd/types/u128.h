#pragma once

#include "scalar_types.h"

struct s128;

//
// u128
//
// An unsigned 128-bit integer type. The API is meant to mimic an intrinsic type
// as closely as is practical, including exhibiting undefined behavior in
// analogous cases (e.g. division by zero). This type is intended to be a
// drop-in replacement once C++ supports an intrinsic `uint128_t` type; when
// that occurs, existing well-behaved uses of `u128` will continue to work
// using that new type.
//
// A `u128` supports the following:
//
//   * Implicit construction from integral types
//   * Explicit conversion to integral types
//
// Additionally, if your compiler supports `__int128`, `u128` is
// interoperable with that type. (Abseil checks for this compatibility through
// the `ABSL_HAVE_INTRINSIC_INT128` macro.)
//
// However, a `u128` differs from intrinsic integral types in the following
// ways:
//
//   * Errors on implicit conversions that do not preserve value (such as
//     loss of precision when converting to float values).
//   * Requires explicit construction from and conversion to floating point
//     types.
//   * Conversion to integral types requires an explicit static_cast() to
//     mimic use of the `-Wnarrowing` compiler flag.
//   * The alignment requirement of `u128` may differ from that of an
//     intrinsic 128-bit integer type depending on platform and build
//     configuration.
//
// Example:
//
//     f32 y = u128(17);  // Error. u128 cannot be implicitly
//                        // converted to float.
//
//     u128 v;
//     u64 i = v;        // Error
//     u64 i = (u64) v;  // OK
//
struct alignas(16) u128 {
#if ENDIAN == LITTLE_ENDIAN
    u64 lo;
    u64 hi;
#elif ENDIAN == BIG_ENDIAN
    u64 hi;
    u64 lo;
#else
#error "Unsupported byte order: must be little or big endian."
#endif

    constexpr u128() {}
    constexpr u128(u64 high, u64 low);

    //
    // Constructors from arithmetic types
    //
    constexpr u128(int v);
    constexpr u128(unsigned int v);
    constexpr u128(long v);
    constexpr u128(unsigned long v);
    constexpr u128(long long v);
    constexpr u128(unsigned long long v);
#ifdef ABSL_HAVE_INTRINSIC_INT128
    constexpr u128(__int128 v);
    constexpr u128(unsigned __int128 v);
#endif  // ABSL_HAVE_INTRINSIC_INT128
    constexpr u128(s128 v);
    constexpr explicit u128(float v);
    constexpr explicit u128(double v);

    //
    // Assignment operators from arithmetic types
    //
    constexpr u128 &operator=(int v);
    constexpr u128 &operator=(unsigned int v);
    constexpr u128 &operator=(long v);
    constexpr u128 &operator=(unsigned long v);
    constexpr u128 &operator=(long long v);
    constexpr u128 &operator=(unsigned long long v);
#ifdef ABSL_HAVE_INTRINSIC_INT128
    constexpr u128 &operator=(__int128 v);
    constexpr u128 &operator=(unsigned __int128 v);
#endif  // ABSL_HAVE_INTRINSIC_INT128
    constexpr u128 &operator=(s128 v);

    //
    // Conversion operators to other arithmetic types
    //
    constexpr explicit operator bool() const;
    constexpr explicit operator char() const;
    constexpr explicit operator signed char() const;
    constexpr explicit operator unsigned char() const;
    constexpr explicit operator char16_t() const;
    constexpr explicit operator char32_t() const;
    constexpr explicit operator wchar_t() const;
    constexpr explicit operator short() const;
    constexpr explicit operator unsigned short() const;
    constexpr explicit operator int() const;
    constexpr explicit operator unsigned int() const;
    constexpr explicit operator long() const;
    constexpr explicit operator unsigned long() const;
    constexpr explicit operator long long() const;
    constexpr explicit operator unsigned long long() const;
#ifdef ABSL_HAVE_INTRINSIC_INT128
    constexpr explicit operator __int128() const;
    constexpr explicit operator unsigned __int128() const;
#endif  // ABSL_HAVE_INTRINSIC_INT128
    explicit operator float() const;
    explicit operator double() const;

    //
    // Arithmetic operators
    //
    constexpr u128 &operator+=(u128 other);
    constexpr u128 &operator-=(u128 other);
    constexpr u128 &operator*=(u128 other);
    constexpr u128 &operator/=(u128 other);
    constexpr u128 &operator%=(u128 other);
    constexpr u128 operator++(int);
    constexpr u128 operator--(int);
    constexpr u128 &operator<<=(int);
    constexpr u128 &operator>>=(int);
    constexpr u128 &operator&=(u128 other);
    constexpr u128 &operator|=(u128 other);
    constexpr u128 &operator^=(u128 other);
    constexpr u128 &operator++();
    constexpr u128 &operator--();
};

/*
// Specialized numeric_limits for u128.
namespace std {
template <>
class numeric_limits<u128> {
   public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = false;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = 128;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
#ifdef ABSL_HAVE_INTRINSIC_INT128
    static constexpr bool traps = numeric_limits<unsigned __int128>::traps;
#else  // ABSL_HAVE_INTRINSIC_INT128
    static constexpr bool traps = numeric_limits<u64>::traps;
#endif  // ABSL_HAVE_INTRINSIC_INT128
    static constexpr bool tinyness_before = false;

    static constexpr u128(min)() { return 0; }
    static constexpr u128 lowest() { return 0; }
    static constexpr u128(max)() { return Uint128Max(); }
    static constexpr u128 epsilon() { return 0; }
    static constexpr u128 round_error() { return 0; }
    static constexpr u128 infinity() { return 0; }
    static constexpr u128 quiet_NaN() { return 0; }
    static constexpr u128 signaling_NaN() { return 0; }
    static constexpr u128 denorm_min() { return 0; }
};
}  // namespace std
*/

// s128
//
// A signed 128-bit integer type. The API is meant to mimic an intrinsic
// integral type as closely as is practical, including exhibiting undefined
// behavior in analogous cases (e.g. division by zero).
//
// An `s128` supports the following:
//
//   * Implicit construction from integral types
//   * Explicit conversion to integral types
//
// However, an `s128` differs from intrinsic integral types in the following ways:
//
//   * It is not implicitly convertible to other integral types.
//   * Requires explicit construction from and conversion to floating point
//     types.

// Additionally, if your compiler supports `__int128`, `s128` is
// interoperable with that type. (Abseil checks for this compatibility through
// the `ABSL_HAVE_INTRINSIC_INT128` macro.)
//
// The design goal for `s128` is that it will be compatible with a future
// `int128_t`, if that type becomes a part of the standard.
//
// Example:
//
//     float y = s128(17);  // Error. s128 cannot be implicitly
//                          // converted to float.
//
//     s128 v;
//     s64 i = v;        // Error
//     s64 i = (s64) v;  // OK
//
struct alignas(16) s128 {
#if ENDIAN == LITTLE_ENDIAN
    u64 lo;
    s64 hi;
#elif ENDIAN == BIG_ENDIAN
    s64 hi;
    u64 lo;
#else
#error "Unsupported byte order: must be little or big endian."
#endif

    constexpr s128() {}
    constexpr s128(u64 high, u64 low);

    //
    // Constructors from arithmetic types
    //
    constexpr s128(int v);
    constexpr s128(unsigned int v);
    constexpr s128(long v);
    constexpr s128(unsigned long v);
    constexpr s128(long long v);
    constexpr s128(unsigned long long v);
#ifdef ABSL_HAVE_INTRINSIC_INT128
    constexpr s128(__int128 v);
    constexpr explicit s128(unsigned __int128 v);
#endif
    constexpr explicit s128(u128 v);
    explicit s128(float v);
    explicit s128(double v);

    //
    // Assignment operators from arithmetic types
    //
    constexpr s128 &operator=(int v);
    constexpr s128 &operator=(unsigned int v);
    constexpr s128 &operator=(long v);
    constexpr s128 &operator=(unsigned long v);
    constexpr s128 &operator=(long long v);
    constexpr s128 &operator=(unsigned long long v);
#ifdef ABSL_HAVE_INTRINSIC_INT128
    constexpr s128 &operator=(__int128 v);
#endif  // ABSL_HAVE_INTRINSIC_INT128

    //
    // Conversion operators to other arithmetic types
    //
    constexpr explicit operator bool() const;
    constexpr explicit operator char() const;
    constexpr explicit operator signed char() const;
    constexpr explicit operator unsigned char() const;
    constexpr explicit operator char16_t() const;
    constexpr explicit operator char32_t() const;
    constexpr explicit operator wchar_t() const;
    constexpr explicit operator short() const;
    constexpr explicit operator unsigned short() const;
    constexpr explicit operator int() const;
    constexpr explicit operator unsigned int() const;
    constexpr explicit operator long() const;
    constexpr explicit operator unsigned long() const;
    constexpr explicit operator long long() const;
    constexpr explicit operator unsigned long long() const;
#ifdef ABSL_HAVE_INTRINSIC_INT128
    constexpr explicit operator __int128() const;
    constexpr explicit operator unsigned __int128() const;
#endif  // ABSL_HAVE_INTRINSIC_INT128
    explicit operator float() const;
    explicit operator double() const;

    //
    // Arithmetic operators
    //
    constexpr s128 &operator+=(s128 other);
    constexpr s128 &operator-=(s128 other);
    constexpr s128 &operator*=(s128 other);
    constexpr s128 &operator/=(s128 other);
    constexpr s128 &operator%=(s128 other);
    constexpr s128 operator++(int);
    constexpr s128 operator--(int);
    constexpr s128 &operator++();
    constexpr s128 &operator--();
    constexpr s128 &operator&=(s128 other);
    constexpr s128 &operator|=(s128 other);
    constexpr s128 &operator^=(s128 other);
    constexpr s128 &operator<<=(int amount);
    constexpr s128 &operator>>=(int amount);
};

/*
// Specialized numeric_limits for s128.
namespace std {
template <>
class numeric_limits<s128> {
   public:
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr float_round_style round_style = round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = false;
    static constexpr int digits = 127;
    static constexpr int digits10 = 38;
    static constexpr int max_digits10 = 0;
    static constexpr int radix = 2;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
#ifdef ABSL_HAVE_INTRINSIC_INT128
    static constexpr bool traps = numeric_limits<__int128>::traps;
#else  // ABSL_HAVE_INTRINSIC_INT128
    static constexpr bool traps = numeric_limits<u64>::traps;
#endif  // ABSL_HAVE_INTRINSIC_INT128
    static constexpr bool tinyness_before = false;

    static constexpr s128(min)() { return Int128Min(); }
    static constexpr s128 lowest() { return Int128Min(); }
    static constexpr s128(max)() { return Int128Max(); }
    static constexpr s128 epsilon() { return 0; }
    static constexpr s128 round_error() { return 0; }
    static constexpr s128 infinity() { return 0; }
    static constexpr s128 quiet_NaN() { return 0; }
    static constexpr s128 signaling_NaN() { return 0; }
    static constexpr s128 denorm_min() { return 0; }
};
}  // namespace std
*/

//
// Implementation details:
//

constexpr u128 &u128::operator=(int v) { return *this = u128(v); }

constexpr u128 &u128::operator=(unsigned int v) {
    return *this = u128(v);
}

constexpr u128 &u128::operator=(long v) {
    return *this = u128(v);
}

constexpr u128 &u128::operator=(unsigned long v) {
    return *this = u128(v);
}

constexpr u128 &u128::operator=(long long v) {
    return *this = u128(v);
}

constexpr u128 &u128::operator=(unsigned long long v) {
    return *this = u128(v);
}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr u128 &u128::operator=(__int128 v) {
    return *this = u128(v);
}

constexpr u128 &u128::operator=(unsigned __int128 v) {
    return *this = u128(v);
}
#endif  // ABSL_HAVE_INTRINSIC_INT128

constexpr u128 &u128::operator=(s128 v) {
    return *this = u128(v);
}

constexpr u128 operator<<(u128 lhs, int amount);
constexpr u128 operator>>(u128 lhs, int amount);
constexpr u128 operator+(u128 lhs, u128 rhs);
constexpr u128 operator-(u128 lhs, u128 rhs);
constexpr u128 operator*(u128 lhs, u128 rhs);
constexpr u128 operator/(u128 lhs, u128 rhs);
constexpr u128 operator%(u128 lhs, u128 rhs);

constexpr u128 &u128::operator<<=(int amount) {
    *this = *this << amount;
    return *this;
}

constexpr u128 &u128::operator>>=(int amount) {
    *this = *this >> amount;
    return *this;
}

constexpr u128 &u128::operator+=(u128 other) {
    *this = *this + other;
    return *this;
}

constexpr u128 &u128::operator-=(u128 other) {
    *this = *this - other;
    return *this;
}

constexpr u128 &u128::operator*=(u128 other) {
    *this = *this * other;
    return *this;
}

constexpr u128 &u128::operator/=(u128 other) {
    *this = *this / other;
    return *this;
}

constexpr u128 &u128::operator%=(u128 other) {
    *this = *this % other;
    return *this;
}

#if ENDIAN == LITTLE_ENDIAN
constexpr u128::u128(u64 high, u64 low)
    : lo{low}, hi{high} {}

constexpr u128::u128(int v)
    : lo{(u64)(v)},
      hi{v < 0 ? U64_MAX : 0} {}
constexpr u128::u128(long v)
    : lo{(u64)(v)},
      hi{v < 0 ? U64_MAX : 0} {}
constexpr u128::u128(long long v)
    : lo{(u64)(v)},
      hi{v < 0 ? U64_MAX : 0} {}

constexpr u128::u128(unsigned int v) : lo{v}, hi{0} {}
constexpr u128::u128(unsigned long v) : lo{v}, hi{0} {}
constexpr u128::u128(unsigned long long v) : lo{v}, hi{0} {}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr u128::u128(__int128 v)
    : lo{(u64)(v & ~u64{0})},
      hi{(u64)((unsigned __int128) (v) >> 64)} {}
constexpr u128::u128(unsigned __int128 v)
    : lo{(u64)(v & ~u64{0})},
      hi{(u64)(v >> 64)} {}
#endif  // ABSL_HAVE_INTRINSIC_INT128

constexpr u128::u128(s128 v)
    : lo{v.lo}, hi{(u64)(v.hi)} {}

#elif ENDIAN == BIG_ENDIAN

constexpr u128::u128(u64 high, u64 low)
    : hi{high}, lo{low} {}

constexpr u128::u128(int v)
    : hi{v < 0 ? U64_MAX : 0},
      lo{(u64)(v)} {}
constexpr u128::u128(long v)
    : hi{v < 0 ? U64_MAX : 0},
      lo{(u64)(v)} {}
constexpr u128::u128(long long v)
    : hi{v < 0 ? U64_MAX : 0},
      lo{(u64)(v)} {}

constexpr u128::u128(unsigned int v) : hi{0}, lo{v} {}
constexpr u128::u128(unsigned long v) : hi{0}, lo{v} {}
constexpr u128::u128(unsigned long long v) : hi{0}, lo{v} {}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr u128::u128(__int128 v)
    : hi{(u64)((unsigned __int128) (v) >> 64)},
      lo{(u64)(v & ~u64{0})} {}
constexpr u128::u128(unsigned __int128 v)
    : hi{(u64)(v >> 64)},
      lo{(u64)(v & ~u64{0})} {}
#endif  // ABSL_HAVE_INTRINSIC_INT128

constexpr u128::u128(s128 v)
    : hi{(u64)(v.hi)}, lo{v.lo} {}

#else
#error "Unsupported byte order: must be little or big endian."
#endif

constexpr u128::operator bool() const { return lo || hi; }

constexpr u128::operator char() const { return static_cast<char>(lo); }

constexpr u128::operator signed char() const {
    return static_cast<signed char>(lo);
}

constexpr u128::operator unsigned char() const {
    return static_cast<unsigned char>(lo);
}

constexpr u128::operator char16_t() const {
    return static_cast<char16_t>(lo);
}

constexpr u128::operator char32_t() const {
    return static_cast<char32_t>(lo);
}

constexpr u128::operator wchar_t() const {
    return static_cast<wchar_t>(lo);
}

constexpr u128::operator short() const { return static_cast<short>(lo); }

constexpr u128::operator unsigned short() const {
    return static_cast<unsigned short>(lo);
}

constexpr u128::operator int() const { return static_cast<int>(lo); }

constexpr u128::operator unsigned int() const {
    return static_cast<unsigned int>(lo);
}

constexpr u128::operator long() const { return static_cast<long>(lo); }

constexpr u128::operator unsigned long() const {
    return static_cast<unsigned long>(lo);
}

constexpr u128::operator long long() const {
    return static_cast<long long>(lo);
}

constexpr u128::operator unsigned long long() const {
    return static_cast<unsigned long long>(lo);
}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr u128::operator __int128() const {
    return (static_cast<__int128>(hi) << 64) + lo;
}

constexpr u128::operator unsigned __int128() const {
    return ((unsigned __int128) (hi) << 64) + lo;
}
#endif  // ABSL_HAVE_INTRINSIC_INT128

extern "C" {
double ldexp(double, int);
}

// @TODO: Constexpr
inline u128::operator float() const {
    return (float) lo + (float) ldexp((double) hi, 64);
}

inline u128::operator double() const {
    return (double) lo + ldexp((double) hi, 64);
}

constexpr bool operator==(u128 lhs, u128 rhs) {
    return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

constexpr bool operator!=(u128 lhs, u128 rhs) {
    return !(lhs == rhs);
}

constexpr bool operator<(u128 lhs, u128 rhs) {
#ifdef ABSL_HAVE_INTRINSIC_INT128
    return (unsigned __int128) (lhs) <
           (unsigned __int128) (rhs);
#else
    return (lhs.hi == rhs.hi) ? (lhs.lo < rhs.lo) : (lhs.hi < rhs.hi);
#endif
}

constexpr bool operator>(u128 lhs, u128 rhs) { return rhs < lhs; }

constexpr bool operator<=(u128 lhs, u128 rhs) { return !(rhs < lhs); }

constexpr bool operator>=(u128 lhs, u128 rhs) { return !(lhs < rhs); }

// Unary operators.

constexpr u128 operator-(u128 val) {
    u64 hi = ~val.hi;
    u64 lo = ~val.lo;
    if (lo == 0) ++hi;  // carry
    return u128(hi, lo);
}

constexpr bool operator!(u128 val) {
    return !val.hi && !val.lo;
}

constexpr u128 operator~(u128 val) {
    return u128(~val.hi, ~val.lo);
}

constexpr u128 operator|(u128 lhs, u128 rhs) {
    return u128(lhs.hi | rhs.hi, lhs.lo | rhs.lo);
}

constexpr u128 operator&(u128 lhs, u128 rhs) {
    return u128(lhs.hi & rhs.hi, lhs.lo & rhs.lo);
}

constexpr u128 operator^(u128 lhs, u128 rhs) {
    return u128(lhs.hi ^ rhs.hi, lhs.lo ^ rhs.lo);
}

constexpr u128 &u128::operator|=(u128 other) {
    hi |= other.hi;
    lo |= other.lo;
    return *this;
}

constexpr u128 &u128::operator&=(u128 other) {
    hi &= other.hi;
    lo &= other.lo;
    return *this;
}

constexpr u128 &u128::operator^=(u128 other) {
    hi ^= other.hi;
    lo ^= other.lo;
    return *this;
}

constexpr u128 operator<<(u128 lhs, int amount) {
#ifdef ABSL_HAVE_INTRINSIC_INT128
    return (unsigned __int128) (lhs) << amount;
#else
    // u64 shifts of >= 64 are undefined, so we will need some
    // special-casing.
    if (amount < 64) {
        if (amount != 0) return u128((lhs.hi << amount) | (lhs.lo >> (64 - amount)), lhs.lo << amount);
        return lhs;
    }
    return u128(lhs.lo << (amount - 64), 0);
#endif
}

constexpr u128 operator>>(u128 lhs, int amount) {
#ifdef ABSL_HAVE_INTRINSIC_INT128
    return (unsigned __int128) (lhs) >> amount;
#else
    // u64 shifts of >= 64 are undefined, so we will need some
    // special-casing.
    if (amount < 64) {
        if (amount != 0) return u128(lhs.hi >> amount, (lhs.lo >> amount) | (lhs.hi << (64 - amount)));
        return lhs;
    }
    return u128(0, lhs.hi >> (amount - 64));
#endif
}

constexpr u128 operator+(u128 lhs, u128 rhs) {
    u128 result = u128(lhs.hi + rhs.hi, lhs.lo + rhs.lo);
    if (result.lo < lhs.lo) return u128(result.hi + 1, result.lo);
    return result;
}

constexpr u128 operator-(u128 lhs, u128 rhs) {
    u128 result = u128(lhs.hi - rhs.hi, lhs.lo - rhs.lo);
    if (lhs.lo < rhs.lo) return u128(result.hi - 1, result.lo);
    return result;
}

constexpr u128 operator*(u128 lhs, u128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
    // TODO(strel) Remove once alignment issues are resolved and unsigned __int128
    // can be used for u128 storage.
    return (unsigned __int128) (lhs) *
           (unsigned __int128) (rhs);
#else  // ABSL_HAVE_INTRINSIC128
    u64 a32 = lhs.lo >> 32;
    u64 a00 = lhs.lo & 0xffffffff;
    u64 b32 = rhs.lo >> 32;
    u64 b00 = rhs.lo & 0xffffffff;
    u128 result = u128(lhs.hi * rhs.lo + lhs.lo * rhs.hi + a32 * b32, a00 * b00);
    result += u128(a32 * b00) << 32;
    result += u128(a00 * b32) << 32;
    return result;
#endif  // ABSL_HAVE_INTRINSIC128
}

constexpr u128 u128::operator++(int) {
    u128 tmp(*this);
    *this += 1;
    return tmp;
}

constexpr u128 u128::operator--(int) {
    u128 tmp(*this);
    *this -= 1;
    return tmp;
}

constexpr u128 &u128::operator++() {
    *this += 1;
    return *this;
}

constexpr u128 &u128::operator--() {
    *this -= 1;
    return *this;
}

constexpr s128 MakeInt128(s64 high, u64 low) {
    return s128(high, low);
}

constexpr s128 &s128::operator=(int v) {
    return *this = s128(v);
}

constexpr s128 &s128::operator=(unsigned int v) {
    return *this = s128(v);
}

constexpr s128 &s128::operator=(long v) {
    return *this = s128(v);
}

constexpr s128 &s128::operator=(unsigned long v) {
    return *this = s128(v);
}

constexpr s128 &s128::operator=(long long v) {
    return *this = s128(v);
}

constexpr s128 &s128::operator=(unsigned long long v) {
    return *this = s128(v);
}

// Arithmetic operators.

constexpr s128 operator+(s128 lhs, s128 rhs);
constexpr s128 operator-(s128 lhs, s128 rhs);
constexpr s128 operator*(s128 lhs, s128 rhs);
constexpr s128 operator/(s128 lhs, s128 rhs);
constexpr s128 operator%(s128 lhs, s128 rhs);
constexpr s128 operator|(s128 lhs, s128 rhs);
constexpr s128 operator&(s128 lhs, s128 rhs);
constexpr s128 operator^(s128 lhs, s128 rhs);
constexpr s128 operator<<(s128 lhs, int amount);
constexpr s128 operator>>(s128 lhs, int amount);

constexpr s128 &s128::operator+=(s128 other) {
    *this = *this + other;
    return *this;
}

constexpr s128 &s128::operator-=(s128 other) {
    *this = *this - other;
    return *this;
}

constexpr s128 &s128::operator*=(s128 other) {
    *this = *this * other;
    return *this;
}

constexpr s128 &s128::operator/=(s128 other) {
    *this = *this / other;
    return *this;
}

constexpr s128 &s128::operator%=(s128 other) {
    *this = *this % other;
    return *this;
}

constexpr s128 &s128::operator|=(s128 other) {
    *this = *this | other;
    return *this;
}

constexpr s128 &s128::operator&=(s128 other) {
    *this = *this & other;
    return *this;
}

constexpr s128 &s128::operator^=(s128 other) {
    *this = *this ^ other;
    return *this;
}

constexpr s128 &s128::operator<<=(int amount) {
    *this = *this << amount;
    return *this;
}

constexpr s128 &s128::operator>>=(int amount) {
    *this = *this >> amount;
    return *this;
}

namespace int128_internal {

// Casts from unsigned to signed while preserving the underlying binary
// representation.
constexpr s64 BitCastToSigned(u64 v) {
    // Casting an unsigned integer to a signed integer of the same
    // width is implementation defined behavior if the source value would not fit
    // in the destination type. We step around it with a roundtrip bitwise not
    // operation to make sure this function remains constexpr. Clang, GCC, and
    // MSVC optimize this to a no-op on x86-64.
    return v & (u64{1} << 63) ? ~static_cast<s64>(~v)
                              : static_cast<s64>(v);
}

}  // namespace int128_internal

// #if defined(ABSL_HAVE_INTRINSIC_INT128)
// #include "absl/numeric/int128_have_intrinsic.inc"  // IWYU pragma: export
// #else  // ABSL_HAVE_INTRINSIC_INT128
// #include "absl/numeric/int128_no_intrinsic.inc"  // IWYU pragma: export
// #endif  // ABSL_HAVE_INTRINSIC_INT128
