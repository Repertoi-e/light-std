#pragma once

#include "../../storage/stack_dynamic_buffer.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

namespace internal {
inline byte g_DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";
}

using thousands_sep_t = void (*)(byte **buffer);

template <typename UInt>
inline byte *format_uint_decimal(byte *buffer, UInt value, size_t formattedSize, string_view thousandsSep = "") {
    u32 digitIndex = 0;

    buffer += formattedSize;
    while (value >= 100) {
        u32 index = (u32)(value % 100) * 2;
        value /= 100;
        *--buffer = (byte) internal::g_DIGITS[index + 1];
        if (++digitIndex % 3 == 0) {
            buffer -= thousandsSep.ByteLength;
            copy_memory(buffer, thousandsSep.Data, thousandsSep.ByteLength);
        }
        *--buffer = (byte) internal::g_DIGITS[index];
        if (++digitIndex % 3 == 0) {
            buffer -= thousandsSep.ByteLength;
            copy_memory(buffer, thousandsSep.Data, thousandsSep.ByteLength);
        }
    }

    if (value < 10) {
        *--buffer = (byte)('0' + value);
        return buffer;
    }

    u32 index = (u32) value * 2;
    *--buffer = (byte) internal::g_DIGITS[index + 1];
    if (++digitIndex % 3 == 0) {
        buffer -= thousandsSep.ByteLength;
        copy_memory(buffer, thousandsSep.Data, thousandsSep.ByteLength);
    }
    *--buffer = (byte) internal::g_DIGITS[index];

    return buffer;
}

// Formats a decimal unsigned integer into _buffer_.
// _thousandsSepFunc_ is a function that is called to add a thousands separator when necessary.
template <size_t N, typename UInt>
inline void format_uint_decimal(stack_dynamic_buffer<N> *buffer, UInt value, size_t formattedSize,
                                string_view thousandsSep = "") {
    auto maxSize = numeric_info<UInt>::digits10 + 1;
    byte formatBuffer[maxSize + maxSize / 3];
    auto *formatted = format_uint_decimal(formatBuffer, value, formattedSize, thousandsSep);
    buffer->append_pointer_and_size(formatted, formatBuffer + formattedSize - formatted);
}

}  // namespace fmt

LSTD_END_NAMESPACE
