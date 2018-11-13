#pragma once

#include "../memory/temporary_allocator.h"

#include "core.h"

#if !defined COMPILER_MSVC
#define CLZ(n) __builtin_clz(n)
#define CLZLL(n) __builtin_clzll(n)
#endif

#if defined COMPILER_MSVC
#include <intrin.h>
GU_BEGIN_NAMESPACE
namespace fmt::internal {
#pragma intrinsic(_BitScanReverse)

inline u32 clz(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}
#define CLZ(n) internal::clz(n)

#if defined PROCESSOR_X64
#pragma intrinsic(_BitScanReverse64)
#endif

inline u32 clzll(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
#ifdef PROCESSOR_X64
    _BitScanReverse64(&r, x);
#else
    // Scan the high 32 bits.
    if (_BitScanReverse(&r, (u32)(x >> 32))) return 63 - (r + 32);
    // Scan the low 32 bits.
    _BitScanReverse(&r, (u32) x);
#endif
    return 63 - r;
}
#define CLZLL(n) internal::clzll(n)
}  // namespace fmt::internal
GU_END_NAMESPACE
#endif

namespace fmt::internal {
// An equivalent of `*(Dest*) (&source)` that doesn't produce
// undefined behavior (e.g. due to type aliasing).
template <typename T, typename U>
inline T bit_cast(const U &source) {
    static_assert(sizeof(T) == sizeof(U), "size mismatch");
    T result;
    CopyMemory(&result, &source, sizeof(T));
    return result;
}

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
constexpr typename std::enable_if_t<std::numeric_limits<T>::is_signed, bool> is_negative(T value) {
    return value < 0;
}

template <typename T>
constexpr typename std::enable_if_t<!std::numeric_limits<T>::is_signed, bool> is_negative(T) {
    return false;
}

// Casts nonnegative integer to unsigned.
template <typename T>
constexpr typename std::make_unsigned_t<T> to_unsigned(T value) {
    assert(value >= 0);
    return (typename std::make_unsigned_t<T>) value;
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

}  // namespace fmt::internal

namespace fmt {

enum class Alignment { DEFAULT, LEFT, RIGHT, CENTER, NUMERIC };
enum class Flag : u32 { SIGN = 1, PLUS = 2, MINUS = 4, HASH = 8 };

inline Flag operator|(Flag lhs, Flag rhs) {
    using T = std::underlying_type_t<Flag>;
    return (Flag)((T) lhs | (T) rhs);
}
inline Flag &operator|=(Flag &lhs, Flag rhs) {
    using T = std::underlying_type_t<Flag>;
    lhs = (Flag)((T) lhs | (T) rhs);
    return lhs;
}

struct Align_Spec {
    u32 Width;
    char32_t Fill;
    Alignment Align;

    constexpr Align_Spec(u32 width, char32_t fill, Alignment align = Alignment::DEFAULT)
        : Width(width), Fill(fill), Align(align) {}
};

class Format_Specs : public Align_Spec {
   public:
    Flag Flags = (Flag) 0;
    s32 Precision = -1;
    char32_t Type;

    constexpr Format_Specs(u32 width = 0, char32_t type = 0, char32_t fill = ' ')
        : Align_Spec(width, fill), Type(type) {}
    constexpr bool has_flag(Flag flag) const { return ((u32) Flags & (u32) flag) != 0; }
};

struct Auto_ID {};

struct Argument_Ref {
    enum class Kind { NONE, INDEX, NAME };
    Kind Kind = Kind::NONE;
    union {
        u32 Index;
        string_view Name;
    };

    constexpr Argument_Ref() : Index(0) {}
    constexpr explicit Argument_Ref(u32 index) : Kind(Kind::INDEX), Index(index) {}
    constexpr explicit Argument_Ref(const string_view &name) : Kind(Kind::NAME), Name(name) {}

    constexpr Argument_Ref &operator=(u32 index) {
        Kind = Kind::INDEX;
        Index = index;
        return *this;
    }
};

struct Dynamic_Format_Specs : Format_Specs {
    Argument_Ref WidthRef;
    Argument_Ref PrecisionRef;
};

// Specifies whether to format T using the standard Formatter.
// It is not possible to use Get_Type in Formatter specialization directly
// because of a bug in MSVC.
template <typename T>
struct Is_Format_Type : std::integral_constant<bool, Get_Type<T>::value != Format_Type::CUSTOM> {};

namespace internal {
constexpr u32 parse_nonnegative_int(string_view::Iterator &it) {
    assert(is_digit(*it));

    u32 value = 0;
    // Convert to unsigned to prevent a warning.
    u32 maxInt = (std::numeric_limits<int>::max)();
    u32 big = maxInt / 10;
    do {
        // Check for overflow.
        if (value > big) {
            value = maxInt + 1;
            break;
        }
        value = value * 10 + u32(*it - '0');
        ++it;
    } while (is_digit(*it));

    // Make sure number is not too big
    assert(value <= maxInt);
    return value;
}

// IDHandler API for dynamic width.
struct Width_Adapter {
    Dynamic_Format_Specs &Specs;
    Parse_Context &ParseContext;

    explicit constexpr Width_Adapter(Dynamic_Format_Specs &specs, Parse_Context &parseContext)
        : Specs(specs), ParseContext(parseContext) {}

    constexpr void operator()() { Specs.WidthRef = Argument_Ref(ParseContext.next_arg_id()); }
    constexpr void operator()(u32 id) { Specs.WidthRef = Argument_Ref(id); }
    constexpr void operator()(const string_view &id) { Specs.WidthRef = Argument_Ref(id); }
};

// IDHandler API for dynamic precision.
struct Precision_Adapter {
    Dynamic_Format_Specs &Specs;
    Parse_Context &ParseContext;

    explicit constexpr Precision_Adapter(Dynamic_Format_Specs &specs, Parse_Context &parseContext)
        : Specs(specs), ParseContext(parseContext) {}

    constexpr void operator()() { Specs.WidthRef = Argument_Ref(ParseContext.next_arg_id()); }
    constexpr void operator()(u32 id) { Specs.PrecisionRef = Argument_Ref(id); }
    constexpr void operator()(const string_view &id) { Specs.PrecisionRef = Argument_Ref(id); }
};

struct ID_Adapter {
    Format_Context &Context;
    Format_Argument &ArgRef;

    constexpr ID_Adapter(Format_Context &context, Format_Argument &argRef) : Context(context), ArgRef(argRef) {}

    void operator()() { ArgRef = Context.next_arg(); }
    void operator()(u32 id) {
        Context.ParseContext.check_arg_id(id);
        ArgRef = Context.get_arg(id);
    }
    void operator()(const string_view &id) { ArgRef = Context.get_arg(id); }
};

enum class Parsing_Error_Code {
    NONE = 0,
    SPEC_NEEDS_NUMERIC_ARG, /*Format specifier requires numeric argument*/
    SPEC_NEEDS_SIGNED_ARG,  /*Format specifier requires signed argument*/
    INVALID_FORMAT_STRING,
    MISSING_PRECISION_SPEC,
    PRECISION_NOT_ALLOWED, /*Precision not allowed for this argument type*/
    INVALID_TYPE_SPEC,
    INVALID_FORMAT_SPEC_CHAR, /*Invalid format specifier for char*/
    INVALID_FILL_CHAR_CURLY   /*Invalid fill character '{' */
};

template <typename IDHandler>
constexpr std::pair<string_view::Iterator, Parsing_Error_Code> parse_arg_id(string_view::Iterator it,
                                                                            IDHandler &&handler) {
    char32_t c = *it;
    if (c == '}' || c == ':') {
        handler();
        return {it, Parsing_Error_Code::NONE};
    }
    if (is_digit(c)) {
        u32 index = parse_nonnegative_int(it);
        if (*it != '}' && *it != ':') {
            return {it, Parsing_Error_Code::INVALID_FORMAT_STRING};
        }
        handler(index);
        return {it, Parsing_Error_Code::NONE};
    }
    if (!is_identifier_start(c)) {
        return {it, Parsing_Error_Code::INVALID_FORMAT_STRING};
    }
    auto start = it;
    do {
        c = *++it;
    } while (is_identifier_start(c) || is_digit(c));
    handler(string_view(it.to_pointer(), (size_t)(it - start)));
    return {it, Parsing_Error_Code::NONE};
}

// TODO: Remove the iterator return.
inline std::pair<string_view::Iterator, Parsing_Error_Code> parse_and_validate_specifiers(Format_Type type,
                                                                                   Parse_Context &parseContext,
                                                                                   Dynamic_Format_Specs &specs) {
	auto it = parseContext.It;

    char32_t c = *it;
    if (!it.valid() || c == '}') {
        return {it, Parsing_Error_Code::NONE};
    }
    // Parse fill and alignment.
    Alignment align = Alignment::DEFAULT;
    s32 i = 1;
    do {
        auto p = it + i;
        switch (*p) {
            case '<':
                align = Alignment::LEFT;
                break;
            case '>':
                align = Alignment::RIGHT;
                break;
            case '=':
                if (!is_type_arithmetic(type)) return {it, Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG};
                align = Alignment::NUMERIC;
                break;
            case '^':
                align = Alignment::CENTER;
                break;
        }
        if (align != Alignment::DEFAULT) {
            if (p != it) {
                if (c == '{') {
                    return {it, Parsing_Error_Code::INVALID_FILL_CHAR_CURLY};
                }
                it += 2;
                specs.Fill = c;
            } else {
                ++it;
            }
            specs.Align = align;
            break;
        }
    } while (--i >= 0);

    // Parse sign.
    if (*it == '+' || *it == '-' || *it == ' ') {
        if (!is_type_arithmetic(type)) {
            return {it, Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG};
        }
        if (is_type_integral(type) && type != Format_Type::S32 && type != Format_Type::S64 &&
            type != Format_Type::CHAR) {
            return {it, Parsing_Error_Code::SPEC_NEEDS_SIGNED_ARG};
        }
    }
    switch (*it) {
        case '+':
            specs.Flags |= Flag::SIGN | Flag::PLUS;
            ++it;
            break;
        case '-':
            specs.Flags |= Flag::MINUS;
            ++it;
            break;
        case ' ':
            specs.Flags |= Flag::SIGN;
            ++it;
            break;
    }

    if (*it == '#') {
        if (!is_type_arithmetic(type)) return {it, Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG};
        specs.Flags |= Flag::HASH;
        ++it;
    }

    // Parse zero flag.
    if (*it == '0') {
        if (!is_type_arithmetic(type)) return {it, Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG};
        specs.Align = Alignment::NUMERIC;
        specs.Fill = '0';
        ++it;
    }

    // Parse width.
    if (is_digit(*it)) {
        specs.Width = parse_nonnegative_int(it);
    } else if (*it == '{') {
        auto [end_it, error] = parse_arg_id(it + 1, Width_Adapter(specs, parseContext));
        it = end_it;
        if (error == Parsing_Error_Code::INVALID_FORMAT_STRING || *it++ != '}') {
            return {it, Parsing_Error_Code::INVALID_FORMAT_STRING};
        }
    }

    // Parse precision.
    if (*it == '.') {
        ++it;
        if (is_digit(*it)) {
            specs.Precision = parse_nonnegative_int(it);
        } else if (*it == '{') {
            auto [end_it, error] = parse_arg_id(it + 1, Precision_Adapter(specs, parseContext));
            it = end_it;
            if (error == Parsing_Error_Code::INVALID_FORMAT_STRING || *it++ != '}') {
                return {it, Parsing_Error_Code::INVALID_FORMAT_STRING};
            }
        } else {
            return {it, Parsing_Error_Code::MISSING_PRECISION_SPEC};
        }
        if (is_type_integral(type) || type == Format_Type::POINTER) {
            return {it, Parsing_Error_Code::MISSING_PRECISION_SPEC};
        }
    }

    // Parse type.
    if (*it != '}' && *it) {
        specs.Type = *it++;
    }

    // These two types are different. The first one is the actual type that
    // this Formatter is parsing, while the second is the char that the user
    // puts in the format string (e.g. {d}, {f}, {} (no char))
    auto typeSpec = specs.Type;
    if (!typeSpec) {
        // There is nothing to check
        return {it, Parsing_Error_Code::NONE};
    }

    switch (type) {
        case Format_Type::NONE:
        case Format_Type::NAMED_ARGUMENT:
            // Error: Invalid argument type
            assert(false);
            break;
        case Format_Type::S32:
        case Format_Type::U32:
        case Format_Type::S64:
        case Format_Type::U64:
        case Format_Type::BOOL: {
            if (typeSpec != 'd' && typeSpec != 'x' && typeSpec != 'X' && typeSpec != 'b' && typeSpec != 'B' &&
                typeSpec != 'o' && typeSpec != 'n') {
                return {it, Parsing_Error_Code::INVALID_TYPE_SPEC};
            }
        } break;
        case Format_Type::CHAR:
            if (typeSpec != 'c') {
                if (typeSpec != 'd' && typeSpec != 'x' && typeSpec != 'X' && typeSpec != 'b' && typeSpec != 'B' &&
                    typeSpec != 'o' && typeSpec != 'n') {
                    return {it, Parsing_Error_Code::INVALID_TYPE_SPEC};
                }
            }
            if (specs.Align == Alignment::NUMERIC || specs.has_flag((Flag) ~0u)) {
                return {it, Parsing_Error_Code::INVALID_FORMAT_SPEC_CHAR};
            }
            break;
        case Format_Type::F64:
            if (typeSpec != 'g' && typeSpec != 'G' && typeSpec != 'e' && typeSpec != 'E' && typeSpec != 'f' &&
                typeSpec != 'F' && typeSpec != 'a' && typeSpec != 'A') {
                return {it, Parsing_Error_Code::INVALID_TYPE_SPEC};
            }
            break;
        case Format_Type::CSTRING:
            if (typeSpec != 's' && typeSpec != 'p') {
                return {it, Parsing_Error_Code::INVALID_TYPE_SPEC};
            }
            break;
        case Format_Type::STRING:
            if (typeSpec != 's') {
                return {it, Parsing_Error_Code::INVALID_TYPE_SPEC};
            }
            break;
        case Format_Type::POINTER:
            if (typeSpec != 'p') {
                return {it, Parsing_Error_Code::INVALID_TYPE_SPEC};
            }
            break;
        case Format_Type::CUSTOM:
            // Custom format specifiers do not belong in this Formatter specialization
            assert(false);
            break;
    }

    return {it, Parsing_Error_Code::NONE};
}

// A functor that doesn't add a thousands separator.
struct No_Thousands_Separator {
    void operator()(char *) {}
};

// A functor that adds a thousands separator.
struct Add_Thousands_Separator {
    string_view Separator;

    // Index of a decimal digit with the least significant digit having index 0.
    u32 DigitIndex = 0;

    explicit Add_Thousands_Separator(const string_view &separator) : Separator(separator) {}

    void operator()(char *&buffer) {
        if (++DigitIndex % 3 != 0) return;
        // TODO: "Danger danger, but who cares"
        buffer -= Separator.BytesUsed;
        CopyMemory(buffer, Separator.Data, Separator.BytesUsed);
    }
};

// Formats a decimal unsigned integer value writing into buffer.
template <typename UInt, typename TS = No_Thousands_Separator>
char *format_uint_to_buffer(char *buffer, UInt value, u32 numDigits, TS thousandsSep = {}) {
    buffer += numDigits;
    char *end = buffer;
    while (value >= 100) {
        // Integer division is slow so do it for a group of two digits instead
        // of for every digit. The idea comes from the talk by Alexandrescu
        // "Three Optimization Tips for C++". See speed-test for a comparison.
        u32 index = (u32)((value % 100) * 2);
        value /= 100;
        *--buffer = DIGITS[index + 1];
        thousandsSep(buffer);
        *--buffer = DIGITS[index];
        thousandsSep(buffer);
    }
    if (value < 10) {
        *--buffer = (char) ('0' + value);
        return end;
    }
    u32 index = (u32)(value * 2);
    *--buffer = DIGITS[index + 1];
    thousandsSep(buffer);
    *--buffer = DIGITS[index];
    return end;
}

template <typename UInt, typename TS = No_Thousands_Separator>
void format_uint_to_builder(String_Builder &builder, UInt value, u32 numDigits, TS thousandsSep = {}) {
    // Buffer should be large enough to hold all digits (digits10 + 1) and null.
    char buffer[std::numeric_limits<UInt>::digits10 + 2];
    format_uint_to_buffer(buffer, value, numDigits, thousandsSep);
    builder.append_pointer_and_size(buffer, numDigits);
}

template <u32 BASE_BITS, typename UInt>
char *format_uint_to_buffer(char *buffer, UInt value, u32 numDigits, bool upper = false) {
    buffer += numDigits;
    char *end = buffer;
    do {
        const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        u32 digit = (value & ((1 << BASE_BITS) - 1));
        *--buffer = BASE_BITS < 4 ? (char) ('0' + digit) : digits[digit];
    } while ((value >>= BASE_BITS) != 0);
    return end;
}

template <unsigned BASE_BITS, typename UInt>
void format_uint_to_builder(String_Builder &builder, UInt value, u32 numDigits, bool upper = false) {
    // Buffer should be large enough to hold all digits (digits / BASE_BITS + 1) and null.
    char buffer[std::numeric_limits<UInt>::digits / BASE_BITS + 2];
    format_uint_to_buffer<BASE_BITS>(buffer, value, numDigits, upper);
    builder.append_pointer_and_size(buffer, numDigits);
}

}  // namespace internal

template <typename F>
void format_padded_to_builder(String_Builder &builder, size_t length, const Align_Spec &spec, F &&func) {
    if (spec.Width <= length) {
        func(builder);
        return;
    }

    size_t padding = spec.Width - length;
    if (spec.Align == Alignment::RIGHT) {
        for (size_t i = 0; i < padding; i++) builder.append(spec.Fill);
        func(builder);
    } else if (spec.Align == Alignment::CENTER) {
        size_t leftPadding = padding / 2;
        for (size_t i = 0; i < leftPadding; i++) builder.append(spec.Fill);
        func(builder);
        for (size_t i = 0; i < padding - leftPadding; i++) builder.append(spec.Fill);
    } else {
        func(builder);
        for (size_t i = 0; i < padding; i++) builder.append(spec.Fill);
    }
}

inline void format_string_to_builder(String_Builder &builder, string_view view, const Format_Specs &specs) {
    size_t precision = (size_t) specs.Precision;
    if (specs.Precision >= 0 && precision < view.Length) {
        view.remove_suffix(view.Length - precision);
    }
    format_padded_to_builder(builder, view.Length, specs, [&](String_Builder &out) { out.append(view); });
}

// Writes an integer in the format
//   <left-padding><prefix><numeric-padding><digits><right-padding>
// where <digits> are written by f(it).
template <typename F>
void format_int_to_builder(String_Builder &builder, u32 numDigits, string_view prefix, const Format_Specs &specs,
                           F &&f) {
    size_t size = prefix.Length + numDigits;
    char32_t fill = specs.Fill;
    size_t padding = 0;
    if (specs.Align == Alignment::NUMERIC) {
        if (specs.Width > size) {
            padding = specs.Width - size;
            size = specs.Width;
        }
    } else if (specs.Precision > (s32) numDigits) {
        size = prefix.Length + (size_t) specs.Precision;
        padding = (size_t) specs.Precision - numDigits;
        fill = '0';
    }
    Align_Spec as = specs;
    if (specs.Align == Alignment::DEFAULT) as.Align = Alignment::RIGHT;
    format_padded_to_builder(builder, size, as, [&](String_Builder &out) {
        if (prefix.Length) {
            out.append(prefix);
        }
        for (size_t i = 0; i < padding; i++) out.append(fill);
        f(out);
    });
}

template <typename T>
void format_int_to_builder(String_Builder &builder, T value, const Format_Specs &specs) {
    char prefix[4] = {0};
    size_t prefixSize = 0;

    using unsigned_type = typename std::make_unsigned_t<T>;
    unsigned_type absValue = (unsigned_type) value;
    if (internal::is_negative(value)) {
        prefix[0] = '-';
        ++prefixSize;
        absValue = 0 - absValue;
    } else if (specs.has_flag(Flag::SIGN)) {
        prefix[0] = specs.has_flag(Flag::PLUS) ? '+' : ' ';
        ++prefixSize;
    }

    switch (specs.Type) {
        case 0:
        case 'd': {
            u32 numDigits = internal::count_digits(absValue);
            format_int_to_builder(builder, numDigits, string_view(prefix, prefixSize), specs, [&](String_Builder &out) {
                internal::format_uint_to_builder(out, absValue, numDigits);
            });
        } break;
        case 'x':
        case 'X': {
            if (specs.has_flag(Flag::HASH)) {
                prefix[prefixSize++] = '0';
                prefix[prefixSize++] = (char) specs.Type;
            }
            u32 numDigits = internal::count_digits<4>(absValue);
            format_int_to_builder(builder, numDigits, string_view(prefix, prefixSize), specs, [&](String_Builder &out) {
                internal::format_uint_to_builder<4>(out, absValue, numDigits, specs.Type != 'x');
            });
        } break;
        case 'b':
        case 'B': {
            if (specs.has_flag(Flag::HASH)) {
                prefix[prefixSize++] = '0';
                prefix[prefixSize++] = (char) specs.Type;
            }
            u32 numDigits = internal::count_digits<1>(absValue);
            format_int_to_builder(builder, numDigits, string_view(prefix, prefixSize), specs, [&](String_Builder &out) {
                internal::format_uint_to_builder<1>(out, absValue, numDigits);
            });
        } break;
        case 'o': {
            u32 numDigits = internal::count_digits<3>(absValue);
            if (specs.has_flag(Flag::HASH) && specs.Precision <= (s32) numDigits) {
                // Octal prefix '0' is counted as a digit, so only add it if precision
                // is not greater than the number of digits.
                prefix[prefixSize++] = '0';
            }
            format_int_to_builder(builder, numDigits, string_view(prefix, prefixSize), specs, [&](String_Builder &out) {
                internal::format_uint_to_builder<3>(out, absValue, numDigits);
            });
        } break;
        case 'n': {
            u32 numDigits = internal::count_digits(absValue);
            char32_t sep = internal::thousands_separator(null);
            char sepEncoded[4];
            encode_code_point(sepEncoded, sep);
            string_view sepView(sepEncoded, get_size_of_code_point(sepEncoded));

            u32 size = numDigits + 1 * ((numDigits - 1) / 3);
            format_int_to_builder(builder, numDigits, string_view(prefix, prefixSize), specs, [&](String_Builder &out) {
                internal::format_uint_to_builder(out, absValue, numDigits, internal::Add_Thousands_Separator{sepView});
            });
        } break;
        default:
            // Shouldn't ever get here, since the specs have been checked in the parse stage.
            assert(false);
    }
}

void report_spec_parsing_error(String_Builder &out, internal::Parsing_Error_Code errorCode);

// Specialize Formatter for non-custom types
template <typename T>
struct Formatter<T, typename std::enable_if_t<Is_Format_Type<T>::value>> {
    Dynamic_Format_Specs Specs;
    internal::Parsing_Error_Code Error = internal::Parsing_Error_Code::NONE;

    // Parses format specifiers stopping either at the end of the range or at the
    // terminating '}'.
    constexpr string_view::Iterator parse(const Parse_Context &parseContext) {
        auto [it, error] = internal::parse_and_validate_specifiers(Get_Type<T>::value, parseContext, Specs);
        if (error != internal::Parsing_Error_Code::NONE) {
            Error = error;
            return it;
        }
    }

    void format(const T &value, Format_Context &f) {
        if (Error != internal::Parsing_Error_Code::NONE) {
            report_spec_parsing_error(f.Out, Error);
            return;
        }

        bool failedMiserably = false;

        // If the arguments for width or precision were not constant but instead another argument,
        // check them and set Width/Precision fields in Specs.
        auto handle = [&](auto &var, auto &ref) {
            if (ref.Kind != Argument_Ref::Kind::NONE) {
                auto arg = ref.Kind == Argument_Ref::Kind::INDEX ? f.get_arg(ref.Index) : f.get_arg(ref.Name);

                s64 value = 0;
                switch (arg._Type) {
                    case Format_Type::S32:
                        value = (s64) arg._Value.S32_Value;
                        break;
                    case Format_Type::U32:
                        value = (s64) arg._Value.U32_Value;
                        break;
                    case Format_Type::S64:
                        value = (s64) arg._Value.S64_Value;
                        break;
                    case Format_Type::U64:
                        value = (s64) arg._Value.U64_Value;
                        break;
                    case Format_Type::BOOL:
                        value = (s64) arg._Value.S32_Value != 0;
                        break;
                    case Format_Type::CHAR:
                        value = (s64)(char32_t) arg._Value.S32_Value;
                        break;
                    default:
                        f.Out.append("{Dynamic width/precision type is not an integer}");
                        failedMiserably = true;
                }
                // Error: Negative value for width/precision
                assert(value >= 0);
                var = (u32) value;
            }
        };
        if (failedMiserably) return;

        handle(Specs.Width, Specs.WidthRef);
        handle(Specs.Precision, Specs.PrecisionRef);

        Format_Argument arg = make_argument(value);
        format_argument(f, arg, Specs);
    }
};

inline void report_spec_parsing_error(String_Builder &out, internal::Parsing_Error_Code errorCode) {
    switch (errorCode) {
        case internal::Parsing_Error_Code::NONE:
            // No error
            break;
        case internal::Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG:
            out.append("{Format specifier requires numeric argument}");
            break;
        case internal::Parsing_Error_Code::SPEC_NEEDS_SIGNED_ARG: /**/
            out.append("{Format specifier requires signed argument}");
            break;
        case internal::Parsing_Error_Code::INVALID_FORMAT_STRING:
            out.append("{Invalid format string}");
            break;
        case internal::Parsing_Error_Code::MISSING_PRECISION_SPEC:
            out.append("{Missing precision specifier}");
            break;
        case internal::Parsing_Error_Code::PRECISION_NOT_ALLOWED:
            out.append("{Precision not allowed for this argument type}");
            break;
        case internal::Parsing_Error_Code::INVALID_TYPE_SPEC:
            out.append("{Invalid type specifier}");
            break;
        case internal::Parsing_Error_Code::INVALID_FORMAT_SPEC_CHAR:
            out.append("{Invalid format specifier for char}");
            break;
        case internal::Parsing_Error_Code::INVALID_FILL_CHAR_CURLY:
            out.append("{Invalid fill character \"{\"}");
            break;
        default:
            // Oops! Not handling every error!
            assert(false);
    }
}

inline void format_argument(Format_Context &f, const Format_Argument &arg, const Format_Specs &specs) {
    switch (arg._Type) {
        case Format_Type::S32:
            format_int_to_builder(f.Out, arg._Value.S32_Value, specs);
            break;
        case Format_Type::U32:
            format_int_to_builder(f.Out, arg._Value.U32_Value, specs);
            break;
        case Format_Type::S64:
            format_int_to_builder(f.Out, arg._Value.S64_Value, specs);
            break;
        case Format_Type::U64:
            format_int_to_builder(f.Out, arg._Value.U64_Value, specs);
            break;
        case Format_Type::BOOL: {
            if (specs.Type) {
                format_int_to_builder(f.Out, arg._Value.S32_Value ? 1 : 0, specs);
            } else {
                string_view view(arg._Value.S32_Value ? "true" : "false");
                format_string_to_builder(f.Out, view, specs);
            }
        } break;
        case Format_Type::CHAR:
            if (specs.Type && specs.Type != 'c') {
                // Int
                format_int_to_builder(f.Out, arg._Value.S32_Value, specs);
            } else {
                format_padded_to_builder(f.Out, 1, specs,
                                         [&](String_Builder &out) { out.append(arg._Value.S32_Value); });
            }
            break;
        case Format_Type::F64:
            // TODO: Floating point numbers!!!
            // TODO: Floating point numbers!!!
            // TODO: Floating point numbers!!!
            assert(false);
            break;
        case Format_Type::CSTRING:
            if (!specs.Type || specs.Type == 's') {
                auto strValue = arg._Value.String_Value;
                if (!strValue.Data) {
                    f.Out.append("{String pointer is null}");
                    return;
                }

                string_view view(strValue.Data, strValue.Size);
                format_string_to_builder(f.Out, view, specs);
            } else if (specs.Type == 'p') {
                Format_Specs hashSpecs = specs;
                hashSpecs.Flags = Flag::HASH;
                hashSpecs.Type = 'x';
                format_int_to_builder(f.Out, (uptr_t) arg._Value.Pointer_Value, hashSpecs);
            }
            break;
        case Format_Type::STRING: {
            auto strValue = arg._Value.String_Value;
            if (!strValue.Data) {
                f.Out.append("{String pointer is null}");
                return;
            }

            string_view view(strValue.Data, strValue.Size);
            format_string_to_builder(f.Out, view, specs);
        } break;
        case Format_Type::POINTER: {
            Format_Specs hashSpecs = specs;
            hashSpecs.Flags = Flag::HASH;
            hashSpecs.Type = 'x';
            format_int_to_builder(f.Out, (uptr_t) arg._Value.Pointer_Value, hashSpecs);
        } break;
        case Format_Type::CUSTOM: {
            auto handle = typename Format_Argument::Handle(arg._Value.Custom_Value);
            handle.format(f);
        } break;
        default:
            // Error: Invalid argument type
            assert(false);
    }
}

namespace internal {
inline void helper_write(String_Builder &builder, string_view::Iterator begin, const string_view::Iterator &end) {
    if (begin == end) return;
    while (true) {
        size_t curly = string_view(begin.to_pointer(), end - begin).find('}');
        if (curly == npos) {
            builder.append_pointer_and_size(begin.to_pointer(), end - begin);
            return;
        }
        auto p = begin + curly + 1;
        if (p == end || *p != '}') {
            // Error: unmatched } in format string
            assert(false);
            return;
        }
        builder.append_pointer_and_size(begin.to_pointer(), p - begin);
        begin = p + 1;
    }
}
}  // namespace internal

template <typename... Args>
string sprint(const string_view &formatString, Args &&... args) {
    fmt::Format_Arguments_Store<Args...> store = {args...};
    auto args = fmt::Format_Arguments(store);

    fmt::Format_Context context(formatString, args);
    fmt::Format_Argument arg;

    auto &it = context.ParseContext.It;
    auto end = context.ParseContext.FormatString.end();
    while (it != end) {
        size_t curly = string_view(it.to_pointer(), end - it).find('{');
        if (*it != '{' && curly == npos) {
            internal::helper_write(context.Out, it, end);
            return to_string(context.Out);
        }
        auto p = it + curly;
        internal::helper_write(context.Out, it, p);
        ++p;
        if (p == end) {
            // Error: Invalid format string
            assert(false);
            return to_string(context.Out);
        }

        if (*p == '}') {
            arg = context.next_arg();

            context.ParseContext.advance_to(p);
            if (arg._Type == fmt::Format_Type::CUSTOM) {
                auto handle = typename fmt::Format_Argument::Handle(arg._Value.Custom_Value);
                handle.format(context);
            } else {
                fmt::format_argument(context, arg, {});
            }
        } else if (*p == '{') {
            internal::helper_write(context.Out, p, p + 1);
        } else {
            context.ParseContext.advance_to(p);

            if (arg._Type == fmt::Format_Type::CUSTOM) {
                auto handle = typename fmt::Format_Argument::Handle(arg._Value.Custom_Value);
                handle.format(context);
            } else {
                auto [end_it, error] = fmt::internal::parse_arg_id(p, fmt::internal::ID_Adapter(context, arg));
                p = end_it;
                if (error != fmt::internal::Parsing_Error_Code::NONE) {
                    context.Out.append("{Invalid format string}");
                    return to_string(context.Out);
                }
                char32_t c = p != end ? *p : 0;
                if (c == '}') {
                    context.ParseContext.advance_to(p);
                    if (arg._Type == fmt::Format_Type::CUSTOM) {
                        auto handle = typename fmt::Format_Argument::Handle(arg._Value.Custom_Value);
                        handle.format(context);
                    } else {
                        fmt::format_argument(context, arg, {});
                    }
                } else if (c == ':') {
                    p++;
                    context.ParseContext.advance_to(p);
                    if (arg._Type == fmt::Format_Type::CUSTOM) {
                        auto handle = typename fmt::Format_Argument::Handle(arg._Value.Custom_Value);
                        handle.format(context);
                    } else {
                        fmt::Dynamic_Format_Specs specs;
                        auto [end_it, error] =
                            fmt::internal::parse_and_validate_specifiers(arg._Type, context.ParseContext, specs);
                        p = end_it;
                        if (error != fmt::internal::Parsing_Error_Code::NONE) {
                            fmt::report_spec_parsing_error(context.Out, error);
                            return to_string(context.Out);
                        }
                        if (*p == '}') {
                            fmt::format_argument(context, arg, specs);
                        }
                    }
                    if (*p != '}') {
                        context.Out.append("{Unknown format specifier}");
						return to_string(context.Out);
                    }
                    context.ParseContext.advance_to(p);
                } else {
                    context.Out.append("{Missing \"}\" in format string}");
					return to_string(context.Out);
                }
            }
        }
        it = p + 1;
    }

    return to_string(context.Out);
}

template <typename... Args>
void print(const string_view &formatString, Args &&... args) {
    // TODO: A way to optimize this would be by directly outputting text to
    // the console instead of sprinting it to a buffer first and then outputting it.
    print_string_to_console(sprint(formatString, std::forward<Args>(args)...));
}

// Format a string using the temporary allocator
template <typename... Args>
inline string tprint(const string &formatString, Args &&... args) {
    assert(__temporary_allocator_data);

    Allocator_Closure oldAllocator;
    oldAllocator = __context.Allocator;
    __context.Allocator = {__temporary_allocator, __temporary_allocator_data};

    auto result = sprint(formatString, std::forward<Args>(args)...);

    __context.Allocator = oldAllocator;

    return result;
}

template <typename T>
inline string to_string(const T& value) {
	// TODO: Speed...
	return sprint("{}", value);
}

}  // namespace fmt
