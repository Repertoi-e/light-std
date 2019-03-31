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
    struct code_point {
        string &Parent;
        size_t Index;

        code_point(string &parent, size_t index) : Parent(parent), Index(index) {}

        code_point &operator=(char32_t other);
        operator char32_t() const;
    };

    template <bool Mutable>
    struct iterator_impl {
       private:
        using parent_type = std::conditional_t<Mutable, string, const string>;
        parent_type &Parent;
        size_t Index;

       public:
        iterator_impl(parent_type &parent, size_t index) : Parent(parent), Index(index) {}

        iterator_impl &operator+=(s64 amount) {
            Index += amount;
            return *this;
        }
        iterator_impl &operator-=(s64 amount) {
            Index -= amount;
            return *this;
        }
        iterator_impl &operator++() { return *this += 1; }
        iterator_impl &operator--() { return *this -= 1; }
        constexpr iterator_impl operator++(s32) {
            iterator_impl temp = *this;
            ++(*this);
            return temp;
        }

        constexpr iterator_impl operator--(s32) {
            iterator_impl temp = *this;
            --(*this);
            return temp;
        }

        s64 operator-(const iterator_impl &other) const {
            size_t lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        iterator_impl operator+(s64 amount) const { return iterator_impl(Parent, Index + amount); }
        iterator_impl operator-(s64 amount) const { return iterator_impl(Parent, Index - amount); }

        friend iterator_impl operator+(s64 amount, const iterator_impl &it) { return it + amount; }
        friend iterator_impl operator-(s64 amount, const iterator_impl &it) { return it - amount; }

        bool operator==(const iterator_impl &other) const { return Index == other.Index; }
        bool operator!=(const iterator_impl &other) const { return Index != other.Index; }
        bool operator>(const iterator_impl &other) const { return Index > other.Index; }
        bool operator<(const iterator_impl &other) const { return Index < other.Index; }
        bool operator>=(const iterator_impl &other) const { return Index >= other.Index; }
        bool operator<=(const iterator_impl &other) const { return Index <= other.Index; }

        template <bool Const = !Mutable>
        std::enable_if_t<!Const, code_point> operator*() {
            return Parent.get(Index);
        }

        template <bool Const = !Mutable>
        std::enable_if_t<Const, char32_t> operator*() const {
            return Parent.get(Index);
        }

        const byte *to_pointer() const { return get_pointer_to_code_point_at(Parent.Data, Parent.Length, (s64) Index); }
    };

   public:
    using code_point = code_point;

    using iterator = iterator_impl<true>;
    using const_iterator = iterator_impl<false>;

    static constexpr size_t SMALL_STRING_BUFFER_SIZE = 8;
    // This implementation uses a small stack allocated buffer
    // for small strings instead of dynamically allocating memory.
    // When the string begins to use more than SMALL_STRING_BUFFER_SIZE
    // bytes of memory, it allocates a buffer on the heap.
    //
    // Note that the "Data" member points to this buffer *or* the dynamically allocated one.
    byte StackData[SMALL_STRING_BUFFER_SIZE] = {0};

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
    mutable allocator_closure Allocator;

    string() {}
    // Create a string with an initial size reserved
    string(size_t size);
    string(const memory_view &memory);
    string(const byte *str) : string(memory_view(str)) {}
    string(const byte *str, size_t size) : string(memory_view(str, size)) {}

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

    // Insert a code point at a specified index
    void insert(s64 index, char32_t codePoint);

    // Insert a string at a specified index
    void insert(s64 index, const memory_view &memory);

    // Insert [begin, end) bytes at a specified index
    void insert_pointer_and_size(s64 index, const byte *begin, size_t size);

    // Removes code point at specified index
    void remove(s64 index);

    // Removes a range of code points
    // [begin, end)
    void remove(s64 begin, s64 end);

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
    string_view substring(s64 begin, s64 end) const { return get_view().substring(begin, end); }

    // Find the first occurence of a code point that is after a specified index
    size_t find(char32_t cp, s64 start = 0) const { return get_view().find(cp, start); }

    // Find the first occurence of a substring that is after a specified index
    size_t find(const string_view &other, s64 start = 0) const { return get_view().find(other, start); }

    // Find the last occurence of a code point that is before a specified index
    size_t find_reverse(char32_t cp, s64 start = 0) const { return get_view().find_reverse(cp, start); }

    // Find the last occurence of a substring that is before a specified index
    size_t find_reverse(const string_view &other, s64 start = 0) const { return get_view().find_reverse(other, start); }

    // Find the first occurence of any code point in the specified view that is after a specified index
    size_t find_any_of(const string_view &cps, s64 start = 0) const { return get_view().find_any_of(cps, start); }

    // Find the last occurence of any code point in the specified view
    // that is before a specified index (0 means: start from the end)
    size_t find_reverse_any_of(const string_view &cps, s64 start = 0) const {
        return get_view().find_reverse_any_of(cps, start);
    }

    // Find the first absence of a code point that is after a specified index
    size_t find_not(char32_t cp, s64 start = 0) const { return get_view().find_not(cp, start); }

    // Find the last absence of a code point that is before the specified index
    size_t find_reverse_not(char32_t cp, s64 start = 0) const { return get_view().find_reverse_not(cp, start); }

    // Find the first absence of any code point in the specified view that is after a specified index
    size_t find_not_any_of(const string_view &cps, s64 start = 0) const {
        return get_view().find_not_any_of(cps, start);
    }

    // Find the first absence of any code point in the specified view that is after a specified index
    size_t find_reverse_not_any_of(const string_view &cps, s64 start = 0) const {
        return get_view().find_reverse_not_any_of(cps, start);
    }

    bool has(char32_t ch) const { return find(ch) != npos; }
    bool has(const string_view &view) const { return find(view) != npos; }

    size_t count(char32_t cp) const { return get_view().count(cp); }
    size_t count(const string_view &view) const { return get_view().count(view); }

    // Append a non encoded character to a string
    void append(char32_t codePoint);

    // Append one string to another
    void append(const memory_view &memory);

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const byte *data, size_t size);

    // Compares the string to _other_ lexicographically.
    // The result is less than 0 if this string sorts before the other,
    // 0 if they are equal, and greater than 0 otherwise.
    s32 compare(const string_view &other) const { return get_view().compare(other); }
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
    bool operator==(const string_view &other) const { return compare(other) == 0; }
    bool operator!=(const string_view &other) const { return !(*this == other); }
    bool operator<(const string_view &other) const { return compare(other) < 0; }
    bool operator>(const string_view &other) const { return compare(other) > 0; }
    bool operator<=(const string_view &other) const { return !(*this > other); }
    bool operator>=(const string_view &other) const { return !(*this < other); }

    string operator+(char32_t codePoint) const {
        string result = *this;
        result.append(codePoint);
        return result;
    }

    string operator+(const memory_view &memory) const {
        string result = *this;
        result.append(memory);
        return result;
    }

    string operator+(const byte *str) const { return *this + memory_view(str); }

    string &operator+=(char32_t codePoint) {
        append(codePoint);
        return *this;
    }

    string &operator+=(const memory_view &memory) {
        append(memory);
        return *this;
    }

    string &operator+=(const byte *str) { return *this += memory_view(str); }

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

    string removed_all(char32_t ch) const;
    string removed_all(const string_view &view) const;
    string replaced_all(char32_t oldCh, char32_t newCh) const;
    string replaced_all(const string_view &oldView, const string_view &newView) const;

    // Converts a utf8 string to a null-terminated wide char string (for use with Windows)
    // The string returned is allocated by this object's allcoator and must be freed by the caller
    wchar_t *to_utf16() const;
    void to_utf16(wchar_t *out) const { return get_view().to_utf16(out); }

    // Converts a null-terminated wide char string to utf-8 and stores it to this string
    void from_utf16(const wchar_t *str, bool overwrite = true);

    // Converts a utf8 string to a null-terminated wide char string (for use with Windows)
    // The string returned is allocated by this object's allcoator and must be freed by the caller
    char32_t *to_utf32() const;
    void to_utf32(char32_t *out) const { return get_view().to_utf32(out); }

    // Converts a null-terminated wide char string to utf-8 and appends it to this string
    void from_utf32(const char32_t *str, bool overwrite = true);

    bool begins_with(char32_t ch) const { return get_view().begins_with(ch); }
    bool begins_with(const memory_view &other) const { return get_view().begins_with(other); }

    bool ends_with(char32_t ch) const { return get_view().ends_with(ch); }
    bool ends_with(const memory_view &other) const { return get_view().ends_with(other); }

    string &operator=(const memory_view &memory);
    string &operator=(const string &other);
    string &operator=(string &&other);

    operator string_view() const { return get_view(); }
    operator memory_view() const {
        memory_view view;
        view.Data = Data;
        view.ByteLength = ByteLength;
        return view;
    }

    // Read/write [] operator
    code_point operator[](s64 index);
    // Read-only [] operator
    char32_t operator[](s64 index) const;

    operator bool() const { return Length != 0; }

    // Substring operator
    string_view operator()(s64 begin, s64 end) const;
};

inline string operator+(const byte *one, const string &other) { return string(one) + other; }

inline bool operator==(const byte *one, const string &other) { return string_view(one) == other; }
inline bool operator!=(const byte *one, const string &other) { return string_view(one) == other; }
inline bool operator>(const byte *one, const string &other) { return string_view(one) > other; }
inline bool operator>=(const byte *one, const string &other) { return string_view(one) >= other; }
inline bool operator<(const byte *one, const string &other) { return string_view(one) < other; }
inline bool operator<=(const byte *one, const string &other) { return string_view(one) <= other; }

LSTD_END_NAMESPACE