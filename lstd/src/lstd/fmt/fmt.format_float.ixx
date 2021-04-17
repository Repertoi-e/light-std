module;

#include "../memory/string_builder.h"

export module fmt.format_float;

export import fmt.format_float.specs;
import fmt.format_float.dragonbox;
import fmt.format_float.grisu;

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

// Formats value using a variation of the Fixed-Precision Positive
// Floating-Point Printout ((FPP)^2) algorithm by Steele & White:
// https://fmt.dev/papers/p372-steele.pdf.

/*
    void fallback_format(f64 d, s32 numDigits, bool binary32, buffer<char> &buf, int &exp10) {
        bigint numerator;    // 2 * R in (FPP)^2.
        bigint denominator;  // 2 * S in (FPP)^2.

        // lower and upper are differences between value and corresponding boundaries.
        bigint lower;             // (M^- in (FPP)^2).
        bigint upper_store;       // upper's value if different from lower.
        bigint *upper = nullptr;  // (M^+ in (FPP)^2).

        fp value;
        // Shift numerator and denominator by an extra bit or two (if lower boundary
        // is closer) to make lower and upper integers. This eliminates multiplication
        // by 2 during later computations.
         bool is_predecessor_closer =
            binary32 ? value.assign(static_cast<float>(d)) : value.assign(d);
        int shift            = is_predecessor_closer ? 2 : 1;
        uint64_t significand = value.f << shift;
        if (value.e >= 0) {
            numerator.assign(significand);
            numerator <<= value.e;
            lower.assign(1);
            lower <<= value.e;
            if (shift != 1) {
                upper_store.assign(1);
                upper_store <<= value.e + 1;
                upper = &upper_store;
            }
            denominator.assign_pow10(exp10);
            denominator <<= shift;
        } else if (exp10 < 0) {
            numerator.assign_pow10(-exp10);
            lower.assign(numerator);
            if (shift != 1) {
                upper_store.assign(numerator);
                upper_store <<= 1;
                upper = &upper_store;
            }
            numerator *= significand;
            denominator.assign(1);
            denominator <<= shift - value.e;
        } else {
            numerator.assign(significand);
            denominator.assign_pow10(exp10);
            denominator <<= shift - value.e;
            lower.assign(1);
            if (shift != 1) {
                upper_store.assign(1ULL << 1);
                upper = &upper_store;
            }
        }

        // Invariant: value == (numerator / denominator) * pow(10, exp10).
        if (numDigits < 0) {
            // Generate the shortest representation.
            if (!upper) upper = &lower;
            bool even  = (value.f & 1) == 0;
            num_digits = 0;
            char *data = buf.data();
            for (;;) {
                int digit = numerator.divmod_assign(denominator);
                bool low  = compare(numerator, lower) - even < 0;  // numerator <[=] lower.
                // numerator + upper >[=] pow10:
                bool high          = add_compare(numerator, *upper, denominator) + even > 0;
                data[num_digits++] = static_cast<char>('0' + digit);
                if (low || high) {
                    if (!low) {
                        ++data[num_digits - 1];
                    } else if (high) {
                        int result = add_compare(numerator, numerator, denominator);
                        // Round half to even.
                        if (result > 0 || (result == 0 && (digit % 2) != 0))
                            ++data[num_digits - 1];
                    }
                    buf.try_resize(to_unsigned(num_digits));
                    exp10 -= num_digits - 1;
                    return;
                }
                numerator *= 10;
                lower *= 10;
                if (upper != &lower) *upper *= 10;
            }
        }
        // Generate the given number of digits.
        exp10 -= num_digits - 1;
        if (num_digits == 0) {
            buf.try_resize(1);
            denominator *= 10;
            buf[0] = add_compare(numerator, numerator, denominator) > 0 ? '1' : '0';
            return;
        }
        buf.try_resize(to_unsigned(num_digits));
        for (int i = 0; i < num_digits - 1; ++i) {
            int digit = numerator.divmod_assign(denominator);
            buf[i]    = static_cast<char>('0' + digit);
            numerator *= 10;
        }
        int digit   = numerator.divmod_assign(denominator);
        auto result = add_compare(numerator, numerator, denominator);
        if (result > 0 || (result == 0 && (digit % 2) != 0)) {
            if (digit == 9) {
                 auto overflow = '0' + 10;
                buf[num_digits - 1] = overflow;
                // Propagate the carry.
                for (int i = num_digits - 1; i > 0 && buf[i] == overflow; --i) {
                    buf[i] = '0';
                    ++buf[i - 1];
                }
                if (buf[0] == overflow) {
                    buf[0] = '1';
                    ++exp10;
                }
                return;
            }
            ++digit;
        }
        buf[num_digits - 1] = static_cast<char>('0' + digit);
    }*/

export s32 fmt_format_non_negative_float(string_builder &floatBuffer, types::is_floating_point auto value, s32 precision, const fmt_float_specs &specs) {
    assert(value >= 0);

    bool fixed = specs.Format == fmt_float_specs::FIXED;
    if (value == 0) {
        if (precision <= 0 || !fixed) {
            string_append(floatBuffer, U'0');
            return 0;
        }

        // @Speed
        For(range(precision)) {
            string_append(floatBuffer, U'0');
        }
        return -precision;
    }

    // If precision is still -1 (because no precision and no spec type was specified), use Dragonbox for the shortest format.
    // We set a default precision to 6 before calling this routine in the cases when the format string didn't specify a precision,
    // but specified a specific format (GENERAL, EXP, FIXED, etc.)
    if (precision < 0) {
        auto dec = dragonbox_to_decimal(value);
        string_append_u64(floatBuffer, dec.Significand);
        return dec.Exponent;
    }

    return grisu_to_decimal(floatBuffer, (f64) value, precision, specs);
}

LSTD_END_NAMESPACE
