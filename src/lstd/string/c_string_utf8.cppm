///
/// This file provides ASCII, null-terminated string and utf-8 utility functions:
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

module;

#include "../common.h"

export module lstd.c_string_utf8;

LSTD_BEGIN_NAMESPACE

template <typename C>
using c_string_type = add_pointer_t<remove_cvref_t<remove_pointer_t<remove_cvref_t<C>>>>;

export {
    // C++ mess
    template <typename C>
    concept any_c_string = is_pointer<C> && is_same_to_one_of < c_string_type<C>,
    char *, wchar *, char8_t *, char16_t *, char32_t *, code_point * > ;

    template <typename C>
    concept any_byte_pointer = is_pointer<C> && is_same_to_one_of < c_string_type<C>,
    char *, char8_t *, unsigned char *, s8 *, u8 *, byte *> ;

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
    s64 utf8_length(const char *str, s64 size) {
        if (!str || size == 0) return 0;

        // Count all first-bytes (the ones that don't match 10xxxxxx).
        s64 length = 0;
        while (size--) {
            if (!((*str++ & 0xc0) == 0x80)) ++length;
        }
        return length;
    }

    // Returns -1 if strings match, else returns the index of the first different byte
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

    // Return -1 if one < other, 0 if one == other and 1 if one > other (not the pointers)
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
    bool is_digit(code_point x) { return x >= '0' && x <= '9'; }

    // This function only works for ascii
    bool is_hex_digit(code_point x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F'); }

    // This function only works for ascii
    bool is_space(code_point x) { return (x >= 9 && x <= 13) || x == 32; }

    // This function only works for ascii
    bool is_blank(code_point x) { return x == 9 || x == 32; }

    // This function only works for ascii
    bool is_alpha(code_point x) { return (x >= 65 && x <= 90) || (x >= 97 && x <= 122); }

    // This function only works for ascii
    bool is_alphanumeric(code_point x) { return is_alpha(x) || is_digit(x); }

    // This function only works for ascii
    bool is_identifier_start(code_point x) { return is_alpha(x) || x == '_'; }

    // This function only works for ascii
    bool is_print(code_point x) { return x > 31 && x != 127; }

    // Convert code point to uppercase
    code_point to_upper(code_point cp) {
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
    code_point to_lower(code_point cp) {
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

    bool is_upper(code_point ch) { return ch != to_lower(ch); }
    bool is_lower(code_point ch) { return ch != to_upper(ch); }

    // Returns -1 if strings match, else returns the index of the first different byte.
    // Ignores the case of the characters.
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

    // Return -1 if one < other, 0 if one == other and 1 if one > other (not the pointers).
    // Ignores the case of the characters.
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
    bool strings_match(C one, C other) { return compare_string(one, other) == -1; }

    // true if strings are equal (not the pointers)
    template <any_c_string C>
    bool strings_match_ignore_case(C one, C other) { return compare_string_ignore_case(one, other) == -1; }

    // Returns the size in bytes of the code point that _str_ points to.
    // If the byte pointed by _str_ is a countinuation utf-8 byte, this function returns 0.
    s8 utf8_get_size_of_cp(const char *str) {
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
    s8 utf8_get_size_of_cp(code_point codePoint) {
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

    // Encodes code point at _str_, assumes there is enough space
    void utf8_encode_cp(char *str, code_point codePoint) {
        s64 size = utf8_get_size_of_cp(codePoint);
        if (size == 1) {
            // 1-byte/7-bit ascii
            // (0b0xxxxxxx)
            str[0] = (char) codePoint;
        } else if (size == 2) {
            // 2-byte/11-bit utf-8 code point
            // (0b110xxxxx 0b10xxxxxx)
            str[0] = 0xc0 | (char) (codePoint >> 6);
            str[1] = 0x80 | (char) (codePoint & 0x3f);
        } else if (size == 3) {
            // 3-byte/16-bit utf-8 code point
            // (0b1110xxxx 0b10xxxxxx 0b10xxxxxx)
            str[0] = 0xe0 | (char) (codePoint >> 12);
            str[1] = 0x80 | (char) ((codePoint >> 6) & 0x3f);
            str[2] = 0x80 | (char) (codePoint & 0x3f);
        } else {
            // 4-byte/21-bit utf-8 code point
            // (0b11110xxx 0b10xxxxxx 0b10xxxxxx 0b10xxxxxx)
            str[0] = 0xf0 | (char) (codePoint >> 18);
            str[1] = 0x80 | (char) ((codePoint >> 12) & 0x3f);
            str[2] = 0x80 | (char) ((codePoint >> 6) & 0x3f);
            str[3] = 0x80 | (char) (codePoint & 0x3f);
        }
    }

    // Decodes a code point from a data pointer
    code_point utf8_decode_cp(const char *str) {
        if (0xf0 == (0xf8 & str[0])) {
            // 4 byte utf-8 code point
            return ((0x07 & str[0]) << 18) | ((0x3f & str[1]) << 12) | ((0x3f & str[2]) << 6) | (0x3f & str[3]);
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
    bool utf8_is_valid_cp(const char *data) {
        u8 *p = (u8 *) data;

        s64 sizeOfCp = utf8_get_size_of_cp(data);
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

    // This returns a pointer to the code point at a specified index in an utf-8 string.
    // This is unsafe, doesn't check if we go over bounds. In the general case you should
    // call this with a result from translate_negative_index(...), which handles out of bounds indexing.
    //
    // If LSTD_ARRAY_BOUNDS_CHECK is defined this fails if we go out of bounds.
    //
    // @Speed @TODO Vectorize for large strings
    const char *utf8_get_pointer_to_cp_at_translated_index(const char *str, s64 byteLength, s64 index) {
        auto *end = str + byteLength;

        For(range(index)) {
            // Danger danger. If the string contains invalid utf8, then we might bypass str == end.
            // That's why we check with >=.

            if (str >= end) {
                assert(false && "Out of bounds");
            }

            str += utf8_get_size_of_cp(str);
        }
        return str;
    }

    // Converts utf-8 to utf-16 and stores in _out_ (assumes there is enough space).
    // Also adds a null-terminator at the end.
    void utf8_to_utf16(const char *str, s64 length, wchar *out) {
        For(range(length)) {
            code_point cp = utf8_decode_cp(str);
            if (cp > 0xffff) {
                *out++ = (u16) ((cp >> 10) + (0xD800u - (0x10000 >> 10)));
                *out++ = (u16) ((cp & 0x3FF) + 0xDC00u);
            } else {
                *out++ = (u16) cp;
            }
            str += utf8_get_size_of_cp(cp);
        }
        *out = 0;
    }

    // Converts utf-8 to utf-32 and stores in _out_ (assumes there is enough space).
    //
    // Also adds a null-terminator at the end.
    void utf8_to_utf32(const char *str, s64 byteLength, code_point *out) {
        auto *end = str + byteLength;

        // Danger danger. If the string contains invalid utf8, then we might bypass str != end and infinite loop.
        // That's why we check with <.
        while (str < end) {
            code_point cp = utf8_decode_cp(str);
            *out++        = cp;
            str += utf8_get_size_of_cp(cp);
        }

        *out = 0;
    }

    // Converts a null-terminated utf-16 to utf-8 and stores in _out_ and _outByteLength_ (assumes there is enough space).
    void utf16_to_utf8(const wchar *str, char *out, s64 *outByteLength) {
        s64 byteLength = 0;
        while (*str) {
            code_point cp = *str;
            if ((cp >= 0xD800) && (cp <= 0xDBFF)) {
                code_point trail = cp = *++str;
                if (!*str) assert(false && "Invalid wchar string");  // @TODO @Robustness Bail on errors

                if ((trail >= 0xDC00) && (trail <= 0xDFFF)) {
                    cp = ((cp - 0xD800) << 10) + (trail - 0xDC00) + 0x0010000;
                } else {
                    assert(false && "Invalid wchar string");  // @TODO @Robustness Bail on errors
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

    // Converts a null-terminated utf-32 to utf-8 and stores in _out_ and _outByteLength_ (assumes there is enough space).
    void utf32_to_utf8(const code_point *str, char *out, s64 *outByteLength) {
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
}

LSTD_END_NAMESPACE
