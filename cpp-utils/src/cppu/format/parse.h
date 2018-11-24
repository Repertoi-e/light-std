#pragma once

#include "core.h"
#include "specs.h"
#include "value.h"

CPPU_BEGIN_NAMESPACE

namespace fmt {

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

constexpr string_view get_message_from_parsing_error_code(Parsing_Error_Code errorCode) {
    switch (errorCode) {
        case Parsing_Error_Code::NONE:
            return "";  // No error
        case Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG:
            return "Format specifier requires numeric argument";
        case Parsing_Error_Code::SPEC_NEEDS_SIGNED_ARG:
            return "Format specifier requires signed argument";
        case Parsing_Error_Code::INVALID_FORMAT_STRING:
            return "Invalid format string";
        case Parsing_Error_Code::MISSING_PRECISION_SPEC:
            return "Missing precision specifier";
        case Parsing_Error_Code::PRECISION_NOT_ALLOWED:
            return "Precision not allowed for this argument type";
        case Parsing_Error_Code::INVALID_TYPE_SPEC:
            return "Invalid type specifier";
        case Parsing_Error_Code::INVALID_FORMAT_SPEC_CHAR:
            return "Invalid format specifier for char";
        case Parsing_Error_Code::INVALID_FILL_CHAR_CURLY:
            return "Invalid fill character \"{\"";
            break;
        default:
            assert(false && "Not handling every error");
            return "";
    }
}

namespace internal {
constexpr u32 parse_nonnegative_int(string_view::iterator &it) {
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
constexpr Parsing_Error_Code parse_arg_id(string_view::iterator &it, IDHandler &&handler) {
    char32_t c = *it;
    if (c == '}' || c == ':') {
        handler();
        return Parsing_Error_Code::NONE;
    }
    if (is_digit(c)) {
        u32 index = parse_nonnegative_int(it);
        if (*it != '}' && *it != ':') {
            return Parsing_Error_Code::INVALID_FORMAT_STRING;
        }
        handler(index);
        return Parsing_Error_Code::NONE;
    }
    if (!is_identifier_start(c)) {
        return Parsing_Error_Code::INVALID_FORMAT_STRING;
    }
    auto start = it;
    do {
        c = *++it;
    } while (is_identifier_start(c) || is_digit(c));
    handler(string_view(start.to_pointer(), (size_t)(it - start)));
    return Parsing_Error_Code::NONE;
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

    constexpr void operator()() { Specs.PrecisionRef = Argument_Ref(ParseContext.next_arg_id()); }
    constexpr void operator()(u32 id) { Specs.PrecisionRef = Argument_Ref(id); }
    constexpr void operator()(const string_view &id) { Specs.PrecisionRef = Argument_Ref(id); }
};

// IDHandler API for arguments.
struct ID_Adapter {
    Format_Context &Context;
    Argument &ArgRef;

    constexpr ID_Adapter(Format_Context &context, Argument &argRef) : Context(context), ArgRef(argRef) {}

    void operator()() { ArgRef = Context.next_arg(); }
    void operator()(u32 id) {
        Context.ParseContext.check_arg_id(id);
        ArgRef = Context.get_arg(id);
    }
    void operator()(const string_view &id) { ArgRef = Context.get_arg(id); }
};

struct Dynamic_Width_Handler {
    u32 &WidthRef;

    Dynamic_Width_Handler(u32 &widthRef) : WidthRef(widthRef) {}

    void operator()(Format_Context &f, s64 value) {
        if (value >= 0) {
            WidthRef = (u32) value;
        } else {
            f.Out.append("{Unexpected negative integer with dynamic width}");
        }
    }
    void on_error(Format_Context &f) { f.Out.append("{Dynamic width is not an integer}"); }
};

struct Dynamic_Precision_Handler {
    s32 &PrecisionRef;

    Dynamic_Precision_Handler(s32 &precisionRef) : PrecisionRef(precisionRef) {}

    void operator()(Format_Context &f, s64 value) {
        if (value >= 0) {
            PrecisionRef = (s32) value;
        } else {
            f.Out.append("{Unexpected negative integer with dynamic precision}");
        }
    }
    void on_error(Format_Context &f) { f.Out.append("{Dynamic precision is not an integer}"); }
};

template <typename Handler>
void handle_dynamic_field(Format_Context &f, const Argument_Ref &ref, Handler &&handler) {
    if (ref.Kind != Argument_Ref::Kind::NONE) {
        auto arg = ref.Kind == Argument_Ref::Kind::INDEX ? f.do_get_arg(ref.Index) : f.get_arg(ref.Name);

        s64 value = 0;
        switch (arg.Type) {
            case Format_Type::S32:
                value = (s64) arg.Value.S32_Value;
                break;
            case Format_Type::U32:
                value = (s64) arg.Value.U32_Value;
                break;
            case Format_Type::S64:
                value = (s64) arg.Value.S64_Value;
                break;
            case Format_Type::U64:
                value = (s64) arg.Value.U64_Value;
                break;
            case Format_Type::BOOL:
                value = (s64) arg.Value.S32_Value != 0;
                break;
            case Format_Type::CHAR:
                value = (s64)(char32_t) arg.Value.S32_Value;
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
inline Parsing_Error_Code parse_and_validate_specs(Format_Type type, Format_Context &f) {
    auto &it = f.ParseContext.It;
    auto &specs = f.ParseContext.Specs;

    char32_t c = *it;
    if (it == f.ParseContext.FormatString.end() || c == '}') {
        return Parsing_Error_Code::NONE;
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
                if (!is_type_arithmetic(type)) return Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG;
                align = Alignment::NUMERIC;
                break;
            case '^':
                align = Alignment::CENTER;
                break;
        }
        if (align != Alignment::DEFAULT) {
            if (p != it) {
                if (c == '{') {
                    return Parsing_Error_Code::INVALID_FILL_CHAR_CURLY;
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
            return Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG;
        }
        if (is_type_integral(type) && type != Format_Type::S32 && type != Format_Type::S64 &&
            type != Format_Type::CHAR) {
            return Parsing_Error_Code::SPEC_NEEDS_SIGNED_ARG;
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
        if (!is_type_arithmetic(type)) return Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG;
        specs.Flags |= Flag::HASH;
        ++it;
    }

    // Parse zero flag.
    if (*it == '0') {
        if (!is_type_arithmetic(type)) return Parsing_Error_Code::SPEC_NEEDS_NUMERIC_ARG;
        specs.Align = Alignment::NUMERIC;
        specs.Fill = '0';
        ++it;
    }

    // Parse width.
    if (is_digit(*it)) {
        specs.Width = internal::parse_nonnegative_int(it);
    } else if (*it == '{') {
        auto error = internal::parse_arg_id(++it, internal::Width_Adapter(specs, f.ParseContext));
        if (error == Parsing_Error_Code::INVALID_FORMAT_STRING || *it++ != '}') {
            return Parsing_Error_Code::INVALID_FORMAT_STRING;
        }
    }

    // Parse precision.
    if (*it == '.') {
        ++it;
        if (is_digit(*it)) {
            specs.Precision = internal::parse_nonnegative_int(it);
        } else if (*it == '{') {
            auto error = internal::parse_arg_id(++it, internal::Precision_Adapter(specs, f.ParseContext));
            if (error == Parsing_Error_Code::INVALID_FORMAT_STRING || *it++ != '}') {
                return Parsing_Error_Code::INVALID_FORMAT_STRING;
            }
        } else {
            return Parsing_Error_Code::MISSING_PRECISION_SPEC;
        }
        if (is_type_integral(type) || type == Format_Type::POINTER) {
            return Parsing_Error_Code::MISSING_PRECISION_SPEC;
        }
    }

    // Parse type.
    if (*it != '}' && *it) {
        specs.Type = *it++;
    }

    if (type == Format_Type::CHAR) {
        if (specs.Align == Alignment::NUMERIC || specs.has_flag((Flag) ~0u)) {
            return Parsing_Error_Code::INVALID_FORMAT_SPEC_CHAR;
        }
    }

    // Handle dynamic fields
    internal::handle_dynamic_field(f, specs.WidthRef, internal::Dynamic_Width_Handler(specs.Width));
    internal::handle_dynamic_field(f, specs.PrecisionRef, internal::Dynamic_Precision_Handler(specs.Precision));

    // Validate printf-like types
    auto typeSpec = specs.Type;
    if (!typeSpec) {
        // There is nothing to check
        return Parsing_Error_Code::NONE;
    }

    switch (type) {
        case Format_Type::NONE:
        case Format_Type::NAMED_ARGUMENT:
            assert(false && "Invalid argument type");
            break;
        case Format_Type::S32:
        case Format_Type::U32:
        case Format_Type::S64:
        case Format_Type::U64:
        case Format_Type::BOOL: {
            if (typeSpec != 'd' && typeSpec != 'x' && typeSpec != 'X' && typeSpec != 'b' && typeSpec != 'B' &&
                typeSpec != 'o' && typeSpec != 'n') {
                return Parsing_Error_Code::INVALID_TYPE_SPEC;
            }
        } break;
        case Format_Type::CHAR:
            if (typeSpec != 'c') {
                if (typeSpec != 'd' && typeSpec != 'x' && typeSpec != 'X' && typeSpec != 'b' && typeSpec != 'B' &&
                    typeSpec != 'o' && typeSpec != 'n') {
                    return Parsing_Error_Code::INVALID_TYPE_SPEC;
                }
            }
            break;
        case Format_Type::F64:
            if (typeSpec != 'g' && typeSpec != 'G' && typeSpec != 'e' && typeSpec != 'E' && typeSpec != 'f' &&
                typeSpec != 'F' && typeSpec != 'a' && typeSpec != 'A') {
                return Parsing_Error_Code::INVALID_TYPE_SPEC;
            }
            break;
        case Format_Type::CSTRING:
            if (typeSpec != 's' && typeSpec != 'p') {
                return Parsing_Error_Code::INVALID_TYPE_SPEC;
            }
            break;
        case Format_Type::STRING:
            if (typeSpec != 's') {
                return Parsing_Error_Code::INVALID_TYPE_SPEC;
            }
            break;
        case Format_Type::POINTER:
            if (typeSpec != 'p') {
                return Parsing_Error_Code::INVALID_TYPE_SPEC;
            }
            break;
        case Format_Type::CUSTOM:
            // Nothing to validate!
            break;
    }

    return Parsing_Error_Code::NONE;
}
}  // namespace fmt

CPPU_END_NAMESPACE