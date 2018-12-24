#pragma once

#include "../common.hpp"

#include "../io/writer.hpp"

CPPU_BEGIN_NAMESPACE

namespace fmt::internal {

const char DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

// A functor that doesn't add a thousands separator.
struct No_Thousands_Separator {
    void operator()(char *) {}
};

// A functor that adds a thousands separator.
struct Add_Thousands_Separator {
    string_view Separator;

    // Index of a decimal digit with the least significant digit having index 0.
    u32 DigitIndex = 0;

    explicit Add_Thousands_Separator(const string_view &separator) : Separator(separator) {}

    void operator()(char *&buffer) {
        if (++DigitIndex % 3 != 0) return;
        // TODO: "Danger danger, but who cares"
        buffer -= Separator.ByteLength;
        copy_memory(buffer, Separator.Data, Separator.ByteLength);
    }
};

// Formats a decimal unsigned integer value writing into buffer.
template <typename UInt, typename TS = No_Thousands_Separator>
char *format_uint_to_buffer(char *buffer, UInt value, u32 numDigits, TS thousandsSep = {}) {
    buffer += numDigits;
    char *end = buffer;
    while (value >= 100) {
        // Integer division is slow so do it for a group of two digits instead
        // of for every digit. The idea comes from the talk by Alexandrescu
        // "Three Optimization Tips for C++". See speed-test for a comparison.
        u32 index = (u32)((value % 100) * 2);
        value /= 100;
        *--buffer = DIGITS[index + 1];
        thousandsSep(buffer);
        *--buffer = DIGITS[index];
        thousandsSep(buffer);
    }
    if (value < 10) {
        *--buffer = (char) ('0' + value);
        return end;
    }
    u32 index = (u32)(value * 2);
    *--buffer = DIGITS[index + 1];
    thousandsSep(buffer);
    *--buffer = DIGITS[index];
    return end;
}

template <typename UInt, typename TS = No_Thousands_Separator>
void format_uint(io::Writer &out, UInt value, u32 numDigits, TS thousandsSep = {}) {
    // Buffer should be large enough to hold all digits (<= digits10 + 1)
    const size_t maxSize = std::numeric_limits<UInt>::digits10 + 1;
    char buffer[maxSize + maxSize / 3];
    format_uint_to_buffer(buffer, value, numDigits, thousandsSep);
    out.write(buffer, numDigits);
}

// Format with a base different from base 10
template <u32 BaseBits, typename UInt>
char *format_uint_to_buffer(char *buffer, UInt value, u32 numDigits, bool upper = false) {
    buffer += numDigits;
    char *end = buffer;
    do {
        const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        u32 digit = (value & ((1 << BaseBits) - 1));
        *--buffer = BaseBits < 4 ? (char) ('0' + digit) : digits[digit];
    } while ((value >>= BaseBits) != 0);
    return end;
}

// Format with a base different from base 10
template <unsigned BaseBits, typename UInt>
void format_uint(io::Writer &out, UInt value, u32 numDigits, bool upper = false) {
    // Buffer should be large enough to hold all digits (digits / BASE_BITS + 1) and null.
    char buffer[std::numeric_limits<UInt>::digits / BaseBits + 2];
    format_uint_to_buffer<BaseBits>(buffer, value, numDigits, upper);
    out.write(buffer, numDigits);
}
}  // namespace fmt::internal

CPPU_END_NAMESPACE
