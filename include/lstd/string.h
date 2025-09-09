#pragma once

#include "delegate.h"
#include "memory.h"
#include "stack_array.h"
#include "unicode.h"

LSTD_BEGIN_NAMESPACE

///
/// The following utility functions are for ASCII and utf-8 null-terminated
/// strings:
///

// * c_string_byte_count - strlen
// * utf8_length
// * compare_string
// * compare_string_lexicographically - strcmp
// * compare_string_ignore_case
// * compare_string_lexicographically_ignore_case
// * strings_match
//
// Working with utf-8 encoding:
// * utf8_get_size_of_cp
// * utf8_encode_cp
// * utf8_decode_cp
// * utf8_is_valid_cp
//
//
// Working with unicode code points, with fast O(1) look-up tables, generated from tools/gen_unicode.py
// * unicode_to_upper
// * unicode_to_lower
// * unicode_is_upper
// * unicode_is_lower
//
// Versions that work only for ascii:
// * ascii_to_upper             - toupper
// * ascii_to_lower             - tolower
// * ascii_is_upper             - isupper
// * ascii_is_lower             - islower
// * ascii_is_digit             - isdigit
// * ascii_is_hex_digit         - isxdigit
// * ascii_is_space             - isspace
// * ascii_is_blank             - isblank
// * ascii_is_alpha             - isalpha
// * ascii_is_alphanumeric      - isalnum
// * ascii_is_print             - isprint
// * ascii_is_identifier_start
//
// Conversions:
// * utf8_to_utf16
// * utf8_to_utf32
// * utf16_to_utf8
// * utf32_to_utf8
//

template <typename C>
using c_string_type = add_pointer_t<remove_cvref_t<remove_pointer_t<remove_cvref_t<C>>>>;

// C++ mess
template <typename C>
concept any_c_string = is_pointer<C> && is_same_to_one_of<c_string_type<C>, char *, wchar *, char8_t *, char16_t *, char32_t *, code_point *>;

//
// == strlen
//
// The length of a null-terminated string. Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// @Speed @TODO Vectorize
s64 c_string_byte_count(any_c_string auto s)
{
  if (!s)
    return 0;

  s64 length = 0;
  while (*s++)
    ++length;
  return length;
}

// The length (in code points) of a utf-8 string
// @Speed @TODO Vectorize
inline s64 utf8_length(const char *str, s64 size)
{
  if (!str || size == 0)
    return 0;

  // Count all first-bytes (the ones that don't match 10xxxxxx).
  s64 length = 0;
  while (size--)
  {
    if (!((*str++ & 0xc0) == 0x80))
      ++length;
  }
  return length;
}

// == strcmp
//
// Return -1 if one < other, 0 if one == other and 1 if one > other (not the
// pointers)
// @Speed @TODO Vectorize
template <any_c_string C>
s32 c_string_order(C one, C other)
{
  assert(one);
  assert(other);

  while (*one && (*one == *other))
    ++one, ++other;
  return (*one > *other) - (*other > *one);
}

// Returns -1 if strings match, else returns
// the index of the first different byte
// @Speed @TODO Vectorize
template <any_c_string C>
s64 c_string_find_first_diff(C one, C other)
{
  assert(one);
  assert(other);

  if (!*one && !*other)
    return -1;

  s64 index = 0;
  while (*one == *other)
  {
    ++one, ++other;
    if (!*one && !*other)
      return -1;
    if (!*one || !*other)
      return index;
    ++index;
  }
  return index;
}

inline char ascii_to_upper(char x)
{
  if (x >= 'a' && x <= 'z')
    return x - 32;
  return x;
}

inline char ascii_to_lower(char x)
{
  if (x >= 'A' && x <= 'Z')
    return x + 32;
  return x;
}

inline bool ascii_is_upper(char x) { return x >= 'A' && x <= 'Z'; }
inline bool ascii_is_lower(char x) { return x >= 'a' && x <= 'z'; }

inline bool ascii_is_digit(char x) { return x >= '0' && x <= '9'; }

inline bool ascii_is_hex_digit(char x)
{
  return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') ||
         (x >= 'A' && x <= 'F');
}

inline bool ascii_is_space(char x) { return (x >= 9 && x <= 13) || x == 32; }

inline bool ascii_is_blank(char x) { return x == 9 || x == 32; }

inline bool ascii_is_alpha(char x)
{
  return (x >= 65 && x <= 90) || (x >= 97 && x <= 122);
}

inline bool ascii_is_alphanumeric(char x) { return ascii_is_alpha(x) || ascii_is_digit(x); }

inline bool ascii_is_identifier_start(char x) { return ascii_is_alpha(x) || x == '_'; }

inline bool ascii_is_print(char x) { return x > 31 && x != 127; }

// Returns -1 if strings match, else returns the index of the first different
// byte. Ignores the case of the characters.
// @Speed @TODO Vectorize
template <any_c_string C>
s64 compare_string_ignore_case(C one, C other)
{
  assert(one);
  assert(other);

  if (!*one && !*other)
    return -1;

  s64 index = 0;
  while (to_lower(*one) == to_lower(*other))
  {
    ++one, ++other;
    if (!*one && !*other)
      return -1;
    if (!*one || !*other)
      return index;
    ++index;
  }
  return index;
}

// Return -1 if one < other, 0 if one == other and 1 if one > other (not the
// pointers). Ignores the case of the characters.
// @Speed @TODO Vectorize
template <any_c_string C>
s32 compare_string_lexicographically_ignore_case(C one, C other)
{
  assert(one);
  assert(other);

  while (*one && (to_lower(*one) == to_lower(*other)))
    ++one, ++other;
  return (*one > *other) - (*other > *one);
}

// true if strings are equal (not the pointers)
template <any_c_string C>
bool strings_match(C one, C other)
{
  return c_string_find_first_diff(one, other) == -1;
}

// true if strings are equal (not the pointers)
template <any_c_string C>
bool strings_match_ignore_case(C one, C other)
{
  return compare_string_ignore_case(one, other) == -1;
}

// Returns the size in bytes of the code point that _str_ points to.
// If the byte pointed by _str_ is a countinuation utf-8 byte, this function
// returns 0.
inline s8 utf8_get_size_of_cp(const char *str)
{
  if (!str)
    return 0;
  if ((*str & 0xc0) == 0x80)
    return 0;

  if (0xf0 == (0xf8 & str[0]))
  {
    return 4;
  }
  else if (0xe0 == (0xf0 & str[0]))
  {
    return 3;
  }
  else if (0xc0 == (0xe0 & str[0]))
  {
    return 2;
  }
  else
  {
    return 1;
  }
}

// Returns the size that the code point would be if it were encoded
inline s8 utf8_get_size_of_cp(code_point codePoint)
{
  if (((s32)0xffffff80 & codePoint) == 0)
  {
    return 1;
  }
  else if (((s32)0xfffff800 & codePoint) == 0)
  {
    return 2;
  }
  else if (((s32)0xffff0000 & codePoint) == 0)
  {
    return 3;
  }
  else
  {
    return 4;
  }
}

// Encodes code point at _str_, assumes there is enough space
inline void utf8_encode_cp(char *str, code_point codePoint)
{
  s64 size = utf8_get_size_of_cp(codePoint);
  if (size == 1)
  {
    // 1-byte/7-bit ascii
    // (0b0xxxxxxx)
    str[0] = (char)codePoint;
  }
  else if (size == 2)
  {
    // 2-byte/11-bit utf-8 code point
    // (0b110xxxxx 0b10xxxxxx)
    str[0] = 0xc0 | (char)(codePoint >> 6);
    str[1] = 0x80 | (char)(codePoint & 0x3f);
  }
  else if (size == 3)
  {
    // 3-byte/16-bit utf-8 code point
    // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
    str[0] = 0xe0 | (char)(codePoint >> 12);
    str[1] = 0x80 | (char)((codePoint >> 6) & 0x3f);
    str[2] = 0x80 | (char)(codePoint & 0x3f);
  }
  else
  {
    // 4-byte/21-bit utf-8 code point
    // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
    str[0] = 0xf0 | (char)(codePoint >> 18);
    str[1] = 0x80 | (char)((codePoint >> 12) & 0x3f);
    str[2] = 0x80 | (char)((codePoint >> 6) & 0x3f);
    str[3] = 0x80 | (char)(codePoint & 0x3f);
  }
}

// Decodes a code point from a data pointer
inline code_point utf8_decode_cp(const char *str)
{
  if (0xf0 == (0xf8 & str[0]))
  {
    // 4 byte utf-8 code point
    return ((0x07 & str[0]) << 18) | ((0x3f & str[1]) << 12) |
           ((0x3f & str[2]) << 6) | (0x3f & str[3]);
  }
  else if (0xe0 == (0xf0 & str[0]))
  {
    // 3 byte utf-8 code point
    return ((0x0f & str[0]) << 12) | ((0x3f & str[1]) << 6) | (0x3f & str[2]);
  }
  else if (0xc0 == (0xe0 & str[0]))
  {
    // 2 byte utf-8 code point
    return ((0x1f & str[0]) << 6) | (0x3f & str[1]);
  }
  else
  {
    // 1 byte utf-8 code point
    return str[0];
  }
}

// Checks whether the encoded code point in data is valid utf-8
inline bool utf8_is_valid_cp(const char *data)
{
  u8 *p = (u8 *)data;

  s64 sizeOfCp = utf8_get_size_of_cp(data);
  if (sizeOfCp == 1)
  {
    if ((s8)*data < 0)
      return false;
  }
  else if (sizeOfCp == 2)
  {
    if (*p < 0xC2 || *p > 0xDF)
      return false;
    ++p;
    if (*p < 0x80 || *p > 0xBF)
      return false;
  }
  else if (sizeOfCp == 3)
  {
    if (*p == 0xE0)
    {
      ++p;
      if (*p < 0xA0 || *p > 0xBF)
        return false;
    }
    else if (*p >= 0xE1 && *p <= 0xEC)
    {
      ++p;
      if (*p < 0x80 || *p > 0xBF)
        return false;
    }
    else if (*p == 0xED)
    {
      ++p;
      if (*p < 0x80 || *p > 0x9F)
        return false;
    }
    else if (*p >= 0xEE && *p <= 0xEF)
    {
      ++p;
      if (*p < 0x80 || *p > 0xBF)
        return false;
    }
    else
    {
      return false;
    }
    // The third byte restriction is the same on all of these
    ++p;
    if (*p < 0x80 || *p > 0xBF)
      return false;
  }
  else if (sizeOfCp == 4)
  {
    if (*p == 0xF0)
    {
      ++p;
      if (*p < 0x90 || *p > 0xBF)
        return false;
    }
    else if (*p >= 0xF1 && *p <= 0xF3)
    {
      ++p;
      if (*p < 0x80 || *p > 0xBF)
        return false;
    }
    else if (*p == 0xF4)
    {
      ++p;
      if (*p < 0x80 || *p > 0x8F)
        return false;
    }
    else
    {
      return false;
    }
    // The third and fourth byte restriction is the same on all of these
    ++p;
    if (*p < 0x80 || *p > 0xBF)
      return false;
  }
  else
  {
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
                                                              s64 index)
{
  auto *end = str + byteLength;

  For(range(index))
  {
    // Danger danger. If the string contains invalid utf8, then we might bypass
    // str == end. That's why we check with >=.

    if (str >= end)
    {
      assert(false && "Out of bounds");
    }

    str += utf8_get_size_of_cp(str);
  }
  return str;
}

// Converts utf-8 to utf-16 and stores in _out_ (assumes there is enough space).
// Also adds a null-terminator at the end.
inline void utf8_to_utf16(const char *str, s64 length, wchar *out)
{
  For(range(length))
  {
    code_point cp = utf8_decode_cp(str);
    if (cp > 0xffff)
    {
      *out++ = (u16)((cp >> 10) + (0xD800u - (0x10000 >> 10)));
      *out++ = (u16)((cp & 0x3FF) + 0xDC00u);
    }
    else
    {
      *out++ = (u16)cp;
    }
    str += utf8_get_size_of_cp(cp);
  }
  *out = 0;
}

// Converts utf-8 to utf-32 and stores in _out_ (assumes there is enough space).
//
// Also adds a null-terminator at the end.
inline void utf8_to_utf32(const char *str, s64 byteLength, code_point *out)
{
  auto *end = str + byteLength;

  // Danger danger. If the string contains invalid utf8, then we might bypass
  // str != end and infinite loop. That's why we check with <.
  while (str < end)
  {
    code_point cp = utf8_decode_cp(str);
    *out++ = cp;
    str += utf8_get_size_of_cp(cp);
  }

  *out = 0;
}

// Converts a null-terminated utf-16 to utf-8 and stores in _out_ and
// _outByteLength_ (assumes there is enough space).
inline void utf16_to_utf8(const wchar *str, char *out, s64 *outByteLength)
{
  s64 byteLength = 0;
  while (*str)
  {
    code_point cp = *str;
    if ((cp >= 0xD800) && (cp <= 0xDBFF))
    {
      code_point trail = cp = *++str;
      if (!*str)
        assert(false &&
               "Invalid wchar string"); // @TODO @Robustness Bail on errors

      if ((trail >= 0xDC00) && (trail <= 0xDFFF))
      {
        cp = ((cp - 0xD800) << 10) + (trail - 0xDC00) + 0x0010000;
      }
      else
      {
        assert(false &&
               "Invalid wchar string"); // @TODO @Robustness Bail on errors
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
                          s64 *outByteLength)
{
  s64 byteLength = 0;
  while (*str)
  {
    utf8_encode_cp(out, *str);
    s64 cpSize = utf8_get_size_of_cp(out);
    out += cpSize;
    byteLength += cpSize;
    ++str;
  }
  *outByteLength = byteLength;
}

// Validates that the given UTF-8 buffer contains only well-formed sequences.
// Does not modify input; runs in O(n). Returns the index of the first
// invalid byte, if found, -1 otherwise.
inline s64 utf8_find_invalid(const char *str, s64 byteLength)
{
  assert(byteLength >= 0);
  if (!str || byteLength == 0)
    return -1;

  const char *p = (const char *)str;
  const char *end = p + byteLength;
  while (p < end)
  {
    // Size based on first byte; 0 means continuation byte at head -> invalid
    s64 cpSize = utf8_get_size_of_cp(p);
    if (cpSize <= 0)
      return p - str;
    if ((end - p) < cpSize)
      return p - str; // truncated sequence
    if (!utf8_is_valid_cp(p))
      return p - str;
    p += cpSize;
  }
  return -1;
}

// Decompose + canonical reorder = NFD
// Returns number of code points in segBuf
inline bool utf8_segment_nfd(const char *&p, const char *end, stack_array<code_point, 1024> &segBuf, s64 &segN)
{
  segN = 0;
  if (p >= end)
    return false; // Decode first code point (starter)
  s64 sz = utf8_get_size_of_cp(p);
  if (sz <= 0 || p + sz > end)
    return false;
  if (!utf8_is_valid_cp(p))
    return false;
  code_point first = utf8_decode_cp(p);
  p += sz; // Full canonical decomposition
  auto decompose_full_into_seg = [&](code_point cp0)
  { constexpr s64 MAX_STACK = 64; constexpr s64 MAX_TMP_DECOMP = 8; stack_array<code_point, MAX_STACK> stk; s64 sp = 0; stk.Data[sp++] = cp0; while (sp > 0) { code_point x = stk.Data[--sp]; code_point tmp[MAX_TMP_DECOMP]; s32 n = unicode_canonical_decompose(x, tmp, MAX_TMP_DECOMP); if (n > 1) { for (s32 i = n - 1; i >= 0; --i) { if (sp < MAX_STACK) stk.Data[sp++] = tmp[i]; } } else { if (segN < 1024) segBuf.Data[segN++] = x; } } };
  decompose_full_into_seg(first); // Append following non-starters
  while (p < end)
  {
    s64 sz2 = utf8_get_size_of_cp(p);
    if (sz2 <= 0 || p + sz2 > end)
      return false;
    if (!utf8_is_valid_cp(p))
      return false;
    code_point c = utf8_decode_cp(p);
    u8 cc = unicode_combining_class(c);
    if (cc == 0)
      break; // next segment starts
    decompose_full_into_seg(c);
    p += sz2;
    if (segN >= 1024)
      break;
  } // Canonical reorder (stable sort by CCC, indices >= 1)
  if (segN > 1)
  {
    for (s64 i = 2; i < segN; ++i)
    {
      code_point key = segBuf.Data[i];
      u8 key_cc = unicode_combining_class(key);
      s64 j = i - 1;
      while (j >= 1 && unicode_combining_class(segBuf.Data[j]) > key_cc)
      {
        segBuf.Data[j + 1] = segBuf.Data[j];
        --j;
      }
      segBuf.Data[j + 1] = key;
    }
  }
  return true;
}

// Functions which normalize utf-8 strings to string builder are defined in "string_builder.h",
// because C++ templates are fun like that.

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

struct string
{
  char *Data = null;
  s64 Count = 0;
  s64 Allocated = 0;

  // We can't treat it as array-like, because for strings
  // we work with indices which are code point based (and not char based).
  static const bool TREAT_AS_ARRAY_LIKE = false;

  string() {}

  // This constructs a view (use make_string to copy)
  string(const char *str) : Data((char *)str), Count(c_string_byte_count(str)) {}

  // This constructs a view (use make_string to copy)
  string(const char8_t *str) : Data((char *)str), Count(c_string_byte_count(str)) {}

  // This constructs a view (use make_string to copy)
  string(const char *str, s64 count) : Data((char *)str), Count(count) {}

  // A special structure which is returned from string_get(),
  // which allows to see the code point at a specific index, but also update
  // it.
  struct code_point_ref
  {
    string ref String;
    s64 Index;

    code_point_ref(string ref s, s64 index) : String(s)
    {
      Index = translate_negative_index(index, length(s));
    }

    code_point_ref &operator=(code_point other)
    {
      set(String, Index, other);
      return *this;
    }

    operator code_point() const
    {
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
  code_point operator[](s64 index) const
  {
    return utf8_decode_cp(
        utf8_get_pointer_to_cp_at_translated_index(Data, length(*this), index));
  }
};

inline void reserve(string ref s, s64 n = -1, allocator alloc = {})
{
  if (n <= 0)
  {
    n = max(s.Count, 8);
  }
  assert(n >= 1);

  using T = remove_pointer_t<decltype(s.Data)>;

  auto *oldData = s.Data;
  if (s.Allocated)
  {
    s.Data = realloc(s.Data, {.NewCount = n});
  }
  else
  {
    // Not our job to free oldData since we don't own it.
    // For subsequent reserves we go through the   realloc   branch above,
    // which properly manages to free the old data (if it couldn't reallocate
    // in place, that is).
    s.Data = malloc<T>(
        {.Count = n,
         .Alloc = alloc}); // If alloc is null we use the Context's allocator
    if (oldData)
      memcpy(s.Data, oldData, s.Count * sizeof(T));
  }

  s.Allocated = n;
}

inline void check_debug_memory(string no_copy s)
{
#if defined DEBUG_MEMORY
  //
  // If you assert here, there are two possible reasons:
  //
  // 1. You created a string from a literal or from memory that wasn't
  // allocated with lstd's allocators, but then you called a function that
  // modifies the string (like set() or operator[] to change a code point or
  // reserve()).
  //
  // Our memory layer keeps track of all allocations, so if you try to
  // modify a string that wasn't allocated with it, it will assert here.
  //
  // 2. Attempting to modify an string from another thread...
  // Caution! This container is not thread-safe!
  //
  // Each thread has it's own memory list and we add this check
  // to make sure that you don't try to modify a string from a different
  // thread than the one that created it.
  //
  if (s.Allocated)
  {
    assert(debug_memory_list_contains((allocation_header *)s.Data - 1));
  }
#endif
}

inline void free(string ref s)
{
  if (s.Allocated && s.Data)
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
s64 search_opt(string str, delegate<bool(code_point)> predicate, search_options options = {});

// Returns the code point index (or -1) if not found.
s64 search_opt(string str, code_point search, search_options options = {});

// Returns the code point index (or -1) if not found.
s64 search_opt(string str, string search, search_options options = {});

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
inline bool strings_match_ignore_case(string a, string b)
{
  return compare_ignore_case(a, b) == -1;
}

// Returns true if _s_ begins with _str_
inline bool match_beginning(string s, string str)
{
  if (str.Count > s.Count)
    return false;
  return memcmp(s.Data, str.Data, str.Count) == 0;
}

// Returns true if _s_ ends with _str_
inline bool match_end(string s, string str)
{
  if (str.Count > s.Count)
    return false;
  return memcmp(s.Data + s.Count - str.Count, str.Data, str.Count) == 0;
}

// Returns a substring with white space removed at the start
inline string trim_start(string s)
{
  auto p = [](code_point cp)
  { return !has(" \n\r\t\v\f", cp); };
  ;
  s64 start = search(s, &p);
  return slice(s, start, length(s));
}

// Returns a substring with white space removed at the end
inline string trim_end(string s)
{
  auto p = [](code_point cp)
  { return !has(" \n\r\t\v\f", cp); };
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

inline void maybe_grow(string ref s, s64 fit)
{
  check_debug_memory(s);

  s64 target = max(ceil_pow_of_2(s.Count + fit + 1), 8);

  s64 space = s.Allocated;
  if (!space)
  {
    reserve(s, target);
    return;
  }

  if (s.Count + fit <= space)
    return;
  reserve(s, target);
}

inline void insert_at_index(string ref s, s64 index, const char *str,
                            s64 size)
{
  maybe_grow(s, size);

  index = translate_negative_index(index, length(s), true);
  auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

  s64 offset = translate_negative_index(t - s.Data, s.Count, true);
  auto *where = s.Data + offset;
  if (offset < s.Count)
  {
    memcpy(where + size, where, (s.Count - offset) * sizeof(*where));
  }
  memcpy(where, str, size * sizeof(*where));
  s.Count += size;
}

inline void insert_at_index(string ref s, s64 index, string str)
{
  insert_at_index(s, index, str.Data, str.Count);
}

inline void insert_at_index(string ref s, s64 index, code_point cp)
{
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);
  insert_at_index(s, index, encodedCp, utf8_get_size_of_cp(cp));
}

inline void add(string ref s, const char *ptr, s64 size)
{
  insert_at_index(s, length(s), ptr, size);
}
inline void add(string ref s, string b)
{
  insert_at_index(s, length(s), b.Data, b.Count);
}
inline void add(string ref s, code_point cp)
{
  insert_at_index(s, length(s), cp);
}

inline string ref operator+=(string ref s, code_point cp)
{
  add(s, cp);
  return s;
}

inline string ref operator+=(string ref s, string str)
{
  add(s, str);
  return s;
}

inline string ref operator+=(string ref s, const char *str)
{
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

// Remove a range of bytes. [byte_begin, byte_end) - for internal use
void remove_range_bytes(string ref s, s64 byte_begin, s64 byte_end);

// Replace a range of code points. [begin, end) with replacement string
void replace_range(string ref str, s64 begin, s64 end, string replace);

// Replace a range of bytes. [byte_begin, byte_end) with replacement string - for internal use
void replace_range_bytes(string ref str, s64 byte_begin, s64 byte_end, string replace);

void remove_all(string ref s, code_point search);
void remove_all(string ref s, string search);
void replace_all(string ref s, code_point search, code_point replace);
void replace_all(string ref s, code_point search, string replace);
void replace_all(string ref s, string search, code_point replace);
void replace_all(string ref s, string search, string replace);

// Returns a deep copy of _str_ and _count_
mark_as_leak inline string make_string(const char *str, s64 count)
{
  string result;
  reserve(result, count);
  add(result, str, count);
  return result;
}

// Returns a deep copy of _str_
mark_as_leak inline string make_string(const char *str)
{
  return make_string(str, c_string_byte_count(str));
}

// Returns a deep copy of _src_
mark_as_leak inline string clone(string no_copy src)
{
  return make_string(src.Data, src.Count);
}

// This iterator is to make range based for loops work.
template <bool Const>
struct string_iterator
{
  using string_t = type_select_t<Const, const string, string>;

  string_t ref String;
  s64 Index;

  string_iterator(string_t ref s, s64 index = 0) : String(s), Index(index) {}

  string_iterator &operator++()
  {
    Index += 1;
    return *this;
  }

  string_iterator operator++(s32)
  {
    string_iterator temp = *this;
    ++(*this);
    return temp;
  }

  auto operator==(string_iterator other) const
  {
    return &String == &other.String && Index == other.Index;
  }
  auto operator!=(string_iterator other) const { return !(*this == other); }

  auto operator*() { return String[Index]; }
};

inline auto begin(string ref str) { return string_iterator<false>(str, 0); }
inline auto begin(string no_copy str) { return string_iterator<true>(str, 0); }
inline auto end(string ref str)
{
  return string_iterator<false>(str, length(str));
}
inline auto end(string no_copy str)
{
  return string_iterator<true>(str, length(str));
}

inline code_point get(string str, s64 index)
{
  if (index < 0)
  {
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
  }
  else
  {
    auto *s =
        utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);
    return utf8_decode_cp(s);
  }
}

inline string slice(string str, s64 begin, s64 end)
{
  s64 len = length(str);
  if (len == 0)
    return string();

  s64 beginIndex = translate_negative_index(begin, len, true);
  s64 endIndex = translate_negative_index(end, len, true);

  const char *beginPtr = utf8_get_pointer_to_cp_at_translated_index(
      str.Data, str.Count, beginIndex);
  const char *endPtr = beginPtr;

  // @Speed
  For(range(beginIndex, endIndex)) endPtr += utf8_get_size_of_cp(endPtr);

  return string((char *)beginPtr, (s64)(endPtr - beginPtr));
}

inline s64 search_opt(string str, delegate<bool(code_point)> predicate,
                      search_options options)
{
  if (!str.Data || str.Count == 0)
    return -1;
  s64 len = length(str);

  // Handle out-of-bounds start gracefully for search operations
  if (options.Start >= len || options.Start < -len)
    return -1;

  options.Start = translate_negative_index(options.Start, len, true);
  For(range(options.Start, options.Reversed ? -1 : len,
            options.Reversed ? -1 : 1)) if (predicate(get(str, it))) return it;
  return -1;
}

inline s64 search_opt(string str, code_point search, search_options options)
{
  if (!str.Data || str.Count == 0)
    return -1;
  s64 len = length(str);

  // Handle out-of-bounds start gracefully for search operations
  if (options.Start >= len || options.Start < -len)
    return -1;

  options.Start = translate_negative_index(options.Start, len, true);
  For(range(options.Start, options.Reversed ? -1 : len,
            options.Reversed ? -1 : 1)) if (get(str, it) == search) return it;
  return -1;
}

inline s64 search_opt(string str, string search, search_options options)
{
  if (!str.Data || str.Count == 0)
    return -1;
  if (!search.Data || search.Count == 0)
    return -1;

  s64 len = length(str);

  // Handle out-of-bounds start gracefully for search operations
  if (options.Start >= len || options.Start < -len)
    return -1;

  options.Start = translate_negative_index(options.Start, len, true);

  s64 searchLength = length(search);

  For(range(options.Start, options.Reversed ? -1 : len, options.Reversed ? -1 : 1))
  {
    s64 progress = 0;
    for (s64 s = it; progress != searchLength && s < len; ++s, ++progress)
    {
      if (!(get(str, s) == get(search, progress)))
        break;
    }
    if (progress == searchLength)
      return it;
  }
  return -1;
}

inline bool has(string str, string s) { return search(str, s) != -1; }

inline bool has(string str, code_point cp)
{
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);
  return search(str, string(encodedCp, utf8_get_size_of_cp(cp))) != -1;
}

inline s64 compare(string s, string other)
{
  if (!s.Count && !other.Count)
    return -1;
  if (!s.Count || !other.Count)
    return 0;

  auto *p1 = s.Data, *p2 = other.Data;
  auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

  s64 index = 0;
  while (utf8_decode_cp(p1) == utf8_decode_cp(p2))
  {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2)
      return -1;
    if (p1 == e1 || p2 == e2)
      return index;
    ++index;
  }
  return index;
}

inline s64 compare_ignore_case(string s, string other)
{
  if (!s.Count && !other.Count)
    return -1;
  if (!s.Count || !other.Count)
    return 0;

  auto *p1 = s.Data, *p2 = other.Data;
  auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

  s64 index = 0;
  while (unicode_to_lower(utf8_decode_cp(p1)) == unicode_to_lower(utf8_decode_cp(p2)))
  {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2)
      return -1;
    if (p1 == e1 || p2 == e2)
      return index;
    ++index;
  }
  return index;
}

inline s32 compare_lexicographically(string a, string b)
{
  if (!a.Count && !b.Count)
    return 0;
  if (!a.Count)
    return -1;
  if (!b.Count)
    return 1;

  auto *p1 = a.Data, *p2 = b.Data;
  auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

  s64 index = 0;
  while (utf8_decode_cp(p1) == utf8_decode_cp(p2))
  {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2)
      return 0;
    if (p1 == e1)
      return -1;
    if (p2 == e2)
      return 1;
    ++index;
  }
  return ((s64)utf8_decode_cp(p1) - (s64)utf8_decode_cp(p2)) < 0 ? -1 : 1;
}

inline s32 compare_lexicographically_ignore_case(string a, string b)
{
  if (!a.Count && !b.Count)
    return 0;
  if (!a.Count)
    return -1;
  if (!b.Count)
    return 1;

  auto *p1 = a.Data, *p2 = b.Data;
  auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

  s64 index = 0;
  while (unicode_to_lower(utf8_decode_cp(p1)) == unicode_to_lower(utf8_decode_cp(p2)))
  {
    p1 += utf8_get_size_of_cp(p1);
    p2 += utf8_get_size_of_cp(p2);
    if (p1 == e1 && p2 == e2)
      return 0;
    if (p1 == e1)
      return -1;
    if (p2 == e2)
      return 1;
    ++index;
  }
  return ((s64)unicode_to_lower(utf8_decode_cp(p1)) -
          (s64)unicode_to_lower(utf8_decode_cp(p2))) < 0
             ? -1
             : 1;
}

inline void replace_range(string ref str, s64 begin, s64 end, string replace)
{
  s64 len = length(str);
  if (len == 0)
    return;

  begin = translate_negative_index(begin, len);
  end = translate_negative_index(end, len, true);

  auto *tbp = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, begin);
  auto *tep = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, end);

  s64 byteBegin = (s64)(tbp - str.Data);
  s64 byteEnd = (s64)(tep - str.Data);

  replace_range_bytes(str, byteBegin, byteEnd, replace);
}

inline void replace_range_bytes(string ref str, s64 byte_begin, s64 byte_end, string replace)
{
  s64 whereSize = byte_end - byte_begin;
  s64 diff = replace.Count - whereSize;

  maybe_grow(str, diff);

  // Recalculate pointers after potential reallocation
  auto where = str.Data + byte_begin;
  auto whereEnd = str.Data + byte_end;

  // Make space for the new elements
  memcpy(where + replace.Count, where + whereSize,
         (str.Count - byte_end) * sizeof(*where));

  // Copy replace elements
  memcpy(where, replace.Data, replace.Count * sizeof(*where));

  str.Count += diff;
}

inline void set(string ref str, s64 index, code_point cp)
{
  check_debug_memory(str);

  index = translate_negative_index(index, length(str));

  const char *target = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);

  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);

  string replace = string(encodedCp, utf8_get_size_of_cp(cp));
  s64 byte_begin = target - str.Data;
  s64 byte_end = target - str.Data + utf8_get_size_of_cp(target);
  replace_range_bytes(str, byte_begin, byte_end, replace);
}

mark_as_leak inline char *to_c_string(string s, allocator alloc)
{
  char *result = malloc<char>({.Count = s.Count + 1, .Alloc = alloc});
  memcpy(result, s.Data, s.Count);
  result[s.Count] = '\0';
  return result;
}

inline bool remove(string ref s, code_point cp)
{
  char encodedCp[4];
  utf8_encode_cp(encodedCp, cp);

  s64 index = search(s, string(encodedCp, utf8_get_size_of_cp(cp)));
  if (index == -1)
    return false;

  remove_range(s, index, index + 1); // Remove 1 code point

  return true;
}

inline void remove_at_index(string ref s, s64 index)
{
  index = translate_negative_index(index, length(s));

  auto *t = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, index);

  s64 b = t - s.Data;
  remove_range_bytes(s, b, b + utf8_get_size_of_cp(t));
}

inline void remove_range_bytes(string ref s, s64 byte_begin, s64 byte_end)
{
  auto where = s.Data + byte_begin;
  auto whereEnd = s.Data + byte_end;

  s64 elementCount = whereEnd - where;
  memcpy(where, whereEnd, (s.Count - byte_end) * sizeof(*where));
  s.Count -= elementCount;
}

inline void remove_range(string ref s, s64 begin, s64 end)
{
  check_debug_memory(s);

  s64 len = length(s);
  if (len == 0)
    return;

  begin = translate_negative_index(begin, len);
  end = translate_negative_index(end, len, true);

  auto *tbp = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, begin);
  auto *tep = utf8_get_pointer_to_cp_at_translated_index(s.Data, s.Count, end);

  s64 byteBegin = (s64)(tbp - s.Data);
  s64 byteEnd = (s64)(tep - s.Data);

  remove_range_bytes(s, byteBegin, byteEnd);
}

inline void replace_all(string ref s, string what, string replace)
{
  // @CutAndPaste from array-like's replace_all.
  // @Volatile
  check_debug_memory(s);

  if (!s.Data || !s.Count)
    return;

  assert(what.Data && what.Count);
  if (replace.Count)
    assert(replace.Data);

  if (what.Count == replace.Count)
  {
    // This case we can handle relatively fast.
    // @Speed Improve by using bit hacks for the case when the elements are less
    // than a pointer size?
    auto *p = s.Data;
    auto *e = s.Data + s.Count;
    while (p != e)
    {
      // @Speed We can do simply memcmp for scalar types and types that don't
      // haveoverloaded ==.
      if (*p == what[0])
      {
        auto *n = p;
        auto *sp = what.Data;
        auto *se = what.Data + what.Count;
        while (n != e && sp != se)
        {
          // Require only operator == to be defined (and not !=).
          if (!(*n == *sp))
            break;
          ++n, ++sp;
        }

        if (sp == se)
        {
          // Match found
          memcpy(p, replace.Data, replace.Count * sizeof(*p));
          p += replace.Count;
        }
        else
        {
          ++p;
        }
      }
      else
      {
        ++p;
      }
    }
  }
  else
  {
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
    s64 i = 0;
    s64 searchLen = length(what);
    s64 replaceLen = length(replace);

    while ((i = search(s, what, .Start = i)) != -1)
    {
      replace_range(s, i, i + searchLen, replace); // Use code-point-based replace_range

      i += replaceLen;
    }
  }
}

inline void replace_all(string ref s, code_point what, code_point replace)
{
  char encodedOld[4];
  utf8_encode_cp(encodedOld, what);

  char encodedNew[4];
  utf8_encode_cp(encodedNew, replace);

  replace_all(s, string(encodedOld, utf8_get_size_of_cp(encodedOld)),
              string(encodedNew, utf8_get_size_of_cp(encodedNew)));
}

inline void remove_all(string ref s, code_point what)
{
  char encodedCp[4];
  utf8_encode_cp(encodedCp, what);

  replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), string(""));
}

inline void remove_all(string ref s, string what)
{
  replace_all(s, what, string(""));
}

inline void replace_all(string ref s, code_point what, string replace)
{
  char encodedCp[4];
  utf8_encode_cp(encodedCp, what);

  replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), replace);
}

inline void replace_all(string ref s, string what, code_point replace)
{
  char encodedCp[4];
  utf8_encode_cp(encodedCp, replace);

  replace_all(s, what, string(encodedCp, utf8_get_size_of_cp(encodedCp)));
}

LSTD_END_NAMESPACE
