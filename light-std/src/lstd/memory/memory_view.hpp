#pragma once

#include "../string/string_utils.hpp"

LSTD_BEGIN_NAMESPACE

struct Memory_View {
    const byte *Data = null;
    size_t ByteLength = 0;

    constexpr Memory_View() {}
    constexpr Memory_View(const byte *data, size_t byteLength) : Data(data), ByteLength(byteLength) {}
    constexpr Memory_View(const char *data, size_t byteLength) : Data((const byte *) data), ByteLength(byteLength) {}

    // A negative index mean from the end (Python-like)
    constexpr byte get(s64 index) const {
        size_t realIndex = translate_index(index, ByteLength);
        assert(realIndex < ByteLength);
        return Data[realIndex];
    }

    // Negative index' mean from the end (Python-like)
    // Gets another Memory_View from this one [begin, end)
    constexpr Memory_View subview(s64 begin, s64 end) {
        // Convert to absolute [begin, end)
        size_t beginIndex = translate_index(begin, ByteLength);
        size_t endIndex = translate_index(end - 1, ByteLength) + 1;

        return Memory_View(Data + beginIndex, endIndex - beginIndex);
    }

    // Find the first occurence of _b_
    size_t find(byte b) const {
        assert(Data);
        for (size_t i = 0; i < ByteLength; ++i)
            if (get(i) == b) return i;
        return npos;
    }

    // Find the last occurence of _b_
    size_t find_reverse(byte b) const {
        assert(Data);
        for (size_t i = ByteLength - 1; i >= 0; --i)
            if (get(i) == b) return i;
        return npos;
    }

    constexpr s32 compare(const Memory_View &other) const {
        if (Data == other.Data && ByteLength == other.ByteLength) return 0;

        if (ByteLength == 0 && other.ByteLength == 0) return 0;
        if (ByteLength == 0) return -((s32) other.get(0));
        if (other.ByteLength == 0) return get(0);

        auto s1 = begin(), s2 = other.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == other.end()) return 0;
            if (s1 == end()) return -((s32) other.get(0));
            if (s2 == other.end()) return get(0);
        }
        return ((s32) *s1 - (s32) *s2);
    }

    constexpr const byte *begin() const { return Data; }
    constexpr const byte *end() const { return Data + ByteLength; }

    constexpr bool operator==(const Memory_View &other) const { return compare(other) == 0; }
    constexpr bool operator!=(const Memory_View &other) const { return !(*this == other); }
    constexpr bool operator<(const Memory_View &other) const { return compare(other) < 0; }
    constexpr bool operator>(const Memory_View &other) const { return compare(other) > 0; }
    constexpr bool operator<=(const Memory_View &other) const { return !(*this > other); }
    constexpr bool operator>=(const Memory_View &other) const { return !(*this < other); }

    constexpr Memory_View(const Memory_View &other) = default;
    constexpr Memory_View(Memory_View &&other) = default;
    constexpr Memory_View &operator=(const Memory_View &other) = default;
    constexpr Memory_View &operator=(Memory_View &&other) = default;

    constexpr byte operator[](s64 index) const { return get(index); }

    operator bool() const { return ByteLength != 0; }

    constexpr void swap(Memory_View &other) {
        {
            auto temp = Data;
            Data = other.Data;
            other.Data = temp;
        }
        {
            auto temp = ByteLength;
            ByteLength = other.ByteLength;
            other.ByteLength = temp;
        }
    }
};
LSTD_END_NAMESPACE