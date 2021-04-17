#include "dragonbox.h"
#include "../memory/string_builder.h"

LSTD_BEGIN_NAMESPACE

void string_append_u64(string_builder &builder, u64 value) {
    constexpr s32 BUFFER_SIZE = numeric_info<u64>::digits10;
    utf8 buffer[BUFFER_SIZE];

    auto *p = buffer + BUFFER_SIZE - 1;

    if (!value) {
        *p-- = '0';
    }

    while (value) {
        auto d = value % 10;
        *p--   = (utf8)('0' + d);
        value /= 10;
    }

    ++p;  // Roll back
    string_append(builder, p, buffer + BUFFER_SIZE - p);
}

s32 to_decimal_impl(string_builder& builder, f64 value) {
    auto dec = jkj::dragonbox::to_decimal(value);
    string_append_u64(builder, dec.significand);
    return dec.exponent;
}

LSTD_END_NAMESPACE
