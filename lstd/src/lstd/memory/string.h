#pragma once

#include "../memory/allocator.h"
#include "array.h"

LSTD_BEGIN_NAMESPACE

// UTF-8 string
// This string doesn't guarantee a null termination at the end.
//
// Documentation for array is relevant here.
//
// Contains the pre-calculated amount of code points in the string.
//
// Functions on this object allow negative reversed indexing which begins at
// the end of the string, so -1 is the last code point -2 the one before that, etc. (Python-style)
struct string : public array_view<utf8> {
    struct code_point_ref {
        string *Parent;
        s64 Index;

        code_point_ref() {}
        code_point_ref(string *parent, s64 index) : Parent(parent), Index(index) {}

        code_point_ref &operator=(utf32 other);
        operator utf32() const;
    };

    s64 Length = 0;  // Length of the string in unicode code points
    s64 Allocated = 0;

    constexpr string() {}

    // Create a string from a null terminated c-string.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    constexpr string(const utf8 *str) : array_view<utf8>((utf8 *) str, c_string_length(str)), Length(utf8_length(str, Count)) {}

    // This constructor allows constructing from the utf8 encoded u8"..." string literals.
    constexpr string(const char8_t *str) : array_view<utf8>((utf8 *) str, c_string_length(str)), Length(utf8_length((utf8 *) str, Count)) {}

    // Create a string from a buffer and a length.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    constexpr string(const utf8 *str, s64 size) : array_view<utf8>((utf8 *) str, size), Length(utf8_length(str, size)) {}

    // Create a string from a buffer and a length.
    // Note that this constructor doesn't validate if the passed in string is valid utf8.
    constexpr string(const byte *str, s64 size) : array_view<utf8>((utf8 *) str, size), Length(utf8_length(Data, size)) {}

    constexpr string(const array_view<utf8> &arr) : array_view<utf8>(arr), Length(utf8_length(Data, Count)) {}
    constexpr string(const array_view<byte> &arr) : array_view<utf8>((utf8 *) arr.Data, arr.Count), Length(utf8_length(Data, Count)) {}

    // Allocates a buffer (using the Context's allocator by default)
    string(utf32 codePoint, s64 repeat);

    // Allocates a buffer (using the Context's allocator by default)
    string(utf16 codePoint, s64 repeat) : string((utf32) codePoint, repeat) {}

    // Converts a null-terminated utf16 string to utf8.
    // Allocates a buffer (using the Context's allocator by default)
    explicit string(const utf16 *str);

    // Converts a null-terminated utf32 string to utf8.
    // Allocates a buffer (using the Context's allocator by default)
    explicit string(const utf32 *str);

    // We no longer use destructors for deallocation. Call release() explicitly (take a look at the defer macro!).
    // ~string() { release(); }

    // Makes sure string has reserved enough space for at least n bytes.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at 8.
    //
    // Allocates a buffer if the string doesn't already point to reserved memory.
    // Allocations are done using the Context's allocator and aligned if there is a specified
    // preferred alignment in the Context.
    void reserve(s64 size);

    // Resets Count and Length without freeing memory
    void reset();

    // Releases the memory allocated by this string and resets its members
    void release();

    // Gets the _index_'th code point in the string.
    // The returned code_point_ref object can be used to modify the code point at that location (by simply =).
    code_point_ref get(s64 index) { return code_point_ref(this, translate_index(index, Length)); }

    constexpr utf32 get(s64 index) const { return decode_cp(get_cp_at_index(Data, Length, index)); }

    constexpr bool begins_with(utf32 cp) const { return get(0) == cp; }
    constexpr bool begins_with(const string &str) const {
        if (str.Count > Count) return false;
        return compare_memory(Data, str.Data, str.Count) == -1;
    }

    constexpr bool ends_with(utf32 cp) const { return get(-1) == cp; }
    constexpr bool ends_with(const string &str) const {
        if (str.Count > Count) return false;
        return compare_memory(Data + Count - str.Count, str.Data, str.Count) == -1;
    }

    // Compares two utf8 encoded strings and returns the index of the code point
    // at which they are different or _-1_ if they are the same.
    constexpr s64 compare(const string &str) const { return compare_utf8(Data, Length, str.Data, str.Length); }

    // Compares two utf8 encoded strings ignoring case and returns the index of the code point
    // at which they are different or _-1_ if they are the same.
    constexpr s64 compare_ignore_case(const string &str) const {
        return compare_utf8_ignore_case(Data, Length, str.Data, str.Length);
    }

    // Compares two utf8 encoded strings and returns -1 if _one_ is before _two_,
    // 0 if one == two and 1 if _two_ is before _one_.
    constexpr s32 compare_lexicographically(const string &str) const {
        return compare_utf8_lexicographically(Data, Length, str.Data, str.Length);
    }

    // Compares two utf8 encoded strings ignorign case and returns -1 if _one_ is before _two_,
    // 0 if one == two and 1 if _two_ is before _one_.
    constexpr s32 compare_lexicographically_ignore_case(const string &str) const {
        return compare_utf8_lexicographically_ignore_case(Data, Length, str.Data, str.Length);
    }

    // Find the index of the first occurence of a code point that is after a specified index
    constexpr s64 find(utf32 cp, s64 start = 0) const {
        auto *p = find_cp_utf8(Data, Length, cp, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the first occurence of a substring that is after a specified index
    constexpr s64 find(const string &str, s64 start = 0) const {
        auto *p = find_substring_utf8(Data, Length, str.Data, str.Length, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the last occurence of a code point that is before a specified index
    constexpr s64 find_reverse(utf32 cp, s64 start = 0) const {
        auto *p = find_cp_utf8_reverse(Data, Length, cp, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the last occurence of a substring that is before a specified index
    constexpr s64 find_reverse(const string &str, s64 start = 0) const {
        auto *p = find_substring_utf8_reverse(Data, Length, str.Data, str.Length, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the first occurence of any code point in _terminators_ that is after a specified index
    constexpr s64 find_any_of(const string &terminators, s64 start = 0) const {
        auto *p = find_utf8_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the last occurence of any code point in _terminators_
    constexpr s64 find_reverse_any_of(const string &terminators, s64 start = 0) const {
        auto *p = find_utf8_reverse_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the first absence of a code point that is after a specified index
    constexpr s64 find_not(utf32 cp, s64 start = 0) const {
        auto *p = find_utf8_not(Data, Length, cp, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the last absence of a code point that is before the specified index
    constexpr s64 find_reverse_not(utf32 cp, s64 start = 0) const {
        auto *p = find_utf8_reverse_not(Data, Length, cp, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the first absence of any code point in _terminators_ that is after a specified index
    constexpr s64 find_not_any_of(const string &terminators, s64 start = 0) const {
        auto *p = find_utf8_not_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Find the index of the first absence of any code point in _terminators_ that is after a specified index
    constexpr s64 find_reverse_not_any_of(const string &terminators, s64 start = 0) const {
        auto *p = find_utf8_reverse_not_any_of(Data, Length, terminators.Data, terminators.Length, start);
        if (!p) return -1;
        return utf8_length(Data, p - Data);
    }

    // Gets [begin, end) range of characters into a new string object.
    // This function doesn't allocate, but just returns a "view".
    constexpr string substring(s64 begin, s64 end) const {
        auto [b, e] = substring_utf8(Data, Length, begin, end);
        return string(b, e - b);
    }

    // Returns a substring with whitespace removed at the start.
    // This function doesn't allocate, but just returns a "view".
    constexpr string trim_start() const { return substring(find_not_any_of(" \n\r\t\v\f"), Length); }

    // Returns a substring with whitespace removed at the end.
    // This function doesn't allocate, but just returns a "view".
    constexpr string trim_end() const { return substring(0, find_reverse_not_any_of(" \n\r\t\v\f") + 1); }

    // Returns a substring with whitespace removed from both sides.
    // This function doesn't allocate, but just returns a "view".
    constexpr string trim() const { return trim_start().trim_end(); }

    // Returns true if the string contains _cp_ anywhere
    constexpr bool has(utf32 cp) const { return find(cp) != -1; }

    // Returns true if the string contains _str_ anywhere
    constexpr bool has(const string &str) const { return find(str) != -1; }

    // Counts the number of occurences of _cp_
    constexpr s64 count(utf32 cp) const {
        s64 result = 0, index = 0;
        while ((index = find(cp, index)) != -1) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    // Counts the number of occurences of _str_
    constexpr s64 count(const string &str) const {
        s64 result = 0, index = 0;
        while ((index = find(str, index)) != -1) {
            ++result, ++index;
            if (index >= Length) break;
        }
        return result;
    }

    // Sets the _index_'th code point in the string.
    // Returns this.
    string *set(s64 index, utf32 codePoint);

    // Insert a code point at a specified index.
    // Returns this.
    string *insert(s64 index, utf32 codePoint);

    // Insert a buffer of bytes at a specified index.
    // Returns this.
    string *insert_pointer_and_size(s64 index, const utf8 *str, s64 size);

    // Insert a string at a specified index.
    // Returns this.
    string *insert_string(s64 index, const string &str) { return insert_pointer_and_size(index, str.Data, str.Count); }

    // Insert a list of bytes at a specified index.
    // Returns this.
    string *insert_list_of_bytes(s64 index, const initializer_list<utf8> &list) { return insert_pointer_and_size(index, list.begin(), list.size()); }

    // Remove code point at specified index.
    // Returns this.
    string *remove(s64 index);

    // Remove a range of code points. [begin, end)
    // Returns this.
    string *remove_range(s64 begin, s64 end);

    // Append a non encoded character to a string.
    // Returns this.
    string *append(utf32 codePoint) { return insert(Length, codePoint); }

    // Append _size_ bytes of string contained in _data_.
    // Returns this.
    string *append_pointer_and_size(const utf8 *str, s64 size) { return insert_pointer_and_size(Length, str, size); }

    // Append one string to another.
    // Returns this.
    string *append_string(const string &str) { return append_pointer_and_size(str.Data, str.Count); }

    // Append a list of bytes to the end.
    // Returns this.
    string *append_list_of_bytes(const initializer_list<utf8> &list) { return append_pointer_and_size(list.begin(), list.size()); }

    // Copy this string's contents and append them _n_ times.
    // Returns this.
    string *repeat(s64 n);

    // Convert this string to uppercase code points.
    // Returns this.
    string *to_upper();

    // Convert this string to lowercase code points.
    // Returns this.
    string *to_lower();

    // Removes all occurences of _cp_.
    // Returns this.
    string *remove_all(utf32 cp);

    // Remove all occurences of _str_.
    // Returns this.
    string *remove_all(const string &str);

    // Replace all occurences of _oldCp_ with _newCp_.
    // Returns this.
    string *replace_all(utf32 oldCp, utf32 newCp);

    // Replace all occurences of _oldStr_ with _newStr_.
    // Returns this.
    string *replace_all(const string &oldStr, const string &newStr);

    // Replace all occurences of _oldCp_ with _newStr_.
    // Returns this.
    string *replace_all(utf32 oldCp, const string &newStr);

    // Replace all occurences of _oldStr_ with _newCp_.
    // Returns this.
    string *replace_all(const string &oldStr, utf32 newCp);

    // Allocates a buffer and copies this string's contents and also appends a zero terminator.
    // The caller is responsible for freeing.
    [[nodiscard("Leak")]] utf8 *to_c_string(allocator alloc = {}) const {
        utf8 *result = allocate_array(utf8, Count + 1, alloc);
        copy_memory(result, Data, Count);
        result[Count] = '\0';
        return result;
    }

    //
    // Iterator:
    //
   private:
    template <bool Const>
    struct string_iterator {
        using string_t = types::select_t<Const, const string, string>;

        string_t *Parent;
        s64 Index;

        string_iterator() {}
        string_iterator(string_t *parent, s64 index) : Parent(parent), Index(index) {}

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

        s64 operator-(const string_iterator &other) const {
            s64 lesser = Index, greater = other.Index;
            if (lesser > greater) {
                lesser = other.Index;
                greater = Index;
            }
            s64 difference = greater - lesser;
            return Index <= other.Index ? difference : -difference;
        }

        string_iterator operator+(s64 amount) const { return string_iterator(Parent, Index + amount); }
        string_iterator operator-(s64 amount) const { return string_iterator(Parent, Index - amount); }

        friend string_iterator operator+(s64 amount, const string_iterator &it) { return it + amount; }
        friend string_iterator operator-(s64 amount, const string_iterator &it) { return it - amount; }

        bool operator==(const string_iterator &other) const { return Index == other.Index; }
        bool operator!=(const string_iterator &other) const { return Index != other.Index; }
        bool operator>(const string_iterator &other) const { return Index > other.Index; }
        bool operator<(const string_iterator &other) const { return Index < other.Index; }
        bool operator>=(const string_iterator &other) const { return Index >= other.Index; }
        bool operator<=(const string_iterator &other) const { return Index <= other.Index; }

        auto operator*() { return Parent->get(Index); }

        operator const utf8 *() const { return get_cp_at_index(Parent->Data, Parent->Length, Index, true); }
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

    // Returns true if the string contains any code points
    explicit operator bool() const { return Length; }

    operator array_view<utf8>() const { return array_view<utf8>(Data, Count); }
    operator array_view<byte>() const { return array_view<byte>((byte *) Data, Count); }

    // The non-const version allows to modify the character by simply =.
    code_point_ref operator[](s64 index) { return get(index); }
    constexpr utf32 operator[](s64 index) const { return get(index); }

    constexpr bool operator==(const string &other) const { return compare(other) == -1; }
    constexpr bool operator!=(const string &other) const { return !(*this == other); }
    constexpr bool operator<(const string &other) const { return compare_lexicographically(other) < 0; }
    constexpr bool operator>(const string &other) const { return compare_lexicographically(other) > 0; }
    constexpr bool operator<=(const string &other) const { return !(*this > other); }
    constexpr bool operator>=(const string &other) const { return !(*this < other); }
};

constexpr bool operator==(const utf8 *one, const string &other) { return other.compare_lexicographically(one) == 0; }
constexpr bool operator!=(const utf8 *one, const string &other) { return !(one == other); }
constexpr bool operator<(const utf8 *one, const string &other) { return other.compare_lexicographically(one) > 0; }
constexpr bool operator>(const utf8 *one, const string &other) { return other.compare_lexicographically(one) < 0; }
constexpr bool operator<=(const utf8 *one, const string &other) { return !(one > other); }
constexpr bool operator>=(const utf8 *one, const string &other) { return !(one < other); }

// Be careful not to call this with _dest_ pointing to _src_!
// Returns just _dest_.
string *clone(string *dest, const string &src);

// Hash for strings
inline u64 get_hash(const string &value) {
    u64 hash = 5381;
    For(value) hash = ((hash << 5) + hash) + it;
    return hash;
}

LSTD_END_NAMESPACE
