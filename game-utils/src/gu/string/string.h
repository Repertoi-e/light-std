#pragma once

#include "../memory/memory.h"

#include "utf8.h"

GU_BEGIN_NAMESPACE

// UTF-8 string
// #Design: This class' API is not final. Functions and operators may and WILL
// change.
//                              (And should change because it's not very
//                              practical atm)
struct string {
    char *Data = 0;
    // The size of the string in bytes, doesn't include the null terminator
    size_t Size = 0;
    // The capacity in bytes, includes the null terminator
    size_t Capacity = 1;

    // The allocator used for expanding the string.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's
    // one automatically.
    Allocator_Closure Allocator;

    string() {
        Allocator = CONTEXT_ALLOC;
        Data = New<char>(1, Allocator);
    }

    string(const char *str);
    // Construct from a cstring pointer and a size (in bytes, not code points!)
    string(const char *str, size_t size);
    string(string const &other);
    string(string &&other);

    // Returns the byte at that index, not necessarily the code point!
    char operator[](size_t index) const { return Data[index]; }
    string &operator=(string const &other);
    string &operator=(string &&other);

    ~string() {
        if (Data) {
            Delete(Data, Allocator);
        }
    }
};

inline string::string(const char *str) : string(str, str ? utf8size(str) : 0) {}

// Construct from a cstring pointer and a size (in bytes, not code points!)
inline string::string(const char *str, size_t size) {
    Allocator = CONTEXT_ALLOC;

    Data = New<char>(size + 1, Allocator);

    if (!str) {
        ZeroMemory(Data, size);
    }

    if (size && str) {
        utf8cpy(Data, str);
    }

    Size = size - 1;  // Exclude the null terminator
    Capacity = size;

    Data[size] = '\0';
}

inline string::string(string const &other) {
    Allocator = other.Allocator;
    Size = other.Size;
    Capacity = other.Capacity;

    Data = New<char>(Capacity, Allocator);

    if (!other.Data) {
        ZeroMemory(Data, Capacity);
    }

    if (Capacity && other.Data) {
        utf8cpy(Data, other.Data);
    }

    Data[Size] = '\0';
}

inline string::string(string &&other) { *this = std::move(other); }

inline string &string::operator=(string const &other) {
    if (Data) {
        Delete(Data, Allocator);
    }

    Allocator = other.Allocator;
    Size = other.Size;
    Capacity = other.Capacity;

    Data = New<char>(Capacity, Allocator);

    if (!other.Data) {
        ZeroMemory(Data, Capacity);
    }

    if (Capacity && other.Data) {
        utf8cpy(Data, other.Data);
    }

    Data[Size] = '\0';

    return *this;
}

inline string &string::operator=(string &&other) {
    if (this != &other) {
        if (Data) {
            Delete(Data, Allocator);
        }
        Allocator = other.Allocator;
        Size = other.Size;
        Capacity = other.Capacity;
        Data = other.Data;

        other.Data = 0;
        other.Size = 0;
        other.Capacity = 0;
    }
    return *this;
}

// Reserve bytes in string
inline void reserve(string &str, size_t size) {
    // Return if there is enough space
    if (str.Capacity > size) {
        return;
    }

    size_t oldSize = utf8size(str.Data);
    size_t newSize = size + 1;

    void *newData = str.Allocator.Function(Allocator_Mode::RESIZE, str.Allocator.Data, newSize, str.Data, oldSize, 0);

    str.Data = (char *) newData;
    str.Capacity = size + 1;
}

inline void clear_string(string &str) {
    str.Size = 0;
    str.Data[0] = '\0';
}

// The size in bytes, not including the null terminator
inline void append_cstring_and_size(string &str, const char *other, size_t size) {
    size_t neededCapacity = str.Size + size;

    reserve(str, neededCapacity);
    CopyMemory(str.Data + str.Size, other, size);

    str.Data[neededCapacity] = 0;
    str.Size = neededCapacity;
}

inline void append_cstring(string &str, const char *other) { append_cstring_and_size(str, other, utf8size(other) - 1); }

inline void append_string(string &str, string const &other) { append_cstring_and_size(str, other.Data, other.Size); }

inline string operator+(string str, string const &other) {
    append_string(str, other);
    return str;
}

inline string operator+(string str, const char *other) {
    append_cstring(str, other);
    return str;
}

inline string &operator+=(string &str, string const &other) {
    append_string(str, other);
    return str;
}

inline string &operator+=(string &str, const char *other) {
    append_cstring(str, other);
    return str;
}

inline size_t length(string const &str) { return utf8len(str.Data); }

inline b32 equal(string const &str, string const &other) { return utf8cmp(str.Data, other.Data) == 0; }

inline b32 operator==(string const &str, string const &other) { return equal(str, other); }

inline b32 operator!=(string const &str, string const &other) { return !(str == other); }

// gbString gb_set_string(gbString str, char const *cstr);
// gbUsize gb_string_allocation_size(gbString const str);
// gbString gb_trim_string(gbString str, char const *cut_set);

inline b32 is_digit(char x) { return x >= '0' && x <= '9'; }
inline b32 is_hexadecimal_digit(char x) { return (x >= '0' && x <= '9') || (x >= 'a' && x <= 'f'); }

inline b32 is_space(char x) { return (x >= 9 && x <= 13) || x == 32; }
inline b32 is_blank(char x) { return x == 9 || x == 32; }

inline b32 is_alpha(char x) { return (x >= 65 && x <= 90) || (x >= 97 && x <= 122); }
inline b32 is_alphanumeric(char x) { return is_alpha(x) || is_digit(x); }

inline b32 is_print(int x) { return x > 31 && x != 127; }

constexpr const char *find_cstring(const char *haystack, const char *needle) {
    if (!haystack || !needle) {
        return 0;
    }

    while (*haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (*n == '\0') {
            return haystack;
        }
        // Didn't match here. Try again further along haystack.
        haystack++;
    }
    return 0;
}

constexpr const char *find_cstring_last(const char *haystack, const char *needle) {
    if (*needle == '\0') {
        return haystack;
    }

    const char *result = 0;
    while (true) {
        const char *candidate = find_cstring(haystack, needle);
        if (!candidate) {
            break;
        }
        result = candidate;
        haystack = candidate + 1;
    }

    return result;
}

GU_END_NAMESPACE