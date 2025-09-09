#pragma once

#include "lstd/xar.h"
#include "lstd/string.h"

arena_allocator_data ARENA_TOKEN_DATA;
#define ARENA_TOKEN (allocator{arena_allocator, &ARENA_TOKEN_DATA})

#define TKN2(x, y) (((y) << 8) | (x))
#define TKN3(x, y, z) (((z) << 16) | ((y) << 8) | (x))

enum token_type
{
    TOKEN_INVALID = 0,

    TOKEN_DOT = '.',
    TOKEN_COMMA = ',',

    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_TIMES = '*',
    TOKEN_SLASH = '/',
    TOKEN_PERCENT = '%',
    TOKEN_ASSIGN = '=',

    TOKEN_AND = '&',
    TOKEN_XOR = '^',
    TOKEN_OR = '|',

    TOKEN_HASH = '#',
    TOKEN_AT = '@',

    TOKEN_TILDE = '~',
    TOKEN_EXCLAMATION = '!',
    TOKEN_COLON = ':',
    TOKEN_SEMICOLON = ';',

    TOKEN_LESS = '<',
    TOKEN_GREATER = '>',

    TOKEN_BRACKET_OPEN = '[',
    TOKEN_BRACKET_CLOSE = ']',

    TOKEN_PAREN_OPEN = '(',
    TOKEN_PAREN_CLOSE = ')',

    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',

    TOKEN_STRING_SINGLE_QUOTE = '\'',
    TOKEN_STRING_DOUBLE_QUOTE = '\"',

    // L"hello"
    TOKEN_STRING_WIDE_SINGLE_QUOTE = '\'' + 256,
    TOKEN_STRING_WIDE_DOUBLE_QUOTE = '\"' + 256,

    TOKEN_IDENTIFIER = 256,
    TOKEN_INTEGER,
    TOKEN_FLOAT,

    TOKEN_TRIPLE_DOT = TKN3('.', '.', '.'),

    TOKEN_ARROW = TKN2('-', '>'),
    TOKEN_DOUBLE_HASH = TKN2('#', '#'),

    TOKEN_DOUBLE_AND = TKN2('&', '&'),
    TOKEN_DOUBLE_OR = TKN2('|', '|'),

    TOKEN_PLUS_EQUAL = TKN2('+', '='),
    TOKEN_MINUS_EQUAL = TKN2('-', '='),
    TOKEN_TIMES_EQUAL = TKN2('*', '='),
    TOKEN_SLASH_EQUAL = TKN2('/', '='),
    TOKEN_PERCENT_EQUAL = TKN2('%', '='),
    TOKEN_OR_EQUAL = TKN2('|', '='),
    TOKEN_AND_EQUAL = TKN2('&', '='),
    TOKEN_XOR_EQUAL = TKN2('^', '='),
    TOKEN_NOT_EQUAL = TKN2('!', '='),
    TOKEN_EQUALITY = TKN2('=', '='),
    TOKEN_GREATER_EQUAL = TKN2('>', '='),
    TOKEN_LESS_EQUAL = TKN2('<', '='),
    TOKEN_LEFT_SHIFT = TKN2('<', '<'),
    TOKEN_RIGHT_SHIFT = TKN2('>', '>'),

    TOKEN_LEFT_SHIFT_EQUAL = TKN3('<', '<', '='),
    TOKEN_RIGHT_SHIFT_EQUAL = TKN3('>', '>', '='),
    TOKEN_INCREMENT = TKN2('+', '+'),
    TOKEN_DECREMENT = TKN2('-', '-'),

    TOKEN_KW_auto = 0x10000000,
    TOKEN_KW_break,
    TOKEN_KW_case,
    TOKEN_KW_char,
    TOKEN_KW_const,
    TOKEN_KW_continue,
    TOKEN_KW_default,
    TOKEN_KW_do,
    TOKEN_KW_double,
    TOKEN_KW_else,
    TOKEN_KW_enum,
    TOKEN_KW_extern,
    TOKEN_KW_float,
    TOKEN_KW_for,
    TOKEN_KW_goto,
    TOKEN_KW_if,
    TOKEN_KW_inline,
    TOKEN_KW_int,
    TOKEN_KW_long,
    TOKEN_KW_register,
    TOKEN_KW_restrict,
    TOKEN_KW_return,
    TOKEN_KW_short,
    TOKEN_KW_signed,
    TOKEN_KW_sizeof,
    TOKEN_KW_static,
    TOKEN_KW_struct,
    TOKEN_KW_switch,
    TOKEN_KW_typedef,
    TOKEN_KW_union,
    TOKEN_KW_unsigned,
    TOKEN_KW_void,
    TOKEN_KW_volatile,
    TOKEN_KW_while,
    TOKEN_KW_Alignas,
    TOKEN_KW_Alignof,
    TOKEN_KW_Atomic,
    TOKEN_KW_Bool,
    TOKEN_KW_Complex,
    TOKEN_KW_Embed,
    TOKEN_KW_Generic,
    TOKEN_KW_Imaginary,
    TOKEN_KW_Pragma,
    TOKEN_KW_Noreturn,
    TOKEN_KW_Static_assert,
    TOKEN_KW_Thread_local,
    TOKEN_KW_Typeof,
    TOKEN_KW_Vector,
    TOKEN_KW_asm,
    TOKEN_KW_attribute,
    TOKEN_KW_cdecl,
    TOKEN_KW_stdcall,
    TOKEN_KW_declspec,
};

#undef TKN2
#undef TKN3

struct token {
    token_type Type = TOKEN_INVALID;
    usize Location = 0;
};

using token_array = exponential_array<token, 23, 8>;

struct input {
    string Source;
    usize Position = 0;
};

bool input_next_token(string input)
{
  if (!str || byteLength < 0) return false;
  const u8 *p = (const u8 *)str;
  const u8 *end = p + byteLength;
  while (p < end)
  {
    // Size based on first byte; 0 means continuation byte at head -> invalid
    s64 cpSize = utf8_get_size_of_cp((const char *)p);
    if (cpSize <= 0) return false;
    if ((end - p) < cpSize) return false; // truncated sequence
    if (!utf8_is_valid_cp((const char *)p)) return false;
    p += cpSize;
  }
  return true;
}
