#pragma once

#include "../memory/memory.h"

#include <iterator>

GU_BEGIN_NAMESPACE

struct Code_Point_Ref;

template <bool Const>
struct String_Iterator;

// UTF-8 string
// This string doesn't guarantee a null termination at the end.
// It stores a data pointer and a length.
//
// * Usually structures in this library are named Like_This, but this
// is an exception since I consider it as a fundamental data type.
struct string {
    // This is exactly like std::string::npos. It's used to represent
    // an invalid index (like the result of find() for example)
    static constexpr size_t NPOS = (size_t) -1;

    static constexpr size_t SMALL_STRING_BUFFER_SIZE = 8;
    // This implementation uses a small stack allocated buffer
    // for small strings instead of dynamically allocating memory.
    // When the string begins to use more than SMALL_STRING_BUFFER_SIZE
    // bytes of memory, it allocates a buffer on the heap.
    //
    // Note that the "Data" member points to this buffer or the dynamically allocated one.
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
    // Construct a string from a null-terminated c-style string.
    string(const char *str);
    // Construct from a c-style string and a length (in code units, not code points)
    string(const char *str, size_t length);
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
    char32_t get(s64 index) const;

    // Sets the _index_'th code point in the string
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    void set(s64 index, char32_t codePoint);

    // Gets [begin, end) range of characters
    // Allows negative reversed indexing which begins at
    // the end of the string, so -1 is the last character
    // -2 the one before that, etc. (Python-style)
    // Note that the string returned is a view into _str_.
    //    It's not actually an allocated string, so it _str_ gets
    //    destroyed, then the returned string will be pointing to
    //	  invalid memory. Copy the returned string explicitly if
    //	  you intend to use it longer than this string.
    const string substring(s64 begin, s64 end) const;

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

    // Returns 0 if strings are equal, -1 if str is lexicographically < other,
    // and +1 if str is lexicographically > other.
    b32 compare(const string &other) const;

    void swap(string &other);

    using String_Iterator_Mut = String_Iterator<false>;
    using String_Iterator_Const = const String_Iterator<true>;

    String_Iterator_Mut begin();
    String_Iterator_Mut end();
    String_Iterator_Const begin() const;
    String_Iterator_Const end() const;

    // Check two strings for equality
    b32 operator==(const string &other) const { return compare(other) == 0; }
    b32 operator!=(const string &other) const { return !(*this == other); }
    b32 operator<(const string &other) const { return compare(other) == -1; }
    b32 operator>(const string &other) const { return compare(other) == 1; }
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

    b32 begins_with(const string &other) const;
    b32 ends_with(const string &other) const;

    // Returns a substring of _str_ with whitespace
    // removed from both sides.
    const string trim() const;

    // Returns a substring of _str_ with whitespace
    // removed at the start.
    const string trim_start() const;

    // Returns a substring of _str_ with whitespace
    // removed at the end.
    const string trim_end() const;

    string &operator=(const string &other);
    string &operator=(string &&other);

    // Read/write [] operator
    Code_Point_Ref operator[](s64 index);
    // Read-only [] operator
    const char32_t operator[](s64 index) const;

    // Substring operator
    string operator()(s64 begin, s64 end) const;

    // If negative, converts index to a positive number
    // depending on the _Length_ of the string.
    size_t _translate_index(s64 index) const;

    // Returns a pointer to the _index_th code point in the string.
    char *_get_pointer_to_index(s64 index) const;
};

struct Code_Point_Ref {
    string &Parent;
    char32_t CodePoint;
    size_t Index;

    Code_Point_Ref(string &parent, char32_t codePoint, size_t index)
        : Parent(parent), CodePoint(codePoint), Index(index) {}

    Code_Point_Ref &operator=(char32_t other);

    operator char32_t const &() { return CodePoint; }
};

//
// String iterator:
//
template <bool Const>
struct String_Iterator : public std::iterator<std::random_access_iterator_tag, char32_t> {
   private:
    std::conditional_t<Const, const string &, string &> Parent;
    size_t Index;

   public:
    String_Iterator(typename std::conditional_t<Const, const string &, string &> str, s64 index = 0) : Parent(str) {
        if (index < (s64) str.Length && index >= 0) {
            Index = index;
        } else {
            Index = string::NPOS;
        }
    }

    String_Iterator &operator+=(s64 amount) {
        if ((amount < 0 && (s64) Index + amount < 0) || Index + amount >= Parent.Length) {
            Index = string::NPOS;
        } else {
            Index += amount;
        }
        return *this;
    }
    String_Iterator &operator-=(s64 amount) { return *this += -amount; }
    String_Iterator &operator++() { return *this += 1; }
    String_Iterator &operator--() { return *this -= 1; }
    String_Iterator operator++(s32) {
        String_Iterator tmp(*this);
        ++(*this);
        return tmp;
    }
    String_Iterator operator--(s32) {
        String_Iterator tmp(*this);
        --(*this);
        return tmp;
    }

    s64 operator-(const String_Iterator &other) const { return (s64) Index - (s64) other.Index; }
    String_Iterator operator+(s64 amount) const { return String_Iterator(Parent, Index + amount); }
    String_Iterator operator-(s64 amount) const { return String_Iterator(Parent, Index - amount); }
    friend inline String_Iterator operator+(s64 amount, const String_Iterator &it) { return it + amount; }
    friend inline String_Iterator operator-(s64 amount, const String_Iterator &it) { return it - amount; }

    b32 operator==(const String_Iterator &other) const { return Index == other.Index; }
    b32 operator!=(const String_Iterator &other) const { return Index != other.Index; }
    b32 operator>(const String_Iterator &other) const { return Index > other.Index; }
    b32 operator<(const String_Iterator &other) const { return Index < other.Index; }
    b32 operator>=(const String_Iterator &other) const { return Index >= other.Index; }
    b32 operator<=(const String_Iterator &other) const { return Index <= other.Index; }

    template <bool _Const = Const>
    std::enable_if_t<_Const, char32_t> operator*() const {
        assert(Index != string::NPOS);
        return Parent[Index];
    }

    template <bool _Const = Const>
    std::enable_if_t<!_Const, Code_Point_Ref> operator*() const {
        assert(Index != string::NPOS);
        return Parent[Index];
    }

    template <bool _Const = Const>
    std::enable_if_t<_Const, char32_t> operator[](s64 index) const {
        assert(Index != string::NPOS);
        assert(Index + index < Parent.Length);
        return Parent[Index + index];
    }

    template <bool _Const = Const>
    std::enable_if_t<!_Const, Code_Point_Ref> operator[](s64 index) const {
        assert(Index != string::NPOS);
        assert(Index + index < Parent.Length);
        return Parent[Index + index];
    }
};

// These functions only work for ascii
inline b32 is_digit(char32_t x) { return x >= '0' && x <= '9'; }
// These functions only work for ascii
inline b32 is_hexadecimal_digit(char32_t x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f'); }

// These functions only work for ascii
inline b32 is_space(char32_t x) { return (x >= 9 && x <= 13) || x == 32; }
// These functions only work for ascii
inline b32 is_blank(char32_t x) { return x == 9 || x == 32; }

// These functions only work for ascii
inline b32 is_alpha(char32_t x) { return (x >= 65 && x <= 90) || (x >= 97 && x <= 122); }
// These functions only work for ascii
inline b32 is_alphanumeric(char32_t x) { return is_alpha(x) || is_digit(x); }

// These functions only work for ascii
inline b32 is_print(char32_t x) { return x > 31 && x != 127; }


//
// Utility utf-8 functions:
//

// Convert code point to uppercase
char32_t to_upper(char32_t codePoint);

// Convert code point to lowercase
char32_t to_lower(char32_t codePoint);

// Returns the size in bytes of the code point that _str_ points to.
// This function reads the first byte and returns that result.
// If the byte pointed by _str_ is a countinuation utf-8 byte, this
// function returns 0
size_t get_size_of_code_point(const char *str);

// Returns the size that the code point would be if it were encoded.
size_t get_size_of_code_point(char32_t codePoint);

// Encodes code point at _str_, assumes there is enough space.
void encode_code_point(char *str, char32_t codePoint);

// Decodes a code point from a data pointer
char32_t decode_code_point(const char *str);


// This is a constexpr function for working with cstyle strings at compile time
// Retrieve the length of a standard cstyle string (==strlen)
// Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in
// bytes of a null terminated utf-8 string.
constexpr size_t cstyle_strlen(const char *str) {
	size_t length = 0;
	while (*str++) length++;
	return length;
}

// This is a constexpr function for working with cstyle strings at compile time
constexpr const char *find_cstring(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (*n == '\0') return haystack;
        // Didn't match here. Try again further along haystack.
        haystack++;
    }
    return 0;
}

// This is a constexpr function for working with cstyle strings at compile time
constexpr const char *find_cstring(const char *haystack, char needle) {
    char data[2] = {};
    data[1] = needle;
    return find_cstring(haystack, data);
}

// This is a constexpr function for working with cstyle strings at compile time
constexpr const char *find_cstring_last(const char *haystack, const char *needle) {
    if (*needle == '\0') return haystack;

    const char *result = null;
    while (true) {
        const char *candidate = find_cstring(haystack, needle);
        if (!candidate) break;

        result = candidate;
        haystack = candidate + 1;
    }

    return result;
}

// This is a constexpr function for working with cstyle strings at compile time
constexpr const char *find_cstring_last(const char *haystack, char needle) {
    char data[2] = {};
    data[1] = needle;
    return find_cstring_last(haystack, data);
}

GU_END_NAMESPACE