#pragma once

#include "../../intrin.h"
#include "../../storage/string_utils.h"

#include "error_handler.h"
#include "specs.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct parse_context {
    string_view FmtString;
    mutable s32 NextArgID = 0;
    size_t It = 0;
    error_handler_t ErrorHandlerFunc = default_error_handler;

    using const_iterator = string_view::const_iterator;

    constexpr parse_context(string_view fmtString, error_handler_t errorHandlerFunc)
        : FmtString(fmtString), ErrorHandlerFunc(errorHandlerFunc) {}

    constexpr const_iterator begin() const { return FmtString.begin(); }
    constexpr const_iterator end() const { return FmtString.end(); }

    constexpr void advance_to(const_iterator it) {
        auto diff = it.to_pointer() - begin().to_pointer();
        It += diff;
        FmtString.Data += diff;
        FmtString.ByteLength -= diff;
    }
    constexpr void advance_to(const byte *ptr) {
        auto diff = ptr - begin().to_pointer();
        It += diff;
        FmtString.Data += diff;
        FmtString.ByteLength -= diff;
    }

    constexpr u32 next_arg_id() const {
        if (NextArgID >= 0) return (u32) NextArgID++;
        on_error("Cannot switch from manual to automatic argument indexing");
        return 0;
    }

    constexpr bool check_arg_id(u32) const {
        if (NextArgID > 0) {
            on_error("Cannot switch from automatic to manual argument indexing");
            return false;
        }
        NextArgID = -1;
        return true;
    }

    constexpr void check_arg_id(string_view) const {}

    constexpr string_view get_original_fmt_string() const {
        return string_view(FmtString.Data - It, FmtString.ByteLength + It);
    }

    constexpr error_context get_error_context() const { return {get_original_fmt_string(), It}; }

    constexpr void on_error(const byte *message) const { ErrorHandlerFunc(message, get_error_context()); }
};

// Parses fill and alignment.
template <typename Handler>
constexpr const byte *parse_align(const byte *begin, const byte *end, Handler *handler) {
    assert(begin != end);

    alignment align = alignment::DEFAULT;
    s32 i = 0;

    auto cpSize = get_size_of_cp(begin);
    if (begin + cpSize != end) i += cpSize;
    do {
        switch (begin[i]) {
            case '<':
                align = alignment::LEFT;
                break;
            case '>':
                align = alignment::RIGHT;
                break;
            case '=':
                align = alignment::NUMERIC;
                break;
            case '^':
                align = alignment::CENTER;
                break;
        }
        if (align != alignment::DEFAULT) {
            if (i > 0) {
                if (*begin == '{') return handler->on_error("Invalid fill character '{'"), begin;
                cpSize = get_size_of_cp(begin);
                char32_t fill = decode_cp(begin);
                begin += 1 + cpSize;
                handler->on_fill(fill);
            } else {
                ++begin;
            }
            handler->on_align(align);
            break;
        }
    } while (i-- > 0);
    return begin;
}

template <typename SpecHandler>
struct width_adapter {
    SpecHandler *Handler;

    constexpr width_adapter(SpecHandler *handler) : Handler(handler) {}

    constexpr void operator()() { Handler->on_dynamic_width(unused{}); }
    constexpr void operator()(u32 id) { Handler->on_dynamic_width(id); }
    constexpr void operator()(string_view id) { Handler->on_dynamic_width(id); }

    constexpr void on_error(const byte *message) { Handler->on_error(message); }
};

template <typename SpecHandler>
struct precision_adapter {
    SpecHandler *Handler;

    constexpr precision_adapter(SpecHandler *handler) : Handler(handler) {}

    constexpr void operator()() { Handler->on_dynamic_precision(unused()); }
    constexpr void operator()(u32 id) { Handler->on_dynamic_precision(id); }
    constexpr void operator()(string_view id) { Handler->on_dynamic_precision(id); }

    constexpr void on_error(const byte *message) { Handler->on_error(message); }
};

template <typename Handler>
constexpr const byte *parse_width(const byte *begin, const byte *end, Handler *handler) {
    assert(begin != end);

    if ('0' <= *begin && *begin <= '9') {
        handler->on_width(parse_nonnegative_int(&begin, end, handler));
    } else if (*begin == '{') {
        ++begin;
        auto idHandler = width_adapter<Handler>(handler);
        if (begin != end) begin = parse_arg_id(begin, end, &idHandler);
        if (begin == end || *begin != '}') return handler->on_error("Invalid format string"), begin;
        ++begin;
    }
    return begin;
}

template <typename Handler>
constexpr const byte *parse_precision(const byte *begin, const byte *end, Handler *handler) {
    ++begin;
    auto c = begin != end ? *begin : 0;
    if (is_digit(c)) {
        handler->on_precision(parse_nonnegative_int(&begin, end, handler));
    } else if (c == '{') {
        ++begin;
        if (begin != end) {
            auto idHandler = precision_adapter<Handler>(handler);
            begin = parse_arg_id(begin, end, &idHandler);
        }
        if (begin == end || *begin++ != '}') return handler->on_error("invalid format string"), begin;
    } else {
        return handler->on_error("missing precision specifier"), begin;
    }
    handler->end_precision();
    return begin;
}

// Parses standard format specifiers and sends notifications about parsed components to handler->
template <typename SpecHandler>
constexpr const byte *parse_fmt_specs(const byte *begin, const byte *end, SpecHandler *handler,
                                      parse_context *context) {
    if (begin == end || *begin == '}') return begin;

    begin = parse_align(begin, end, handler);
    if (begin == end) return begin;

    // Parse sign
    switch (*begin) {
        case '+':
            handler->on_plus();
            ++begin;
            break;
        case '-':
            handler->on_minus();
            ++begin;
            break;
        case ' ':
            handler->on_space();
            ++begin;
            break;
    }
    if (begin == end) return begin;

    if (*begin == '#') {
        handler->on_hash();
        if (++begin == end) return begin;
    }

    if (*begin == '0') {
        handler->on_zero();
        if (++begin == end) return begin;
    }

    begin = parse_width(begin, end, handler);
    if (begin == end) return begin;

    if (*begin == '.') {
        begin = parse_precision(begin, end, handler);
    }

    if (begin != end && *begin != '}') handler->on_type(*begin++);

    return begin;
}

namespace internal {

// A format specifier handler that sets fields in basic_format_specs.
struct specs_setter {
    format_specs *Specs;

    constexpr specs_setter(format_specs *specs) : Specs(specs) {}

    constexpr void on_align(alignment align) { Specs->Align = align; }
    constexpr void on_fill(char32_t fill) { Specs->Fill = fill; }
    constexpr void on_plus() { Specs->Flags |= flag::SIGN | flag::PLUS; }
    constexpr void on_minus() { Specs->Flags |= flag::MINUS; }
    constexpr void on_space() { Specs->Flags |= flag::SIGN; }
    constexpr void on_hash() { Specs->Flags |= flag::HASH; }

    constexpr void on_zero() {
        Specs->Align = alignment::NUMERIC;
        Specs->Fill = '0';
    }

    constexpr void on_width(u32 width) { Specs->Width = width; }
    constexpr void on_precision(u32 precision) { Specs->Precision = (s32) precision; }
    constexpr void end_precision() {}

    constexpr void on_text_style(text_style style) { Specs->TextStyle = style; }

    constexpr void on_type(byte type) { Specs->Type = type; }
};

// Format spec handler that saves references to dynamic width and precision arguments.
struct dynamic_specs_handler : public specs_setter {
    parse_context *Context;
    dynamic_format_specs *Specs;

    dynamic_specs_handler(parse_context *context, dynamic_format_specs *specs)
        : specs_setter(specs), Context(context), Specs(specs) {}

    template <typename Id>
    constexpr void on_dynamic_width(Id argID) {
        Specs->WidthRef = make_arg_ref(argID);
    }

    template <typename Id>
    constexpr void on_dynamic_precision(Id argID) {
        Specs->PrecisionRef = make_arg_ref(argID);
    }

    constexpr void on_error(const byte *message) { Context->on_error(message); }

   private:
    constexpr arg_ref make_arg_ref(u32 argID) {
        Context->check_arg_id(argID);
        return arg_ref(argID);
    }

    constexpr arg_ref make_arg_ref(unused) { return arg_ref(Context->next_arg_id()); }

    constexpr arg_ref make_arg_ref(string_view argID) {
        Context->check_arg_id(argID);
        auto view =
            string_view(Context->begin().to_pointer(), Context->end().to_pointer() - Context->begin().to_pointer());
        return arg_ref(string_view_metadata(view, argID));
    }
};

struct numeric_specs_checker {
    error_handler_t ErrorHandlerFunc;
    error_context ErrorContext;
    type ArgType;

    constexpr numeric_specs_checker(error_handler_t errorHandlerFunc, error_context errorContext, type argType)
        : ErrorHandlerFunc(errorHandlerFunc), ErrorContext(errorContext), ArgType(argType) {}

    constexpr void require_numeric_argument() {
        if (ArgType == type::CUSTOM) return;
        if (!is_fmt_type_numeric(ArgType)) ErrorHandlerFunc("Format specifier requires numeric argument", ErrorContext);
    }

    constexpr void check_sign() {
        if (ArgType == type::CUSTOM) return;
        require_numeric_argument();
        if (is_fmt_type_integral(ArgType) && ArgType != type::S32 && ArgType != type::S64) {
            ErrorHandlerFunc("Format specifier requires signed argument", ErrorContext);
        }
    }

    constexpr void check_precision() {
        if (ArgType == type::CUSTOM) return;
        if (is_fmt_type_integral(ArgType) || ArgType == type::POINTER) {
            ErrorHandlerFunc("Precision not allowed for this argument type", ErrorContext);
        }
    }
};

// A format specifier handler that checks if specifiers are consistent with the argument type.
// @TODO: Redesign this and remove need for function overrides
template <typename Handler>
struct specs_checker : public Handler {
    numeric_specs_checker Checker;

    constexpr specs_checker(Handler handler, error_handler_t errorHandlerFunc, error_context errorContext, type argType)
        : Handler(handler), Checker(errorHandlerFunc, errorContext, argType) {}

    constexpr void on_align(alignment align) {
        if (align == alignment::NUMERIC) Checker.require_numeric_argument();
        Handler::on_align(align);
    }

    constexpr void on_plus() {
        Checker.check_sign();
        Handler::on_plus();
    }

    constexpr void on_minus() {
        Checker.check_sign();
        Handler::on_minus();
    }

    constexpr void on_space() {
        Checker.check_sign();
        Handler::on_space();
    }

    constexpr void on_hash() {
        Checker.require_numeric_argument();
        Handler::on_hash();
    }

    constexpr void on_zero() {
        Checker.require_numeric_argument();
        Handler::on_zero();
    }

    constexpr void on_text_style(text_style style) { Handler::on_text_style(style); }

    constexpr void end_precision() { Checker.check_precision(); }
};
}  // namespace internal

template <typename T>
constexpr const byte *parse_fmt_specs(parse_context *context, dynamic_format_specs *specs) {
    auto type = type_constant_v<T>;

    internal::specs_checker handler(internal::dynamic_specs_handler(context, specs), context->ErrorHandlerFunc,
                                    context->get_error_context(), type);
    auto it = parse_fmt_specs(context->begin().to_pointer(), context->end().to_pointer(), &handler, context);

    switch (type) {
        case type::NONE:
        case type::NAMED_ARG:
            assert(false && "Invalid argument type");
            break;
        case type::S32:
        case type::U32:
        case type::S64:
        case type::U64:
        case type::BOOL: {
            auto handler = internal::int_type_checker{context->ErrorHandlerFunc, context->get_error_context()};
            internal::handle_int_type_spec(specs->Type, &handler);
            if (specs->Type && specs->Type == 'c') {
                if (specs->Align == alignment::NUMERIC || (u32) specs->Flags != 0) {
                    context->on_error("Invalid format specifier for char");
                }
            }
        } break;
        case type::F64: {
            auto handler = internal::float_type_checker{context->ErrorHandlerFunc, context->get_error_context()};
            internal::handle_float_type_spec(specs->Type, &handler);
        } break;
        case type::CSTRING: {
            auto handler = internal::cstring_type_checker{context->ErrorHandlerFunc, context->get_error_context()};
            internal::handle_cstring_type_spec(specs->Type, &handler);
        } break;
        case type::STRING:
            if (specs->Type && specs->Type != 's') context->on_error("Invalid type specifier");
            break;
        case type::POINTER:
            if (specs->Type && specs->Type != 'p') context->on_error("Invalid type specifier");
            break;
        case type::CUSTOM:
            break;
    }
    return it;
}

// Parses the range [begin, end) as an unsigned integer. This function assumes
// that the range is non-empty and the first character is a digit.
template <typename Handler>
constexpr u32 parse_nonnegative_int(const byte **begin, const byte *end, Handler *handler) {
    assert(*begin != end && '0' <= **begin && **begin <= '9');

    if (**begin == '0') {
        ++(*begin);
        return 0;
    }
    u32 value = 0;
    // Convert to unsigned to prevent a warning.
    u32 maxInt = numeric_info<s32>::max();
    u32 big = maxInt / 10;
    do {
        // Check for overflow.
        if (value > big) {
            value = maxInt + 1;
            break;
        }
        value = value * 10 + u32(**begin - '0');
        ++(*begin);
    } while (*begin != end && '0' <= **begin && **begin <= '9');
    if (value > maxInt) handler->on_error("Number is too big");
    return value;
}

template <typename IDHandler>
constexpr const byte *parse_arg_id(const byte *begin, const byte *end, IDHandler *handler) {
    assert(begin != end);

    byte c = *begin;
    if (c == '}' || c == ':') {
        (*handler)();
        return begin;
    }

    if (c >= '0' && c <= '9') {
        u32 index = parse_nonnegative_int(&begin, end, handler);
        if (begin == end || (*begin != '}' && *begin != ':')) {
            handler->on_error("Invalid format string");
            return begin;
        }
        (*handler)(index);
        return begin;
    }
    if (!is_alpha(c) && c != '_') {
        handler->on_error("invalid format string");
        return begin;
    }
    auto it = begin;
    do {
        ++it;
    } while (it != end && (is_alphanumeric(c = *it) || c == '_'));
    (*handler)(string_view(begin, (size_t)(it - begin)));
    return it;
}

namespace internal {
template <typename Handler>
constexpr u8 parse_rgb_channel(const byte **begin, const byte *end, Handler *handler, bool last) {
    u32 channel = parse_nonnegative_int(begin, end, handler);
    if (channel > 255) handler->on_error("Invalid RGB channel value - it must be in the range [0-255]");
    if (!last) {
        if (**begin != ';') handler->on_error("Expected ';' after parsing first or second channel");
        if (*begin == end || **begin == '}') {
            handler->on_error("Invalid RGB color - expected 3 channels separated by ';'");
        }
    } else {
        if (**begin != '}' && **begin != ';') {
            handler->on_error("Invalid RGB color - expected '}' or a ';' followed by emphasis or background specifier");
        }
    }
    return (u8) channel;
}
}  // namespace internal

template <typename Handler>
constexpr text_style parse_text_style(const byte **begin, const byte *end, Handler *handler) {
    text_style result;

    if (is_alpha(**begin)) {
        bool terminal = false;
        if (**begin == 't') {
            terminal = true;
            ++*begin;
        }

        auto *nameBegin = *begin;
        while (*begin != end && is_identifier_start(**begin)) ++*begin;

        if (**begin != ';' && **begin != '}') {
            handler->on_error("Invalid color name - it must be all caps and contain only letters");
        }

        auto name = string_view(nameBegin, *begin - nameBegin);
        if (terminal) {
            terminal_color c = string_to_terminal_color(name);
            if (c == terminal_color::NONE) {
                // Color with that name not found, treat it as emphasis
                *begin -= name.ByteLength;
                goto handle_emphasis;
            }
            result.ColorKind = text_style::color_kind::TERMINAL;
            result.Color.Terminal = c;
        } else {
            color c = string_to_color(name);
            if (c == color::NONE) {
                // Color with that name not found, treat it as emphasis
                *begin -= name.ByteLength;
                goto handle_emphasis;
            }
            result.ColorKind = text_style::color_kind::RGB;
            result.Color.RGB = (u32) c;
        }
    } else if (is_digit(**begin)) {
        // Parse an RGB true color
        u8 r = internal::parse_rgb_channel(begin, end, handler, false);
        ++*begin;
        u8 g = internal::parse_rgb_channel(begin, end, handler, false);
        ++*begin;
        u8 b = internal::parse_rgb_channel(begin, end, handler, true);
        result.ColorKind = text_style::color_kind::RGB;
        result.Color.RGB = (r << 16) | (g << 8) | b;
    } else if (**begin == '}') {
        // Empty text style spec means "reset"
        return result;
    }

    if (**begin == ';') {
        ++*begin;
        if (*begin + 2 < end) {
            if (**begin == 'B' && *(*begin + 1) == 'G') {
                if (result.ColorKind == text_style::color_kind::NONE) {
                    handler->on_error("Color specified as background but there is no color");
                }
                result.Background = true;
                *begin += 2;
                return result;
            }
        }
    handle_emphasis:
        // We get here either by failing to match a color name or by parsing a color first and then reaching another ';'
        while (*begin != end && is_alpha(**begin)) {
            switch (**begin) {
                case 'B':
                    result.Emphasis |= emphasis::BOLD;
                    break;
                case 'I':
                    result.Emphasis |= emphasis::ITALIC;
                    break;
                case 'U':
                    result.Emphasis |= emphasis::UNDERLINE;
                    break;
                case 'S':
                    result.Emphasis |= emphasis::STRIKETHROUGH;
                    break;
                default:
                    // Note, we might have gotten here if we failed to match a color name
                    handler->on_error(
                        "Invalid emphasis char - "
                        "valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)");
            }
            ++*begin;
        }
    }
    return result;
}
}  // namespace fmt

LSTD_END_NAMESPACE
