#pragma once

#include "common.h"
#include "string_builder.h"
#include "array_like.h"
#include "variant.h"
#include "hash_table.h"
#include "linked_list_like.h"

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
              write_list(f, src.Data, src.DIM);
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
                  {
                    array<fmt_arg> _fields;
                    add(_fields, fmt_arg_make(src.s));
                    add(_fields, fmt_arg_make(src.i));
                    add(_fields, fmt_arg_make(src.j));
                    add(_fields, fmt_arg_make(src.k));
                    write_tuple(f, "quat", _fields);
                    free(_fields);
                  }
              }
          }
      };
*/

enum class fmt_type
{
  NONE = 0,
  S64,
  U64,
  BOOL,
  F32,
  F64,
  STRING,
  POINTER,
  CUSTOM
};

inline bool fmt_type_is_integral(fmt_type type)
{
  return type == fmt_type::S64 || type == fmt_type::U64 || type == fmt_type::BOOL;
}

inline bool fmt_type_is_arithmetic(fmt_type type)
{
  return fmt_type_is_integral(type) || type == fmt_type::F32 || type == fmt_type::F64;
}

struct fmt_context;

template <typename T>
struct formatter;

template <typename T>
concept has_formatter = requires(const T &value, fmt_context *f) {
  formatter<remove_cvref_t<T>>{}.format(value, f);
};

// Type-erased value storage used by fmt_arg
struct fmt_value
{
  struct custom_value
  {
    const void *Data;
    void (*FormatFunc)(fmt_context *formatContext, const void *arg);
  };

  union
  {
    s64 S64;
    u64 U64;
    f32 F32;
    f64 F64;

    void *Pointer;
    string String;

    custom_value Custom;
  };

  fmt_value(s64 v = 0) : S64(v) {}
  fmt_value(bool v) : S64(v) {} // Store bools in S64
  fmt_value(u64 v) : U64(v) {}
  fmt_value(f32 v) : F32(v) {}
  fmt_value(f64 v) : F64(v) {}
  fmt_value(void *v) : Pointer(v) {}
  fmt_value(string v) : String(v) {}

  // Attempt to call a custom formatter.
  // Compile-time asserts if there was no overload.
  template <typename T>
  fmt_value(T *v)
  {
    Custom.Data = (const void *)v;
    Custom.FormatFunc = call_write_on_custom_arg<T>;
  }

  template <typename T>
  static void call_write_on_custom_arg(fmt_context *formatContext, const void *arg)
  {
    static_assert(has_formatter<T>, "No formatter found for custom type T");

    auto f = formatter<remove_cvref_t<T>>{};
    f.format(*static_cast<const T *>(arg), formatContext);
  }
};

//
// If the value is not arithmetic (custom or string type)
// then the life time of the parameter isn't extended
// (we just hold a pointer)! That means that the parameters
// need to outlive the parse and format function itself.
//
struct fmt_arg
{
  fmt_type Type = fmt_type::NONE;
  fmt_value Value;
};

// Maps formatting arguments to types that can be used to construct a fmt_value.
//
// The order in which we look:
//   * is string constructible from T? then we map to string(T)
//   * is the type a code_point_ref? maps to u64 (we want the value in that
//   case)
//   * is the type an (unsigned) integral? maps to (u64) s64
//   * is the type an enum? calls map_arg again with the underlying type
//   * is the type a floating point? maps to f64
//   * is the type a pointer? if it's non-void we throw an error, otherwise we
//   map to (void *) v
//   * is the type a bool? maps to bool
//   * otherwise maps to &v (value then setups a function call to a custom
//   formatter)
auto fmt_map_arg(auto no_copy v)
{
  using T = remove_cvref_t<decltype(v)>;

  if constexpr (is_same<string, T> || is_constructible<string, T>)
  {
    return string(v);
  }
  else if constexpr (is_same<T, string::code_point_ref>)
  {
    return (u64)v;
  }
  else if constexpr (is_same<bool, T>)
  {
    return v;
  }
  else if constexpr (is_unsigned_integral<T>)
  {
    return (u64)v;
  }
  else if constexpr (is_signed_integral<T>)
  {
    return (s64)v;
  }
  else if constexpr (is_enum<T>)
  {
    return fmt_map_arg((underlying_type_t<T>)v);
  }
  else if constexpr (is_floating_point<T>)
  {
    return v;
  }
  else if constexpr (is_pointer<T>)
  {
    if constexpr (is_same<T, void *>)
    {
      return v;
    }
    else
    {
      return &v; // Require a custom formatter for non-void pointers
    }
  }
  else
  {
    return &v;
  }
}

// Map the result type of fmt_map_arg to a fmt_type at compile time
template <typename M>
constexpr fmt_type fmt_type_of_mapped()
{
  using T = remove_cvref_t<M>;
  if constexpr (is_same<T, string>)
    return fmt_type::STRING;
  else if constexpr (is_same<T, bool>)
    return fmt_type::BOOL;
  else if constexpr (is_same<T, s64>)
    return fmt_type::S64;
  else if constexpr (is_same<T, u64>)
    return fmt_type::U64;
  else if constexpr (is_same<T, f32>)
    return fmt_type::F32;
  else if constexpr (is_same<T, f64>)
    return fmt_type::F64;
  else if constexpr (is_same<T, void *>)
    return fmt_type::POINTER;
  else if constexpr (is_pointer<T>)
    return fmt_type::CUSTOM; // non-void pointer => custom
  else
    return fmt_type::CUSTOM;
}

fmt_arg fmt_arg_make(auto no_copy v)
{
  auto mapped = fmt_map_arg(v);
  return {fmt_type_of_mapped<decltype(mapped)>(), fmt_value(mapped)};
}

// Visits an argument dispatching with the right value based on the argument
// type
template <typename Visitor>
auto fmt_arg_visit(Visitor visitor, fmt_arg ar) -> decltype(visitor(0))
{
  switch (ar.Type)
  {
  case fmt_type::NONE:
    break;
  case fmt_type::S64:
    return visitor(ar.Value.S64);
  case fmt_type::U64:
    return visitor(ar.Value.U64);
  case fmt_type::BOOL:
    return visitor(ar.Value.S64 != 0); // We store bools in S64
  case fmt_type::F32:
    return visitor(ar.Value.F32);
  case fmt_type::F64:
    return visitor(ar.Value.F64);
  case fmt_type::STRING:
    return visitor(ar.Value.String);
  case fmt_type::POINTER:
    return visitor(ar.Value.Pointer);
  case fmt_type::CUSTOM:
    return visitor(ar.Value.Custom);
  }
  return visitor(unused{});
}

// @Locale
struct fmt_float_specs
{
  enum format
  {
    GENERAL, // General: chooses exponent notation or fixed point based on
             // magnitude.
    EXP,     // Exponent notation with the default precision of 6, e.g. 1.2e-3.
    FIXED,   // Fixed point with the default precision of 6, e.g. 0.0012.
    HEX
  };

  bool ShowPoint; // Whether to add a decimal point (even if no digits follow it)

  format Format;
  bool Upper;
};

// The optional align is one of the following:
//   '<' - Forces the field to be left-aligned within the available space
//   (default)
//   '>' - Forces the field to be right-aligned within the available space.
//   '=' - Forces the padding to be placed after the sign (if any)
//         but before the digits.  This is used for printing fields
//         in the form '+000000120'. This alignment option is only
//         valid for numeric types.
//   '^' - Forces the field to be centered within the available space
enum class fmt_alignment
{
  NONE = 0,
  LEFT,    // <
  RIGHT,   // >
  NUMERIC, // =
  CENTER   // ^
};

inline fmt_alignment get_alignment_from_char(code_point ch)
{
  if (ch == '<')
    return fmt_alignment::LEFT;
  if (ch == '>')
    return fmt_alignment::RIGHT;
  if (ch == '=')
    return fmt_alignment::NUMERIC;
  if (ch == '^')
    return fmt_alignment::CENTER;
  return fmt_alignment::NONE;
}

// The 'sign' option is only valid for numeric types, and can be one of the
// following:
//   '+'  - Indicates that a sign should be used for both positive as well as
//   negative numbers
//   '-'  - Indicates that a sign should be used only for negative numbers
//   (default) ' '  - Indicates that a leading space should be used on positive
//   numbers
enum class fmt_sign
{
  NONE = 0,
  PLUS,

  // MINUS has the same behaviour as NONE on our types,
  // but the user might want to have different formating
  // on their custom types when minus is specified,
  // so we record it when parsing anyway.
  MINUS,
  SPACE,
};

struct fmt_specs
{
  code_point Fill = ' ';
  fmt_alignment Align = fmt_alignment::NONE;

  fmt_sign Sign = fmt_sign::NONE;
  bool Hash = false;

  u32 Width = 0;
  s32 Precision = -1;

  char Type = 0;

  // User data for custom formatting context
  // Example for tables: stores the current indentation level for pretty-printing
  s32 UserData = 0;
};

// Dynamic means that the width/precision was specified in a separate argument
// and not as a constant in the format string
struct fmt_dynamic_specs : fmt_specs
{
  s64 WidthIndex = -1;
  s64 PrecisionIndex = -1;
};

struct fmt_parse_context
{
  string FormatString;
  string It; // How much left we have to parse from the format string

  s32 NextArgID = 0;

  fmt_parse_context(string formatString = "") : FormatString(formatString), It(formatString) {}

  // The position tells where to point the caret in the format string, so it is
  // clear where exactly the error happened. If left as -1 we calculate using
  // the current It.
  //
  // (We may want to pass a different position if we are in the middle of
  // parsing and the It is not pointing at the right place).
  //
  // This is only used to provide useful error messages.
  void on_error(string message, s64 position = -1);

  bool check_arg_id(u32);
  u32 next_arg_id();

  // Some specifiers require numeric arguments and we do error checking,
  // CUSTOM arguments don't get checked
  void require_arithmetic_arg(fmt_type argType, s64 errorPosition = -1);

  // Some specifiers require signed numeric arguments and we do error
  // checking, CUSTOM arguments don't get checked
  void require_signed_arithmetic_arg(fmt_type argType, s64 errorPosition = -1);

  // Integer values and pointers aren't allowed to get precision. CUSTOM
  // argument is again, not checked.
  void check_precision_for_arg(fmt_type argType, s64 errorPosition = -1);
};

// Note: When parsing, if we reach the end before } or : or whatever we don't
// report an error. The caller of this should handle that. Returns -1 if an
// error occured (the error is reported).
s64 fmt_parse_arg_id(fmt_parse_context *p);

// _argType_ is the type of the argument for which we are parsing the specs.
// It is used for error checking, e.g, to check if it's numeric when we
// encounter numeric-only specs.
//
// Note: When parsing, if we reach the end before } we don't report an error.
// The caller of this should handle that.
bool fmt_parse_specs(fmt_parse_context *p, fmt_type argType, fmt_dynamic_specs *specs);

// This writer contains a pointer to another writer.
// We output formatted stuff to the other writer.
//
// We implement write() to take format specs into account (width, padding, fill,
// specs for numeric arguments, etc.) but we provide write_no_specs(...) which
// outputs values directly to the writer. This means that you can use this to
// convert floats, integers, to strings by calling just write_no_specs().
//
// We also store a parse context (if a format string was passed), otherwise it
// remains unused.
struct fmt_context : writer
{
  writer *Out; // The real output

  // Holds the format string (and how much we've parsed)
  // and some state about the argument ids (when using automatic indexing).
  fmt_parse_context Parse;

  array<fmt_arg> Args;

  // null if no specs were parsed.
  // When writing a custom formatter use this for checking specifiers.
  // e.g.
  //     if (f->Specs && f->Specs->Hash) { ... }
  //
  // These are "dynamic" format specs because width or precision might have
  // been specified by another argument (instead of being a literal in the
  // format string).
  fmt_dynamic_specs *Specs = null;

  fmt_context(writer *out, string fmtString, array<fmt_arg> args)
      : Out(out), Parse(fmtString), Args(args) {}

  void write(const char *data, s64 count) override;
  void flush() override { Out->flush(); }

  // The position tells where to point the caret in the format string, so it is
  // clear where exactly the error happened. If left as -1 we calculate using the
  // current Parse.It.
  //
  // The only reason we may want to pass an explicit position is if we are in the
  // middle of parsing and parse.It is not pointing at the right place.
  //
  // This routine is used to provide useful error messages.
  void on_error(string message, s64 position = -1)
  {
    Parse.on_error(message, position);
  }
};

inline void write(fmt_context *f, string s) { f->write(s.Data, s.Count); }

// General formatting routines which take specifiers into account:
void write(fmt_context *f, is_integral auto value);
void write(fmt_context *f, is_floating_point auto value);
void write(fmt_context *f, bool value);
void write(fmt_context *f, const void *value);

// These routines write the value directly, without looking at formatting specs.
// Useful when writing a custom formatter and there were specifiers but they
// shouldn't propagate downwards when printing simpler types.
void write_no_specs(fmt_context *f, is_integral auto value);
void write_no_specs(fmt_context *f, is_floating_point auto value);
void write_no_specs(fmt_context *f, bool value);
void write_no_specs(fmt_context *f, const void *value);

inline void write_no_specs(fmt_context *f, string str) { write(f->Out, str); }
inline void write_no_specs(fmt_context *f, const char *str)
{
  write(f->Out, str, c_string_byte_count(str));
}

inline void write_no_specs(fmt_context *f, const char *str, s64 size)
{
  write(f->Out, str, size);
}

inline void write_no_specs(fmt_context *f, code_point cp) { write(f->Out, cp); }

fmt_dynamic_specs fmt_forwarded_specs_for_arg(fmt_dynamic_specs original, fmt_arg ar);

// Write with spec forwarding (implemented in .cpp)
void write_with_forwarding(fmt_context *F, fmt_arg ar, bool noSpecs);
void write_with_forwarding_pretty(fmt_context *F, fmt_arg ar, bool noSpecs, s32 indentSize, s32 nextLevel);

// POD field structs for struct/table formatting
struct fmt_struct_field
{
  string Name;
  fmt_arg Arg;
};

struct fmt_kv_entry
{
  fmt_arg Key;
  fmt_arg Value;
};

// Free-format functions (implemented in .cpp)
void write_struct(fmt_context *F, string name, array<fmt_struct_field> fields, bool noSpecs = false);
void write_tuple(fmt_context *F, string name, array<fmt_arg> fields, bool noSpecs = false);
void write_list(fmt_context *F, const array<fmt_arg> &items, bool noSpecs = false);
void write_table(fmt_context *F, array<fmt_kv_entry> entries, bool noSpecs = false, bool pretty = false, s32 indentSize = 0, s32 currentLevel = 0);

void write_list(fmt_context *F, any_array_like auto items, bool noSpecs = false)
{
  array<fmt_arg> args;
  defer(free(args));
  For(items) add(args, fmt_arg_make(it));
  write_list(F, args, noSpecs);
}

// Used to dispatch values to write/write_no_specs functions. Used in
// conjunction with fmt_arg_visit.
struct fmt_context_visitor
{
  fmt_context *F;
  bool NoSpecs;

  fmt_context_visitor(fmt_context *f, bool noSpecs = false)
      : F(f), NoSpecs(noSpecs) {}

  void operator()(s32 value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(u32 value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(s64 value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(u64 value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(bool value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(f32 value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(f64 value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(string value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(const void *value)
  {
    NoSpecs ? write_no_specs(F, value) : write(F, value);
  }
  void operator()(fmt_value::custom_value custom)
  {
    custom.FormatFunc(F, custom.Data);
  }

  void operator()(unused)
  {
    F->on_error("Internal error while formatting");
    assert(false);
  }
};

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
        F->on_error("Negative width");
        return (u32)-1;
      }
      else if ((u64)value > numeric<s32>::max())
      {
        F->on_error("Width value is too big");
        return (u32)-1;
      }
      return (u32)value;
    }
    else
    {
      F->on_error("Width was not an integer");
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
        F->on_error("Negative precision");
        return -1;
      }
      else if ((u64)value > numeric<s32>::max())
      {
        F->on_error("Precision value is too big");
        return -1;
      }
      return (s32)value;
    }
    else
    {
      F->on_error("Precision was not an integer");
      return -1;
    }
  }
};

// Expects a valid fmt_context (take a look in the implementation of
// fmt_to_writer). Main function that does the parsing and formatting.
void fmt_parse_and_format(fmt_context *f);

template <typename... Args>
void fmt_to_writer(writer *out, string fmtString, Args no_copy... arguments)
{
  static const s64 NUM_ARGS = sizeof...(Args);
  stack_array<fmt_arg, NUM_ARGS> args;

  args = {fmt_arg_make(arguments)...};
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
    fmt_arg arg = fmt_arg_make(value);
    fmt_arg_visit(fmt_context_visitor(f), arg);
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
      write_list(f, a);
      // Restore (not strictly necessary here) and close
      f->Specs = original_specs;
      write_no_specs(f, " }");
    }
    else
    {
      write_list(f, a);
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
      write_list(f, a);
      // Restore and close
      f->Specs = original_specs;
      write_no_specs(f, " }");
    }
    else
    {
      // Normal format: [...]
      write_list(f, a);
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
      v.visit([f, original_specs](const auto &value) {
        using ValueType = decay_t<decltype(value)>;
        if constexpr (!is_same<ValueType, typename variant<MEMBERS...>::nil>) {
          if (original_specs) {
            auto forwarded_specs = fmt_forwarded_specs_for_arg(*original_specs, fmt_arg_make(value));
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

      array<fmt_kv_entry> entries;
      // Need to cast away const to iterate since hash table iterators expect non-const
      auto &mutable_table = const_cast<hash_table<K, V> &>(table);
      for (auto [key, value] : mutable_table)
        add(entries, fmt_kv_entry{fmt_arg_make(*key), fmt_arg_make(*value)});
      // Restore specs so table entries get forwarded specs (including pretty)
      f->Specs = original_specs;
      write_table(f, entries, /*noSpecs=*/false, use_pretty, indent_size, current_level);
      free(entries);

      write_no_specs(f, " }");
    }
    else
    {
      // Default format: displays as a simple table
      // e.g. { "key1": 1, "key2": 2, "key3": 3 }
      array<fmt_kv_entry> entries;
      auto &mutable_table = const_cast<hash_table<K, V> &>(table);
      for (auto [key, value] : mutable_table)
        add(entries, fmt_kv_entry{fmt_arg_make(*key), fmt_arg_make(*value)});
      write_table(f, entries, /*noSpecs=*/false, use_pretty, indent_size, current_level);
      free(entries);
    }
  }
};

// Formatters for linked list views
template <typename Node>
  requires(singly_linked_node_like<Node> && !doubly_linked_node_like<Node>)
struct formatter<Node *>
{
  void format(Node *no_copy v, fmt_context *f)
  {
    bool use_debug = f->Specs && f->Specs->Hash;

    // Collect node values into fmt_args so we can reuse format_list
    array<fmt_arg> items;
    for (auto p = v; p; p = p->Next)
      add(items, fmt_arg_make(*p));

    if (use_debug)
    {
      auto *orig = f->Specs;
      f->Specs = nullptr;

      write_no_specs(f, "<singly_linked_list_like> { count: ");
      format_value(items.Count, f);
      write_no_specs(f, ", data: ");

      // Restore specs for elements
      // When only precision is specified with no type, prefer fixed-point for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0)
      {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      }
      else
      {
        f->Specs = orig;
      }

      write_list(f, items);

      // Restore and close
      f->Specs = orig;
      write_no_specs(f, " }");
    }
    else
    {
      auto *orig = f->Specs;
      // Coerce precision-only to fixed for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0)
      {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      }
      write_list(f, items);
      f->Specs = orig;
    }

    free(items);
  }
};

template <typename Node>
  requires(doubly_linked_node_like<Node>)
struct formatter<Node *>
{
  void format(const Node *no_copy v, fmt_context *f)
  {
    bool use_debug = f->Specs && f->Specs->Hash;

    // Collect node values into fmt_args so we can reuse format_list
    array<fmt_arg> items;
    for (auto p = v; p; p = p->Next)
      add(items, fmt_arg_make(*p));

    if (use_debug)
    {
      auto *orig = f->Specs;
      write_no_specs(f, "<doubly_linked_list_like> { count: ");
      f->Specs = nullptr;
      format_value(items.Count, f);
      write_no_specs(f, ", data: ");
      // Restore specs for elements
      // When only precision is specified with no type, prefer fixed-point for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0)
      {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      }
      else
      {
        f->Specs = orig;
      }

      write_list(f, items);

      // Restore and close
      f->Specs = orig;
      write_no_specs(f, " }");
    }
    else
    {
      auto *orig = f->Specs;
      // Coerce precision-only to fixed for floats inside lists
      fmt_dynamic_specs coerced;
      if (orig && orig->Type == 0 && orig->Precision >= 0)
      {
        coerced = *orig;
        coerced.Type = 'f';
        f->Specs = &coerced;
      }
      write_list(f, items);
      f->Specs = orig;
    }

    free(items);
  }
};

LSTD_END_NAMESPACE
