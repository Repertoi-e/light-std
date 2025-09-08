#pragma once

#include "common.h"
#include "string_builder.h"
#include "array_like.h"
#include "variant.h"
#include "hash_table.h"
#include "linked_list_like.h"
#include "type_info.h"
#include "fmt/arg.h"
#include "fmt/context.h"
#include "fmt/interp.h"
#include "fmt/storage_types.h"
#include "fmt/text_style.h"

LSTD_BEGIN_NAMESPACE

//
// Format specification:
//
// The formatting engine in this library is similar to how python handles
// advanced string formatting.
//
// Format strings consist of characters and fields encoded in utf-8.
// Fields define how an argument gets formatted to the output while the rest of
// the characters get transfered unchanged.
//
// Fields are defined with curly braces, like so:
//    print("This is an {}", "example") -> "This is an example"
// Braces can be escaped by doubling:
//    print("Hey there :-{{}}")         -> "Hey there :-{}"
//
// You can specify which argument a field refers to by index or by name.
//    print("{0} {1} {0}", "first", "second") -> "first second first"
//   Note: If you leave the braces without an index, by default it is
//   automatically incremented
//         with each new field. However you may not switch between automatic and
//         manual indexing throughout the formatting string.
//
// Format specifiers follow the index, with ":" separating the two:
//    print("{:<8}", "Jon")  -> "     Jon"
//
// The general form of a standard format specifier is:
//     [[fill]align][sign][#][0][width][.precision][type]
// The brackets ([]) indicate an optional element.
//
//     The optional align flag is one of the following:
//       '<' - Forces the field to be left-aligned within the available space
//       (default)
//       '>' - Forces the field to be right-aligned within the available space.
//       '^' - Forces the field to be centered within the available space
//       '=' - Forces the padding to be placed after the sign (if any)
//             but before the digits.  This is used for printing fields
//             in the form '+000000120'. This alignment option is only
//             valid for numeric types.
//
//     Note that unless a minimum field width is defined, the field width will
//     always be the same size as the data to fill it, so that the alignment
//     option has no meaning in this case.
//
//     The optional 'fill' code point defines the code point to be used to pad
//     the field to the minimum width. The fill code point, if present, must be
//     followed by an alignment flag. The fill element can be multiple bytes and
//     must be encoded in utf-8.
//
//     The 'sign' option is only valid for numeric types, and can be one of the
//     following:
//       '+'  - Indicates that a sign should be used for both positive as well
//       as negative numbers
//       '-'  - Indicates that a sign should be used only for negative numbers
//       (default) ' '  - Indicates that a leading space should be used on
//       positive numbers
//
//     If the '#' character is present, integers use the 'alternate form' for
//     formatting. This means that binary, octal, and hexadecimal output will be
//     prefixed with '0b', '0o', and '0x', respectively.
//
//     'width' is a decimal integer defining the minimum field width.
//     If not specified, then the field width will be determined by the content.
//
//     If the width field is preceded by a zero('0') character, this enables
//     zero-padding. This is equivalent to an alignment type of '=' and a fill
//     character of '0'.
//
//     The 'precision' is a decimal number indicating how many digits should be
//     displayed after the decimal point in a floating point conversion. For
//     non-numeric types the field indicates the maximum field size - in other
//     words, how many characters will be used from the field content. The
//     precision is ignored for integer conversions.
//
//     Finally, the 'type' byte determines how the data should be presented:
//
//      Integers:
//       'b' - Binary integer. Outputs the number in base 2.
//       'c' - Code point. Converts the integer to the corresponding unicode
//       code point before printing. 'd' - Decimal integer. Outputs the number
//       in base 10. 'o' - Octal format. Outputs the number in base 8. 'x' - Hex
//       format. Outputs the number in base 16, using lower-case letters for the
//       digits above 9. 'X' - Hex format. Outputs the number in base 16, using
//       upper-case letters for the digits above 9. 'n' - Number. This is the
//       same as 'd', except that it inserts thousands separator
//             (currently a dot that doesn't get determined by the locale)
//       '' (NONE) - the same as 'd'
//
//      Floats:
//       'e' - Exponent notation. Prints the number in scientific notation using
//       'e' for the exponent. 'E' - Uppercase version of 'e'. 'f' - Fixed
//       point. Displays the number as a fixed-point number. 'F' - Uppercase
//       version of 'f'. 'g' - General format. This prints the number as a
//       fixed-point number, unless the number is too large,
//             in which case it switches to 'e' exponent notation.
//       'G' - Uppercase version of 'g'.
//       '%' - Percentage. Multiplies the number by 100 and displays in fixed
//       ('f') format, followed by a percent sign.
//       '' (NONE) - similar to 'g', except that it prints at least one
//             digit after the decimal point.
//
//      Pointers:
//       'p' - Outputs a 'const void *' formatted in hexadecimal. Example:
//       0xab5c8fea84
//       '' (NONE) - the same as 'p'
//
//      C-Style string:
//       'p' - Treats the argument as a pointer.
//       's' - Outputs it as an utf-8 encoded string.
//       '' (NONE) - the same as 's'
//
//      Strings:
//       's' - Outputs it as an utf-8 encoded string.
//       '' (NONE) - the same as 's'
//
//
//
// There is also a way to specify text styles directly in the format string.
// Without requiring an argument. Text styles are defined by a opening curly
// brace ('{') followed by '!', then the text style and a closing brace ('}') An
// empty text style resets any foreground/background color and text emphasis.
//    print("{!}")
//
// There are 3 ways to define a text color. The color is optional but it must be
// the first thing after '!'.
//    1) Using the name of a color, e.g. {!CORNFLOWER_BLUE}
//       A full list of recognized colors is available in "lstd/fmt/colors.def"
//       and programatically in the color enum
//	        print("{!DARK_MAGENTA}")
//    2) Using the name of a "terminal" color. Use these colors if the console
//    you are printing to doesn't support
//       24 bit true color. These are the most basic colors supported by almost
//       any console.
//           BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
//           and the bright versions: BRIGHT_BLACK, BRIGHT_RED, ...
//       In order to distinguish between 24-bit color and 4-bit, a leading 't'
//       is required after the '!'
//	        print("{!tBRIGHT_CYAN}")
//    3) Using RGB. This gives you full control and ability to specify any
//    24-bit color.
//       Channels are parsed in the order red - green - blue and must be
//       separated by ';'
//	        print("{!50;230;170}")
//       Values must be in the range [0-255]
//
// After the text color you can mark is as background using "BG":
//	        print("{!WHITE;BG}")
// That means the color applies to the background and not the foreground.
//
// If you didn't mark the color as background, you can specify a series of
// characters that define the emphasis style of the text.
//	        print("{!WHITE;BIUS}")
//    Here "BIUS" specifies all the types of emphasis:
//      B (bold), I (italic, rarely supported by consoles though), U (underline)
//      and S (strikethrough)
//    They can be in any order and are optional.
//    e.g. valid emphasis strings are: "BI", "U", "B", "SU", "SB", etc...
//       Note: When parsing, if we fail to find the name of a color, e.g.
//       {!IMAGINARYCOLOR}, we treat
//             the series of characters as emphasis, although any character
//             encountered that is not a valid emphasis gets reported as an
//             error. This allows specifying emphasis without color:
//                 print("{!BU}");
//
// You can disable text styles with _Context.FmtDisableAnsiCodes_.
// That is useful when logging to a file and not a console. The ansi escape
// codes look like garbage in files.
//
//
// CUSTOM TYPE FORMATTING:
//
// There is a way to add formatting support for custom types:
//
// Formatter specialization (trait-like approach):
//
//    template <>
//    struct formatter<my_type> {
//        void format(const my_type &value, fmt_context *f) {
//            // Your formatting logic here
//            fmt_to_writer(f, "my_type(x: {}, y: {})", value.x, value.y);
//        }
//    };
//
// Your formatter can also check format specifiers:
//    template <>
//    struct formatter<my_type> {
//        void format(const my_type &value, fmt_context *f) {
//            bool use_debug = f->Specs && f->Specs->Hash;
//            if (use_debug) {
//                fmt_to_writer(f, "my_type {{ x: {}, y: {} }}", value.x, value.y);
//            } else {
//                fmt_to_writer(f, "({}, {})", value.x, value.y);
//            }
//        }
//    };
//
// More example formatters for custom types:
//
/*
      // Example: Custom vector formatter
      // Formats vector in the following way: [1, 2, ...]
      template <typename T, s32 Dim, bool Packed>
      struct formatter<vec<T, Dim, Packed>> {
          void format(const vec<T, Dim, Packed> &src, fmt_context *f) {
              format_list(f).entries(src.Data, src.DIM)->finish();
          }
      };

      // Example: Custom matrix formatter with spec support
      // Formats in the following way: [ 1, 2, 3; 4, 5, 6; 7, 8, 9]
      // Alternate (using # specifier):
      // [  1,   2,   3
      //    3,  41,   5
      //  157,   8,   9]
      template <typename T, s64 R, s64 C, bool Packed>
      struct formatter<mat<T, R, C, Packed>> {
          void format(const mat<T, R, C, Packed> &src, fmt_context *f) {
              write_no_specs(f, "[");

              bool alternate = f->Specs && f->Specs->Hash;
              s64 max = 0;

              // Calculate max width for alignment if using alternate format
              if (alternate) {
                  for (s32 i = 0; i < src.Height; ++i) {
                      for (s32 j = 0; j < src.Width; ++j) {
                          s64 length = fmt_calculate_length("{}", src(i, j));
                          if (length > max) max = length;
                      }
                  }
              }

              // Format matrix elements
              for (s32 i = 0; i < src.Height; ++i) {
                  for (s32 j = 0; j < src.Width; ++j) {
                      if (alternate) {
                          fmt_to_writer(f, "{:<{}}", src(i, j), max);
                      } else {
                          fmt_to_writer(f, "{}", src(i, j));
                      }
                      if (j != src.Width - 1) write_no_specs(f, ", ");
                  }
                  if (i < src.Height - 1) {
                      write_no_specs(f, alternate ? "\n " : "; ");
                  }
              }

              write_no_specs(f, "]");
          }
      };

      // Example: Custom quaternion formatter
      // Formats in the following way: quat(1, 0, 0, 0)
      // Alternate (using # specifier): [ 60 deg @ [0, 1, 0] ] (rotation in degrees around axis)
      template <typename T, bool Packed>
      struct formatter<tquat<T, Packed>> {
          void format(const tquat<T, Packed> &src, fmt_context *f) {
              bool alternate = f->Specs && f->Specs->Hash;
              if (alternate) {
                  write_no_specs(f, "[ ");
                  fmt_to_writer(f, "{:.1f}", src.angle() / TAU * 360);
                  write_no_specs(f, " deg @ ");
                  fmt_to_writer(f, "{}", src.axis());
                  write_no_specs(f, " ]");
              } else {
                  format_tuple(f, "quat")
                      .field(src.s)
                      ->field(src.i)
                      ->field(src.j)
                      ->field(src.k)
                      ->finish();
              }
          }
      };
*/

struct fmt_context;

//
// These are the most common functions.
// If you are doing something specific, you can look
// into the implementation details further down this file.
//

// Formats to a writer.
template <typename... Args>
void fmt_to_writer(writer *out, string fmtString, Args no_copy... arguments);

// Formats to a counting writer and returns the result - how many bytes would be
// written with the given format string and args.
template <typename... Args>
s64 fmt_calculate_length(string fmtString, Args no_copy... arguments);

// Formats to a string. The caller is responsible for freeing.
template <typename... Args>
mark_as_leak string sprint(string fmtString, Args no_copy... arguments);

// Formats to a string. Uses the temporary allocator.
template <typename... Args>
string tprint(string fmtString, Args no_copy... arguments);

// Formats to a string then converts to null-terminated string. Uses the
// temporary allocator.
template <typename... Args>
char *mprint(string fmtString, Args no_copy... arguments);

// Calls fmt_to_writer on Context.Log - which is pointing to the console by
// default, but that can be changed to redirect the output.
template <typename... Args>
void print(string fmtString, Args no_copy... arguments);

// Expects a valid fmt_context (take a look in the implementation of
// fmt_to_writer). Does all the magic of parsing the format string and
// formatting the arguments.
void fmt_parse_and_format(fmt_context *f);

struct fmt_width_checker
{
  fmt_context *F;

  template <typename T>
  u32 operator()(T value)
  {
    if constexpr (is_integral<T>)
    {
      if (sign_bit(value))
      {
        on_error(F, "Negative width");
        return (u32)-1;
      }
      else if ((u64)value > numeric<s32>::max())
      {
        on_error(F, "Width value is too big");
        return (u32)-1;
      }
      return (u32)value;
    }
    else
    {
      on_error(F, "Width was not an integer");
      return (u32)-1;
    }
  }
};

struct fmt_precision_checker
{
  fmt_context *F;

  template <typename T>
  s32 operator()(T value)
  {
    if constexpr (is_integral<T>)
    {
      if (sign_bit(value))
      {
        on_error(F, "Negative precision");
        return -1;
      }
      else if ((u64)value > numeric<s32>::max())
      {
        on_error(F, "Precision value is too big");
        return -1;
      }
      return (s32)value;
    }
    else
    {
      on_error(F, "Precision was not an integer");
      return -1;
    }
  }
};

inline fmt_arg fmt_get_arg_from_index(fmt_context *f, s64 index)
{
  if (index >= f->Args.Count)
  {
    on_error(f, "Argument index out of range");
    return {};
  }
  return f->Args[index];
}

inline bool fmt_handle_dynamic_specs(fmt_context *f)
{
  assert(f->Specs);

  if (f->Specs->WidthIndex != -1)
  {
    auto width = fmt_get_arg_from_index(f, f->Specs->WidthIndex);
    if (width.Type != fmt_type::NONE)
    {
      f->Specs->Width = fmt_visit_arg(fmt_width_checker{f}, width);
      if (f->Specs->Width == (u32)-1)
        return false;
    }
  }
  if (f->Specs->PrecisionIndex != -1)
  {
    auto precision = fmt_get_arg_from_index(f, f->Specs->PrecisionIndex);
    if (precision.Type != fmt_type::NONE)
    {
      f->Specs->Precision = fmt_visit_arg(fmt_precision_checker{f}, precision);
      if (f->Specs->Precision == numeric<s32>::min())
        return false;
    }
  }

  return true;
}

inline void fmt_parse_and_format(fmt_context *f)
{
  fmt_interp *p = &f->Parse;

  auto write_until = [&](const char *end)
  {
    if (!p->It.Count)
      return;
    while (true)
    {
      auto searchString = string(p->It.Data, end - p->It.Data);

      s64 bracket = search(searchString, '}');
      if (bracket == -1)
      {
        write_no_specs(f, p->It.Data, end - p->It.Data);
        return;
      }

      auto *pbracket = utf8_get_pointer_to_cp_at_translated_index(
          searchString.Data, searchString.Count, bracket);
      if (*(pbracket + 1) != '}')
      {
        on_error(f,
                 "Unmatched \"}\" in format string - if you want to print it "
                 "use \"}}\" to escape",
                 pbracket - f->Parse.FormatString.Data);
        return;
      }

      write_no_specs(f, p->It.Data, pbracket - p->It.Data);
      write_no_specs(f, "}");

      s64 advance = pbracket + 2 - p->It.Data;
      p->It.Data += advance, p->It.Count -= advance;
    }
  };

  fmt_arg currentArg;

  while (p->It.Count)
  {
    s64 bracket = search(p->It, '{');
    if (bracket == -1)
    {
      write_until(p->It.Data + p->It.Count);
      return;
    }

    auto *pbracket = utf8_get_pointer_to_cp_at_translated_index(
        p->It.Data, p->It.Count, bracket);
    write_until(pbracket);

    s64 advance = pbracket + 1 - p->It.Data;
    p->It.Data += advance, p->It.Count -= advance;

    if (!p->It.Count)
    {
      on_error(f, "Invalid format string");
      return;
    }
    if (p->It[0] == '}')
    {
      // Implicit {} means "get the next argument"
      currentArg = fmt_get_arg_from_index(f, p->next_arg_id());
      if (currentArg.Type == fmt_type::NONE)
        return; // The error was reported in _f->get_arg_from_ref_

      fmt_visit_arg(fmt_context_visitor(f), currentArg);
    }
    else if (p->It[0] == '{')
    {
      // {{ means we escaped a {.
      write_until(p->It.Data + 1);
    }
    else if (p->It[0] == '!')
    {
      ++p->It.Data, --p->It.Count; // Skip the !

      auto [success, style] = fmt_parse_text_style(p);
      if (!success)
        return;
      if (!p->It.Count || p->It[0] != '}')
      {
        on_error(f, "\"}\" expected");
        return;
      }

      if (!Context.FmtDisableAnsiCodes)
      {
        char ansiBuffer[7 + 3 * 4 + 1];
        auto *ansiEnd = color_to_ansi(ansiBuffer, style);
        write_no_specs(f, ansiBuffer, ansiEnd - ansiBuffer);

        u8 emphasis = (u8)style.Emphasis;
        if (emphasis)
        {
          assert(!style.Background);
          ansiEnd = emphasis_to_ansi(ansiBuffer, emphasis);
          write_no_specs(f, ansiBuffer, ansiEnd - ansiBuffer);
        }
      }
    }
    else
    {
      // Parse integer specified or a named argument
      s64 argId = fmt_parse_arg_id(p);
      if (argId == -1)
        return;

      currentArg = fmt_get_arg_from_index(f, argId);
      if (currentArg.Type == fmt_type::NONE)
        return; // The error was reported in _f->get_arg_from_ref_

      code_point c = p->It.Count ? p->It[0] : 0;
      if (c == '}')
      {
        fmt_visit_arg(fmt_context_visitor(f), currentArg);
      }
      else if (c == ':')
      {
        ++p->It.Data, --p->It.Count; // Skip the :

        fmt_dynamic_specs specs = {};
        bool success = fmt_parse_specs(p, currentArg.Type, &specs);
        if (!success)
          return;
        if (!p->It.Count || p->It[0] != '}')
        {
          on_error(f, "\"}\" expected");
          return;
        }

        f->Specs = &specs;
        success = fmt_handle_dynamic_specs(f);
        if (!success)
          return;

        fmt_visit_arg(fmt_context_visitor(f), currentArg);

        f->Specs = null;
      }
      else
      {
        on_error(f, "\"}\" expected");
        return;
      }
    }
    ++p->It.Data, --p->It.Count; // Go to the next byte
  }
}

template <typename... Args>
void fmt_to_writer(writer *out, string fmtString, Args no_copy... arguments)
{
  static const s64 NUM_ARGS = sizeof...(Args);
  stack_array<fmt_arg, NUM_ARGS> args;

  args = {fmt_make_arg(arguments)...};
  auto f = fmt_context(out, fmtString, args);

  fmt_parse_and_format(&f);
  f.flush();
}

template <typename... Args>
s64 fmt_calculate_length(string fmtString, Args no_copy... arguments)
{
  counting_writer writer;
  fmt_to_writer(&writer, fmtString, arguments...);
  return writer.Count;
}

template <typename... Args>
mark_as_leak string sprint(string fmtString, Args no_copy... arguments)
{
  string_builder b;

  string_builder_writer writer;
  writer.Builder = &b;
  fmt_to_writer(&writer, fmtString, arguments...);

  string combined = builder_to_string(&b);
  free_buffers(&b);

  return combined;
}

template <typename... Args>
string tprint(string fmtString, Args no_copy... arguments)
{
  PUSH_ALLOC(TemporaryAllocator) { return sprint(fmtString, arguments...); }
}

template <typename... Args>
char *mprint(string fmtString, Args no_copy... arguments)
{
  PUSH_ALLOC(TemporaryAllocator)
  {
    return to_c_string(sprint(fmtString, arguments...));
  }
}

template <typename... Args>
void print(string fmtString, Args no_copy... arguments)
{
  assert(Context.Log && "Context log was null. By default it points to cout.");
  fmt_to_writer(Context.Log, fmtString, arguments...);
}

//
// Primary formatter template - specialize this for custom types
// This follows a trait-like pattern similar to Rust's formatting traits
//
template <typename T>
struct formatter
{
  // The format() method should be specialized for each type
  // void format(const T &value, fmt_context *f) { ... }
};

template <typename T>
void format_value(const T &value, fmt_context *f)
{
  if constexpr (has_formatter<T>)
  {
    formatter<remove_cvref_t<T>>{}.format(value, f);
  }
  else
  {
    // Fall back to standard formatting for built-in types
    fmt_arg arg = fmt_make_arg(value);
    fmt_visit_arg(fmt_context_visitor(f), arg);
  }
}

//
// Formatter specializations for built-in types:
//

// Formatter for string_builder
template <>
struct formatter<string_builder>
{
  void format(const string_builder &b, fmt_context *f)
  {
    auto *buffer = &b.BaseBuffer;
    while (buffer)
    {
      write_no_specs(f, buffer->Data, buffer->Occupied);
      buffer = buffer->Next;
    }
  }
};

// Formatter for static array-like types (stack_array, etc.)
template <typename T>
  requires any_array_like<T> && (!any_dynamic_array_like<T>)
struct formatter<T>
{
  void format(const T &a, fmt_context *f)
  {
    bool use_debug = f->Specs && f->Specs->Hash;
    if (use_debug)
    {
      // Avoid forwarding type-specific specs to metadata fields (like count)
      auto *original_specs = f->Specs;
      write_no_specs(f, "<array_like> { count: ");
      f->Specs = nullptr;
      format_value(a.Count, f);
      write_no_specs(f, ", data: ");
      // Restore specs for list entries so element-level forwarding works
      f->Specs = original_specs;
      format_list(f).entries(a.Data, a.Count)->finish();
      // Restore (not strictly necessary here) and close
      f->Specs = original_specs;
      write_no_specs(f, " }");
    }
    else
    {
      format_list(f).entries(a.Data, a.Count)->finish();
    }
  }
};

// Formatter for dynamic array-like types (array, etc.)
template <typename T>
  requires any_dynamic_array_like<T>
struct formatter<T>
{
  void format(const T &a, fmt_context *f)
  {
    bool use_debug = f->Specs && f->Specs->Hash;

    if (use_debug)
    {
      // Debug format: array { count: X, capacity: Y, allocated: Z, data: [...] }
      auto *original_specs = f->Specs;
      write_no_specs(f, "<dynamic_array_like> { count: ");
      // Do not apply value specs to metadata fields
      f->Specs = nullptr;
      format_value(a.Count, f);
      write_no_specs(f, ", allocated: ");
      format_value(a.Allocated, f);
      write_no_specs(f, ", data: ");
      // Restore specs so element list receives forwarded specs
      f->Specs = original_specs;
      format_list(f).entries(a.Data, a.Count)->finish();
      // Restore and close
      f->Specs = original_specs;
      write_no_specs(f, " }");
    }
    else
    {
      // Normal format: [...]
      format_list(f).entries(a.Data, a.Count)->finish();
    }
  }
};

// Formatter for variant<MEMBERS...>
template <typename... MEMBERS>
struct formatter<variant<MEMBERS...>>
{
  void format(const variant<MEMBERS...> &v, fmt_context *f)
  {
    if (!v)
    {
      write_no_specs(f, "nullvar");
    }
    else
    {
      // Store original specs for restoration
      fmt_dynamic_specs *original_specs = f->Specs;

      // Visit the variant and format the contained value with appropriate specs
      v.visit([f, original_specs](const auto &value)
              {
        using ValueType = decay_t<decltype(value)>;
        if constexpr (!is_same<ValueType, typename variant<MEMBERS...>::nil>) {
          if (original_specs) {
            auto forwarded_specs = create_forwarded_specs(*original_specs, value);
            f->Specs = &forwarded_specs;
            format_value(value, f);
          } else {
            f->Specs = nullptr;
            format_value(value, f);
          }
          
          // Restore original specs
          f->Specs = original_specs;
        } });
    }
  }
};

// Formatter for optional<T> (which is just variant<T>)
template <typename T>
struct formatter<optional<T>>
{
  void format(const optional<T> &opt, fmt_context *f)
  {
    if (opt)
    {
      format_value(opt.template strict_get<T>(), f);
    }
    else
    {
      // Optional is empty
      write_no_specs(f, "nullopt");
    }
  }
};

// Formatter for hash table
template <typename K, typename V>
struct formatter<hash_table<K, V>>
{
  void format(const hash_table<K, V> &table, fmt_context *f)
  {
    bool use_debug = f->Specs && f->Specs->Hash;
    // Pretty-printing is specifically space fill character + width > 0
    bool use_pretty = f->Specs && f->Specs->Fill == ' ' && f->Specs->Width > 0;
    s32 indent_size = use_pretty ? f->Specs->Width : 0;
    s32 current_level = f->Specs ? f->Specs->UserData : 0;

    if (use_debug)
    {
      // Alternate format: displays as a more detailed view
      // e.g. hash_table<string, int> { count: 3, entries: { "key1": 1, "key2": 2, "key3": 3 } }
      auto *original_specs = f->Specs;
      write_no_specs(f, "hash_table { count: ");
      // Do not forward specs to metadata count
      f->Specs = nullptr;
      format_value(table.Count, f);
      write_no_specs(f, ", entries: ");

      format_dict dict(f);
      // Need to cast away const to iterate since hash table iterators expect non-const
      auto &mutable_table = const_cast<hash_table<K, V> &>(table);
      for (auto [key, value] : mutable_table)
      {
        dict.entry(*key, *value);
      }
      // Restore specs so dict entries get forwarded specs (including pretty)
      f->Specs = original_specs;
      if (use_pretty)
        dict.pretty(indent_size, current_level);
      dict.finish();

      write_no_specs(f, " }");
    }
    else
    {
      // Default format: displays as a simple dictionary
      // e.g. { "key1": 1, "key2": 2, "key3": 3 }
      format_dict dict(f);
      // Need to cast away const to iterate since hash table iterators expect non-const
      auto &mutable_table = const_cast<hash_table<K, V> &>(table);
      for (auto [key, value] : mutable_table)
      {
        dict.entry(*key, *value);
      }
      if (use_pretty)
        dict.pretty(indent_size, current_level);
      dict.finish();
    }
  }
};

// Formatters for linked list views
template <typename Node>
  requires (singly_linked_node_like<Node> && !doubly_linked_node_like<Node>)
struct formatter<Node*> {
  void format(Node* no_copy v, fmt_context *f) {
    bool use_debug = f->Specs && f->Specs->Hash;

    // Collect node values into fmt_args so we can reuse format_list
    array<fmt_arg> items;
    for (auto p = v; p; p = p->Next) add(items, fmt_make_arg(*p));

    if (use_debug) {
      auto *orig = f->Specs;
      f->Specs = nullptr;

      write_no_specs(f, "<singly_linked_list_like> { count: ");
      format_value(items.Count, f);
      write_no_specs(f, ", data: ");

      // Restore specs for elements
      // When only precision is specified with no type, prefer fixed-point for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0) {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      } else {
        f->Specs = orig;
      }

      format_list(f).entries_args(items)->finish();

      // Restore and close
      f->Specs = orig;
      write_no_specs(f, " }");
    } else {
      auto *orig = f->Specs;
      // Coerce precision-only to fixed for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0) {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      }
      format_list(f).entries_args(items)->finish();
      f->Specs = orig;
    }

    free(items);
  }
};

template <typename Node>
  requires (doubly_linked_node_like<Node>)
struct formatter<Node*> {
  void format(const Node* no_copy v, fmt_context *f) {
    bool use_debug = f->Specs && f->Specs->Hash;

    // Collect node values into fmt_args so we can reuse format_list
    array<fmt_arg> items;
    for (auto p = v; p; p = p->Next) add(items, fmt_make_arg(*p));

    if (use_debug) {
      auto *orig = f->Specs;
      write_no_specs(f, "<doubly_linked_list_like> { count: ");
      f->Specs = nullptr;
      format_value(items.Count, f);
      write_no_specs(f, ", data: ");
      // Restore specs for elements
      // When only precision is specified with no type, prefer fixed-point for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0) {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      } else {
        f->Specs = orig;
      }

      format_list(f).entries_args(items)->finish();

      // Restore and close
      f->Specs = orig;
      write_no_specs(f, " }");
    } else {
      auto *orig = f->Specs;
      // Coerce precision-only to fixed for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0) {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      }
      format_list(f).entries_args(items)->finish();
      f->Specs = orig;
    }

    free(items);
  }
};

LSTD_END_NAMESPACE
