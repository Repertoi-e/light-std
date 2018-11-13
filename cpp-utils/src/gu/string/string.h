#pragma once

#include "string_view.h"

GU_BEGIN_NAMESPACE

// UTF-8 string
// This string doesn't guarantee a null termination at the end.
// It stores a data pointer and a length.
//
// You can think of this structure as an extension to string_view,
// providing:
//	- Own allocated data pointer
//	- Mutability
//	- Dynamic reallocation when out of space
//
// There is also a small optimization implemented for small strings.
// Instead of dynamically allocating a block of memory we use a small
// stack allocated one.
//
// * Usually structures in this library are named Like_This, but this
// is an exception since I consider it as a fundamental data type.
struct string {
    struct Code_Point_Ref {
        string &_Parent;
        char32_t _CodePoint;
        size_t _Index;

        Code_Point_Ref(string &parent, char32_t codePoint, size_t index)
            : _Parent(parent), _CodePoint(codePoint), _Index(index) {}

        Code_Point_Ref &operator=(char32_t other) {
            _Parent.set(_Index, other);
            _CodePoint = other;
            return *this;
        }

        operator char32_t const &() { return _CodePoint; }
    };

    template <bool Const>
    struct Iterator : public std::iterator<std::random_access_iterator_tag, char32_t> {
       private:
        std::conditional_t<Const, const string &, string &> Parent;
        size_t Index;

       public:
        Iterator(typename std::conditional_t<Const, const string &, string &> str, s64 index = 0) : Parent(str) {
            if (index < (s64) str.Length && index >= 0) {
                Index = index;
            } else {
                Index = npos;
            }
        }

        Iterator &operator+=(s64 amount) {
            if ((amount < 0 && (s64) Index + amount < 0) || Index + amount >= Parent.Length) {
                Index = npos;
            } else {
                Index += amount;
            }
            return *this;
        }
        Iterator &operator-=(s64 amount) { return *this += -amount; }
        Iterator &operator++() { return *this += 1; }
        Iterator &operator--() { return *this -= 1; }
        Iterator operator++(s32) {
            Iterator tmp(*this);
            ++(*this);
            return tmp;
        }
        Iterator operator--(s32) {
            Iterator tmp(*this);
            --(*this);
            return tmp;
        }

        s64 operator-(const Iterator &other) const { return (s64) Index - (s64) other.Index; }
        Iterator operator+(s64 amount) const { return Iterator(Parent, Index + amount); }
        Iterator operator-(s64 amount) const { return Iterator(Parent, Index - amount); }
        friend inline Iterator operator+(s64 amount, const Iterator &it) { return it + amount; }
        friend inline Iterator operator-(s64 amount, const Iterator &it) { return it - amount; }

        b32 operator==(const Iterator &other) const { return Index == other.Index; }
        b32 operator!=(const Iterator &other) const { return Index != other.Index; }
        b32 operator>(const Iterator &other) const { return Index > other.Index; }
        b32 operator<(const Iterator &other) const { return Index < other.Index; }
        b32 operator>=(const Iterator &other) const { return Index >= other.Index; }
        b32 operator<=(const Iterator &other) const { return Index <= other.Index; }

        template <bool _Const = Const>
        std::enable_if_t<_Const, char32_t> operator*() const {
            assert(Index != npos);
            return Parent[Index];
        }

        template <bool _Const = Const>
        std::enable_if_t<!_Const, Code_Point_Ref> operator*() const {
            assert(Index != npos);
            return Parent[Index];
        }

        template <bool _Const = Const>
        std::enable_if_t<_Const, char32_t> operator[](s64 index) const {
            assert(Index != npos);
            assert(Index + index < Parent.Length);
            return Parent[Index + index];
        }

        template <bool _Const = Const>
        std::enable_if_t<!_Const, Code_Point_Ref> operator[](s64 index) const {
            assert(Index != npos);
            assert(Index + index < Parent.Length);
            return Parent[Index + index];
        }

		const char *to_pointer() { return string_view(Parent)._get_pointer_to_index((s64) Index); }
    };

    using Iterator_Mut = Iterator<false>;
    using Iterator_Const = const Iterator<true>;

    static constexpr size_t SMALL_STRING_BUFFER_SIZE = 8;
    // This implementation uses a small stack allocated buffer
    // for small strings instead of dynamically allocating memory.
    // When the string begins to use more than SMALL_STRING_BUFFER_SIZE
    // bytes of memory, it allocates a buffer on the heap.
    //
    // Note that the "Data" member points to this buffer *or* the dynamically allocated one.
    char _StackData[SMALL_STRING_BUFFER_SIZE];

    // This member points to a valid utf-8 string in memory.
    // Each 'char' means one byte, which doesn't guarantee a valid utf-8 code point
    // since they can be multiple bytes. You almost never would want
    // to modify or access characters from this member unless you want
    // something very specific.
    char *Data = _StackData;

    // The number of reserved bytes in the string. This is used only
    // if the string is using a dynamically allocated buffer.
    size_t _Reserved = 0;

    // The number of code units in the string, >= the number of code points
    size_t BytesUsed = 0;

    // The number of code points in the string.
    size_t Length = 0;

    // The allocator used for expanding the string.
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

    string() {}
    // Construct from a null-terminated c-style string.
    string(const char *str);
    // Construct from a c-style string and a size (in code units, not code points)
    string(const char *str, size_t size);
    explicit string(const string_view &view);
    string(const string &other);
    string(string &&other);
    ~string();

    // Releases the memory allocated by this string.
    void release();

    // Clears all characters from the string
    // (Sets BytesUsed and Length to 0)
    void clear() { BytesUsed = Length = 0; }

    // Reserve bytes in string
    void reserve(size_t size);

    // Gets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    char32_t get(s64 index) const { return string_view(*this).get(index); }

    // Sets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    void set(s64 index, char32_t codePoint);

    // Gets [begin, end) range of characters
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    // The string returned is a string view.
    const string_view substring(s64 begin, s64 end) const { return string_view(*this).substring(begin, end); }

    // Find the first occurence of _ch_
    size_t find(char32_t ch) const { return string_view(*this).find(ch); }

    // Find the first occurence of _other_
    size_t find(const string_view &other) const { return string_view(*this).find(other); }

    // Find the last occurence of _ch_
    size_t find_last(char32_t ch) const { return string_view(*this).find_last(ch); }

    // Find the last occurence of _other_
    size_t find_last(const string_view &other) const { return string_view(*this).find_last(other); }

	b32 has(char32_t ch) const { return find(ch) != npos; }
	b32 has(const string_view &other) const { return find(other) != npos; }

    // Append one string to another
    void append(const string &other);

    // Append a non encoded character to a string
    void append(char32_t codePoint);

    // Append a null terminated utf-8 c-style string to a string.
    // If the cstyle string is not a valid utf-8 string the
    // modified string is will also not be valid.
    void append_cstring(const char *other);

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const char *data, size_t size);

    // Compares the string to _other_ lexicographically.
    // The result is less than 0 if this string sorts before the other,
    // 0 if they are equal, and greater than 0 otherwise.
    s32 compare(const string &other) const { return string_view(*this).compare((string_view) other); }

    void swap(string &other);

    Iterator_Mut begin();
    Iterator_Mut end();
    Iterator_Const begin() const;
    Iterator_Const end() const;

    // Check two strings for equality
    b32 operator==(const string &other) const { return compare(other) == 0; }
    b32 operator!=(const string &other) const { return !(*this == other); }
    b32 operator<(const string &other) const { return compare(other) < 0; }
    b32 operator>(const string &other) const { return compare(other) > 0; }
    b32 operator<=(const string &other) const { return !(*this > other); }
    b32 operator>=(const string &other) const { return !(*this < other); }

    string operator+(const string &other) {
        string result = *this;
        result.append(other);
        return result;
    }

    string operator+(const char *other) {
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

    string &operator+=(const char *other) {
        append_cstring(other);
        return *this;
    }

    string &operator+=(char32_t codePoint) {
        append(codePoint);
        return *this;
    }

    string operator*(size_t n) {
        string result = *this;
        result.repeat(n);
        return result;
    }

    string &operator*=(size_t n) {
        repeat(n);
        return *this;
    }

    //
    // String utility functions:
    //

    // Copy this string and repeat it _n_ times.
    // repeat(1) does nothing to the string.
    void repeat(size_t n);

    // Copy string and convert it to uppercase characters
    string get_upper() const;

    // Copy string and convert it to lowercase characters
    string get_lower() const;

    // Returns a substring of _str_ with whitespace
    // removed from both sides.
    string_view trim() const { return trim_start().trim_end(); }

    // Returns a substring of _str_ with whitespace
    // removed at the start.
    string_view trim_start() const { return string_view(*this).trim_start(); }

    // Returns a substring of _str_ with whitespace
    // removed at the end.
    string_view trim_end() const { return string_view(*this).trim_end(); }

    b32 begins_with(char32_t ch) const { return string_view(*this).begins_with(ch); }
    b32 begins_with(const string_view &other) const { return string_view(*this).begins_with(other); }

    b32 ends_with(char32_t ch) const { return string_view(*this).ends_with(ch); }
    b32 ends_with(const string_view &other) { return string_view(*this).ends_with(other); }

    string &operator=(const string_view &view);
    string &operator=(const string &other);
    string &operator=(string &&other);

    explicit operator string_view() const { return string_view(*this); }

    // Read/write [] operator
    Code_Point_Ref operator[](s64 index);
    // Read-only [] operator
    const char32_t operator[](s64 index) const;

    // Substring operator
    string_view operator()(s64 begin, s64 end) const;
};

GU_END_NAMESPACE