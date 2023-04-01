#pragma once

#include "lstd/common.h"
#include "lstd/fmt.h"

LSTD_BEGIN_NAMESPACE

// Used for generating unique ids
struct guid {
  u8 Data[16];
  static const s64 Count = 16;

  // By default the guid is zero
  guid() { For(range(16)) Data[it] = 0; }

  explicit operator bool() {
    For(range(16)) if (Data[it]) return true;
    return false;
  }
};

u64 get_hash(guid value) {
  u64 hash = 5381;
  For(value.Data) hash = (hash << 5) + hash + it;
  return hash;
}

//
// Provides write_custom implementation for GUIDs for the lstd.fmt module.
//
// Formats GUID in the following way: 00000000-0000-0000-0000-000000000000
// Allows specifiers:
//   'n' - 00000000000000000000000000000000
//   'd' - 00000000-0000-0000-0000-000000000000
//   'b' - {00000000-0000-0000-0000-000000000000}
//   'p' - (00000000-0000-0000-0000-000000000000)
//   'x' - {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
// The default format is the same as 'd'.
//
//   'N' - Like 'n', but make the letters in the guid uppercase.
//   'D' - Uppercase version of 'd'
//   'B' - Uppercase version of 'b'
//   'P' - Uppercase version of 'p'
//   'X' - Uppercase version of 'x'
//
void write_custom(fmt_context* f, const guid* g) {
  char type = 'd';
  if (f->Specs) {
    type = f->Specs->Type;
  }

  bool upper = is_upper(type);
  type = (char)to_lower(type);

  if (type != 'n' && type != 'd' && type != 'b' && type != 'p' && type != 'x') {
    on_error(f, "Invalid type specifier for a guid",
             f->Parse.It.Data - f->Parse.FormatString.Data - 1);
    return;
  }

  code_point openParenthesis = 0, closedParenthesis = 0;
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
    auto* old = f->Specs;
    f->Specs = null;

    u8* p = (u8*)g->Data;
    if (upper) {
      fmt_to_writer(
          f,
          "{{{:#04X}{:02X}{:02X}{:02X},{:#04X}{:02X},{:#04X}{:02X},{{{:#04X},{:"
          "#04X},{:#04X},{:#04X},{:#04X},{:#04X},{:#04X},{:#04X}}}}}",
          p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10],
          p[11], p[12], p[13], p[14], p[15]);
    } else {
      fmt_to_writer(
          f,
          "{{{:#04x}{:02x}{:02x}{:02x},{:#04x}{:02x},{:#04x}{:02x},{{{:#04x},{:"
          "#04x},{:#04x},{:#04x},{:#04x},{:#04x},{:#04x},{:#04x}}}}}",
          p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10],
          p[11], p[12], p[13], p[14], p[15]);
    }

    f->Specs = old;
    return;
  }

  if (openParenthesis) write_no_specs(f, openParenthesis);

  auto* old = f->Specs;
  f->Specs = null;

  const byte* p = g->Data;
  For(range(16)) {
    if (hyphen && (it == 4 || it == 6 || it == 8 || it == 10)) {
      write_no_specs(f, (code_point)'-');
    }
    if (upper) {
      fmt_to_writer(f, "{:02X}", (u8)*p);
    } else {
      fmt_to_writer(f, "{:02x}", (u8)*p);
    }
    ++p;
  }
  f->Specs = old;

  if (closedParenthesis) write_no_specs(f, closedParenthesis);
}

struct parse_guid_options {
  // Do we handle formats starting with parentheses - ( or {.
  bool Parentheses = true;

  // Doesn't pay attention to the position or the number of hyphens in the
  // input, just ignores them. This makes parsing go faster when you don't care
  // if the input is partially incorrect or you know it is not!
  bool RelaxHyphens = false;
};

//
// Parses the following GUID representations:
// - 81a130d2502f4cf1a37663edeb000e9f
// - 81a130d2-502f-4cf1-a376-63edeb000e9f
// - {81a130d2-502f-4cf1-a376-63edeb000e9f}
// - (81a130d2-502f-4cf1-a376-63edeb000e9f)
// - {0x81a130d2,0x502f,0x4cf1,{0xa3,0x76,0x63,0xed,0xeb,0x00,0x0e,0x9f}}
//
// Doesn't pay attention to capitalization (both uppercase/lowercase/mixed are
// valid).
//
// Returns: the guid parsed, a status, and the rest of the buffer
//
template <parse_guid_options Options = parse_guid_options{}>
parse_result<guid> parse_guid(string buffer) {
  guid empty;

#undef FAIL
#define FAIL {empty, PARSE_INVALID, p}

  string p = buffer;
  if (!p.Count) return FAIL;

  bool parentheses = false, curly = false;
  if constexpr (Options.Parentheses) {
    if (p[0] == '(' || p[0] == '{') {
      parentheses = true;
      curly = p[0] == '{';
      advance_cp(&p, 1);
      if (!p.Count) return FAIL;
    }
  }

  guid result;

  if (p.Count - 1 && p[0] == '0') {
    // Parse following format:
    // {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
    // We choose that format if the first byte begins with 0x.
    if (p[1] == 'x' || p[1] == 'X') {
      if (!parentheses || !curly) {
        // In this case the error is that there is 0x but we didn't start with a
        // {
        return FAIL;
      }

      bool status;

      auto* resultBuffer = &result.Data[0];

#define EAT_HEX_BYTES(count)                 \
  For(range(count)) {                        \
    auto [value, status] = eat_hex_byte(&p); \
    if (!status) return FAIL;                \
    *resultBuffer++ = value;                 \
  }

#define EXPECT_SEQUENCE(sequence)                       \
  status = expect_sequence<true>(&p, string(sequence)); \
  if (!status) return FAIL;

      EXPECT_SEQUENCE("0x");
      EAT_HEX_BYTES(4);
      EXPECT_SEQUENCE(",0x");

      EAT_HEX_BYTES(2);
      EXPECT_SEQUENCE(",0x");

      EAT_HEX_BYTES(2);
      EXPECT_SEQUENCE(",{0x");

      For(range(7)) {
        EAT_HEX_BYTES(1);
        EXPECT_SEQUENCE(",0x");
      }
      EAT_HEX_BYTES(1);
      EXPECT_SEQUENCE("}}");

#undef EAT_HEX_BYTES
#undef EXPECT_SEQUENCE

      return {result, PARSE_SUCCESS, p};
    }
  }

  // In the case above we the format with 0x and the commas:
  //   {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
  //
  // Below we parse every other format:
  //   81a130d2502f4cf1a37663edeb000e9f
  //   81a130d2-502f-4cf1-a376-63edeb000e9f
  //   {81a130d2-502f-4cf1-a376-63edeb000e9f}
  //   (81a130d2-502f-4cf1-a376-63edeb000e9f)

  bool hyphens = false;

  auto* resultBuffer = &result.Data[0];

  s32 counter = 0;
  while (true) {
    // We expect hyphens at positions 4, 6, 8 and 10.
    // Unless Options.RelaxHyphens was specified.
    if constexpr (!Options.RelaxHyphens) {
      if (counter == 4) {
        if (!hyphens) {
          if (p[0] == '-') {
            hyphens = true;
            advance_cp(&p, 1);
            if (!p.Count) return FAIL;
          }
        }
      }

      if (hyphens && (counter == 6 || counter == 8 || counter == 10)) {
        if (p[0] == '-') {
          advance_cp(&p, 1);
          if (!p.Count) return FAIL;
        } else {
          return FAIL;
        }
      }
    } else {
      if (p[0] == '-') {
        advance_cp(&p, 1);
        if (!p.Count) return FAIL;
      }
    }

    auto [value, status] = eat_hex_byte(&p);
    if (!status) return FAIL;
    *resultBuffer++ = value;

    ++counter;

    // We have eaten 16 hex bytes.
    if (counter == 16) break;
  }

  // Expect a closing parenthesis
  if constexpr (Options.Parentheses) {
    if (parentheses) {
      bool status = expect_cp(&p, curly ? '}' : ')');
      if (!status) return FAIL;
    }
  }
  return {result, PARSE_SUCCESS, p};
}

#undef FAIL
#undef EAT_HEX_BYTES
#undef EXPECT_SEQUENCE

LSTD_END_NAMESPACE
