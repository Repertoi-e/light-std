#pragma once

#include "../memory/memory.h"

#include <iterator>

// This is a constexpr function for working with cstyle strings at compile time
// Retrieve the length of a standard cstyle string (==strlen)
// Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in
// bytes of a null terminated utf-8 string.
constexpr size_t cstring_strlen(const char *str) {
    size_t length = 0;
    while (*str++) length++;
    return length;
}

// These functions only work for ascii
constexpr b32 is_digit(char32_t x) { return x >= '0' && x <= '9'; }
// These functions only work for ascii
constexpr b32 is_hexadecimal_digit(char32_t x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f'); }

// These functions only work for ascii
constexpr b32 is_space(char32_t x) { return (x >= 9 && x <= 13) || x == 32; }
// These functions only work for ascii
constexpr b32 is_blank(char32_t x) { return x == 9 || x == 32; }

// These functions only work for ascii
constexpr b32 is_alpha(char32_t x) { return (x >= 65 && x <= 90) || (x >= 97 && x <= 122); }
// These functions only work for ascii
constexpr b32 is_alphanumeric(char32_t x) { return is_alpha(x) || is_digit(x); }

constexpr b32 is_identifier_start(char32_t x) { return is_alpha(x) || x == '_'; }

// These functions only work for ascii
constexpr b32 is_print(char32_t x) { return x > 31 && x != 127; }

//
// Utility utf-8 functions:
//

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

// Returns the size in bytes of the code point that _str_ points to.
// This function reads the first byte and returns that result.
// If the byte pointed by _str_ is a countinuation utf-8 byte, this
// function returns 0
constexpr size_t get_size_of_code_point(const char *str) {
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
constexpr size_t get_size_of_code_point(char32_t codePoint) {
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
constexpr void encode_code_point(char *str, char32_t codePoint) {
    size_t size = get_size_of_code_point(codePoint);
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
constexpr char32_t decode_code_point(const char *str) {
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

struct string;

// This object represents a non-owning pointer to
// to a utf-8 string and the amount of bytes stored there.
// This is useful when working with literals or just views
// into other _string_s in general (e.g. substrings) and
// avoids having to allocate memory.
struct string_view {
    struct Iterator : public std::iterator<std::random_access_iterator_tag, char32_t> {
       private:
        const string_view &Parent;
        size_t Index;

       public:
        constexpr Iterator(const string_view &str, s64 index = 0) : Parent(str), Index(0) {
            if (index < (s64) str.Length && index >= 0) {
                Index = index;
            } else {
                Index = npos;
            }
        }

        constexpr Iterator(const Iterator &other) : Parent(other.Parent), Index(other.Index) {}
        constexpr Iterator(Iterator &&other) : Parent(other.Parent), Index(other.Index) {}

        constexpr Iterator &operator=(const Iterator &other) {
            assert(Parent == other.Parent);
            Index = other.Index;
            return *this;
        }

        constexpr Iterator &operator=(Iterator &&other) {
            assert(Parent == other.Parent);
            Index = other.Index;
            return *this;
        }

        constexpr Iterator &operator+=(s64 amount) {
            if ((amount < 0 && (s64) Index + amount < 0) || Index + amount >= Parent.Length) {
                Index = npos;
            } else {
                Index += amount;
            }
            return *this;
        }
        constexpr Iterator &operator-=(s64 amount) {
            assert(Index != npos);
            return *this += -amount;
        }
        constexpr Iterator &operator++() {
            assert(Index != npos);
            return *this += 1;
        }
        constexpr Iterator &operator--() {
            assert(Index != npos);
            return *this -= 1;
        }
        constexpr Iterator operator++(s32) {
            assert(Index != npos);
            Iterator tmp(*this);
            ++(*this);
            return tmp;
        }
        constexpr Iterator operator--(s32) {
            assert(Index != npos);
            Iterator tmp(*this);
            --(*this);
            return tmp;
        }

        constexpr s64 operator-(const Iterator &other) const {
            size_t index = Index;
            if (index == npos) index = Parent.Length;
            size_t otherIndex = other.Index;
            if (otherIndex == npos) otherIndex = other.Parent.Length;

            return (s64) index - (s64) otherIndex;
        }
        constexpr Iterator operator+(s64 amount) const {
            assert(Index != npos);
            return Iterator(Parent, Index + amount);
        }
        constexpr Iterator operator-(s64 amount) const {
            assert(Index != npos);
            return Iterator(Parent, Index - amount);
        }
        constexpr friend inline Iterator operator+(s64 amount, const Iterator &it) {
            assert(it.Index != npos);
            return it + amount;
        }
        constexpr friend inline Iterator operator-(s64 amount, const Iterator &it) {
            assert(it.Index != npos);
            return it - amount;
        }

        constexpr b32 operator==(const Iterator &other) const { return Index == other.Index; }
        constexpr b32 operator!=(const Iterator &other) const { return Index != other.Index; }
        constexpr b32 operator>(const Iterator &other) const { return Index > other.Index; }
        constexpr b32 operator<(const Iterator &other) const { return Index < other.Index; }
        constexpr b32 operator>=(const Iterator &other) const { return Index >= other.Index; }
        constexpr b32 operator<=(const Iterator &other) const { return Index <= other.Index; }

        constexpr char32_t operator*() const { return Parent[Index]; }
        constexpr char32_t operator[](s64 index) const { return Parent[Index + index]; }

        // Returns whether this iterator contains a valid index.
        constexpr b32 valid() const { return Index != npos; }
        constexpr const char *to_pointer() const { return Parent._get_pointer_to_index((s64) Index); }
    };

    const char *Data = null;
    size_t BytesUsed = 0;
    size_t Length = 0;

    constexpr string_view() {}
    explicit string_view(const string &str);
    // Construct from a null-terminated c-style string.
    constexpr string_view(const char *str) : string_view(str, str ? cstring_strlen(str) : 0) {}

    // Construct from a c-style string and a size (in code units, not code points)
    constexpr string_view(const char *str, size_t size) {
        Data = str;
        BytesUsed = size;
        if (Data) {
            const char *end = str + size;
            while (str < end) {
                str += get_size_of_code_point(str);
                Length++;
            }
        }
    }

    constexpr string_view(const string_view &other) = default;
    constexpr string_view(string_view &&other) = default;

    // Gets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    constexpr char32_t get(s64 index) const { return decode_code_point(_get_pointer_to_index(index)); }

    // Gets [begin, end) range of characters
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    // Note that the string returned is a view into _str_.
    //    It's not actually an allocated string, so it _str_ gets
    //    destroyed, then the returned string will be pointing to
    //      invalid memory. Copy the returned string explicitly if
    //      you intend to use it longer than this string.
    constexpr string_view substring(s64 begin, s64 end) const {
        // Convert to absolute [begin, end)
        size_t beginIndex = _translate_index(begin);
        size_t endIndex = _translate_index(end - 1) + 1;

        const char *beginPtr = Data;
        for (size_t i = 0; i < beginIndex; i++) beginPtr += get_size_of_code_point(beginPtr);
        const char *endPtr = beginPtr;
        for (size_t i = beginIndex; i < endIndex; i++) endPtr += get_size_of_code_point(endPtr);

        string_view result;
        result.Data = beginPtr;
        result.BytesUsed = (uptr_t)(endPtr - beginPtr);
        result.Length = endIndex - beginIndex;
        return result;
    }

    // Find the first occurence of _ch_
    constexpr size_t find(char32_t ch) const {
        assert(Data);
        for (size_t i = 0; i < Length; ++i)
            if (get(i) == ch) return i;
        return npos;
    }

    // Find the first occurence of _other_
    constexpr size_t find(const string_view &other) const {
        assert(Data);
        assert(other.Data);

        for (size_t haystack = 0; haystack < Length; ++haystack) {
            size_t i = haystack;
            size_t n = 0;
            while (n < other.Length && get(i) == other.get(n)) {
                ++n;
                ++i;
            }
            if (n == other.Length) {
                return haystack;
            }
        }
        return npos;
    }

    // Find the last occurence of _ch_
    constexpr size_t find_last(char32_t ch) const {
        assert(Data);
        for (size_t i = Length - 1; i >= 0; --i)
            if (get(i) == ch) return i;
        return npos;
    }

    // Find the last occurence of _other_
    constexpr size_t find_last(const string_view &other) const {
        assert(Data);
        assert(other.Data);

        for (size_t haystack = Length - 1; haystack >= 0; --haystack) {
            size_t i = haystack;
            size_t n = 0;
            while (n < other.Length && get(i) == other.get(n)) {
                ++n;
                ++i;
            }
            if (n == other.Length) {
                return haystack;
            }
        }
        return npos;
    }

    constexpr b32 has(char32_t ch) const { return find(ch) != npos; }
    constexpr b32 has(const string_view &other) const { return find(other) != npos; }

    // Moves the beginning forwards by n characters.
    constexpr void remove_prefix(size_t n) {
        assert(Data);
        assert(n <= Length);

        const char *newData = _get_pointer_to_index(n);
        BytesUsed -= newData - Data;
        Length -= n;

        Data = newData;
    }

    // Moves the end backwards by n characters.
    constexpr void remove_suffix(size_t n) {
        assert(Data);
        assert(n <= Length);

        BytesUsed = _get_pointer_to_index(-((s64)n)) - Data;
        Length -= n;
    }

    constexpr string_view trim() const { return trim_start().trim_end(); }
    constexpr string_view trim_start() const {
        assert(Data);

        string_view result = *this;
        for (size_t i = 0; i < Length; i++) {
            char32_t ch = get(i);
            if (!is_space(ch)) break;

            size_t codePointSize = get_size_of_code_point(ch);
            result.Data += codePointSize;
            result.BytesUsed -= codePointSize;
            result.Length--;
        }
        return result;
    }

    constexpr string_view trim_end() const {
        assert(Data);

        string_view result = *this;
        for (s64 i = 1; (size_t) i <= Length; i++) {
            char32_t ch = get(-i);
            if (!is_space(ch)) break;

            size_t codePointSize = get_size_of_code_point(ch);
            result.BytesUsed -= codePointSize;
            result.Length--;
        }
        return result;
    }

    b32 begins_with(char32_t ch) const { return get(0) == ch; }
    b32 begins_with(const string_view &other) const { return CompareMemory(Data, other.Data, other.BytesUsed) == 0; }

    b32 ends_with(char32_t ch) const { return get(-1) == ch; }
    b32 ends_with(const string_view &other) const {
        return CompareMemory(Data + BytesUsed - other.BytesUsed, other.Data, other.BytesUsed) == 0;
    }

    constexpr void swap(string_view &other) {
        // Sigh.
        // std::swap(Data, other.Data);
        // std::swap(BytesUsed, other.BytesUsed);
        // std::swap(Length, other.Length);
        {
            auto temp = Data;
            Data = other.Data;
            other.Data = temp;
        }
        {
            auto temp = BytesUsed;
            BytesUsed = other.BytesUsed;
            other.BytesUsed = temp;
        }
        {
            auto temp = Length;
            Length = other.Length;
            other.Length = temp;
        }
    }

    constexpr Iterator begin() const { return Iterator(*this, 0); }
    constexpr Iterator end() const { return Iterator(*this, npos); }

    // Compares the string view to _other_ lexicographically.
    // The result is less than 0 if this string_view sorts before the other,
    // 0 if they are equal, and greater than 0 otherwise.
    constexpr s32 compare(const string_view &other) const {
        if (Data == other.Data && BytesUsed == other.BytesUsed && Length == other.Length) return 0;
        if (Length == 0 && other.Length == 0) return 0;
        if (Length == 0) return -((s32) other.get(0));
        if (other.Length == 0) return get(0);

        Iterator s1 = begin(), s2 = other.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == other.end()) return 0;
            if (s1 == end() || s2 == other.end()) break;
        }
        if (s1 == end()) return -((s32) other.get(0));
        if (s2 == other.end()) return get(0);

        return ((s32) *s1 - (s32) *s2);
    }

    // Check two string views for equality
    constexpr b32 operator==(const string_view &other) const { return compare(other) == 0; }
    constexpr b32 operator!=(const string_view &other) const { return !(*this == other); }
    constexpr b32 operator<(const string_view &other) const { return compare(other) < 0; }
    constexpr b32 operator>(const string_view &other) const { return compare(other) > 0; }
    constexpr b32 operator<=(const string_view &other) const { return !(*this > other); }
    constexpr b32 operator>=(const string_view &other) const { return !(*this < other); }

    constexpr string_view &operator=(const string_view &other) = default;
    constexpr string_view &operator=(string_view &&other) = default;

    // Read-only [] operator
    constexpr const char32_t operator[](s64 index) const { return get(index); }

    // Substring operator
    constexpr string_view operator()(s64 begin, s64 end) const { return substring(begin, end); }

    constexpr size_t _translate_index(s64 index) const {
        if (index < 0) {
            s64 actual = Length + index;
            assert(actual >= 0);
            assert((size_t) actual < Length);
            return (size_t) actual;
        }
        assert((size_t) index < Length);
        return (size_t) index;
    }

    constexpr const char *_get_pointer_to_index(s64 index) const {
        size_t actualIndex = _translate_index(index);

        const char *s = Data;
        for (size_t i = 0; i < actualIndex; i++) s += get_size_of_code_point(s);
        return s;
    }
};
