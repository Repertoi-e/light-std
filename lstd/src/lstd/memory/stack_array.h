#pragma once

#include "../internal/common.h"
#include "string_utils.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
constexpr T *quick_sort_partition(T *first, T *last, T *pivot) {
    --last;
    swap(*pivot, *last);
    pivot = last;

    while (true) {
        while (*first < *pivot) ++first;
        --last;
        while (*pivot < *last) --last;
        if (first >= last) {
            swap(*pivot, *first);
            return first;
        }
        swap(*first, *last);
        ++first;
    }
}

template <typename T>
constexpr void quick_sort(T *first, T *last) {
    if (first >= last) return;

    auto *pivot = first + (last - first) / 2;
    auto *nextPivot = quick_sort_partition(first, last, pivot);
    quick_sort(first, nextPivot);
    quick_sort(nextPivot + 1, last);
}

// @TODO: Document use cases for this and why is it different from array<T>
template <typename T, s64 N>
struct stack_array {
    using data_t = T;

    using iterator = data_t *;
    using const_iterator = const data_t *;

    data_t Data[N ? N : 1];
    static constexpr s64 Count = N;

    constexpr data_t &get(s64 index) { return Data[translate_index(index, Count)]; }
    constexpr const data_t &get(s64 index) const { return Data[translate_index(index, Count)]; }

    // Compares this array to _arr_ and returns the index of the first element that is different.
    // If the arrays are equal, the returned value is -1.
    constexpr s32 compare(const stack_array &arr) const {
        if (!Count && !arr.Count) return -1;
        if (!Count || !arr.Count) return 0;

        auto s1 = begin(), s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return -1;
            if (s1 == end()) return s1 - begin();
            if (s2 == arr.end()) return s2 - arr.begin();
        }
        return s1 - begin();
    }

    // Compares this array to to _arr_ lexicographically.
    // The result is less than 0 if this array sorts before the other, 0 if they are equal, and greater than 0 otherwise.
    constexpr s32 compare_lexicographically(const stack_array &arr) const {
        if (!Count && !arr.Count) return -1;
        if (!Count) return -1;
        if (!arr.Count) return 1;

        auto s1 = begin(), s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return 0;
            if (s1 == end()) return -1;
            if (s2 == arr.end()) return 1;
        }
        return s1 < s2 ? -1 : 1;
    }

    //
    // Operators:
    //

    operator array<T>() const;

    constexpr data_t &operator[](s64 index) { return get(index); }
    constexpr const data_t &operator[](s64 index) const { return get(index); }

    constexpr iterator begin() { return Data; }
    constexpr iterator end() { return Data + Count; }
    constexpr const_iterator begin() const { return Data; }
    constexpr const_iterator end() const { return Data + Count; }

    // Check two arrays for equality
    constexpr bool operator==(const stack_array &other) const { return compare_lexicographically(other) == 0; }
    constexpr bool operator!=(const stack_array &other) const { return !(*this == other); }
    constexpr bool operator<(const stack_array &other) const { return compare_lexicographically(other) < 0; }
    constexpr bool operator>(const stack_array &other) const { return compare_lexicographically(other) > 0; }
    constexpr bool operator<=(const stack_array &other) const { return !(*this > other); }
    constexpr bool operator>=(const stack_array &other) const { return !(*this < other); }
};

namespace internal {
template <typename D, typename...>
struct return_type_helper {
    using type = D;
};
template <typename... Types>
struct return_type_helper<void, Types...> : common_type<Types...> {};

template <class T, s64 N, s64... I>
constexpr stack_array<remove_cv_t<T>, N> to_array_impl(T (&a)[N], index_sequence<I...>) {
    return {{a[I]...}};
}
}  // namespace internal

template <typename D = void, class... Types>
constexpr stack_array<typename internal::return_type_helper<D, Types...>::type, sizeof...(Types)> to_stack_array(
    Types &&... t) {
    return {(Types &&)(t)...};
}

template <typename T, s64 N>
constexpr stack_array<remove_cv_t<T>, N> to_stack_array(T (&a)[N]) {
    return internal::to_array_impl(a, make_index_sequence<N>{});
}

LSTD_END_NAMESPACE
