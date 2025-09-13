#include "lstd/parse.h"
#include "lstd/context.h"
#include "lstd/fmt.h"

LSTD_BEGIN_NAMESPACE

enum emphasis : u8
{
  BOLD = BIT(0),
  ITALIC = BIT(1),
  UNDERLINE = BIT(2),
  STRIKETHROUGH = BIT(3)
};

// Colors are defined all-uppercase.
// Terminal colors are meant to be used if the output console doesn't support
// true color. There are 8 colors with additional 8 bright versions.
enum class terminal_color : u32
{
#define COLOR_DEF(x, y) x = y,
#include "terminal_colors.inl"
#undef COLOR_DEF
};

// Colors are defined all-uppercase.
// This enum contains names for popular colors and values for them in hex.
enum class color : u32
{
#define COLOR_DEF(x, y) x = y,
#include "colors.inl"
#undef COLOR_DEF
};

inline string color_to_string(color c)
{
  switch (c)
  {
#define COLOR_DEF(x, y) \
  case color::x:        \
    return string(#x);
#include "colors.inl"
#undef COLOR_DEF
  default:
    return "UNKNOWN";
  }
}

// Colors are defined all-uppercase and this function is case-sensitive
//   e.g. cornflower_blue doesn't return color::CORNFLOWER_BLUE
// Returns color::NONE (with value of black) if not found.
inline color string_to_color(string str)
{
#define COLOR_DEF(x, y)               \
  if (strings_match(str, string(#x))) \
    return color::x;
#include "colors.inl"
#undef COLOR_DEF
  return color::NONE;
}

inline string terminal_color_to_string(terminal_color c)
{
  switch (c)
  {
#define COLOR_DEF(x, y)   \
  case terminal_color::x: \
    return string(#x);
#include "terminal_colors.inl"
#undef COLOR_DEF
  default:
    return "NONE";
  }
}

// Colors are defined all-uppercase and this function is case-sensitive
//   e.g. bright_black doesn't return color::BRIGHT_BLACK
// Returns terminal_color::NONE (invalid) if not found.
inline terminal_color string_to_terminal_color(string str)
{
#define COLOR_DEF(x, y)               \
  if (strings_match(str, string(#x))) \
    return terminal_color::x;
#include "terminal_colors.inl"
#undef COLOR_DEF
  return terminal_color::NONE;
}

// Used when making ANSI escape codes for text styles
inline char *u8_to_esc(char *p, char delimiter, u8 c)
{
  *p++ = '0' + c / 100;
  *p++ = '0' + c / 10 % 10;
  *p++ = '0' + c % 10;
  *p++ = delimiter;
  return p;
}

struct fmt_text_style
{
  enum class color_kind
  {
    NONE = 0,
    RGB,
    TERMINAL
  };

  color_kind ColorKind = color_kind::NONE;
  union
  {
    u32 RGB = 0;
    terminal_color Terminal;
  } Color{};

  bool Background = false;
  u8 Emphasis = 0;
};

inline char *color_to_ansi(char *buffer, fmt_text_style style)
{
  char *p = buffer;
  if (style.ColorKind != fmt_text_style::color_kind::NONE)
  {
    if (style.ColorKind == fmt_text_style::color_kind::TERMINAL)
    {
      // Background terminal colors are 10 more than the foreground ones
      u32 value = (u32)style.Color.Terminal + (style.Background ? 10 : 0);

      *p++ = '\x1b';
      *p++ = '[';

      if (value >= 100)
      {
        *p++ = '1';
        value %= 100;
      }
      *p++ = '0' + value / 10;
      *p++ = '0' + value % 10;

      *p++ = 'm';
    }
    else
    {
      memcpy(p, style.Background ? "\x1b[48;2;" : "\x1b[38;2;", 7);
      p += 7;

      p = u8_to_esc(p, ';', (u8)((style.Color.RGB >> 16) & 0xFF));
      p = u8_to_esc(p, ';', (u8)((style.Color.RGB >> 8) & 0xFF));
      p = u8_to_esc(p, 'm', (u8)((style.Color.RGB) & 0xFF));
    }
  }
  else if ((u8)style.Emphasis == 0)
  {
    // Empty text style means "reset"
    memcpy(p, "\x1b[0m", 4);
    p += 4;
  }
  return p;
}

inline char *emphasis_to_ansi(char *buffer, u8 emphasis)
{
  u8 codes[4] = {};
  if (emphasis & BOLD)
    codes[0] = 1;
  if (emphasis & ITALIC)
    codes[1] = 3;
  if (emphasis & UNDERLINE)
    codes[2] = 4;
  if (emphasis & STRIKETHROUGH)
    codes[3] = 9;

  char *p = buffer;
  For(range(4))
  {
    if (!codes[it])
      continue;

    *p++ = '\x1b';
    *p++ = '[';
    *p++ = '0' + codes[it];
    *p++ = 'm';
  }
  return p;
}

bool handle_emphasis(fmt_parse_context *p, fmt_text_style *textStyle)
{
  while (p->It.Count && ascii_is_alpha(p->It[0]))
  {
    switch (p->It[0])
    {
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
      p->on_error("Invalid emphasis character - valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)");
      return false;
    }
    ++p->It.Data, --p->It.Count;
  }
  return true;
}

u32 parse_rgb_channel(fmt_parse_context *p, bool last)
{
  auto [channel, status, rest] =
      parse_int<u8, parse_int_options{.ParseSign = false,
                                      .LookForBasePrefix = true}>(p->It);

  if (status == PARSE_INVALID)
  {
    p->on_error("Invalid integer channel value",
                (const char *)rest.Data - p->FormatString.Data);
    return (u32)-1;
  }

  if (status == PARSE_TOO_MANY_DIGITS)
  {
    p->on_error("Channel value too big - it must be in the range [0-255]",
                (const char *)rest.Data - p->FormatString.Data - 1);
    return (u32)-1;
  }

  p->It = string(rest);
  if (!p->It.Count)
    return (u32)-1;

  if (!last)
  {
    if (p->It[0] != ';')
    {
      p->on_error("\";\" expected followed by the next channel value");
      return (u32)-1;
    }
    if (p->It[0] == '}' || p->It.Count < 2 || !ascii_is_digit(*(p->It.Data + 1)))
    {
      p->on_error("Expected an integer specifying a channel value (3 channels required)", p->It.Data - p->FormatString.Data + 1);
      return (u32)-1;
    }
  }
  else
  {
    if (p->It[0] != '}' && p->It[0] != ';')
    {
      p->on_error("\"}\" expected (or \";\" for BG specifier or emphasis)");
      return (u32)-1;
    }
  }
  return channel;
}

inline fmt_arg fmt_get_arg_from_index(fmt_context *f, s64 index)
{
  if (index >= f->Args.Count)
  {
    f->on_error("Argument index out of range");
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
      f->Specs->Width = fmt_arg_visit(fmt_width_checker{f}, width);
      if (f->Specs->Width == (u32)-1)
        return false;
    }
  }
  if (f->Specs->PrecisionIndex != -1)
  {
    auto precision = fmt_get_arg_from_index(f, f->Specs->PrecisionIndex);
    if (precision.Type != fmt_type::NONE)
    {
      f->Specs->Precision = fmt_arg_visit(fmt_precision_checker{f}, precision);
      if (f->Specs->Precision == numeric<s32>::min())
        return false;
    }
  }

  return true;
}

struct fmt_parse_text_style_result
{
  bool Success;
  fmt_text_style TextStyle;
};

inline bool ascii_is_identifier_start(char x) { return ascii_is_alpha(x) || x == '_'; }

fmt_parse_text_style_result fmt_parse_text_style(fmt_parse_context *p)
{
  fmt_text_style textStyle = {};

  if (ascii_is_alpha(p->It[0]))
  {
    bool terminal = false;
    if (p->It[0] == 't')
    {
      terminal = true;
      ++p->It.Data, --p->It.Count;
    }

    const char *it = p->It.Data;
    s64 n = p->It.Count;
    do
    {
      ++it, --n;
    } while (n && ascii_is_identifier_start(*it));
    if (!n)
      return {true, textStyle};

    auto name = string(p->It.Data, it - p->It.Data);
    p->It = string(it, n);

    if (p->It[0] != ';' && p->It[0] != '}')
    {
      p->on_error("Invalid color name - it must be a valid identifier (without digits)");
      return {false, {}};
    }

    if (terminal)
    {
      terminal_color c = string_to_terminal_color(name);
      if (c == terminal_color::NONE)
      {
        p->It.Data -= name.Count, p->It.Count += name.Count;
        if (!handle_emphasis(p, &textStyle))
          return {false, {}};
        return {true, textStyle};
      }
      textStyle.ColorKind = fmt_text_style::color_kind::TERMINAL;
      textStyle.Color.Terminal = c;
    }
    else
    {
      color c = string_to_color(name);
      if (c == color::NONE)
      {
        p->It.Data -= name.Count, p->It.Count += name.Count;
        if (!handle_emphasis(p, &textStyle))
          return {false, {}};
        return {true, textStyle};
      }
      textStyle.ColorKind = fmt_text_style::color_kind::RGB;
      textStyle.Color.RGB = (u32)c;
    }
  }
  else if (ascii_is_digit(p->It[0]))
  {
    u32 r = parse_rgb_channel(p, false);
    if (r == (u32)-1)
      return {false, {}};
    ++p->It.Data, --p->It.Count;
    u32 g = parse_rgb_channel(p, false);
    if (g == (u32)-1)
      return {false, {}};
    ++p->It.Data, --p->It.Count;
    u32 b = parse_rgb_channel(p, true);
    if (b == (u32)-1)
      return {false, {}};
    textStyle.ColorKind = fmt_text_style::color_kind::RGB;
    textStyle.Color.RGB = (r << 16) | (g << 8) | b;
  }
  else if (p->It[0] == '#')
  {
    assert(false && "Parse #ffffff rgb color");
  }
  else if (p->It[0] == '}')
  {
    return {true, textStyle};
  }

  if (p->It[0] == ';')
  {
    ++p->It.Data, --p->It.Count;
    if (p->It.Count > 2)
    {
      if (strings_match(string(p->It.Data, 2), "BG"))
      {
        if (textStyle.ColorKind == fmt_text_style::color_kind::NONE)
        {
          p->on_error("Color specified as background but there was no color parsed");
          return {false, {}};
        }
        textStyle.Background = true;
        p->It.Data += 2, p->It.Count -= 2;
        return {true, textStyle};
      }
    }
    if (!handle_emphasis(p, &textStyle))
      return {false, {}};
  }
  return {true, textStyle};
}

inline void fmt_parse_and_format(fmt_context *f)
{
  fmt_parse_context *p = &f->Parse;

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
        f->on_error(
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
      f->on_error("Invalid format string");
      return;
    }
    if (p->It[0] == '}')
    {
      // Implicit {} means "get the next argument"
      currentArg = fmt_get_arg_from_index(f, p->next_arg_id());
      if (currentArg.Type == fmt_type::NONE)
        return; // The error was reported in _f->get_arg_from_ref_

      fmt_arg_visit(fmt_context_visitor(f), currentArg);
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
        f->on_error("\"}\" expected");
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
        fmt_arg_visit(fmt_context_visitor(f), currentArg);
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
          f->on_error("\"}\" expected");
          return;
        }

        f->Specs = &specs;
        success = fmt_handle_dynamic_specs(f);
        if (!success)
          return;

        fmt_arg_visit(fmt_context_visitor(f), currentArg);

        f->Specs = null;
      }
      else
      {
        f->on_error("\"}\" expected");
        return;
      }
    }
    ++p->It.Data, --p->It.Count; // Go to the next byte
  }
}

fmt_dynamic_specs create_safe_specs(const fmt_dynamic_specs &original)
{
  auto safe_specs = original;
  safe_specs.Type = 0;
  safe_specs.Precision = -1;
  safe_specs.Fill = ' ';
  safe_specs.Width = 0;
  safe_specs.UserData = 0;
  return safe_specs;
}

fmt_dynamic_specs create_forwarded_specs(const fmt_dynamic_specs &original)
{
  auto forwarded = original;
  forwarded.Fill = ' ';
  forwarded.Width = 0;
  forwarded.UserData = 0;
  return forwarded;
}

inline bool type_selector_compatible(char typeChar, fmt_type k)
{
  if (typeChar == 0)
    return true;
  switch (k)
  {
  case fmt_type::F32:
  case fmt_type::F64:
    return typeChar == 'f' || typeChar == 'F' || typeChar == 'g' || typeChar == 'G' || typeChar == 'e' || typeChar == 'E' || typeChar == '%';
  case fmt_type::S64:
  case fmt_type::U64:
  case fmt_type::BOOL:
    return typeChar == 'd' || typeChar == 'x' || typeChar == 'X' || typeChar == 'o' || typeChar == 'b' || typeChar == 'B' || typeChar == 'c' || typeChar == 'n';
  case fmt_type::STRING:
    return typeChar == 's' || typeChar == 'q' || typeChar == 'p';
  case fmt_type::POINTER:
    return typeChar == 'p';
  case fmt_type::CUSTOM:
    return true;
  default:
    return false;
  }
}

fmt_dynamic_specs fmt_forwarded_specs_for_arg(fmt_dynamic_specs original, fmt_arg ar)
{
  if (ar.Type == fmt_type::CUSTOM)
    return create_forwarded_specs(original);
  if (type_selector_compatible(original.Type, ar.Type))
    return create_forwarded_specs(original);
  return create_safe_specs(original);
}

void write_arg_with_forwarding(fmt_context *F, fmt_arg ar, bool noSpecs)
{
  fmt_dynamic_specs *original = F->Specs;
  if (original)
  {
    auto sp = [&]() -> fmt_dynamic_specs
    {
      if (ar.Type == fmt_type::CUSTOM)
        return create_forwarded_specs(*original);
      return type_selector_compatible(original->Type, ar.Type) ? create_forwarded_specs(*original) : create_safe_specs(*original);
    }();
    F->Specs = &sp;
    fmt_arg_visit(fmt_context_visitor(F, noSpecs), ar);
  }
  else
  {
    F->Specs = nullptr;
    fmt_arg_visit(fmt_context_visitor(F, noSpecs), ar);
  }
  F->Specs = original;
}

void write_arg_with_forwarding_pretty(fmt_context *F, fmt_arg ar, bool noSpecs, s32 indentSize, s32 nextLevel)
{
  fmt_dynamic_specs *original = F->Specs;
  if (original)
  {
    auto sp = [&]() -> fmt_dynamic_specs
    {
      auto t = (ar.Type == fmt_type::CUSTOM || type_selector_compatible(original->Type, ar.Type))
                   ? create_forwarded_specs(*original)
                   : create_safe_specs(*original);
      t.UserData = nextLevel;
      t.Width = indentSize;
      return t;
    }();
    F->Specs = &sp;
    fmt_arg_visit(fmt_context_visitor(F, noSpecs), ar);
  }
  else
  {
    F->Specs = nullptr;
    fmt_arg_visit(fmt_context_visitor(F, noSpecs), ar);
  }
  F->Specs = original;
}

void write_struct(fmt_context *F, string name, array<fmt_struct_field> fields, bool noSpecs)
{
  write_no_specs(F, name);
  write_no_specs(F, " {");
  auto *p = fields.Data;
  auto *end = fields.Data + fields.Count;
  if (p != end)
  {
    write_no_specs(F, " ");
    write_no_specs(F, p->Name);
    write_no_specs(F, ": ");
    write_arg_with_forwarding(F, p->Arg, noSpecs);
    ++p;
    while (p != end)
    {
      write_no_specs(F, ", ");
      write_no_specs(F, p->Name);
      write_no_specs(F, ": ");
      write_arg_with_forwarding(F, p->Arg, noSpecs);
      ++p;
    }
    write_no_specs(F, " ");
  }
  write_no_specs(F, "}");
}

void write_tuple(fmt_context *F, string name, array<fmt_arg> fields, bool noSpecs)
{
  write_no_specs(F, name);
  write_no_specs(F, "(");
  auto *p = fields.Data;
  auto *end = fields.Data + fields.Count;
  if (p != end)
  {
    write_arg_with_forwarding(F, *p, noSpecs);
    ++p;
    while (p != end)
    {
      write_no_specs(F, ", ");
      write_arg_with_forwarding(F, *p, noSpecs);
      ++p;
    }
  }
  write_no_specs(F, ")");
}

void write_list(fmt_context *F, const array<fmt_arg> &items, bool noSpecs)
{
  write_no_specs(F, "[");
  auto *p = items.Data;
  auto *end = items.Data + items.Count;
  if (p != end)
  {
    write_arg_with_forwarding(F, *p, noSpecs);
    ++p;
    while (p != end)
    {
      write_no_specs(F, ", ");
      write_arg_with_forwarding(F, *p, noSpecs);
      ++p;
    }
  }
  write_no_specs(F, "]");
}

void write_table(fmt_context *F, array<fmt_kv_entry> entries, bool noSpecs, bool pretty, s32 indentSize, s32 currentLevel)
{
  if (!pretty)
  {
    write_no_specs(F, "{");
    auto *p = entries.Data;
    auto *end = entries.Data + entries.Count;
    if (p != end)
    {
      write_no_specs(F, " ");
      write_arg_with_forwarding(F, p->Key, noSpecs);
      write_no_specs(F, ": ");
      write_arg_with_forwarding(F, p->Value, noSpecs);
      ++p;
      while (p != end)
      {
        write_no_specs(F, ", ");
        write_arg_with_forwarding(F, p->Key, noSpecs);
        write_no_specs(F, ": ");
        write_arg_with_forwarding(F, p->Value, noSpecs);
        ++p;
      }
      write_no_specs(F, " ");
    }
    write_no_specs(F, "}");
    return;
  }

  if (entries.Count == 0)
  {
    write_no_specs(F, "{}");
    return;
  }
  write_no_specs(F, "{\n");
  auto *p = entries.Data;
  auto *end = entries.Data + entries.Count;
  bool first = true;
  while (p != end)
  {
    if (!first)
      write_no_specs(F, ",\n");
    first = false;
    for (s32 i = 0; i < (currentLevel + 1) * indentSize; ++i)
      write_no_specs(F, " ");
    write_arg_with_forwarding(F, p->Key, noSpecs);
    write_no_specs(F, ": ");
    write_arg_with_forwarding_pretty(F, p->Value, noSpecs, indentSize, currentLevel + 1);
    ++p;
  }
  write_no_specs(F, "\n");
  for (s32 i = 0; i < currentLevel * indentSize; ++i)
    write_no_specs(F, " ");
  write_no_specs(F, "}");
}

bool fmt_parse_context::check_arg_id(u32)
{
  if (NextArgID > 0)
  {
    on_error("Cannot switch from automatic to manual argument indexing");
    return false;
  }
  NextArgID = -1;
  return true;
}

u32 fmt_parse_context::next_arg_id()
{
  if (NextArgID >= 0)
    return (u32)NextArgID++;
  on_error("Cannot switch from manual to automatic argument indexing");
  return 0;
}

// Some specifiers require numeric arguments and we do error checking,
// CUSTOM arguments don't get checked
void fmt_parse_context::require_arithmetic_arg(fmt_type argType, s64 errorPosition)
{
  assert(argType != fmt_type::NONE);
  if (argType == fmt_type::CUSTOM)
    return;
  if (!fmt_type_is_arithmetic(argType))
    on_error("Format specifier requires an arithmetic argument", errorPosition);
}

// Some specifiers require signed numeric arguments and we do error
// checking, CUSTOM arguments don't get checked
void fmt_parse_context::require_signed_arithmetic_arg(fmt_type argType, s64 errorPosition)
{
  assert(argType != fmt_type::NONE);
  if (argType == fmt_type::CUSTOM)
    return;

  require_arithmetic_arg(argType, errorPosition);
  if (fmt_type_is_integral(argType) && argType != fmt_type::S64)
  {
    on_error("Format specifier requires a signed integer argument (got unsigned)", errorPosition);
  }
}

// Integer values and pointers aren't allowed to get precision. CUSTOM
// argument is again, not checked.
void fmt_parse_context::check_precision_for_arg(fmt_type argType, s64 errorPosition)
{
  assert(argType != fmt_type::NONE);
  if (argType == fmt_type::CUSTOM)
    return;
  if (fmt_type_is_integral(argType))
  {
    on_error("Precision is not allowed for integer types", errorPosition);
  }
  if (argType == fmt_type::POINTER)
  {
    on_error("Precision is not allowed for pointer type", errorPosition);
  }
}

void fmt_parse_context::on_error(string message, s64 position)
{
  if (position == -1)
    position = It.Data - FormatString.Data;
  Context.FmtParseErrorHandler(message, FormatString, position);
}

s64 fmt_parse_arg_id(fmt_parse_context *p)
{
  code_point ch = p->It[0];
  if (ch == '}' || ch == ':')
    return p->next_arg_id();

  if (ascii_is_digit(ch))
  {
    auto [value, status, rest] = parse_int<u32, parse_int_options{.ParseSign = false}>(p->It, 10);
    p->It = string(rest);

    if (status == PARSE_TOO_MANY_DIGITS)
    {
      p->on_error("Argument index is an integer which is too large");
      return -1;
    }
    if (!p->It.Count)
    {
      p->on_error("Format string ended abruptly");
      return -1;
    }

    ch = p->It[0];
    if ((ch != '}' && ch != ':'))
    {
      p->on_error("Expected \":\" or \"}\"");
      return -1;
    }

    p->check_arg_id(value);
    return (s64)value;
  }
  p->on_error("Expected a number - an index to an argument");
  return -1;
}

bool parse_fill_and_align(fmt_parse_context *p, fmt_type argType, fmt_specs *specs)
{
  code_point fill = p->It[0];
  string rest = slice(p->It, 1, length(p->It));

  auto align = get_alignment_from_char(fill);
  if (align == fmt_alignment::NONE)
  {
    if (!rest.Count)
      return true;
    align = get_alignment_from_char(rest[0]);
    advance_bytes(&rest, 1);
  }
  else
  {
    fill = ' ';
  }

  if (align != fmt_alignment::NONE)
  {
    s64 errorPosition = (const char *)rest.Data - p->FormatString.Data;
    if (fill == '{')
    {
      p->on_error("Invalid fill character \"{\"", errorPosition - 2);
      return false;
    }
    if (fill == '}')
    {
      p->on_error("Invalid fill character \"}\"", errorPosition - 2);
      return false;
    }

    p->It = rest;
    specs->Fill = fill;
    specs->Align = align;
    if (align == fmt_alignment::NUMERIC)
      p->require_arithmetic_arg(argType);
  }
  return true;
}

bool parse_width(fmt_parse_context *p, fmt_dynamic_specs *specs)
{
  if (ascii_is_digit(p->It[0]))
  {
    auto [value, status, rest] = parse_int<u32, parse_int_options{.ParseSign = false}>(p->It, 10);
    p->It = string(rest);
    specs->Width = value;
    if (status == PARSE_TOO_MANY_DIGITS)
    {
      p->on_error("We parsed an integer width which was too large");
      return {};
    }
    if (specs->Width == (u32)-1)
      return false;
  }
  else if (p->It[0] == '{')
  {
    ++p->It.Data, --p->It.Count;
    if (p->It.Count)
    {
      specs->WidthIndex = fmt_parse_arg_id(p);
      if (specs->WidthIndex == -1)
        return false;
    }
    if (!p->It.Count || p->It[0] != '}')
    {
      p->on_error("Expected a closing \"}\" after parsing an argument ID for a dynamic width");
      return false;
    }
    ++p->It.Data, --p->It.Count;
  }
  return true;
}

bool parse_precision(fmt_parse_context *p, fmt_type argType, fmt_dynamic_specs *specs)
{
  ++p->It.Data, --p->It.Count;
  if (!p->It.Count)
  {
    p->on_error("Missing precision specifier (we parsed a dot but nothing valid after that)");
    return false;
  }

  if (ascii_is_digit(p->It[0]))
  {
    auto [value, status, rest] = parse_int<u32, parse_int_options{.ParseSign = false}>(p->It, 10);
    p->It = string(rest);
    specs->Precision = value;
    if (status == PARSE_TOO_MANY_DIGITS)
    {
      p->on_error("We parsed an integer precision which was too large");
      return {};
    }
    if (specs->Precision == (u32)-1)
      return false;
  }
  else if (p->It[0] == '{')
  {
    ++p->It.Data, --p->It.Count;
    if (p->It.Count)
    {
      specs->PrecisionIndex = fmt_parse_arg_id(p);
      if (specs->PrecisionIndex == -1)
        return false;
    }
    if (!p->It.Count || p->It[0] != '}')
    {
      p->on_error("Expected a closing \"}\" after parsing an argument ID for a dynamic precision");
      return false;
    }
    ++p->It.Data, --p->It.Count;
  }
  else
  {
    p->on_error("Missing precision specifier (we parsed a dot but nothing valid after that)");
    return false;
  }
  p->check_precision_for_arg(argType, p->It.Data - p->FormatString.Data - 1);
  return true;
}

bool fmt_parse_specs(fmt_parse_context *p, fmt_type argType, fmt_dynamic_specs *specs)
{
  if (p->It[0] == '}')
    return true;

  if (!parse_fill_and_align(p, argType, specs))
    return false;
  if (!p->It.Count)
    return true;

  switch (p->It[0])
  {
  case '+':
    p->require_signed_arithmetic_arg(argType);
    specs->Sign = fmt_sign::PLUS;
    ++p->It.Data, --p->It.Count;
    break;
  case '-':
    p->require_signed_arithmetic_arg(argType);
    specs->Sign = fmt_sign::MINUS;
    ++p->It.Data, --p->It.Count;
    break;
  case ' ':
    p->require_signed_arithmetic_arg(argType);
    specs->Sign = fmt_sign::SPACE;
    ++p->It.Data, --p->It.Count;
    break;
  }
  if (!p->It.Count)
    return true;

  if (p->It[0] == '#')
  {
    p->require_arithmetic_arg(argType);
    specs->Hash = true;
    ++p->It.Data, --p->It.Count;
    if (!p->It.Count)
      return true;
  }

  if (p->It[0] == '0')
  {
    p->require_arithmetic_arg(argType);
    specs->Align = fmt_alignment::NUMERIC;
    specs->Fill = '0';
    ++p->It.Data, --p->It.Count;
    if (!p->It.Count)
      return true;
  }

  if (!parse_width(p, specs))
    return false;
  if (!p->It.Count)
    return true;

  if (p->It[0] == '.')
  {
    if (!parse_precision(p, argType, specs))
      return false;
  }

  if (p->It.Count && p->It[0] != '}')
  {
    specs->Type = (char)p->It[0];
    ++p->It.Data, --p->It.Count;
  }
  return true;
}

LSTD_END_NAMESPACE
