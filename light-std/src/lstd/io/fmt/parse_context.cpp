#include "parse_context.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
void parse_context::require_numeric_arg(type argType) {
    assert(argType != type::NONE);
    if (argType == type::CUSTOM) return;
    if (!is_fmt_type_numeric(argType)) on_error("Format specifier requires numeric argument");
}

void parse_context::require_signed_arg(type argType) {
    assert(argType != type::NONE);
    if (argType == type::CUSTOM) return;

    require_numeric_arg(argType);
    if (is_fmt_type_integral(argType) && argType != type::S32 && argType != type::S64) {
        on_error("Format specifier requires a signed integer argument");
    }
}

void parse_context::check_precision_for_arg(type argType) {
    assert(argType != type::NONE);
    if (argType == type::CUSTOM) return;
    if (is_fmt_type_integral(argType) || argType == type::POINTER) {
        on_error("Precision is not allowed for this argument type");
    }
}

arg_ref parse_context::parse_arg_id() {
    assert(It != End);

    byte c = *It;
    if (c == '}' || c == ':') {
        return arg_ref(next_arg_id());
    }

    if (is_digit(c)) {
        u32 index = parse_nonnegative_int();
        if (It == End || (*It != '}' && *It != ':')) {
            on_error("Invalid format string");
            return {};
        }
        check_arg_id(index);
        return arg_ref(index);
    }
    if (!is_alpha(c) && c != '_') {
        on_error("Invalid format string");
        return {};
    }
    auto *it = It;
    do {
        ++it;
    } while (it != End && (is_alphanumeric(c = *it) || c == '_'));

    auto name = string(It, (size_t)(it - It));
    It = it;
    return arg_ref(name);
}

bool parse_context::parse_fmt_specs(type argType, dynamic_format_specs *specs) {
    if (It == End || *It == '}') return true;  // No specs to parse, but that's not an error

    if (!parse_align(argType, specs)) return false;
    if (It == End) return true;

    // Parse sign
    switch (*It) {
        case '+':
            require_signed_arg(argType);
            specs->Flags |= flag::SIGN | flag::PLUS;
            ++It;
            break;
        case '-':
            require_signed_arg(argType);
            specs->Flags |= flag::MINUS;
            ++It;
            break;
        case ' ':
            require_signed_arg(argType);
            specs->Flags |= flag::SIGN;
            ++It;
            break;
    }
    if (It == End) return true;

    if (*It == '#') {
        require_numeric_arg(argType);
        specs->Flags |= flag::HASH;
        if (++It == End) return true;
    }

    if (*It == '0') {
        require_numeric_arg(argType);
        specs->Align = alignment::NUMERIC;
        specs->Fill = '0';
        if (++It == End) return true;
    }

    if (!parse_width(specs)) return false;
    if (It == End) return true;

    if (*It == '.') {
        if (!parse_precision(argType, specs)) return false;
    }

    if (It != End && *It != '}') specs->Type = *It++;
    return true;
}

bool parse_context::parse_text_style(text_style *textStyle) {
    if (is_alpha(*It)) {
        bool terminal = false;
        if (*It == 't') {
            terminal = true;
            ++It;
        }

        auto *nameBegin = It;
        while (It != End && is_identifier_start(*It)) ++It;

        if (It == End) return true;
        if (*It != ';' && *It != '}') {
            on_error("Invalid color name - it must be a valid identifier");
            return false;
        }

        auto name = string(nameBegin, It - nameBegin);
        if (terminal) {
            terminal_color c = string_to_terminal_color(name);
            if (c == terminal_color::NONE) {
                // Color with that name not found, treat it as emphasis
                It -= name.ByteLength;
                if (!handle_emphasis(textStyle)) return false;
                return true;
            }
            textStyle->ColorKind = text_style::color_kind::TERMINAL;
            textStyle->Color.Terminal = c;
        } else {
            color c = string_to_color(name);
            if (c == color::NONE) {
                // Color with that name not found, treat it as emphasis
                It -= name.ByteLength;
                if (!handle_emphasis(textStyle)) return false;
                return true;
            }
            textStyle->ColorKind = text_style::color_kind::RGB;
            textStyle->Color.RGB = (u32) c;
        }
    } else if (is_digit(*It)) {
        // Parse an RGB true color
        u32 r = parse_rgb_channel(false);
        if (r == (u32) -1) return false;
        ++It;
        u32 g = parse_rgb_channel(false);
        if (g == (u32) -1) return false;
        ++It;
        u32 b = parse_rgb_channel(true);
        if (b == (u32) -1) return false;
        textStyle->ColorKind = text_style::color_kind::RGB;
        textStyle->Color.RGB = (r << 16) | (g << 8) | b;
    } else if (*It == '}') {
        // Empty text style spec means "reset"
        return true;
    }

    if (*It == ';') {
        ++It;
        if (It + 2 < End) {
            if (string(It, 2) == "BG") {
                assert(textStyle->ColorKind != text_style::color_kind::NONE);  // "BG" specifier encountered but there
                                                                               // was no color parsed before it
                textStyle->Background = true;
                It += 2;
                return true;
            }
        }
        if (!handle_emphasis(textStyle)) return false;
    }
    return true;
}

u32 parse_context::parse_nonnegative_int() {
    assert(It != End && '0' <= *It && *It <= '9');

    if (*It == '0') {
        ++It;
        return 0;
    }
    u32 value = 0;
    u32 maxInt = numeric_info<s32>::max();
    u32 big = maxInt / 10;
    do {
        // Check for overflow.
        if (value > big) {
            value = maxInt + 1;
            break;
        }
        value = value * 10 + u32(*It - '0');
        ++It;
    } while (It != End && '0' <= *It && *It <= '9');
    if (value > maxInt) {
        on_error("Number is too big");
        while (It != End && '0' <= *It && *It <= '9') ++It;
        return (u32) -1;
    }
    return value;
}

bool parse_context::parse_align(type argType, format_specs *specs) {
    assert(It != End);

    alignment align = alignment::DEFAULT;
    s32 i = 0;

    auto cpSize = get_size_of_cp(It);
    if (It + cpSize != End) i += cpSize;
    do {
        switch (It[i]) {
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
                if (*It == '{') {
                    on_error("Invalid fill character '{'");
                    return false;
                }
                char32_t fill = decode_cp(It);
                It += 1 + get_size_of_cp(fill);
                specs->Fill = fill;
            } else {
                ++It;
            }
            specs->Align = align;
            if (align == alignment::NUMERIC) require_numeric_arg(argType);
            break;
        }
    } while (i-- > 0);
    return true;
}

bool parse_context::parse_width(dynamic_format_specs *specs) {
    assert(It != End);

    if (is_digit(*It)) {
        specs->Width = parse_nonnegative_int();
        if (specs->Width == (u32) -1) return false;
    } else if (*It == '{') {
        ++It;
        if (It != End) specs->WidthRef = parse_arg_id();
        if (It == End || *It != '}') {
            on_error("Invalid format string");
            return false;
        }
        ++It;
    }
    return true;
}

bool parse_context::parse_precision(type argType, dynamic_format_specs *specs) {
    assert(It != End);

    // Skip the '.'
    ++It;

    byte c = It != End ? *It : 0;
    if (is_digit(c)) {
        u32 value = parse_nonnegative_int();
        if (value == (u32) -1) return false;
        specs->Precision = (s32) value;
    } else if (c == '{') {
        ++It;
        if (It != End) specs->PrecisionRef = parse_arg_id();
        if (It == End || *It++ != '}') {
            on_error("Invalid format string");
            return false;
        }
    } else {
        on_error("Missing precision specifier");
        return false;
    }
    check_precision_for_arg(argType);
    return true;
}

u32 parse_context::parse_rgb_channel(bool last) {
    u32 channel = parse_nonnegative_int();
    if (channel > 255) {
        on_error("Invalid channel value - it must be in the range [0-255]");
        return (u32) -1;
    }
    if (It == End) return (u32) -1;
    if (!last) {
        if (*It != ';') {
            on_error("';' expected");
            return (u32) -1;
        }
        if (*It == '}' || !is_digit(*(It + 1))) {
            on_error("Integer expected");
            return (u32) -1;
        }
    } else {
        if (*It != '}' && *It != ';') {
            on_error("'}' or ';' expected");
            return (u32) -1;
        }
    }
    return channel;
}

bool parse_context::handle_emphasis(text_style *textStyle) {
    // We get here either by failing to match a color name or by parsing a color first and then reaching another ';'
    while (It != End && is_alpha(*It)) {
        switch (*It) {
            case 'B':
                textStyle->Emphasis |= emphasis::BOLD;
                break;
            case 'I':
                textStyle->Emphasis |= emphasis::ITALIC;
                break;
            case 'U':
                textStyle->Emphasis |= emphasis::UNDERLINE;
                break;
            case 'S':
                textStyle->Emphasis |= emphasis::STRIKETHROUGH;
                break;
            default:
                // Note: we might have gotten here if we failed to match a color name
                on_error(
                    "Invalid emphasis character - "
                    "valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)");
                return false;
        }
        ++It;
    }
    return true;
}

}  // namespace fmt

LSTD_END_NAMESPACE
