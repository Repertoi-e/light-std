#pragma once

#include "../memory/memory.h"

#include "utf8.h"

GU_BEGIN_NAMESPACE

struct Code_Point_Ref;

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
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    string() {}
    // Construct a string from a null-terminated c-style string.
    string(const char *str);
    // Construct from a c-style string and a length (in code units, not code points)
    string(const char *str, size_t length);
    string(string const &other);
    string(string &&other);
    ~string();

    string &operator=(string const &other);
    string &operator=(string &&other);

    // Read-only [] operator
    Code_Point_Ref operator[](s64 index);
    const char32_t operator[](s64 index) const;

	string operator()(s64 begin, s64 end) const;
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

// Releases the memory allocated by this string.
void release(string &str);

// Clears all characters from the string
// (Sets BytesUsed and Length to 0)
inline void clear(string &str) {
    str.BytesUsed = 0;
    str.Length = 0;
}

// Reserve bytes in string
void reserve(string &str, size_t size);

// Gets the _index_'th code point in the string
// Allows negative reversed indexing which begins at
// the end of the string, so -1 is the last character
// -2 the one before that, etc. (Python-style)
char32_t get(string const &str, s64 index);

// Sets the _index_'th code point in the string
// Allows negative reversed indexing which begins at
// the end of the string, so -1 is the last character
// -2 the one before that, etc. (Python-style)
void set(string &str, s64 index, char32_t codePoint);

// Gets [begin, end) range of characters
// Allows negative reversed indexing which begins at
// the end of the string, so -1 is the last character
// -2 the one before that, etc. (Python-style)
// Note that the string returned is a view into _str_.
//    It's not actually an allocated string, so it _str_ gets
//    destroyed, then the returned string will be pointing to 
//	  invalid memory. Copy the returned string to bypass that.
string substring(string const &str, s64 begin, s64 end);

// Returns a pointer to the _index_th code point in the string.
char *get_pointer_to_index(string const &str, s64 index);

// Returns 0 if strings are equal, -1 if str is lexicographically < other,
// and +1 if str is lexicographically > other.
b32 compare(string const &str, string const &other);

// Check two strings for equality
inline b32 operator==(string const &str, string const &other) { return compare(str, other) == 0; }
inline b32 operator!=(string const &str, string const &other) { return !(str == other); }
inline b32 operator<(string const &str, string const &other) { return compare(str, other) == -1; }
inline b32 operator>(string const &str, string const &other) { return compare(str, other) == 1; }
inline b32 operator<=(string const &str, string const &other) { return !(str > other); }
inline b32 operator>=(string const &str, string const &other) { return !(str < other); }

// Append one string to another
void append(string &str, string const &other);

// Append a non encoded character to a string
void append(string &str, char32_t codePoint);

// Append a null terminated utf-8 c-style string to a string.
// If the cstyle string is not a valid utf-8 string the
// modified string is will also not be valid.
void append_cstring(string &str, const char *other);

// Append _size_ bytes of string contained in _data_
void append_pointer_and_size(string &str, const char *data, size_t size);

inline string operator+(string str, string const &other) {
    append(str, other);
    return str;
}

inline string operator+(string str, const char *other) {
    append_cstring(str, other);
    return str;
}

inline string operator+(string str, char32_t codePoint) {
    append(str, codePoint);
    return str;
}

inline string &operator+=(string &str, string const &other) {
    append(str, other);
    return str;
}

inline string &operator+=(string &str, const char *other) {
    append_cstring(str, other);
    return str;
}

inline string &operator+=(string &str, char32_t codePoint) {
    append(str, codePoint);
    return str;
}

//
// String utility functions:
//

// Copy string and convert it in uppercase characters
string to_upper(string str);

// Copy string and convert it to lowercase characters
string to_lower(string str);

b32 begins_with(string const &str, string const &other);
b32 ends_with(string const &str, string const &other);


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

// Retrieve the length of a standard cstyle string.
// Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in
// bytes of a null terminated utf-8 string.
size_t cstyle_strlen(const char *str);

//
// Utility utf-8 functions:
//

// Convert code point to uppercase
char32_t to_upper(char32_t codePoint);

// Convert code point to lowercase
char32_t to_lower(char32_t codePoint);

// Returns the size in bytes of the code point that _str_ points to.
// This function reads the first byte and returns that result.
// Doesn't guarantee that an actual code point is there.
size_t get_size_of_code_point(const char *str);

// Returns the size that the code point would be if it were encoded.
size_t get_size_of_code_point(char32_t codePoint);

// Encodes code point at _str_, assumes there is enough space.
void encode_code_point(char *str, char32_t codePoint);

// Decodes a code point from a data pointer
char32_t decode_code_point(const char *str);

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