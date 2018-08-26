#pragma once

#include "../common.h"

GU_BEGIN_NAMESPACE

template <typename T, size_t size>
struct Array {
    T Data[size];
    static constexpr size_t Count = size;

    constexpr T& operator[](size_t index) { return Data[index]; }
};

template <typename T, size_t size>
inline constexpr T* begin(Array<T, size>& array) {
    return array.Data;
}

template <typename T, size_t size>
inline constexpr T* end(Array<T, size>& array) {
    return array.Data + array.Count;
}

GU_END_NAMESPACE