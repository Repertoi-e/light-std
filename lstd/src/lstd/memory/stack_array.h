#pragma once

#include "../internal/common.h"
#include "../types/sequence.h"
#include "array_like.h"
#include "string_utils.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
using quick_sort_comparison_func = s32 (*)(const T *, const T *);

template <typename T>
constexpr T *quick_sort_partition(T *first, T *last, T *pivot, quick_sort_comparison_func<T> func) {
    --last;
    swap(*pivot, *last);
    pivot = last;

    while (true) {
        while (func(first, pivot) < 0) ++first;
        --last;
        while (func(pivot, last) < 0) --last;
        if (func(first, last) > 0) {
            swap(*pivot, *first);
            return first;
        } else {
            swap(*first, *last);
            ++first;
        }
    }
}

template <typename T>
s32 default_comparison(const T *lhs, const T *rhs) {
    if (*lhs > *rhs) return 1;
    if (*lhs < *rhs) return -1;
    return 0;
}

template <typename T>
constexpr void quick_sort(T *first, T *last, quick_sort_comparison_func<T> func = default_comparison<T>) {
    if (first >= last) return;

    auto *pivot = first + (last - first) / 2;
    auto *nextPivot = quick_sort_partition(first, last, pivot, func);
    quick_sort(first, nextPivot, func);
    quick_sort(nextPivot + 1, last, func);
}

template <typename T>
struct array_view;

//
// A wrapper around T arr[..] which makes it easier to pass around and work with.
//
// To make an array from a list of elements use:
//
//  auto arr1 = to_stack_array(1, 4, 9);
//  auto arr2 = to_stack_array<s64>(1, 4, 9);
//
// To iterate:
// For(arr1) {
//     ...
// }
//
// For(range(arr1.Count)) {
//     T element = arr1[it];
//     ...
// }
//
// Different from array<T>, because the latter supports dynamic resizing.
// This object contains no other member than T Data[N], _Count_ is a static member for the given type and doesn't take space.
template <typename T_, s64 N>
struct stack_array {
    using T = T_;

    // :CodeReusability: Automatically generates ==, !=, <, <=, >, >=, compare_*, find_*, has functions etc.. take a look at "array_like.h"
    static constexpr bool IS_ARRAY_LIKE = true;

    T Data[N ? N : 1];
    static constexpr s64 Count = N;

    //
    // Iterators:
    //
    using iterator = T *;
    using const_iterator = const T *;

    constexpr iterator begin() { return Data; }
    constexpr iterator end() { return Data + Count; }
    constexpr const_iterator begin() const { return Data; }
    constexpr const_iterator end() const { return Data + Count; }

    //
    // Operators:
    //
    operator array_view<T>() const;

    constexpr T &operator[](s64 index) { return Data[translate_index(index, Count)]; }
    constexpr const T &operator[](s64 index) const { return Data[translate_index(index, Count)]; }
};

namespace internal {
template <typename D, typename...>
struct return_type_helper {
    using type = D;
};
template <typename... Types>
struct return_type_helper<void, Types...> : types::common_type<Types...> {};

template <class T, s64 N, s64... I>
constexpr stack_array<types::remove_cv_t<T>, N> to_array_impl(T (&a)[N], integer_sequence<I...>) {
    return {{a[I]...}};
}
}  // namespace internal

template <typename D = void, class... Types>
constexpr stack_array<typename internal::return_type_helper<D, Types...>::type, sizeof...(Types)> to_stack_array(Types &&...t) {
    return {(Types &&)(t)...};
}

template <typename T, s64 N>
constexpr stack_array<types::remove_cv_t<T>, N> to_stack_array(T (&a)[N]) {
    return internal::to_array_impl(a, make_integer_sequence<N>{});
}

LSTD_END_NAMESPACE
