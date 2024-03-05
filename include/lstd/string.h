#pragma once

#include "delegate.h"
#include "memory.h"
#include "stack_array.h"

LSTD_BEGIN_NAMESPACE

///
/// The following utility functions are for ASCII and utf-8 null-terminated
/// strings:
///

// * c_string_length - strlen
// * utf8_length
// * compare_string
// * compare_string_lexicographically - strcmp
// * compare_string_ignore_case
// * compare_string_lexicographically_ignore_case
// * strings_match
//
// Working with code points:
// * to_upper              - toupper
// * to_lower              - tolower
// * is_upper              - isupper
// * is_lower              - islower
// * utf8_get_size_of_cp
// * utf8_encode_cp
// * utf8_decode_cp
// * utf8_is_valid_cp
//
// These work only for ascii:
// * is_digit             - isdigit
// * is_hex_digit         - isxdigit
// * is_space             - isspace
// * is_blank             - isblank
// * is_alpha             - isalpha
// * is_alphanumeric      - isalnum
// * is_print             - isprint
// * is_identifier_start
//
// Conversions:
// * utf8_to_utf16
// * utf8_to_utf32
// * utf16_to_utf8
// * utf32_to_utf8
//

template <typename C>
using c_string_type =
    add_pointer_t<remove_cvref_t<remove_pointer_t<remove_cvref_t<C>>>>;

// C++ mess
template <typename C>
concept any_c_string = is_pointer<C> && is_same_to_one_of < c_string_type<C>,
char *, wchar *, char8_t *, char16_t *, char32_t *, code_point * > ;

template <typename C>
concept any_byte_pointer =
    is_pointer<C> && is_same_to_one_of < c_string_type<C>,
char *, char8_t *, unsigned char *, s8 *, u8 *, byte * > ;

// The length of a null-terminated string. Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// @Speed @TODO Vectorize
s64 c_string_length(any_c_string auto str) {
  if (!str) return 0;

  s64 length = 0;
  while (*str++) ++length;
  return length;
}

// The length (in code points) of a utf-8 string
// @Speed @TODO Vectorize
inline s64 utf8_length(const char *str, s64 size) {
  if (!str || size == 0) return 0;

  // Count all first-bytes (the ones that don't match 10xxxxxx).
  s64 length = 0;
  while (size--) {
    if (!((*str++ & 0xc0) == 0x80)) ++length;
  }
  return length;
}

// Returns -1 if strings match, else returns the index of the first different
// byte
// @Speed @TODO Vectorize
template <any_c_string C>
s64 compare_string(C one, C other) {
  assert(one);
  assert(other);

  if (!*one && !*other) return -1;

  s64 index = 0;
  while (*one == *other) {
    ++one, ++other;
    if (!*one && !*other) return -1;
    if (!*one || !*other) return index;
    ++index;
  }
  return index;
}

// Return -1 if one < other, 0 if one == other and 1 if one > other (not the
// pointers)
// @Speed @TODO Vectorize
template <any_c_string C>
s32 compare_string_lexicographically(C one, C other) {
  assert(one);
  assert(other);

  while (*one && (*one == *other)) ++one, ++other;
  return (*one > *other) - (*other > *one);
}

// @TODO
// This function only works for ascii
inline bool is_digit(code_point x) { return x >= '0' && x <= '9'; }

// This function only works for ascii
inline bool is_hex_digit(code_point x) {
  return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') ||
         (x >= 'A' && x <= 'F');
}

// This function only works for ascii
inline bool is_space(code_point x) { return (x >= 9 && x <= 13) || x == 32; }

// This function only works for ascii
inline bool is_blank(code_point x) { return x == 9 || x == 32; }

// This function only works for ascii
inline bool is_alpha(code_point x) {
  return (x >= 65 && x <= 90) || (x >= 97 && x <= 122);
}

// This function only works for ascii
inline bool is_alphanumeric(code_point x) { return is_alpha(x) || is_digit(x); }

// This function only works for ascii
inline bool is_identifier_start(code_point x) {
  return is_alpha(x) || x == '_';
}

// This function only works for ascii
inline bool is_print(code_point x) { return x > 31 && x != 127; }

// Convert code point to uppercase
inline code_point to_upper(code_point cp) {
  if (((0x0061 <= cp) && (0x007a >= cp)) ||
      ((0x00e0 <= cp) && (0x00f6 >= cp)) ||
      ((0x00f8 <= cp) && (0x00fe >= cp)) ||
      ((0x03b1 <= cp) && (0x03c1 >= cp)) ||
      ((0x03c3 <= cp) && (0x03cb >= cp))) {
    return cp - 32;
  }

  if (((0x0100 <= cp) && (0x012f >= cp)) ||
      ((0x0132 <= cp) && (0x0137 >= cp)) ||
      ((0x014a <= cp) && (0x0177 >= cp)) ||
      ((0x0182 <= cp) && (0x0185 >= cp)) ||
      ((0x01a0 <= cp) && (0x01a5 >= cp)) ||
      ((0x01de <= cp) && (0x01ef >= cp)) ||
      ((0x01f8 <= cp) && (0x021f >= cp)) ||
      ((0x0222 <= cp) && (0x0233 >= cp)) ||
      ((0x0246 <= cp) && (0x024f >= cp)) ||
      ((0x03d8 <= cp) && (0x03ef >= cp))) {
    return cp & ~0x1;
  }

  if (((0x0139 <= cp) && (0x0148 >= cp)) ||
      ((0x0179 <= cp) && (0x017e >= cp)) ||
      ((0x01af <= cp) && (0x01b0 >= cp)) ||
      ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
      ((0x01cd <= cp) && (0x01dc >= cp))) {
    return (cp - 1) | 0x1;
  }

  if (cp == 0x00ff) return 0x0178;
  if (cp == 0x0180) return 0x0243;
  if (cp == 0x01dd) return 0x018e;
  if (cp == 0x019a) return 0x023d;
  if (cp == 0x019e) return 0x0220;
  if (cp == 0x0292) return 0x01b7;
  if (cp == 0x01c6) return 0x01c4;
  if (cp == 0x01c9) return 0x01c7;
  if (cp == 0x01cc) return 0x01ca;
  if (cp == 0x01f3) return 0x01f1;
  if (cp == 0x01bf) return 0x01f7;
  if (cp == 0x0188) return 0x0187;
  if (cp == 0x018c) return 0x018b;
  if (cp == 0x0192) return 0x0191;
  if (cp == 0x0199) return 0x0198;
  if (cp == 0x01a8) return 0x01a7;
  if (cp == 0x01ad) return 0x01ac;
  if (cp == 0x01b0) return 0x01af;
  if (cp == 0x01b9) return 0x01b8;
  if (cp == 0x01bd) return 0x01bc;
  if (cp == 0x01f5) return 0x01f4;
  if (cp == 0x023c) return 0x023b;
  if (cp == 0x0242) return 0x0241;
  if (cp == 0x037b) return 0x03fd;
  if (cp == 0x037c) return 0x03fe;
  if (cp == 0x037d) return 0x03ff;
  if (cp == 0x03f3) return 0x037f;
  if (cp == 0x03ac) return 0x0386;
  if (cp == 0x03ad) return 0x0388;
  if (cp == 0x03ae) return 0x0389;
  if (cp == 0x03af) return 0x038a;
  if (cp == 0x03cc) return 0x038c;
  if (cp == 0x03cd) return 0x038e;
  if (cp == 0x03ce) return 0x038f;
  if (cp == 0x0371) return 0x0370;
  if (cp == 0x0373) return 0x0372;
  if (cp == 0x0377) return 0x0376;
  if (cp == 0x03d1) return 0x03f4;
  if (cp == 0x03d7) return 0x03cf;
  if (cp == 0x03f2) return 0x03f9;
  if (cp == 0x03f8) return 0x03f7;
  if (cp == 0x03fb) return 0x03fa;

  // No uppercase!
  return cp;
}

// Convert code point to lowercase
inline code_point to_lower(code_point cp) {
  if (((0x0041 <= cp) && (0x005a >= cp)) ||
      ((0x00c0 <= cp) && (0x00d6 >= cp)) ||
      ((0x00d8 <= cp) && (0x00de >= cp)) ||
      ((0x0391 <= cp) && (0x03a1 >= cp)) ||
      ((0x03a3 <= cp) && (0x03ab >= cp))) {
    return cp + 32;
  }

  if (((0x0100 <= cp) && (0x012f >= cp)) ||
      ((0x0132 <= cp) && (0x0137 >= cp)) ||
      ((0x014a <= cp) && (0x0177 >= cp)) ||
      ((0x0182 <= cp) && (0x0185 >= cp)) ||
      ((0x01a0 <= cp) && (0x01a5 >= cp)) ||
      ((0x01de <= cp) && (0x01ef >= cp)) ||
      ((0x01f8 <= cp) && (0x021f >= cp)) ||
      ((0x0222 <= cp) && (0x0233 >= cp)) ||
      ((0x0246 <= cp) && (0x024f >= cp)) ||
      ((0x03d8 <= cp) && (0x03ef >= cp))) {
    return cp | 0x1;
  }

  if (((0x0139 <= cp) && (0x0148 >= cp)) ||
      ((0x0179 <= cp) && (0x017e >= cp)) ||
      ((0x01af <= cp) && (0x01b0 >= cp)) ||
      ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
      ((0x01cd <= cp) && (0x01dc >= cp))) {
    return (cp + 1) & ~0x1;
  }

  if (cp == 0x0178) return 0x00ff;
  if (cp == 0x0243) return 0x0180;
  if (cp == 0x018e) return 0x01dd;
  if (cp == 0x023d) return 0x019a;
  if (cp == 0x0220) return 0x019e;
  if (cp == 0x01b7) return 0x0292;
  if (cp == 0x01c4) return 0x01c6;
  if (cp == 0x01c7) return 0x01c9;
  if (cp == 0x01ca) return 0x01cc;
  if (cp == 0x01f1) return 0x01f3;
  if (cp == 0x01f7) return 0x01bf;
  if (cp == 0x0187) return 0x0188;
  if (cp == 0x018b) return 0x018c;
  if (cp == 0x0191) return 0x0192;
  if (cp == 0x0198) return 0x0199;
  if (cp == 0x01a7) return 0x01a8;
  if (cp == 0x01ac) return 0x01ad;
  if (cp == 0x01af) return 0x01b0;
  if (cp == 0x01b8) return 0x01b9;
  if (cp == 0x01bc) return 0x01bd;
  if (cp == 0x01f4) return 0x01f5;
  if (cp == 0x023b) return 0x023c;
  if (cp == 0x0241) return 0x0242;
  if (cp == 0x03fd) return 0x037b;
  if (cp == 0x03fe) return 0x037c;
  if (cp == 0x03ff) return 0x037d;
  if (cp == 0x037f) return 0x03f3;
  if (cp == 0x0386) return 0x03ac;
  if (cp == 0x0388) return 0x03ad;
  if (cp == 0x0389) return 0x03ae;
  if (cp == 0x038a) return 0x03af;
  if (cp == 0x038c) return 0x03cc;
  if (cp == 0x038e) return 0x03cd;
  if (cp == 0x038f) return 0x03ce;
  if (cp == 0x0370) return 0x0371;
  if (cp == 0x0372) return 0x0373;
  if (cp == 0x0376) return 0x0377;
  if (cp == 0x03f4) return 0x03d1;
  if (cp == 0x03cf) return 0x03d7;
  if (cp == 0x03f9) return 0x03f2;
  if (cp == 0x03f7) return 0x03f8;
  if (cp == 0x03fa) return 0x03fb;

  // No lower case!
  return cp;
}

inline bool is_upper(code_point ch) { return ch != to_lower(ch); }
inline bool is_lower(code_point ch) { return ch != to_upper(ch); }

// Returns -1 if strings match, else returns the index of the first different
// byte. Ignores the case of the characters.
// @Speed @TODO Vectorize
template <any_c_string C>
s64 compare_string_ignore_case(C one, C other) {
  assert(one);
  assert(other);

  if (!*one && !*other) return -1;

  s64 index = 0;
  while (to_lower(*one) == to_lower(*other)) {
    ++one, ++other;
    if (!*one && !*other) return -1;
    if (!*one || !*other) return index;
    ++index;
  }
  return index;
}

// Return -1 if one < other, 0 if one == other and 1 if one > other (not the
// pointers). Ignores the case of the characters.
// @Speed @TODO Vectorize
template <any_c_string C>
s32 compare_string_lexicographically_ignore_case(C one, C other) {
  assert(one);
  assert(other);

  while (*one && (to_lower(*one) == to_lower(*other))) ++one, ++other;
  return (*one > *other) - (*other > *one);
}

// true if strings are equal (not the pointers)
template <any_c_string C>
bool strings_match(C one, C other) {
  return compare_string(one, other) == -1;
}

// true if strings are equal (not the pointers)
template <any_c_string C>
bool strings_match_ignore_case(C one, C other) {
  return compare_string_ignore_case(one, other) == -1;
}

// Returns the size in bytes of the code point that _str_ points to.
// If the byte pointed by _str_ is a countinuation utf-8 byte, this function
// returns 0.
inline s8 utf8_get_size_of_cp(const char *str) {
  if (!str) return 0;
  if ((*str & 0xc0) == 0x80) return 0;

  if (0xf0 == (0xf8 & str[0])) {
    return 4;
  } else if (0xe0 == (0xf0 & str[0])) {
    return 3;
  } else if (0xc0 == (0xe0 & str[0])) {
    return 2;
  } else {
    return 1;
  }
}

// Returns the size that the code point would be if it were encoded
inline s8 utf8_get_size_of_cp(code_point codePoint) {
  if (((s32)0xffffff80 & codePoint) == 0) {
    return 1;
  } else if (((s32)0xfffff800 & codePoint) == 0) {
    return 2;
  } else if (((s32)0xffff0000 & codePoint) == 0) {
    return 3;
  } else {
    return 4;
  }
}

// Encodes code point at _str_, assumes there is enough space
inline void utf8_encode_cp(char *str, code_point codePoint) {
  s64 size = utf8_get_size_of_cp(codePoint);
  if (size == 1) {
    // 1-byte/7-bit ascii
    // (0b0xxxxxxx)
    str[0] = (char)codePoint;
  } else if (size == 2) {
    // 2-byte/11-bit utf-8 code point
    // (0b110xxxxx 0b10xxxxxx)
    str[0] = 0xc0 | (char)(codePoint >> 6);
    str[1] = 0x80 | (char)(codePoint & 0x3f);
  } else if (size == 3) {
    // 3-byte/16-bit utf-8 code point
    // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
    str[0] = 0xe0 | (char)(codePoint >> 12);
    str[1] = 0x80 | (char)((codePoint >> 6) & 0x3f);
    str[2] = 0x80 | (char)(codePoint & 0x3f);
  } else {
    // 4-byte/21-bit utf-8 code point
    // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
    str[0] = 0xf0 | (char)(codePoint >> 18);
    str[1] = 0x80 | (char)((codePoint >> 12) & 0x3f);
    str[2] = 0x80 | (char)((codePoint >> 6) & 0x3f);
    str[3] = 0x80 | (char)(codePoint & 0x3f);
  }
}

// Decodes a code point from a data pointer
inline code_point utf8_decode_cp(const char *str) {
  if (0xf0 == (0xf8 & str[0])) {
    // 4 byte utf-8 code point
    return ((0x07 & str[0]) << 18) | ((0x3f & str[1]) << 12) |
           ((0x3f & str[2]) << 6) | (0x3f & str[3]);
  } else if (0xe0 == (0xf0 & str[0])) {
    // 3 byte utf-8 code point
    return ((0x0f & str[0]) << 12) | ((0x3f & str[1]) << 6) | (0x3f & str[2]);
  } else if (0xc0 == (0xe0 & str[0])) {
    // 2 byte utf-8 code point
    return ((0x1f & str[0]) << 6) | (0x3f & str[1]);
  } else {
    // 1 byte utf-8 code point
    return str[0];
  }
}

// Checks whether the encoded code point in data is valid utf-8
inline bool utf8_is_valid_cp(const char *data) {
  u8 *p = (u8 *)data;

  s64 sizeOfCp = utf8_get_size_of_cp(data);
  if (sizeOfCp == 1) {
    if ((s8)*data < 0) return false;
  } else if (sizeOfCp == 2) {
    if (*p < 0xC2 || *p > 0xDF) return false;
    ++p;
    if (*p < 0x80 || *p > 0xBF) return false;
  } else if (sizeOfCp == 3) {
    if (*p == 0xE0) {
      ++p;
      if (*p < 0xA0 || *p > 0xBF) return false;
    } else if (*p >= 0xE1 && *p <= 0xEC) {
      ++p;
      if (*p < 0x80 || *p > 0xBF) return false;
    } else if (*p == 0xED) {
      ++p;
      if (*p < 0x80 || *p > 0x9F) return false;
    } else if (*p >= 0xEE && *p <= 0xEF) {
      ++p;
      if (*p < 0x80 || *p > 0xBF) return false;
    } else {
      return false;
    }
    // The third byte restriction is the same on all of these
    ++p;
    if (*p < 0x80 || *p > 0xBF) return false;
  } else if (sizeOfCp == 4) {
    if (*p == 0xF0) {
      ++p;
      if (*p < 0x90 || *p > 0xBF) return false;
    } else if (*p >= 0xF1 && *p <= 0xF3) {
      ++p;
      if (*p < 0x80 || *p > 0xBF) return false;
    } else if (*p == 0xF4) {
      ++p;
      if (*p < 0x80 || *p > 0x8F) return false;
    } else {
      return false;
    }
    // The third and fourth byte restriction is the same on all of these
    ++p;
    if (*p < 0x80 || *p > 0xBF) return false;
  } else {
    return false;
  }
  return true;
}

// This returns a pointer to the code point at a specified index in an utf-8
// string. This is unsafe, doesn't check if we go over bounds. In the general
// case you should call this with a result from translate_negative_index(...),
// which handles out of bounds indexing.
//
// If LSTD_ARRAY_BOUNDS_CHECK is defined this fails if we go out of bounds.
//
// @Speed @TODO Vectorize for large strings
inline const char *utf8_get_pointer_to_cp_at_translated_index(const char *str,
                                                              s64 byteLength,
                                                              s64 index) {
  auto *end = str + byteLength;

  For(range(index)) {
    // Danger danger. If the string contains invalid utf8, then we might bypass
    // str == end. That's why we check with >=.

    if (str >= end) {
      assert(false && "Out of bounds");
    }

    str += utf8_get_size_of_cp(str);
  }
  return str;
}

// Converts utf-8 to utf-16 and stores in _out_ (assumes there is enough space).
// Also adds a null-terminator at the end.
inline void utf8_to_utf16(const char *str, s64 length, wchar *out) {
  For(range(length)) {
    code_point cp = utf8_decode_cp(str);
    if (cp > 0xffff) {
      *out++ = (u16)((cp >> 10) + (0xD800u - (0x10000 >> 10)));
      *out++ = (u16)((cp & 0x3FF) + 0xDC00u);
    } else {
      *out++ = (u16)cp;
    }
    str += utf8_get_size_of_cp(cp);
  }
  *out = 0;
}

// Converts utf-8 to utf-32 and stores in _out_ (assumes there is enough space).
//
// Also adds a null-terminator at the end.
inline void utf8_to_utf32(const char *str, s64 byteLength, code_point *out) {
  auto *end = str + byteLength;

  // Danger danger. If the string contains invalid utf8, then we might bypass
  // str != end and infinite loop. That's why we check with <.
  while (str < end) {
    code_point cp = utf8_decode_cp(str);
    *out++ = cp;
    str += utf8_get_size_of_cp(cp);
  }

  *out = 0;
}

// Converts a null-terminated utf-16 to utf-8 and stores in _out_ and
// _outByteLength_ (assumes there is enough space).
inline void utf16_to_utf8(const wchar *str, char *out, s64 *outByteLength) {
  s64 byteLength = 0;
  while (*str) {
    code_point cp = *str;
    if ((cp >= 0xD800) && (cp <= 0xDBFF)) {
      code_point trail = cp = *++str;
      if (!*str)
        assert(false &&
               "Invalid wchar string");  // @TODO @Robustness Bail on errors

      if ((trail >= 0xDC00) && (trail <= 0xDFFF)) {
        cp = ((cp - 0xD800) << 10) + (trail - 0xDC00) + 0x0010000;
      } else {
        assert(false &&
               "Invalid wchar string");  // @TODO @Robustness Bail on errors
      }
    }

    utf8_encode_cp(out, cp);
    s64 cpSize = utf8_get_size_of_cp(out);
    out += cpSize;
    byteLength += cpSize;
    ++str;
  }
  *outByteLength = byteLength;
}

// Converts a null-terminated utf-32 to utf-8 and stores in _out_ and
// _outByteLength_ (assumes there is enough space).
inline void utf32_to_utf8(const code_point *str, char *out,
                          s64 *outByteLength) {
  s64 byteLength = 0;
  while (*str) {
    utf8_encode_cp(out, *str);
    s64 cpSize = utf8_get_size_of_cp(out);
    out += cpSize;
    byteLength += cpSize;
    ++str;
  }
  *outByteLength = byteLength;
}

// @TODO: Provide a _string_utf8_validate()_.
// @TODO: Provide a _string_utf8_normalize()_.

//
// This is a string object with text operations on it (assuming valid encoded
// utf-8), that is not null-terminated, but works like an array. It's a basic
// wrapper around contiguous memory, it contains a char pointer and a size, so
// any binary file can be read to it without problem.
//
// Functions on this object allow negative reversed indexing which begins at
// the end of the array, so -1 is the last code point -2 the one before that,
// etc. (Python-style)
//
// By default it works like a view, a string doesn't own it's memory unless you
// explicitly treat it like that. We have a very fluid philosophy of containers
// and ownership. We don't implement copy constructors or destructors, which
// means that the programmer is totally in control of how the memory gets
// managed. (See :TypePolicy in "common.h")
//
// That also means that substrings don't allocate memory,
// they are just a new data pointer and count.
//
// In order to get a deep copy of a string use clone().
//
// This object being just two 64 bit integers can be cheaply and safely passed
// to functions by value without performance concerns and indirection.
// (Remember that the array doesn't "own" it's buffer, so no copy happens)
//
struct string;

s64 length(string no_copy s);
void set(string ref s, s64 index, code_point cp);
code_point get(string str, s64 index);
void check_debug_memory(string no_copy s);

struct string {
  char *Data = null;
  s64 Count = 0;
  s64 Allocated = 0;

  // We can't treat it as array-like, because for strings
  // we work with indices which are code point based (and not char based).
  static const bool TREAT_AS_ARRAY_LIKE = false;

  string() {}

  // This constructs a view (use make_string to copy)
  string(const char *str) : Data((char *)str), Count(c_string_length(str)) {}

  // This constructs a view (use make_string to copy)
  string(const char8_t *str) : Data((char *)str), Count(c_string_length(str)) {}

  // This constructs a view (use make_string to copy)
  string(const char *str, s64 count) : Data((char *)str), Count(count) {}

  // A special structure which is returned from string_get(),
  // which allows to see the code point at a specific index, but also update
  // it.
  struct code_point_ref {
    string ref String;
    s64 Index;

    code_point_ref(string ref s, s64 index) : String(s) {
      Index = translate_negative_index(index, length(s));
    }

    code_point_ref &operator=(code_point other) {
      set(String, Index, other);
      return *this;
    }

    operator code_point() const {
      return utf8_decode_cp(utf8_get_pointer_to_cp_at_translated_index(
          String.Data, length(String), Index));
    }
  };

  //
  // for operator[] we return a special structure that allows
  // assigning a new code point.
  //
  // This is legal to do:
  //
  //      string a = "Hello";
  //      a[0] = u8'Л';
  //      // a is now "Лello" and contains a two byte code point in the
  //      beginning,
  //      // an allocation was possibly made and memory was moved around.
  //
  // Indexing is code point based, so:
  //      a[1] = u8'Ли';
  //      // a is now "Лиllo", despite "Л" being two bytes
  //      // a[1] referred to the second code point.
  //
  //
  code_point_ref operator[](s64 index) { return code_point_ref(*this, index); }
  code_point operator[](s64 index) const {
    return utf8_decode_cp(
        utf8_get_pointer_to_cp_at_translated_index(Data, length(*this), index));
  }
};

inline void reserve(string ref s, s64 n = -1, allocator alloc = {}) {
  if (n <= 0) {
    n = max(s.Count, 8);
  }
  assert(n >= 1);

  using T = remove_pointer_t<decltype(s.Data)>;

  auto *oldData = s.Data;
  if (s.Allocated) {
    s.Data = realloc(s.Data, {.NewCount = n});
  } else {
    // Not our job to free oldData since we don't own it.
    // For subsequent reserves we go through the   realloc   branch above,
    // which properly manages to free the old data (if it couldn't reallocate
    // in place, that is).
    s.Data = malloc<T>(
        {.Count = n,
         .Alloc = alloc});  // If alloc is null we use theContext's allocator
    if (oldData) memcpy(s.Data, oldData, s.Count * sizeof(T));
  }

  s.Allocated = n;
}

inline void check_debug_memory(string no_copy s) {
  assert(s.Allocated);
#if defined DEBUG_MEMORY
  //
  // If you assert here, there are two possible reasons:
  //
  // 1. Attempting to modify a string which is a view
  // and _Data_ wasn't dynamically allocated. This may
  // also happen if an string points to a string from
  // the text table of the executable.
  //
  // Make sure you call  reserve()  beforehand
  // to copy the contents and make a string which owns
  // memory.
  //
  // 2. Attempting to modify an string from another thread...
  // Caution! This container is not thread-safe!
  //
  assert(debug_memory_list_contains((allocation_header *)s.Data - 1));
#endif
}

inline void free(string ref s) {
  free(s.Data);
  s.Count = s.Allocated = 0;
}

// This is <= Count
inline s64 length(string no_copy s) { return utf8_length(s.Data, s.Count); }

// Doesn't allocate memory, strings in this library are not null-terminated.
// We allow negative reversed indexing which begins at the end of the string,
// so -1 is the last code point, -2 is the one before that, etc.
// (Python-style)
string slice(string s, s64 begin, s64 end);

//
// Utilities to convert to c-style strings.
// Functions for conversion between utf-8, utf-16 
// and utf-32 are provided in "string.h"
//

// Allocates a buffer, copies the string's contents and also appends a zero
// terminator. Uses the Context's current allocator. The caller is responsible
// for freeing.
mark_as_leak char *to_c_string(string s, allocator alloc = {});

// Allocates a buffer, copies the string's contents and also appends a zero
// terminator. Uses the temporary allocator.
//
// Implemented in "context.h" because otherwise we import in circle.
char *to_c_string_temp(string s);

// Returns the code point index (or -1) if not found.
s64 search(string str, delegate<bool(code_point)> predicate,
           search_options options = {});

// Returns the code point index (or -1) if not found.
s64 search(string str, code_point search, search_options options = {});

// Returns the code point index (or -1) if not found.
s64 search(string str, string search, search_options options = {});

bool has(string str, code_point cp);
bool has(string str, string s);

//
// BIG NOTE: To check equality (with operator == which is defined in
// array_like.cpp) we check the bytes. However that doesn't always work for
// Unicode. Some strings which have different representation might be
// considered equal.
//    e.g. the character é can be represented either as 'é' or as '´'
//         combined with 'e' (two separate characters).
// Note that UTF-8 specifically requires the shortest-possible encoding for
// characters, but you still have to be careful (some outputs might not
// necessarily conform to that).
//
// @TODO @Robustness We should provide a string normalization function to deal
// with this
//

// Compares two utf-8 encoded strings and returns the index
// of the code point at which they are different or _-1_ if they are the same.
s64 compare(string s, string other);

// Compares two utf-8 encoded strings while ignoring case and returns the
// index of the code point at which they are different or _-1_ if they are the
// same.
s64 compare_ignore_case(string s, string other);

// Compares two utf-8 encoded strings lexicographically and returns:
//  -1 if _a_ is before _b_
//   0 if a == b
//   1 if _b_ is before _a_
s32 compare_lexicographically(string a, string b);

// Compares two utf-8 encoded strings lexicographically while ignoring case
// and returns:
//  -1 if _a_ is before _b_
//   0 if a == b
//   1 if _b_ is before _a_
s32 compare_lexicographically_ignore_case(string a, string b);

inline bool strings_match(string a, string b) { return compare(a, b) == -1; }
inline bool strings_match_ignore_case(string a, string b) {
  return compare_ignore_case(a, b) == -1;
}

// Returns true if _s_ begins with _str_
inline bool match_beginning(string s, string str) {
  if (str.Count > s.Count) return false;
  return memcmp(s.Data, str.Data, str.Count) == 0;
}

// Returns true if _s_ ends with _str_
inline bool match_end(string s, string str) {
  if (str.Count > s.Count) return false;
  return memcmp(s.Data + s.Count - str.Count, str.Data, str.Count) == 0;
}

// Returns a substring with white space removed at the start
inline string trim_start(string s) {
  auto p = [](code_point cp) { return !has(" \n\r\t\v\f", cp); };
  ;
  s64 start = search(s, &p);
  return slice(s, start, length(s));
}

// Returns a substring with white space removed at the end
inline string trim_end(string s) {
  auto p = [](code_point cp) { return !has(" \n\r\t\v\f", cp); };
  ;
  s64 end = search(s, &p, search_options{.Start = -1, .Reversed = true}) + 1;
  return slice(s, 0, end);
}

// Returns a substring with white space removed from both sides
inline string trim(string s) { return trim_end(trim_start(s)); }

// Changes the code point at _index_ to a new one.
// Since utf-8 code points are not the same byte count,
// this may need to reorder stuff and expand the string.
// So we assert that the string is dynamically allocated.
// You can use the next method in order to get finer control.
void set(string ref s, s64 index, code_point cp);

inline void maybe_grow(string ref s, s64 fit) {
  check_debug_memory(s);

  s64 space = s.Allocated;

  if (s.Count + fit <= space) return;

  s64 target = max(ceil_pow_of_2(s.Count + fit + 1), 8);
  reserve(s, target);
}

inline void insert_at_index(string ref s, s64 index, const char *str,
                            s64 size) {
  maybe_grow(s, size);

  index = translate_negative_index(index, length(s), true);
  auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

  s64 offset = translate_negative_index(t - s.Data, s.Count, true);
  auto *where = s.Data + offset;
  if (offset < s.Count) {
    memcpy(where + size, where, (s.Count - offset) * sizeof(*where));
  }
  memcpy(where, str, size * sizeof(*where));
  s.Count += size;
}

inline void insert_at_index(string ref s, s64 index, string str) {
  insert_at_index(s, index, str.Data, str.Count);
}

inline void insert_at_index(string ref s, s64 index, code_point cp) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);
  insert_at_index(s, index, encodedCp, utf8_get_size_of_cp(cp));
}

inline void add(string ref s, const char *ptr, s64 size) {
  insert_at_index(s, length(s), ptr, size);
}
inline void add(string ref s, string b) {
  insert_at_index(s, length(s), b.Data, b.Count);
}
inline void add(string ref s, code_point cp) {
  insert_at_index(s, length(s), cp);
}

inline string ref operator+=(string ref s, code_point cp) {
  add(s, cp);
  return s;
}

inline string ref operator+=(string ref s, string str) {
  add(s, str);
  return s;
}

inline string ref operator+=(string ref s, const char *str) {
  add(s, string(str));
  return s;
}

// Remove the first occurrence of a code point.
// Returns true on success (false if _cp_ was not found in the string).
bool remove(string ref s, code_point cp);

// Remove code point at specified index.
void remove_at_index(string ref s, s64 index);

// Remove a range of code points. [begin, end)
void remove_range(string ref s, s64 begin, s64 end);

void remove_all(string ref s, code_point search);
void remove_all(string ref s, string search);
void replace_all(string ref s, code_point search, code_point replace);
void replace_all(string ref s, code_point search, string replace);
void replace_all(string ref s, string search, code_point replace);
void replace_all(string ref s, string search, string replace);

// Returns a deep copy of _str_ and _count_
mark_as_leak inline string make_string(const char *str, s64 count) {
  string result;
  reserve(result, count);
  add(result, str, count);
  return result;
}

// Returns a deep copy of _str_
mark_as_leak inline string make_string(const char *str) {
  return make_string(str, c_string_length(str));
}

// Returns a deep copy of _src_
mark_as_leak inline string clone(string no_copy src) {
  return make_string(src.Data, src.Count);
}

// This iterator is to make range based for loops work.
template <bool Const>
struct string_iterator {
  using string_t = type_select_t<Const, const string, string>;

  string_t ref String;
  s64 Index;

  string_iterator(string_t ref s, s64 index = 0) : String(s), Index(index) {}

  string_iterator &operator++() {
    Index += 1;
    return *this;
  }

  string_iterator operator++(s32) {
    string_iterator temp = *this;
    ++(*this);
    return temp;
  }

  auto operator==(string_iterator other) const {
    return &String == &other.String && Index == other.Index;
  }
  auto operator!=(string_iterator other) const { return !(*this == other); }

  auto operator*() { return String[Index]; }
};

inline auto begin(string ref str) { return string_iterator<false>(str, 0); }
inline auto begin(string no_copy str) { return string_iterator<true>(str, 0); }
inline auto end(string ref str) {
  return string_iterator<false>(str, length(str));
}
inline auto end(string no_copy str) {
  return string_iterator<true>(str, length(str));
}

inline code_point get(string str, s64 index) {
  if (index < 0) {
    // @Speed... should we cache this in _string_?
    // We need to calculate the total length (in code points)
    // in order for the negative index to be converted properly.
    s64 len = length(str);
    index = translate_negative_index(index, len);

    // If LSTD_ARRAY_BOUNDS_CHECK is defined:
    // _utf8_get_pointer_to_cp_at_translated_index()_ also checks for out of
    // bounds but we are sure the index is valid (since translate_index also
    // checks for out of bounds) so we can get away with the unsafe version
    // here.
    auto *s = str.Data;
    For(range(index)) s += utf8_get_size_of_cp(s);
    return utf8_decode_cp(s);
  } else {
    auto *s =
        utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);
    return utf8_decode_cp(s);
  }
}

inline string slice(string str, s64 begin, s64 end) {
  s64 len = length(str);
  s64 beginIndex = translate_negative_index(begin, len, true);
  s64 endIndex = translate_negative_index(end, len, true);

  const char *beginPtr = utf8_get_pointer_to_cp_at_translated_index(
      str.Data, str.Count, beginIndex);
  const char *endPtr = beginPtr;

  // @Speed
  For(range(beginIndex, endIndex)) endPtr += utf8_get_size_of_cp(endPtr);

  return string((char *)beginPtr, (s64)(endPtr - beginPtr));
}

inline s64 search(string str, delegate<bool(code_point)> predicate,
                  search_options options) {
  if (!str.Data || str.Count == 0) return -1;
  s64 len = length(str);
  options.Start = translate_negative_index(options.Start, len, true);
  For(range(options.Start, options.Reversed ? -1 : len,
            options.Reversed ? -1 : 1)) if (predicate(get(str, it))) return it;
  return -1;
}

inline s64 search(string str, code_point search, search_options options) {
  if (!str.Data || str.Count == 0) return -1;
  s64 len = length(str);
  options.Start = translate_negative_index(options.Start, len, true);
  For(range(options.Start, options.Reversed ? -1 : len,
            options.Reversed ? -1 : 1)) if (get(str, it) == search) return it;
  return -1;
}

inline s64 search(string str, string search, search_options options) {
  if (!str.Data || str.Count == 0) return -1;
  if (!search.Data || search.Count == 0) return -1;

  s64 len = length(str);
  options.Start = translate_negative_index(options.Start, len, true);

  s64 searchLength = length(search);

  For(range(options.Start, options.Reversed ? -1 : len,
            options.Reversed ? -1 : 1)) {
    s64 progress = 0;
    for (s64 s = it; progress != searchLength; ++s, ++progress) {
      if (!(get(str, s) == get(search, progress))) break;
    }
    if (progress == searchLength) return it;
  }
  return -1;
}

inline bool has(string str, string s) { return search(str, s) != -1; }

inline bool has(string str, code_point cp) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);
  return search(str, string(encodedCp, utf8_get_size_of_cp(cp))) != -1;
}

inline s64 compare(string s, string other) {
  if (!s.Count && !other.Count) return -1;
  if (!s.Count || !other.Count) return 0;

  auto *p1 = s.Data, *p2 = other.Data;
  auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

  s64 index = 0;
  while (utf8_decode_cp(p1) == utf8_decode_cp(p2)) {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2) return -1;
    if (p1 == e1 || p2 == e2) return index;
    ++index;
  }
  return index;
}

inline s64 compare_ignore_case(string s, string other) {
  if (!s.Count && !other.Count) return -1;
  if (!s.Count || !other.Count) return 0;

  auto *p1 = s.Data, *p2 = other.Data;
  auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

  s64 index = 0;
  while (to_lower(utf8_decode_cp(p1)) == to_lower(utf8_decode_cp(p2))) {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2) return -1;
    if (p1 == e1 || p2 == e2) return index;
    ++index;
  }
  return index;
}

inline s32 compare_lexicographically(string a, string b) {
  if (!a.Count && !b.Count) return 0;
  if (!a.Count) return -1;
  if (!b.Count) return 1;

  auto *p1 = a.Data, *p2 = b.Data;
  auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

  s64 index = 0;
  while (utf8_decode_cp(p1) == utf8_decode_cp(p2)) {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2) return 0;
    if (p1 == e1) return -1;
    if (p2 == e2) return 1;
    ++index;
  }
  return ((s64)utf8_decode_cp(p1) - (s64)utf8_decode_cp(p2)) < 0 ? -1 : 1;
}

inline s32 compare_lexicographically_ignore_case(string a, string b) {
  if (!a.Count && !b.Count) return 0;
  if (!a.Count) return -1;
  if (!b.Count) return 1;

  auto *p1 = a.Data, *p2 = b.Data;
  auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

  s64 index = 0;
  while (to_lower(utf8_decode_cp(p1)) == to_lower(utf8_decode_cp(p2))) {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2) return 0;
    if (p1 == e1) return -1;
    if (p2 == e2) return 1;
    ++index;
  }
  return ((s64)to_lower(utf8_decode_cp(p1)) -
          (s64)to_lower(utf8_decode_cp(p2))) < 0
             ? -1
             : 1;
}

inline void replace_range(string ref str, s64 begin, s64 end, string replace) {
  s64 targetBegin = translate_negative_index(begin, str.Count);
  s64 targetEnd = translate_negative_index(end, str.Count, true);

  s64 whereSize = targetEnd - targetBegin;

  s64 diff = replace.Count - whereSize;

  if (diff > 0) {
    maybe_grow(str, diff);
  }

  auto where = str.Data + targetBegin;

  // Make space for the new elements
  memcpy(where + replace.Count, where + whereSize,
         (str.Count - targetBegin - whereSize) * sizeof(*where));

  // Copy replace elements
  memcpy(where, replace.Data, replace.Count * sizeof(*where));

  str.Count += diff;
}

inline void set(string ref str, s64 index, code_point cp) {
  check_debug_memory(str);

  index = translate_negative_index(index, length(str));

  const char *target =
      utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);

  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);

  string replace = string(encodedCp, utf8_get_size_of_cp(cp));
  s64 begin = target - str.Data;
  s64 end = target - str.Data + utf8_get_size_of_cp(target);
  replace_range(str, begin, end, replace);
}

mark_as_leak inline char *to_c_string(string s, allocator alloc) {
  char *result = malloc<char>({.Count = s.Count + 1, .Alloc = alloc});
  memcpy(result, s.Data, s.Count);
  result[s.Count] = '\0';
  return result;
}

inline bool remove(string ref s, code_point cp) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);

  s64 index = search(s, string(encodedCp, utf8_get_size_of_cp(cp)));
  if (index == -1) return false;

  remove_range(s, index, index + utf8_get_size_of_cp(cp));

  return true;
}

inline void remove_at_index(string ref s, s64 index) {
  index = translate_negative_index(index, length(s));

  auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

  s64 b = t - s.Data;
  remove_range(s, b, b + utf8_get_size_of_cp(t));
}

inline void remove_range(string ref s, s64 begin, s64 end) {
  check_debug_memory(s);

  s64 len = length(s);

  begin = translate_negative_index(begin, len);
  end = translate_negative_index(end, len, true);

  auto *tbp =
      utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, begin);
  auto *tep = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, end);

  begin = (s64)(tbp - s.Data);
  end = (s64)(tep - s.Data);

  s64 tp = translate_negative_index(begin, s.Count);
  s64 te = translate_negative_index(end, s.Count, true);

  auto where = s.Data + tp;
  auto whereEnd = s.Data + te;

  s64 elementCount = whereEnd - where;
  memcpy(where, whereEnd, (s.Count - tp - elementCount) * sizeof(*where));
  s.Count -= elementCount;
}

inline void replace_all(string ref s, string what, string replace) {
  // @CutAndPaste from array-like's replace_all.
  // @Volatile
  check_debug_memory(s);

  if (!s.Data || !s.Count) return;

  assert(what.Data && what.Count);
  if (replace.Count) assert(replace.Data);

  if (what.Count == replace.Count) {
    // This case we can handle relatively fast.
    // @Speed Improve by using bit hacks for the case when the elements are less
    // than a pointer size?
    auto *p = s.Data;
    auto *e = s.Data + s.Count;
    while (p != e) {
      // @Speed We can do simply memcmp for scalar types and types that don't
      // haveoverloaded ==.
      if (*p == what[0]) {
        auto *n = p;
        auto *sp = what.Data;
        auto *se = what.Data + what.Count;
        while (n != e && sp != se) {
          // Require only operator == to be defined (and not !=).
          if (!(*n == *sp)) break;
          ++n, ++sp;
        }

        if (sp == se) {
          // Match found
          memcpy(p, replace.Data, replace.Count * sizeof(*p));
          p += replace.Count;
        } else {
          ++p;
        }
      } else {
        ++p;
      }
    }
  } else {
    //
    // @Speed This is the slow and dumb version for now.
    // We can improve performance by either:
    // * Allocating a buffer first which holds the result (space cost increases)
    // * Doing two passes, first one counting the number of occurrences
    //   so we know the offsets for the second pass.
    // Though the second option would only work if what.Count > replace.Count.
    //
    // I think going with the former makes the most sense,
    // however at that point letting the caller write their own routine
    // will probably be better, since we can't for sure know the context
    // and if allocating another (possibly big) array is fine.
    //
    s64 diff = replace.Count - what.Count;

    s64 i = 0;
    while (i < s.Count &&
           (i = search(s, what, search_options{.Start = i})) != -1) {
      replace_range(s, i, i + what.Count,
                    replace);  // @Speed Slow and dumb version for now
      i += replace.Count;
    }
  }
}

inline void replace_all(string ref s, code_point what, code_point replace) {
  char encodedOld[4];
  utf8_encode_cp(encodedOld, what);

  char encodedNew[4];
  utf8_encode_cp(encodedNew, replace);

  replace_all(s, string(encodedOld, utf8_get_size_of_cp(encodedOld)),
              string(encodedNew, utf8_get_size_of_cp(encodedNew)));
}

inline void remove_all(string ref s, code_point what) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, what);

  replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), string(""));
}

inline void remove_all(string ref s, string what) {
  replace_all(s, what, string(""));
}

inline void replace_all(string ref s, code_point what, string replace) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, what);

  replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), replace);
}

inline void replace_all(string ref s, string what, code_point replace) {
  char encodedCp[4];
  utf8_encode_cp(encodedCp, replace);

  replace_all(s, what, string(encodedCp, utf8_get_size_of_cp(encodedCp)));
}

LSTD_END_NAMESPACE
