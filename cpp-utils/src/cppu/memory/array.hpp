#pragma once

#include "../common.hpp"

#include "range.hpp"

#include <algorithm>

CPPU_BEGIN_NAMESPACE

template <typename T, size_t Size>
struct Array {
    T Data[Size];
    static constexpr size_t Count = Size;

    using iterator = T *;
    using const_iterator = const T *;

    constexpr iterator begin() { return Data; }
    constexpr iterator end() { return Data + Count; }
    constexpr const_iterator begin() const { return Data; }
    constexpr const_iterator end() const { return Data + Count; }

    // Find the index of the first occuring _item_ in the array, npos if it's not found
    constexpr size_t find(const T &item) const {
        const T *index = Data;
        for (auto i : range(Count)) {
            if (*index++ == item) {
                return i;
            }
        }
        return npos;
    }

    // Find the index of the last occuring _item_ in the array, npos if it's not found
    constexpr size_t find_last(const T &item) const {
        const T *index = Data + Count - 1;
        for (auto i : range(Count)) {
            if (*index-- == item) {
                return Count - i - 1;
            }
        }
        return npos;
    }

    constexpr void sort() { std::sort(begin(), end()); }
    template <typename Pred>

    constexpr void sort(Pred &&predicate) {
        std::sort(begin(), end(), predicate);
    }

    constexpr b32 has(const T &item) const { return find(item) != npos; }

    constexpr T &get(size_t index) { return Data[index]; }
    constexpr const T &get(size_t index) const { return Data[index]; }

    constexpr T &operator[](size_t index) { return get(index); }
    constexpr const T &operator[](size_t index) const { return get(index); }

    constexpr b32 operator==(const Array &other) {
        if (Count != other.Count) return false;
        for (size_t i = 0; i < Count; i++) {
            if (Data[i] != other.Data[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr b32 operator!=(const Array &other) { return !(*this == other); }
};

template <typename... T>
constexpr auto to_array(T &&... values)
    -> Array<typename std::decay_t<typename std::common_type_t<T...>>, sizeof...(T)> {
    return Array<typename std::decay_t<typename std::common_type_t<T...>>, sizeof...(T)>{std::forward<T>(values)...};
}

CPPU_END_NAMESPACE