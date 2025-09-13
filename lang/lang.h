#pragma once

#include "lstd/xar.h"
#include "lstd/fmt.h"
#include "lstd/context.h"
#include "lstd/hash_table.h"

#include "snipffi.h"
#include "diagnostics.h"  

//
// Atoms:
//
inline arena_allocator_data ARENA_ATOMS_DATA;
#define ARENA_ATOMS (allocator{arena_allocator, &ARENA_ATOMS_DATA})

//
// Atoms are de-duplicated strings that live for the entire compilation.
// They are used for identifiers, keywords, number and string literals, etc.
//
// The idea is to allow for fast comparisons by comparing atom* 
// instead of string contents.
//
// To handle Unicode correctly, we expect the input strings
// to be in normalized form C (NFC). We do this at the beginning
// before even tokenizing (look at tokenizer_prepare_source).
//
// You can create a string view into the atom's data using string(a.Data, a.Count) from an atom*.
//
// The reason we don't use a string directly is because essentially an atom
// acts like an inline string Count + Data one after the other.
// We can do this because we are guaranteed that the atom lives in our atom arena and
// was allocated there. The same flexibility is not available for normal strings, because
// they keep a pointer to data that can be anywhere in (read-only) memory.
//
struct atom {
  s64 Count = 0;
  char Data[1];
}; 

using atoms_table = hash_table<atom*, atom*>;
inline atoms_table ATOMS_TABLE; 

//
// Find an atom in the table by its string contents and hash.
// Returns null if not found.
//
inline atom* atoms_table_probe(string s, u64 hash) {
  if (!ATOMS_TABLE.Count) return null;

  s64 index = hash & ATOMS_TABLE.Allocated - 1;
  For_as(_, range(ATOMS_TABLE.Allocated)) {
    auto it = ATOMS_TABLE.Entries.Data + index;

    if (it->Hash == hash && it->Key->Count == s.Count && memcmp(it->Key->Data, s.Data, s.Count) == 0)
      return it->Key;

    ++index;
    if (index >= ATOMS_TABLE.Allocated) index = 0;
  }
  return null;
}

//
// Creates an empty atom, with Count = 0 and Data[0] = '\0'.
// Used for building an atom when identifier has Unicode escapes,
// or when string literals have escapes,
// or when doing concatenation of string literals.
//
// Most of the time you want to use atom_put(string) instead,
// see note about atom_put below.
//
inline atom* atom_new() {
  // Already has space for null terminator from atom Data[1]
  atom* a = (atom*)malloc<byte>({.Alloc = ARENA_ATOMS, .Count = (s64) sizeof(atom)}); 
  a->Count = 0;
  a->Data[0] = '\0';
  return a;
}

//
// Call this after atom_new to append more data to the atom.
// This may reallocate the atom, but since we are using an arena allocator
// this actually just means allocating a new atom and copying the data over.
// In-case the atom is reallocated, the returned pointer will be different
// from the input one.
//
// To ensure the atom doesn't move around in memory, you can make sure
// you finish building an atom before allocating any other atoms.
//
inline atom* atom_push(atom* a, string s) {
  if (!s.Data || !s.Count) return a;

  atom* new_a = (atom *) realloc<byte>((byte *) a, {.NewCount = a->Count + s.Count + (s64) sizeof(atom)});
  if (a != new_a) {
    memcpy(new_a->Data, a->Data, a->Count);
  }
  memcpy(new_a->Data + a->Count, s.Data, s.Count);
  new_a->Count = a->Count + s.Count;
  new_a->Data[new_a->Count] = '\0';
  return new_a;
}

//
// When the atom is finished being built (atom_new + atom_push calls),
// put it in the table of atoms for de-duplication.
// This may return a different atom* if an identical atom already exists,
// in this case the old one is freed, but that only
// works if the atom was allocated in the ARENA_ATOMS arena,
// and no other atoms were allocated in between.
//
inline atom* atom_put(atom *a) {
  u64 hash = get_hash(a);
  atom* found_a = atoms_table_probe(string(a->Data, a->Count), hash);
  if (found_a) {
    // Free the newly built atom, since an identical one already exists, 
    // only works if it was at the top of ARENA_ATOMS.
    // Otherwise we leak.
    free(a); 
    return found_a;
  }

  add_prehashed(ATOMS_TABLE, hash, a, a);
  return a;
}

//
// Skip the building process and directly put a string in the atoms table.
// This is the most common way to create atoms, for identifiers and keywords.
// It is also used for string literals that don't have escapes.
//
inline atom* atom_put(string s)
{
  if (!s.Data || !s.Count) return null;

  u64 hash = get_hash(s);
  atom* a = atoms_table_probe(s, hash);
  if (a) return a;

  a = (atom*)malloc<char>({.Alloc = ARENA_ATOMS, .Count = s.Count + (s64) sizeof(atom)}); 
  a->Count = s.Count;
  memcpy(a->Data, s.Data, s.Count);
  a->Data[s.Count] = '\0';

  add_prehashed(ATOMS_TABLE, hash, a, a);
  return a;
}

//
// Tokens and tokenizer:
//

inline arena_allocator_data ARENA_TOKEN_DATA;
#define ARENA_TOKEN (allocator{arena_allocator, &ARENA_TOKEN_DATA})

#define TKN2(x, y) (((y) << 8) | (x))
#define TKN3(x, y, z) (((z) << 16) | ((y) << 8) | (x))

#define TOKEN_FLAG_WIDE   0x0100

enum token_type
{
  TOKEN_INVALID = 0,
  TOKEN_POISONED = 1,

  TOKEN_UNICODE_PUNCTUATION = 2, // Any math or symbol single character punctuation that doesn't yet have a specific token type

  TOKEN_NEWLINE = '\n',

  TOKEN_DOT = '.',
  TOKEN_COMMA = ',',

  TOKEN_PLUS = '+',
  TOKEN_MINUS = '-',
  TOKEN_ASTERISK = '*',
  TOKEN_SLASH = '/',
  TOKEN_PERCENT = '%',
  TOKEN_EQUAL = '=',

  TOKEN_AND = '&',
  TOKEN_HAT = '^',
  TOKEN_BAR = '|',

  TOKEN_HASH = '#',
  TOKEN_AT = '@',

  TOKEN_TILDE = '~',
  TOKEN_EXCLAMATION = '!',
  TOKEN_QUESTION = '?',
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

  TOKEN_STRING = '\"',

  TOKEN_IDENTIFIER = 256,
  TOKEN_INTEGER,
  TOKEN_FLOAT,

  TOKEN_TRIPLE_DOT = TKN3('.', '.', '.'),
  TOKEN_DOUBLE_DOT = TKN2('.', '.'),

  TOKEN_ARROW = TKN2('=', '>'),

  TOKEN_DOUBLE_AND = TKN2('&', '&'),
  TOKEN_DOUBLE_BAR = TKN2('|', '|'),

  TOKEN_PLUS_EQUAL = TKN2('+', '='),
  TOKEN_MINUS_EQUAL = TKN2('-', '='),
  TOKEN_ASTERISK_EQUAL = TKN2('*', '='),
  TOKEN_SLASH_EQUAL = TKN2('/', '='),
  TOKEN_PERCENT_EQUAL = TKN2('%', '='),
  TOKEN_BAR_EQUAL = TKN2('|', '='),
  TOKEN_AND_EQUAL = TKN2('&', '='),
  TOKEN_HAT_EQUAL = TKN2('^', '='),
  TOKEN_NOT_EQUAL = TKN2('!', '='),
  TOKEN_EQUAL_EQUAL = TKN2('=', '='),
  TOKEN_GREATER_EQUAL = TKN2('>', '='),
  TOKEN_LESS_EQUAL = TKN2('<', '='),
  TOKEN_LEFT_SHIFT = TKN2('<', '<'),
  TOKEN_RIGHT_SHIFT = TKN2('>', '>'),

  TOKEN_LEFT_SHIFT_EQUAL = TKN3('<', '<', '='),
  TOKEN_RIGHT_SHIFT_EQUAL = TKN3('>', '>', '='),
  TOKEN_INCREMENT = TKN2('+', '+'),
  TOKEN_DECREMENT = TKN2('-', '-'),

  TOKEN_KW_break = 0x10000000, // break
  TOKEN_KW_char,               // char
  TOKEN_KW_continue,           // continue
  TOKEN_KW_pass,               // pass
  TOKEN_KW_do,                 // do
  TOKEN_KW_if,                 // if
  TOKEN_KW_else,               // else
  TOKEN_KW_for,                // for
  TOKEN_KW_return,             // return
  TOKEN_KW_struct,             // struct
  TOKEN_KW_void,               // void
  TOKEN_KW_while,              // while

  TOKEN_COUNT
};

#undef TKN2
#undef TKN3

inline bool token_is_keyword(token_type type)
{
  return type >= TOKEN_KW_break && type < TOKEN_COUNT;
}

struct token
{
  token_type Type = TOKEN_INVALID;
  s64 Location = 0; // Offset in the source code string where the token starts

  atom *Atom = null; // The de-duplicated string representation for identifiers, number literals, string literals
  union {
    s128 IntValue = 0; // TOKEN_INTEGER
    f64 FloatValue; // TOKEN_FLOAT
    code_point CpValue; // TOKEN_UNICODE_PUNCTUATION
  };

  token() {}
  token(token_type type, s64 location, atom *atom = null) : Type(type), Location(location), Atom(atom), IntValue(0) {}
};

using token_array = exponential_array<token, 23, 8>;

//
// Note: Thoughout the tokenizer, we expect the source code to be null-terminated with '\0'.
// This simplifies a lot of checks.
//
// We also run UTF-8 NFC normalization on all source code strings
// to support Unicode correctly on identifier names which can contain
// combining characters, etc.
//
struct tokenizer
{
  const char *Start; // Points to the beginning of the source code
  const char *Current;
  const char *FileName = null;      // Optional file name for diagnostics

  // 1-based current line number (tracks newline traversals)
  s64 CurrentLine = 1;                 

  // Points to the start of the current line, to see if Start == CurrentLineStart, i.e. we are at column 0
  const char* CurrentLineStart = null; 

  // Optional sink to capture diagnostics, if not set diagnostics go to stderr
  array<string>* DiagnosticsSink = null;
};

inline u64 get_hash(tokenizer no_copy tz)
{
  return get_hash((void *)tz.Start) ^ get_hash((void *)tz.Current);
}

//
// Prepares the source code for tokenization.
// This normalizes it to UTF-8 NFC, and adds
// '\0' termination at the end.
//
inline const char *tokenizer_prepare_source(string sourceCode)
{
  if(sourceCode.Count >= 0xFFFFFFFF) {
    ERR(mprint("Source code too large ({:n} bytes)", sourceCode.Count));
    return null;
  }

  string_builder sb;
  if (!utf8_normalize_nfc_to_string_builder(sourceCode.Data, sourceCode.Count, sb))
  {
    ERR("Failed to normalize source code to a UTF-8 NFC string");
    return null;
  }
  add(sb, '\0');
  return builder_to_string_and_free_builder(sb).Data;
}

// Gets the next token from the tokenizer,
// increments the position. You can cheaply call this
// to reparse the same token multiple times, if
// you reset the position.
token tokenizer_next_token(tokenizer ref tz);

// Parses a number (integer or float) from the given string position.
// Handles various bases (hex 0x, octal 0, binary 0b) and float formats.
// Returns a token with the number atom as payload.
// If invalid number, returns TOKEN_INVALID.
token tokenizer_next_number_literal(tokenizer ref tz);

// Parses a string literal.
// Handles escape sequences, including Unicode escapes.
// Returns a token with the string atom as payload.
// If invalid string (e.g. missing ending quote), returns TOKEN_INVALID
token tokenizer_next_string_literal(tokenizer ref tz);

// Iteratively skip ASCII/Unicode whitespace and C/C++ style comments.
// Doesn't skip newlines, those are significant, e.g. for the preprocessor.
void tokenizer_skip_trivia(tokenizer ref tz);

// Hooks into the global diagnostic context,
// so that diagnostics can report line numbers and file names,
// without having to pass the tokenizer/source code around everywhere to report errors.
inline void diagnostics_set_active_tokenizer(const tokenizer* tz) {
  auto get_line = [](const void* p)->s64 { return p ? ((const tokenizer*)p)->CurrentLine : 1; };
  auto get_filename = [](const void* p)->const char* { return p ? ((const tokenizer*)p)->FileName : null; };
  diag_set_active_tokenizer(tz, get_line, get_filename);
  diag_set_source(tz ? tz->Start : null);
  if (tz && tz->DiagnosticsSink) diag_set_sink(tz->DiagnosticsSink); else diag_set_sink(null);
}

inline void diagnostics_clear_active_tokenizer() {
  diag_set_active_tokenizer(null, null, null); 
  diag_set_sink(null);
  diag_set_source(null);
}

inline token_array tokenizer_tokenize(const char* sourceCode, const char* FileName = null, array<string>* DiagnosticsSink = null)
{
  PUSH_ALLOC(ARENA_TOKEN)
  {
    tokenizer tz = {sourceCode, sourceCode, FileName, 1, sourceCode, DiagnosticsSink};

    token_array tokens;
    diagnostics_set_active_tokenizer(&tz);

    while (*tz.Current != '\0')
    {
      const char* start = tz.Current;
      token t = tokenizer_next_token(tz);
      tokenizer_skip_trivia(tz);

      if (tz.Current && t.Type == TOKEN_INVALID)
      {
        t.Type = TOKEN_POISONED;
        code_point cp = utf8_decode_cp(start);
        WARN_ANNOTATED("Invalid token", start, tz.Current, mprint("Remove this: {:c} U+{:X}", cp, cp));
        continue;
      }
      add(tokens, t);
    }

    diagnostics_clear_active_tokenizer();
    return tokens;
  }
}

inline token_array tokenizer_tokenize(string sourceCode, const char* FileName = null, array<string>* DiagnosticsSink = null)
{
  PUSH_ALLOC(ARENA_TOKEN)
  {
    const char* sc = tokenizer_prepare_source(sourceCode);
    if (!sc) return {};
    return tokenizer_tokenize(sc, FileName, DiagnosticsSink);
  }
}

// Converts a token type to its literal enum name, e.g. TOKEN_PLUS
const char *token_type_to_string(token_type t);

//
// Converts a token to its string representation.
// For identifiers, integers, floats and strings
// we return the actual text from the source code.
// Note: We don't store lengths or strings in the token,
// as this is mostly used for error messages, and
// unnecessarily bloats the token structure, leading to
// more cache misses and memory usage.
//
// So this function needs the source code as well,
// and we call the tokenizer to re-tokenize and get
// the actual text.
//
inline string token_to_string(token t)
{
  switch (t.Type)
  {
  case TOKEN_FLOAT:
  {
    assert(t.Atom);
    return sprint("{} (value: {})", string(t.Atom->Data, t.Atom->Count), t.FloatValue);
  }
  case TOKEN_INTEGER:
  {
    assert(t.Atom);
    return sprint("{} (value: {})", string(t.Atom->Data, t.Atom->Count), t.IntValue);
  }
  case TOKEN_IDENTIFIER:
  {
    assert(t.Atom);
    return string(t.Atom->Data, t.Atom->Count);
  }
  case TOKEN_STRING:
  {
    assert(t.Atom);
    return sprint("\"{}\"", string(t.Atom->Data, t.Atom->Count));
  }
  case TOKEN_UNICODE_PUNCTUATION:
    return sprint("{:c} U+{:X}", t.CpValue, t.CpValue);
  case TOKEN_POISONED:
    return "TOKEN_POISONED";
  break;
  default:
  {
    const char* token_to_string_gen(token_type t);
    return token_to_string_gen(t.Type);
  }
  }
}

//
// Parsing:
//

