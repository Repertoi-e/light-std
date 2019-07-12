#pragma once

#include "format_decimal.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {
template <u32 BASE_BITS, typename UInt>
inline byte *format_uint_base(byte *buffer, UInt value, size_t formattedSize, bool upper = false) {
    buffer += formattedSize;
    do {
        const byte *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
        u32 digit = (value & ((1 << BASE_BITS) - 1));
        *--buffer = (byte)(BASE_BITS < 4 ? (byte)('0' + digit) : digits[digit]);
    } while ((value >>= BASE_BITS) != 0);
    return buffer;
}

// Formats a unsigned integer into _buffer_ (in a given base).
template <u32 BASE_BITS, size_t N, typename UInt>
inline void format_uint_base(stack_dynamic_buffer<N> *buffer, UInt value, size_t formattedSize, bool upper = false) {
    auto maxSize = numeric_info<UInt>::digits / BASE_BITS + 2;
    byte formatBuffer[maxSize];
    auto *formatted = format_uint_base<BASE_BITS>(formatBuffer, value, formattedSize, upper);
    buffer->append_pointer_and_size(formatted, formatBuffer + formattedSize - formatted);
}
}  // namespace fmt

LSTD_END_NAMESPACE