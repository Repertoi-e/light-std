#pragma once

/// Provides a string type

#include "string_utils.h"

#include "../memory/allocator.h"

#include "owner_pointers.h"

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
// This type extends the API of _string_view_ (which is entirely constexpr)
// But constexpr methods in _string_view_ aren't constexpr here. So _string_ is runtime only.
struct string {
    struct code_point {
        string *Parent;
        size_t Index;

        code_point() = default;
        code_point(string *parent, size_t index) : Parent(parent), Index(index) {}

        code_point &operator=(char32_t other);
        operator char32_t() const;
    };

    const char *Data = null;

    // Length in bytes
    size_t ByteLength = 0;

    // Length in code points
    size_t Length = 0;

    // Non-zero if Data was allocated by string and needs to be freed
    size_t Reserved = 0;

    string() = default;

    // Create a string from a null terminated c-string.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    string(const char *str) : Data(str), ByteLength(c_string_strlen(str)) { Length = utf8_strlen(str, ByteLength); }

    // Create a string from a buffer and a length.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    string(const char *str, size_t size) : Data(str), ByteLength(size), Length(utf8_strlen(str, size)) {}

    string(string_view view) : Data(view.Data), ByteLength(view.ByteLength), Length(view.Length) {}

    string(char32_t codePoint, size_t repeat, allocator alloc = {null, null});
    string(wchar_t codePoint, size_t repeat, allocator alloc = {null, null})
        : string((char32_t) codePoint, repeat, alloc) {}

    // Converts a null-terminated wide char string to utf8.
    // Allocates a buffer.
    explicit string(const wchar_t *str);

    // Converts a null-terminated utf32 string to utf8.
    // Allocates a buffer.
    explicit string(const char32_t *str);

    // Create a string with an initial size reserved.
    // Allocates a buffer (using the Context's allocator by default)
    explicit string(size_t size, allocator alloc = {null, null});

    ~string() { release(); }

    // Makes sure string has reserved enough space for at least n bytes.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at 8.
    //
    // Allocates a buffer if the string doesn't already point to reserved memory
    // (using the Context's allocator by default).
    // You can also use this function to change the allocator of a string before using it.
    //    reserve(0, ...) is enough to allocate an 8 byte buffer with the passed in allocator.
    //
    // For robustness, this function asserts if you pass an allocator, but the string has already
    // reserved a buffer with a *different* allocator.
    //
    // If the string points to reserved memory but doesn't own it, this function asserts.
    void reserve(size_t size, allocator alloc = {null, null});

    // Releases the memory allocated by this string.
    // If this string doesn't own the memory it points to, this function does nothing.
    void release();

    // Gets the _index_'th code point in the string.
    // The returned code_point object can be used to modify the code point at that location (by assigning).
    code_point get(s64 index) { return code_point(this, translate_index(index, Length)); }

    char32_t get(s64 index) const { return decode_cp(get_cp_at_index(Data, Length, index)); }

    bool begins_with(char32_t cp) const { return get(0) == cp; }
    bool begins_with(string str) const {
        assert(str.ByteLength < ByteLength);
        return compare_memory(Data, str.Data, str.ByteLength) == npos;
    }

    bool ends_with(char32_t cp) const { return get(-1) == cp; }
    bool ends_with(string str) const {
        assert(str.ByteLength < ByteLength);
        return compare_memory(Data + ByteLength - str.ByteLength, str.Data, str.ByteLength) == npos;
    }

    // Compares two utf8 encoded strings and returns the index of the code point
    // at which they are different or _npos_ if they are the same.
    size_t compare(string str) const { return compare_utf8(Data, Length, str.Data, str.Length); }

    // Compares two utf8 encoded strings ignoring case and returns the index of the code point
    // at which they are different or _npos_ if they are the same.
    size_t compare_ignore_case(string str) const {
        return compare_utf8_ignore_case(Data, Length, str.Data, str.Length);
    }

    // Compares two utf8 encoded strings and returns -1 if _one_ is before _two_,
    // 0 if one == two and 1 if _two_ is before _one_.
    s32 compare_lexicographically(string str) const {
        return compare_utf8_lexicographically(Data, Length, str.Data, str.Length);
    }

    // Compares two utf8 encoded strings ignorign case and returns -1 if _one_ is before _two_,
    // 0 if one == two and 1 if _two_ is before _one_.
    s32 compare_lexicographically_ignore_case(string str) const {
        return compare_utf8_lexicographically_ignore_case(Data, Length, str.Data, str.Length);
    }

    // Find the index of the first occurence of a code point that is after a specified index
    size_t find(char32_t cp, s64 start = 0) const {
        auto *p = find_cp_utf8(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first occurence of a substring that is after a specified index
    size_t find(string str, s64 start = 0) const {
        auto *p = find_substring_utf8(Data, Length, str.Data, str.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last occurence of a code point that is before a specified index
    size_t find_reverse(char32_t cp, s64 start = 0) const {
        auto *p = find_cp_utf8_reverse(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last occurence of a substring that is before a specified index
    size_t find_reverse(string str, s64 start = 0) const {
        auto *p = find_substring_utf8_reverse(Data, Length, str.Data, str.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first occurence of any code point in _terminators_ that is after a specified index
    size_t find_any_of(string terminators, s64 start = 0) const {
        auto *p = find_utf8_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last occurence of any code point in _terminators_
    size_t find_reverse_any_of(string terminators, s64 start = 0) const {
        auto *p = find_utf8_reverse_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first absence of a code point that is after a specified index
    size_t find_not(char32_t cp, s64 start = 0) const {
        auto *p = find_utf8_not(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the last absence of a code point that is before the specified index
    size_t find_reverse_not(char32_t cp, s64 start = 0) const {
        auto *p = find_utf8_reverse_not(Data, Length, cp, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first absence of any code point in _terminators_ that is after a specified index
    size_t find_not_any_of(string terminators, s64 start = 0) const {
        auto *p = find_utf8_not_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Find the index of the first absence of any code point in _terminators_ that is after a specified index
    size_t find_reverse_not_any_of(string terminators, s64 start = 0) const {
        auto *p = find_utf8_reverse_not_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return npos;
        return utf8_strlen(Data, p - Data);
    }

    // Gets [begin, end) range of characters into a new string object
    string substring(s64 begin, s64 end) const {
        auto sub = substring_utf8(Data, Length, begin, end);
        return string(sub.First, sub.Second - sub.First);
    }

    // Returns a substring with whitespace removed at the start
    string trim_start() const { return substring(find_not_any_of(" \n\r\t\v\f"), Length); }

    // Returns a substring with whitespace removed at the end
    string trim_end() const { return substring(0, find_reverse_not_any_of(" \n\r\t\v\f") + 1); }

    // Returns a substring with whitespace removed from both sides
    string trim() const { return trim_start().trim_end(); }

    // Returns true if the string contains _cp_ anywhere
    bool has(char32_t cp) const { return find(cp) != npos; }

    // Returns true if the string contains _str_ anywhere
    bool has(string str) const { return find(str) != npos; }

    // Counts the number of occurences of _cp_
    size_t count(char32_t cp) const {
        size_t result = 0, index = 0;
        while ((index = find(cp, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    // Counts the number of occurences of _str_
    size_t count(string str) const {
        size_t result = 0, index = 0;
        while ((index = find(str, index)) != npos) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    // Sets the _index_'th code point in the string
    string *set(s64 index, char32_t codePoint);

    // Insert a code point at a specified index
    string *insert(s64 index, char32_t codePoint);

    // Insert a string at a specified index
    string *insert(s64 index, string str);

    // Insert a buffer of bytes at a specified index
    string *insert_pointer_and_size(s64 index, const char *str, size_t size);

    // Remove code point at specified index
    string *remove(s64 index);

    // Remove a range of code points.
    // [begin, end)
    string *remove(s64 begin, s64 end);

    // Append a non encoded character to a string
    string *append(char32_t codePoint) { return insert(Length, codePoint); }

    // Append one string to another
    string *append(string str) { return append_pointer_and_size(str.Data, str.ByteLength); }

    // Append _size_ bytes of string contained in _data_
    string *append_pointer_and_size(const char *str, size_t size) { return insert_pointer_and_size(Length, str, size); }

    // Copy this string's contents and append them _n_ times
    string *repeat(size_t n);

    // Convert this string to uppercase code points
    string *to_upper();

    // Convert this string to lowercase code points
    string *to_lower();

    // Removes all occurences of _cp_
    string *remove_all(char32_t cp);

    // Remove all occurences of _str_
    string *remove_all(string str);

    // Replace all occurences of _oldCp_ with _newCp_
    string *replace_all(char32_t oldCp, char32_t newCp);

    // Replace all occurences of _oldStr_ with _newStr_
    string *replace_all(string oldStr, string newStr);

    // Replace all occurences of _oldCp_ with _newStr_
    string *replace_all(char32_t oldCp, string newStr);

    // Replace all occurences of _oldStr_ with _newCp_
    string *replace_all(string oldStr, char32_t newCp);

    // The caller is responsible for freeing
    const char *to_c_string(allocator alloc = {null, null}) {
        char *result = new (alloc) char[ByteLength + 1];
        copy_memory(result, Data, ByteLength);
        result[ByteLength] = '\0';
        return result;
    }

    // Return true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<string>(Data) == this; }

    //
    // Iterator:
    //
   private:
    template <bool Const>
    struct string_iterator {
        using string_t = type_select_t<Const, const string, string>;

        string_t *Parent;
        size_t Index;

        string_iterator() = default;
        string_iterator(string_t *parent, size_t index) : Parent(parent), Index(index) {}

        string_iterator &operator+=(s64 amount) { return Index += amount, *this; }
        string_iterator &operator-=(s64 amount) { return Index -= amount, *this; }
        string_iterator &operator++() { return *this += 1; }
        string_iterator &operator--() { return *this -= 1; }
        string_iterator operator++(s32) {
            string_iterator temp = *this;
            return ++(*this), temp;
        }

        string_iterator operator--(s32) {
            string_iterator temp = *this;
            return --(*this), temp;
        }

        s64 operator-(string_iterator other) const {
            size_t lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        string_iterator operator+(s64 amount) const { return string_iterator(Parent, Index + amount); }
        string_iterator operator-(s64 amount) const { return string_iterator(Parent, Index - amount); }

        friend string_iterator operator+(s64 amount, string_iterator it) { return it + amount; }
        friend string_iterator operator-(s64 amount, string_iterator it) { return it - amount; }

        bool operator==(string_iterator other) const { return Index == other.Index; }
        bool operator!=(string_iterator other) const { return Index != other.Index; }
        bool operator>(string_iterator other) const { return Index > other.Index; }
        bool operator<(string_iterator other) const { return Index < other.Index; }
        bool operator>=(string_iterator other) const { return Index >= other.Index; }
        bool operator<=(string_iterator other) const { return Index <= other.Index; }

        auto operator*() { return Parent->get(Index); }

        operator const char *() const { return get_cp_at_index(Parent->Data, Parent->Length, (s64) Index, true); }
    };

   public:
    using iterator = string_iterator<false>;
    using const_iterator = string_iterator<true>;

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, Length); }

    const_iterator begin() const { return const_iterator(this, 0); }
    const_iterator end() const { return const_iterator(this, Length); }

    //
    // Operators:
    //
    operator string_view() const {
        string_view view;
        view.Data = Data;
        view.ByteLength = ByteLength;
        view.Length = Length;
        return view;
    }
    explicit operator bool() const { return ByteLength; }

    code_point operator[](s64 index) { return get(index); }
    char32_t operator[](s64 index) const { return get(index); }

    bool operator==(string other) const { return compare(other) == npos; }
    bool operator!=(string other) const { return !(*this == other); }
    bool operator<(string other) const { return compare_lexicographically(other) < 0; }
    bool operator>(string other) const { return compare_lexicographically(other) > 0; }
    bool operator<=(string other) const { return !(*this > other); }
    bool operator>=(string other) const { return !(*this < other); }
};

inline bool operator==(const char *one, string other) { return other.compare_lexicographically(one) == 0; }
inline bool operator!=(const char *one, string other) { return !(one == other); }
inline bool operator<(const char *one, string other) { return other.compare_lexicographically(one) > 0; }
inline bool operator>(const char *one, string other) { return other.compare_lexicographically(one) < 0; }
inline bool operator<=(const char *one, string other) { return !(one > other); }
inline bool operator>=(const char *one, string other) { return !(one < other); }

string *clone(string *dest, string src);
string *move(string *dest, string *src);

LSTD_END_NAMESPACE
