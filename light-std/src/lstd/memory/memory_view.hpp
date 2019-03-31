#pragma once

#include "../string/string_utils.hpp"

LSTD_BEGIN_NAMESPACE

struct memory_view {
    const byte* Data = null;
    size_t ByteLength = 0;

    constexpr memory_view() = default;

    // Construct from null-terminated c-style string
    constexpr memory_view(const byte* str);
    constexpr memory_view(const byte* data, size_t byteLength);

    constexpr memory_view(const memory_view& other) = default;
    constexpr memory_view(memory_view&& other) = default;
    constexpr memory_view& operator=(const memory_view& other) = default;
    constexpr memory_view& operator=(memory_view&& other) = default;

    // A negative index mean from the end (Python-like)
    constexpr byte get(s64 index) const;

    // Negative index' mean from the end (Python-like)
    // Gets another Memory_View from this one [begin, end)
    constexpr memory_view subview(s64 begin, s64 end) const;

    // Find the first occurence of _b_
    size_t find(byte b) const;

    // Find the last occurence of _b_
    size_t find_reverse(byte b) const;

    constexpr s32 compare(const memory_view& other) const;

    constexpr const byte* begin() const;
    constexpr const byte* end() const;

    constexpr bool operator==(const memory_view& other) const;
    constexpr bool operator!=(const memory_view& other) const;
    constexpr bool operator<(const memory_view& other) const;
    constexpr bool operator>(const memory_view& other) const;
    constexpr bool operator<=(const memory_view& other) const;
    constexpr bool operator>=(const memory_view& other) const;

    operator bool() const;
    constexpr byte operator[](s64 index) const;

    constexpr void swap(memory_view& other);
};

constexpr memory_view::memory_view(const byte* str) : Data(str), ByteLength(cstring_strlen(str)) {}
constexpr memory_view::memory_view(const byte* data, size_t byteLength) : Data(data), ByteLength(byteLength) {}

constexpr byte memory_view::get(s64 index) const {
    size_t realIndex = translate_index(index, ByteLength);
    assert(realIndex < ByteLength);
    return Data[realIndex];
}

constexpr memory_view memory_view::subview(s64 begin, s64 end) const {
    // Convert to absolute [begin, end)
    size_t beginIndex = translate_index(begin, ByteLength);
    size_t endIndex = translate_index(end - 1, ByteLength) + 1;

    return memory_view(Data + beginIndex, endIndex - beginIndex);
}

inline size_t memory_view::find(byte b) const {
    assert(Data);
    for (size_t i = 0; i < ByteLength; ++i)
        if (get(i) == b) return i;
    return npos;
}

inline size_t memory_view::find_reverse(byte b) const {
    assert(Data);
    for (size_t i = ByteLength - 1; i >= 0; --i)
        if (get(i) == b) return i;
    return npos;
}

constexpr s32 memory_view::compare(const memory_view& other) const {
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

constexpr const byte* memory_view::begin() const { return Data; }
constexpr const byte* memory_view::end() const { return Data + ByteLength; }

constexpr bool memory_view::operator==(const memory_view& other) const { return compare(other) == 0; }
constexpr bool memory_view::operator!=(const memory_view& other) const { return !(*this == other); }
constexpr bool memory_view::operator<(const memory_view& other) const { return compare(other) < 0; }
constexpr bool memory_view::operator>(const memory_view& other) const { return compare(other) > 0; }
constexpr bool memory_view::operator<=(const memory_view& other) const { return !(*this > other); }
constexpr bool memory_view::operator>=(const memory_view& other) const { return !(*this < other); }
constexpr byte memory_view::operator[](s64 index) const { return get(index); }

inline memory_view::operator bool() const { return ByteLength != 0; }

constexpr void memory_view::swap(memory_view& other) {
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

LSTD_END_NAMESPACE
