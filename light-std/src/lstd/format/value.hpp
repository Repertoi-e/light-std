#pragma once

#include "../common.hpp"

#include "../string/string.hpp"

#if !defined LSTD_NO_CRT
#include <locale>
#endif

#if COMPILER != MSVC
#define CLZ(n) __builtin_clz(n)
#define CLZLL(n) __builtin_clzll(n)
#endif

#if COMPILER == MSVC
#include <intrin.h>
LSTD_BEGIN_NAMESPACE
namespace fmt::internal {
#pragma intrinsic(_BitScanReverse)

inline u32 clz(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}
#define CLZ(n) internal::clz(n)

#pragma intrinsic(_BitScanReverse64)
inline u32 clzll(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse64(&r, x);
    return 63 - r;
}
#define CLZLL(n) internal::clzll(n)
}  // namespace fmt::internal
LSTD_END_NAMESPACE
#endif

LSTD_BEGIN_NAMESPACE

namespace fmt {

namespace internal {
#define POWERS_OF_10(factor)                                                                                        \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, \
        factor * 100000000, factor * 1000000000

inline constexpr u32 POWERS_OF_10_32[] = {1, POWERS_OF_10(1)};
inline constexpr u64 POWERS_OF_10_64[] = {1, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

inline constexpr u32 ZERO_OR_POWERS_OF_10_32[] = {0, POWERS_OF_10(1)};
inline constexpr u64 ZERO_OR_POWERS_OF_10_64[] = {0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull),
                                                  10000000000000000000ull};

inline constexpr char RESET_COLOR[] = "\x1b[0m";

#if !defined LSTD_FMT_THOUSANDS_SEPARATOR
#if !defined LSTD_NO_CRT
inline char32_t thousands_separator() {
    std::locale loc = std::locale();
    return (char32_t) std::use_facet<std::numpunct<wchar_t>>(loc).thousands_sep();
}
#else
inline char32_t thousands_separator() { return ','; }
#endif
#else
inline char32_t thousands_separator() { return LSTD_FMT_THOUSANDS_SEPARATOR; }
#endif
}  // namespace internal

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
constexpr std::enable_if_t<std::numeric_limits<T>::is_signed, bool> is_negative(T value) {
    return value < 0;
}

template <typename T>
constexpr std::enable_if_t<!std::numeric_limits<T>::is_signed, bool> is_negative(T) {
    return false;
}

// Casts nonnegative integer to unsigned.
template <typename T>
constexpr std::make_unsigned_t<T> to_unsigned(T value) {
    assert(value >= 0);
    return (std::make_unsigned_t<T>) value;
}

inline bool is_infinity(f64 x) {
    union {
        u64 u;
        double f;
    } ieee754;
    ieee754.f = x;
    return ((u32)(ieee754.u >> 32) & 0x7fffffff) == 0x7ff00000 && ((u32) ieee754.u == 0);
}

inline bool is_nan(f64 x) {
    union {
        u64 u;
        double f;
    } ieee754;
    ieee754.f = x;
    return ((u32)(ieee754.u >> 32) & 0x7fffffff) + ((u32) ieee754.u != 0) > 0x7ff00000;
}

namespace internal {

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline u32 count_digits(u64 n) {
    s32 t = (64 - CLZLL(n | 1)) * 1233 >> 12;
    return to_unsigned(t) - (n < ZERO_OR_POWERS_OF_10_64[t]) + 1;
}

template <unsigned BITS, typename UInt>
u32 count_digits(UInt value) {
    UInt n = value;
    u32 numDigits = 0;
    do {
        ++numDigits;
    } while ((n >>= BITS) != 0);
    return numDigits;
}

}  // namespace internal

enum class format_type {
    NONE = 0,
    NAMED_ARGUMENT,

    // Integers
    S32,
    U32,
    S64,
    U64,
    BOOL,
    LAST_INTEGER_TYPE = BOOL,

    // Floating-point
    F64,
    LAST_NUMERIC_TYPE = F64,

    CSTRING,
    STRING,
    POINTER,
    CUSTOM
};
inline format_type operator|(format_type lhs, format_type rhs) {
    using T = std::underlying_type_t<format_type>;
    return (format_type)((T) lhs | (T) rhs);
}
inline format_type &operator|=(format_type &lhs, format_type rhs) {
    using T = std::underlying_type_t<format_type>;
    lhs = (format_type)((T) lhs | (T) rhs);
    return lhs;
}

constexpr bool is_type_integral(format_type type) {
    assert(type != format_type::NAMED_ARGUMENT);
    return type > format_type::NONE && type <= format_type::LAST_INTEGER_TYPE;
}

constexpr bool is_type_arithmetic(format_type type) {
    assert(type != format_type::NAMED_ARGUMENT);
    return type > format_type::NONE && type <= format_type::LAST_NUMERIC_TYPE;
}

struct format_context;
struct string_value {
    const byte *Data;
    size_t Size;
};

struct custom_value {
    const void *Data;
    void (*Format)(const void *arg, format_context &f);
};

struct named_argument_base;

struct value {
    union {
        s32 S32_Value;
        u32 U32_Value;
        s64 S64_Value;
        u64 U64_Value;
        f64 F64_Value;
        const void *Pointer_Value;
        memory_view String_Value;
        custom_value Custom_Value;
    };

    constexpr value(int value = 0) : S32_Value(value) {}
    value(unsigned value) { U32_Value = value; }
    value(long long value) { S64_Value = value; }
    value(unsigned long long value) { U64_Value = value; }
    value(f64 value) { F64_Value = value; }
    value(const byte *value) : String_Value(value, cstring_strlen(value)) {}

    value(const string_view &value) : String_Value(value.Data, value.ByteLength) {}
    value(const string &value) : String_Value(value.Data.get(), value.ByteLength) {}
    value(const void *value) { Pointer_Value = value; }

    template <typename T>
    explicit value(const T &value) {
        Custom_Value.Data = &value;
        Custom_Value.Format = &format_custom_arg<T>;
    }

   private:
    template <typename T>
    static void format_custom_arg(const void *arg, format_context &f);
};

// Value initializer used to delay conversion to value and reduce memory churn.
template <typename T, format_type Type>
struct init_value {
    static const format_type Type_Tag = Type;

    T StoredValue;

    constexpr init_value(const T &value) : StoredValue(value) {}
    constexpr operator value() const { return value(StoredValue); }
};

#define MAKE_VALUE_HELPER(TAG, ArgType, ValueType) \
    constexpr init_value<ValueType, TAG> make_value(ArgType value) { return (ValueType) value; }

#define MAKE_VALUE_SAME_HELPER(TAG, Type) \
    constexpr init_value<Type, TAG> make_value(Type value) { return value; }

MAKE_VALUE_HELPER(format_type::BOOL, bool, int)
MAKE_VALUE_HELPER(format_type::S32, short, int)
MAKE_VALUE_HELPER(format_type::U32, unsigned short, unsigned)
MAKE_VALUE_SAME_HELPER(format_type::S32, int)
MAKE_VALUE_SAME_HELPER(format_type::U32, unsigned)

// To minimize the number of types we need to deal with, long is translated
// either to int or to long long depending on its size.
using long_type = std::conditional_t<sizeof(long) == sizeof(int), int, long long>;
MAKE_VALUE_HELPER((sizeof(long) == sizeof(int) ? format_type::S32 : format_type::S64), long, long_type)
using ulong_type = std::conditional_t<sizeof(unsigned long) == sizeof(unsigned), unsigned, unsigned long long>;
MAKE_VALUE_HELPER((sizeof(unsigned long) == sizeof(unsigned) ? format_type::U32 : format_type::U64), unsigned long,
                  ulong_type)

MAKE_VALUE_SAME_HELPER(format_type::S64, long long)
MAKE_VALUE_SAME_HELPER(format_type::U64, unsigned long long)

MAKE_VALUE_HELPER(format_type::S32, signed char, int)
MAKE_VALUE_HELPER(format_type::U32, unsigned char, unsigned)

constexpr init_value<int, format_type::S32> make_value(char val) { return val; }

MAKE_VALUE_HELPER(format_type::F64, f32, f64)
MAKE_VALUE_SAME_HELPER(format_type::F64, f64)

MAKE_VALUE_HELPER(format_type::CSTRING, byte *, const byte *)
MAKE_VALUE_SAME_HELPER(format_type::CSTRING, const byte *)
MAKE_VALUE_SAME_HELPER(format_type::STRING, string_view)
MAKE_VALUE_HELPER(format_type::STRING, const string &, string_view)

MAKE_VALUE_HELPER(format_type::POINTER, void *, const void *)
MAKE_VALUE_SAME_HELPER(format_type::POINTER, const void *)

MAKE_VALUE_HELPER(format_type::POINTER, std::nullptr_t, const void *)

#undef MAKE_VALUE_HELPER
#undef MAKE_VALUE_SAME_HELPER

template <typename T>
std::enable_if_t<!std::is_same_v<T, char>> make_value(const T *) {
    static_assert(!sizeof(T), "Formatting of non-void pointers is not allowed");
}

template <typename T>
std::enable_if_t<std::is_enum_v<T> && !std::is_arithmetic_v<T> && std::is_convertible_v<T, s32>,
                 init_value<s32, format_type::S32>>
make_value(const T &value) {
    return static_cast<int>(value);
}

template <typename T>
std::enable_if_t<std::is_constructible_v<string_view, T>, init_value<string_view, format_type::STRING>> make_value(
    const T &value) {
    return string_view(value);
}

template <typename T>
std::enable_if_t<std::is_arithmetic_v<T> || (!std::is_convertible_v<T, s32> && !std::is_convertible_v<T, string_view> &&
                                             !std::is_constructible_v<string_view, T>),
                 // Implicit conversion to string is not handled here
                 init_value<const T &, format_type::CUSTOM>>
make_value(const T &value) {
    return value;
}

template <typename T>
struct get_type {
    using value_type = decltype(make_value(std::declval<std::decay_t<T> &>()));
    static const format_type Value = value_type::Type_Tag;
};
}  // namespace fmt

LSTD_END_NAMESPACE
