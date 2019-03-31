#pragma once

#include "core.hpp"
#include "specs.hpp"
#include "value.hpp"

LSTD_BEGIN_NAMESPACE

namespace fmt {

template <typename T>
constexpr std::enable_if_t<std::is_integral_v<T>, std::pair<T, bool>> parse_int(const byte *&it, s32 base = 0) {
    // Skip white space
    while (is_space(*it)) {
        ++it;
    }
    bool negative = false;
    if (*it == '-') {
        negative = true;
        ++it;
    }
    if (*it == '+') {
        ++it;
    }

    byte ch = *it++;
    if ((base == 0 || base == 16) && ch == '0' && (*it == 'x' || *it == 'X')) {
        ++it;
        ch = *it++;
        base = 16;
    }
    if (base == 0) {
        base = ch == '0' ? 8 : 10;
    }

    T maxValue;
    if constexpr (std::is_signed_v<T>) {
        maxValue = negative ? -(std::numeric_limits<T>::min()) : std::numeric_limits<T>::max();
    } else {
        maxValue = (std::numeric_limits<T>::max)();
    }
    T cutoff = maxValue / base;
    s32 cutlim = maxValue % (T) base;

    defer { --it; };

    T value = 0;
    while (true) {
        if (is_digit(ch)) {
            ch -= '0';
        } else if (is_alpha(ch)) {
            ch -= to_upper(ch) == (u32) ch ? 'A' - 10 : 'a' - 10;
        } else {
            break;
        }

        if (ch >= base) break;
        if (value > cutoff || (value == cutoff && (s32) ch > cutlim)) {
            if constexpr (std::is_unsigned_v<T>) {
                return {negative ? (0 - maxValue) : maxValue, false};
            } else {
                return {(negative ? -1 : 1) * maxValue, false};
            }
        } else {
            value = value * base + ch;
        }
        ch = *it++;
    }

    if constexpr (std::is_unsigned_v<T>) {
        return {negative ? (0 - value) : value, true};
    } else {
        return {(negative ? -1 : 1) * value, true};
    }
}

enum class parsing_error_code {
    NONE = 0,
    SPEC_NEEDS_NUMERIC_ARG, /*Format specifier requires numeric argument*/
    SPEC_NEEDS_SIGNED_ARG,  /*Format specifier requires signed argument*/
    INVALID_FORMAT_STRING,
    MISSING_PRECISION_SPEC,
    PRECISION_NOT_ALLOWED, /*Precision not allowed for this argument type*/
    INVALID_TYPE_SPEC,
    INVALID_FILL_CHAR_CURLY /*Invalid fill character '{' */
};

constexpr string_view get_message_from_parsing_error_code(parsing_error_code errorCode) {
    switch (errorCode) {
        case parsing_error_code::NONE:
            return "";  // No error
        case parsing_error_code::SPEC_NEEDS_NUMERIC_ARG:
            return "Format specifier requires numeric argument";
        case parsing_error_code::SPEC_NEEDS_SIGNED_ARG:
            return "Format specifier requires signed argument";
        case parsing_error_code::INVALID_FORMAT_STRING:
            return "Invalid format string";
        case parsing_error_code::MISSING_PRECISION_SPEC:
            return "Missing precision specifier";
        case parsing_error_code::PRECISION_NOT_ALLOWED:
            return "Precision not allowed for this argument type";
        case parsing_error_code::INVALID_TYPE_SPEC:
            return "Invalid type specifier";
        case parsing_error_code::INVALID_FILL_CHAR_CURLY:
            return "Invalid fill character \"{\"";
        default:
            assert(false && "Not handling every error");
            return "";
    }
}

namespace internal {
constexpr u32 parse_nonnegative_int(const byte *&it) {
    assert(is_digit(*it));

    u32 value = 0;
    // Convert to unsigned to prevent a warning.
    u32 maxInt = (std::numeric_limits<s32>::max)();
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

template <typename IDHandler>
constexpr parsing_error_code parse_arg_id(const byte *&it, IDHandler &&handler) {
    byte c = *it;
    if (c == '}' || c == ':') {
        handler();
        return parsing_error_code::NONE;
    }
    if (is_digit(c)) {
        assert(is_digit(*it));
        auto [index, success] = parse_int<u32>(it, 10);
        assert(success);
        if (*it != '}' && *it != ':') {
            return parsing_error_code::INVALID_FORMAT_STRING;
        }
        handler(index);
        return parsing_error_code::NONE;
    }
    if (!is_identifier_start(c)) {
        return parsing_error_code::INVALID_FORMAT_STRING;
    }
    auto start = it;
    do {
        c = *++it;
    } while (is_identifier_start(c) || is_digit(c));
    handler(string_view(start, (size_t)(it - start)));
    return parsing_error_code::NONE;
}

// IDHandler API for dynamic width.
struct width_adapter {
    dynamic_format_specs &Specs;
    parse_context &ParseContext;

    explicit constexpr width_adapter(dynamic_format_specs &specs, parse_context &parseContext)
        : Specs(specs), ParseContext(parseContext) {}

    constexpr void operator()() const { Specs.WidthRef = argument_ref(ParseContext.next_arg_id()); }
    constexpr void operator()(u32 id) const { Specs.WidthRef = argument_ref(id); }
    constexpr void operator()(const string_view &id) const { Specs.WidthRef = argument_ref(id); }
};

// IDHandler API for dynamic precision.
struct precision_adapter {
    dynamic_format_specs &Specs;
    parse_context &ParseContext;

    explicit constexpr precision_adapter(dynamic_format_specs &specs, parse_context &parseContext)
        : Specs(specs), ParseContext(parseContext) {}

    constexpr void operator()() const { Specs.PrecisionRef = argument_ref(ParseContext.next_arg_id()); }
    constexpr void operator()(u32 id) const { Specs.PrecisionRef = argument_ref(id); }
    constexpr void operator()(const string_view &id) const { Specs.PrecisionRef = argument_ref(id); }
};

// IDHandler API for arguments.
struct id_adapter {
    format_context &Context;
    argument &ArgRef;

    constexpr id_adapter(format_context &context, argument &argRef) : Context(context), ArgRef(argRef) {}

    void operator()() const { ArgRef = Context.next_arg(); }
    void operator()(u32 id) const {
        Context.ParseContext.check_arg_id(id);
        ArgRef = Context.get_arg(id);
    }
    void operator()(const string_view &id) const { ArgRef = Context.get_arg(id); }
};

struct dynamic_width_handler {
    u32 &WidthRef;

    dynamic_width_handler(u32 &widthRef) : WidthRef(widthRef) {}

    void operator()(format_context &f, s64 value) const {
        if (value >= 0) {
            WidthRef = (u32) value;
        } else {
            f.Out.append("{Unexpected negative integer with dynamic width}");
        }
    }
    void on_error(format_context &f) const { f.Out.append("{Dynamic width is not an integer}"); }
};

struct dynamic_precision_handler {
    s32 &PrecisionRef;

    dynamic_precision_handler(s32 &precisionRef) : PrecisionRef(precisionRef) {}

    void operator()(format_context &f, s64 value) const {
        if (value >= 0) {
            PrecisionRef = (s32) value;
        } else {
            f.Out.append("{Unexpected negative integer with dynamic precision}");
        }
    }
    void on_error(format_context &f) const { f.Out.append("{Dynamic precision is not an integer}"); }
};

template <typename Handler>
void handle_dynamic_field(format_context &f, const argument_ref &ref, Handler &&handler) {
    if (ref.Kind != argument_ref::kind::NONE) {
        auto arg = ref.Kind == argument_ref::kind::INDEX ? f.do_get_arg(ref.Index) : f.get_arg(ref.Name);

        s64 value = 0;
        switch (arg.Type) {
            case format_type::S32:
                value = (s64) arg.Value.S32_Value;
                break;
            case format_type::U32:
                value = (s64) arg.Value.U32_Value;
                break;
            case format_type::S64:
                value = (s64) arg.Value.S64_Value;
                break;
            case format_type::U64:
                value = (s64) arg.Value.U64_Value;
                break;
            case format_type::BOOL:
                value = (s64) arg.Value.S32_Value != 0;
                break;
            default:
                handler.on_error(f);
                return;
        }
        handler(f, value);
    }
}

}  // namespace internal

// Parses default type specs and advances parse iterator.
inline parsing_error_code parse_and_validate_specs(format_type type, format_context &f) {
    auto &it = f.ParseContext.It;
    auto &specs = f.ParseContext.Specs;

    char32_t c = *it;
    if (it == f.ParseContext.FormatString.end().to_pointer() || c == '}') {
        return parsing_error_code::NONE;
    }

    // Parse fill and alignment.
    alignment align = alignment::DEFAULT;
    s32 i = 1;
    do {
        auto p = it + i;
        switch (*p) {
            case '<':
                align = alignment::LEFT;
                break;
            case '>':
                align = alignment::RIGHT;
                break;
            case '=':
                if (!is_type_arithmetic(type)) return parsing_error_code::SPEC_NEEDS_NUMERIC_ARG;
                align = alignment::NUMERIC;
                break;
            case '^':
                align = alignment::CENTER;
                break;
        }
        if (align != alignment::DEFAULT) {
            if (p != it) {
                if (c == '{') {
                    return parsing_error_code::INVALID_FILL_CHAR_CURLY;
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
            return parsing_error_code::SPEC_NEEDS_NUMERIC_ARG;
        }
        if (is_type_integral(type) && type != format_type::S32 && type != format_type::S64) {
            return parsing_error_code::SPEC_NEEDS_SIGNED_ARG;
        }
    }
    switch (*it) {
        case '+':
            specs.Flags |= flag::SIGN | flag::PLUS;
            ++it;
            break;
        case '-':
            specs.Flags |= flag::MINUS;
            ++it;
            break;
        case ' ':
            specs.Flags |= flag::SIGN;
            ++it;
            break;
    }

    if (*it == '#') {
        if (!is_type_arithmetic(type)) return parsing_error_code::SPEC_NEEDS_NUMERIC_ARG;
        specs.Flags |= flag::HASH;
        ++it;
    }

    // Parse zero flag.
    if (*it == '0') {
        if (!is_type_arithmetic(type)) return parsing_error_code::SPEC_NEEDS_NUMERIC_ARG;
        specs.Align = alignment::NUMERIC;
        specs.Fill = '0';
        ++it;
    }

    // Parse width.
    if (is_digit(*it)) {
        specs.Width = internal::parse_nonnegative_int(it);
    } else if (*it == '{') {
        auto error = parse_arg_id(++it, internal::width_adapter(specs, f.ParseContext));
        if (error == parsing_error_code::INVALID_FORMAT_STRING || *it++ != '}') {
            return parsing_error_code::INVALID_FORMAT_STRING;
        }
    }

    // Parse precision.
    if (*it == '.') {
        ++it;
        if (is_digit(*it)) {
            specs.Precision = internal::parse_nonnegative_int(it);
        } else if (*it == '{') {
            auto error = parse_arg_id(++it, internal::precision_adapter(specs, f.ParseContext));
            if (error == parsing_error_code::INVALID_FORMAT_STRING || *it++ != '}') {
                return parsing_error_code::INVALID_FORMAT_STRING;
            }
        } else {
            return parsing_error_code::MISSING_PRECISION_SPEC;
        }
        if (is_type_integral(type) || type == format_type::POINTER) {
            return parsing_error_code::MISSING_PRECISION_SPEC;
        }
    }

    // Parse type.
    if (*it != '}' && *it) {
        specs.Type = *it++;
    }

    // Handle dynamic fields
    handle_dynamic_field(f, specs.WidthRef, internal::dynamic_width_handler(specs.Width));
    handle_dynamic_field(f, specs.PrecisionRef, internal::dynamic_precision_handler(specs.Precision));

    // Validate printf-like types
    auto typeSpec = specs.Type;
    if (!typeSpec) {
        // There is nothing to check
        return parsing_error_code::NONE;
    }

    switch (type) {
        case format_type::NONE:
        case format_type::NAMED_ARGUMENT:
            assert(false && "Invalid argument type");
            break;
        case format_type::S32:
        case format_type::U32:
        case format_type::S64:
        case format_type::U64:
        case format_type::BOOL: {
            if (typeSpec != 'd' && typeSpec != 'x' && typeSpec != 'X' && typeSpec != 'b' && typeSpec != 'B' &&
                typeSpec != 'o' && typeSpec != 'n') {
                // Allow treating integers as chars (except S64, U64 and BOOL)
                if (typeSpec != 'c') {
                    return parsing_error_code::INVALID_TYPE_SPEC;
                } else if (type == format_type::S64 || type == format_type::U64 || type == format_type::BOOL) {
                    return parsing_error_code::INVALID_TYPE_SPEC;
                }
            }
        } break;
        case format_type::F64:
            if (typeSpec != 'g' && typeSpec != 'G' && typeSpec != 'e' && typeSpec != 'E' && typeSpec != 'f' &&
                typeSpec != 'F' && typeSpec != 'a' && typeSpec != 'A') {
                return parsing_error_code::INVALID_TYPE_SPEC;
            }
            break;
        case format_type::CSTRING:
            if (typeSpec != 's' && typeSpec != 'p') {
                return parsing_error_code::INVALID_TYPE_SPEC;
            }
            break;
        case format_type::STRING:
            if (typeSpec != 's') {
                return parsing_error_code::INVALID_TYPE_SPEC;
            }
            break;
        case format_type::POINTER:
            if (typeSpec != 'p') {
                return parsing_error_code::INVALID_TYPE_SPEC;
            }
            break;
        case format_type::CUSTOM:
            // Nothing to validate!
            break;
    }

    return parsing_error_code::NONE;
}
}  // namespace fmt

LSTD_END_NAMESPACE