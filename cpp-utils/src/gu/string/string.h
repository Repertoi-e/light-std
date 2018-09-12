#pragma once

#include "../memory/memory.h"

#include "utf8.h"

GU_BEGIN_NAMESPACE

// This class is a wrapper around a pointer
// to an encoded utf-8 code point.
struct Code_Point_Ref {
    char *Data;

    Code_Point_Ref &operator=(char32_t other) {
		
		
		return *this;
	}

	b32 operator==(char32_t other) {
		
	}

    b32 operator!=(char32_t other) { return !(*this == other); }
};

// UTF-8 string
// This string doesn't guarantee a null termination at the end.
// It stores a data pointer and a length.
//
// * Usually structures in this library are named Like_This, but this
// is an exception since I consider it as a fundamental data type.
struct string {
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

    // The number of bytes used in the string, >= the number of utf-8 code points
    // that the string represents
    // In order to get the length of the string in utf-8 code points, use
    // the length() function.
    size_t CountBytes = 0;

    // The allocator used for expanding the string.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    string() {}
    // Construct a string from a null-terminated c-style string.
    string(const char *str);
    // Construct from a c-style string and a length (in bytes, not utf-8 code points)
    string(const char *str, size_t length);
    string(string const &other);
    string(string &&other);
    ~string();

    string &operator=(string const &other);
    string &operator=(string &&other);

    // Returns a reference to the the code point at that index
	// so it can be accessed and modified
    const char32_t operator[](size_t index) const;
};

// Retrieve the length of a standard cstyle string.
// Doesn't care about encoding.
// Note that this calculation does not include the null byte.
// This function can also be used to determine the size in
// bytes of a null terminated utf-8 string.
size_t cstyle_strlen(const char *str);

// Releases the memory allocated by this string.
void release(string &str);

// Return sthe number of code points in the string.
size_t length(string const &str);

// Clears all characters from the string
// (Sets CountBytes to 0)
inline void clear_string(string &str) { str.CountBytes = 0; }

// Reserve bytes in string
void reserve(string &str, size_t size);

// Sets the _index_'th code point of the string 
void set(string &str, size_t index, char32_t codePoint);

// Returns a pointer to the _index_th code point in the string.
char *get_pointer_to_index(string &str, size_t index);

// Check two strings for equality
b32 equal(string const &str, string const &other);
inline b32 operator==(string const &str, string const &other) { return equal(str, other); }
inline b32 operator!=(string const &str, string const &other) { return !(str == other); }

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

inline string operator+=(string str, char32_t codePoint) {
	append(str, codePoint);
	return str;
}

// #TODO: More string utility functions
// gbString gb_trim_string(gbString str, char const *cut_set);

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

// Returns the size in bytes of the code point that _str_ points to.
// This function reads the first byte and returns that result.
// Doesn't guarantee that an actual code point is there.
size_t get_size_of_code_point(const char *str);

// Returns the size that the code point would be if it were encoded.
size_t get_size_of_code_point(char32_t codePoint);

// Encodes code point at _str_, assumes there is enough space.
void encode_code_point(char *str, char32_t codePoint);


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