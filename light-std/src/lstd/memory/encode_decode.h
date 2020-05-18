#pragma once

/// Provides encode/decode functions for base 16, 32 and 64

//
// e.g usage: encode<base_32>(...)
//

#include "../internal/context.h"
#include "../memory/string_utils.h"

LSTD_BEGIN_NAMESPACE

namespace internal {

constexpr char extract_partial_bits(char value, u32 startBit, u32 bitsCount) {
    assert(startBit + bitsCount < 8);
    char t1 = value >> (8 - bitsCount - startBit);
    char t2 = t1 & ~((u32)(-1) << bitsCount);
    return t2;
}

constexpr char extract_overlapping_bits(char previous, char next, u32 startBit, u32 bitsCount) {
    assert(startBit + bitsCount < 16);
    s32 bitsCountInNext = bitsCount - (8 - startBit);
    char t1 = previous << bitsCountInNext;
    char t2 = next >> (8 - bitsCountInNext) & ~((u32) -1 << bitsCountInNext);
    return (t1 | t2) & ~((u32) -1 << bitsCount);
}
}  // namespace internal

struct base_16 {
    static constexpr size_t GROUP_LENGTH = 4;

    static constexpr char encode(u32 index) {
        const char *dictionary = "0123456789ABCDEF";
        assert(index < c_string_length(dictionary));
        return dictionary[index];
    }

    static constexpr char decode(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }
};

struct base_32 {
    static constexpr size_t GROUP_LENGTH = 5;

    static constexpr char encode(u32 index) {
        const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        assert(index < c_string_length(dictionary));
        return dictionary[index];
    }

    static constexpr char decode(char c) {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= '2' && c <= '7') return c - '2' + 26;
        return -1;
    }
};

struct base_64 {
    static constexpr size_t GROUP_LENGTH = 6;

    static constexpr char encode(u32 index) {
        const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        assert(index < c_string_length(dictionary));
        return dictionary[index];
    }

    static constexpr char decode(char c) {
        s32 alphabetLength = 26;
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + alphabetLength * 1;
        if (c >= '0' && c <= '9') return c - '0' + alphabetLength * 2;
        if (c == '+') return c - '+' + alphabetLength * 2 + 10;
        if (c == '/') return c - '/' + alphabetLength * 2 + 11;
        return -1;
    }
};

template <typename Traits>
constexpr void decode(const char *start, const char *end, char *out) {
    auto *begin = out;
    auto *iter = start;

    char buffer = 0;

    s32 outputCurrentBit = 0;
    while (iter != end) {
        if (is_space(*iter)) {
            ++iter;
            continue;
        }
        char value = Traits::decode(*iter);
        if (value == (char) -1) {
            // Malformed data, but let's go on...
            ++iter;
            continue;
        }
        u32 bits = min<size_t>(outputCurrentBit + Traits::GROUP_LENGTH, 8) - outputCurrentBit;
        if (bits == Traits::GROUP_LENGTH) {
            // The value fits within current byte, so we can extract it directly.
            buffer |= value << (8 - outputCurrentBit - Traits::GROUP_LENGTH);
            outputCurrentBit += Traits::GROUP_LENGTH;
            // Check if we filled up current byte completely.
            // In such case we flush output and continue.
            if (outputCurrentBit == 8) {
                *out++ = buffer;
                buffer = 0;
                outputCurrentBit = 0;
            }
        } else {
            // The value spans across the current and the next byte.
            s32 bits_in_next_byte = Traits::GROUP_LENGTH - bits;
            // Fill the current byte and flush it to our output.
            buffer |= value >> bits_in_next_byte;
            *out++ = buffer;
            buffer = 0;
            // Save the remainder of our value in the buffer.
            // It will be flushed during next iterations.
            buffer |= value << (8 - bits_in_next_byte);
            outputCurrentBit = bits_in_next_byte;
        }
        ++iter;
    }
    return out - begin;
}

template <typename Traits>
constexpr size_t encode(const char *start, const char *end, char *out) {
    auto *begin = out;
    auto *iter = start;

    bool hasBacklog = false;
    char backlog = 0;
    s32 startBit = 0;
    while (hasBacklog || iter != end) {
        if (!hasBacklog) {
            if (startBit + Traits::GROUP_LENGTH < 8) {
                // The value fits within single byte, so we can extract it directly.
                char v = internal::extract_partial_bits(*iter, startBit, Traits::GROUP_LENGTH);
                *out++ = Traits::encode(v);
                // Since we know that startBit + Traits::GROUP_LENGTH < 8 we don't need to go to the next byte.
                startBit += Traits::GROUP_LENGTH;
            } else {
                // Our bits are spanning across byte border.
                // We need to keep the starting point and move over to next byte.
                backlog = *iter++;
                hasBacklog = true;
            }
        } else {
            // Encode value which is made from bits spanning across byte boundary.
            char v;
            if (iter == end) {
                v = internal::extract_overlapping_bits(backlog, 0, startBit, Traits::GROUP_LENGTH);
            } else {
                v = internal::extract_overlapping_bits(backlog, *iter, startBit, Traits::GROUP_LENGTH);
            }
            *out++ = Traits::encode(v);
            hasBacklog = false;
            startBit = (startBit + Traits::GROUP_LENGTH) % 8;
        }
    }
    return out - begin;
}

LSTD_END_NAMESPACE
