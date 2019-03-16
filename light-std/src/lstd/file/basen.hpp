/**
 * base-n, 1.0
 * Copyright (C) 2012 Andrzej Zawadzki (azawadzki@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#pragma once

#include "../common.hpp"

#include "../string/string_utils.hpp"

namespace bn {

namespace impl {

const int ERROR = -1;

namespace {

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

}  // namespace

struct b16_conversion_traits {
    static constexpr size_t GROUP_LENGTH = 4;

    static constexpr byte encode(u32 index) {
        const char *const dictionary = "0123456789ABCDEF";
        assert(index < cstring_strlen((byte *) dictionary));
        return (byte) dictionary[index];
    }

    static constexpr byte decode(byte c) {
        if (c >= '0' && c <= '9') {
            return c - '0';
        } else if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        return ERROR;
    }
};

struct b32_conversion_traits {
    static constexpr size_t GROUP_LENGTH = 5;

    static constexpr byte encode(u32 index) {
        const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
        assert(index < cstring_strlen((byte *) dictionary));
        return (byte) dictionary[index];
    }

    static constexpr byte decode(byte c) {
        if (c >= 'A' && c <= 'Z') {
            return c - 'A';
        } else if (c >= '2' && c <= '7') {
            return c - '2' + 26;
        }
        return ERROR;
    }
};

struct b64_conversion_traits {
    static constexpr size_t GROUP_LENGTH = 6;

    static constexpr byte encode(u32 index) {
        const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        assert(index < cstring_strlen((byte *) dictionary));
        return (byte) dictionary[index];
    }

    static constexpr byte decode(byte c) {
        s32 alphabetLength = 26;
        if (c >= 'A' && c <= 'Z') {
            return c - 'A';
        } else if (c >= 'a' && c <= 'z') {
            return c - 'a' + alphabetLength * 1;
        } else if (c >= '0' && c <= '9') {
            return c - '0' + alphabetLength * 2;
        } else if (c == '+') {
            return c - '+' + alphabetLength * 2 + 10;
        } else if (c == '/') {
            return c - '/' + alphabetLength * 2 + 11;
        }
        return ERROR;
    }
};

template <class ConversionTraits, class Iter1, class Iter2>
constexpr void decode(Iter1 start, Iter1 end, Iter2 out) {
    Iter1 iter = start;

    byte buffer = 0;

    s32 outputCurrentBit = 0;
    while (iter != end) {
        if (std::isspace(*iter)) {
            ++iter;
            continue;
        }
        char value = ConversionTraits::decode(*iter);
        if (value == ERROR) {
            // malformed data, but let's go on...
            ++iter;
            continue;
        }
        u32 bits_in_current_byte = min(outputCurrentBit + ConversionTraits::GROUP_LENGTH, 8) - outputCurrentBit;
        if (bits_in_current_byte == ConversionTraits::GROUP_LENGTH) {
            // the value fits within current byte, so we can extract it directly
            buffer |= value << (8 - outputCurrentBit - ConversionTraits::GROUP_LENGTH);
            outputCurrentBit += ConversionTraits::GROUP_LENGTH;
            // check if we filled up current byte completely; in such case we flush output and continue
            if (outputCurrentBit == 8) {
                *out++ = buffer;
                buffer = 0;
                outputCurrentBit = 0;
            }
        } else {
            // the value spans across the current and the next byte
            int bits_in_next_byte = ConversionTraits::GROUP_LENGTH - bits_in_current_byte;
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

template <class ConversionTraits, class Iter1, class Iter2>
constexpr void encode(Iter1 start, Iter1 end, Iter2 out) {
    Iter1 iter = start;

    bool hasBacklog = false;
    byte backlog = 0;
    s32 startBit = 0;
    while (hasBacklog || iter != end) {
        if (!hasBacklog) {
            if (startBit + ConversionTraits::GROUP_LENGTH < 8) {
                // the value fits within single byte, so we can extract it
                // directly
                char v = extract_partial_bits(*iter, startBit, ConversionTraits::GROUP_LENGTH);
                *out++ = ConversionTraits::encode(v);
                // since we know that startBit + ConversionTraits::GROUP_LENGTH < 8 we don't need to go
                // to the next byte
                startBit += ConversionTraits::GROUP_LENGTH;
            } else {
                // our bits are spanning across byte border; we need to keep the
                // starting point and move over to next byte.
                backlog = *iter++;
                hasBacklog = true;
            }
        } else {
            // encode value which is made from bits spanning across byte
            // boundary
            char v;
            if (iter == end) {
                v = extract_overlapping_bits(backlog, 0, startBit, ConversionTraits::GROUP_LENGTH);
            } else {
                v = extract_overlapping_bits(backlog, *iter, startBit, ConversionTraits::GROUP_LENGTH);
            }
            *out++ = ConversionTraits::encode(v);
            hasBacklog = false;
            startBit = (startBit + ConversionTraits::GROUP_LENGTH) % 8;
        }
    }
}

}  // namespace impl

using namespace bn::impl;

template <class Iter1, class Iter2>
constexpr void encode_b16(Iter1 start, Iter1 end, Iter2 out) {
    encode<b16_conversion_traits>(start, end, out);
}

template <class Iter1, class Iter2>
constexpr void encode_b32(Iter1 start, Iter1 end, Iter2 out) {
    encode<b32_conversion_traits>(start, end, out);
}

template <class Iter1, class Iter2>
constexpr void encode_b64(Iter1 start, Iter1 end, Iter2 out) {
    encode<b64_conversion_traits>(start, end, out);
}

template <class Iter1, class Iter2>
constexpr void decode_b16(Iter1 start, Iter1 end, Iter2 out) {
    decode<b16_conversion_traits>(start, end, out);
}

template <class Iter1, class Iter2>
constexpr void decode_b32(Iter1 start, Iter1 end, Iter2 out) {
    decode<b32_conversion_traits>(start, end, out);
}

template <class Iter1, class Iter2>
constexpr void decode_b64(Iter1 start, Iter1 end, Iter2 out) {
    decode<b64_conversion_traits>(start, end, out);
}

}  // namespace bn
