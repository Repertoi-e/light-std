#pragma once

/// Provides a string type

#include "string_utils.h"

LSTD_BEGIN_NAMESPACE

// UTF-8 string
// This string doesn't guarantee a null termination at the end.
//
// This object may represents a non-owning pointer to to a utf8 string or
// a pointer to an allocated memory block. Copying it does a shallow copy.
// In order to get a new string object and deep copy the contents use clone().
//
// Also contains the amount of bytes used to represent the string,
// as well as the the length in code points.
//
// Methods in this object allow negative reversed indexing which begins at
// the end of the string, so -1 is the last character -2 the one before that, etc. (Python-style)
//
// This type is partially constexpr.
// (allocations, mutability, etc. obviously aren't supported at compile time, but everything else is)
//
// @TODO: Small string optimization
struct string {
    struct code_point {
        string *Parent;
        size_t Index;

        code_point(string *parent, size_t index) : Parent(parent), Index(index) {}

        code_point &operator=(char32_t other);
        operator char32_t() const;
    };

    template <bool Mutable>
    struct iterator_impl {
        using parent_t = type_select_t<Mutable, string, const string>;

        parent_t *Parent = null;
        size_t Index = 0;

        constexpr iterator_impl(parent_t *parent, size_t index) : Parent(parent), Index(index) {}

        constexpr iterator_impl &operator+=(s64 amount) { return Index += amount, *this; }
        constexpr iterator_impl &operator-=(s64 amount) { return Index -= amount, *this; }
        constexpr iterator_impl &operator++() { return *this += 1; }
        constexpr iterator_impl &operator--() { return *this -= 1; }
        constexpr iterator_impl operator++(s32) {
            iterator_impl temp = *this;
            return ++(*this), temp;
        }

        constexpr iterator_impl operator--(s32) {
            iterator_impl temp = *this;
            return --(*this), temp;
        }

        constexpr s64 operator-(const iterator_impl &other) const {
            size_t lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        constexpr iterator_impl operator+(s64 amount) const { return iterator_impl(Parent, Index + amount); }
        constexpr iterator_impl operator-(s64 amount) const { return iterator_impl(Parent, Index - amount); }

        friend iterator_impl operator+(s64 amount, const iterator_impl &it) { return it + amount; }
        friend iterator_impl operator-(s64 amount, const iterator_impl &it) { return it - amount; }

        constexpr bool operator==(const iterator_impl &other) const { return Index == other.Index; }
        constexpr bool operator!=(const iterator_impl &other) const { return Index != other.Index; }
        constexpr bool operator>(const iterator_impl &other) const { return Index > other.Index; }
        constexpr bool operator<(const iterator_impl &other) const { return Index < other.Index; }
        constexpr bool operator>=(const iterator_impl &other) const { return Index >= other.Index; }
        constexpr bool operator<=(const iterator_impl &other) const { return Index <= other.Index; }

        template <bool Const = !Mutable>
        enable_if_t<!Const, code_point> operator*() {
            return Parent->get(Index);
        }

        template <bool Const = !Mutable>
        constexpr enable_if_t<Const, char32_t> operator*() const {
            return Parent->get(Index);
        }

        constexpr const byte *to_pointer() const { return get_cp_at_index(Parent->Data, Parent->Length, (s64) Index); }
    };

    using iterator = iterator_impl<true>;
    using const_iterator = iterator_impl<false>;

    // This memory is a valid utf-8 string in memory.
    //
    // Const so the user doesn't change it accidentally
    //
    // A byte doesn't guarantee a valid code point since they can be multiple bytes in utf8.
    // You almost never would want to modify or access characters from this member unless you want
    // something very specific.
    const byte *Data = null;

    // Non-zero if Data was allocated by string and needs to be freed
    size_t Reserved = 0;

    // The number of code units (bytes) in the string
    size_t ByteLength = 0;

    // Length of the string in code points
    size_t Length = 0;

    constexpr string() = default;

    // Create a string from a null terminated c-string
    // Note that this constructor doesn't validate if the passed in string is valid utf8
    constexpr string(const byte *str) : string(str, cstring_strlen(str)) {}

    // Create a string from a buffer and a length
    // Note that this constructor doesn't validate if the passed in string is valid utf8
    constexpr string(const byte *str, size_t size) {
        Data = str;
        ByteLength = size;
        Length = utf8_strlen(str, size);
    }

    // Converts a null-terminated wide char string to utf8
    // Allocates a buffer
    explicit string(const wchar_t *str);

    // Converts a null-terminated utf32 string to utf8
    // Allocates a buffer
    explicit string(const char32_t *str);

    // Create a string with an initial size reserved
    // Allocates a buffer
    explicit string(size_t size);

    // Makes sure string has reserved enough space for at least n bytes
    // Allocates a buffer if the string doesn't point to reserved memory.
    // If the string points to reserved memory but doesn't own it, this function asserts.
    void reserve(size_t bytes);

    // Releases the memory allocated by this string.
    // If this string doesn't own the memory it points to, this function does nothing.
    void release();

    // Gets the _index_'th code point in the string
    // The returned code_point object can be used to modify the code point at that location (by assigning)
    code_point get(s64 index) { return code_point(this, translate_index(index, Length)); }

    // Gets the _index_'th code point in the string
    constexpr char32_t get(s64 index) const { return decode_cp(get_cp_at_index(Data, Length, index)); }

    // Sets the _index_'th code point in the string
    void set(s64 index, char32_t codePoint);

    // Insert a code point at a specified index
    void insert(s64 index, char32_t codePoint);

    // Insert a string at a specified index
    void insert(s64 index, const string &str);

    // Insert a buffer of bytes at a specified index
    void insert_pointer_and_size(s64 index, const byte *begin, size_t size);

    // Removes code point at specified index
    void remove(s64 index);

    // Removes a range of code points
    // [begin, end)
    void remove(s64 begin, s64 end);

    // Append a non encoded character to a string
    void append(char32_t codePoint);

    // Append one string to another
    void append(const string &str);

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const byte *data, size_t size);

    // Clone this string and copy its contents _n_ times.
    string repeated(size_t n) const;

    // Clone string and convert it to uppercase characters
    string get_upper() const;

    // Clone string and convert it to lowercase characters
    string get_lower() const;

    // Removes all occurences of _cp_
    void remove_all(char32_t cp);

    // Removes all occurences of _str_
    void remove_all(const string &str);

    // Replaces all occurences of _oldCp_ with _newCp_
    void replace_all(char32_t oldCp, char32_t newCp);

    // Replaces all occurences of _oldStr_ with _newStr_
    void replace_all(const string &oldStr, const string &newStr);

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && get_owner() == this; }

    //
    // Utility member functions that are constexpr:
    //

    constexpr bool begins_with(char32_t cp) const { return get(0) == cp; }
    constexpr bool begins_with(const string &str) const {
        return compare_memory_constexpr(Data, str.Data, str.ByteLength) == npos;
    }

    constexpr bool ends_with(char32_t cp) const { return get(-1) == cp; }
    constexpr bool ends_with(const string &str) const {
        return compare_memory_constexpr(Data + ByteLength - str.ByteLength, str.Data, str.ByteLength) == npos;
    }

    // Compares the string to _str_ lexicographically.
    // The result is less than 0 if this string sorts before the other, 0 if they are equal,
    // and greater than 0 otherwise.
    constexpr s32 compare(const string &str) const {
        // If the memory and the lengths are the same, the strings are equal!
        if (Data == str.Data && ByteLength == str.ByteLength) return 0;

        if (Length == 0 && str.Length == 0) return 0;
        if (Length == 0) return -((s32) str.get(0));
        if (str.Length == 0) return get(0);

        auto s1 = begin(), s2 = str.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == str.end()) return 0;
            if (s1 == end()) return -((s32) str.get(0));
            if (s2 == str.end()) return get(0);
        }
        return ((s32) *s1 - (s32) *s2);
    }

    // Compares the string to _str_ lexicographically while ignoring case.
    // The result is less than 0 if this string sorts before the other, 0 if they are equal,
    // and greater than 0 otherwise.
    constexpr s32 compare_ignore_case(const string &str) const {
        // If the memory and the lengths are the same, the views are equal!
        if (Data == str.Data && ByteLength == str.ByteLength) return 0;
        if (Length == 0 && str.Length == 0) return 0;
        if (Length == 0) return -((s32) to_lower(str.get(0)));
        if (str.Length == 0) return to_lower(get(0));

        auto s1 = begin(), s2 = str.begin();
        while (to_lower(*s1) == to_lower(*s2)) {
            ++s1, ++s2;
            if (s1 == end() && s2 == str.end()) return 0;
            if (s1 == end()) return -((s32) to_lower(str.get(0)));
            if (s2 == str.end()) return to_lower(get(0));
        }
        return ((s32) to_lower(*s1) - (s32) to_lower(*s2));
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
    constexpr size_t find(const string &str, s64 start = 0) const {
        assert(Data);
        assert(str.Data);
        assert(str.Length);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        For(range(start, Length)) {
            auto progress = str.begin();
            for (auto search = begin() + it; progress != str.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == str.end()) return it;
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
    constexpr size_t find_reverse(const string &str, s64 start = 0) const {
        assert(Data);
        assert(str.Data);
        assert(str.Length);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        For(range(start - str.Length + 1, -1, -1)) {
            auto progress = str.begin();
            for (auto search = begin() + it; progress != str.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == str.end()) return it;
        }
        return npos;
    }

    // Find the first occurence of any code point in the specified view that is after a specified index
    constexpr size_t find_any_of(const string &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        auto p = begin() + start;
        For(range(start, Length)) if (cps.has(*p++)) return it;
        return npos;
    }

    // Find the last occurence of any code point in the specified view
    // that is before a specified index (0 means: start from the end)
    constexpr size_t find_reverse_any_of(const string &cps, s64 start = 0) const {
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
    constexpr size_t find_not_any_of(const string &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);

        auto p = begin() + start;
        For(range(start, Length)) if (!cps.has(*p++)) return it;
        return npos;
    }

    // Find the first absence of any code point in the specified view that is after a specified index
    constexpr size_t find_reverse_not_any_of(const string &cps, s64 start = 0) const {
        assert(Data);
        if (Length == 0) return npos;

        start = translate_index(start, Length);
        if (start == 0) start = Length - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (!cps.has(*p--)) return it;
        return npos;
    }

    // Gets [begin, end) range of characters into a new string object.
    // This function doesn't deep copy, use clone() on the result to do that.
    constexpr string substring(s64 begin, s64 end) const {
        // Convert to absolute [begin, end)
        size_t beginIndex = translate_index(begin, Length);
        size_t endIndex = translate_index(end, Length, true);

        const byte *beginPtr = get_cp_at_index(Data, Length, beginIndex);
        const byte *endPtr = beginPtr;
        For(range(beginIndex, endIndex)) endPtr += get_size_of_cp(endPtr);

        string result;
        result.Data = beginPtr;
        result.ByteLength = (uptr_t)(endPtr - beginPtr);
        result.Length = endIndex - beginIndex;
        return result;
    }

    // Returns a substring of _str_ with whitespace removed from both sides.
    constexpr string trim() const { return trim_start().trim_end(); }

    // Returns a substring of with whitespace removed at the start.
    constexpr string trim_start() const { return substring(find_not_any_of(" \n\r\t\v\f"), Length); }

    // Returns a substring of with whitespace removed at the end.
    constexpr string trim_end() const { return substring(0, find_reverse_not_any_of(" \n\r\t\v\f") + 1); }

    // Returns true if the string contains _cp_ anywhere
    constexpr bool has(char32_t cp) const { return find(cp) != npos; }

    // Returns true if the string contains _str_ anywhere
    constexpr bool has(const string &str) const { return find(str) != npos; }

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
    constexpr size_t count(const string &str) const {
        size_t result = 0, index = 0;
        while ((index = find(str, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    //
    // Operators:
    //

    // Read/write [] operator
    code_point operator[](s64 index) { return get(index); }

    // Read-only [] operator
    constexpr char32_t operator[](s64 index) const { return get(index); }

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, Length); }
    constexpr const_iterator begin() const { return const_iterator(this, Length); }
    constexpr const_iterator end() const { return const_iterator(this, Length); }

    // Check two strings for equality
    bool operator==(const string &other) const { return compare(other) == 0; }
    bool operator!=(const string &other) const { return !(*this == other); }
    bool operator<(const string &other) const { return compare(other) < 0; }
    bool operator>(const string &other) const { return compare(other) > 0; }
    bool operator<=(const string &other) const { return !(*this > other); }
    bool operator>=(const string &other) const { return !(*this < other); }

    string operator+(char32_t codePoint) const {
        string result = *this;
        result.append(codePoint);
        return result;
    }

    string operator+(const string &memory) const {
        string result = *this;
        result.append(memory);
        return result;
    }

    string operator+(const byte *str) const { return *this + string(str); }

    string &operator+=(char32_t codePoint) {
        append(codePoint);
        return *this;
    }

    string &operator+=(const string &memory) {
        append(memory);
        return *this;
    }

    string &operator+=(const byte *str) { return *this += string(str); }

    string operator*(size_t n) { return repeated(n); }

    string &operator*=(size_t n) {
        *this = repeated(n);
        return *this;
    }

   private:
    void change_owner(string *newOwner) const;
    string *get_owner() const;

    friend string clone(const string &);
    friend string *move(string *, const string &);
};

string clone(const string &value);
string *move(string *dest, const string &src);

LSTD_END_NAMESPACE