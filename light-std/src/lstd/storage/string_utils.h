#pragma once

/// Provides ascii and utf8 string utility functions

#include "../common.h"
#include "../intrin.h"

LSTD_BEGIN_NAMESPACE

// Retrieve the length of a standard cstyle string. Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in bytes of a null terminated utf8 string.
constexpr size_t c_string_strlen(const char *str) {
    if (!str) return 0;

    size_t length = 0;
    while (*str++) ++length;
    return length;
}

// Overload for wide chars
constexpr size_t c_string_strlen(const wchar_t *str) {
    if (!str) return 0;

    size_t length = 0;
    while (*str++) ++length;
    return length;
}

// Overload for utf32 chars
constexpr size_t c_string_strlen(const char32_t *str) {
    if (!str) return 0;

    size_t length = 0;
    while (*str++) ++length;
    return length;
}

constexpr size_t compare_c_string(const char *one, const char *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return npos;

    size_t index = 0;
    while (*one == *other) {
        ++one, ++other;
        if (!*one && !*other) return npos;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr size_t compare_c_string(const wchar_t *one, const wchar_t *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return npos;

    size_t index = 0;
    while (*one == *other) {
        ++one, ++other;
        if (!*one && !*other) return npos;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr size_t compare_c_string(const char32_t *one, const char32_t *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return npos;

    size_t index = 0;
    while (*one == *other) {
        ++one, ++other;
        if (!*one && !*other) return npos;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr s32 compare_c_string_lexicographically(const char *one, const char *other) {
    assert(one);
    assert(other);

    while (*one && (*one == *other)) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

constexpr s32 compare_c_string_lexicographically(const wchar_t *one, const wchar_t *other) {
    assert(one);
    assert(other);

    while (*one && (*one == *other)) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

constexpr s32 compare_c_string_lexicographically(const char32_t *one, const char32_t *other) {
    assert(one);
    assert(other);

    while (*one && (*one == *other)) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

// Retrieve the length (in code points) of an encoded utf8 string
constexpr size_t utf8_strlen(const char *str, size_t size) {
    if (!str || size == 0) return 0;

    size_t length = 0;
    while (size--) {
        if (!((*str++ & 0xc0) == 0x80)) ++length;
    }
    return length;
}

// These functions only work for ascii
constexpr bool is_digit(char32_t x) { return x >= '0' && x <= '9'; }
// These functions only work for ascii
constexpr bool is_hex_digit(char32_t x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f'); }

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

constexpr size_t compare_c_string_ignore_case(const char *one, const char *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return npos;

    size_t index = 0;
    while (to_lower(*one) == to_lower(*other)) {
        ++one, ++other;
        if (!*one && !*other) return npos;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr size_t compare_c_string_ignore_case(const wchar_t *one, const wchar_t *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return npos;

    size_t index = 0;
    while (to_lower((char32_t) *one) == to_lower((char32_t) *other)) {
        ++one, ++other;
        if (!*one && !*other) return npos;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr size_t compare_c_string_ignore_case(const char32_t *one, const char32_t *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return npos;

    size_t index = 0;
    while (to_lower(*one) == to_lower(*other)) {
        ++one, ++other;
        if (!*one && !*other) return npos;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr s32 compare_c_string_lexicographically_ignore_case(const char *one, const char *other) {
    assert(one);
    assert(other);

    while (*one && (to_lower(*one) == to_lower(*other))) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

constexpr s32 compare_c_string_lexicographically_ignore_case(const wchar_t *one, const wchar_t *other) {
    assert(one);
    assert(other);

    while (*one && (to_lower((char32_t) *one) == to_lower((char32_t) *other))) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

constexpr s32 compare_c_string_lexicographically_ignore_case(const char32_t *one, const char32_t *other) {
    assert(one);
    assert(other);

    while (*one && (to_lower(*one) == to_lower(*other))) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

// Returns the size in bytes of the code point that _str_ points to.
// If the byte pointed by _str_ is a countinuation utf8 byte, this function returns 0
constexpr s8 get_size_of_cp(const char *str) {
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
constexpr void encode_cp(char *str, char32_t codePoint) {
    size_t size = get_size_of_cp(codePoint);
    if (size == 1) {
        // 1-byte/7-bit ascii
        // (0b0xxxxxxx)
        str[0] = (char) codePoint;
    } else if (size == 2) {
        // 2-byte/11-bit utf8 code point
        // (0b110xxxxx 0b10xxxxxx)
        str[0] = 0xc0 | (char)(codePoint >> 6);
        str[1] = 0x80 | (char)(codePoint & 0x3f);
    } else if (size == 3) {
        // 3-byte/16-bit utf8 code point
        // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xe0 | (char)(codePoint >> 12);
        str[1] = 0x80 | (char)((codePoint >> 6) & 0x3f);
        str[2] = 0x80 | (char)(codePoint & 0x3f);
    } else {
        // 4-byte/21-bit utf8 code point
        // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xf0 | (char)(codePoint >> 18);
        str[1] = 0x80 | (char)((codePoint >> 12) & 0x3f);
        str[2] = 0x80 | (char)((codePoint >> 6) & 0x3f);
        str[3] = 0x80 | (char)(codePoint & 0x3f);
    }
}

// Decodes a code point from a data pointer
constexpr char32_t decode_cp(const char *str) {
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
constexpr const char *get_cp_at_index(const char *str, size_t length, s64 index, bool toleratePastLast = false) {
    For(range(translate_index(index, length, toleratePastLast))) str += get_size_of_cp(str);
    return str;
}

// Compares two utf8 encoded strings and returns the index of the code point
// at which they are different or _npos_ if they are the same.
constexpr size_t compare_utf8(const char *one, size_t length1, const char *two, size_t length2) {
    if (length1 == 0 && length2 == 0) return npos;
    if (length1 == 0 || length2 == 0) return 0;

    auto *e1 = get_cp_at_index(one, length1, length1, true);
    auto *e2 = get_cp_at_index(two, length2, length2, true);
    size_t index = 0;
    while (decode_cp(one) == decode_cp(two)) {
        one += get_size_of_cp(one);
        two += get_size_of_cp(two);
        if (one == e1 && two == e2) return npos;
        if (one == e1 || two == e2) return index;
        ++index;
    }
    return index;
}

// Compares two utf8 encoded strings ignoring case and returns the index of the code point
// at which they are different or _npos_ if they are the same.
constexpr size_t compare_utf8_ignore_case(const char *one, size_t length1, const char *two, size_t length2) {
    if (length1 == 0 && length2 == 0) return npos;
    if (length1 == 0 || length2 == 0) return 0;

    auto *e1 = get_cp_at_index(one, length1, length1, true);
    auto *e2 = get_cp_at_index(two, length2, length2, true);
    size_t index = 0;
    while (to_lower(decode_cp(one)) == to_lower(decode_cp(two))) {
        one += get_size_of_cp(one);
        two += get_size_of_cp(two);
        if (one == e1 && two == e2) return npos;
        if (one == e1 || two == e2) return index;
        ++index;
    }
    return index;
}

// Compares two utf8 encoded strings and returns -1 if _one_ is before _two_,
// 0 if one == two and 1 if _two_ is before _one_.
constexpr s32 compare_utf8_lexicographically(const char *one, size_t length1, const char *two, size_t length2) {
    if (length1 == 0 && length2 == 0) return 0;
    if (length1 == 0) return -1;
    if (length2 == 0) return 1;

    auto *e1 = get_cp_at_index(one, length1, length1, true);
    auto *e2 = get_cp_at_index(two, length2, length2, true);
    size_t index = 0;
    while (decode_cp(one) == decode_cp(two)) {
        one += get_size_of_cp(one);
        two += get_size_of_cp(two);
        if (one == e1 && two == e2) return 0;
        if (one == e1) return -1;
        if (two == e2) return 1;
        ++index;
    }
    return ((s64) decode_cp(one) - (s64) decode_cp(two)) < 0 ? -1 : 1;
}

// Compares two utf8 encoded strings ignorign case and returns -1 if _one_ is before _two_,
// 0 if one == two and 1 if _two_ is before _one_.
constexpr s32 compare_utf8_lexicographically_ignore_case(const char *one, size_t length1, const char *two,
                                                         size_t length2) {
    if (length1 == 0 && length2 == 0) return 0;
    if (length1 == 0) return -1;
    if (length2 == 0) return 1;

    auto *e1 = get_cp_at_index(one, length1, length1, true);
    auto *e2 = get_cp_at_index(two, length2, length2, true);
    size_t index = 0;
    while (to_lower(decode_cp(one)) == to_lower(decode_cp(two))) {
        one += get_size_of_cp(one);
        two += get_size_of_cp(two);
        if (one == e1 && two == e2) return 0;
        if (one == e1) return -1;
        if (two == e2) return 1;
        ++index;
    }
    return ((s64) to_lower(decode_cp(one)) - (s64) to_lower(decode_cp(two))) < 0 ? -1 : 1;
}

// Find the first occurence of a substring that is after a specified index
constexpr const char *find_substring_utf8(const char *haystack, size_t length1, const char *needle, size_t length2,
                                          s64 start = 0) {
    assert(haystack);
    assert(needle);
    assert(length2);

    if (length1 == 0) return null;

    auto *p = get_cp_at_index(haystack, length1, translate_index(start, length1));
    auto *end = get_cp_at_index(haystack, length1, length1, true);

    auto *needleEnd = get_cp_at_index(needle, length2, length2, true);

    while (p != end) {
        while (end - p > 4) {
            if (U32_HAS_VALUE(*(u32 *) p, *needle)) break;
            p += 4;
        }

        while (p != end) {
            if (*p == *needle) break;
            ++p;
        }

        if (p == end) return null;

        auto *search = p + 1;
        auto *progress = needle + 1;
        while (search != end && progress != needleEnd && *search == *progress) ++search, ++progress;
        if (progress == needleEnd) return p;
        ++p;
    }
    return null;
}

// Find the first occurence of a code point that is after a specified index
constexpr const char *find_cp_utf8(const char *str, size_t length, char32_t cp, s64 start = 0) {
    char encoded[4]{};
    encode_cp(encoded, cp);
    return find_substring_utf8(str, length, encoded, 1, start);
}

// Find the last occurence of a substring that is before a specified index
constexpr const char *find_substring_utf8_reverse(const char *haystack, size_t length1, const char *needle,
                                                  size_t length2, s64 start = 0) {
    assert(haystack);
    assert(needle);
    assert(length2);

    if (length1 == 0) return null;

    if (start == 0) start = length1;
    auto *p = get_cp_at_index(haystack, length1, translate_index(start, length1, true) - 1);
    auto *end = get_cp_at_index(haystack, length1, length1, true);

    auto *needleEnd = get_cp_at_index(needle, length2, length2, true);

    while (p > haystack) {
        while (p - haystack > 4) {
            if (U32_HAS_VALUE(*((u32 *) (p - 3)), *needle)) break;
            p -= 4;
        }

        while (p != haystack) {
            if (*p == *needle) break;
            --p;
        }

        if (*p != *needle && p == haystack) return null;

        auto *search = p + 1;
        auto *progress = needle + 1;
        while (search != end && progress != needleEnd && *search == *progress) ++search, ++progress;
        if (progress == needleEnd) return p;
        --p;
    }
    return null;
}

// Find the last occurence of a code point that is before a specified index
constexpr const char *find_cp_utf8_reverse(const char *str, size_t length, char32_t cp, s64 start = 0) {
    char encoded[4]{};
    encode_cp(encoded, cp);
    return find_substring_utf8_reverse(str, length, encoded, 1, start);
}

// Find the first occurence of any code point in _terminators_ that is after a specified index
constexpr const char *find_utf8_any_of(const char *str, size_t length1, const char *terminators, size_t length2,
                                       s64 start = 0) {
    assert(str);
    assert(terminators);
    assert(length2);

    if (length1 == 0) return null;

    start = translate_index(start, length1);
    str = get_cp_at_index(str, length1, start);

    For(range(start, length1)) {
        if (find_cp_utf8(terminators, length2, decode_cp(str)) != null) return str;
        str += get_size_of_cp(str);
    }
    return null;
}

// Find the last occurence of any code point in _terminators_
constexpr const char *find_utf8_reverse_any_of(const char *str, size_t length1, const char *terminators, size_t length2,
                                               s64 start = 0) {
    assert(str);
    assert(terminators);
    assert(length2);

    if (length1 == 0) return null;

    if (start == 0) start = length1;
    start = translate_index(start, length1, true) - 1;
    str = get_cp_at_index(str, length1, start);

    For(range(start, -1, -1)) {
        if (find_cp_utf8(terminators, length2, decode_cp(str)) != null) return str;
        str -= get_size_of_cp(str);
    }
    return null;
}

// Find the first absence of a code point that is after a specified index
constexpr const char *find_utf8_not(const char *str, size_t length, char32_t cp, s64 start = 0) {
    assert(str);

    if (length == 0) return null;

    auto *p = get_cp_at_index(str, length, translate_index(start, length));
    auto *end = get_cp_at_index(str, length, length, true);

    char encoded[4]{};
    encode_cp(encoded, cp);
    auto *encodedEnd = encoded + get_size_of_cp(encoded);

    while (p != end) {
        while (p != end) {
            if (*p != *encoded) break;
            ++p;
        }

        if (p == end) return null;

        auto *search = p + 1;
        auto *progress = encoded + 1;
        while (search != end && progress != encodedEnd && *search != *progress) ++search, ++progress;
        if (progress == encodedEnd) return p;
        ++p;
    }
    return null;
}

// Find the last absence of a code point that is before the specified index
constexpr const char *find_utf8_reverse_not(const char *str, size_t length, char32_t cp, s64 start = 0) {
    assert(str);

    if (length == 0) return null;

    if (start == 0) start = length;
    auto *p = get_cp_at_index(str, length, translate_index(start, length, true) - 1);
    auto *end = get_cp_at_index(str, length, length, true);

    char encoded[4]{};
    encode_cp(encoded, cp);
    auto *encodedEnd = encoded + get_size_of_cp(encoded);

    while (p > str) {
        while (p != str) {
            if (*p != *encoded) break;
            --p;
        }

        if (*p == *encoded && p == str) return null;

        auto *search = p + 1;
        auto *progress = encoded + 1;
        while (search != end && progress != encodedEnd && *search != *progress) ++search, ++progress;
        if (progress == encodedEnd) return p;
        --p;
    }
    return null;
}

// Find the first absence of any code point in _terminators_ that is after a specified index
constexpr const char *find_utf8_not_any_of(const char *str, size_t length1, const char *terminators, size_t length2,
                                           s64 start = 0) {
    assert(str);
    assert(terminators);
    assert(length2);

    if (length1 == 0) return null;

    start = translate_index(start, length1);
    str = get_cp_at_index(str, length1, start);

    For(range(start, length1)) {
        if (find_cp_utf8(terminators, length2, decode_cp(str)) == null) return str;
        str += get_size_of_cp(str);
    }
    return null;
}

// Find the first absence of any code point in _terminators_ that is after a specified index
constexpr const char *find_utf8_reverse_not_any_of(const char *str, size_t length1, const char *terminators,
                                                   size_t length2, s64 start = 0) {
    assert(str);
    assert(terminators);
    assert(length2);

    if (length1 == 0) return null;
    if (start == 0) start = length1;

    start = translate_index(start, length1, true) - 1;
    str = get_cp_at_index(str, length1, start);

    For(range(start, -1, -1)) {
        if (find_cp_utf8(terminators, length2, decode_cp(str)) == null) return str;
        str -= get_size_of_cp(str);
    }
    return null;
}

// Gets [begin, end) range of characters of a utf8 string.
// Returns begin-end pointers.
constexpr pair<const char *, const char *> substring_utf8(const char *str, size_t length, s64 begin, s64 end) {
    // Convert to absolute [begin, end)
    size_t beginIndex = translate_index(begin, length);
    size_t endIndex = translate_index(end, length, true);

    const char *beginPtr = get_cp_at_index(str, length, beginIndex);
    const char *endPtr = beginPtr;
    For(range(beginIndex, endIndex)) endPtr += get_size_of_cp(endPtr);

    return {beginPtr, endPtr};
}

// Converts utf8 to utf16 and stores in _out_ (assumes there is enough space).
// Also adds a null-terminator at the end.
constexpr void utf8_to_utf16(const char *str, size_t length, wchar_t *out) {
    For(range(length)) {
        char32_t cp = decode_cp(str);
        if (cp > 0xffff) {
            *out++ = (u16)((cp >> 10) + (0xd800u - (0x10000 >> 10)));
            *out++ = (u16)((cp & 0x3ff) + 0xdc00u);
        } else {
            *out++ = (u16) cp;
        }
        str += get_size_of_cp(cp);
    }
    *out = 0;
}

// Converts utf8 to utf32 and stores in _out_ (assumes there is enough space).
// Also adds a null-terminator at the end.
constexpr void utf8_to_utf32(const char *str, size_t length, char32_t *out) {
    For(range(length)) {
        char32_t cp = decode_cp(str);
        *out++ = cp;
        str += get_size_of_cp(cp);
    }
    *out = 0;
}

// Converts a null-terminated utf16 to utf8 and stores in _out_ and _outByteLength_ (assumes there is enough space).
constexpr void utf16_to_utf8(const wchar_t *str, char *out, size_t *outByteLength) {
    size_t byteLength = 0;
    while (*str) {
        encode_cp(out, *str);
        size_t cpSize = get_size_of_cp(out);
        out += cpSize;
        byteLength += cpSize;
        ++str;
    }
    *outByteLength = byteLength;
}

// Converts a null-terminated utf32 to utf8 and stores in _out_ and _outByteLength_ (assumes there is enough space).
constexpr void utf32_to_utf8(const char32_t *str, char *out, size_t *outByteLength) {
    size_t byteLength = 0;
    while (*str) {
        encode_cp(out, *str);
        size_t cpSize = get_size_of_cp(out);
        out += cpSize;
        byteLength += cpSize;
        ++str;
    }
    *outByteLength = byteLength;
}

// A string object that is entirely constexpr
struct string_view {
    const char *Data = null;

    // Length in bytes
    size_t ByteLength = 0;

    // Length in code points
    size_t Length = 0;

    constexpr string_view() = default;
    constexpr string_view(const char *str) : Data(str), ByteLength(c_string_strlen(str)), Length(0) {
        Length = utf8_strlen(str, ByteLength);
    }
    constexpr string_view(const char *str, size_t size) : Data(str), ByteLength(size), Length(utf8_strlen(str, size)) {}

    constexpr char32_t get(s64 index) const { return decode_cp(get_cp_at_index(Data, Length, index)); }

    constexpr bool begins_with(char32_t cp) const { return get(0) == cp; }
    constexpr bool begins_with(string_view str) const {
        assert(str.ByteLength < ByteLength);
        return compare_memory_constexpr(Data, str.Data, str.ByteLength) == npos;
    }

    constexpr bool ends_with(char32_t cp) const { return get(-1) == cp; }
    constexpr bool ends_with(string_view str) const {
        assert(str.ByteLength < ByteLength);
        return compare_memory_constexpr(Data + ByteLength - str.ByteLength, str.Data, str.ByteLength) == npos;
    }

    // Compares two utf8 encoded strings and returns the index of the code point
    // at which they are different or _npos_ if they are the same.
    constexpr size_t compare(string_view str) const { return compare_utf8(Data, Length, str.Data, str.Length); }

    // Compares two utf8 encoded strings ignoring case and returns the index of the code point
    // at which they are different or _npos_ if they are the same.
    constexpr size_t compare_ignore_case(string_view str) const {
        return compare_utf8_ignore_case(Data, Length, str.Data, str.Length);
    }

    // Compares two utf8 encoded strings and returns -1 if _one_ is before _two_,
    // 0 if one == two and 1 if _two_ is before _one_.
    constexpr s32 compare_lexicographically(string_view str) const {
        return compare_utf8_lexicographically(Data, Length, str.Data, str.Length);
    }

    // Compares two utf8 encoded strings ignorign case and returns -1 if _one_ is before _two_,
    // 0 if one == two and 1 if _two_ is before _one_.
    constexpr s32 compare_lexicographically_ignore_case(string_view str) const {
        return compare_utf8_lexicographically_ignore_case(Data, Length, str.Data, str.Length);
    }

    // Find the index of the first occurence of a code point that is after a specified index
    constexpr size_t find(char32_t cp, s64 start = 0) const {
        auto *p = find_cp_utf8(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first occurence of a substring that is after a specified index
    constexpr size_t find(string_view str, s64 start = 0) const {
        auto *p = find_substring_utf8(Data, Length, str.Data, str.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last occurence of a code point that is before a specified index
    constexpr size_t find_reverse(char32_t cp, s64 start = 0) const {
        auto *p = find_cp_utf8_reverse(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last occurence of a substring that is before a specified index
    constexpr size_t find_reverse(string_view str, s64 start = 0) const {
        auto *p = find_substring_utf8_reverse(Data, Length, str.Data, str.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first occurence of any code point in _terminators_ that is after a specified index
    constexpr size_t find_any_of(string_view terminators, s64 start = 0) const {
        auto *p = find_utf8_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last occurence of any code point in _terminators_
    constexpr size_t find_reverse_any_of(string_view terminators, s64 start = 0) const {
        auto *p = find_utf8_reverse_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first absence of a code point that is after a specified index
    constexpr size_t find_not(char32_t cp, s64 start = 0) const {
        auto *p = find_utf8_not(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last absence of a code point that is before the specified index
    constexpr size_t find_reverse_not(char32_t cp, s64 start = 0) const {
        auto *p = find_utf8_reverse_not(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first absence of any code point in _terminators_ that is after a specified index
    constexpr size_t find_not_any_of(string_view terminators, s64 start = 0) const {
        auto *p = find_utf8_not_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first absence of any code point in _terminators_ that is after a specified index
    constexpr size_t find_reverse_not_any_of(string_view terminators, s64 start = 0) const {
        auto *p = find_utf8_reverse_not_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Gets [begin, end) range of characters into a new string_view object
    constexpr string_view substring(s64 begin, s64 end) const {
        auto sub = substring_utf8(Data, Length, begin, end);
        return string_view(sub.First, sub.Second - sub.First);
    }

    // Returns a substring with whitespace removed at the start
    constexpr string_view trim_start() const { return substring(find_not_any_of(" \n\r\t\v\f"), Length); }

    // Returns a substring with whitespace removed at the end
    constexpr string_view trim_end() const { return substring(0, find_reverse_not_any_of(" \n\r\t\v\f") + 1); }

    // Returns a substring with whitespace removed from both sides
    constexpr string_view trim() const { return trim_start().trim_end(); }

    // Returns true if the string contains _cp_ anywhere
    constexpr bool has(char32_t cp) const { return find(cp) != npos; }

    // Returns true if the string contains _str_ anywhere
    constexpr bool has(string_view str) const { return find(str) != npos; }

    // Counts the number of occurences of _cp_
    constexpr size_t count(char32_t cp) const {
        size_t result = 0, index = 0;
        while ((index = find(cp, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    // Counts the number of occurences of _str_
    constexpr size_t count(string_view str) const {
        size_t result = 0, index = 0;
        while ((index = find(str, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    //
    // Iterator:
    //
    struct const_iterator {
        const string_view *Parent;
        size_t Index;

        const_iterator() = default;
        constexpr const_iterator(const string_view *parent, size_t index) : Parent(parent), Index(index) {}

        constexpr const_iterator &operator+=(s64 amount) { return Index += amount, *this; }
        constexpr const_iterator &operator-=(s64 amount) { return Index -= amount, *this; }
        constexpr const_iterator &operator++() { return *this += 1; }
        constexpr const_iterator &operator--() { return *this -= 1; }
        constexpr const_iterator operator++(s32) {
            const_iterator temp = *this;
            return ++(*this), temp;
        }

        constexpr const_iterator operator--(s32) {
            const_iterator temp = *this;
            return --(*this), temp;
        }

        constexpr s64 operator-(const_iterator other) const {
            size_t lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        constexpr const_iterator operator+(s64 amount) const { return const_iterator(Parent, Index + amount); }
        constexpr const_iterator operator-(s64 amount) const { return const_iterator(Parent, Index - amount); }

        friend constexpr const_iterator operator+(s64 amount, const_iterator it) { return it + amount; }
        friend constexpr const_iterator operator-(s64 amount, const_iterator it) { return it - amount; }

        constexpr bool operator!=(const_iterator other) const { return Index != other.Index; }
        constexpr bool operator==(const_iterator other) const { return Index == other.Index; }
        constexpr bool operator>(const_iterator other) const { return Index > other.Index; }
        constexpr bool operator<(const_iterator other) const { return Index < other.Index; }
        constexpr bool operator>=(const_iterator other) const { return Index >= other.Index; }
        constexpr bool operator<=(const_iterator other) const { return Index <= other.Index; }

        constexpr char32_t operator*() const { return Parent->get(Index); }

        constexpr operator const char *() const {
            return get_cp_at_index(Parent->Data, Parent->Length, (s64) Index, true);
        }
    };

    constexpr const_iterator begin() const { return const_iterator(this, 0); }
    constexpr const_iterator end() const { return const_iterator(this, Length); }

    //
    // Operators:
    //
    constexpr char32_t operator[](s64 index) const { return get(index); }

    constexpr bool operator==(string_view other) const { return compare(other) == npos; }
    constexpr bool operator!=(string_view other) const { return !(*this == other); }
    constexpr bool operator<(string_view other) const { return compare_lexicographically(other) < 0; }
    constexpr bool operator>(string_view other) const { return compare_lexicographically(other) > 0; }
    constexpr bool operator<=(string_view other) const { return !(*this > other); }
    constexpr bool operator>=(string_view other) const { return !(*this < other); }
};

constexpr bool operator==(const char *one, string_view other) {
    return other.compare_lexicographically(string_view(one)) == 0;
}
constexpr bool operator!=(const char *one, string_view other) { return !(one == other); }
constexpr bool operator<(const char *one, string_view other) {
    return other.compare_lexicographically(string_view(one)) > 0;
}
constexpr bool operator>(const char *one, string_view other) {
    return other.compare_lexicographically(string_view(one)) < 0;
}
constexpr bool operator<=(const char *one, string_view other) { return !(one > other); }
constexpr bool operator>=(const char *one, string_view other) { return !(one < other); }

LSTD_END_NAMESPACE
