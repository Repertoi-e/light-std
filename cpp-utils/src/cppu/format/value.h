#pragma once

#include "../common.h"

#include "../string/string.h"

CPPU_BEGIN_NAMESPACE

#if !defined COMPILER_MSVC
#define CLZ(n) __builtin_clz(n)
#define CLZLL(n) __builtin_clzll(n)
#endif

#if defined COMPILER_MSVC
#include <intrin.h>
CPPU_BEGIN_NAMESPACE
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
CPPU_END_NAMESPACE
#endif

namespace fmt {

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
constexpr typename std::enable_if_t<std::numeric_limits<T>::is_signed, b32> is_negative(T value) {
    return value < 0;
}

template <typename T>
constexpr typename std::enable_if_t<!std::numeric_limits<T>::is_signed, b32> is_negative(T) {
    return false;
}

// Casts nonnegative integer to unsigned.
template <typename T>
constexpr typename std::make_unsigned_t<T> to_unsigned(T value) {
    assert(value >= 0);
    return (typename std::make_unsigned_t<T>) value;
}

namespace internal {

#define POWERS_OF_10(factor)                                                                                        \
    factor * 10, factor * 100, factor * 1000, factor * 10000, factor * 100000, factor * 1000000, factor * 10000000, \
        factor * 100000000, factor * 1000000000

constexpr u32 POWERS_OF_10_32[] = {1, POWERS_OF_10(1)};
constexpr u32 ZERO_OR_POWERS_OF_10_32[] = {0, POWERS_OF_10(1)};
constexpr u64 ZERO_OR_POWERS_OF_10_64[] = {0, POWERS_OF_10(1), POWERS_OF_10(1000000000ull), 10000000000000000000ull};

// Normalized 64-bit significands of pow(10, k), for k = -348, -340, ..., 340.
constexpr u64 POW10_SIGNIFICANDS[] = {
    0xfa8fd5a0081c0288, 0xbaaee17fa23ebf76, 0x8b16fb203055ac76, 0xcf42894a5dce35ea, 0x9a6bb0aa55653b2d,
    0xe61acf033d1a45df, 0xab70fe17c79ac6ca, 0xff77b1fcbebcdc4f, 0xbe5691ef416bd60c, 0x8dd01fad907ffc3c,
    0xd3515c2831559a83, 0x9d71ac8fada6c9b5, 0xea9c227723ee8bcb, 0xaecc49914078536d, 0x823c12795db6ce57,
    0xc21094364dfb5637, 0x9096ea6f3848984f, 0xd77485cb25823ac7, 0xa086cfcd97bf97f4, 0xef340a98172aace5,
    0xb23867fb2a35b28e, 0x84c8d4dfd2c63f3b, 0xc5dd44271ad3cdba, 0x936b9fcebb25c996, 0xdbac6c247d62a584,
    0xa3ab66580d5fdaf6, 0xf3e2f893dec3f126, 0xb5b5ada8aaff80b8, 0x87625f056c7c4a8b, 0xc9bcff6034c13053,
    0x964e858c91ba2655, 0xdff9772470297ebd, 0xa6dfbd9fb8e5b88f, 0xf8a95fcf88747d94, 0xb94470938fa89bcf,
    0x8a08f0f8bf0f156b, 0xcdb02555653131b6, 0x993fe2c6d07b7fac, 0xe45c10c42a2b3b06, 0xaa242499697392d3,
    0xfd87b5f28300ca0e, 0xbce5086492111aeb, 0x8cbccc096f5088cc, 0xd1b71758e219652c, 0x9c40000000000000,
    0xe8d4a51000000000, 0xad78ebc5ac620000, 0x813f3978f8940984, 0xc097ce7bc90715b3, 0x8f7e32ce7bea5c70,
    0xd5d238a4abe98068, 0x9f4f2726179a2245, 0xed63a231d4c4fb27, 0xb0de65388cc8ada8, 0x83c7088e1aab65db,
    0xc45d1df942711d9a, 0x924d692ca61be758, 0xda01ee641a708dea, 0xa26da3999aef774a, 0xf209787bb47d6b85,
    0xb454e4a179dd1877, 0x865b86925b9bc5c2, 0xc83553c5c8965d3d, 0x952ab45cfa97a0b3, 0xde469fbd99a05fe3,
    0xa59bc234db398c25, 0xf6c69a72a3989f5c, 0xb7dcbf5354e9bece, 0x88fcf317f22241e2, 0xcc20ce9bd35c78a5,
    0x98165af37b2153df, 0xe2a0b5dc971f303a, 0xa8d9d1535ce3b396, 0xfb9b7cd9a4a7443c, 0xbb764c4ca7a44410,
    0x8bab8eefb6409c1a, 0xd01fef10a657842c, 0x9b10a4e5e9913129, 0xe7109bfba19c0c9d, 0xac2820d9623bf429,
    0x80444b5e7aa7cf85, 0xbf21e44003acdd2d, 0x8e679c2f5e44ff8f, 0xd433179d9c8cb841, 0x9e19db92b4e31ba9,
    0xeb96bf6ebadf77d9, 0xaf87023b9bf0ee6b};

// Binary exponents of pow(10, k), for k = -348, -340, ..., 340, corresponding to significands above.
constexpr s16 POW10_EXPONENTS[] = {
    -1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007, -980, -954, -927, -901, -874, -847, -821, -794, -768,
    -741,  -715,  -688,  -661,  -635,  -608,  -582,  -555,  -529,  -502, -475, -449, -422, -396, -369, -343, -316, -289,
    -263,  -236,  -210,  -183,  -157,  -130,  -103,  -77,   -50,   -24,  3,    30,   56,   83,   109,  136,  162,  189,
    216,   242,   269,   295,   322,   348,   375,   402,   428,   455,  481,  508,  534,  561,  588,  614,  641,  667,
    694,   720,   747,   774,   800,   827,   853,   880,   907,   933,  960,  986,  1013, 1039, 1066};

constexpr char RESET_COLOR[] = "\x1b[0m";

#if !defined CPPU_FMT_THOUSANDS_SEPARATOR
#if !defined CPPU_NO_CRT
inline char32_t thousands_separator() {
    std::locale loc = std::locale();
    return (char32_t) std::use_facet<std::numpunct<wchar_t>>(loc).thousands_sep();
}
#else
inline char32_t thousands_separator() { return ','; }
#endif
#else
inline char32_t thousands_separator() { return CPPU_FMT_THOUSANDS_SEPARATOR; }
#endif

#undef POWERS_OF_10

// An equivalent of `*(Dest*) (&source)` that doesn't produce
// undefined behavior (e.g. due to type aliasing).
template <typename T, typename U>
inline T bit_cast(const U &source) {
    static_assert(sizeof(T) == sizeof(U), "size mismatch");
    T result;
    CopyMemory(&result, &source, sizeof(T));
    return result;
}

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

enum class Format_Type {
    NONE = 0,
    NAMED_ARGUMENT,

    // Integers
    S32,
    U32,
    S64,
    U64,
    BOOL,
    CHAR,
    LAST_INTEGER_TYPE = CHAR,

    // Floating-point
    F64,
    LAST_NUMERIC_TYPE = F64,

    CSTRING,
    STRING,
    POINTER,
    CUSTOM
};
inline Format_Type operator|(Format_Type lhs, Format_Type rhs) {
    using T = std::underlying_type_t<Format_Type>;
    return (Format_Type)((T) lhs | (T) rhs);
}
inline Format_Type &operator|=(Format_Type &lhs, Format_Type rhs) {
    using T = std::underlying_type_t<Format_Type>;
    lhs = (Format_Type)((T) lhs | (T) rhs);
    return lhs;
}

constexpr b32 is_type_integral(Format_Type type) {
    assert(type != Format_Type::NAMED_ARGUMENT);
    return type > Format_Type::NONE && type <= Format_Type::LAST_INTEGER_TYPE;
}

constexpr b32 is_type_arithmetic(Format_Type type) {
    assert(type != Format_Type::NAMED_ARGUMENT);
    return type > Format_Type::NONE && type <= Format_Type::LAST_NUMERIC_TYPE;
}

struct Format_Context;
struct String_Value {
    const char *Data;
    size_t Size;
};

struct Custom_Value {
    const void *Data;
    void (*Format)(const void *arg, Format_Context &f);
};

struct Named_Argument_Base;

struct Value {
    union {
        s32 S32_Value;
        u32 U32_Value;
        s64 S64_Value;
        u64 U64_Value;
        f64 F64_Value;
        const void *Pointer_Value;
        String_Value String_Value;
        Custom_Value Custom_Value;
    };

    constexpr Value(s32 value = 0) : S32_Value(value) {}
    Value(u32 value) { U32_Value = value; }
    Value(s64 value) { S64_Value = value; }
    Value(u64 value) { U64_Value = value; }
    Value(f64 value) { F64_Value = value; }
    Value(const char *value) {
        String_Value.Data = value;
        String_Value.Size = cstring_strlen(value);
    }

    Value(const string_view &value) {
        String_Value.Data = value.Data;
        String_Value.Size = value.ByteLength;
    }
    Value(const void *value) { Pointer_Value = value; }

    template <typename T>
    explicit Value(const T &value) {
        Custom_Value.Data = &value;
        Custom_Value.Format = &format_custom_arg<T>;
    }

    const Named_Argument_Base &as_named_arg() { return *static_cast<const Named_Argument_Base *>(Pointer_Value); }

   private:
    template <typename T>
    static void format_custom_arg(const void *arg, Format_Context &f);
};

// Value initializer used to delay conversion to value and reduce memory churn.
template <typename T, Format_Type Type>
struct Init_Value {
    static const Format_Type Type_Tag = Type;

    T StoredValue;

    constexpr Init_Value(const T &value) : StoredValue(value) {}
    constexpr operator Value() const { return Value(StoredValue); }
};

#define MAKE_VALUE_HELPER(TAG, ArgType, ValueType) \
    constexpr Init_Value<ValueType, TAG> make_value(ArgType value) { return static_cast<ValueType>(value); }

#define MAKE_VALUE_SAME_HELPER(TAG, Type) \
    constexpr Init_Value<Type, TAG> make_value(Type value) { return value; }

MAKE_VALUE_HELPER(Format_Type::BOOL, bool, s32)
MAKE_VALUE_HELPER(Format_Type::S32, s16, s32)
MAKE_VALUE_HELPER(Format_Type::U32, u16, u32)
MAKE_VALUE_SAME_HELPER(Format_Type::S32, s32)
MAKE_VALUE_SAME_HELPER(Format_Type::U32, u32)

// To minimize the number of types we need to deal with, long is translated
// either to int or to long long depending on its size.
using long_type = std::conditional_t<sizeof(long) == sizeof(s32), s32, s64>;
MAKE_VALUE_HELPER((sizeof(long) == sizeof(s32) ? Format_Type::S32 : Format_Type::S64), long, long_type)
using ulong_type = std::conditional_t<sizeof(unsigned long) == sizeof(u32), u32, u64>;
MAKE_VALUE_HELPER((sizeof(unsigned long) == sizeof(u32) ? Format_Type::U32 : Format_Type::U64), unsigned long,
                  ulong_type)

MAKE_VALUE_SAME_HELPER(Format_Type::S64, s64)
MAKE_VALUE_SAME_HELPER(Format_Type::U64, u64)
MAKE_VALUE_HELPER(Format_Type::S32, s8, s32)
MAKE_VALUE_HELPER(Format_Type::U32, u8, u32)
MAKE_VALUE_HELPER(Format_Type::CHAR, char32_t, s32)

MAKE_VALUE_HELPER(Format_Type::F64, f32, f64)
MAKE_VALUE_SAME_HELPER(Format_Type::F64, f64)

MAKE_VALUE_HELPER(Format_Type::CSTRING, char *, const char *);
MAKE_VALUE_SAME_HELPER(Format_Type::CSTRING, const char *);
MAKE_VALUE_SAME_HELPER(Format_Type::STRING, string_view);
MAKE_VALUE_HELPER(Format_Type::STRING, const string &, string_view);

MAKE_VALUE_HELPER(Format_Type::POINTER, void *, const void *);
MAKE_VALUE_SAME_HELPER(Format_Type::POINTER, const void *)

MAKE_VALUE_HELPER(Format_Type::POINTER, std::nullptr_t, const void *)

#undef MAKE_VALUE_HELPER
#undef MAKE_VALUE_SAME_HELPER

template <typename T>
typename std::enable_if_t<!std::is_same_v<T, char>> make_value(const T *) {
    static_assert(!sizeof(T), "Formatting of non-void pointers is not allowed");
}

template <typename T>
inline typename std::enable_if_t<std::is_enum_v<T> && !std::is_arithmetic_v<T> && std::is_convertible_v<T, s32>,
                                 Init_Value<s32, Format_Type::S32>>
make_value(const T &value) {
    return static_cast<int>(value);
}

template <typename T>
inline typename std::enable_if_t<std::is_constructible_v<string_view, T>, Init_Value<string_view, Format_Type::STRING>>
make_value(const T &value) {
    return string_view(value);
}

template <typename T>
inline typename std::enable_if_t<std::is_arithmetic_v<T> || !std::is_convertible_v<T, s32> &&
                                                                !std::is_convertible_v<T, string_view> &&
                                                                !std::is_constructible_v<string_view, T>,
                                 // Implicit conversion to string is not handled here
                                 Init_Value<const T &, Format_Type::CUSTOM>>
make_value(const T &value) {
    return value;
}

template <typename T>
struct Get_Type {
    using value_type = decltype(make_value(std::declval<typename std::decay_t<T> &>()));
    static const Format_Type Value = value_type::Type_Tag;
};
}  // namespace fmt

CPPU_END_NAMESPACE
