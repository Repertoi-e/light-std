#pragma once

#include "../common.h"
#include "string_utils.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
constexpr T *partition(T *first, T *last, T *pivot) {
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
constexpr void quicksort(T *first, T *last) {
    if (first >= last) return;

    auto *pivot = first + (last - first) / 2;
    auto *nextPivot = partition(first, last, pivot);
    quicksort(first, nextPivot);
    quicksort(nextPivot + 1, last);
}

template <typename T, size_t N>
struct stack_array {
    static_assert(N > 0, "Cannot have a zero-sized array.");

    using data_t = T;

    using iterator = data_t *;
    using const_iterator = const data_t *;

    data_t Data[N];
    static constexpr size_t Count = N;

    constexpr data_t &get(s64 index) { return Data[translate_index(index, Count)]; }
    constexpr const data_t &get(s64 index) const { return Data[translate_index(index, Count)]; }

    constexpr void sort() { quicksort(Data, Data + Count); }

    // Compares this array to _arr_ and returns the index of the first element that is different.
    // If the arrays are equal, the returned value is npos (-1)
    constexpr s32 compare(const stack_array &arr) const {
        auto s1 = begin(), s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return npos;
            if (s1 == end()) return s1 - begin();
            if (s2 == arr.end()) return s2 - arr.begin();
        }
        return s1 - begin();
    }

    // Compares this array to to _arr_ lexicographically.
    // The result is less than 0 if this array sorts before the other, 0 if they are equal,
    // and greater than 0 otherwise.
    constexpr s32 compare_lexicographically(const stack_array &arr) const {
        auto s1 = begin(), s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return 0;
            if (s1 == end()) return -1;
            if (s2 == arr.end()) return 1;
        }
        return s1 < s2 ? -1 : 1;
    }

    // Find the first occurence of an element that is after a specified index
    size_t find(const T &element, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (*p++ == element) return it;
        return npos;
    }

    // Find the first occurence of a subarray that is after a specified index
    template <size_t NN>
    size_t find(const stack_array<T, NN> &arr, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        For(range(start, Count)) {
            auto progress = arr.begin();
            for (auto search = begin() + it; progress != arr.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == arr.end()) return it;
        }
        return npos;
    }

    // Find the last occurence of an element that is before a specified index
    size_t find_reverse(const T &element, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (*p-- == element) return it;
        return npos;
    }

    // Find the last occurence of a subarray that is before a specified index
    template <size_t NN>
    size_t find_reverse(const stack_array<T, NN> &arr, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        For(range(start - arr.Count + 1, -1, -1)) {
            auto progress = arr.begin();
            for (auto search = begin() + it; progress != arr.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == arr.end()) return it;
        }
        return npos;
    }

    // Find the first occurence of any element in the specified subarray that is after a specified index
    template <size_t NN>
    size_t find_any_of(const stack_array<T, NN> &allowed, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (allowed.has(*p++)) return it;
        return npos;
    }

    // Find the last occurence of any element in the specified subarray
    // that is before a specified index (0 means: start from the end)
    template <size_t NN>
    size_t find_reverse_any_of(const stack_array<T, NN> &allowed, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (allowed.has(*p--)) return it;
        return npos;
    }

    // Find the first absence of an element that is after a specified index
    size_t find_not(const data_t &element, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (*p++ != element) return it;
        return npos;
    }

    // Find the last absence of an element that is before the specified index
    size_t find_reverse_not(const data_t &element, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (*p-- != element) return it;
        return npos;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    template <size_t NN>
    size_t find_not_any_of(const stack_array<T, NN> &banned, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (!banned.has(*p++)) return it;
        return npos;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    template <size_t NN>
    size_t find_reverse_not_any_of(const stack_array<T, NN> &banned, s64 start = 0) const {
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (!banned.has(*p--)) return it;
        return npos;
    }

    constexpr bool has(const data_t &item) const { return find(item) != npos; }

    //
    // Operators:
    //

    constexpr data_t &operator[](s64 index) { return get(index); }
    constexpr const data_t &operator[](s64 index) const { get(index); }

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

template <class T, size_t N, size_t... I>
constexpr stack_array<remove_cv_t<T>, N> to_array_impl(T (&a)[N], index_sequence<I...>) {
    return {{a[I]...}};
}
}  // namespace internal

template <typename D = void, class... Types>
constexpr stack_array<typename internal::return_type_helper<D, Types...>::type, sizeof...(Types)> to_array(
    Types &&... t) {
    return {(Types &&)(t)...};
}

template <typename T, size_t N>
constexpr stack_array<remove_cv_t<T>, N> to_array(T (&a)[N]) {
    return internal::to_array_impl(a, make_index_sequence<N>{});
}

LSTD_END_NAMESPACE