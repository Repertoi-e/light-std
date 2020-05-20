#pragma once

#include "../internal/context.h"
#include "../io.h"
#include "../math.h"
#include "../memory/array.h"
#include "../memory/guid.h"
#include "fmt/format_context.h"

//
// Format specification:
//
// The formatting engine in this library is similar to how python handles advanced string formatting.
//
// Format strings consist of characters and fields encoded in utf-8.
// Fields define how an argument gets formatted to the output while the rest of the characters get transfered unchanged.
//
// Fields are defined with curly braces, like so:
//    fmt::print("This is an {}", "example") -> "This is an example"
// Braces can be escaped by doubling:
//    fmt::print("Hey there :-{{}}")         -> "Hey there :-{}"
//
// You can specify which argument a field refers to by index or by name.
//    fmt::print("{0} {1} {0}", "first", "second") -> "first second first"
//   Note: If you leave the braces without an index, by default it is automatically incremented
//         with each new field. However you may not switch between automatic and manual indexing
//         throughout the formatting string.
//
//    You can also use named arguments:
//    fmt::print("Hello {name}! You are {seconds} seconds late.", fmt::named("name", "Jon"), fmt::named("seconds", 10))
//            -> "Hello Jon! You are 5 seconds late."
//
//    The argument must be passed through fmt::named and with the name you want to use, otherwise an error is reported:
//    fmt::print("{name}", "Jon") -> doesn't work
//
// Format specifiers follow the name, with ":" separating the two:
//    fmt::print("{:<8}", "Jon")  -> "     Jon"
//
// The general form of a standard format specifier is:
//     [[fill]align][sign][#][0][width][.precision][type]
// The brackets ([]) indicate an optional element.
//
//     The optional align flag is one of the following:
//       '<' - Forces the field to be left-aligned within the available space (default)
//       '>' - Forces the field to be right-aligned within the available space.
//       '=' - Forces the padding to be placed after the sign (if any)
//             but before the digits.  This is used for printing fields
//             in the form '+000000120'. This alignment option is only
//             valid for numeric types.
//       '^' - Forces the field to be centered within the available space
//
//     Note that unless a minimum field width is defined, the field width will always be the same size as the data to
//     fill it, so that the alignment option has no meaning in this case.
//
//     The optional 'fill' code point defines the code point to be used to pad the field to the minimum width.
//     The fill code point, if present, must be followed by an alignment flag.
//     The fill element can be multiple bytes and must be encoded in utf-8.
//
//     The 'sign' option is only valid for numeric types, and can be one of the following:
//       '+'  - Indicates that a sign should be used for both positive as well as negative numbers
//       '-'  - Indicates that a sign should be used only for negative numbers (default)
//       ' '  - Indicates that a leading space should be used on positive numbers
//
//     If the '#' character is present, integers use the 'alternate form' for formatting.
//     This means that binary, octal, and hexadecimal output will be prefixed with '0b', '0o', and '0x', respectively.
//
//     'width' is a decimal integer defining the minimum field width.
//     If not specified, then the field width will be determined by the content.
//
//     If the width field is preceded by a zero('0') character, this enables zero - padding.
//     This is equivalent to an alignment type of '=' and a fill character of '0'.
//
//     The 'precision' is a decimal number indicating how many digits should be displayed after the decimal point in a
//     floating point conversion. For non-numeric types the field indicates the maximum field size - in other words, how
//     many characters will be used from the field content. The precision is ignored for integer conversions.
//
//     Finally, the 'type' byte determines how the data should be presented:
//
//      Integers:
//       'b' - Binary. Outputs the number in base 2.
//       'c' - Character. Converts the integer to the corresponding Unicode character before printing.
//       'd' - Decimal Integer. Outputs the number in base 10.
//       'o' - Octal format. Outputs the number in base 8.
//       'x' - Hex format. Outputs the number in base 16, using lower-case letters for the digits above 9.
//       'X' - Hex format. Outputs the number in base 16, using upper-case letters for the digits above 9.
//       'n' - Number. This is the same as 'd', except that it inserts thousands separator
//             (currently a dot that doesn't get determined by the locale)
//       '' (None) - the same as 'd'
//
//      Floats:
//       'e' - Exponent notation. Prints the number in scientific notation using 'e' for the exponent.
//       'E' - Uppercase version of 'e'.
//       'f' - Fixed point. Displays the number as a fixed-point number.
//       'F' - Uppercase version of 'f'.
//       'g' - General format. This prints the number as a fixed-point number, unless the number is too large,
//             in which case it switches to 'e' exponent notation.
//       'G' - Uppercase version of 'g'.
//       '%' - Percentage. Multiplies the number by 100 and displays in fixed ('f') format, followed by a percent sign.
//       '' (None) - similar to 'g', except that it prints at least one
//             digit after the decimal point.
//
//      Pointers:
//       'p' - Outputs a 'const void *' formatted in hexadecimal. Example: 0xab5c8fea84
//       '' (None) - the same as 'p'
//
//      C-Style string:
//       'p' - Treats the argument as a pointer.
//       's' - Outputs it as an utf-8 encoded string.
//       '' (None) - the same as 's'
//
//      Strings (string, string_view, etc.):
//       's' - Outputs it as an utf-8 encoded string.
//       '' (None) - the same as 's'
//
//      Guid:
//       'n' - 00000000000000000000000000000000
//       'N' - Uppercase version of 'n'
//       'd' - 00000000-0000-0000-0000-000000000000
//       'D' - Uppercase version of 'd'
//       'b' - {00000000-0000-0000-0000-000000000000}
//       'B' - Uppercase version of 'b'
//       'p' - (00000000-0000-0000-0000-000000000000)
//       'P' - Uppercase version of 'p'
//       'x' - {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
//       'X' - Uppercase version of 'x'
//       '' (None) - the same as 'd'
//
// There is also a way to specify text styles directly in the format string. Without requiring an argument.
// Text styles are defined by a opening curly brace ('{') followed by '!', then the text style and a closing brace ('}')
// An empty text style resets any foreground/background color and text emphasis.
//    fmt::print("{!}")
//
// There are 3 ways to define a text color. The color is optional but it must be the first thing after '!'.
//    1) Using the name of a color, e.g. {!CORNFLOWER_BLUE}
//       A full list of recognized colors is available in "lstd/io/fmt/colors.def"
//       and programatically in the fmt::color enum
//	        fmt::print("{!DARK_MAGENTA}")
//    2) Using the name of a "terminal" color. Use these colors if the console you are printing to doesn't support
//       true color. These are the most basic colors supported by almost any console.
//           BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
//           and the bright versions: BRIGHT_BLACK, BRIGHT_RED, ...
//       In order to distinguish between 24-bit color and 4-bit, a leading 't' is required after the '!'
//	        fmt::print("{!tBRIGHT_CYAN}")
//    3) Using RGB. This gives you full control and ability to specify any 24-bit color.
//       Channels are parsed in the order red - green - blue and must be separated by ';'
//	        fmt::print("{!50;230;170}")
//       Values must be in the range [0-255]
//
// After the text color you can mark is as background using "BG":
//	        fmt::print("{!WHITE;BG}")
// That means the color applies to the background and not the foreground.
//
// If you didn't mark the color as background, you can specify a series of
// characters that define the emphasis style of the text.
//	        fmt::print("{!WHITE;BIUS}")
//    Here "BIUS" specifies all the types of emphasis:
//      B (bold), I (italic, rarely supported by consoles tho), U (underline) and S (strikethrough)
//    They can be in any order and are optional.
//    e.g. valid emphasis strings are: "BI", "U", "B", "SU", "SB", etc...
//       Note: When parsing, if we fail to find the name of a color, e.g. {!IMAGINARYCOLOR}, we treat
//             the series of characters as emphasis, although any character encountered that is not a
//             valid emphasis gets reported as an error.
//             This allows specifying emphasis without color: fmt::print("{!BU}");

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Defined in fmt.cpp
void parse_fmt_string(string fmtString, format_context *f);

// Formats to writer
template <typename... Args>
void to_writer(io::writer *out, string fmtString, Args &&... args) {
    
    args_store<remove_reference_t<Args>...> store;  // This needs to outlive _parse_fmt_string_
    store.populate(args...);

    auto bakedArgs = fmt::args(store);
    auto f = format_context(out, fmtString, bakedArgs, default_error_handler);
    parse_fmt_string(fmtString, &f);
    f.flush();
}

// Formats to a counting writer and returns the result
template <typename... Args>
size_t calculate_formatted_size(string fmtString, Args &&... args) {
    io::counting_writer writer;
    to_writer(&writer, fmtString, ((Args &&) args)...);
    return writer.Count;
}

// Formats to a string
template <typename... Args>
void sprint(string *out, string fmtString, Args &&... args) {
    out->reserve(out->ByteLength +
                 calculate_formatted_size(fmtString, ((Args &&) args)...));  // @Speed Is this actually better?

    auto writer = io::string_builder_writer();
    to_writer(&writer, fmtString, ((Args &&) args)...);
    writer.Builder.combine(out);
}

// Formats to io::cout
template <typename... Args>
void print(string fmtString, Args &&... args) {
    to_writer(Context.Log, fmtString, ((Args &&) args)...);
}

// Formatters for array, stack_array and thread::id

template <typename T>
struct formatter<array<T>> {
    void format(array<T> src, format_context *f) { f->debug_list().entries(src.Data, src.Count)->finish(); }
};

template <typename T, size_t N>
struct formatter<stack_array<T, N>> {
    void format(stack_array<T, N> src, format_context *f) { f->debug_list().entries(src.Data, src.Count)->finish(); }
};

template <>
struct formatter<thread::id> {
    void format(thread::id src, format_context *f) { f->write(src.Value); }
};

// Formatters for math types

template <typename T, s32 Dim, bool Packed>
struct formatter<vec<T, Dim, Packed>> {
    void format(const vec<T, Dim, Packed> &src, format_context *f) {
        f->debug_list().entries(src.Data, src.Dim)->finish();
    }
};

// Prints in format: [ 1, 2, 3; 4, 5, 6; 7, 8, 9]
// Alternate (using # specifier):
// [  1,   2,   3
//    3,  41,   5
//  157,   8,   9]
template <typename T, s32 R, s32 C, bool Packed>
struct formatter<mat<T, R, C, Packed>> {
    void format(const mat<T, R, C, Packed> &src, format_context *f) {
        f->write("[");

        bool alternate = f->Specs && f->Specs->has_flag(flag::HASH);
        size_t max = 0;
        if (alternate) {
            for (s32 i = 0; i < src.Height; ++i) {
                for (s32 j = 0; j < src.Width; ++j) {
                    size_t s;
                    if constexpr (is_floating_point_v<T>) {
                        s = calculate_formatted_size("{:f}", src(i, j));
                    } else {
                        s = calculate_formatted_size("{}", src(i, j));
                    }
                    if (s > max) max = s;
                }
            }
        }

        auto *old = f->Specs;
        f->Specs = null;
        for (s32 i = 0; i < src.Height; ++i) {
            for (s32 j = 0; j < src.Width; ++j) {
                if (alternate) {
                    if constexpr (is_floating_point_v<T>) {
                        to_writer(f, "{0:<{1}f}", src(i, j), max);
                    } else {
                        to_writer(f, "{0:<{1}}", src(i, j), max);
                    }
                } else {
                    if constexpr (is_floating_point_v<T>) {
                        to_writer(f, "{0:f}", src(i, j));
                    } else {
                        to_writer(f, "{0:}", src(i, j));
                    }
                }
                if (j != src.Width - 1) f->write(", ");
            }
            if (i < src.R - 1) f->write(alternate ? "\n " : "; ");
        }
        f->Specs = old;
        f->write("]");
    }
};

// Prints in format: quat(1, 0, 0, 0)
// Alternate (using # specifier): [ 60 deg @ [0, 1, 0] ] (rotation in degrees around axis)
template <typename T, bool Packed>
struct formatter<tquat<T, Packed>> {
    void format(const tquat<T, Packed> &src, format_context *f) {
        bool alternate = f->Specs && f->Specs->has_flag(flag::HASH);
        if (alternate) {
            f->write("[");
            to_writer(f, "{.f}", src.angle() / TAU * 360);
            f->write(" deg @ ");
            to_writer(f, "{}", src.axis());
            f->write("]");
        } else {
            f->debug_tuple("quat").field(src.s)->field(src.i)->field(src.j)->field(src.k)->finish();
        }
    }
};

// Prints in format: quat(1, 0, 0, 0)
// Alternate (using # specifier): [ 60 deg @ [0, 1, 0] ] (rotation in degrees around axis)
template <>
struct formatter<guid> {
    void format(const guid &src, format_context *f) {
        char type = 'd';
        if (f->Specs) {
            type = f->Specs->Type;
        }

        bool upper = is_upper(type);
        type = (char) to_lower(type);

        if (type != 'n' && type != 'd' && type != 'b' && type != 'p' && type != 'x') {
            f->on_error("Invalid type specifier");
            return;
        }

        char32_t openParenthesis = 0, closedParenthesis = 0;
        bool hyphen = true;

        if (type == 'n') {
            hyphen = false;
        } else if (type == 'b') {
            openParenthesis = '{';
            closedParenthesis = '}';
        } else if (type == 'p') {
            openParenthesis = '(';
            closedParenthesis = ')';
        } else if (type == 'x') {
            union {
                char Data[16];
                struct {
                    u32 D1;
                    u16 D5, D7;
                    u8 D9, D10, D11, D12, D13, D14, D15, D16;
                };
            } u;
            copy_memory(u.Data, src.Data, 16);

            auto *old = f->Specs;
            f->Specs = null;
            if (upper) {
                to_writer(
                    f,
                    "{{{:#010X},{:#06X},{:#06X},{{{:#04X},{:#04X},{:#04X},{:#04X},{:#04X},{:#04X},{:#04X},{:#04X}}}}}",
                    u.D1, u.D5, u.D7, u.D9, u.D10, u.D11, u.D12, u.D13, u.D14, u.D15, u.D16);
            } else {
                to_writer(
                    f,
                    "{{{:#010x},{:#06x},{:#06x},{{{:#04x},{:#04x},{:#04x},{:#04x},{:#04x},{:#04x},{:#04x},{:#04x}}}}}",
                    u.D1, u.D5, u.D7, u.D9, u.D10, u.D11, u.D12, u.D13, u.D14, u.D15, u.D16);
            }
            f->Specs = old;
            return;
        }

        if (openParenthesis) f->write_no_specs(openParenthesis);

        auto *old = f->Specs;
        f->Specs = null;

        const char *p = src.Data;
        For(range(16)) {
            if (hyphen && (it == 4 || it == 6 || it == 8 || it == 10)) {
                f->write_no_specs((char32_t) '-');
            }
            if (upper) {
                to_writer(f, "{:02X}", (u8) *p);
            } else {
                to_writer(f, "{:02x}", (u8) *p);
            }
            ++p;
        }
        f->Specs = old;

        if (closedParenthesis) f->write_no_specs(closedParenthesis);
    }
};

}  // namespace fmt

LSTD_END_NAMESPACE