#pragma once

#include "../types.h"

// @CodeReusability This file implements find, find_not, find_any_of, find_not_any_of, has, compare, compare_lexicographically and operators ==, !=, <, <=, >, >=
// for structures that have members Data and Count - we call these array-likes.
//
// e.g. stack_array, array, delegate have these members, so find(arr, 3) automatically resolves to the definition below.
// Your types which have _Data_, and _Count_ also get this treatment.
//
// @TODO: Maybe instead of automatically detecting members we should have an explicit macro which expands to some template code meta magic to tell
// is_array_like to recognize the type. And also maybe the user has a different naming convention (or he can't modify the source of the type he is
// trying to mark as array_like) so we should be able to specify the respective names for the _Data_ and _Count_ members.

LSTD_BEGIN_NAMESPACE

// True if the type has _Data_ and _Count_ members.
//
// We use this for generic methods below that work on either array or stack_array or your own custom array types! We want code reusability!
template <typename T>
struct is_array_like {
    template <typename U>
    static true_t test(decltype(U::Data) *, decltype(U::Count) *);

    template <typename>
    static false_t test(...);

    static constexpr bool value = is_same_v<decltype(test<T>(null, null)), true_t>;
};

template <typename T>
constexpr bool is_array_like_v = is_array_like<T>::value;

// This returns the type of the _Data_ member of an array-like object
template <typename ArrayT>
using get_type_of_data_t = remove_pointer_t<decltype(ArrayT::Data)>;

//
// Find functions for arrays:
//

// Find the first occurence of an element which matches the predicate and is after a specified index.
// Predicate must take a single argument (the current element) and return if it matches.
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, s64> find(const ArrayT &arr, const delegate<bool(const decltype(*ArrayT::Data) &)> &predicate, s64 start = 0, bool reverse = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reverse ? -1 : arr.Count), (reverse ? -1 : 1))) if (predicate(arr.Data[it])) return it;
    return -1;
}

// Find the first occurence of an element that is after a specified index
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, s64> find(const ArrayT &arr, const get_type_of_data_t<ArrayT> &element, s64 start = 0, bool reverse = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    start = translate_index(start, arr.Count);
    For(range(start, (reverse ? -1 : arr.Count), (reverse ? -1 : 1))) if (arr.Data[it] == element) return it;
    return -1;
}

// Find the first occurence of a subarray that is after a specified index
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, s64> find(const ArrayT &arr, const ArrayT &arr2, s64 start = 0, bool reverse = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    if (!arr2.Data || arr2.Count == 0) return -1;
    start = translate_index(start, arr.Count) - arr2.Count;
    start = min(start, arr2.Count - arr2.Count);  // We start at most the end minus _arr2_'s length because it cannot start later
    For(range(start, (reverse ? -1 : arr.Count), (reverse ? -1 : 1))) {
        auto progress = arr2.Data;
        for (auto search = arr.Data + it; progress != arr2.Data + arr2.Count; ++search, ++progress) {
            if (*search != *progress) break;
        }
        if (progress == arr.end()) return it;
    }
    return -1;
}

// Find the first occurence of any element in the specified subarray that is after a specified index
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, s64> find_any_of(const ArrayT &arr, const ArrayT &allowed, s64 start = 0, bool reverse = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    if (!allowed.Data || allowed.Count == 0) return -1;
    start = translate_index(start, Count);
    For(range(start, (reverse ? -1 : arr.Count), (reverse ? -1 : 1))) if (allowed.has(arr.Data[it])) return it;
    return -1;
}

// Find the first absence of an element that is after a specified index
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, s64> find_not(const ArrayT &arr, const decltype(*ArrayT::Data) &element, s64 start = 0, bool reversed = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    start = translate_index(start, Count);
    For(range(start, (reverse ? -1 : arr.Count), (reverse ? -1 : 1))) if (arr.Data[it] != element) return it;
    return -1;
}

// Find the first absence of any element in the specified subarray that is after a specified index
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, s64> find_not_any_of(const ArrayT &arr, const ArrayT &banned, s64 start = 0, bool reverse = false) {
    if (!arr.Data || arr.Count == 0) return -1;
    if (!banned.Data || banned.Count == 0) return -1;
    start = translate_index(start, Count);
    For(range(start, (reverse ? -1 : arr.Count), (reverse ? -1 : 1))) if (!banned.has(arr.Data[it])) return it;
    return -1;
}

// Checks if _item_ is contained in the array
template <typename ArrayT>
constexpr enable_if_t<is_array_like_v<ArrayT>, bool> has(const ArrayT &arr, const get_type_of_data_t<ArrayT> &item) { return find(arr, item) != -1; }

//
// Compare functions for arrays:
//

// Compares this array to _arr_ and returns the index of the first element that is different.
// If the arrays are equal, the returned value is -1.
template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, s64> compare(const ArrayT &arr1, const ArrayU &arr2) {
    using T = decltype(*arr1.Data);
    using U = decltype(*arr2.Data);

    static_assert(is_equal_comparable_v<T, U>, "Arrays have types which cannot be compared with operator ==");

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
template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, s32> compare_lexicographically(const ArrayT &arr1, const ArrayU &arr2) {
    using T = decltype(*arr1.Data);
    using U = decltype(*arr2.Data);

    static_assert(is_equal_comparable_v<T, U>, "Arrays have types which cannot be compared with operator ==");
    static_assert(is_less_comparable_v<T, U>, "Arrays have types which cannot be compared with operator <");

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

template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, bool> operator==(const ArrayT &arr1, const ArrayU &arr2) {
    return compare(arr1, arr2) == -1;
}

template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, bool> operator!=(const ArrayT &arr1, const ArrayU &arr2) {
    return compare(arr1, arr2) != -1;
}

template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, bool> operator<(const ArrayT &arr1, const ArrayU &arr2) {
    return compare_lexicographically(arr1, arr2) < 0;
}

template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, bool> operator>(const ArrayT &arr1, const ArrayU &arr2) {
    return compare_lexicographically(arr1, arr2) > 0;
}

template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, bool> operator<=(const ArrayT &arr1, const ArrayU &arr2) {
    return !(arr1 > arr2);
}

template <typename ArrayT, typename ArrayU>
constexpr enable_if_t<is_array_like_v<ArrayT> && is_array_like_v<ArrayU>, bool> operator>=(const ArrayT &arr1, const ArrayU &arr2) {
    return !(arr1 < arr2);
}

LSTD_END_NAMESPACE
