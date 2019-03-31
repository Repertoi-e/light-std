#pragma once

#include "../context.hpp"
#include "../string/string_utils.hpp"

LSTD_BEGIN_NAMESPACE

namespace internal {

constexpr byte extract_partial_bits(byte value, u32 startBit, u32 bitsCount) {
    assert(startBit + bitsCount < 8);
    // shift extracted bits to the beginning of the byte
    byte t1 = value >> (8 - bitsCount - startBit);
    // mask out bits on the left
    byte t2 = t1 & ~(-1U << bitsCount);
    return t2;
}

constexpr byte extract_overlapping_bits(byte previous, byte next, u32 startBit, u32 bitsCount) {
    assert(startBit + bitsCount < 16);
    s32 bitsCountInNext = bitsCount - (8 - startBit);
    byte t1 = previous << bitsCountInNext;
    byte t2 = next >> (8 - bitsCountInNext) & ~(-1U << bitsCountInNext);
    return (t1 | t2) & ~(-1U << bitsCount);
}
}  // namespace internal

struct base_16 {
    static constexpr size_t GROUP_LENGTH = 4;

    static constexpr byte encode(u32 index) {
        const byte *dictionary = "0123456789ABCDEF";
        assert(index < cstring_strlen(dictionary));
        return dictionary[index];
    }

    static constexpr byte decode(byte c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }
};

struct base_32 {
    static constexpr size_t GROUP_LENGTH = 5;

    static constexpr byte encode(u32 index) {
        const byte *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        assert(index < cstring_strlen(dictionary));
        return dictionary[index];
    }

    static constexpr byte decode(byte c) {
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= '2' && c <= '7') return c - '2' + 26;
        return -1;
    }
};

struct base_64 {
    static constexpr size_t GROUP_LENGTH = 6;

    static constexpr byte encode(u32 index) {
        const byte *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        assert(index < cstring_strlen(dictionary));
        return dictionary[index];
    }

    static constexpr byte decode(byte c) {
        s32 alphabetLength = 26;
        if (c >= 'A' && c <= 'Z') return c - 'A';
        if (c >= 'a' && c <= 'z') return c - 'a' + alphabetLength * 1;
        if (c >= '0' && c <= '9') return c - '0' + alphabetLength * 2;
        if (c == '+') return c - '+' + alphabetLength * 2 + 10;
        if (c == '/') return c - '/' + alphabetLength * 2 + 11;
        return -1;
    }

    template <typename Traits>
    constexpr void decode(const byte *start, const byte *end, byte *out) {
        auto *iter = start;

        byte buffer = 0;

        s32 outputCurrentBit = 0;
        while (iter != end) {
            if (is_space(*iter)) {
                ++iter;
                continue;
            }
            byte value = Traits::decode(*iter);
            if (value == (byte) -1) {
                // malformed data, but let's go on...
                ++iter;
                continue;
            }
            u32 bits = min<size_t>(outputCurrentBit + Traits::GROUP_LENGTH, 8) - outputCurrentBit;
            if (bits == Traits::GROUP_LENGTH) {
                // the value fits within current byte, so we can extract it directly
                buffer |= value << (8 - outputCurrentBit - Traits::GROUP_LENGTH);
                outputCurrentBit += Traits::GROUP_LENGTH;
                // check if we filled up current byte completely; in such case we flush output and continue
                if (outputCurrentBit == 8) {
                    *out++ = buffer;
                    buffer = 0;
                    outputCurrentBit = 0;
                }
            } else {
                // the value spans across the current and the next byte
                s32 bits_in_next_byte = Traits::GROUP_LENGTH - bits;
                // fill the current byte and flush it to our output
                buffer |= value >> bits_in_next_byte;
                *out++ = buffer;
                buffer = 0;
                // save the remainder of our value in the buffer; it will be flushed
                // during next iterations
                buffer |= value << (8 - bits_in_next_byte);
                outputCurrentBit = bits_in_next_byte;
            }
            ++iter;
        }
    }

    template <typename Traits>
    constexpr void encode(const byte *start, const byte *end, byte *out) {
        auto *iter = start;

        bool hasBacklog = false;
        byte backlog = 0;
        s32 startBit = 0;
        while (hasBacklog || iter != end) {
            if (!hasBacklog) {
                if (startBit + Traits::GROUP_LENGTH < 8) {
                    // the value fits within single byte, so we can extract it
                    // directly
                    byte v = extract_partial_bits(*iter, startBit, Traits::GROUP_LENGTH);
                    *out++ = Traits::encode(v);
                    // since we know that startBit + Traits::GROUP_LENGTH < 8 we don't need to go
                    // to the next byte
                    startBit += Traits::GROUP_LENGTH;
                } else {
                    // our bits are spanning across byte border; we need to keep the
                    // starting point and move over to next byte.
                    backlog = *iter++;
                    hasBacklog = true;
                }
            } else {
                // encode value which is made from bits spanning across byte
                // boundary
                byte v;
                if (iter == end) {
                    v = extract_overlapping_bits(backlog, 0, startBit, Traits::GROUP_LENGTH);
                } else {
                    v = extract_overlapping_bits(backlog, *iter, startBit, Traits::GROUP_LENGTH);
                }
                *out++ = Traits::encode(v);
                hasBacklog = false;
                startBit = (startBit + Traits::GROUP_LENGTH) % 8;
            }
        }
    }

    LSTD_END_NAMESPACE
