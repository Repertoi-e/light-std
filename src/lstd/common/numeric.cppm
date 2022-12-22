module;

#include "namespace.h"
#include "platform.h"

#include <intrin.h> // for _BitScanReverse64 on MSVC @Platform

export module lstd.numeric;

export {
    //
    // The following integral types are defined here s8, s16, s32, s64, s128,
    //      and corresponding unsigned types: u8, u16, u32, u64, u128),
    //      vector types (aligned on 16 byte boundaries for SIMDs),
    //		f32 (float), f64 (double), wchar (for Windows), 
    //      code_point (for the integer value of a Unicode code point), 
    //      and byte (unsigned char)
    //
    // Note: We don't support long doubles (lf64) or operations with them throughout the library.
    //
    //
    // We also define numeric<T>, a way to get info about numbers, e.g. min/max, 
    // max digits, for floats: exponent, mantissa bits, etc.
    //
    // numeric<T> is useful when writing template functions and you don't know 
    // the specific integral type, so you cannot just assume the max size for 
    // 32 bit integer for example. In that case you can use numeric<T>::max()
    //

    //
    // Fundamental types:
    //
    using s8 = char;
    using s16 = short;
    using s32 = int;
    using s64 = long long;

    using u8 = unsigned char;
    using u16 = unsigned short;
    using u32 = unsigned;
    using u64 = unsigned long long;

#if COMPILER == MSVC
    using wchar = wchar_t;   // Only useful for Windows calls. Please don't use utf-16 in your programs...
    using code_point = char32_t;  // Holds the integer value of a Unicode cp.
#else
#error Implement.
#endif

    using byte = unsigned char;

    using f32 = float;
    using f64 = double;

    //
    // Vector types (aligned on 16 byte boundaries for SIMDs)
    //
}

template <typename T, s64 Count> union alignas(16) base_vector_type { T Values[Count]; };

export {
    using u8v16 = base_vector_type<u8, 16>;
    using u16v8 = base_vector_type<u16, 8>;
    using u32v4 = base_vector_type<u32, 4>;
    using u64v2 = base_vector_type<u64, 2>;

    using s8v16 = base_vector_type<s8, 16>;
    using s16v8 = base_vector_type<s16, 8>;
    using s32v4 = base_vector_type<s32, 4>;
    using s64v2 = base_vector_type<s64, 2>;
    using f32v4 = base_vector_type<f32, 4>;
    using f64v2 = base_vector_type<f64, 2>;
}

LSTD_BEGIN_NAMESPACE

struct numeric_base {
    static constexpr bool is_integral = false;
    static constexpr s32 digits = 0;
    static constexpr s32 digits10 = 0;
    static constexpr s32 max_digits10 = 0;
};

struct numeric_integer_base : numeric_base {
    static constexpr bool is_integral = true;
};

export {
    template <typename T>
    struct numeric : public numeric_base {
        using value_t = T;
    };

    // Const/volatile variations of numeric.
    template <typename T> struct numeric<const T> : public numeric<T> {};
    template <typename T> struct numeric<volatile T> : public numeric<T> {};
    template <typename T> struct numeric<const volatile T> : public numeric<T> {};

    template <>
    struct numeric<char> : public numeric_integer_base {
        static constexpr char min() { return (-128); }
        static constexpr char max() { return (127); }

        static constexpr s32 digits = 8;
        static constexpr s32 digits10 = 2;
    };

    template <>
    struct numeric<wchar_t> : public numeric_integer_base {
        static constexpr wchar_t min() { return 0x0000; }
        static constexpr wchar_t max() { return 0xffff; }

        static constexpr s32 digits = 16;
        static constexpr s32 digits10 = 4;
    };

    template <>
    struct numeric<bool> : public numeric_integer_base {
        // limits for type bool
        static constexpr bool min() { return false; }
        static constexpr bool max() { return true; }

        static constexpr s32 digits = 1;
    };

    template <>
    struct numeric<u8> : public numeric_integer_base {
        static constexpr u8 min() { return 0; }
        static constexpr u8 max() { return (255); }

        static constexpr s32 digits = 8;
        static constexpr s32 digits10 = 2;
    };

    template <>
    struct numeric<s16> : public numeric_integer_base {
        static constexpr s16 min() { return (-32768); }
        static constexpr s16 max() { return (32767); }

        static constexpr s32 digits = 15;
        static constexpr s32 digits10 = 4;
    };

#ifdef _NATIVE_WCHAR_T_DEFINED
    template <>
    struct numeric<u16> : public numeric_integer_base {
        static constexpr u16 min() { return 0; }
        static constexpr u16 max() { return (65535); }

        static constexpr s32 digits = 16;
        static constexpr s32 digits10 = 4;
    };
#endif

    template <>
    struct numeric<char8_t> : public numeric_integer_base {
        static constexpr char8_t min() { return 0; }
        static constexpr char8_t max() { return (255); }

        static constexpr s32 digits = 8;
        static constexpr s32 digits10 = 2;
    };

    template <>
    struct numeric<char16_t> : public numeric_integer_base {
        static constexpr char16_t min() { return 0; }
        static constexpr char16_t max() { return (65535); }

        static constexpr s32 digits = 16;
        static constexpr s32 digits10 = 4;
    };

    template <>
    struct numeric<s32> : public numeric_integer_base {
        static constexpr s32 min() { return (-2147483647 - 1); }
        static constexpr s32 max() { return (2147483647); }

        static constexpr s32 digits = 31;
        static constexpr s32 digits10 = 9;
    };

    template <>
    struct numeric<u32> : public numeric_integer_base {
        static constexpr u32 min() { return 0; }
        static constexpr u32 max() { return (4294967295U); }

        static constexpr s32 digits = 32;
        static constexpr s32 digits10 = 9;
    };

    template <>
    struct numeric<long> : public numeric_integer_base {
        static_assert(sizeof s32 == sizeof(long));
        static constexpr long min() { return (-2147483647 - 1); }
        static constexpr long max() { return (2147483647); }

        static constexpr s32 digits = 31;
        static constexpr s32 digits10 = 9;
    };

    template <>
    struct numeric<unsigned long> : public numeric_integer_base {
        static_assert(sizeof u32 == sizeof(unsigned long));
        static constexpr unsigned long min() { return 0; }
        static constexpr unsigned long max() { return (4294967295U); }

        static constexpr s32 digits = 32;
        static constexpr s32 digits10 = 9;
    };

    template <>
    struct numeric<char32_t> : public numeric_integer_base {
    public:
        static constexpr char32_t min() { return 0; }
        static constexpr char32_t max() { return (4294967295U); }

        static constexpr s32 digits = 32;
        static constexpr s32 digits10 = 9;
    };

#define S64_C(c) c##L
#define U64_C(c) c##UL

    template <>
    struct numeric<s64> : public numeric_integer_base {
        static constexpr s64 min() { return (-S64_C(9223372036854775807L) - 1); }
        static constexpr s64 max() { return (S64_C(9223372036854775807)); }

        static constexpr s32 digits = 63;
        static constexpr s32 digits10 = 18;
    };

    template <>
    struct numeric<u64> : public numeric_integer_base {
    public:
        static constexpr u64 min() { return 0; }
        static constexpr u64 max() { return (U64_C(18446744073709551615)); }

        static constexpr s32 digits = 64;
        static constexpr s32 digits10 = 19;
    };

    template <>
    struct numeric<f32> {
    public:
        static constexpr f32 min() { return 1.175494351e-38F; }
        static constexpr f32 max() { return 3.402823466e+38F; }
        static constexpr f32 epsilon() { return  1.192092896e-07F; } // smallest suchthat 1.0 + epsilon != 1.0
        static constexpr f32 round_error() { return 0.5F; }
        static constexpr f32 denorm_min() { return 1.401298464e-45F; }
        static constexpr f32 infinity() { return __builtin_huge_valf(); }
        static constexpr f32 quiet_NaN() { return __builtin_nanf("0"); }
        static constexpr f32 signaling_NaN() { return __builtin_nansf("1"); }

        static constexpr s32 digits = 23 + 1; // including the hidden bit
        static constexpr s32 digits10 = 6; // # of decimal digits of precision
        static constexpr s32 max_digits10 = 9; // # of decimal digits of precision
        static constexpr s32 max_exponent = 127;
        static constexpr s32 max_exponent10 = 38;
        static constexpr s32 min_exponent = -126;
        static constexpr s32 min_exponent10 = -37;
        static constexpr s32 bits_mantissa = 23; // # of bits in mantissa, excluding the hidden bit (which is always interpreted as 1 for normal numbers)
        static constexpr s32 bits_exponent = 8;
        static constexpr s32 exponent_bias = 127;
    };

    template <>
    struct numeric<f64> {
    public:
        static constexpr f64 min() { return 2.2250738585072014e-308; }
        static constexpr f64 max() { return 1.7976931348623158e+308; }
        static constexpr f64 epsilon() { return  2.2204460492503131e-016; } // smalles such that 1.0 + epsilon != 1.0
        static constexpr f64 round_error() { return 0.5; }
        static constexpr f64 denorm_min() { return 4.9406564584124654e-324; }
        static constexpr f64 infinity() { return __builtin_huge_val(); }
        static constexpr f64 quiet_NaN() { return __builtin_nan("0"); }
        static constexpr f64 signaling_NaN() { return __builtin_nans("1"); }

        static constexpr s32 digits = 52 + 1; // including the hidden bit
        static constexpr s32 digits10 = 15; // # of decimal digits of precision
        static constexpr s32 max_digits10 = 17; // # of decimal digits of precision
        static constexpr s32 max_exponent = 1023;
        static constexpr s32 max_exponent10 = 308;
        static constexpr s32 min_exponent = -1022;
        static constexpr s32 min_exponent10 = -307;
        static constexpr s32 bits_mantissa = 52;  // number of bits in mantissa,excluding the hidden bit (which is always interpreted as 1 for normalnumbers)
        static constexpr s32 bits_exponent = 11;
        static constexpr s32 exponent_bias = 1023;
    };
}

LSTD_END_NAMESPACE

//
// Intrinsic-like s128 and u128 types:
//

struct s128;

export {
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
    // However, a `u128` differs from intrinsic integral types in the following ways:
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

        //
        // Constructors from arithmetic types
        //
#if ENDIAN == LITTLE_ENDIAN
        constexpr u128(u64 high, u64 low) : lo{ low }, hi{ high } {}
        constexpr u128(s32 v) : lo{ (u64)(v) }, hi{ v < 0 ? numeric<u64>::max() : 0 } {}
        constexpr u128(s64 v) : lo{ (u64)(v) }, hi{ v < 0 ? numeric<u64>::max() : 0 } {}
        constexpr u128(u32 v) : lo{ v }, hi{ 0 } {}
        constexpr u128(u64 v) : lo{ v }, hi{ 0 } {}
#elif ENDIAN == BIG_ENDIAN
        constexpr u128(u64 high, u64 low) : hi{ high }, lo{ low } {}
        constexpr u128(s32 v) : hi{ v < 0 ? numeric<u64>::max() : 0 }, lo{ (u64)(v) } {}
        constexpr u128(s64 v) : hi{ v < 0 ? numeric<u64>::max() : 0 }, lo{ (u64)(v) } {}
        constexpr u128(u32 v) : hi{ 0 }, lo{ v } {}
        constexpr u128(u64 v) : hi{ 0 }, lo{ v } {}
#else
#error "Unsupported byte order: must be little or big endian."
#endif
        constexpr u128(s128 v);
        constexpr explicit u128(float v);
        constexpr explicit u128(double v);

        //
        // Assignment operators from arithmetic types
        //
        constexpr u128& operator=(s32 v) { return *this = u128(v); }
        constexpr u128& operator=(u32 v) { return *this = u128(v); }
        constexpr u128& operator=(s64 v) { return *this = u128(v); }
        constexpr u128& operator=(u64 v) { return *this = u128(v); }
        constexpr u128& operator=(s128 v);

        //
        // Conversion operators to other arithmetic types
        //
        constexpr explicit operator bool() const { return lo || hi; }
        constexpr explicit operator s8() const { return (s8)lo; }
        constexpr explicit operator u8() const { return (u8)lo; }
        constexpr explicit operator code_point() const { return (code_point)lo; }
        constexpr explicit operator wchar() const { return (wchar)lo; }
        constexpr explicit operator s16() const { return (s16)lo; }
        constexpr explicit operator u16() const { return (u16)lo; }
        constexpr explicit operator s32() const { return (s32)lo; }
        constexpr explicit operator u32() const { return (u32)lo; }
        constexpr explicit operator s64() const { return (s64)lo; }
        constexpr explicit operator u64() const { return (u64)lo; }
        explicit operator float() const;
        explicit operator double() const;

        //
        // Arithmetic operators
        //
        constexpr u128& operator+=(u128 other);
        constexpr u128& operator-=(u128 other);
        constexpr u128& operator*=(u128 other);
        constexpr u128& operator/=(u128 other);
        constexpr u128& operator%=(u128 other);
        constexpr u128 operator++(s32);
        constexpr u128 operator--(s32);
        constexpr u128& operator<<=(s32);
        constexpr u128& operator>>=(s32);
        constexpr u128& operator&=(u128 other);
        constexpr u128& operator|=(u128 other);
        constexpr u128& operator^=(u128 other);
        constexpr u128& operator++();
        constexpr u128& operator--();
};
}

// Casts from unsigned to signed while preserving the underlying binary representation.
constexpr s64 s64_bit_cast_to_u64(u64 v) {
    // Casting an unsigned integer to a signed integer of the same
    // width is implementation defined behavior if the source value would not fit
    // in the destination type. We step around it with a roundtrip bitwise not
    // operation to make sure this function remains constexpr. Clang, GCC, and
    // MSVC optimize this to a no-op on x86-64.
    return v & (u64{ 1 } << 63) ? ~((s64)(~v)) : ((s64)(v));
}

export {
    //
    // s128
    //
    // A signed 128-bit integer type. See docs for u128.
    //
    // The design goal for `s128` is that it will be compatible with a future
    // `int128_t`, if that type becomes a part of the standard.
    //
    // However, an `s128` differs from intrinsic integral types in the following ways:
    //
    //   * It is not implicitly convertible to other integral types.
    //   * Requires explicit construction from and conversion to floating point
    //     types.
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

        //
        // Constructors from arithmetic types
        //

#if ENDIAN == LITTLE_ENDIAN
        constexpr s128(s64 high, u64 low) : lo(low), hi(high) {}
        constexpr s128(s32 v) : lo{ (u64)v }, hi{ v < 0 ? ~s64{0} : 0 } {}
        constexpr s128(s64 v) : lo{ (u64)v }, hi{ v < 0 ? ~s64{0} : 0 } {}
        constexpr s128(u32 v) : lo{ v }, hi{ 0 } {}
        constexpr s128(u64 v) : lo{ v }, hi{ 0 } {}
        explicit constexpr s128(u128 v) : lo{ v.lo }, hi{ (s64)v.hi } {}
#elif ENDIAN == BIG_ENDIAN
        constexpr s128(s64 high, u64 low) : hi{ high }, lo{ low } {}
        constexpr s128(s32 v) : hi{ v < 0 ? ~s64{0} : 0 }, lo{ (u64)v } {}
        constexpr s128(s64 v) : hi{ v < 0 ? ~s64{0} : 0 }, lo{ (u64)v } {}
        constexpr s128(u32 v) : hi{ 0 }, lo{ v } {}
        constexpr s128(u64 v) : hi{ 0 }, lo{ v } {}
        explicit constexpr s128(u128 v) : hi{ (s64)v.hi }, lo{ v.lo } {}
#else
#error "Unsupported byte order: must be little or big endian."
#endif
        explicit s128(float v);
        explicit s128(double v);

        //
        // Assignment operators from arithmetic types
        //
        constexpr s128& operator=(s32 v) { return *this = s128(v); }
        constexpr s128& operator=(u32 v) { return *this = s128(v); }
        constexpr s128& operator=(s64 v) { return *this = s128(v); }
        constexpr s128& operator=(u64 v) { return *this = s128(v); }

        //
        // Conversion operators to other arithmetic types
        //
        constexpr explicit operator bool() const { return lo || hi; }
        constexpr explicit operator s8() const { return (s8)(s64)(*this); }
        constexpr explicit operator u8() const { return (u8)lo; }
        constexpr explicit operator wchar() const { return (wchar)(s64)(*this); }
        constexpr explicit operator code_point() const { return (code_point)lo; }
        constexpr explicit operator s16() const { return (s16)(s64)(*this); }
        constexpr explicit operator u16() const { return (u16)lo; }
        constexpr explicit operator s32() const { return (s32)(s64)(*this); }
        constexpr explicit operator u32() const { return (u32)lo; }

        constexpr explicit operator s64() const {
            // We don't bother checking the value of hi. If *this < 0, lo's high bit
            // must be set in order for the value to fit into a s64. Conversely, if
            // lo's high bit is set, *this must be < 0 for the value to fit.
            return s64_bit_cast_to_u64(lo);
        }

        constexpr explicit operator u64() const { return lo; }
        explicit operator float() const;
        explicit operator double() const;

        //
        // Arithmetic operators
        //
        constexpr s128& operator+=(s128 other);
        constexpr s128& operator-=(s128 other);
        constexpr s128& operator*=(s128 other);
        constexpr s128& operator/=(s128 other);
        constexpr s128& operator%=(s128 other);
        constexpr s128 operator++(s32);
        constexpr s128 operator--(s32);
        constexpr s128& operator++();
        constexpr s128& operator--();
        constexpr s128& operator&=(s128 other);
        constexpr s128& operator|=(s128 other);
        constexpr s128& operator^=(s128 other);
        constexpr s128& operator<<=(s32 amount);
        constexpr s128& operator>>=(s32 amount);
};

constexpr u128& u128::operator=(s128 v) { return *this = u128(v); }

#if ENDIAN == LITTLE_ENDIAN
constexpr u128::u128(s128 v) : lo{ v.lo }, hi{ (u64)(v.hi) } {}
#elif ENDIAN == BIG_ENDIAN
constexpr u128::u128(s128 v) : hi{ (u64)(v.hi) }, lo{ v.lo } {}
#else
#error "Unsupported byte order: must be little or big endian."
#endif
}

extern "C" {
#if defined LSTD_DONT_DEFINE_STD && COMPILER == MSVC
    __declspec(dllimport) double ldexp(double, s32);  // Sigh...
#else
    double ldexp(double, s32);
#endif
}  // TODO: Constexpr

export {
    inline u128::operator float() const { return (float)lo + (float)ldexp((double)hi, 64); }
    inline u128::operator double() const { return (double)lo + ldexp((double)hi, 64); }

    /*
    inline s128::operator float() const {
        // We must convert the absolute value and then negate as needed, because
        // floating point types are typically sign-magnitude. Otherwise, the
        // difference between the high and low 64 bits when interpreted as two's
        // complement overwhelms the precision of the mantissa.
        //
        // Also check to make sure we don't negate Int128Min()
        return hi < 0 && *this != Int128Min() ? -static_cast<float>(-*this) : (float) lo + ldexp((float) hi, 64);
    }
    inline s128::operator double() const {  // See comment in s128::operator float() above.
        return hi < 0 && *this != Int128Min() ? -static_cast<double>(-*this) : (double) lo + ldexp((double) hi, 64);
    }
    */

    constexpr bool operator==(u128 lhs, u128 rhs) { return lhs.lo == rhs.lo && lhs.hi == rhs.hi; }
    constexpr bool operator!=(u128 lhs, u128 rhs) { return !(lhs == rhs); }
    constexpr bool operator<(u128 lhs, u128 rhs) { return (lhs.hi == rhs.hi) ? (lhs.lo < rhs.lo) : (lhs.hi < rhs.hi); }
    constexpr bool operator>(u128 lhs, u128 rhs) { return rhs < lhs; }
    constexpr bool operator<=(u128 lhs, u128 rhs) { return !(rhs < lhs); }
    constexpr bool operator>=(u128 lhs, u128 rhs) { return !(lhs < rhs); }

    constexpr bool operator==(s128 lhs, s128 rhs) { return (lhs.lo == rhs.lo && lhs.hi == rhs.hi); }
    constexpr bool operator!=(s128 lhs, s128 rhs) { return !(lhs == rhs); }
    constexpr bool operator<(s128 lhs, s128 rhs) { return (lhs.hi == rhs.hi) ? (lhs.lo < rhs.lo) : (lhs.hi < rhs.hi); }
    constexpr bool operator>(s128 lhs, s128 rhs) { return (lhs.hi == rhs.hi) ? (lhs.lo > rhs.lo) : (lhs.hi > rhs.hi); }
    constexpr bool operator<=(s128 lhs, s128 rhs) { return !(lhs > rhs); }
    constexpr bool operator>=(s128 lhs, s128 rhs) { return !(lhs < rhs); }

    constexpr u128 operator-(u128 val) {
        u64 hi = ~val.hi;
        u64 lo = ~val.lo;
        if (lo == 0) ++hi;  // carry
        return u128(hi, lo);
    }

    constexpr bool operator!(u128 val) { return !val.hi && !val.lo; }
    constexpr u128 operator~(u128 val) { return u128(~val.hi, ~val.lo); }
    constexpr u128 operator|(u128 lhs, u128 rhs) { return u128(lhs.hi | rhs.hi, lhs.lo | rhs.lo); }
    constexpr u128 operator&(u128 lhs, u128 rhs) { return u128(lhs.hi & rhs.hi, lhs.lo & rhs.lo); }
    constexpr u128 operator^(u128 lhs, u128 rhs) { return u128(lhs.hi ^ rhs.hi, lhs.lo ^ rhs.lo); }

    constexpr u128& u128::operator|=(u128 other) {
        hi |= other.hi;
        lo |= other.lo;
        return *this;
    }

    constexpr u128& u128::operator&=(u128 other) {
        hi &= other.hi;
        lo &= other.lo;
        return *this;
    }

    constexpr u128& u128::operator^=(u128 other) {
        hi ^= other.hi;
        lo ^= other.lo;
        return *this;
    }

    constexpr u128 operator<<(u128 lhs, s32 amount) {
        // u64 shifts of >= 64 are undefined, so we will need to check some special cases
        if (amount < 64) {
            if (amount != 0) return u128((lhs.hi << amount) | (lhs.lo >> (64 - amount)), lhs.lo << amount);
            return lhs;
        }
        return u128(lhs.lo << (amount - 64), 0);
    }

    constexpr u128 operator>>(u128 lhs, s32 amount) {
        // u64 shifts of >= 64 are undefined, so we will need to check some special cases
        if (amount < 64) {
            if (amount != 0) return u128(lhs.hi >> amount, (lhs.lo >> amount) | (lhs.hi << (64 - amount)));
            return lhs;
        }
        return u128(0, lhs.hi >> (amount - 64));
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
        u64 a32 = lhs.lo >> 32;
        u64 a00 = lhs.lo & 0xffffffff;
        u64 b32 = rhs.lo >> 32;
        u64 b00 = rhs.lo & 0xffffffff;
        u128 result = u128(lhs.hi * rhs.lo + lhs.lo * rhs.hi + a32 * b32, a00 * b00);
        result += u128(a32 * b00) << 32;
        result += u128(a00 * b32) << 32;
        return result;
    }

    constexpr void div_mod(u128 dividend, u128 divisor, u128* quotient_ret, u128* remainder_ret);

    constexpr u128 operator/(u128 lhs, u128 rhs) {
        u128 quotient = 0, remainder = 0;
        div_mod(lhs, rhs, &quotient, &remainder);
        return quotient;
    }

    constexpr u128 operator%(u128 lhs, u128 rhs) {
        u128 quotient = 0, remainder = 0;
        div_mod(lhs, rhs, &quotient, &remainder);
        return remainder;
    }

    constexpr u128 u128::operator++(s32) {
        u128 tmp(*this);
        *this += 1;
        return tmp;
    }

    constexpr u128 u128::operator--(s32) {
        u128 tmp(*this);
        *this -= 1;
        return tmp;
    }

    constexpr u128& u128::operator++() {
        *this += 1;
        return *this;
    }

    constexpr u128& u128::operator--() {
        *this -= 1;
        return *this;
    }

    constexpr s128 operator-(s128 v) {
        s64 hi = ~v.hi;
        u64 lo = ~v.lo + 1;
        if (lo == 0) ++hi;  // carry
        return s128(hi, lo);
    }

    constexpr bool operator!(s128 v) { return !v.lo && !v.hi; }

    constexpr s128 operator~(s128 val) { return s128(~val.hi, ~val.lo); }

    constexpr s128 operator+(s128 lhs, s128 rhs) {
        s128 result = s128(lhs.hi + rhs.hi, lhs.lo + rhs.lo);
        if (result.lo < lhs.lo) {  // check for carry
            return s128(result.hi + 1, result.lo);
        }
        return result;
    }

    constexpr s128 operator-(s128 lhs, s128 rhs) {
        s128 result = s128(lhs.hi - rhs.hi, lhs.lo - rhs.lo);
        if (lhs.lo < rhs.lo) {  // check for carry
            return s128(result.hi - 1, result.lo);
        }
        return result;
    }

    constexpr s128 operator*(s128 lhs, s128 rhs) {
        u128 result = u128(lhs) * rhs;
        return s128(s64_bit_cast_to_u64(result.hi), result.lo);
    }

    constexpr u128 unsigned_absolute_value(s128 v) {
        return v.hi < 0 ? -u128(v) : u128(v);  // Cast to uint128 before possibly negating because -Int128Min() is undefined.
    }

    constexpr s128 operator/(s128 lhs, s128 rhs) {
        // assert(lhs != Int128Min() || rhs != -1);  // UB on two's complement.

        u128 quotient = 0, remainder = 0;
        div_mod(unsigned_absolute_value(lhs), unsigned_absolute_value(rhs), &quotient, &remainder);
        if ((lhs.hi < 0) != (rhs.hi < 0)) quotient = -quotient;
        return s128(s64_bit_cast_to_u64(quotient.hi), quotient.lo);
    }

    constexpr s128 operator%(s128 lhs, s128 rhs) {
        // assert(lhs != Int128Min() || rhs != -1);  // UB on two's complement.

        u128 quotient = 0, remainder = 0;
        div_mod(unsigned_absolute_value(lhs), unsigned_absolute_value(rhs), &quotient, &remainder);
        if (lhs.hi < 0) remainder = -remainder;
        return s128(s64_bit_cast_to_u64(remainder.hi), remainder.lo);
    }

    constexpr s128 s128::operator++(s32) {
        s128 tmp(*this);
        *this += 1;
        return tmp;
    }

    constexpr s128 s128::operator--(s32) {
        s128 tmp(*this);
        *this -= 1;
        return tmp;
    }

    constexpr s128& s128::operator++() {
        *this += 1;
        return *this;
    }

    constexpr s128& s128::operator--() {
        *this -= 1;
        return *this;
    }

    constexpr s128 operator|(s128 lhs, s128 rhs) { return s128(lhs.hi | rhs.hi, lhs.lo | rhs.lo); }
    constexpr s128 operator&(s128 lhs, s128 rhs) { return s128(lhs.hi & rhs.hi, lhs.lo & rhs.lo); }
    constexpr s128 operator^(s128 lhs, s128 rhs) { return s128(lhs.hi ^ rhs.hi, lhs.lo ^ rhs.lo); }

    constexpr s128 operator<<(s128 lhs, s32 amount) {
        // u64 shifts of >= 64 are undefined, so we will need to check some special cases
        if (amount < 64) {
            if (amount != 0) return s128((lhs.hi << amount) | (s64)(lhs.lo >> (64 - amount)), lhs.lo << amount);
            return lhs;
        }
        return s128((s64)(lhs.lo << (amount - 64)), 0);
    }

    constexpr s128 operator>>(s128 lhs, s32 amount) {
        // u64 shifts of >= 64 are undefined, so we will need to check some special cases
        if (amount < 64) {
            if (amount != 0) return s128(lhs.hi >> amount, (lhs.lo >> amount) | ((u64)lhs.hi << (64 - amount)));
            return lhs;
        }
        return s128(0, (u64)(lhs.hi >> (amount - 64)));
    }

    constexpr u128& u128::operator<<=(s32 amount) { return (*this = *this << amount), * this; }
    constexpr u128& u128::operator>>=(s32 amount) { return (*this = *this >> amount), * this; }
    constexpr u128& u128::operator+=(u128 other) { return (*this = *this + other), * this; }
    constexpr u128& u128::operator-=(u128 other) { return (*this = *this - other), * this; }
    constexpr u128& u128::operator*=(u128 other) { return (*this = *this * other), * this; }
    constexpr u128& u128::operator/=(u128 other) { return (*this = *this / other), * this; }
    constexpr u128& u128::operator%=(u128 other) { return (*this = *this % other), * this; }
    constexpr s128& s128::operator+=(s128 other) { return (*this = *this + other), * this; }
    constexpr s128& s128::operator-=(s128 other) { return (*this = *this - other), * this; }
    constexpr s128& s128::operator*=(s128 other) { return (*this = *this * other), * this; }
    constexpr s128& s128::operator/=(s128 other) { return (*this = *this / other), * this; }
    constexpr s128& s128::operator%=(s128 other) { return (*this = *this % other), * this; }
    constexpr s128& s128::operator|=(s128 other) { return (*this = *this | other), * this; }
    constexpr s128& s128::operator&=(s128 other) { return (*this = *this & other), * this; }
    constexpr s128& s128::operator^=(s128 other) { return (*this = *this ^ other), * this; }
    constexpr s128(&s128::operator<<=(s32 amount)) { return (*this = *this << amount), * this; }
    constexpr s128& s128::operator>>=(s32 amount) { return (*this = *this >> amount), * this; }
}

LSTD_BEGIN_NAMESPACE

export {
    template <>
    struct numeric<u128> : public numeric_integer_base {
    public:
        static constexpr u128 min() { return 0; }
        static constexpr u128 max() { return u128(numeric<u64>::max(), numeric<u64>::max()); }

        static constexpr s32 digits = 128;
        static constexpr s32 digits10 = 38;
    };

    template <>
    struct numeric<s128> : public numeric_integer_base {
        static constexpr s128 min() { return s128(numeric<s64>::min(), 0); }
        static constexpr s128 max() { return s128(numeric<s64>::max(), numeric<u64>::max()); }

        static constexpr s32 digits = 127;
        static constexpr s32 digits10 = 38;
    };
}

LSTD_END_NAMESPACE

#if COMPILER == MSVC
#pragma intrinsic(_BitScanReverse64)
#endif

s32 msb(u64 x) {
    unsigned long r = 0;
    return _BitScanReverse64(&r, x) ? ((s32)r) : -1; // @Platform
}

s32 msb(u128 x) {
    if (x.hi != 0) return 64 + msb(x.hi);
    return msb(x.lo);
}

// Long division/modulo for u128 implemented using the shiftsubtractdivisionalgorithmadaptedfrom:
// https://stackoverflow.com/questions/5386377/division-without-using
constexpr void div_mod(u128 dividend, u128 divisor, u128* quotient_ret, u128* remainder_ret) {
    LSTD_USING_NAMESPACE;

    if (divisor == 0) return;

    if (divisor > dividend) {
        *quotient_ret = 0;
        *remainder_ret = dividend;
        return;
    }

    if (divisor == dividend) {
        *quotient_ret = 1;
        *remainder_ret = 0;
        return;
    }

    u128 denominator = divisor;
    u128 quotient = 0;

    // Left aligns the MSB of the denominator and the dividend.
    s32 shift = msb(dividend) - msb(denominator);
    denominator <<= shift;

    // Uses shift-subtract algorithm to divide dividend by denominator. The
    // remainder will be left in dividend.
    for (s32 i = 0; i <= shift; ++i) {
        quotient <<= 1;
        if (dividend >= denominator) {
            dividend -= denominator;
            quotient |= 1;
        }
        denominator >>= 1;
    }

    *quotient_ret = quotient;
    *remainder_ret = dividend;
}