#pragma once

#include "../context.h"
#include "../io.h"
#include "fmt/color.h"
#include "fmt/formatter.h"
#include "fmt/string_checker.h"

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
//     [[fill]align][sign][#][0][minimumwidth][.precision][type]
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
//       'E' - Exponent notation. Same as 'e' except it converts the number to uppercase.
//       'f' - Fixed point. Displays the number as a fixed-point number.
//       'F' - Fixed point. Same as 'f' except it converts the number to uppercase.
//       'g' - General format. This prints the number as a fixed-point number, unless the number is too large,
//             in which case it switches to 'e' exponent notation.
//       'G' - General format. Same as 'g' except switches to 'E' if the number gets to large.
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

LSTD_BEGIN_NAMESPACE

namespace fmt {

// Formats to writer
template <typename... Args, typename S>
enable_if_t<can_be_fmt_string_v<S>> format_to_writer(io::writer *out, S fmtString, Args &&... args) {
    maybe_compile_time_check_format_string(fmtString);

    auto f = format_context(out, string_view(fmtString), {fmt::make_fmt_args(((Args &&) args)...)});
    auto handler = fmt::format_handler(&f);
    parse_format_string<false>(&f.Parse, &handler);
    f.flush();
}

// Formats to a counting writer and returns the result
template <typename... Args, typename S>
enable_if_t<can_be_fmt_string_v<S>, size_t> calculate_formatted_size(S fmtString, Args &&... args) {
    io::counting_writer writer;
    format_to_writer(&writer, fmtString, ((Args &&) args)...);
    return writer.Count;
}

// Formats to a string
template <typename... Args, typename S>
enable_if_t<can_be_fmt_string_v<S>> sprint(S fmtString, Args &&... args) {
    string result;
    result.reserve(calculate_formatted_size(fmtString, ((Args &&) args)...));  // @Speed Is this actually better?

    auto writer = io::string_writer(&result);
    format_to_writer(&writer, fmtString, ((Args &&) args)...);
    return result;
}

// Formats to io::cout
template <typename... Args, typename S>
enable_if_t<can_be_fmt_string_v<S>> print(S fmtString, Args &&... args) {
    format_to_writer(Context.Log, fmtString, ((Args &&) args)...);
}

}  // namespace fmt

LSTD_END_NAMESPACE