#pragma once

/// Provides ascii and utf8 string utility functions

#include "../internal/common.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
using c_string_type = types::remove_const_t<types::remove_pointer_t<T>> *;

// @Cleanup This looks messy
template <typename T>
concept c_string = types::is_same<c_string_type<T>, char8_t *> || types::is_same<c_string_type<T>, utf8 *> || types::is_same<c_string_type<T>, utf16 *> || types::is_same<c_string_type<T>, utf32 *>;

// Retrieve the length of a standard cstyle string. Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in bytes of a null terminated utf8 string.
template <c_string T>
constexpr s64 c_string_length(T str) {
    if (!str) return 0;

    s64 length = 0;
    while (*str++) ++length;
    return length;
}

// Retrieve the length (in code points) for a utf8 string
constexpr s64 utf8_length(const utf8 *str, s64 size) {
    if (!str || size == 0) return 0;

    s64 length = 0;
    while (size--) {
        if (!((*str++ & 0xc0) == 0x80)) ++length;
    }
    return length;
}

template <c_string T>
constexpr s64 compare_c_string(T one, T other) {
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

template <c_string T>
constexpr s32 compare_c_string_lexicographically(T one, T other) {
    assert(one);
    assert(other);

    while (*one && (*one == *other)) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

// This function only works for ascii
constexpr bool is_digit(utf32 x) { return x >= '0' && x <= '9'; }

// This function only works for ascii
constexpr bool is_hex_digit(utf32 x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F'); }

// This function only works for ascii
constexpr bool is_space(utf32 x) { return (x >= 9 && x <= 13) || x == 32; }

// This function only works for ascii
constexpr bool is_blank(utf32 x) { return x == 9 || x == 32; }

// This function only works for ascii
constexpr bool is_alpha(utf32 x) { return (x >= 65 && x <= 90) || (x >= 97 && x <= 122); }

// This function only works for ascii
constexpr bool is_alphanumeric(utf32 x) { return is_alpha(x) || is_digit(x); }

// This function only works for ascii
constexpr bool is_identifier_start(utf32 x) { return is_alpha(x) || x == '_'; }

// This function only works for ascii
constexpr bool is_print(utf32 x) { return x > 31 && x != 127; }

// Convert code point to uppercase
constexpr utf32 to_upper(utf32 cp) {
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
    // No uppercase!
    return cp;
}

// Convert code point to lowercase
constexpr utf32 to_lower(utf32 cp) {
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

constexpr bool is_upper(utf32 ch) { return ch != to_lower(ch); }
constexpr bool is_lower(utf32 ch) { return ch != to_upper(ch); }

constexpr s64 compare_c_string_ignore_case(const utf8 *one, const utf8 *other) {
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

constexpr s64 compare_c_string_ignore_case(const utf16 *one, const utf16 *other) {
    assert(one);
    assert(other);

    if (!*one && !*other) return -1;

    s64 index = 0;
    while (to_lower((utf32) *one) == to_lower((utf32) *other)) {
        ++one, ++other;
        if (!*one && !*other) return -1;
        if (!*one || !*other) return index;
        ++index;
    }
    return index;
}

constexpr s64 compare_c_string_ignore_case(const utf32 *one, const utf32 *other) {
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

constexpr s32 compare_c_string_lexicographically_ignore_case(const utf8 *one, const utf8 *other) {
    assert(one);
    assert(other);

    while (*one && (to_lower(*one) == to_lower(*other))) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

constexpr s32 compare_c_string_lexicographically_ignore_case(const utf16 *one, const utf16 *other) {
    assert(one);
    assert(other);

    while (*one && (to_lower((utf32) *one) == to_lower((utf32) *other))) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

constexpr s32 compare_c_string_lexicographically_ignore_case(const utf32 *one, const utf32 *other) {
    assert(one);
    assert(other);

    while (*one && (to_lower(*one) == to_lower(*other))) ++one, ++other;
    return (*one > *other) - (*other > *one);
}

// Returns the size in bytes of the code point that _str_ points to.
// If the byte pointed by _str_ is a countinuation utf8 byte, this function returns 0
constexpr s8 get_size_of_cp(const utf8 *str) {
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
constexpr s8 get_size_of_cp(utf32 codePoint) {
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
constexpr void encode_cp(utf8 *str, utf32 codePoint) {
    s64 size = get_size_of_cp(codePoint);
    if (size == 1) {
        // 1-byte/7-bit ascii
        // (0b0xxxxxxx)
        str[0] = (utf8) codePoint;
    } else if (size == 2) {
        // 2-byte/11-bit utf8 code point
        // (0b110xxxxx 0b10xxxxxx)
        str[0] = 0xc0 | (utf8)(codePoint >> 6);
        str[1] = 0x80 | (utf8)(codePoint & 0x3f);
    } else if (size == 3) {
        // 3-byte/16-bit utf8 code point
        // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xe0 | (utf8)(codePoint >> 12);
        str[1] = 0x80 | (utf8)((codePoint >> 6) & 0x3f);
        str[2] = 0x80 | (utf8)(codePoint & 0x3f);
    } else {
        // 4-byte/21-bit utf8 code point
        // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
        str[0] = 0xf0 | (utf8)(codePoint >> 18);
        str[1] = 0x80 | (utf8)((codePoint >> 12) & 0x3f);
        str[2] = 0x80 | (utf8)((codePoint >> 6) & 0x3f);
        str[3] = 0x80 | (utf8)(codePoint & 0x3f);
    }
}

// Decodes a code point from a data pointer
constexpr utf32 decode_cp(const utf8 *str) {
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

// Checks whether the encoded code point in data is valid utf8
//
// @Speed @Speed @Speed @Speed @Speed @Speed @Speed @Speed @Speed
// @Speed @Speed @Speed @Speed @Speed @Speed @Speed @Speed @Speed
// @Speed @Speed @Speed @Speed @Speed @Speed @Speed @Speed @Speed
constexpr bool is_valid_utf8(const utf8 *data) {
    u8 *p = (u8 *) data;

    s64 sizeOfCp = get_size_of_cp(data);
    if (sizeOfCp == 1) {
        if ((s8) *data < 0) return false;
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

//
//
// @TODO: We use this for arrays too!
// Maybe move it somewhere else?

// This function translates an index that may be negative to an actual index.
// For example 5 maps to 5
// but -5 maps to length - 5
// This function is used to support python-like negative indexing.
//
// This function checks if the index is in range.
//
// If _toleratePastLast_ is true, pointing to one past the end is accepted.
constexpr always_inline s64 translate_index(s64 index, s64 length, bool toleratePastLast = false) {
    s64 checkLength = toleratePastLast ? (length + 1) : length;

    if (index < 0) {
        s64 actual = length + index;
        assert(actual >= 0);
        assert(actual < checkLength);
        return actual;
    }
    assert(index < checkLength);
    return index;
}

// This returns a pointer to the code point at a specified index in an utf8 string.
// If _toleratePastLast_ is true, pointing to one past the end is accepted.
constexpr const utf8 *get_cp_at_index(const utf8 *str, s64 length, s64 index, bool toleratePastLast = false) {
    For(range(translate_index(index, length, toleratePastLast))) str += get_size_of_cp(str);
    return str;
}
//
//

// Converts utf8 to utf16 and stores in _out_ (assumes there is enough space).
// Also adds a null-terminator at the end.
constexpr void utf8_to_utf16(const utf8 *str, s64 length, utf16 *out) {
    For(range(length)) {
        utf32 cp = decode_cp(str);
        if (cp > 0xffff) {
            *out++ = (u16)((cp >> 10) + (0xD800u - (0x10000 >> 10)));
            *out++ = (u16)((cp & 0x3FF) + 0xDC00u);
        } else {
            *out++ = (u16) cp;
        }
        str += get_size_of_cp(cp);
    }
    *out = 0;
}

// Converts utf8 to utf32 and stores in _out_ (assumes there is enough space).
// Also adds a null-terminator at the end.
constexpr void utf8_to_utf32(const utf8 *str, s64 length, utf32 *out) {
    For(range(length)) {
        utf32 cp = decode_cp(str);
        *out++ = cp;
        str += get_size_of_cp(cp);
    }
    *out = 0;
}

// Converts a null-terminated utf16 to utf8 and stores in _out_ and _outByteLength_ (assumes there is enough space).
constexpr void utf16_to_utf8(const utf16 *str, utf8 *out, s64 *outByteLength) {
    s64 byteLength = 0;
    while (*str) {
        utf32 cp = *str;
        if ((cp >= 0xD800) && (cp <= 0xDBFF)) {
            utf32 trail = cp = *++str;
            if (!*str) assert(false && "Invalid utf16 string");

            if ((trail >= 0xDC00) && (trail <= 0xDFFF)) {
                cp = ((cp - 0xD800) << 10) + (trail - 0xDC00) + 0x0010000;
            } else {
                assert(false && "Invalid utf16 string");
            }
        }

        encode_cp(out, cp);
        s64 cpSize = get_size_of_cp(out);
        out += cpSize;
        byteLength += cpSize;
        ++str;
    }
    *outByteLength = byteLength;
}

// Converts a null-terminated utf32 to utf8 and stores in _out_ and _outByteLength_ (assumes there is enough space).
constexpr void utf32_to_utf8(const utf32 *str, utf8 *out, s64 *outByteLength) {
    s64 byteLength = 0;
    while (*str) {
        encode_cp(out, *str);
        s64 cpSize = get_size_of_cp(out);
        out += cpSize;
        byteLength += cpSize;
        ++str;
    }
    *outByteLength = byteLength;
}

LSTD_END_NAMESPACE
