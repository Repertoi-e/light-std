#pragma once

#include "../common.hpp"

#include "range.hpp"

#if !defined LSTD_NO_CRT
#include <algorithm>
#endif

LSTD_BEGIN_NAMESPACE

template <typename T, size_t Size>
struct array {
    using data_t = T;

    data_t Data[Size];
    static constexpr size_t Count = Size;

    using iterator = data_t*;
    using const_iterator = const data_t*;

    constexpr iterator begin();
    constexpr iterator end();
    constexpr const_iterator begin() const;
    constexpr const_iterator end() const;

    // Find the index of the first occuring _item_ in the array, npos if it's not found
    constexpr size_t find(const data_t& item) const;

    // Find the index of the last occuring _item_ in the array, npos if it's not found
    constexpr size_t find_reverse(const data_t& item) const;

#if !defined LSTD_NO_CRT
    constexpr void sort();

    template <typename Pred>
    constexpr void sort(Pred&& predicate);
#endif

    constexpr bool has(const data_t& item) const;

    constexpr data_t& get(size_t index);
    constexpr const data_t& get(size_t index) const;

    constexpr data_t& operator[](size_t index);
    constexpr const data_t& operator[](size_t index) const;

    constexpr bool operator==(const array& other);
    constexpr bool operator!=(const array& other);
};

template <typename T, size_t Size>
constexpr typename array<T, Size>::iterator array<T, Size>::begin() {
    return Data;
}

template <typename T, size_t Size>
constexpr typename array<T, Size>::iterator array<T, Size>::end() {
    return Data + Count;
}

template <typename T, size_t Size>
constexpr typename array<T, Size>::const_iterator array<T, Size>::begin() const {
    return Data;
}

template <typename T, size_t Size>
constexpr typename array<T, Size>::const_iterator array<T, Size>::end() const {
    return Data + Count;
}

template <typename T, size_t Size>
constexpr size_t array<T, Size>::find(const data_t& item) const {
    const data_t* index = Data;
    For(range(Count)) {
        if (*index++ == item) {
            return it;
        }
    }
    return npos;
}

template <typename T, size_t Size>
constexpr size_t array<T, Size>::find_reverse(const data_t& item) const {
    const data_t* index = Data + Count - 1;
    For(range(Count)) {
        if (*index-- == item) {
            return Count - it - 1;
        }
    }
    return npos;
}

template <typename T, size_t Size>
constexpr void array<T, Size>::sort() {
    std::sort(begin(), end());
}

template <typename T, size_t Size>
template <typename Pred>
constexpr void array<T, Size>::sort(Pred&& predicate) {
    std::sort(begin(), end(), predicate);
}

template <typename T, size_t Size>
constexpr bool array<T, Size>::has(const data_t& item) const {
    return find(item) != npos;
}

template <typename T, size_t Size>
constexpr typename array<T, Size>::data_t& array<T, Size>::get(size_t index) {
    return Data[index];
}

template <typename T, size_t Size>
constexpr const typename array<T, Size>::data_t& array<T, Size>::get(size_t index) const {
    return Data[index];
}

template <typename T, size_t Size>
constexpr typename array<T, Size>::data_t& array<T, Size>::operator[](size_t index) {
    return get(index);
}

template <typename T, size_t Size>
constexpr const typename array<T, Size>::data_t& array<T, Size>::operator[](size_t index) const {
    return get(index);
}

template <typename T, size_t Size>
constexpr bool array<T, Size>::operator==(const array& other) {
    if (Count != other.Count) return false;
    For(range(Count)) {
        if (Data[it] != other.Data[it]) {
            return false;
        }
    }
    return true;
}

template <typename T, size_t Size>
constexpr bool array<T, Size>::operator!=(const array& other) {
    return !(*this == other);
}

template <typename... T>
constexpr auto to_array(T&&... values) -> array<std::decay_t<std::common_type_t<T...>>, sizeof...(T)> {
    return array<std::decay_t<std::common_type_t<T...>>, sizeof...(T)>{std::forward<T>(values)...};
}

LSTD_END_NAMESPACE