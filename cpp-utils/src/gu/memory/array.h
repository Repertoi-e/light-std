#pragma once

#include "../common.h"

GU_BEGIN_NAMESPACE

template <typename T, size_t size>
struct Array {
    T Data[size];
    static constexpr size_t Count = size;

    constexpr T &operator[](size_t index) { return Data[index]; }

    constexpr bool operator==(Array const &other) {
        if (Count != other.Count) return false;
        for (size_t i = 0; i < Count; i++) {
            if (Data[i] != other.Data[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr bool operator!=(Array const &other) { return !(*this == other); }
};

template <typename T, size_t size>
inline constexpr T *begin(Array<T, size> &array) {
    return array.Data;
}

template <typename T, size_t size>
inline constexpr T *end(Array<T, size> &array) {
    return array.Data + array.Count;
}

GU_END_NAMESPACE