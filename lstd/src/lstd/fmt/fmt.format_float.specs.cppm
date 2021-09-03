module;

#include "../memory/string_builder.h"

export module fmt.format_float.specs;

export import fmt.specs;
export import fmt.parse_context;

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

        bool ShowPoint;  // Whether to add a decimal point (even if no digits follow it)

        format Format;
        bool Upper;
    };

    fmt_float_specs fmt_parse_float_specs(fmt_parse_context * p, const fmt_specs &specs) {
        fmt_float_specs result;

        result.ShowPoint = specs.Hash;
        result.Upper     = false;

        switch (specs.Type) {
            case 0:
                result.Format = fmt_float_specs::GENERAL;
                // result.ShowPoint = true;  // :PythonLikeConsistency: See other note with this tag in fmt.context.ixx
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
                p->on_error("Invalid type specifier for a f32", p->It.Data - p->FormatString.Data - 1);
                break;
        }
        return result;
    }

    // Used to store a floating point number as F * pow(2, E), where F is the significand and E is the exponent.
    // Used by both Dragonbox and Grisu.
    template <types::is_floating_point F>
    struct decimal_fp {
        using significand_t = types::select_t<sizeof(F) == sizeof(f32), u32, u64>;

        significand_t Significand;
        s32 Exponent;
        s32 MantissaBit;  // Required by dragon4
    };

    using fp = decimal_fp<f64>;

    // Assigns _d_ to this and return true if predecessor is closer than successor (is the high margin twice as large as the low margin).
    template <types::is_floating_point F>
    bool fp_assign_new(fp & f, F newValue) {
        u64 implicitBit     = 1ull << numeric_info<F>::bits_mantissa;
        u64 significandMask = implicitBit - 1;

        u64 exponentMask = ((1ull << numeric_info<F>::bits_exponent) - 1) << numeric_info<F>::bits_mantissa;

        auto br = types::bit_cast<types::select_t<sizeof(F) == sizeof(f32), u32, u64>>(newValue);

        f.Significand = br & significandMask;
        s32 biasedExp = (s32) ((br & exponentMask) >> numeric_info<F>::bits_mantissa);

        // Predecessor is closer if _f_ is a normalized power of 2 (f.Significand == 0)
        // other than the smallest normalized number (biasedExp > 1).
        bool isPredecessorCloser = f.Significand == 0 && biasedExp > 1;

        if (biasedExp) {
            f.Significand += implicitBit;
            f.MantissaBit = numeric_info<F>::bits_mantissa;
        } else {
            biasedExp     = 1;                       // Subnormals use biased exponent 1 (min exponent).
            f.MantissaBit = msb(f.Significand | 1);  // Integer log2
        }
        f.Exponent = biasedExp - numeric_info<F>::exponent_bias - numeric_info<F>::bits_mantissa;

        return isPredecessorCloser;
    }

    // Normalizes the value converted from double and multiplied by (1 << SHIFT).
    template <s32 SHIFT>
    fp fp_normalize(fp value) {
        constexpr u64 IMPLICIT_BIT = 1ull << numeric_info<f64>::bits_mantissa;

        // Handle subnormals.
        u64 shifted_implicit_bit = IMPLICIT_BIT << SHIFT;
        while ((value.Significand & shifted_implicit_bit) == 0) {
            value.Significand <<= 1;
            --value.Exponent;
        }

        // Subtract 1 to account for hidden bit.
        s32 offset = (s32) ((sizeof(u64) * 8) - numeric_info<f64>::bits_mantissa - SHIFT - 1);
        value.Significand <<= offset;
        value.Exponent -= offset;
        return value;
    }

    always_inline fp operator*(fp x, fp y) {
        // Computes x.Significand * y.Significand / pow(2, 64) rounded to nearest with half-up tie breaking.
        u128 product = u128(x.Significand) * y.Significand;

        u64 f         = (u64) (product >> 64);
        x.Significand = ((u64) product & (1ull << 63)) != 0 ? f + 1 : f;

        x.Exponent += y.Exponent + 64;
        return x;
    }
}

LSTD_END_NAMESPACE
