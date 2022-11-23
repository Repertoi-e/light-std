module;

#include "../common.h"

export module lstd.fmt.format_float.dragon4;

import lstd.fmt.format_float.specs;

import lstd.big_integer;
import lstd.string_builder;

//
// This is an implementation the Dragon4 algorithm to convert a binary number in
// floating point format to a decimal number in string format.
//
// See the following papers for more information on the algorithm:
//  "How to Print Floating-Point Numbers Accurately"
//    Steele and White
//    http://kurtstephens.com/files/p372-steele.pdf
//  "Printing Floating-Point Numbers Quickly and Accurately"
//    Burger and Dybvig
//    http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.72.4656
//
//
// It is a modified version of numpy's dragon4 implementation,
// ... which is a modification of Ryan Juckett's version.
//
// Here are the appropriate licenses:

/*
 * Copyright (c) 2014 Ryan Juckett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * Copyright (c) 2005-2021, NumPy Developers.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 * 
 *     * Neither the name of the NumPy Developers nor the names of any
 *        contributors may be used to endorse or promote products derived
 *        from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

LSTD_BEGIN_NAMESPACE

// Assigns pow(10, exp) to a big int
big_integer bigint_pow10(s32 exp) {
    assert(exp >= 0);
    if (exp == 0) return cast_big(1);

    // Find the top bit
    s32 bitmask = 1;
    while (exp >= bitmask) bitmask <<= 1;
    bitmask >>= 1;

    // pow(10, exp) = pow(5, exp) * pow(2, exp).
    // First compute pow(5, exp) by repeated squaring and multiplication.
    big_integer b = cast_big(5);
    bitmask >>= 1;
    while (bitmask != 0) {
        b = b * b;
        if ((exp & bitmask) != 0) b = b * cast_big(5);
        bitmask >>= 1;
    }
    b = b << exp;  // Multiply by pow(2, exp) by shifting.

    return b;
}

bool is_digit_in_valid_range(big_integer* b) {
    if (!b->Size) return true;
    return b->Size == 1 && get_digit(b, 0) < 10;
}

u32 get_digit_or_zero(big_integer* b) {
    if (b->Size) return get_digit(b, 0);
    return 0;
}

//
// -- The original comment from numpy's source:
// This implementation is essentially a port of the "Figure 3" Scheme code from
// Burger and Dybvig, but with the following additional differences:
//   1. Instead of finding the highest k such that high < B**k, we search
//      for the one where v < B**k. This has a downside that if a power
//      of 10 exists between v and high, we will output a 9 instead of a 1 as
//      first digit, violating the "no-carry" guarantee of the paper. This is
//      accounted for in a new post-processing loop which implements a carry
//      operation. The upside is one less BigInt multiplication.
//   2. The approximate value of k found is offset by a different amount
//      (0.69), in order to hit the "fast" branch more often. This is
//      extensively described on Ryan Juckett's website.
//   3. The fixed precision mode is much simpler than proposed in the paper.
//      It simply outputs digits by repeatedly dividing by 10. The new "carry"
//      loop at the end rounds this output nicely.
//  There is also some new code to account for details of the BigInt
//  implementation, which are not present in the paper since it does not specify
//  details of the integer calculations.
//
// There is some more documentation of these changes on Ryan Juckett's website
// at http://www.ryanjuckett.com/programming/printing-floating-point-numbers/
//
// This code also has a few implementation differences from Ryan Juckett's
// version:
//  1. fixed overflow problems when significand was 64 bits (in float128 types),   -- These are not relevant to lstd
//     by replacing multiplication by 2 or 4 by BigInt_ShiftLeft calls.            -- These are not relevant to lstd
//  2. Increased c_BigInt_MaxBlocks, for 128-bit floats                            -- These are not relevant to lstd
//  3. Added more entries to the g_PowerOf10_Big table, for 128-bit floats.        -- These are not relevant to lstd
//  4. Added unbiased rounding calculation with isEven. Ryan Juckett's
//     implementation did not implement "IEEE unbiased rounding", except in the
//     last digit. This has been added back, following the Burger & Dybvig
//     code, using the isEven variable.
//
// Arguments:
// _b_           - buffer, should be big enough to hold the output.
// _outWritten_  - contains the number of digits written.
// _outExp_      - the base 10 exponent of the last digit.
//               ^ This is different in our implementation, originally this returns the base10 exponent of the FIRST digit.
//                 But the rest of our formatting expects it to be in this format. It's a simple conversion to get the
//                 exponent of the first digit (if needed).
//
// _precision_   - print at least and at most this many digits; -1 for unspecified.
//                 Precision here means not how much to write after the decimal point,
//                 but how many digits in TOTAL.
//
// _v_           - f32 or f64; contains the value of the float to be formatted.
//
export void dragon4_format_float(char *b, s32 bSize, s64 *outWritten, s32 *outExp, s32 precision, is_floating_point auto v) {
    // floating point value above/below the significand.

    // Cap precision to not overflow _b_
    precision = min(precision, bSize);

    // Lower and upper are differences between value and corresponding boundaries.
    big_integer numerator, denominator, lower, upperStore;
    big_integer *upper = null;  // Optional, may point to upperStore.

    fp value;
    bool isPredecessorCloser = fp_assign_new(value, v);  // Called "hasUnequalMargins" in the original

    s32 shift = isPredecessorCloser ? 2 : 1;

    s32 exponent    = value.Exponent;
    u64 significand = value.Significand << shift;

    if (exponent >= 0) {
        // If we have no fractional component
        //
        // 1) Expand the input value by multiplying out the significand and
        //    exponent. This represents the input value in its whole number
        //    representation.
        // 2) Apply an additional scale of 2 such that later comparisons
        //    against the margin values are simplified.
        // 3) Set the margin value to the lowest significand bit's scale.

        numerator = cast_big(significand);
        numerator = numerator << exponent;
        lower     = cast_big(1);
        lower     = lower << exponent;
        if (shift != 1) {
            upperStore = cast_big(1);
            upperStore = upperStore << (exponent + 1);
            upper      = &upperStore;
        }
        denominator = bigint_pow10(*outExp);
        denominator = denominator << shift;
    } else if (*outExp < 0) {
        numerator = bigint_pow10(-(*outExp));
        lower     = numerator;
        if (shift != 1) {
            upperStore = numerator;
            upperStore = upperStore << 1;
            upper      = &upperStore;
        }
        numerator   = numerator * cast_big(significand);
        denominator = cast_big(1);
        denominator = denominator << (shift - exponent);
    } else {
        numerator   = cast_big(significand);
        denominator = bigint_pow10(*outExp);
        denominator = denominator << (shift - exponent);
        lower       = cast_big(1);
        if (shift != 1) {
            upperStore = cast_big(2);
            upper      = &upperStore;
        }
    }

    // Invariant: value == (numerator / denominator) * pow(10, outExp).
    if (precision < 0) {
        // Generate the shortest unique representation
        if (!upper) upper = &lower;
        bool even = (value.Significand & 1) == 0;

        precision = 0;
        while (true) {
            // Divide out the scale to extract the digit.
            auto [digit, mod] = divmod(numerator, denominator);
            numerator         = mod;

            assert(is_digit_in_valid_range(&digit));
            u32 outputDigit = get_digit_or_zero(&digit);

            bool low  = compare(numerator, lower) - even < 0;
            bool high = compare(numerator + *upper, denominator) + even > 0;

            // Store the output digit.
            b[precision++] = '0' + outputDigit;

            if (low || high) {
                if (low) {
                    ++b[precision - 1];
                } else if (high) {
                    s64 cmp = compare(numerator * cast_big(2), denominator);

                    // Round half to even.
                    if (cmp > 0 || (cmp == 0 && (outputDigit % 2) != 0)) {
                        ++b[precision - 1];
                    }
                }

                *outWritten = precision;
                *outExp -= precision - 1;

                return;
            }

            numerator = numerator * cast_big(10);
            lower     = lower * cast_big(10);
            if (upper != &lower) *upper = *upper * cast_big(10);
        }
    }

    //
    // If 0 precision, write out a 1 or 0.
    // 
    // This is not in the original algorithm.
    // We check for precision 0 here because we need to generate
    // a lonely 0 (or 1), otherwise our formatting looks incorrect.
    // 
    // Precisely, the format string "{:#.0f}" should produce "0." when given a value lower than 0.5.
    // Note that Python would round 0.5 to 0, and not to 1 like we do here. I'm not exactly sure
    // why its interpreter does that, however I dub our behaviour more correct.
    // 
    // When fed with e.g. 0.7, we take the outside branch and round up, and correctly produce "1.".
    // 
    // Side note about the formatting syntax:
    // If you are confused about the pointy dot produced in the final output: the # specifier tells the 
    // formatter to output a dot despite the fact that the specified precision is 0. This is useful when 
    // you want to be explicit that you are printing a floating point number, and not to be confused with 
    // an integer. For e.g. "{:.0f}" (without the hash) with the value 42.2 would print "42", which is 
    // indistinguishable from printing the integer 42.
    //
    *outExp -= precision - 1;
    if (precision == 0) {
        *outWritten = 1;
        denominator = denominator * cast_big(10);
        b[0]        = compare(numerator * cast_big(2), denominator) > 0 ? '1' : '0';
        return;
    }

    // Write out however many digits were requested
    *outWritten = precision;
    For(range(precision - 1)) {
        auto [digit, mod] = divmod(numerator, denominator);
        numerator         = mod;

        assert(is_digit_in_valid_range(&digit));
        u32 outputDigit = get_digit_or_zero(&digit);

        b[it] = '0' + outputDigit;

        numerator = numerator * cast_big(10);
    }

    // The final one, also handle rounding..
    auto [digit, mod] = divmod(numerator, denominator);
    numerator         = mod;

    assert(is_digit_in_valid_range(&digit));
    u32 outputDigit = get_digit_or_zero(&digit);

    s64 cmp = compare(numerator * cast_big(2), denominator);
    if (cmp > 0 || (cmp == 0 && (outputDigit % 2) != 0)) {
        if (outputDigit == 9) {
            char overflow    = '0' + 10;
            b[precision - 1] = overflow;

            // Propagate the carry.
            for (s32 i = precision - 1; i > 0 && b[i] == overflow; --i) {
                b[i] = '0';
                ++b[i - 1];
            }
            if (b[0] == overflow) {
                b[0] = '1';
                ++*outExp;
            }
            return;
        }
        ++outputDigit;
    }
    b[precision - 1] = '0' + outputDigit;
}

LSTD_END_NAMESPACE
