#pragma once

#include "../memory/memory.hpp"
#include "../memory/memory_view.hpp"

#include "string_utils.hpp"

LSTD_BEGIN_NAMESPACE

struct string;

// This object represents a non-owning pointer to
// to a utf-8 string. It also contains the amount of bytes stored there,
// as well as the amount of codepoints. (utf8 length != byte length)
// string_view is useful when working with literal strings or when
// you don't want to allocate memory for a new string (eg. a substring)
struct string_view {
    struct iterator {
       private:
        const byte *Current;

        // Returns a pointer to Current + _n_ utf-8 characters
        constexpr const byte *get_current_after(s64 n) const {
            const byte *result = Current;
            if (n > 0) {
                For(range(n)) result += get_size_of_code_point(result);
            }
            if (n < 0) {
                For(range(n, 0)) {
                    do {
                        result--;
                    } while ((*result & 0xC0) == 0x80);
                }
            }
            return result;
        }

       public:
        constexpr iterator(const byte *data) : Current(data) {}

        constexpr iterator &operator+=(s64 amount) {
            Current = get_current_after(amount);
            return *this;
        }
        constexpr iterator &operator-=(s64 amount) {
            Current = get_current_after(-amount);
            return *this;
        }
        constexpr iterator &operator++() { return *this += 1; }
        constexpr iterator &operator--() { return *this -= 1; }
        constexpr iterator operator++(s32) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        constexpr iterator operator--(s32) {
            iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr s64 operator-(const iterator &other) const {
            s64 difference = 0;
            const byte *lesser = Current, *greater = other.Current;
            if (lesser > greater) {
                lesser = other.Current;
                greater = Current;
            }
            while (lesser != greater) {
                lesser += get_size_of_code_point(lesser);
                difference++;
            }
            return Current <= other.Current ? -difference : difference;
        }

        constexpr iterator operator+(s64 amount) const { return iterator(get_current_after(amount)); }
        constexpr iterator operator-(s64 amount) const { return iterator(get_current_after(-amount)); }

        constexpr friend iterator operator+(s64 amount, const iterator &it) { return it + amount; }
        constexpr friend iterator operator-(s64 amount, const iterator &it) { return it - amount; }

        constexpr bool operator==(const iterator &other) const { return Current == other.Current; }
        constexpr bool operator!=(const iterator &other) const { return Current != other.Current; }
        constexpr bool operator>(const iterator &other) const { return Current > other.Current; }
        constexpr bool operator<(const iterator &other) const { return Current < other.Current; }
        constexpr bool operator>=(const iterator &other) const { return Current >= other.Current; }
        constexpr bool operator<=(const iterator &other) const { return Current <= other.Current; }

        constexpr char32_t operator*() const { return decode_code_point(Current); }

        constexpr const byte *to_pointer() const { return Current; }
    };

    const byte *Data = null;
    size_t ByteLength = 0;

    // Length of the string in code points
    size_t Length = 0;

    constexpr string_view() {}

    constexpr string_view(const byte *str) : string_view(memory_view(str)) {}
    constexpr string_view(const byte *str, size_t size) : string_view(memory_view(str, size)) {}

    // Construct from a c-style string and a size (in code units, not code points)
    constexpr string_view(const memory_view &memory) {
        Data = memory.Data;
        ByteLength = memory.ByteLength;
        if (Data) {
            Length = utf8_strlen(Data, ByteLength);
        }
    }

    constexpr string_view(const string_view &other) = default;
    constexpr string_view(string_view &&other) = default;

    // Gets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    constexpr char32_t get(s64 index) const {
        return decode_code_point(get_pointer_to_code_point_at(Data, Length, index));
    }

    // Gets [begin, end) range of characters
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    constexpr string_view substring(s64 begin, s64 end) const {
        // Convert to absolute [begin, end)
        size_t beginIndex = translate_index(begin, Length);
        size_t endIndex = translate_index_unchecked(end, Length);

        assert(endIndex <= Length);

        const byte *beginPtr = get_pointer_to_code_point_at(Data, Length, beginIndex);
        const byte *endPtr = beginPtr;
        For(range(beginIndex, endIndex)) endPtr += get_size_of_code_point(endPtr);

        string_view result;
        result.Data = beginPtr;
        result.ByteLength = (uptr_t)(endPtr - beginPtr);
        result.Length = endIndex - beginIndex;
        return result;
    }

    // Find the first occurence of a code point that is after a specified index
    constexpr size_t find(char32_t cp, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        auto p = begin() + start;
        For(range(start, Length)) if (*p++ == cp) return it;
        return npos;
    }

    // Find the first occurence of a substring that is after a specified index
    constexpr size_t find(const string_view &other, s64 start = 0) const {
        assert(Data);
        assert(other.Data);
        assert(other.Length);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        For(range(start, Length)) {
            auto progress = other.begin();
            for (auto search = begin() + it; progress != other.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == other.end()) return it;
        }
        return npos;
    }

    // Find the last occurence of a code point that is before a specified index
    constexpr size_t find_reverse(char32_t cp, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (*p-- == cp) return it;
        return npos;
    }

    // Find the last occurence of a substring that is before a specified index
    constexpr size_t find_reverse(const string_view &other, s64 start = 0) const {
        assert(Data);
        assert(other.Data);
        assert(other.Length);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        For(range(start - other.Length + 1, -1, -1)) {
            auto progress = other.begin();
            for (auto search = begin() + it; progress != other.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == other.end()) return it;
        }
        return npos;
    }

    // Find the first occurence of any code point in the specified view that is after a specified index
    constexpr size_t find_any_of(const string_view &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        auto p = begin() + start;
        For(range(start, Length)) if (cps.has(*p++)) return it;
        return npos;
    }

    // Find the last occurence of any code point in the specified view
    // that is before a specified index (0 means: start from the end)
    constexpr size_t find_reverse_any_of(const string_view &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (cps.has(*p--)) return it;
        return npos;
    }

    // Find the first absence of a code point that is after a specified index
    constexpr size_t find_not(char32_t cp, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        auto p = begin() + start;
        For(range(start, Length)) if (*p++ != cp) return it;
        return npos;
    }

    // Find the last absence of a code point that is before the specified index
    constexpr size_t find_reverse_not(char32_t cp, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (*p-- != cp) return it;
        return npos;
    }

    // Find the first absence of any code point in the specified view that is after a specified index
    constexpr size_t find_not_any_of(const string_view &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        auto p = begin() + start;
        For(range(start, Length)) if (!cps.has(*p++)) return it;
        return npos;
    }

    // Find the first absence of any code point in the specified view that is after a specified index
    constexpr size_t find_reverse_not_any_of(const string_view &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (!cps.has(*p--)) return it;
        return npos;
    }

    constexpr bool has(char32_t cp) const { return find(cp) != npos; }
    constexpr bool has(const string_view &view) const { return find(view) != npos; }

    constexpr size_t count(char32_t cp) const {
        size_t result = 0, index = 0;
        while ((index = find(cp, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    constexpr size_t count(const string_view &view) const {
        size_t result = 0, index = 0;
        while ((index = find(view, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    // Moves the beginning forwards by n characters.
    constexpr void remove_prefix(size_t n) {
        assert(Data);
        assert(n <= Length);

        const byte *newData = get_pointer_to_code_point_at(Data, Length, n);
        ByteLength -= newData - Data;
        Length -= n;

        Data = newData;
    }

    // Moves the end backwards by n characters.
    constexpr void remove_suffix(size_t n) {
        assert(Data);
        assert(n <= Length);

        ByteLength = get_pointer_to_code_point_at(Data, Length, -(s64) n) - Data;
        Length -= n;
    }

    constexpr string_view trim() const { return trim_start().trim_end(); }
    constexpr string_view trim_start() const {
        assert(Data);

        string_view result = *this;
        For(*this) {
            if (!is_space(it)) break;

            size_t codePointSize = get_size_of_code_point(it);
            result.Data += codePointSize;
            result.ByteLength -= codePointSize;
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
            result.ByteLength -= codePointSize;
            result.Length--;
        }
        return result;
    }

    constexpr bool begins_with(char32_t ch) const { return get(0) == ch; }
    constexpr bool begins_with(const memory_view &other) const {
        return compare_memory_constexpr(Data, other.Data, other.ByteLength) == 0;
    }

    constexpr bool ends_with(char32_t ch) const { return get(-1) == ch; }
    constexpr bool ends_with(const memory_view &other) const {
        return compare_memory_constexpr(Data + ByteLength - other.ByteLength, other.Data, other.ByteLength) == 0;
    }

    // Converts a utf8 string to a null-terminated wide char string (for use with Windows)
    // Assumes _out_ has enough space
    constexpr void to_utf16(wchar_t *out) const {
        auto *p = out;
        For(*this) {
            if (it > 0xffff) {
                *p++ = (u16)((it >> 10) + (0xd800u - (0x10000 >> 10)));
                *p++ = (u16)((it & 0x3ff) + 0xdc00u);
            } else {
                *p++ = (u16) it;
            }
        }
        *p = 0;
    }

    // Converts a utf8 string to a null-terminated utf32 string
    // Assumes _out_ has enough space
    constexpr void to_utf32(char32_t *out) const {
        auto *p = out;
        For(*this) { *p++ = it; }
        *p = 0;
    }

    constexpr void swap(string_view &other) {
        {
            auto temp = Data;
            Data = other.Data;
            other.Data = temp;
        }
        {
            auto temp = ByteLength;
            ByteLength = other.ByteLength;
            other.ByteLength = temp;
        }
        {
            auto temp = Length;
            Length = other.Length;
            other.Length = temp;
        }
    }

    constexpr iterator begin() const { return iterator(Data); }
    constexpr iterator end() const { return iterator(Data + ByteLength); }

    // Compares the string view to _other_ lexicographically.
    // The result is less than 0 if this string_view sorts before the other,
    // 0 if they are equal, and greater than 0 otherwise.
    constexpr s32 compare(const string_view &other) const {
        // If the memory and the lengths are the same, the views are equal!
        if (Data == other.Data && ByteLength == other.ByteLength) return 0;

        if (Length == 0 && other.Length == 0) return 0;
        if (Length == 0) return -((s32) other.get(0));
        if (other.Length == 0) return get(0);

        auto s1 = begin(), s2 = other.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == other.end()) return 0;
            if (s1 == end()) return -((s32) other.get(0));
            if (s2 == other.end()) return get(0);
        }
        return ((s32) *s1 - (s32) *s2);
    }

    // Compares the string view to _other_ lexicographically. Case insensitive.
    // The result is less than 0 if this string_view sorts before the other,
    // 0 if they are equal, and greater than 0 otherwise.
    constexpr s32 compare_ignore_case(const string_view &other) const {
        // If the memory and the lengths are the same, the views are equal!
        if (Data == other.Data && ByteLength == other.ByteLength) return 0;
        if (Length == 0 && other.Length == 0) return 0;
        if (Length == 0) return -((s32) to_lower(other.get(0)));
        if (other.Length == 0) return to_lower(get(0));

        auto s1 = begin(), s2 = other.begin();
        while (to_lower(*s1) == to_lower(*s2)) {
            ++s1, ++s2;
            if (s1 == end() && s2 == other.end()) return 0;
            if (s1 == end()) return -((s32) to_lower(other.get(0)));
            if (s2 == other.end()) return to_lower(get(0));
        }
        return ((s32) to_lower(*s1) - (s32) to_lower(*s2));
    }

    // Check two string views for equality
    constexpr bool operator==(const string_view &other) const { return compare(other) == 0; }
    constexpr bool operator!=(const string_view &other) const { return !(*this == other); }
    constexpr bool operator<(const string_view &other) const { return compare(other) < 0; }
    constexpr bool operator>(const string_view &other) const { return compare(other) > 0; }
    constexpr bool operator<=(const string_view &other) const { return !(*this > other); }
    constexpr bool operator>=(const string_view &other) const { return !(*this < other); }

    constexpr string_view &operator=(const string_view &other) = default;
    constexpr string_view &operator=(string_view &&other) = default;

    // Read-only [] operator
    constexpr char32_t operator[](s64 index) const { return get(index); }

    operator bool() const { return Length != 0; }
    operator memory_view() const { return memory_view(Data, ByteLength); }

    // Substring operator
    constexpr string_view operator()(s64 begin, s64 end) const { return substring(begin, end); }
};

constexpr bool operator==(const byte *one, const string_view &other) { return other.compare(string_view(one)) == 0; }
constexpr bool operator!=(const byte *one, const string_view &other) { return !(one == other); }
constexpr bool operator<(const byte *one, const string_view &other) { return other.compare(string_view(one)) > 0; }
constexpr bool operator>(const byte *one, const string_view &other) { return other.compare(string_view(one)) < 0; }
constexpr bool operator<=(const byte *one, const string_view &other) { return !(one > other); }
constexpr bool operator>=(const byte *one, const string_view &other) { return !(one < other); }

LSTD_END_NAMESPACE
