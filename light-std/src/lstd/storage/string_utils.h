#pragma once

/// Provides ascii and utf8 string utility functions

#include "../common.h"

LSTD_BEGIN_NAMESPACE

// Retrieve the length of a standard cstyle string. Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in bytes of a null terminated utf8 string.
constexpr size_t cstring_strlen(const byte *str) {
    if (!str) return 0;

    size_t length = 0;
    while (*str++) ++length;
    return length;
}

// Overload for wide chars
constexpr size_t cstring_strlen(const wchar_t *str) {
    if (!str) return 0;

    size_t length = 0;
    while (*str++) ++length;
    return length;
}

// Overload for utf32 chars
constexpr size_t cstring_strlen(const char32_t *str) {
    if (!str) return 0;

    size_t length = 0;
    while (*str++) ++length;
    return length;
}

// Retrieve the length (in code points) of an encoded utf8 string
constexpr size_t utf8_strlen(const byte *str, size_t size) {
    size_t length = 0;
    while (size--) {
        if (!((*str++ & 0xc0) == 0x80)) ++length;
    }
    return length;
}

// These functions only work for ascii
constexpr bool is_digit(char32_t x) { return x >= '0' && x <= '9'; }
// These functions only work for ascii
constexpr bool is_hexadecimal_digit(char32_t x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f'); }

// These functions only work for ascii
constexpr bool is_space(char32_t x) { return (x >= 9 && x <= 13) || x == 32; }
// These functions only work for ascii
constexpr bool is_blank(char32_t x) { return x == 9 || x == 32; }

// These functions only work for ascii
constexpr bool is_alpha(char32_t x) { return (x >= 65 && x <= 90) || (x >= 97 && x <= 122); }
// These functions only work for ascii
constexpr bool is_alphanumeric(char32_t x) { return is_alpha(x) || is_digit(x); }

constexpr bool is_identifier_start(char32_t x) { return is_alpha(x) || x == '_'; }

// These functions only work for ascii
constexpr bool is_print(char32_t x) { return x > 31 && x != 127; }

// Convert code point to uppercase
constexpr char32_t to_upper(char32_t cp) {
    if (((0x0061 <= cp) && (0x007a >= cp)) || ((0x00e0 <= cp) && (0x00f6 >= cp)) ||
        ((0x00f8 <= cp) && (0x00fe >= cp)) || ((0x03b1 <= cp) && (0x03c1 >= cp)) ||
        ((0x03c3 <= cp) && (0x03cb >= cp))) {
        return cp - 32;
    }
    if (((0x0100 <= cp) && (0x012f >= cp)) || ((0x0132 <= cp) && (0x0137 >= cp)) ||
        ((0x014a <= cp) && (0x0177 >= cp)) || ((0x0182 <= cp) && (0x0185 >= cp)) ||
        ((0x01a0 <= cp) && (0x01a5 >= cp)) || ((0x01de <= cp) && (0x01ef >= cp)) ||
        ((0x01f8 <= cp) && (0x021f >= cp)) || ((0x0222 <= cp) && (0x0233 >= cp)) ||
        ((0x0246 <= cp) && (0x024f >= cp)) || ((0x03d8 <= cp) && (0x03ef >= cp))) {
        return cp & ~0x1;
    }
    if (((0x0139 <= cp) && (0x0148 >= cp)) || ((0x0179 <= cp) && (0x017e >= cp)) ||
        ((0x01af <= cp) && (0x01b0 >= cp)) || ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
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
    // No upper case!
    return cp;
}

// Convert code point to lowercase
constexpr char32_t to_lower(char32_t cp) {
    if (((0x0041 <= cp) && (0x005a >= cp)) || ((0x00c0 <= cp) && (0x00d6 >= cp)) ||
        ((0x00d8 <= cp) && (0x00de >= cp)) || ((0x0391 <= cp) && (0x03a1 >= cp)) ||
        ((0x03a3 <= cp) && (0x03ab >= cp))) {
        return cp + 32;
    }
    if (((0x0100 <= cp) && (0x012f >= cp)) || ((0x0132 <= cp) && (0x0137 >= cp)) ||
        ((0x014a <= cp) && (0x0177 >= cp)) || ((0x0182 <= cp) && (0x0185 >= cp)) ||
        ((0x01a0 <= cp) && (0x01a5 >= cp)) || ((0x01de <= cp) && (0x01ef >= cp)) ||
        ((0x01f8 <= cp) && (0x021f >= cp)) || ((0x0222 <= cp) && (0x0233 >= cp)) ||
        ((0x0246 <= cp) && (0x024f >= cp)) || ((0x03d8 <= cp) && (0x03ef >= cp))) {
        return cp | 0x1;
    }
    if (((0x0139 <= cp) && (0x0148 >= cp)) || ((0x0179 <= cp) && (0x017e >= cp)) ||
        ((0x01af <= cp) && (0x01b0 >= cp)) || ((0x01b3 <= cp) && (0x01b6 >= cp)) ||
        ((0x01cd <= cp) && (0x01dc >= cp))) {
        return (cp + 1) & ~0x1;
    }
    if (cp == 0x0178) return 0x00ff;
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

constexpr bool is_upper(char32_t ch) { return ch != to_lower(ch); }
constexpr bool is_lower(char32_t ch) { return ch != to_upper(ch); }

// Returns the size in bytes of the code point that _str_ points to.
// If the byte pointed by _str_ is a countinuation utf8 byte, this function returns 0
constexpr s8 get_size_of_cp(const byte *str) {
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

// Returns the size that the code point would be if it were encoded.
constexpr s8 get_size_of_cp(char32_t codePoint) {
    if (((s32) 0xffffff80 & codePoint) == 0) {
        return 1;
    } else if (((s32) 0xfffff800 & codePoint) == 0) {
        return 2;
    } else if (((s32) 0xffff0000 & codePoint) == 0) {
        return 3;
    } else {
        return 4;
    }
}

// Encodes code point at _str_, assumes there is enough space.
constexpr void encode_cp(byte *str, char32_t codePoint) {
    size_t size = get_size_of_cp(codePoint);
    if (size == 1) {
        // 1-byte/7-bit ascii
        // (0b0xxxxxxx)
        str[0] = (byte) codePoint;
    } else if (size == 2) {
        // 2-byte/11-bit utf8 code point
        // (0b110xxxxx 0b10xxxxxx)
        str[0] = 0xc0 | (byte)(codePoint >> 6);
        str[1] = 0x80 | (byte)(codePoint & 0x3f);
    } else if (size == 3) {
        // 3-byte/16-bit utf8 code point
        // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xe0 | (byte)(codePoint >> 12);
        str[1] = 0x80 | (byte)((codePoint >> 6) & 0x3f);
        str[2] = 0x80 | (byte)(codePoint & 0x3f);
    } else {
        // 4-byte/21-bit utf8 code point
        // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xf0 | (byte)(codePoint >> 18);
        str[1] = 0x80 | (byte)((codePoint >> 12) & 0x3f);
        str[2] = 0x80 | (byte)((codePoint >> 6) & 0x3f);
        str[3] = 0x80 | (byte)(codePoint & 0x3f);
    }
}

// Decodes a code point from a data pointer
constexpr char32_t decode_cp(const byte *str) {
    if (0xf0 == (0xf8 & str[0])) {
        // 4 byte utf8 code point
        return ((0x07 & str[0]) << 18) | ((0x3f & str[1]) << 12) | ((0x3f & str[2]) << 6) | (0x3f & str[3]);
    } else if (0xe0 == (0xf0 & str[0])) {
        // 3 byte utf8 code point
        return ((0x0f & str[0]) << 12) | ((0x3f & str[1]) << 6) | (0x3f & str[2]);
    } else if (0xc0 == (0xe0 & str[0])) {
        // 2 byte utf8 code point
        return ((0x1f & str[0]) << 6) | (0x3f & str[1]);
    } else {
        // 1 byte utf8 code point
        return str[0];
    }
}

// Returns the index of the code point where _byteIndex_ is pointing to (may be inside a code point)
constexpr size_t get_cp_index_from_byte_index(const byte *str, size_t size, size_t byteIndex) {
    assert(byteIndex < size);
    size_t result = 0;

    auto *targetByte = str + byteIndex;
    while (str < targetByte) {
        str += get_size_of_cp(str);
        ++result;
    }
    return result;
}

// This function translates an index that may be negative to an actual index.
// For example 5 maps to 5
// but -5 maps to length - 5
// This function is used to support python-like negative indexing.
//
// This function checks if the index is in range
//
// If _toleratePastLast_ is true, pointing to one past the end is accepted
constexpr size_t translate_index(s64 index, size_t length, bool toleratePastLast = false) {
    size_t checkLength = toleratePastLast ? (length + 1) : length;

    if (index < 0) {
        s64 actual = length + index;
        assert(actual >= 0);
        assert((size_t) actual < checkLength);
        return (size_t) actual;
    }
    assert((size_t) index < checkLength);
    return (size_t) index;
}

// This returns a pointer to the code point at a specified index in an utf8 string
// If _toleratePastLast_ is true, pointing to one past the end is accepted
constexpr const byte *get_cp_at_index(const byte *str, size_t length, s64 index, bool toleratePastLast = false) {
    For(range(translate_index(index, length, toleratePastLast))) str += get_size_of_cp(str);
    return str;
}

// Returns the index of the first byte of the code point which _cpIndex_ is pointing to
constexpr size_t get_byte_index_from_cp_index(const byte *str, size_t length, size_t cpIndex) {
    assert(cpIndex < length);
    return get_cp_at_index(str, length, (s64) cpIndex, true) - str;
}

#define COMMON_STRING_API_IMPL(name, cexpr_keyword)                                                              \
    const byte *Data = null;                                                                                     \
    size_t ByteLength = 0;                                                                                       \
                                                                                                                 \
    /* Length in code points */                                                                                  \
    size_t Length = 0;                                                                                           \
                                                                                                                 \
    /* Gets the _index_'th code point in the string	*/                                                           \
    cexpr_keyword char32_t get(s64 index) const { return decode_cp(get_cp_at_index(Data, Length, index)); }      \
                                                                                                                 \
    cexpr_keyword bool begins_with(char32_t cp) const { return get(0) == cp; }                                   \
    cexpr_keyword bool begins_with(name str) const {                                                             \
        assert(str.ByteLength < ByteLength);                                                                     \
        return compare_memory_constexpr(Data, str.Data, str.ByteLength) == npos;                                 \
    }                                                                                                            \
                                                                                                                 \
    cexpr_keyword bool ends_with(char32_t cp) const { return get(-1) == cp; }                                    \
    cexpr_keyword bool ends_with(name str) const {                                                               \
        assert(str.ByteLength < ByteLength);                                                                     \
        return compare_memory_constexpr(Data + ByteLength - str.ByteLength, str.Data, str.ByteLength) == npos;   \
    }                                                                                                            \
                                                                                                                 \
    /* Compares the string to _str_ and returns the index of the first differnt code point */                    \
    /* If the strings are equal, the returned value is npos (-1) */                                              \
    cexpr_keyword size_t compare(name str) {                                                                     \
        /* If the memory pointers and the lengths are the same, the strings are equal! */                        \
        if (Data == str.Data && ByteLength == str.ByteLength) return npos;                                       \
                                                                                                                 \
        auto s1 = begin();                                                                                       \
        auto s2 = str.begin();                                                                                   \
        while (*s1 == *s2) {                                                                                     \
            ++s1, ++s2;                                                                                          \
            if (s1 == end() && s2 == str.end()) return npos;                                                     \
            if (s1 == end() || s2 == str.end()) return s1.Index;                                                 \
        }                                                                                                        \
        return s1.Index;                                                                                         \
    }                                                                                                            \
                                                                                                                 \
    /* Compares the string to _str_ (ignoring case) and returns the index of the first different code point */   \
    /* If the strings are equal, the returned value is npos (-1) */                                              \
    cexpr_keyword size_t compare_ignore_case(name str) {                                                  \
        /* If the memory pointers and the lengths are the same, the strings are equal! */                        \
        if (Data == str.Data && ByteLength == str.ByteLength) return npos;                                       \
                                                                                                                 \
        auto s1 = begin();                                                                                       \
        auto s2 = str.begin();                                                                                   \
        while (::to_lower(*s1) == ::to_lower(*s2)) {                                                             \
            ++s1, ++s2;                                                                                          \
            if (s1 == end() && s2 == str.end()) return npos;                                                     \
            if (s1 == end() || s2 == str.end()) return s1.Index;                                                 \
        }                                                                                                        \
        return s1.Index;                                                                                         \
    }                                                                                                            \
                                                                                                                 \
    /* Compares the string to _str_ lexicographically. */                                                        \
    /* The result is less than 0 if this string sorts before the other, 0 if they are equal, */                  \
    /* and greater than 0 otherwise. */                                                                          \
    cexpr_keyword s32 compare_lexicographically(name str) const {                                         \
        /* If the memory pointers and the lengths are the same, the strings are equal! */                        \
        if (Data == str.Data && ByteLength == str.ByteLength) return 0;                                          \
                                                                                                                 \
        if (Length == 0 && str.Length == 0) return 0;                                                            \
        if (Length == 0) return -((s32) str.get(0));                                                             \
        if (str.Length == 0) return get(0);                                                                      \
                                                                                                                 \
        auto s1 = begin();                                                                                       \
        auto s2 = str.begin();                                                                                   \
        while (*s1 == *s2) {                                                                                     \
            ++s1, ++s2;                                                                                          \
            if (s1 == end() && s2 == str.end()) return 0;                                                        \
            if (s1 == end()) return -((s32) str.get(0));                                                         \
            if (s2 == str.end()) return get(0);                                                                  \
        }                                                                                                        \
        return ((s32) *s1 - (s32) *s2);                                                                          \
    }                                                                                                            \
                                                                                                                 \
    /* Compares the string to _str_ lexicographically while ignoring case of code points. */                     \
    /* The result is less than 0 if this string sorts before the other, 0 if they are equal, */                  \
    /* and greater than 0 otherwise. */                                                                          \
    cexpr_keyword s32 compare_lexicographically_ignore_case(name str) const {                             \
        /* If the memory pointers and the lengths are the same, the strings are equal! */                        \
        if (Data == str.Data && ByteLength == str.ByteLength) return 0;                                          \
        if (Length == 0 && str.Length == 0) return 0;                                                            \
        if (Length == 0) return -((s32)::to_lower(str.get(0)));                                                  \
        if (str.Length == 0) return ::to_lower(get(0));                                                          \
                                                                                                                 \
        auto s1 = begin();                                                                                       \
        auto s2 = str.begin();                                                                                   \
        while (::to_lower(*s1) == ::to_lower(*s2)) {                                                             \
            ++s1, ++s2;                                                                                          \
            if (s1 == end() && s2 == str.end()) return 0;                                                        \
            if (s1 == end()) return -((s32)::to_lower(str.get(0)));                                              \
            if (s2 == str.end()) return ::to_lower(get(0));                                                      \
        }                                                                                                        \
        return ((s32)::to_lower(*s1) - (s32)::to_lower(*s2));                                                    \
    }                                                                                                            \
                                                                                                                 \
    /* Find the first occurence of a code point that is after a specified index */                               \
    cexpr_keyword size_t find(char32_t cp, s64 start = 0) const {                                                \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, Length)) if (*p++ == cp) return it;                                                     \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the first occurence of a substring that is after a specified index */                                \
    cexpr_keyword size_t find(name str, s64 start = 0) const {                                            \
        assert(Data);                                                                                            \
        assert(str.Data);                                                                                        \
        assert(str.Length);                                                                                      \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
                                                                                                                 \
        For(range(start, Length)) {                                                                              \
            auto progress = str.begin();                                                                         \
            for (auto search = begin() + it; progress != str.end(); ++search, ++progress) {                      \
                if (*search != *progress) break;                                                                 \
            }                                                                                                    \
            if (progress == str.end()) return it;                                                                \
        }                                                                                                        \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the last occurence of a code point that is before a specified index */                               \
    cexpr_keyword size_t find_reverse(char32_t cp, s64 start = 0) const {                                        \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
        if (start == 0) start = Length - 1;                                                                      \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, -1, -1)) if (*p-- == cp) return it;                                                     \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the last occurence of a substring that is before a specified index */                                \
    cexpr_keyword size_t find_reverse(name str, s64 start = 0) const {                                    \
        assert(Data);                                                                                            \
        assert(str.Data);                                                                                        \
        assert(str.Length);                                                                                      \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
        if (start == 0) start = Length - 1;                                                                      \
                                                                                                                 \
        For(range(start - str.Length + 1, -1, -1)) {                                                             \
            auto progress = str.begin();                                                                         \
            for (auto search = begin() + it; progress != str.end(); ++search, ++progress) {                      \
                if (*search != *progress) break;                                                                 \
            }                                                                                                    \
            if (progress == str.end()) return it;                                                                \
        }                                                                                                        \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the first occurence of any code point in the specified view that is after a specified index */       \
    cexpr_keyword size_t find_any_of(name cps, s64 start = 0) const {                                     \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, Length)) if (cps.has(*p++)) return it;                                                  \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the last occurence of any code point in the specified view */                                        \
    /* that is before a specified index (0 means: start from the end) */                                         \
    cexpr_keyword size_t find_reverse_any_of(name cps, s64 start = 0) const {                             \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
        if (start == 0) start = Length - 1;                                                                      \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, -1, -1)) if (cps.has(*p--)) return it;                                                  \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the first absence of a code point that is after a specified index */                                 \
    cexpr_keyword size_t find_not(char32_t cp, s64 start = 0) const {                                            \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, Length)) if (*p++ != cp) return it;                                                     \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the last absence of a code point that is before the specified index */                               \
    cexpr_keyword size_t find_reverse_not(char32_t cp, s64 start = 0) const {                                    \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
        if (start == 0) start = Length - 1;                                                                      \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, 0, -1)) if (*p-- != cp) return it;                                                      \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the first absence of any code point in the specified view that is after a specified index */         \
    cexpr_keyword size_t find_not_any_of(name cps, s64 start = 0) const {                                 \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, Length)) if (!cps.has(*p++)) return it;                                                 \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Find the first absence of any code point in the specified view that is after a specified index */         \
    cexpr_keyword size_t find_reverse_not_any_of(name cps, s64 start = 0) const {                         \
        assert(Data);                                                                                            \
        if (Length == 0) return npos;                                                                            \
                                                                                                                 \
        start = translate_index(start, Length);                                                                  \
        if (start == 0) start = Length - 1;                                                                      \
                                                                                                                 \
        auto p = begin() + start;                                                                                \
        For(range(start, 0, -1)) if (!cps.has(*p--)) return it;                                                  \
        return npos;                                                                                             \
    }                                                                                                            \
                                                                                                                 \
    /* Gets [begin, end) range of characters into a new string object. */                                        \
    cexpr_keyword name substring(s64 begin, s64 end) const {                                                     \
        /* Convert to absolute [begin, end) */                                                                   \
        size_t beginIndex = translate_index(begin, Length);                                                      \
        size_t endIndex = translate_index(end, Length, true);                                                    \
        const byte *beginPtr = get_cp_at_index(Data, Length, beginIndex);                                        \
        const byte *endPtr = beginPtr;                                                                           \
        For(range(beginIndex, endIndex)) endPtr += get_size_of_cp(endPtr);                                       \
        name result = {};                                                                                        \
        result.Data = beginPtr;                                                                                  \
        result.ByteLength = (uptr_t)(endPtr - beginPtr);                                                         \
        result.Length = endIndex - beginIndex;                                                                   \
        return result;                                                                                           \
    }                                                                                                            \
                                                                                                                 \
    /* Returns a substring with whitespace removed at the start */                                               \
    cexpr_keyword name trim_start() const { return substring(find_not_any_of(" \n\r\t\v\f"), Length); }          \
                                                                                                                 \
    /* Returns a substring with whitespace removed at the end */                                                 \
    cexpr_keyword name trim_end() const { return substring(0, find_reverse_not_any_of(" \n\r\t\v\f") + 1); }     \
                                                                                                                 \
    /* Returns a substring with whitespace removed from both sides */                                            \
    cexpr_keyword name trim() const { return trim_start().trim_end(); }                                          \
                                                                                                                 \
    /* Returns true if the string contains _cp_ anywhere */                                                      \
    cexpr_keyword bool has(char32_t cp) const { return find(cp) != npos; }                                       \
                                                                                                                 \
    /* Returns true if the string contains _str_ anywhere */                                                     \
    cexpr_keyword bool has(name str) const { return find(str) != npos; }                                  \
                                                                                                                 \
    /* Counts the number of occurences of _cp_ */                                                                \
    cexpr_keyword size_t count(char32_t cp) const {                                                              \
        size_t result = 0, index = 0;                                                                            \
        while ((index = find(cp, index)) != npos) {                                                              \
            ++result, ++index;                                                                                   \
            if (index >= Length) break;                                                                          \
        }                                                                                                        \
        return result;                                                                                           \
    }                                                                                                            \
                                                                                                                 \
    /* Counts the number of occurences of _str_ */                                                               \
    cexpr_keyword size_t count(name str) const {                                                          \
        size_t result = 0, index = 0;                                                                            \
        while ((index = find(str, index)) != npos) {                                                             \
            ++result, ++index;                                                                                   \
            if (index >= Length) break;                                                                          \
        }                                                                                                        \
        return result;                                                                                           \
    }                                                                                                            \
                                                                                                                 \
    /* Converts a utf8 string to a null-terminated wide char string (commonly used in Windows platform calls) */ \
    /* Assumes _out_ has enough space to hold the converted bytes */                                             \
    cexpr_keyword void to_utf16(wchar_t *out) {                                                                  \
        auto *str = Data;                                                                                        \
        auto *p = out;                                                                                           \
        For(range(Length)) {                                                                                     \
            char32_t cp = decode_cp(str);                                                                        \
            if (cp > 0xffff) {                                                                                   \
                *p++ = (u16)((cp >> 10) + (0xd800u - (0x10000 >> 10)));                                          \
                *p++ = (u16)((cp & 0x3ff) + 0xdc00u);                                                            \
            } else {                                                                                             \
                *p++ = (u16) cp;                                                                                 \
            }                                                                                                    \
            str += get_size_of_cp(cp);                                                                           \
        }                                                                                                        \
        *p = 0;                                                                                                  \
    }                                                                                                            \
                                                                                                                 \
    /* Converts a utf8 string to a null-terminated utf32 string */                                               \
    /* Assumes _out_ has enough space to hold the converted bytes */                                             \
    cexpr_keyword void to_utf32(char32_t *out) {                                                                 \
        auto *str = Data;                                                                                        \
        auto *p = out;                                                                                           \
        For(range(Length)) {                                                                                     \
            char32_t cp = decode_cp(str);                                                                        \
            *p++ = cp;                                                                                           \
            str += get_size_of_cp(cp);                                                                           \
        }                                                                                                        \
        *p = 0;                                                                                                  \
    }                                                                                                            \
                                                                                                                 \
    /*			 */                                                                                                     \
    /* Iterator: */                                                                                              \
    /*			 */                                                                                                     \
    struct const_iterator {                                                                                      \
        const name *Parent;                                                                                      \
        size_t Index;                                                                                            \
                                                                                                                 \
        const_iterator() = default;                                                                              \
        constexpr const_iterator(const name *parent, size_t index) : Parent(parent), Index(index) {}             \
                                                                                                                 \
        constexpr const_iterator &operator+=(s64 amount) { return Index += amount, *this; }                      \
        constexpr const_iterator &operator-=(s64 amount) { return Index -= amount, *this; }                      \
        constexpr const_iterator &operator++() { return *this += 1; }                                            \
        constexpr const_iterator &operator--() { return *this -= 1; }                                            \
        constexpr const_iterator operator++(s32) {                                                               \
            const_iterator temp = *this;                                                                         \
            return ++(*this), temp;                                                                              \
        }                                                                                                        \
                                                                                                                 \
        constexpr const_iterator operator--(s32) {                                                               \
            const_iterator temp = *this;                                                                         \
            return --(*this), temp;                                                                              \
        }                                                                                                        \
                                                                                                                 \
        constexpr s64 operator-(const const_iterator &other) const {                                             \
            size_t lesser = Index, greater = other.Index;                                                        \
            if (lesser > greater) {                                                                              \
                lesser = other.Index;                                                                            \
                greater = Index;                                                                                 \
            }                                                                                                    \
            s64 difference = greater - lesser;                                                                   \
            return Index <= other.Index ? difference : -difference;                                              \
        }                                                                                                        \
                                                                                                                 \
        constexpr const_iterator operator+(s64 amount) const { return const_iterator(Parent, Index + amount); }  \
        constexpr const_iterator operator-(s64 amount) const { return const_iterator(Parent, Index - amount); }  \
                                                                                                                 \
        friend constexpr const_iterator operator+(s64 amount, const const_iterator &it) { return it + amount; }  \
        friend constexpr const_iterator operator-(s64 amount, const const_iterator &it) { return it - amount; }  \
                                                                                                                 \
        constexpr bool operator==(const const_iterator &other) const { return Index == other.Index; }            \
        constexpr bool operator!=(const const_iterator &other) const { return Index != other.Index; }            \
        constexpr bool operator>(const const_iterator &other) const { return Index > other.Index; }              \
        constexpr bool operator<(const const_iterator &other) const { return Index < other.Index; }              \
        constexpr bool operator>=(const const_iterator &other) const { return Index >= other.Index; }            \
        constexpr bool operator<=(const const_iterator &other) const { return Index <= other.Index; }            \
                                                                                                                 \
        cexpr_keyword char32_t operator*() const { return Parent->get(Index); }                                  \
                                                                                                                 \
        constexpr const byte *to_pointer() const {                                                               \
            return get_cp_at_index(Parent->Data, Parent->Length, (s64) Index, true);                             \
        }                                                                                                        \
    };                                                                                                           \
                                                                                                                 \
    cexpr_keyword const_iterator begin() const { return const_iterator(this, 0); }                               \
    cexpr_keyword const_iterator end() const { return const_iterator(this, Length); }                            \
                                                                                                                 \
    /*			  */                                                                                                    \
    /* Operators: */                                                                                             \
    /*			  */                                                                                                    \
    cexpr_keyword char32_t operator[](s64 index) const { return get(index); }                                    \
                                                                                                                 \
    /* Check two strings for equality */                                                                         \
    cexpr_keyword bool operator==(name other) const { return compare_lexicographically(other) == 0; }     \
    cexpr_keyword bool operator!=(name other) const { return !(*this == other); }                         \
    cexpr_keyword bool operator<(name other) const { return compare_lexicographically(other) < 0; }       \
    cexpr_keyword bool operator>(name other) const { return compare_lexicographically(other) > 0; }       \
    cexpr_keyword bool operator<=(name other) const { return !(*this > other); }                          \
    cexpr_keyword bool operator>=(name other) const { return !(*this < other); }

// A string object that is entirely constexpr
struct string_view {
    // This is macroed in order to allow "string" to implement it too and be consistent with string_view
    COMMON_STRING_API_IMPL(string_view, constexpr)

    string_view() = default;

    constexpr string_view(const byte *str) : Data(str), ByteLength(cstring_strlen(str)), Length(0) {
        Length = utf8_strlen(str, ByteLength);
    }

    constexpr string_view(const byte *str, size_t size) : Data(str), ByteLength(size), Length(utf8_strlen(str, size)) {}
};

constexpr bool operator==(const byte *one, const string_view &other) {
    return other.compare_lexicographically(string_view(one)) == 0;
}
constexpr bool operator!=(const byte *one, const string_view &other) { return !(one == other); }
constexpr bool operator<(const byte *one, const string_view &other) {
    return other.compare_lexicographically(string_view(one)) > 0;
}
constexpr bool operator>(const byte *one, const string_view &other) {
    return other.compare_lexicographically(string_view(one)) < 0;
}
constexpr bool operator<=(const byte *one, const string_view &other) { return !(one > other); }
constexpr bool operator>=(const byte *one, const string_view &other) { return !(one < other); }

LSTD_END_NAMESPACE
