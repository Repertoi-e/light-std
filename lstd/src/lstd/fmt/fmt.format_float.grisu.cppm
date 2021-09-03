module;

#include "../memory/string_builder.h"

export module fmt.format_float.grisu;

import fmt.format_float.specs;
import fmt.format_float.dragon4;

//
// Use Grisu + Dragon4 when formatting a float with a given precision:
// https://www.cs.tufts.edu/~nr/cs257/archive/florian-loitsch/printf.pdf.//
//

LSTD_BEGIN_NAMESPACE

// Normalized 64-bit significands of pow(10, k), for k = -348, -340, ..., 340.
constexpr u64 POW10_SIGNIFICANDS[] = {0xfa8fd5a0081c0288, 0xbaaee17fa23ebf76, 0x8b16fb203055ac76, 0xcf42894a5dce35ea, 0x9a6bb0aa55653b2d, 0xe61acf033d1a45df, 0xab70fe17c79ac6ca, 0xff77b1fcbebcdc4f, 0xbe5691ef416bd60c, 0x8dd01fad907ffc3c, 0xd3515c2831559a83, 0x9d71ac8fada6c9b5, 0xea9c227723ee8bcb, 0xaecc49914078536d, 0x823c12795db6ce57, 0xc21094364dfb5637, 0x9096ea6f3848984f, 0xd77485cb25823ac7, 0xa086cfcd97bf97f4, 0xef340a98172aace5, 0xb23867fb2a35b28e, 0x84c8d4dfd2c63f3b, 0xc5dd44271ad3cdba, 0x936b9fcebb25c996, 0xdbac6c247d62a584, 0xa3ab66580d5fdaf6, 0xf3e2f893dec3f126, 0xb5b5ada8aaff80b8, 0x87625f056c7c4a8b, 0xc9bcff6034c13053, 0x964e858c91ba2655, 0xdff9772470297ebd, 0xa6dfbd9fb8e5b88f, 0xf8a95fcf88747d94, 0xb94470938fa89bcf, 0x8a08f0f8bf0f156b, 0xcdb02555653131b6, 0x993fe2c6d07b7fac, 0xe45c10c42a2b3b06, 0xaa242499697392d3, 0xfd87b5f28300ca0e, 0xbce5086492111aeb, 0x8cbccc096f5088cc, 0xd1b71758e219652c, 0x9c40000000000000, 0xe8d4a51000000000, 0xad78ebc5ac620000, 0x813f3978f8940984, 0xc097ce7bc90715b3, 0x8f7e32ce7bea5c70, 0xd5d238a4abe98068, 0x9f4f2726179a2245, 0xed63a231d4c4fb27, 0xb0de65388cc8ada8, 0x83c7088e1aab65db, 0xc45d1df942711d9a, 0x924d692ca61be758, 0xda01ee641a708dea, 0xa26da3999aef774a, 0xf209787bb47d6b85, 0xb454e4a179dd1877, 0x865b86925b9bc5c2, 0xc83553c5c8965d3d, 0x952ab45cfa97a0b3, 0xde469fbd99a05fe3, 0xa59bc234db398c25, 0xf6c69a72a3989f5c, 0xb7dcbf5354e9bece, 0x88fcf317f22241e2, 0xcc20ce9bd35c78a5, 0x98165af37b2153df, 0xe2a0b5dc971f303a, 0xa8d9d1535ce3b396, 0xfb9b7cd9a4a7443c, 0xbb764c4ca7a44410, 0x8bab8eefb6409c1a, 0xd01fef10a657842c, 0x9b10a4e5e9913129, 0xe7109bfba19c0c9d, 0xac2820d9623bf429, 0x80444b5e7aa7cf85, 0xbf21e44003acdd2d, 0x8e679c2f5e44ff8f, 0xd433179d9c8cb841, 0x9e19db92b4e31ba9, 0xeb96bf6ebadf77d9, 0xaf87023b9bf0ee6b};

// Binary exponents of pow(10, k), for k = -348, -340, ..., 340, corresponding
// to significands above.
constexpr s16 POW10_EXPONENTS[] = {
    -1220, -1193, -1166, -1140, -1113, -1087, -1060, -1034, -1007, -980, -954, -927, -901, -874, -847, -821, -794, -768, -741, -715, -688, -661, -635, -608, -582, -555, -529, -502, -475, -449, -422, -396, -369, -343, -316, -289, -263, -236, -210, -183, -157, -130, -103, -77, -50, -24, 3, 30, 56, 83, 109, 136, 162, 189, 216, 242, 269, 295, 322, 348, 375, 402, 428, 455, 481, 508, 534, 561, 588, 614, 641, 667, 694, 720, 747, 774, 800, 827, 853, 880, 907, 933, 960, 986, 1013, 1039, 1066};

// Returns a cached power of 10 'c_k = c_k.Significand * pow(2, c_k.Exponent)' such that its
// (binary) exponent satisfies 'minExponent <= c_k.Exponent <= minExponent + 28'.
fp get_cached_power(s32 minExponent, s32 *pow10) {
    constexpr s32 SHIFT       = 32;
    constexpr s64 SIGNIFICAND = (s64) 0x4d104d427de7fbcc;

    s32 index = (s32) (((minExponent + (sizeof(u64) * 8) - 1) * (SIGNIFICAND >> SHIFT) + ((1ll << SHIFT) - 1)) >> 32);

    // Decimal exponent of the first (smallest) cached power of 10.
    constexpr s32 FIRST_DEC_EXP = -348;
    constexpr s32 DEC_EXP_STEP  = 8;

    // Difference between 2 consecutive decimal exponents in cached powers of 10.
    index  = (index - FIRST_DEC_EXP - 1) / DEC_EXP_STEP + 1;
    *pow10 = FIRST_DEC_EXP + index * DEC_EXP_STEP;
    return {POW10_SIGNIFICANDS[index], POW10_EXPONENTS[index]};
}

enum class gen_digits_result {
    MORE,  // Generate more digits.
    DONE,  // Done generating digits.
    ERROR  // Digit generation cancelled due to an error.
};

struct gen_digits_state {
    utf8 *Buffer;
    s32 Size;

    bool Fixed;
    s32 Exp10;
    s32 Precision;
};

enum class round_direction { UNKNOWN,
                             UP,
                             DOWN };

// Given the divisor (normally a power of 10), the remainder = v % divisor for
// some number v and the error, returns whether v should be rounded up, down, or
// whether the rounding direction can't be determined due to error.
//
// _error_ should be less than divisor / 2.
always_inline round_direction get_round_direction(u64 divisor, u64 remainder, u64 error) {
    assert(remainder < divisor);      // divisor - remainder won't overflow.
    assert(error < divisor);          // divisor - error won't overflow.
    assert(error < divisor - error);  // error * 2 won't overflow.

    // Round down if (remainder + error) * 2 <= divisor.
    if (remainder <= divisor - remainder && error * 2 <= divisor - remainder * 2) return round_direction::DOWN;

    // Round up if (remainder - error) * 2 >= divisor.
    if (remainder >= error && remainder - error >= divisor - (remainder - error)) {
        return round_direction::UP;
    }
    return round_direction::UNKNOWN;
}

gen_digits_result gen_digits_on_start(gen_digits_state &state, u64 divisor, u64 remainder, u64 error, s32 exp) {
    // Non-fixed formats require at least one digit and no precision adjustment.
    if (!state.Fixed) return gen_digits_result::MORE;

    // Adjust fixed precision by exponent because it is relative to decimal point.
    state.Precision += exp + state.Exp10;

    // Check if precision is satisfied just by leading zeros, e.g.
    // "{:.2f}", 0.001 gives "0.00" without generating any digits.
    if (state.Precision > 0) return gen_digits_result::MORE;
    if (state.Precision < 0) return gen_digits_result::DONE;

    auto dir = get_round_direction(divisor, remainder, error);
    if (dir == round_direction::UNKNOWN) return gen_digits_result::ERROR;

    state.Buffer[state.Size++] = dir == round_direction::UP ? '1' : '0';
    return gen_digits_result::DONE;
}

gen_digits_result on_digit(gen_digits_state &state, char digit, u64 divisor, u64 remainder, u64 error, bool integral) {
    assert(remainder < divisor);

    state.Buffer[state.Size++] = digit;

    if (!integral && error >= remainder) return gen_digits_result::ERROR;
    if (state.Size < state.Precision) return gen_digits_result::MORE;

    if (!integral) {
        // Check if error * 2 < divisor with overflow prevention.
        // The check is not needed for the integral part because error = 1 and divisor > (1 << 32) there.
        if (error >= divisor || error >= divisor - error) return gen_digits_result::ERROR;
    } else {
        assert(error == 1 && divisor > 2);
    }

    auto dir = get_round_direction(divisor, remainder, error);
    if (dir != round_direction::UP) {
        return dir == round_direction::DOWN ? gen_digits_result::DONE : gen_digits_result::ERROR;
    }

    // Add one to the last digit, because we are rounding up
    state.Buffer[state.Size - 1] += 1;

    // Carry on with adding one to the previous digits if we have reached past 9
    for (s32 i = state.Size - 1; i > 0 && state.Buffer[i] > '9'; --i) {
        state.Buffer[i] = '0';
        state.Buffer[i - 1] += 1;
    }

    // If we have reached the beginning, transform 9 to 10 , e.g. 0.9 -> 1.0
    if (state.Buffer[0] > '9') {
        state.Buffer[0] = '1';
        if (state.Fixed) {
            state.Buffer[state.Size++] = '0';
        } else {
            ++state.Exp10;
        }
    }
    return gen_digits_result::DONE;
}

// Generates output using the Grisu digit-gen algorithm.
// error: the size of the region (lower, upper) outside of which numbers
// definitely do not round to value (Delta in Grisu3).
gen_digits_result gen_digits(gen_digits_state &state, fp value, u64 error, s32 *exp) {
    fp one = {1ull << -value.Exponent, value.Exponent};

    // The integral part of scaled value (p1 in Grisu) = value / one.
    // It cannot be zero because it contains a product of two 64-bit numbers with MSB set
    // (due to normalization) - 1, shifted right by at most 60 bits.
    auto integral = (u32) (value.Significand >> -one.Exponent);
    assert(integral != 0);
    assert(integral == value.Significand >> -one.Exponent);

    // The fractional part of scaled value (p2 in Grisu) c = value % one.
    u64 fractional = value.Significand & (one.Significand - 1);

    *exp = count_digits(integral);  // kappa in Grisu.

    // Divide by 10 to prevent overflow.
    auto result = gen_digits_on_start(state, POWERS_OF_10_64[*exp - 1] << -one.Exponent, value.Significand / 10, error * 10, *exp);
    if (result != gen_digits_result::MORE) return result;

    // Generate digits for the integral part. This can produce up to 10 digits.
    do {
        u32 digit = 0;

        auto divmod_integral = [&](u32 divisor) {
            digit = integral / divisor;
            integral %= divisor;
        };

        // This optimization by Milo Yip reduces the number of integer divisions by
        // one per iteration.
        switch (*exp) {
            case 10:
                divmod_integral(1000000000);
                break;
            case 9:
                divmod_integral(100000000);
                break;
            case 8:
                divmod_integral(10000000);
                break;
            case 7:
                divmod_integral(1000000);
                break;
            case 6:
                divmod_integral(100000);
                break;
            case 5:
                divmod_integral(10000);
                break;
            case 4:
                divmod_integral(1000);
                break;
            case 3:
                divmod_integral(100);
                break;
            case 2:
                divmod_integral(10);
                break;
            case 1:
                digit    = integral;
                integral = 0;
                break;
            default:
                assert(false && "Invalid number of digits");
        }

        --*exp;

        auto remainder = ((u64) integral << -one.Exponent) + fractional;

        result = on_digit(state, (char) ('0' + digit), POWERS_OF_10_64[*exp] << -one.Exponent, remainder, error, true);
        if (result != gen_digits_result::MORE) return result;
    } while (*exp > 0);

    // Generate digits for the fractional part.
    while (true) {
        fractional *= 10;
        error *= 10;

        char digit = (char) ('0' + (fractional >> -one.Exponent));
        fractional &= one.Significand - 1;
        --*exp;

        result = on_digit(state, digit, one.Significand, fractional, error, false);
        if (result != gen_digits_result::MORE) return result;
    }
}

// The returned exponent is the exponent base 10 of the LAST written digit in _floatBuffer_.
// In the end, _floatBuffer_ contains the digits of the final number to be written out, without the dot.
export s32 grisu_format_float(string_builder &floatBuffer, types::is_floating_point auto v, s32 precision, const fmt_float_specs &specs) {
    constexpr s32 MIN_EXP = -60;  // alpha in Grisu.

    fp normalized;
    fp_assign_new(normalized, v);
    normalized = fp_normalize<0>(normalized);

    s32 cachedExp10 = 0;  // K in Grisu.

    fp cachedPow = get_cached_power(MIN_EXP - (normalized.Exponent + (sizeof(u64) * 8)), &cachedExp10);
    normalized   = normalized * cachedPow;

    // Limit precision to the maximum possible number of significant digits in an
    // IEEE754 double because we don't need to generate zeros.
    constexpr s32 MAX_DOUBLE_DIGITS = 767;
    if (precision > MAX_DOUBLE_DIGITS) precision = MAX_DOUBLE_DIGITS;

    bool fixed = specs.Format == fmt_float_specs::FIXED;

    gen_digits_state state;
    state.Buffer    = floatBuffer.BaseBuffer.Data + floatBuffer.BaseBuffer.Occupied;
    state.Size      = 0;
    state.Fixed     = fixed;
    state.Exp10     = -cachedExp10;
    state.Precision = precision;

    s32 exp = 0;
    if (gen_digits(state, normalized, 1, &exp) == gen_digits_result::ERROR) {
        // On error we fallback to the dragon4 algorithm...
        exp += state.Size - cachedExp10 - 1;

        utf8 *buf = floatBuffer.BaseBuffer.Data + floatBuffer.BaseBuffer.Occupied;
        s64 written;

        // Here we pass handler.Precision, and not the raw precision we
        // are given when calling this function, because that one specifies
        // how many digits to print AFTER the decimal point.
        // Grisu updates it to include the stuff before that.
        // So essentially handler.Precision contains the number of digits
        // to be generated, and dragon4 expects that.
        dragon4_format_float(buf, &written, &exp, state.Precision, v);

        floatBuffer.BaseBuffer.Occupied += written;
    } else {
        // Success!
        exp += state.Exp10;
        floatBuffer.BaseBuffer.Occupied += state.Size;
    }

    // Remove trailing zeros after decimal point
    if (!fixed && !specs.ShowPoint) {
        s64 size = floatBuffer.BaseBuffer.Occupied;

        while (size > 0 && floatBuffer.BaseBuffer.Data[size - 1] == '0') {
            --size;
            ++exp;
        }
        floatBuffer.BaseBuffer.Occupied = size;
    }
    return exp;
}

LSTD_END_NAMESPACE
