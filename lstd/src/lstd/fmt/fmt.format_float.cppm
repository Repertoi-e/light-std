module;

#include "../common.h"

export module lstd.fmt.format_float;

export import lstd.fmt.format_float.specs;
import lstd.fmt.format_float.dragonbox;
import lstd.fmt.format_float.grisu;

LSTD_BEGIN_NAMESPACE

void append_u64(string_builder *builder, u64 value) {
    constexpr s32 BUFFER_SIZE = numeric_info<u64>::digits10;
    char buffer[BUFFER_SIZE];

    auto *p = buffer + BUFFER_SIZE - 1;

    if (!value) {
        *p-- = '0';
    }

    while (value) {
        auto d = value % 10;
        *p--   = (char) ('0' + d);
        value /= 10;
    }

    ++p;  // Roll back
    append(builder, p, buffer + BUFFER_SIZE - p);
}

// The returned exponent is the exponent base 10 of the LAST written digit in _floatBuffer_.
// In the end, _floatBuffer_ contains the digits of the final number to be written out, without the dot.
export s32 fmt_format_non_negative_float(string_builder *floatBuffer, types::is_floating_point auto value, s32 precision, const fmt_float_specs &specs) {
    assert(value >= 0);

    bool fixed = specs.Format == fmt_float_specs::FIXED;
    if (value == 0) {
        if (precision <= 0 || !fixed) {
            append(floatBuffer, U'0');
            return 0;
        }

        // @Speed
        For(range(precision)) {
            append(floatBuffer, U'0');
        }
        return -precision;
    }

    // If precision is still -1 (because no precision and no spec type was specified), use Dragonbox for the shortest format.
    // We set a default precision to 6 before calling this routine in the cases when the format string didn't specify a precision,
    // but specified a specific format (GENERAL, EXP, FIXED, etc.)
    if (precision < 0) {
        auto dec = dragonbox_format_float(value);
        append_u64(floatBuffer, dec.Significand);
        return dec.Exponent;
    }

    return grisu_format_float(floatBuffer, value, precision, specs);
}

LSTD_END_NAMESPACE
