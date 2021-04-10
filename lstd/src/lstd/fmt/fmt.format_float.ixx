module;

#include "../memory/string_builder.h"

export module fmt.format_float;

import fmt.specs;
import fmt.parse_context;

LSTD_BEGIN_NAMESPACE

export {
    // @Locale
    struct fmt_float_specs {
        enum format {
            GENERAL,  // General: chooses exponent notation or fixed point based on magnitude.
            EXP,      // Exponent notation with the default precision of 6, e.g. 1.2e-3.
            FIXED,    // Fixed point with the default precision of 6, e.g. 0.0012.
            HEX
        };

        s32 Precision;
        u32 Width;

        fmt_sign Sign;

        bool ShowPoint;  // Whether to add a decimal point (even if no digits follow it)

        format Format;
        bool Upper;
    };

    fmt_float_specs fmt_parse_float_specs(fmt_parse_context * p, const fmt_specs &specs) {
        fmt_float_specs result;

        result.Precision = specs.Precision;
        result.Width = specs.Width;

        result.ShowPoint = specs.Hash;
        result.Upper = false;

        switch (specs.Type) {
            case 0:
                result.Format = fmt_float_specs::GENERAL;
                break;
            case 'G':
                result.Upper = true;
                [[fallthrough]];
            case 'g':
                result.Format = fmt_float_specs::GENERAL;
                break;
            case 'E':
                result.Upper = true;
                [[fallthrough]];
            case 'e':
                result.Format = fmt_float_specs::EXP;
                result.ShowPoint |= specs.Precision != 0;
                break;
            case 'F':
                result.Upper = true;
                [[fallthrough]];
            case '%':  // When the spec is '%' we display the number with fixed format and multiply it by 100
                [[fallthrough]];
            case 'f':
                result.Format = fmt_float_specs::FIXED;
                result.ShowPoint |= specs.Precision != 0;
                break;
            case 'A':
                result.Upper = true;
                [[fallthrough]];
            case 'a':
                result.Format = fmt_float_specs::HEX;
                break;
            default:
                p->on_error("Invalid type specifier for a float", p->It.Data - p->FormatString.Data - 1);
                break;
        }
        return result;
    }

    namespace dragonbox {
    template <types::is_floating_point F>
    struct decimal_fp {
        using significand_t = types::select_t<sizeof(F) == 32, u32, u64>;

        significand_t Significand;
        s32 Exponent;
    };

    template <types::is_floating_point T>
    decimal_fp<T> to_decimal(T x) {
        /*
        // Step 1: integer promotion & Schubfach multiplier calculation.
        using uint_t = types::select_t<sizeof(F) == 32, u32, u64>;

        using cache_entry_type = typename cache_accessor<T>::cache_entry_type;
        auto br = bit_cast<uint_t>(x);

        // Extract significand bits and exponent bits.
        const uint_t significand_mask =
            (static_cast<uint_t>(1) << float_info<T>::significand_bits) - 1;
        uint_t significand = (br & significand_mask);
        int exponent = static_cast<int>((br & exponent_mask<T>()) >>
                                        float_info<T>::significand_bits);

        if (exponent != 0) {  // Check if normal.
            exponent += float_info<T>::exponent_bias - float_info<T>::significand_bits;

            // Shorter interval case; proceed like Schubfach.
            if (significand == 0) return shorter_interval_case<T>(exponent);

            significand |=
                (static_cast<uint_t>(1) << float_info<T>::significand_bits);
        } else {
            // Subnormal case; the interval is always regular.
            if (significand == 0) return {0, 0};
            exponent = float_info<T>::min_exponent - float_info<T>::significand_bits;
        }

        const bool include_left_endpoint = (significand % 2 == 0);
        const bool include_right_endpoint = include_left_endpoint;

        // Compute k and beta.
        const int minus_k = floor_log10_pow2(exponent) - float_info<T>::kappa;
        const cache_entry_type cache = cache_accessor<T>::get_cached_power(-minus_k);
        const int beta_minus_1 = exponent + floor_log2_pow10(-minus_k);

        // Compute zi and deltai
        // 10^kappa <= deltai < 10^(kappa + 1)
        const uint32_t deltai = cache_accessor<T>::compute_delta(cache, beta_minus_1);
        const uint_t two_fc = significand << 1;
        const uint_t two_fr = two_fc | 1;
        const uint_t zi =
            cache_accessor<T>::compute_mul(two_fr << beta_minus_1, cache);

        // Step 2: Try larger divisor; remove trailing zeros if necessary

        // Using an upper bound on zi, we might be able to optimize the division
        // better than the compiler; we are computing zi / big_divisor here
        decimal_fp<T> ret_value;
        ret_value.significand = divide_by_10_to_kappa_plus_1(zi);
        uint32_t r = static_cast<uint32_t>(zi - float_info<T>::big_divisor *
                                                    ret_value.significand);

        if (r > deltai) {
            goto small_divisor_case_label;
        } else if (r < deltai) {
            // Exclude the right endpoint if necessary
            if (r == 0 && !include_right_endpoint &&
                is_endpoint_integer<T>(two_fr, exponent, minus_k)) {
                --ret_value.significand;
                r = float_info<T>::big_divisor;
                goto small_divisor_case_label;
            }
        } else {
            // r == deltai; compare fractional parts
            // Check conditions in the order different from the paper
            // to take advantage of short-circuiting
            const uint_t two_fl = two_fc - 1;
            if ((!include_left_endpoint ||
                 !is_endpoint_integer<T>(two_fl, exponent, minus_k)) &&
                !cache_accessor<T>::compute_mul_parity(two_fl, cache, beta_minus_1)) {
                goto small_divisor_case_label;
            }
        }
        ret_value.exponent = minus_k + float_info<T>::kappa + 1;

        // We may need to remove trailing zeros
        ret_value.exponent += remove_trailing_zeros(ret_value.significand);
        return ret_value;

        // Step 3: Find the significand with the smaller divisor

    small_divisor_case_label:
        ret_value.significand *= 10;
        ret_value.exponent = minus_k + float_info<T>::kappa;

        const uint32_t mask = (1u << float_info<T>::kappa) - 1;
        auto dist = r - (deltai / 2) + (float_info<T>::small_divisor / 2);

        // Is dist divisible by 2^kappa?
        if ((dist & mask) == 0) {
            const bool approx_y_parity =
                ((dist ^ (float_info<T>::small_divisor / 2)) & 1) != 0;
            dist >>= float_info<T>::kappa;

            // Is dist divisible by 5^kappa?
            if (check_divisibility_and_divide_by_pow5<float_info<T>::kappa>(dist)) {
                ret_value.significand += dist;

                // Check z^(f) >= epsilon^(f)
                // We have either yi == zi - epsiloni or yi == (zi - epsiloni) - 1,
                // where yi == zi - epsiloni if and only if z^(f) >= epsilon^(f)
                // Since there are only 2 possibilities, we only need to care about the
                // parity. Also, zi and r should have the same parity since the divisor
                // is an even number
                if (cache_accessor<T>::compute_mul_parity(two_fc, cache, beta_minus_1) !=
                    approx_y_parity) {
                    --ret_value.significand;
                } else {
                    // If z^(f) >= epsilon^(f), we might have a tie
                    // when z^(f) == epsilon^(f), or equivalently, when y is an integer
                    if (is_center_integer<T>(two_fc, exponent, minus_k)) {
                        ret_value.significand = ret_value.significand % 2 == 0
                                                    ? ret_value.significand
                                                    : ret_value.significand - 1;
                    }
                }
            }
            // Is dist not divisible by 5^kappa?
            else {
                ret_value.significand += dist;
            }
        }
        // Is dist not divisible by 2^kappa?
        else {
            // Since we know dist is small, we might be able to optimize the division
            // better than the compiler; we are computing dist / small_divisor here
            ret_value.significand +=
                small_division_by_pow10<float_info<T>::kappa>(dist);
        }
        return ret_value;
        */
    }
    }  // namespace dragonbox

    template <types::is_floating_point T>
    s32 fmt_format_non_negative_float(string_builder & floatBuffer, T value, const fmt_float_specs &specs) {
        assert(value >= 0);

        s32 precision = specs.Precision;

        bool fixed = specs.Format == fmt_float_specs::FIXED;
        if (value == 0) {
            if (precision <= 0 || !fixed) {
                string_append(floatBuffer, '0');
                return 0;
            }

            // @Speed
            For(range(precision)) {
                string_append(floatBuffer, '0');
            }
            return -precision;
        }

        // If precision is still -1 (because no precision and no spec type was specified), use Dragonbox for the shortest format.
        // We set a default precision to 6 before calling this routine in the cases when the format string didn't specify a precision,
        // but specified a specific format (GENERAL, EXP, FIXED, etc.)
        if (precision < 0) {
            //dragonbox::decimal_fp dec = dragonbox::to_decimal(value);
            //string_append(floatBuffer, dec.Significand);
            //return dec.Exponent;
        }

        // Use Grisu + Dragon4 for the given precision:
        // https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf.
        // s32 exp = 0;
        // constexpr s32 min_exp = -60; // alpha in Grisu.
        // s32 cached_exp10 = 0;         // K in Grisu.
        //
        // fp normalized = normalize(fp(value));
        // const auto cached_pow = get_cached_power(
        //     min_exp - (normalized.e + fp::significand_size), cached_exp10);
        // normalized = normalized * cached_pow;
        // // Limit precision to the maximum possible number of significant digits in an
        // // IEEE754 double because we don't need to generate zeros.
        // const int max_double_digits = 767;
        // if (precision > max_double_digits) precision = max_double_digits;
        // fixed_handler handler{buf.data(), 0, precision, -cached_exp10, fixed};
        // if (grisu_gen_digits(normalized, 1, exp, handler) == digits::error) {
        //     exp += handler.size - cached_exp10 - 1;
        //     fallback_format(value, handler.precision, specs.binary32, buf, exp);
        // } else {
        //     exp += handler.exp10;
        //     buf.try_resize(to_unsigned(handler.size));
        // }
        // if (!fixed && !specs.showpoint) {
        //     // Remove trailing zeros.
        //     auto num_digits = buf.size();
        //     while (num_digits > 0 && buf[num_digits - 1] == '0') {
        //         --num_digits;
        //         ++exp;
        //     }
        //     buf.try_resize(num_digits);
        // }
        // return exp;
        return 0;
    }
}

LSTD_END_NAMESPACE
