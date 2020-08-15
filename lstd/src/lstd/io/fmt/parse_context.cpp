#include "parse_context.h"

#include "../../parse.h"
#include "../fmt.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
void parse_context::require_arithmetic_arg(type argType, s64 errorPosition) {
    assert(argType != type::NONE);
    if (argType == type::CUSTOM) return;
    if (!is_fmt_type_arithmetic(argType)) on_error("Format specifier requires an arithmetic argument", errorPosition);
}

void parse_context::require_signed_arithmetic_arg(type argType, s64 errorPosition) {
    assert(argType != type::NONE);
    if (argType == type::CUSTOM) return;

    require_arithmetic_arg(argType, errorPosition);
    if (is_fmt_type_integral(argType) && argType != type::S64) {
        on_error("Format specifier requires a signed integer argument (got unsigned)", errorPosition);
    }
}

void parse_context::check_precision_for_arg(type argType, s64 errorPosition) {
    assert(argType != type::NONE);
    if (argType == type::CUSTOM) return;
    if (is_fmt_type_integral(argType)) {
        on_error("Precision is not allowed for integer types", errorPosition);
    }
    if (argType == type::POINTER) {
        on_error("Precision is not allowed for pointer type", errorPosition);
    }
}

constexpr parse_int_options parse_int_options_fmt = parse_int_options(byte_to_digit_default, false, false, false);

s64 parse_context::parse_arg_id() {
    char ch = It[0];
    if (ch == '}' || ch == ':') {
        return next_arg_id();
    }

    if (is_digit(ch)) {
        u32 value, status;
        tie(value, status, It) = parse_int<u32, &parse_int_options_fmt>(It, 10);

        if (status == PARSE_TOO_MANY_DIGITS) {
            on_error("Argument index is an integer which is too large");
            return -1;
        }

        if (!It.Count) {
            on_error("Format string ended abruptly");
            return -1;
        }

        ch = It[0];
        if ((ch != '}' && ch != ':')) {
            on_error("Expected \":\" or \"}\"");
            return -1;
        }

        check_arg_id(value);
        return (s64) value;
    } else {
        on_error("Expected a number - an index to an argument");
        return -1;
    }
}

// Note: When parsing this if we reach the end before } we don't report an error. The caller of this should handle that.
bool parse_context::parse_fmt_specs(type argType, dynamic_format_specs *specs) {
    if (It[0] == '}') return true;  // No specs to parse

    if (!parse_fill_and_align(argType, specs)) return false;

    if (!It.Count) return true;  // No more specs to parse. Tried to parse so far: align

    // Try to parse sign
    switch (It[0]) {
        case '+':
            require_signed_arithmetic_arg(argType);

            specs->Sign = sign::PLUS;

            ++It.Data, --It.Count;
            break;
        case '-':
            require_signed_arithmetic_arg(argType);

            // MINUS has the same behaviour as NONE on the basic types but the user might want to have different
            // formating on their custom types when minus is specified, so we record it anyway.
            specs->Sign = sign::MINUS;

            ++It.Data, --It.Count;
            break;
        case ' ':
            require_signed_arithmetic_arg(argType);

            specs->Sign = sign::SPACE;

            ++It.Data, --It.Count;
            break;
    }
    if (!It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign

    if (It[0] == '#') {
        require_arithmetic_arg(argType);
        specs->Hash = true;

        ++It.Data, --It.Count;
        if (!It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign, #
    }

    // 0 means = alignment with the character 0 as fill
    if (It[0] == '0') {
        require_arithmetic_arg(argType);
        specs->Align = alignment::NUMERIC;
        specs->Fill = '0';

        ++It.Data, --It.Count;
        if (!It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign, #, 0
    }

    if (!parse_width(specs)) return false;
    if (!It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign, #, 0, width

    if (It[0] == '.') {
        if (!parse_precision(argType, specs)) return false;
    }

    // If we still haven't reached the end or a '}' we treat the byte as the type specifier.
    if (It.Count && It[0] != '}') {
        specs->Type = It[0];
        ++It.Data, --It.Count;
    }

    return true;
}

bool parse_context::parse_text_style(text_style *textStyle) {
    if (is_alpha(It[0])) {
        bool terminal = false;
        if (It[0] == 't') {
            terminal = true;
            ++It.Data, --It.Count;  // Skip the t
        }

        const char *it = It.Data;
        s64 n = It.Count;
        do {
            ++it, --n;
        } while (n && is_identifier_start(*it));

        if (!n) return true;  // The caller should check for closing }

        auto name = string(It.Data, it - It.Data);

        It = array<char>((char *) it, n);

        if (It[0] != ';' && It[0] != '}') {
            on_error("Invalid color name - it must be a valid identifier (without digits)");
            return false;
        }

        if (terminal) {
            terminal_color c = string_to_terminal_color(name);
            if (c == terminal_color::NONE) {
                // Color with that name not found, roll back and treat it as emphasis
                It.Data -= name.ByteLength, It.Count += name.ByteLength;

                if (!handle_emphasis(textStyle)) return false;
                return true;
            }
            textStyle->ColorKind = text_style::color_kind::TERMINAL;
            textStyle->Color.Terminal = c;
        } else {
            color c = string_to_color(name);
            if (c == color::NONE) {
                // Color with that name not found, roll back and treat it as emphasis
                It.Data -= name.ByteLength, It.Count += name.ByteLength;

                if (!handle_emphasis(textStyle)) return false;
                return true;
            }
            textStyle->ColorKind = text_style::color_kind::RGB;
            textStyle->Color.RGB = (u32) c;
        }
    } else if (is_digit(It[0])) {
        // Parse an RGB true color
        u32 r = parse_rgb_channel(false);
        if (r == (u32) -1) return false;
        ++It.Data, --It.Count;  // Skip the ;

        u32 g = parse_rgb_channel(false);
        if (g == (u32) -1) return false;
        ++It.Data, --It.Count;  // Skip the ;

        u32 b = parse_rgb_channel(true);
        if (b == (u32) -1) return false;
        textStyle->ColorKind = text_style::color_kind::RGB;
        textStyle->Color.RGB = (r << 16) | (g << 8) | b;
    } else if (It[0] == '#') {
        assert(false && "Parse #ffffff rgb color");
    } else if (It[0] == '}') {
        // Empty text style ({!}) spec means "reset the formatting"
        return true;
    }

    // Handle emphasis or BG, if specified
    if (It[0] == ';') {
        ++It.Data, --It.Count;  // Skip the ;
        if (It.Count > 2) {
            if (string(It.Data, 2) == "BG") {
                if (textStyle->ColorKind == text_style::color_kind::NONE) {
                    on_error("Color specified as background but there was no color parsed");
                    return false;
                }

                textStyle->Background = true;
                It.Data += 2, It.Count -= 2;
                return true;
            }
        }
        if (!handle_emphasis(textStyle)) return false;
    }
    return true;
}

file_scope alignment get_alignment_from_char(char ch) {
    if (ch == '<') {
        return alignment::LEFT;
    } else if (ch == '>') {
        return alignment::RIGHT;
    } else if (ch == '=') {
        return alignment::NUMERIC;
    } else if (ch == '^') {
        return alignment::CENTER;
    }
    return alignment::NONE;
}

bool parse_context::parse_fill_and_align(type argType, format_specs *specs) {
    auto [fill, status, rest] = eat_code_point(It);
    if (status == PARSE_INVALID) {
        on_error("Invalid UTF8 encountered in format string");
        return false;
    }

    assert(status != PARSE_EXHAUSTED);

    // First we check if the code point we parsed was an alingment specifier, if it was then there was no fill.
    // We leave it as ' ' by default and continue afterwards for error checking.
    auto align = get_alignment_from_char((char) fill);
    if (align == alignment::NONE) {
        // If there was nothing in _rest_ then it wasn't a fill code point because there is no alignment (in rest).
        // We don't parse anything and roll back.
        if (!rest.Count) return true;

        // We now check if the next char in rest is an alignment specifier.
        align = get_alignment_from_char(rest[0]);
        ++rest.Data, --rest.Count;  // Skip the align, later we advance _It_ to _rest_
    } else {
        fill = ' ';  // If we parsed an alignment but no fill then the fill must be ' ' by default
    }

    // If we got here and didn't get an alignment specifier we roll back and don't parse anything.
    if (align != alignment::NONE) {
        s64 errorPosition = rest.Data - FormatString.Data;
        if (fill == '{') {
            on_error("Invalid fill character \"{\"", errorPosition - 2);
            return false;
        }

        It = rest;  // Advance forward

        specs->Fill = fill;
        specs->Align = align;

        if (align == alignment::NUMERIC) require_arithmetic_arg(argType, errorPosition - 1);
    }
    return true;
}

bool parse_context::parse_width(dynamic_format_specs *specs) {
    if (is_digit(It[0])) {
        parse_status status;
        tie(specs->Width, status, It) = parse_int<u32, &parse_int_options_fmt>(It, 10);

        if (status == PARSE_TOO_MANY_DIGITS) {
            on_error("We parsed an integer width which was too large");
            return {};
        }
        if (specs->Width == (u32) -1) return false;
    } else if (It[0] == '{') {
        ++It.Data, --It.Count;  // Skip the }

        if (It.Count) {
            specs->WidthIndex = parse_arg_id();
            if (specs->WidthIndex == -1) return false;  // The error was reported in _parse_arg_id_
        }
        if (!It.Count || It[0] != '}') {
            on_error("Expected a closing \"}\" after parsing an argument ID for a dynamic width");
            return false;
        }

        ++It.Data, --It.Count;  // Skip the {
    }
    return true;
}

bool parse_context::parse_precision(type argType, dynamic_format_specs *specs) {
    ++It.Data, --It.Count;  // Skip the .

    if (!It.Count) {
    missing:
        on_error("Missing precision specifier (we parsed a dot but nothing valid after that)");
        return false;
    }

    if (is_digit(It[0])) {
        parse_status status;
        tie(specs->Precision, status, It) = parse_int<u32, &parse_int_options_fmt>(It, 10);

        if (status == PARSE_TOO_MANY_DIGITS) {
            on_error("We parsed an integer precision which was too large");
            return {};
        }
        if (specs->Precision == (u32) -1) return false;
    } else if (It[0] == '{') {
        ++It.Data, --It.Count;  // Skip the }

        if (It.Count) {
            specs->PrecisionIndex = parse_arg_id();
            if (specs->PrecisionIndex == -1) return false;  // The error was reported in _parse_arg_id_
        }
        if (!It.Count || It[0] != '}') {
            on_error("Expected a closing \"}\" after parsing an argument ID for a dynamic precision");
            return false;
        }

        ++It.Data, --It.Count;  // Skip the {
    } else {
        goto missing;
    }
    check_precision_for_arg(argType, It.Data - FormatString.Data - 1);
    return true;
}

constexpr parse_int_options parse_int_options_rgb_channel = parse_int_options(byte_to_digit_default, false, false, true);

u32 parse_context::parse_rgb_channel(bool last) {
    auto [channel, status, rest] = parse_int<u8, &parse_int_options_rgb_channel>(It);

    if (status == PARSE_INVALID) {
        on_error("Invalid character encountered when parsing an integer channel value", rest.Data - FormatString.Data);
        return (u32) -1;
    }

    if (status == PARSE_TOO_MANY_DIGITS) {
        on_error("Channel value too big - it must be in the range [0-255]", rest.Data - FormatString.Data - 1);
        return (u32) -1;
    }

    It = rest;
    if (status == PARSE_EXHAUSTED) return (u32) -1;

    if (!It.Count) return (u32) -1;

    if (!last) {
        if (It[0] != ';') {
            on_error("\";\" expected followed by the next channel value");
            return (u32) -1;
        }
        if (It[0] == '}' || It.Count < 2 || !is_digit(*(It.Data + 1))) {
            on_error("Expected an integer specifying a channel value (3 channels required)", It.Data - FormatString.Data + 1);
            return (u32) -1;
        }
    } else {
        if (It[0] != '}' && It[0] != ';') {
            on_error("\"}\" expected (or \";\" for BG specifier or emphasis)");
            return (u32) -1;
        }
    }
    return channel;
}

bool parse_context::handle_emphasis(text_style *textStyle) {
    // We get here either by failing to match a color name or by parsing a color first and then reaching another ';'
    while (It.Count && is_alpha(It[0])) {
        switch (It[0]) {
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
        ++It.Data, --It.Count;  // Go to the next byte
    }
    return true;
}

void parse_context::default_error_handler(const string &message, const string &formatString, s64 position) {
    // An error during formatting occured.
    // If you are running a debugger it has now hit a breakpoint.

    // Make escape characters appear as they would in a string literal
    string str = formatString;
    str.replace_all('\"', "\\\"")
        ->replace_all('\\', "\\\\")
        ->replace_all('\a', "\\a")
        ->replace_all('\b', "\\b")
        ->replace_all('\f', "\\f")
        ->replace_all('\n', "\\n")
        ->replace_all('\r', "\\r")
        ->replace_all('\t', "\\t")
        ->replace_all('\v', "\\v");

    fmt::print("\n\n>>> {!GRAY}An error during formatting occured: {!YELLOW}{}{!GRAY}\n", message);
    fmt::print("    ... the error happened here:\n");
    fmt::print("        {!}{}{!GRAY}\n", str);
    fmt::print("        {: >{}} {!} \n\n", "^", position + 1);
#if defined NDEBUG
    os_exit();
#else
    // More info has been printed to the console but here's the error message:
    auto errorMessage = message;
    assert(false);
#endif
}

}  // namespace fmt

LSTD_END_NAMESPACE
