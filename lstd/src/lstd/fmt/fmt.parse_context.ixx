module;

#include "../parse.h"

export module fmt.parse_context;

import fmt.arg;
import fmt.specs;
import fmt.text_style;

LSTD_BEGIN_NAMESPACE

export {
    //
    // In this file we define how we parse a format string - specs, colors and text style, etc.
    //

    struct fmt_parse_context {
        string FormatString;
        string It;  // How much left we have to parse from the format string

        s32 NextArgID = 0;

        fmt_parse_context(const string &formatString = "") : FormatString(formatString), It(formatString) {}  

        bool check_arg_id(u32) {
            if (NextArgID > 0) {
                on_error("Cannot switch from automatic to manual argument indexing");
                return false;
            }
            NextArgID = -1;
            return true;
        }

        u32 next_arg_id() {
            if (NextArgID >= 0) return (u32) NextArgID++;
            on_error("Cannot switch from manual to automatic argument indexing");
            return 0;
        }

        // Some specifiers require numeric arguments and we do error checking, CUSTOM arguments don't get checked
        void require_arithmetic_arg(fmt_type argType, s64 errorPosition = -1) {
            assert(argType != fmt_type::None);
            if (argType == fmt_type::Custom) return;
            if (!fmt_is_type_arithmetic(argType)) on_error("Format specifier requires an arithmetic argument", errorPosition);
        }

        // Some specifiers require signed numeric arguments and we do error checking, CUSTOM arguments don't get checked
        void require_signed_arithmetic_arg(fmt_type argType, s64 errorPosition = -1) {
            assert(argType != fmt_type::None);
            if (argType == fmt_type::Custom) return;

            require_arithmetic_arg(argType, errorPosition);
            if (fmt_is_type_integral(argType) && argType != fmt_type::S64) {
                on_error("Format specifier requires a signed integer argument (got unsigned)", errorPosition);
            }
        }

        // Integer values and pointers aren't allowed to get precision. CUSTOM argument is again, not checked.
        void check_precision_for_arg(fmt_type argType, s64 errorPosition = -1) {
            assert(argType != fmt_type::None);
            if (argType == fmt_type::Custom) return;
            if (fmt_is_type_integral(argType)) {
                on_error("Precision is not allowed for integer types", errorPosition);
            }
            if (argType == fmt_type::Pointer) {
                on_error("Precision is not allowed for pointer type", errorPosition);
            }
        }

        // The position tells where to point the caret in the format string, so it is clear where exactly the error happened.
        // If left as -1 we calculate using the current It.
        //
        // (We may want to pass a different position if we are in the middle of parsing and the It is not pointing at the right place).
        //
        // This is only used to provide useful error messages.
        inline void on_error(const string &message, s64 position = -1) {
            if (position == -1) position = It.Data - FormatString.Data;
            Context.FmtParseErrorHandler(message, FormatString, position);
        }
    };

    // Note: When parsing, if we reach the end before } or : or whatever we don't report an error.
    // The caller of this should handle that. Returns -1 if an error occured (the error is reported).
    s64 parse_arg_id(fmt_parse_context * p);

    // _argType_ is the type of the argument for which we are parsing the specs.
    // It is used for error checking, e.g, to check if it's numeric when we encounter numeric-only specs.
    //
    // Note: When parsing, if we reach the end before } we don't report an error. The caller of this should handle that.
    bool parse_fmt_specs(fmt_parse_context * p, fmt_type argType, dynamic_format_specs * specs);

    // @TODO: Return text_style with the bool in a pair
    bool parse_text_style(fmt_parse_context * p, text_style * textStyle);
}

fmt_alignment get_alignment_from_char(utf8 ch) {
    if (ch == '<') {
        return fmt_alignment::LEFT;
    } else if (ch == '>') {
        return fmt_alignment::RIGHT;
    } else if (ch == '=') {
        return fmt_alignment::NUMERIC;
    } else if (ch == '^') {
        return fmt_alignment::CENTER;
    }
    return fmt_alignment::NONE;
}

// Note: When parsing, if we reach the end before } or : or whatever we don't report an error.
// The caller of this should handle that. Returns -1 if an error occured (the error is reported).
export s64 parse_arg_id(fmt_parse_context *p) {
    utf32 ch = p->It[0];
    if (ch == '}' || ch == ':') {
        return p->next_arg_id();
    }

    if (is_digit(ch)) {
        auto [value, status, rest] = parse_int<u32, parse_int_options{.ParseSign = false}>(p->It, 10);
        p->It = string(rest);

        if (status == PARSE_TOO_MANY_DIGITS) {
            p->on_error("Argument index is an integer which is too large");
            return -1;
        }

        if (!p->It.Count) {
            p->on_error("Format string ended abruptly");
            return -1;
        }

        ch = p->It[0];
        if ((ch != '}' && ch != ':')) {
            p->on_error("Expected \":\" or \"}\"");
            return -1;
        }

        p->check_arg_id(value);
        return (s64) value;
    } else {
        p->on_error("Expected a number - an index to an argument");
        return -1;
    }
}

bool parse_fill_and_align(fmt_parse_context *p, fmt_type argType, fmt_specs *specs) {
    auto [fill, status, rest] = eat_code_point(p->It);
    if (status == PARSE_INVALID) {
        p->on_error("Invalid UTF8 encountered in format string");
        return false;
    }

    assert(status != PARSE_EXHAUSTED);

    // First we check if the code point we parsed was an alingment specifier, if it was then there was no fill.
    // We leave it as ' ' by default and continue afterwards for error checking.
    auto align = get_alignment_from_char((utf8) fill);
    if (align == fmt_alignment::NONE) {
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
    if (align != fmt_alignment::NONE) {
        s64 errorPosition = (const utf8 *) rest.Data - p->FormatString.Data;
        if (fill == '{') {
            p->on_error("Invalid fill character \"{\"", errorPosition - 2);
            return false;
        }
        if (fill == '}') {
            p->on_error("Invalid fill character \"}\"", errorPosition - 2);
            return false;
        }

        p->It = string(rest);  // Advance forward

        specs->Fill = fill;
        specs->Align = align;

        if (align == fmt_alignment::NUMERIC) p->require_arithmetic_arg(argType, errorPosition - 1);
    }
    return true;
}

bool parse_width(fmt_parse_context *p, dynamic_format_specs *specs) {
    if (is_digit(p->It[0])) {
        auto [value, status, rest] = parse_int<u32, parse_int_options{.ParseSign = false}>(p->It, 10);
        p->It = string(rest);

        specs->Width = value;

        if (status == PARSE_TOO_MANY_DIGITS) {
            p->on_error("We parsed an integer width which was too large");
            return {};
        }
        if (specs->Width == (u32) -1) return false;
    } else if (p->It[0] == '{') {
        ++p->It.Data, --p->It.Count;  // Skip the }

        if (p->It.Count) {
            specs->WidthIndex = parse_arg_id(p);
            if (specs->WidthIndex == -1) return false;  // The error was reported in _parse_arg_id_
        }
        if (!p->It.Count || p->It[0] != '}') {
            p->on_error("Expected a closing \"}\" after parsing an argument ID for a dynamic width");
            return false;
        }

        ++p->It.Data, --p->It.Count;  // Skip the {
    }
    return true;
}

bool parse_precision(fmt_parse_context *p, fmt_type argType, dynamic_format_specs *specs) {
    ++p->It.Data, --p->It.Count;  // Skip the .

    if (!p->It.Count) {
    missing:
        p->on_error("Missing precision specifier (we parsed a dot but nothing valid after that)");
        return false;
    }

    if (is_digit(p->It[0])) {
        auto [value, status, rest] = parse_int<u32, parse_int_options{.ParseSign = false}>(p->It, 10);
        p->It = string(rest);

        specs->Precision = value;

        if (status == PARSE_TOO_MANY_DIGITS) {
            p->on_error("We parsed an integer precision which was too large");
            return {};
        }
        if (specs->Precision == (u32) -1) return false;
    } else if (p->It[0] == '{') {
        ++p->It.Data, --p->It.Count;  // Skip the }

        if (p->It.Count) {
            specs->PrecisionIndex = parse_arg_id(p);
            if (specs->PrecisionIndex == -1) return false;  // The error was reported in _parse_arg_id_
        }
        if (!p->It.Count || p->It[0] != '}') {
            p->on_error("Expected a closing \"}\" after parsing an argument ID for a dynamic precision");
            return false;
        }

        ++p->It.Data, --p->It.Count;  // Skip the {
    } else {
        goto missing;
    }

    p->check_precision_for_arg(argType, p->It.Data - p->FormatString.Data - 1);
    return true;
}

// Note: When parsing this if we reach the end before } we don't report an error. The caller of this should handle that.
export bool parse_fmt_specs(fmt_parse_context *p, fmt_type argType, dynamic_format_specs *specs) {
    if (p->It[0] == '}') return true;  // No specs to parse

    if (!parse_fill_and_align(p, argType, specs)) return false;

    if (!p->It.Count) return true;  // No more specs to parse. Tried to parse so far: align

    // Try to parse sign
    switch (p->It[0]) {
        case '+':
            p->require_signed_arithmetic_arg(argType);

            specs->Sign = fmt_sign::PLUS;

            ++p->It.Data, --p->It.Count;
            break;
        case '-':
            p->require_signed_arithmetic_arg(argType);

            // MINUS has the same behaviour as NONE on the basic types but the user might want to have different
            // formating on their custom types when minus is specified, so we record it anyway.
            specs->Sign = fmt_sign::MINUS;

            ++p->It.Data, --p->It.Count;
            break;
        case ' ':
            p->require_signed_arithmetic_arg(argType);

            specs->Sign = fmt_sign::SPACE;

            ++p->It.Data, --p->It.Count;
            break;
    }
    if (!p->It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign

    if (p->It[0] == '#') {
        p->require_arithmetic_arg(argType);
        specs->Hash = true;

        ++p->It.Data, --p->It.Count;
        if (!p->It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign, #
    }

    // 0 means = alignment with the character 0 as fill
    if (p->It[0] == '0') {
        p->require_arithmetic_arg(argType);
        specs->Align = fmt_alignment::NUMERIC;
        specs->Fill = '0';

        ++p->It.Data, --p->It.Count;
        if (!p->It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign, #, 0
    }

    if (!parse_width(p, specs)) return false;
    if (!p->It.Count) return true;  // No more specs to parse. Tried to parse so far: align, sign, #, 0, width

    if (p->It[0] == '.') {
        if (!parse_precision(p, argType, specs)) return false;
    }

    // If we still haven't reached the end or a '}' we treat the byte as the type specifier.
    if (p->It.Count && p->It[0] != '}') {
        specs->Type = (utf8) p->It[0];
        ++p->It.Data, --p->It.Count;
    }

    return true;
}

bool handle_emphasis(fmt_parse_context *p, text_style *textStyle) {
    // We get here either by failing to match a color name or by parsing a color first and then reaching another ';'
    while (p->It.Count && is_alpha(p->It[0])) {
        switch (p->It[0]) {
            case 'B':
                textStyle->Emphasis |= BOLD;
                break;
            case 'I':
                textStyle->Emphasis |= ITALIC;
                break;
            case 'U':
                textStyle->Emphasis |= UNDERLINE;
                break;
            case 'S':
                textStyle->Emphasis |= STRIKETHROUGH;
                break;
            default:
                // Note: we might have gotten here if we failed to match a color name
                p->on_error("Invalid emphasis character - valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)");
                return false;
        }
        ++p->It.Data, --p->It.Count;  // Go to the next byte
    }
    return true;
}

u32 parse_rgb_channel(fmt_parse_context *p, bool last) {
    auto [channel, status, rest] = parse_int<u8, parse_int_options{.ParseSign = false, .LookForBasePrefix = true}>(p->It);

    if (status == PARSE_INVALID) {
        p->on_error("Invalid character encountered when parsing an integer channel value", (const utf8 *) rest.Data - p->FormatString.Data);
        return (u32) -1;
    }

    if (status == PARSE_TOO_MANY_DIGITS) {
        p->on_error("Channel value too big - it must be in the range [0-255]", (const utf8 *) rest.Data - p->FormatString.Data - 1);
        return (u32) -1;
    }

    p->It = string(rest);
    if (status == PARSE_EXHAUSTED) return (u32) -1;

    if (!p->It.Count) return (u32) -1;

    if (!last) {
        if (p->It[0] != ';') {
            p->on_error("\";\" expected followed by the next channel value");
            return (u32) -1;
        }
        if (p->It[0] == '}' || p->It.Count < 2 || !is_digit(*(p->It.Data + 1))) {
            p->on_error("Expected an integer specifying a channel value (3 channels required)", p->It.Data - p->FormatString.Data + 1);
            return (u32) -1;
        }
    } else {
        if (p->It[0] != '}' && p->It[0] != ';') {
            p->on_error("\"}\" expected (or \";\" for BG specifier or emphasis)");
            return (u32) -1;
        }
    }
    return channel;
}

export bool parse_text_style(fmt_parse_context *p, text_style *textStyle) {
    if (is_alpha(p->It[0])) {
        bool terminal = false;
        if (p->It[0] == 't') {
            terminal = true;
            ++p->It.Data, --p->It.Count;  // Skip the t
        }

        const utf8 *it = p->It.Data;
        s64 n = p->It.Count;
        do {
            ++it, --n;
        } while (n && is_identifier_start(*it));

        if (!n) return true;  // The caller should check for closing }

        auto name = string(p->It.Data, it - p->It.Data);

        p->It = string(it, n);

        if (p->It[0] != ';' && p->It[0] != '}') {
            p->on_error("Invalid color name - it must be a valid identifier (without digits)");
            return false;
        }

        if (terminal) {
            terminal_color c = string_to_terminal_color(name);
            if (c == terminal_color::NONE) {
                // Color with that name not found, roll back and treat it as emphasis
                p->It.Data -= name.Count, p->It.Count += name.Count;

                if (!handle_emphasis(p, textStyle)) return false;
                return true;
            }
            textStyle->ColorKind = text_style::color_kind::TERMINAL;
            textStyle->Color.Terminal = c;
        } else {
            color c = string_to_color(name);
            if (c == color::NONE) {
                // Color with that name not found, roll back and treat it as emphasis
                p->It.Data -= name.Count, p->It.Count += name.Count;

                if (!handle_emphasis(p, textStyle)) return false;
                return true;
            }
            textStyle->ColorKind = text_style::color_kind::RGB;
            textStyle->Color.RGB = (u32) c;
        }
    } else if (is_digit(p->It[0])) {
        // Parse an RGB true color
        u32 r = parse_rgb_channel(p, false);
        if (r == (u32) -1) return false;
        ++p->It.Data, --p->It.Count;  // Skip the ;

        u32 g = parse_rgb_channel(p, false);
        if (g == (u32) -1) return false;
        ++p->It.Data, --p->It.Count;  // Skip the ;

        u32 b = parse_rgb_channel(p, true);
        if (b == (u32) -1) return false;
        textStyle->ColorKind = text_style::color_kind::RGB;
        textStyle->Color.RGB = (r << 16) | (g << 8) | b;
    } else if (p->It[0] == '#') {
        assert(false && "Parse #ffffff rgb color");
    } else if (p->It[0] == '}') {
        // Empty text style ({!}) spec means "reset the formatting"
        return true;
    }

    // Handle emphasis or BG, if specified
    if (p->It[0] == ';') {
        ++p->It.Data, --p->It.Count;  // Skip the ;
        if (p->It.Count > 2) {
            if (string(p->It.Data, 2) == "BG") {
                if (textStyle->ColorKind == text_style::color_kind::NONE) {
                    p->on_error("Color specified as background but there was no color parsed");
                    return false;
                }

                textStyle->Background = true;
                p->It.Data += 2, p->It.Count -= 2;
                return true;
            }
        }
        if (!handle_emphasis(p, textStyle)) return false;
    }
    return true;
}

LSTD_END_NAMESPACE
