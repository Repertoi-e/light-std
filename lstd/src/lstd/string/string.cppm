module;

#include "../common.h"

export module lstd.string;

export import lstd.delegate;
export import lstd.stack_array;

LSTD_BEGIN_NAMESPACE

// @TODO: Make fully constexpr

//
// String doesn't guarantee a null termination at the end.
// It's essentially a data pointer and a count.
//
// This means that you can load a binary file into a string.
//
// The routines defined in array.cppm work with _string_ because
// _string_ is a typedef for array<char>. However they treat indices
// as pointing to bytes and NOT to code points.
//
// This file provides functions prefixed with string_ which
// treat indices properly (pointing to code points).
// Whenever working with strings we assume valid UTF-8.
// We don't do any checks, that is left up to the programmer to verify.
//
// @TODO: Provide a _string_utf8_validate()_.
//
// Functions on this object allow negative reversed indexing which begins at
// the end of the string, so -1 is the last byte, -2 the one before that, etc. (Python-style)
//
// Getting substrings doesn't allocate memory but just returns a new data pointer and count
// since strings in this library are not null-terminated.
//

export {
    using string = array<char>;

    constexpr s64 string_length(string s) { return utf8_length(s.Data, s.Count); }

    // Changes the code point at _index_ to a new one. May allocate and change the byte count of the string.
    void string_set(string * str, s64 index, code_point cp);

    struct code_point_ref {
        string *String;
        s64 Index;

        constexpr code_point_ref(string *s, s64 index) {
            String = s;
            Index  = translate_index(index, string_length(*s));
        }

        code_point_ref &operator=(code_point other) {
            string_set(String, Index, other);
            return *this;
        }

        constexpr operator code_point() { return utf8_decode_cp(utf8_get_pointer_to_cp_at_translated_index(String->Data, String->Count, Index)); }
    };

    // We return a special structure that allows assigning a new code point.
    // This is legal to do:
    //
    //      string a = "Hello";
    //      string_get(a, 0) = u8'Л';
    //      // a is now "Лello" and contains a two byte code point in the beginning.
    //
    // We need to do this because not all code points are 1 byte.
    constexpr code_point_ref string_get(string * str, s64 index) { return code_point_ref(str, index); }

    // Overload [] operator to call string_get:
    // - So indices are treated as pointing to code points.
    // - Handles assigning a new code point which is of different size.
    constexpr code_point_ref get_operator_square_brackets(string * s, s64 index) { return string_get(s, index); }

    // Doesn't allocate memory, strings in this library are not null-terminated.
    // We allow negative reversed indexing which begins at the end of the string,
    // so -1 is the last code point, -2 is the one before that, etc. (Python-style)
    constexpr string substring(string str, s64 begin, s64 end);

    //
    // Utilities to convert to c-style strings.
    // Functions for conversion between utf-8, utf-16 and utf-32 are provided in lstd.string_utils
    //

    // Allocates a buffer, copies the string's contents and also appends a zero terminator.
    // Uses the Context's current allocator. The caller is responsible for freeing.
    [[nodiscard("Leak")]] char *string_to_c_string(string s, allocator alloc = {});

    // Allocates a buffer, copies the string's contents and also appends a zero terminator.
    // Uses the temporary allocator.
    //
    // Implemented in lstd.context because otherwise we import in circle.
    char *string_to_c_string_temp(string s);

    //
    // String modification:
    //

    void string_insert_at_index(string * s, s64 index, const char *str, s64 size);
    void string_insert_at_index(string * s, s64 index, string str);
    void string_insert_at_index(string * s, s64 index, code_point cp);

    void string_append(string * str, const char *ptr, s64 size);
    void string_append(string * str, string s);
    void string_append(string * str, code_point cp);

    // Remove the first occurence of a code point.
    // Returns true on success (false if _cp_ was not found in the string).
    bool string_remove(string * s, code_point cp);

    // Remove code point at specified index.
    void string_remove_at_index(string * s, s64 index);

    // Remove a range of code points. [begin, end)
    void string_remove_range(string * s, s64 begin, s64 end);

    //
    // These get resolution in array.cppm.
    // For working with raw bytes and not code points...
    //
    // auto *insert_at_index(string *str, s64 index, char element);
    // auto *insert_pointer_and_size_at_index(string *str, s64 index, char *ptr, s64 size);
    // auto *insert_array_at_index(string *str, s64 index, array<char> arr2);
    // bool remove_ordered(string * str, char element);
    // bool remove_unordered(string * str, char element);
    // void remove_ordered_at_index(string * str, s64 index);
    // void remove_unordered_at_index(string * str, s64 index);
    // void remove_range(string * str, s64 begin, s64 end);
    // void replace_range(string * str, s64 begin, s64 end, string replace);
    //
    //
    // These actually work fine since we don't take indices, but we also provide
    // replace_all and remove_all overloads that work with code points and not single bytes:
    //
    // void replace_all(string * str, string search, string replace);
    // void remove_all(string *str, string search);
    //

    void replace_all(string * s, code_point search, code_point replace);
    void remove_all(string * s, code_point search);
    void remove_all(string * s, string search);
    void replace_all(string * s, code_point search, string replace);
    void replace_all(string * s, string search, code_point replace);

    //
    // String searching:
    //

    //
    // Since strings are array_likes, these get resolution:
    //
    // constexpr s64 find(string str, const delegate<bool(char &)> &predicate, s64 start = 0, bool reversed = false);
    // constexpr s64 find(string str, char search, s64 start = 0, bool reversed = false);
    // constexpr s64 find(string str, string search, s64 start = 0, bool reversed = false);
    // constexpr s64 find_any_of(string str, string allowed, s64 start = 0, bool reversed = false);
    // constexpr s64 find_not(string str, char element, s64 start = 0, bool reversed = false);
    // constexpr s64 find_not_any_of(string str, string banned, s64 start = 0, bool reversed = false);
    // constexpr bool has(string str, char item);
    //
    // They work for bytes and take indices that point to bytes.
    //

    //
    // Here are versions that work with code points and with indices that point to code points.
    //

    constexpr s64 string_find(string str, const delegate<bool(code_point)> &predicate, s64 start = 0, bool reversed = false);
    constexpr s64 string_find(string str, code_point search, s64 start = 0, bool reversed = false);
    constexpr s64 string_find(string str, string search, s64 start = 0, bool reversed = false);
    constexpr s64 string_find_any_of(string str, string allowed, s64 start = 0, bool reversed = false);
    constexpr s64 string_find_not(string str, code_point search, s64 start = 0, bool reversed = false);
    constexpr s64 string_find_not_any_of(string str, string banned, s64 start = 0, bool reversed = false);

    constexpr bool string_has(string str, code_point cp);

    //
    // Comparison:
    //

    //
    // Since strings are array_likes, these get resolution:
    //
    // constexpr s64 compare(string arr1, string arr2);
    // constexpr s32 compare_lexicographically(string arr1, string arr2);
    //
    // However they return indices that point to bytes.
    //
    // Here we provide string comparison functions which return indices to code points.
    //
    // Note: To check equality (with operator == which is defined in array_like.cpp)
    // we check the bytes. However that doesn't always work for Unicode.
    // Some strings which have different representation might be considered equal.
    //    e.g. the character é can be represented either as 'é' or as '´' combined with 'e' (two characters).
    // Note that UTF-8 specifically requires the shortest-possible encoding for characters,
    // but you still have to be careful (some convertors might not necessarily do that).
    //
    //

    // Compares two utf-8 encoded strings and returns the index
    // of the code point at which they are different or _-1_ if they are the same.
    constexpr s64 string_compare(string s, string other);

    // Compares two utf-8 encoded strings while ignoring case and returns the index
    // of the code point at which they are different or _-1_ if they are the same.
    constexpr s64 string_compare_ignore_case(string s, string other);

    // Compares two utf-8 encoded strings lexicographically and returns:
    //  -1 if _a_ is before _b_
    //   0 if a == b
    //   1 if _b_ is before _a_
    constexpr s32 string_compare_lexicographically(string a, string b);

    // Compares two utf-8 encoded strings lexicographically while ignoring case and returns:
    //  -1 if _a_ is before _b_
    //   0 if a == b
    //   1 if _b_ is before _a_
    constexpr s32 string_compare_lexicographically_ignore_case(string a, string b);

    // Returns true if _s_ begins with _str_
    constexpr bool match_beginning(string s, string str);

    // Returns true if _s_ ends with _str_
    constexpr bool match_end(string s, string str);

    // Returns a substring with white space removed at the start
    constexpr string trim_start(string s) { return substring(s, string_find_not_any_of(s, " \n\r\t\v\f"), string_length(s)); }

    // Returns a substring with white space removed at the end
    constexpr string trim_end(string s) { return substring(s, 0, string_find_not_any_of(s, " \n\r\t\v\f", string_length(s), true) + 1); }

    // Returns a substring with white space removed from both sides
    constexpr string trim(string s) { return trim_end(trim_start(s)); }

    template <bool Const>
    struct string_iterator {
        using string_t = types::select_t<Const, const string, string>;

        string_t *String;
        s64 Index;

        string_iterator(string_t *s = null, s64 index = 0) : String(s), Index(index) {}

        string_iterator &operator++() {
            Index += 1;
            return *this;
        }

        string_iterator operator++(s32) {
            string_iterator temp = *this;
            ++(*this);
            return temp;
        }

        auto operator<=>(string_iterator other) const { return Index <=> other.Index; };
        auto operator*() { return string_get(String, Index); }
        operator const char *() const { return utf8_get_pointer_to_cp_at_translated_index(String->Data, String->Count, Index); }
    };

    // To make range based for loops work.
    auto begin(string & str) { return string_iterator<false>(&str, 0); }
    auto end(string & str) { return string_iterator<false>(&str, string_length(str)); }

    auto begin(const string &str) { return string_iterator<true>(&str, 0); }
    auto end(const string &str) { return string_iterator<true>(&str, string_length(str)); }

    // Hashing strings...
    u64 get_hash(string value) {
        u64 hash        = 5381;
        For(value) hash = ((hash << 5) + hash) + it;
        return hash;
    }
}

constexpr code_point string_get(const string &str, s64 index) {
    if (index < 0) {
        // @Speed... should we cache this in _string_?
        // We need to calculate the total length (in code points)
        // in order for the negative index to be converted properly.
        s64 length = string_length(str);
        index      = translate_index(index, length);

        // If LSTD_ARRAY_BOUNDS_CHECK is defined:
        // _utf8_get_pointer_to_cp_at_translated_index()_ also checks for out of bounds but we are sure the index is valid
        // (since translate_index also checks for out of bounds) so we can get away with the unsafe version here.
        auto *s = str.Data;
        For(range(index)) s += utf8_get_size_of_cp(s);
        return utf8_decode_cp(s);
    } else {
        auto *s = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, index);
        return utf8_decode_cp(s);
    }
}

constexpr string substring(string str, s64 begin, s64 end) {
    s64 length     = string_length(str);
    s64 beginIndex = translate_index(begin, length);
    s64 endIndex   = translate_index(end, length, true);

    const char *beginPtr = utf8_get_pointer_to_cp_at_translated_index(str.Data, str.Count, beginIndex);
    const char *endPtr   = beginPtr;

    // @Speed
    For(range(beginIndex, endIndex)) endPtr += utf8_get_size_of_cp(endPtr);

    return string((char *) beginPtr, (s64) (endPtr - beginPtr));
}

constexpr bool operator==(string one, const char *other) { return string_compare(one, string(other)) == -1; }

constexpr bool match_beginning(string s, string str) {
    if (str.Count > s.Count) return false;
    return compare_memory(s.Data, str.Data, str.Count) == 0;
}

// Returns true if _s_ ends with _str_
constexpr bool match_end(string s, string str) {
    if (str.Count > s.Count) return false;
    return compare_memory(s.Data + s.Count - str.Count, str.Data, str.Count) == 0;
}

constexpr s64 string_find(string str, const delegate<bool(code_point)> &predicate, s64 start, bool reversed) {
    if (!str.Data || str.Count == 0) return -1;
    s64 length = string_length(str);
    start      = translate_index(start, length);
    For(range(start, reversed ? -1 : length, reversed ? -1 : 1)) if (predicate(string_get(str, it))) return it;
    return -1;
}

constexpr s64 string_find(string str, code_point search, s64 start, bool reversed) {
    if (!str.Data || str.Count == 0) return -1;
    s64 length = string_length(str);
    start      = translate_index(start, length);
    For(range(start, reversed ? -1 : length, reversed ? -1 : 1)) if (string_get(str, it) == search) return it;
    return -1;
}

constexpr s64 string_find(string str, string search, s64 start, bool reversed) {
    if (!str.Data || str.Count == 0) return -1;
    if (!search.Data || search.Count == 0) return -1;

    s64 length = string_length(str);
    start      = translate_index(start, length);
    start      = translate_index(start, string_length(str));

    s64 searchLength = string_length(search);

    For(range(start, reversed ? -1 : length, reversed ? -1 : 1)) {
        s64 progress = 0;
        for (s64 s = it; progress != searchLength; ++s, ++progress) {
            if (!(string_get(str, s) == string_get(search, progress))) break;
        }
        if (progress == searchLength) return it;
    }
    return -1;
}

constexpr bool string_has(string str, code_point cp) {
    char encodedCp[4];
    utf8_encode_cp(encodedCp, cp);

    return string_find(str, string(encodedCp, utf8_get_size_of_cp(cp))) != -1;
}

constexpr s64 string_find_any_of(string str, string allowed, s64 start, bool reversed) {
    if (!str.Data || str.Count == 0) return -1;
    s64 length = string_length(str);
    start      = translate_index(start, length);
    For(range(start, reversed ? -1 : length, reversed ? -1 : 1)) if (string_has(allowed, string_get(str, it))) return it;
    return -1;
}

constexpr s64 string_find_not(string str, code_point search, s64 start, bool reversed) {
    if (!str.Data || str.Count == 0) return -1;
    s64 length = string_length(str);
    start      = translate_index(start, length);
    For(range(start, reversed ? -1 : length, reversed ? -1 : 1)) if (string_get(str, it) != search) return it;
    return -1;
}

constexpr s64 string_find_not_any_of(string str, string banned, s64 start, bool reversed) {
    if (!str.Data || str.Count == 0) return -1;
    s64 length = string_length(str);
    start      = translate_index(start, length);
    For(range(start, reversed ? -1 : length, reversed ? -1 : 1)) if (!string_has(banned, string_get(str, it))) return it;
    return -1;
}

constexpr s64 string_compare(string s, string other) {
    if (!s && !other) return -1;
    if (!s || !other) return 0;

    auto *p1 = s.Data, *p2 = other.Data;
    auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

    s64 index = 0;
    while (utf8_decode_cp(p1) == utf8_decode_cp(p2)) {
        p1 += utf8_get_size_of_cp(p1);
        p2 += utf8_get_size_of_cp(p2);
        if (p1 == e1 && p2 == e2) return -1;
        if (p1 == e1 || p2 == e2) return index;
        ++index;
    }
    return index;
}

constexpr s64 string_compare_ignore_case(string s, string other) {
    if (!s && !other) return -1;
    if (!s || !other) return 0;

    auto *p1 = s.Data, *p2 = other.Data;
    auto *e1 = p1 + s.Count, *e2 = p2 + other.Count;

    s64 index = 0;
    while (to_lower(utf8_decode_cp(p1)) == to_lower(utf8_decode_cp(p2))) {
        p1 += utf8_get_size_of_cp(p1);
        p2 += utf8_get_size_of_cp(p2);
        if (p1 == e1 && p2 == e2) return -1;
        if (p1 == e1 || p2 == e2) return index;
        ++index;
    }
    return index;
}

constexpr s32 string_compare_lexicographically(string a, string b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;

    auto *p1 = a.Data, *p2 = b.Data;
    auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

    s64 index = 0;
    while (utf8_decode_cp(p1) == utf8_decode_cp(p2)) {
        p1 += utf8_get_size_of_cp(p1);
        p2 += utf8_get_size_of_cp(p2);
        if (p1 == e1 && p2 == e2) return 0;
        if (p1 == e1) return -1;
        if (p2 == e2) return 1;
        ++index;
    }
    return ((s64) utf8_decode_cp(p1) - (s64) utf8_decode_cp(p2)) < 0 ? -1 : 1;
}

constexpr s32 string_compare_lexicographically_ignore_case(string a, string b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;

    auto *p1 = a.Data, *p2 = b.Data;
    auto *e1 = p1 + a.Count, *e2 = p2 + b.Count;

    s64 index = 0;
    while (to_lower(utf8_decode_cp(p1)) == to_lower(utf8_decode_cp(p2))) {
        p1 += utf8_get_size_of_cp(p1);
        p2 += utf8_get_size_of_cp(p2);
        if (p1 == e1 && p2 == e2) return 0;
        if (p1 == e1) return -1;
        if (p2 == e2) return 1;
        ++index;
    }
    return ((s64) to_lower(utf8_decode_cp(p1)) - (s64) to_lower(utf8_decode_cp(p2))) < 0 ? -1 : 1;
}

void string_set(string *str, s64 index, code_point cp) {
    const char *target = utf8_get_pointer_to_cp_at_translated_index(str->Data, str->Count, index);

    char encodedCp[4];
    utf8_encode_cp(encodedCp, cp);

    replace_range(str, target - str->Data, target - str->Data + utf8_get_size_of_cp(target), string(encodedCp, utf8_get_size_of_cp(cp)));
}

[[nodiscard("Leak")]] char *string_to_c_string(string s, allocator alloc) {
    char *result = malloc<char>({.Count = s.Count + 1, .Alloc = alloc});
    copy_memory(result, s.Data, s.Count);
    result[s.Count] = '\0';
    return result;
}

void string_insert_at_index(string *s, s64 index, const char *str, s64 size) {
    index = translate_index(index, string_length(*s));
    insert_pointer_and_size_at_index(s, index, str, size);
}

void string_insert_at_index(string *s, s64 index, string str) { string_insert_at_index(s, index, str.Data, str.Count); }

void string_insert_at_index(string *s, s64 index, code_point cp) {
    char encodedCp[4];
    utf8_encode_cp(encodedCp, cp);
    string_insert_at_index(s, index, encodedCp, utf8_get_size_of_cp(cp));
}

void string_append(string *str, const char *ptr, s64 size) { insert_pointer_and_size_at_index(str, str->Count, ptr, size); }

void string_append(string *str, string s) { insert_at_index(str, str->Count, s); }
void string_append(string *str, code_point cp) { string_insert_at_index(str, string_length(*str), cp); }

bool string_remove(string *s, code_point cp) {
    char encodedCp[4];
    utf8_encode_cp(encodedCp, cp);

    s64 index = find(*s, string(encodedCp, utf8_get_size_of_cp(cp)));
    if (index == -1) return false;

    remove_range(s, index, index + utf8_get_size_of_cp(cp));

    return true;
}

void string_remove_at_index(string *s, s64 index) {
    index = translate_index(index, string_length(*s));

    auto *t = utf8_get_pointer_to_cp_at_translated_index(s->Data, s->Count, index);

    s64 b = t - s->Data;
    remove_range(s, b, b + utf8_get_size_of_cp(t));
}

void string_remove_range(string *s, s64 begin, s64 end) {
    s64 length = string_length(*s);

    begin = translate_index(begin, length);
    end   = translate_index(end, length);

    auto *tb = utf8_get_pointer_to_cp_at_translated_index(s->Data, s->Count, begin);
    auto *te = utf8_get_pointer_to_cp_at_translated_index(s->Data, s->Count, end);
    remove_range(s, tb - s->Data, te - s->Data);
}

void replace_all(string *s, code_point search, code_point replace) {
    char encodedOld[4];
    utf8_encode_cp(encodedOld, search);

    char encodedNew[4];
    utf8_encode_cp(encodedNew, replace);

    replace_all(s, string(encodedOld, utf8_get_size_of_cp(encodedOld)), string(encodedNew, utf8_get_size_of_cp(encodedNew)));
}

void remove_all(string *s, code_point search) {
    char encodedCp[4];
    utf8_encode_cp(encodedCp, search);

    replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), string(""));
}

void remove_all(string *s, string search) { replace_all(s, search, string("")); }

void replace_all(string *s, code_point search, string replace) {
    char encodedCp[4];
    utf8_encode_cp(encodedCp, search);

    replace_all(s, string(encodedCp, utf8_get_size_of_cp(encodedCp)), replace);
}

void replace_all(string *s, string search, code_point replace) {
    char encodedCp[4];
    utf8_encode_cp(encodedCp, replace);

    replace_all(s, search, string(encodedCp, utf8_get_size_of_cp(encodedCp)));
}

LSTD_END_NAMESPACE
