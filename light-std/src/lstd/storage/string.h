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
// This type extends the API of string_view (which is entirely constexpr)
// But constexpr methods in string_view aren't constexpr here. So "string" is runtime only.
struct string {
    struct code_point {
        string *Parent;
        size_t Index;

        code_point() = default;
        code_point(string *parent, size_t index) : Parent(parent), Index(index) {}

        code_point &operator=(char32_t other);
        operator char32_t() const;
    };

    // Non-zero if Data was allocated by string and needs to be freed
    size_t Reserved = 0;

    // This implements API shared with string_view
    COMMON_STRING_API_IMPL(string, inline)

    string() = default;

    // Create a string from a null terminated c-string.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    string(const byte *str) : Data(str), ByteLength(cstring_strlen(str)) { Length = utf8_strlen(str, ByteLength); }

    // Create a string from a buffer and a length.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    string(const byte *str, size_t size) : Data(str), ByteLength(size), Length(utf8_strlen(str, size)) {}

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

    // Sets the _index_'th code point in the string
    void set(s64 index, char32_t codePoint);

    // Insert a code point at a specified index
    void insert(s64 index, char32_t codePoint);

    // Insert a string at a specified index
    void insert(s64 index, string str);

    // Insert a buffer of bytes at a specified index
    void insert_pointer_and_size(s64 index, const byte *str, size_t size);

    // Remove code point at specified index
    void remove(s64 index);

    // Remove a range of code points.
    // [begin, end)
    void remove(s64 begin, s64 end);

    // Append a non encoded character to a string
    void append(char32_t codePoint) { insert(Length, codePoint); }

    // Append one string to another
    void append(string str) { append_pointer_and_size(str.Data, str.ByteLength); }

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const byte *str, size_t size) { insert_pointer_and_size(Length, str, size); }

    // Clone this string and copy its contents _n_ times
    string repeated(size_t n) const;

    // Clone string and convert it to uppercase characters
    string get_upper() const;

    // Clone string and convert it to lowercase characters
    string get_lower() const;

    // Removes all occurences of _cp_
    void remove_all(char32_t cp);

    // Remove all occurences of _str_
    void remove_all(string str);

    // Replace all occurences of _oldCp_ with _newCp_
    void replace_all(char32_t oldCp, char32_t newCp);

    // Replace all occurences of _oldStr_ with _newStr_
    void replace_all(string oldStr, string newStr);

    // Return true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<string>(Data) == this; }

    //
    // Iterator:
    //
    struct iterator {
        string *Parent;
        size_t Index;

        iterator() = default;
        iterator(string *parent, size_t index) : Parent(parent), Index(index) {}

        iterator &operator+=(s64 amount) { return Index += amount, *this; }
        iterator &operator-=(s64 amount) { return Index -= amount, *this; }
        iterator &operator++() { return *this += 1; }
        iterator &operator--() { return *this -= 1; }
        iterator operator++(s32) {
            iterator temp = *this;
            return ++(*this), temp;
        }

        iterator operator--(s32) {
            iterator temp = *this;
            return --(*this), temp;
        }

        s64 operator-(const iterator &other) const {
            size_t lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        iterator operator+(s64 amount) const { return iterator(Parent, Index + amount); }
        iterator operator-(s64 amount) const { return iterator(Parent, Index - amount); }

        friend iterator operator+(s64 amount, const iterator &it) { return it + amount; }
        friend iterator operator-(s64 amount, const iterator &it) { return it - amount; }

        bool operator==(const iterator &other) const { return Index == other.Index; }
        bool operator!=(const iterator &other) const { return Index != other.Index; }
        bool operator>(const iterator &other) const { return Index > other.Index; }
        bool operator<(const iterator &other) const { return Index < other.Index; }
        bool operator>=(const iterator &other) const { return Index >= other.Index; }
        bool operator<=(const iterator &other) const { return Index <= other.Index; }

        code_point operator*() { return Parent->get(Index); }
        const byte *to_pointer() const { return get_cp_at_index(Parent->Data, Parent->Length, (s64) Index); }
    };

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, Length); }

    //
    // Operators:
    //
    operator string_view() {
        string_view view;
        view.Data = Data;
        view.ByteLength = ByteLength;
        view.Length = Length;
        return view;
    }
    explicit operator bool() const { return ByteLength; }

    code_point operator[](s64 index) { return get(index); }

    string operator+(char32_t codePoint) const {
        string result = *this;
        result.append(codePoint);
        return result;
    }

    string operator+(string memory) const {
        string result = *this;
        result.append(memory);
        return result;
    }

    string operator+(const byte *str) const { return *this + string(str); }

    string &operator+=(char32_t codePoint) {
        append(codePoint);
        return *this;
    }

    string &operator+=(string memory) {
        append(memory);
        return *this;
    }

    string &operator+=(const byte *str) { return *this += string(str); }

    string operator*(size_t n) { return repeated(n); }

    string &operator*=(size_t n) {
        *this = repeated(n);
        return *this;
    }
};

inline bool operator==(const byte *one, string other) { return other.compare_lexicographically(one) == 0; }
inline bool operator!=(const byte *one, string other) { return !(one == other); }
inline bool operator<(const byte *one, string other) { return other.compare_lexicographically(one) > 0; }
inline bool operator>(const byte *one, string other) { return other.compare_lexicographically(one) < 0; }
inline bool operator<=(const byte *one, string other) { return !(one > other); }
inline bool operator>=(const byte *one, string other) { return !(one < other); }

inline string operator+(const byte *one, string other) { return string(one) + other; }
inline string operator*(size_t n, string str) { return str.repeated(n); }

string *clone(string *dest, string src);
string *move(string *dest, string *src);

LSTD_END_NAMESPACE

// :ExplicitDeclareIsPod
// The only reason "string" isn't classified as POD is because of the "non-trivial" destructor.
// Note that without the destructor, is_pod_v<string> is true.
// Here we manually declare an overload for is_pod_v (with the helper macro DECLARE_IS_POD from types.h)...
// I consider this a fine exception for objects that somehow need to manage memory.
DECLARE_IS_POD(string, true)