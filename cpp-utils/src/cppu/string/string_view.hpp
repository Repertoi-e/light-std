#pragma once

#include "../memory/memory.hpp"

#include "string_utils.hpp"

struct string;

// This object represents a non-owning pointer to
// to a utf-8 string and the amount of bytes stored there.
// This is useful when working with literals or just views
// into other _string_s in general (e.g. substrings) and
// avoids having to allocate memory.
struct string_view {
   private:
    struct Iterator : public std::iterator<std::random_access_iterator_tag, char32_t> {
       private:
        const char *Current;

        // Returns a pointer to Current + _n_ utf-8 characters
        constexpr const char *get_current_after(s64 n) const {
            const char *result = Current;
            if (n > 0) {
                for (s32 i : range(n)) {
                    result += get_size_of_code_point(result);
                }
            }
            if (n < 0) {
                for (s32 i : range(n, 0)) {
                    do {
                        result--;
                    } while ((*result & 0xC0) == 0x80);
                }
            }
            return result;
        }

       public:
        constexpr Iterator(const char *data) : Current(data) {}

        constexpr Iterator &operator+=(s64 amount) {
            Current = get_current_after(amount);
            return *this;
        }
        constexpr Iterator &operator-=(s64 amount) {
            Current = get_current_after(-amount);
            return *this += -amount;
        }
        constexpr Iterator &operator++() { return *this += 1; }
        constexpr Iterator &operator--() { return *this -= 1; }
        constexpr Iterator operator++(s32) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }
        constexpr Iterator operator--(s32) {
            Iterator temp = *this;
            --(*this);
            return temp;
        }

        constexpr s64 operator-(const Iterator &other) const {
            s64 difference = 0;
            const char *lesser = Current, *greater = other.Current;
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

        constexpr Iterator operator+(s64 amount) const { return Iterator(get_current_after(amount)); }
        constexpr Iterator operator-(s64 amount) const { return Iterator(get_current_after(-amount)); }

        constexpr friend inline Iterator operator+(s64 amount, const Iterator &it) { return it + amount; }
        constexpr friend inline Iterator operator-(s64 amount, const Iterator &it) { return it - amount; }

        constexpr b32 operator==(const Iterator &other) const { return Current == other.Current; }
        constexpr b32 operator!=(const Iterator &other) const { return Current != other.Current; }
        constexpr b32 operator>(const Iterator &other) const { return Current > other.Current; }
        constexpr b32 operator<(const Iterator &other) const { return Current < other.Current; }
        constexpr b32 operator>=(const Iterator &other) const { return Current >= other.Current; }
        constexpr b32 operator<=(const Iterator &other) const { return Current <= other.Current; }

        constexpr char32_t operator*() const { return decode_code_point(Current); }

        constexpr const char *to_pointer() const { return Current; }
    };

   public:
    using iterator = Iterator;

    const char *Data = null;
    size_t ByteLength = 0;
    size_t Length = 0;

    constexpr string_view() {}
    explicit string_view(const string &str);
    // Construct from a null-terminated c-style string.
    constexpr string_view(const char *str) : string_view(str, str ? cstring_strlen(str) : 0) {}

    // Construct from a c-style string and a size (in code units, not code points)
    constexpr string_view(const char *str, size_t size) {
        Data = str;
        ByteLength = size;
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
    constexpr char32_t get(s64 index) const {
        return decode_code_point(get_pointer_to_code_point_at(Data, Length, index));
    }

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
        size_t beginIndex = translate_index(begin, Length);
        size_t endIndex = translate_index(end - 1, Length) + 1;

        const char *beginPtr = Data;
        for (size_t i = 0; i < beginIndex; i++) beginPtr += get_size_of_code_point(beginPtr);
        const char *endPtr = beginPtr;
        for (size_t i = beginIndex; i < endIndex; i++) endPtr += get_size_of_code_point(endPtr);

        string_view result;
        result.Data = beginPtr;
        result.ByteLength = (uptr_t)(endPtr - beginPtr);
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

        const char *newData = get_pointer_to_code_point_at(Data, Length, n);
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
        for (size_t i = 0; i < Length; i++) {
            char32_t ch = get(i);
            if (!is_space(ch)) break;

            size_t codePointSize = get_size_of_code_point(ch);
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

    b32 begins_with(char32_t ch) const { return get(0) == ch; }
    b32 begins_with(const string_view &other) const { return compare_memory(Data, other.Data, other.ByteLength) == 0; }

    b32 ends_with(char32_t ch) const { return get(-1) == ch; }
    b32 ends_with(const string_view &other) const {
        return compare_memory(Data + ByteLength - other.ByteLength, other.Data, other.ByteLength) == 0;
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
        if (Data == other.Data && ByteLength == other.ByteLength && Length == other.Length) return 0;
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

    operator bool() const { return Length != 0; }

    // Substring operator
    constexpr string_view operator()(s64 begin, s64 end) const { return substring(begin, end); }
};

constexpr b32 operator==(const char *one, const string_view &other) { return other.compare(other) == 0; }
constexpr b32 operator!=(const char *one, const string_view &other) { return !(one == other); }
constexpr b32 operator<(const char *one, const string_view &other) { return other.compare(one) > 0; }
constexpr b32 operator>(const char *one, const string_view &other) { return other.compare(one) < 0; }
constexpr b32 operator<=(const char *one, const string_view &other) { return !(one > other); }
constexpr b32 operator>=(const char *one, const string_view &other) { return !(one < other); }
