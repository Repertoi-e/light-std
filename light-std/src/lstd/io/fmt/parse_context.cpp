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

    auto name = string_view(It, (size_t)(it - It));
    It = it;
    return arg_ref(name);
}

dynamic_format_specs parse_context::parse_fmt_specs(type argType) {
    dynamic_format_specs result = {};
    defer({
        if (It == End || *It != '}') {
            on_error("Missing '}' in format string or unknown format specifier");
        }
    });

    if (It == End || *It == '}') return result;

    parse_align(argType, &result);
    if (It == End) return result;

    // Parse sign
    switch (*It) {
        case '+':
            require_signed_arg(argType);
            result.Flags |= flag::SIGN | flag::PLUS;
            ++It;
            break;
        case '-':
            require_signed_arg(argType);
            result.Flags |= flag::MINUS;
            ++It;
            break;
        case ' ':
            require_signed_arg(argType);
            result.Flags |= flag::SIGN;
            ++It;
            break;
    }
    if (It == End) return result;

    if (*It == '#') {
        require_numeric_arg(argType);
        result.Flags |= flag::HASH;
        if (++It == End) return result;
    }

    if (*It == '0') {
        require_numeric_arg(argType);
        result.Align = alignment::NUMERIC;
        result.Fill = '0';
        if (++It == End) return result;
    }

    parse_width(&result);
    if (It == End) return result;

    if (*It == '.') {
        parse_precision(argType, &result);
    }

    if (It != End && *It != '}') result.Type = *It++;
    return result;
}

text_style parse_context::parse_text_style() {
    text_style result;

    if (is_alpha(*It)) {
        bool terminal = false;
        if (*It == 't') {
            terminal = true;
            ++It;
        }
        if (It == End) {
            on_error("Invalid format string");
            return result;
        }
        auto *nameBegin = It;
        while (It != End && is_identifier_start(*It)) ++It;

        if (It == End) {
            on_error("Invalid format string");
            return result;
        }
        if (*It != ';' && *It != '}') {
            on_error("Invalid color name - it must be all caps and contain only letters");
            return result;
        }

        auto name = string_view(nameBegin, It - nameBegin);
        if (terminal) {
            terminal_color c = string_to_terminal_color(name);
            if (c == terminal_color::NONE) {
                // Color with that name not found, treat it as emphasis
                It -= name.ByteLength;
                handle_emphasis(&result);
                return result;
            }
            result.ColorKind = text_style::color_kind::TERMINAL;
            result.Color.Terminal = c;
        } else {
            color c = string_to_color(name);
            if (c == color::NONE) {
                // Color with that name not found, treat it as emphasis
                It -= name.ByteLength;
                handle_emphasis(&result);
                return result;
            }
            result.ColorKind = text_style::color_kind::RGB;
            result.Color.RGB = (u32) c;
        }
    } else if (is_digit(*It)) {
        // Parse an RGB true color
        u8 r = parse_rgb_channel(false);
        ++It;
        u8 g = parse_rgb_channel(false);
        ++It;
        u8 b = parse_rgb_channel(true);
        result.ColorKind = text_style::color_kind::RGB;
        result.Color.RGB = (r << 16) | (g << 8) | b;
    } else if (*It == '}') {
        // Empty text style spec means "reset"
        return result;
    }

    if (*It == ';') {
        ++It;
        if (It + 2 < End) {
            if (*It == 'B' && *(It + 1) == 'G') {
                if (result.ColorKind == text_style::color_kind::NONE) {
                    on_error("Color specified as type background but there is no color");
                    return result;
                }
                result.Background = true;
                It += 2;
                return result;
            }
        }
        handle_emphasis(&result);
    }
    return result;
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
    if (value > maxInt) on_error("Number is too big");
    return value;
}

void parse_context::parse_align(type argType, format_specs *specs) {
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
                    return;
                }
                cpSize = get_size_of_cp(It);
                char32_t fill = decode_cp(It);
                It += 1 + cpSize;
                specs->Fill = fill;
            } else {
                ++It;
            }
            specs->Align = align;
            if (align == alignment::NUMERIC) require_numeric_arg(argType);
            break;
        }
    } while (i-- > 0);
}

void parse_context::parse_width(dynamic_format_specs *specs) {
    assert(It != End);

    if (is_digit(*It)) {
        specs->Width = parse_nonnegative_int();
    } else if (*It == '{') {
        ++It;
        if (It != End) specs->WidthRef = parse_arg_id();
        if (It == End || *It != '}') {
            on_error("Invalid format string");
            return;
        }
        ++It;
    }
}

void parse_context::parse_precision(type argType, dynamic_format_specs *specs) {
    assert(It != End);

    // Skip the '.'
    ++It;

    byte c = It != End ? *It : 0;
    if (is_digit(c)) {
        specs->Precision = parse_nonnegative_int();
    } else if (c == '{') {
        ++It;
        if (It != End) specs->PrecisionRef = parse_arg_id();
        if (It == End || *It++ != '}') {
            on_error("Invalid format string");
            return;
        }
    } else {
        on_error("Missing precision specifier");
        return;
    }
    check_precision_for_arg(argType);
}

u8 parse_context::parse_rgb_channel(bool last) {
    u32 channel = parse_nonnegative_int();
    if (channel > 255) on_error("Invalid RGB channel value - it must be in the range [0-255]");
    if (!last) {
        if (*It != ';') on_error("Expected ';' after parsing first or second channel");
        if (It == End || *It == '}') {
            on_error("Invalid RGB color - expected 3 channels separated by ';'");
            return 0;
        }
    } else {
        if (*It != '}' && *It != ';') {
            on_error("Invalid RGB color - expected '}' or a ';' followed by emphasis or background specifier");
            return 0;
        }
    }
    return (u8) channel;
}

void parse_context::handle_emphasis(text_style *style) {
    // We get here either by failing to match a color name or by parsing a color first and then reaching another ';'
    while (It != End && is_alpha(*It)) {
        switch (*It) {
            case 'B':
                style->Emphasis |= emphasis::BOLD;
                break;
            case 'I':
                style->Emphasis |= emphasis::ITALIC;
                break;
            case 'U':
                style->Emphasis |= emphasis::UNDERLINE;
                break;
            case 'S':
                style->Emphasis |= emphasis::STRIKETHROUGH;
                break;
            default:
                // Note: we might have gotten here if we failed to match a color name
                on_error(
                    "Invalid emphasis character - "
                    "valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)");
                return;
        }
        ++It;
    }
}

}  // namespace fmt

LSTD_END_NAMESPACE
