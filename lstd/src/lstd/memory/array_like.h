#pragma once

#include "../memory/string_utils.h"
#include "../types.h"

//
// :CodeReusability: This file implements find, find_not, find_any_of, find_not_any_of, has, 
//                                        compare, compare_lexicographically and operators ==, !=, <, <=, >, >=
// for structures that have members Data and Count and a flag IS_ARRAY_LIKE - we call these array-likes.
//
// The flag is there to not collide with user types that can have Data and Count members but should not be considered array-like..
// The flag is good because it is explicit!
//
// e.g. stack_array, array, delegate, guid, have these members, so != is automatically generated to all three types using the definition below.
// It also resolves for different types, e.g. stack_array != array, etc. for which otherwise we need a combinatorial amount of code.
// 
// Your flagged types will also automatically get this treatment.
//

LSTD_BEGIN_NAMESPACE

// True if the type has _Data_ and _Count_ members.
// We use this for generic methods below that work on either array or stack_array or your own custom array types! We want code reusability!
template <typename T>
concept array_like = requires(T t) {
    {t.IS_ARRAY_LIKE == true};
    {t.Data};
    {t.Count};
};

// This returns the type of the _Data_ member of an array-like object
template <typename ArrayT>
using array_like_data_t = types::remove_pointer_t<decltype(ArrayT::Data)>;

// We undef this at the end of the file.. short name just for brevity
#define data_t array_like_data_t

//
// Search functions for arrays:
//

// Find the first occurence of an element which matches the predicate and is after a specified index.
// Predicate must take a single argument (the current element) and return if it matches.
template <array_like T>
constexpr s64 find(const T &arr, const delegate<bool(data_t<T> &)> &predicate, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reversed ? -1 : arr.Count), (reversed ? -1 : 1))) if (predicate(arr.Data[it])) return it;
    return -1;
}

// Find the first occurence of an element that is after a specified index
template <array_like T>
constexpr s64 find(const T &arr, const data_t<T> &element, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reversed ? -1 : arr.Count), (reversed ? -1 : 1))) if (arr.Data[it] == element) return it;
    return -1;
}

// Find the first occurence of a subarray that is after a specified index
template <array_like T>
constexpr s64 find(const T &arr, const T &arr2, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    if (!arr2.Data || arr2.Count == 0) return -1;
    start = translate_index(start, arr.Count) - arr2.Count;
    start = min(start, arr2.Count - arr2.Count);  // We start at most the end minus _arr2_'s length because it cannot start later
    For(range(start, (reversed ? -1 : arr.Count), (reversed ? -1 : 1))) {
        auto progress = arr2.Data;
        for (auto search = arr.Data + it; progress != arr2.Data + arr2.Count; ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == arr.end()) return it;
    }
    return -1;
}

// Find the first occurence of any element in the specified subarray that is after a specified index
template <array_like T>
constexpr s64 find_any_of(const T &arr, const T &allowed, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    if (!allowed.Data || allowed.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reversed ? -1 : arr.Count), (reversed ? -1 : 1))) if (allowed.has(arr.Data[it])) return it;
    return -1;
}

// Find the first absence of an element that is after a specified index
template <array_like T>
constexpr s64 find_not(const T &arr, const data_t<T> &element, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reversed ? -1 : arr.Count), (reversed ? -1 : 1))) if (arr.Data[it] != element) return it;
    return -1;
}

// Find the first absence of any element in the specified subarray that is after a specified index
template <array_like T>
constexpr s64 find_not_any_of(const T &arr, const T &banned, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    if (!banned.Data || banned.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reversed ? -1 : arr.Count), (reversed ? -1 : 1))) if (!banned.has(arr.Data[it])) return it;
    return -1;
}

// Checks if _item_ is contained in the array
template <array_like T>
constexpr bool has(const T &arr, const data_t<T> &item) { return find(arr, item) != -1; }

//
// Compare functions for arrays:
//

template <typename T, typename U>
concept array_likes_comparable = requires(T t, U u) { t.Data[0] == u.Data[0]; };

template <typename T, typename U>
concept array_likes_lexicographically_comparable = requires(T t, U u) {
    t.Data[0] == u.Data[0];
    t.Data[0] < u.Data[0];
};

// Compares this array to _arr_ and returns the index of the first element that is different.
// If the arrays are equal, the returned value is -1.
template <array_like T, array_like U>
requires array_likes_comparable<T, U> constexpr s64 compare(const T &arr1, const U &arr2) {
    if ((void *) arr1.Data == (void *) arr2.Data && arr1.Count == arr2.Count) return -1;

    if (!arr1.Count && !arr2.Count) return -1;
    if (!arr1.Count || !arr2.Count) return 0;

    auto *s1 = arr1.Data;
    auto *s2 = arr2.Data;
    auto *end1 = arr1.Data + arr1.Count;
    auto *end2 = arr2.Data + arr2.Count;
    while (*s1 == *s2) {
        ++s1, ++s2;
        if (s1 == end1 && s2 == end2) return -1;
        if (s1 == end1) return s1 - arr1.Data;
        if (s2 == end2) return s2 - arr2.Data;
    }
    return s1 - arr1.Data;
}

// Compares this array to to _arr_ lexicographically.
// The result is -1 if this array sorts before the other, 0 if they are equal, and +1 otherwise.
template <array_like T, array_like U>
requires array_likes_lexicographically_comparable<T, U> s32 compare_lexicographically(const T &arr1, const U &arr2) {
    if ((void *) arr1.Data == (void *) arr2.Data && arr1.Count == arr2.Count) return 0;

    if (!arr1.Count && !arr2.Count) return 0;
    if (!arr1.Count) return -1;
    if (!arr2.Count) return 1;

    auto *s1 = arr1.Data;
    auto *s2 = arr2.Data;
    auto *end1 = arr1.Data + arr1.Count;
    auto *end2 = arr2.Data + arr2.Count;
    while (*s1 == *s2) {
        ++s1, ++s2;
        if (s1 == end1 && s2 == end2) return 0;
        if (s1 == end1) return -1;
        if (s2 == end2) return 1;
    }
    return *s1 < *s2 ? -1 : 1;
}

//
// Comparison operators for arrays:
//

// @Cleanup: C++20 space ship operator to reduce this bloat..
// .. except that I tried it and it doesn't work for some reason?
// For now let's implement them separately..

template <array_like T, array_like U>
requires array_likes_comparable<T, U> constexpr bool operator==(const T &arr1, const U &arr2) {
    return compare(arr1, arr2) == -1;
}

template <array_like T, array_like U>
requires array_likes_comparable<T, U> constexpr bool operator!=(const T &arr1, const U &arr2) {
    return compare(arr1, arr2) != -1;
}

template <array_like T, array_like U>
requires array_likes_lexicographically_comparable<T, U> constexpr bool operator<(const T &arr1, const U &arr2) {
    return compare_lexicographically(arr1, arr2) < 0;
}

template <array_like T, array_like U>
requires array_likes_lexicographically_comparable<T, U> constexpr bool operator<=(const T &arr1, const U &arr2) {
    return !(compare_lexicographically(arr1, arr2) > 0);
}

template <array_like T, array_like U>
requires array_likes_lexicographically_comparable<T, U> constexpr bool operator>(const T &arr1, const U &arr2) {
    return compare_lexicographically(arr1, arr2) > 0;
}

template <array_like T, array_like U>
requires array_likes_lexicographically_comparable<T, U> constexpr bool operator>=(const T &arr1, const U &arr2) {
    return !(compare_lexicographically(arr1, arr2) < 0);
}

#undef data_t

LSTD_END_NAMESPACE
