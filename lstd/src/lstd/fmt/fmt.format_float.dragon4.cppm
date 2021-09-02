module;

#include "../memory/string_builder.h"

export module fmt.format_float.dragon4;

import fmt.format_float.specs;

import lstd.big_integer;

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
big_int_2048 bigint_pow10(s32 exp) {
    assert(exp >= 0);
    if (exp == 0) {
        return 1;
    }

    // Find the top bit
    s32 bitmask = 1;
    while (exp >= bitmask) bitmask <<= 1;
    bitmask >>= 1;

    // pow(10, exp) = pow(5, exp) * pow(2, exp).
    // First compute pow(5, exp) by repeated squaring and multiplication.
    big_int_2048 b = 5;
    bitmask >>= 1;
    while (bitmask != 0) {
        b = b * b;
        if ((exp & bitmask) != 0) b *= 5;
        bitmask >>= 1;
    }
    b <<= exp;  // Multiply by pow(2, exp) by shifting.

    return b;
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
//  1. fixed overflow problems when significand was 64 bits (in float128 types),
//     by replacing multiplication by 2 or 4 by BigInt_ShiftLeft calls.
//  2. Increased c_BigInt_MaxBlocks, for 128-bit floats
//  3. Added more entries to the g_PowerOf10_Big table, for 128-bit floats.
//  4. Added unbiased rounding calculation with isEven. Ryan Juckett's
//     implementation did not implement "IEEE unbiased rounding", except in the
//     last digit. This has been added back, following the Burger & Dybvig
//     code, using the isEven variable.
//
// Arguments:
// _b_           - buffer, should be big enough to hold the output.
// _outWritten_  - contains the number of digits written.
// _outExp_      - the base 10 exponent of the first digit, this could have been overriden by the function.
// _cutoffMax_  - cut off printing after this many digits; -1 for no cutoff.
// _v_           - f32 or f64; contains the value of the float to be formatted.
//
export void dragon4_format_float(utf8 *b, s64 *outWritten, s32 *outExp, s32 cutoffMax, types::is_floating_point auto v) {
    // We compute values in integer format by rescaling as
    //   significand = scaledValue / scale
    //   marginLow = scaledMarginLow / scale
    //   marginHigh = scaledMarginHigh / scale
    // Here, marginLow and marginHigh represent 1/2 of the distance to the next
    // floating point value above/below the significand.
    big_int_2048 scale, scaledValue, scaledMarginLow, optionalMarginHigh;

    // scaledMarginHigh will point to scaledMarginLow in the case they must be
    // equal to each other, otherwise it will point to optionalMarginHigh.
    big_int_2048 *scaledMarginHigh = null;

    fp value;
    bool isPredecessorCloser = fp_assign_new(value, v);  // Called "hasUnequalMargins" in the original

    s32 exponent = value.Exponent;
    scaledValue  = value.Significand;

    bool isEven = scaledValue % 2 == 0;

    if (isPredecessorCloser) {
        if (exponent > 0) {
            // If we have no fractional component
            //
            // 1) Expand the input value by multiplying out the significand and
            //    exponent. This represents the input value in its whole number
            //    representation.
            // 2) Apply an additional scale of 2 such that later comparisons
            //    against the margin values are simplified.
            // 3) Set the margin value to the lowest significand bit's scale.

            // scaledValue = 2 * 2 * significand * 2^exponent
            scaledValue <<= (exponent + 2);

            // scale = 2 * 2 * 1
            scale = 4;

            // scaledMarginLow = 2 * 2^(exponent-1)
            scaledMarginLow = 1;
            scaledMarginLow <<= exponent;

            // scaledMarginHigh   = 2 * 2 * 2^(exponent-1)
            optionalMarginHigh = 1;
            optionalMarginHigh <<= (exponent + 1);
        }

        else {
            // We have a fractional exponent.
            //
            // In order to track the significand data as an integer, we store it as
            // is with a large scale

            // scaledValue = 2 * 2 * significand
            scaledValue <<= 2;

            // scale = 2 * 2 * 2^(-exponent)
            scale = 1;
            scale <<= (-exponent + 2);

            // scaledMarginLow = 2 * 2^(-1)
            scaledMarginLow = 1;

            // scaledMarginHigh = 2 * 2 * 2^(-1)
            optionalMarginHigh = 2;
        }

        // The high and low margins are different
        scaledMarginHigh = &optionalMarginHigh;
    } else {
        if (exponent > 0) {
            // If we have no fractional component

            // scaledValue = 2 * significand * 2^exponent
            scaledValue <<= (exponent + 1);

            // scale = 2 * 1
            scale = 2;

            // scaledMarginLow = 2 * 2^(exponent-1)
            scaledMarginLow = 1;
            scaledMarginLow <<= exponent;
        } else {
            // We have a fractional exponent.
            //
            // In order to track the significand data as an integer, we store it as
            // is with a large scale

            // scaledValue = 2 * significand
            scaledValue <<= 1;

            // scale = 2 * 2^(-exponent)
            scale = 1;
            scale <<= (-exponent + 1);

            // scaledMarginLow =  2 * 2^(-1)
            scaledMarginLow = 1;
        }

        // The high and low margins are equal.
        scaledMarginHigh = &scaledMarginLow;
    }

    constexpr f64 LOG10_2 = 0.30102999566398119521373889472449;

    // Compute an estimate for digitExp that will be correct or undershoot
    // by one.  This optimization is based on the paper "Printing Floating-Point
    // Numbers Quickly and Accurately" by Burger and Dybvig
    // http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.72.4656
    // We perform an additional subtraction of 0.69 to increase the frequency of
    // a failed estimate because that lets us take a faster branch in the code.
    // 0.69 is chosen because 0.69 + log10(2) is less than one by a reasonable
    // epsilon that will account for any floating point error.
    //
    // We want to set digitExp to floor(log10(v)) + 1
    //  v = significand * 2^exponent
    //  log2(v) = log2(significand) + exponent;
    //  log10(v) = log2(v) * log10(2)
    //  floor(log2(v)) = mantissaBit + exponent;
    //  log10(v) - log10(2) < (mantissaBit + exponent) * log10(2) <= log10(v)
    //  log10(v) < (mantissaBit + exponent) * log10(2) + log10(2) <= log10(v) + log10(2)
    //  floor(log10(v)) < ceil((mantissaBit + exponent) * log10(2)) <= floor(log10(v)) + 1
    s32 digitExp = (s32) (ceil((f64) (value.MantissaBit + exponent) * LOG10_2 - 0.69));

    // Divide value by 10^digitExp.
    if (digitExp > 0) {
        // A positive exponent creates a division so we multiply the scale.
        scale *= bigint_pow10(digitExp);
    } else if (digitExp < 0) {
        // A negative exponent creates a multiplication so we multiply up the
        // scaledValue, scaledMarginLow and scaledMarginHigh.
        auto pow10 = bigint_pow10(-digitExp);
        scaledValue *= pow10;
        scaledMarginLow *= pow10;

        if (scaledMarginHigh != &scaledMarginLow) {
            *scaledMarginHigh = scaledMarginLow * 2;
        }
    }

    // If (value >= 1), our estimate for digitExp was too low.
    if (compare(scaledValue, scale) >= 0) {
        // The exponent estimate was incorrect.
        // Increment the exponent and don't perform the premultiply needed
        // for the first loop iteration.
        digitExp += 1;
    } else {
        // The exponent estimate was correct.
        // Multiply larger by the output base to prepare for the first loop iteration.
        scaledValue *= 10;
        scaledMarginLow *= 10;
        if (scaledMarginHigh != &scaledMarginLow) {
            *scaledMarginHigh = scaledMarginLow * 2;
        }
    }

    // Compute the cutoffMax exponent (the exponent of the final digit to
    // print). Default to the maximum size of the output buffer.
    s32 cutoffMaxExp = digitExp - 16 * 1024 * 1024;  // Dummy buffer size

    s32 desiredCutoffExponent = digitExp - cutoffMax;
    if (desiredCutoffExponent > cutoffMaxExp) {
        cutoffMaxExp = desiredCutoffExponent;
    }

    // Otherwise it's CutoffMode_FractionLength. Print cutoffMax digits
    // past the decimal point or until we reach the buffer size
    // * desiredCutoffExponent = -cutoffMax;
    // * if (desiredCutoffExponent > cutoffMaxExp) {
    // *     cutoffMaxExp = desiredCutoffExponent;
    // * }

    // Output the exponent of the first digit we will print .
    *outExp = digitExp - 1;
    
    // In preparation for calling BigInt_DivideWithRemainder_MaxQuotient9(), we
    // need to scale up our values such that the highest block of the
    // denominator is greater than or equal to 8. We also need to guarantee that
    // the numerator can never have a length greater than the denominator after
    // each loop iteration.  This requires the highest block of the denominator
    // to be less than or equal to 429496729 which is the highest number that
    // can be multiplied by 10 without overflowing to a new block.
    /* assert(scale.Size > 0);
    u32 hiBlock = scale.Bigits[scale.Size - 1];
    if (hiBlock < 8 || hiBlock > 429496729) {
        // Perform a bit shift on all values to get the highest block of the
        // denominator into the range [8,429496729]. We are more likely to make
        // accurate quotient estimations in
        // BigInt_DivideWithRemainder_MaxQuotient9() with higher denominator
        // values so we shift the denominator to place the highest bit at index
        // 27 of the highest block.  This is safe because (2^28 - 1) = 268435455
        // which is less than 429496729. This means that all values with a
        // highest bit at index 27 are within range.
        u32 hiBlockLog2 = msb(hiBlock | 1); // Integer log2
        assert(hiBlockLog2 < 3 || hiBlockLog2 > 27);
        u32 shift = (32 + 27 - hiBlockLog2) % 32;

        scale <<= shift;
        scaledValue <<= shift;
        scaledMarginLow <<= shift;
        if (scaledMarginHigh != &scaledMarginLow) {
            *scaledMarginHigh = scaledMarginLow * 2;
        }
    }*/

    utf8 *curDigit = b;
    u32 outputDigit;

    while (true) {
        digitExp -= 1;

        // Divide out the scale to extract the digit.
        auto [digit, mod] = divmod(scaledValue, scale);
        scaledValue       = mod;

        assert(digit < 10);
        outputDigit = digit.Bigits[0];

        if ((scaledValue.Size == 0) | (digitExp == cutoffMaxExp)) break;

        // Store the output digit.
        *curDigit = (utf8) ('0' + outputDigit);
        ++curDigit;

        // Multiply larger by the output base.
        scaledValue *= 10;
    }

    // Round to the closest digit by comparing value with 0.5. To do this we
    // need to convert the inequality to large integer values.
    //  compare(value, 0.)
    //  compare(scale * value, scale * 0.5)
    //  compare(2 * scale * value, scale)
    scaledValue *= 2;

    s64 cmp        = compare(scaledValue, scale);
    bool roundDown = cmp < 0;

    // If we are directly in the middle, round towards the even digit (i.e. IEEE rounding rules).
    if (cmp == 0) {
        roundDown = (outputDigit & 1) == 0;
    }

    // Print the rounded digit.
    if (roundDown) {
        *curDigit = (char) ('0' + outputDigit);
        ++curDigit;
    } else {
        if (outputDigit == 9) {
            // Find the first non-nine prior digit.
            while (true) {
                if (curDigit == b) {
                    // If we are at the first digit.
                    *curDigit = '1';
                    ++curDigit;
                    *outExp += 1;
                    break;
                }

                --curDigit;
                if (*curDigit != '9') {
                    *curDigit += 1;
                    ++curDigit;
                    break;
                }
            }
        } else {
            // Values in the range [0,8] can perform a simple round up.
            *curDigit = (char) ('0' + outputDigit + 1);
            ++curDigit;
        }
    }

    // Return the number of digits output.
    *outWritten = (u32) (curDigit - b);
}

LSTD_END_NAMESPACE
