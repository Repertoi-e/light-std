#pragma once

#include "../memory/memory.hpp"

#include "string_view.hpp"

LSTD_BEGIN_NAMESPACE

// UTF-8 string
// This string doesn't guarantee a null termination at the end.
// It stores a data pointer and a length.
//
// You can think of this structure as an extension to string_view,
// providing:
//    - Own allocated data pointer
//    - Mutability
//    - Dynamic reallocation when out of space
//
// There is also a small optimization implemented for small strings.
// Instead of dynamically allocating a block of memory we use a small
// stack allocated one.
//
// * Usually structures in this library are named Like_This, but this
// is an exception since I consider it a fundamental data type.
struct string {
   private:
    struct Code_Point {
        string &Parent;
        size_t Index;

        Code_Point(string &parent, size_t index) : Parent(parent), Index(index) {}

        Code_Point &operator=(char32_t other);
        operator char32_t() const;
    };

    template <bool Mutable>
    struct Iterator {
       private:
        using parent_type = typename std::conditional_t<Mutable, string, const string>;
        parent_type &Parent;
        size_t Index;

       public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = typename std::conditional_t<Mutable, Code_Point, char32_t>;
        using difference_type = ptr_t;
        using pointer = value_type *;
        using reference = value_type &;

        Iterator(parent_type &parent, size_t index) : Parent(parent), Index(index) {}

        Iterator &operator+=(s64 amount) {
            Index += amount;
            return *this;
        }
        Iterator &operator-=(s64 amount) {
            Index -= amount;
            return *this;
        }
        Iterator &operator++() { return *this += 1; }
        Iterator &operator--() { return *this -= 1; }
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

        s64 operator-(const Iterator &other) const {
            size_t lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        Iterator operator+(s64 amount) const { return Iterator(Parent, Index + amount); }
        Iterator operator-(s64 amount) const { return Iterator(Parent, Index - amount); }

        friend inline Iterator operator+(s64 amount, const Iterator &it) { return it + amount; }
        friend inline Iterator operator-(s64 amount, const Iterator &it) { return it - amount; }

        bool operator==(const Iterator &other) const { return Index == other.Index; }
        bool operator!=(const Iterator &other) const { return Index != other.Index; }
        bool operator>(const Iterator &other) const { return Index > other.Index; }
        bool operator<(const Iterator &other) const { return Index < other.Index; }
        bool operator>=(const Iterator &other) const { return Index >= other.Index; }
        bool operator<=(const Iterator &other) const { return Index <= other.Index; }

        template <bool Const = !Mutable>
        std::enable_if_t<!Const, Code_Point> operator*() {
            return Parent.get(Index);
        }

        template <bool Const = !Mutable>
        std::enable_if_t<Const, char32_t> operator*() const {
            return Parent.get(Index);
        }

        const byte *to_pointer() const { return get_pointer_to_code_point_at(Parent.Data, Parent.Length, (s64) Index); }
    };

   public:
    using code_point = Code_Point;

    using iterator = Iterator<true>;
    using const_iterator = Iterator<false>;

    static constexpr size_t SMALL_STRING_BUFFER_SIZE = 8;
    // This implementation uses a small stack allocated buffer
    // for small strings instead of dynamically allocating memory.
    // When the string begins to use more than SMALL_STRING_BUFFER_SIZE
    // bytes of memory, it allocates a buffer on the heap.
    //
    // Note that the "Data" member points to this buffer *or* the dynamically allocated one.
    byte StackData[SMALL_STRING_BUFFER_SIZE];

    // This member points to a valid utf-8 string in memory.
    // Each 'char' means one byte, which doesn't guarantee a valid utf-8 code point
    // since they can be multiple bytes. You almost never would want
    // to modify or access characters from this member unless you want
    // something very specific.
    byte *Data = StackData;

    // The number of reserved bytes in the string. This is used only
    // if the string is using a dynamically allocated buffer.
    size_t Reserved = 0;

    // The number of code units in the string, >= the number of code points
    size_t ByteLength = 0;

    // Length of the string in code points
    size_t Length = 0;

    // The allocator used for expanding the string.
    // This value is null until this object allocates memory or the user sets it manually.
    mutable Allocator_Closure Allocator;

    string() {}
    // Construct from a null-terminated c-style string.
    string(const byte *str);
    // Construct from a c-style string and a size (in code units, not code points)
    string(const byte *str, size_t size);
    // Construct from a null-terminated c-style string.
    string(const char *str) : string((const byte *) str) {}
    // Construct from a c-style string and a size (in code units, not code points)
    string(const char *str, size_t size) : string((const byte *) str, size) {}
    explicit string(const string_view &view);
    string(const string &other);
    string(string &&other);
    ~string();

    // Releases the memory allocated by this string.
    void release();

    // Clears all characters from the string
    // (Sets ByteLength and Length to 0)
    void clear() { ByteLength = Length = 0; }

    // Reserve bytes in string
    void reserve(size_t size);

    // Gets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    code_point get(s64 index) { return code_point(*this, translate_index(index, Length)); }
    char32_t get(s64 index) const { return get_view().get(index); }

    // Sets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    void set(s64 index, char32_t codePoint);

    // Adds a codepoint at specified index, such that `this[index] == codePoint`
    void add(s64 index, char32_t codePoint);

    // Removes codepoint at specified index
    void remove(s64 index);

    // Gets [begin, end) range of characters
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    // The string returned is a string view.
    // Note that the string returned is a view into _str_.
    //    It's not actually an allocated string, so it _str_ gets
    //    destroyed, then the returned string will be pointing to
    //      invalid memory. Copy the returned string explicitly if
    //      you intend to use it longer than this string.
    const string_view substring(s64 begin, s64 end) const { return get_view().substring(begin, end); }

    // Find the first occurence of _ch_ starting at specified index
    size_t find(char32_t ch, s64 start = 0) const { return get_view().find(ch, start); }

    // Find the first occurence of _other_ starting at specified index
    size_t find(const string_view &view, s64 start = 0) const { return get_view().find(view, start); }

    // Find the last occurence of _ch_ starting at specified index
    size_t find_last(char32_t ch, s64 start = 0) const { return get_view().find_last(ch, start); }

    // Find the last occurence of _other_ starting at specified index
    size_t find_last(const string_view &view, s64 start = 0) const { return get_view().find_last(view, start); }

    bool has(char32_t ch) const { return find(ch) != npos; }
    bool has(const string_view &view) const { return find(view) != npos; }

    size_t count(char32_t cp) { return get_view().count(cp); }
    size_t count(const string_view &view) { return get_view().count(view); }

    // Append one string to another
    void append(const string &other);

    // Append a non encoded character to a string
    void append(char32_t codePoint);

    // Append a null terminated utf-8 c-style string to a string.
    // If the cstyle string is not a valid utf-8 string the
    // modified string is will also not be valid.
    void append_cstring(const byte *other);
    void append_cstring(const char *other) { append_cstring((const byte *) other); }

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const byte *data, size_t size);
    void append_pointer_and_size(const char *data, size_t size) { append_pointer_and_size((const byte *) data, size); }

    // Compares the string to _other_ lexicographically.
    // The result is less than 0 if this string sorts before the other,
    // 0 if they are equal, and greater than 0 otherwise.
    s32 compare(const string &other) const { return get_view().compare(other.get_view()); }
    s32 compare(const string_view &other) const { return get_view().compare(other); }

    s32 compare_ignore_case(const string &other) const { return get_view().compare_ignore_case(other.get_view()); }
    s32 compare_ignore_case(const string_view &other) const { return get_view().compare_ignore_case(other); }

    string_view get_view() const {
        string_view view;
        view.Data = Data;
        view.ByteLength = ByteLength;
        view.Length = Length;
        return view;
    }

    void swap(string &other);

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

    // Check two strings for equality
    bool operator==(const string &other) const { return compare(other) == 0; }
    bool operator!=(const string &other) const { return !(*this == other); }
    bool operator<(const string &other) const { return compare(other) < 0; }
    bool operator>(const string &other) const { return compare(other) > 0; }
    bool operator<=(const string &other) const { return !(*this > other); }
    bool operator>=(const string &other) const { return !(*this < other); }

    string operator+(const string &other) {
        string result = *this;
        result.append(other);
        return result;
    }

    string operator+(const byte *other) {
        string result = *this;
        result.append_cstring(other);
        return result;
    }

    string operator+(char32_t codePoint) {
        string result = *this;
        result.append(codePoint);
        return result;
    }

    string &operator+=(const string &other) {
        append(other);
        return *this;
    }

    string &operator+=(const byte *other) {
        append_cstring(other);
        return *this;
    }

    string &operator+=(char32_t codePoint) {
        append(codePoint);
        return *this;
    }

    string operator*(size_t n) { return repeated(n); }

    string &operator*=(size_t n) {
        *this = repeated(n);
        return *this;
    }

    //
    // String utility functions:
    //

    // Copy this string and repeat it _n_ times.
    // repeated(1) returns the string.
    string repeated(size_t n);

    // Copy string and convert it to uppercase characters
    string get_upper() const;

    // Copy string and convert it to lowercase characters
    string get_lower() const;

    // Returns a substring of _str_ with whitespace
    // removed from both sides.
    string_view trim() const { return trim_start().trim_end(); }

    // Returns a substring of _str_ with whitespace
    // removed at the start.
    string_view trim_start() const { return get_view().trim_start(); }

    // Returns a substring of _str_ with whitespace
    // removed at the end.
    string_view trim_end() const { return get_view().trim_end(); }

    // Converts a utf8 string to a null-terminated wide char string (for use with Windows)
    // The string returned is allocated by this object's allcoator and must be freed by the caller
    wchar_t *to_utf16() const;

    // Converts a null-terminated wide char string to utf-8 and stores it in this object
    void from_utf16(const wchar_t *str);
   
    // Converts a utf8 string to a null-terminated wide char string (for use with Windows)
    // The string returned is allocated by this object's allcoator and must be freed by the caller
    char32_t *to_utf32() const;

    // Converts a null-terminated wide char string to utf-8 and stores it in this object
    void from_utf32(const char32_t *str);

    bool begins_with(char32_t ch) const { return get_view().begins_with(ch); }
    bool begins_with(const string_view &other) const { return get_view().begins_with(other); }

    bool ends_with(char32_t ch) const { return get_view().ends_with(ch); }
    bool ends_with(const string_view &other) { return get_view().ends_with(other); }

    string &operator=(const string &other);
    string &operator=(string &&other);

    operator string_view() const { return get_view(); }

    // Read/write [] operator
    code_point operator[](s64 index);
    // Read-only [] operator
    char32_t operator[](s64 index) const;

    operator bool() const { return Length != 0; }

    // Substring operator
    string_view operator()(s64 begin, s64 end) const;
};

constexpr size_t a = sizeof(string);

inline string operator+(const byte *one, const string &other) { return string(one) + other; }
inline string operator+(const char *one, const string &other) { return string(one) + other; }

inline bool operator==(const byte *one, const string &other) { return other.compare(string_view(one)) == 0; }
inline bool operator!=(const byte *one, const string &other) { return !(one == other); }
inline bool operator<(const byte *one, const string &other) { return other.compare(string_view(one)) > 0; }
inline bool operator>(const byte *one, const string &other) { return other.compare(string_view(one)) < 0; }
inline bool operator<=(const byte *one, const string &other) { return !(one > other); }
inline bool operator>=(const byte *one, const string &other) { return !(one < other); }

inline bool operator==(const char *one, const string &other) { return other.compare(string_view(one)) == 0; }
inline bool operator!=(const char *one, const string &other) { return !(one == other); }
inline bool operator<(const char *one, const string &other) { return other.compare(string_view(one)) > 0; }
inline bool operator>(const char *one, const string &other) { return other.compare(string_view(one)) < 0; }
inline bool operator<=(const char *one, const string &other) { return !(one > other); }
inline bool operator>=(const char *one, const string &other) { return !(one < other); }

LSTD_END_NAMESPACE