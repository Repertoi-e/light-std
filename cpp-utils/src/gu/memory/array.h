#pragma once

#include "../common.h"

GU_BEGIN_NAMESPACE

template <typename T, size_t size>
struct Array {
    T Data[size];
    static constexpr size_t Count = size;

    constexpr T *begin() { return Data; }
    constexpr T *end() { return Data + Count; }
    constexpr const T *begin() const { return Data; }
    constexpr const T *end() const { return Data + Count; }

    constexpr T &operator[](size_t index) { return Data[index]; }

    constexpr bool operator==(const Array &other) {
        if (Count != other.Count) return false;
        for (size_t i = 0; i < Count; i++) {
            if (Data[i] != other.Data[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr bool operator!=(const Array &other) { return !(*this == other); }
};

GU_END_NAMESPACE