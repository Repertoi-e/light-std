#pragma once

#include "lstd/xar.h"
#include "lstd/string.h"
#include "lstd/context.h"

template <typename... Args>
void error(string message, Args no_copy... arguments)
{
    print("{!RED}error:{!} {}\n", tprint(message, arguments...));
}

template <typename... Args>
void warn(string message, Args no_copy... arguments)
{
    print("{!YELLOW}warning:{!} {}\n", tprint(message, arguments...));
}

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

  TOKEN_KW_auto = 0x10000000, // auto
  TOKEN_KW_break, // break
  TOKEN_KW_case, // case
  TOKEN_KW_char, // char
  TOKEN_KW_const, // const
  TOKEN_KW_continue, // continue
  TOKEN_KW_default, // default
  TOKEN_KW_do, // do
  TOKEN_KW_double, // double
  TOKEN_KW_else, // else
  TOKEN_KW_enum, // enum
  TOKEN_KW_extern, // extern
  TOKEN_KW_float, // float
  TOKEN_KW_for, // for
  TOKEN_KW_goto, // goto
  TOKEN_KW_if, // if
  TOKEN_KW_inline, // inline
  TOKEN_KW_int, // int
  TOKEN_KW_long, // long
  TOKEN_KW_register, // register
  TOKEN_KW_restrict, // restrict
  TOKEN_KW_return, // return
  TOKEN_KW_short, // short
  TOKEN_KW_signed, // signed
  TOKEN_KW_sizeof, // sizeof
  TOKEN_KW_static, // static
  TOKEN_KW_struct, // struct
  TOKEN_KW_switch, // switch
  TOKEN_KW_typedef, // typedef
  TOKEN_KW_union, // union
  TOKEN_KW_unsigned, // unsigned
  TOKEN_KW_void, // void
  TOKEN_KW_volatile, // volatile
  TOKEN_KW_while, // while
  TOKEN_KW_Alignas, // _Alignas
  TOKEN_KW_Alignof, // _Alignof
  TOKEN_KW_Atomic, // _Atomic
  TOKEN_KW_Bool, // _Bool
  TOKEN_KW_Complex, // _Complex
  TOKEN_KW_Embed, // _Embed
  TOKEN_KW_Generic, // _Generic
  TOKEN_KW_Imaginary, // _Imaginary
  TOKEN_KW_Pragma, // _Pragma
  TOKEN_KW_Noreturn, // _Noreturn
  TOKEN_KW_Static_assert, // _Static_assert
  TOKEN_KW_Thread_local, // _Thread_local
  TOKEN_KW_Typeof, // _Typeof
  TOKEN_KW_Vector, // _Vector
  TOKEN_KW_asm, // asm
  TOKEN_KW_attribute, // attribute
  TOKEN_KW_cdecl, // cdecl
  TOKEN_KW_stdcall, // stdcall
  TOKEN_KW_declspec, // declspec

  TOKEN_COUNT
};

const char *token_to_string(token_type t);

inline bool token_is_keyword(token_type type)
{
  return type >= TOKEN_KW_auto && type < TOKEN_COUNT;
}

#undef TKN2
#undef TKN3

struct token
{
  token_type Type = TOKEN_INVALID;
  usize Location = 0;
};

using token_array = exponential_array<token, 23, 8>;

struct tokenizer
{
  string Source;
  s64 Position = 0; // Byte position in the source
};

inline bool tokenizer_at_end(tokenizer ref tokenizer)
{
  return tokenizer.Position >= tokenizer.Source.Count;
}

inline byte tokenizer_peek_ascii(tokenizer ref tokenizer)
{
  assert(!tokenizer_at_end(tokenizer) && "Peeking past the end of the tokenizer source");
  return tokenizer.Source.Data[tokenizer.Position];
}

inline void tokenizer_advance(tokenizer ref tokenizer, s64 n = 1)
{
  tokenizer.Position += n;
}

// Gets the next token from the tokenizer,
// increments the position. You can cheaply call this
// to reparse the same token multiple times, if
// you reset the position.
token tokenizer_next_token(tokenizer ref tokenizer);

// Iteratively skip ASCII/Unicode whitespace and C/C++ style comments.
void tokenizer_skip_trivia(tokenizer ref tz);

token_array tokenizer_tokenize(string source)
{
  PUSH_ALLOC(ARENA_TOKEN)
  {
    tokenizer tz = {.Source = source};

    token_array tokens;
    while (!tokenizer_at_end(tz))
    {
      token t = tokenizer_next_token(tz);
      if (t.Type == TOKEN_INVALID)
      {
        warn("Invalid token at byte offset {}", t.Location);
        continue;
      }
      add(tokens, t);
    }
    return tokens;
  }
}