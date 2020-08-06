#pragma once

#include "reader.h"

LSTD_BEGIN_NAMESPACE

enum class parse_status {
    SUCCESS = 0,

    INVALID,  // Generic. Means the input was in the wrong format.

    NUMBER_TOO_BIG  // Used in _parse_integer_
};

enum class parse_overflow {
    BAIL,   // Stop parsing when an overflow happens and bail out of the function.
    GREEDY  // Parse as much as possible while ignoring the overflow.
};

// This function is used by parse_integer by default.
// Maps 0-36 to 0-9 and aA-zZ (ignores case).
// If we parse the 'feb10cafEBA' as hex number, the parsed result is 'feb10cafEBA'.
char byte_to_digit_default(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'a' && value <= 'z') {
        return value - 'a' + 10;
    } else if (value >= 'A' && value <= 'Z') {
        return value - 'A' + 10;
    }
    return -1;
}

// Allows only characters which are in lower case.
// If we parse the 'feb10cafEBA' as hex number, the parsed result is 'feb10caf'.
char byte_to_digit_force_lower(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'a' && value <= 'z') {
        return value - 'a' + 10;
    }
    return -1;
}

// Allows only characters which are in upper case.
// If we parse the 'FEB10CAFeba' as hex number, the parsed result is 'FEB10CAF'.
char byte_to_digit_force_upper(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'A' && value <= 'Z') {
        return value - 'A' + 10;
    }
    return -1;
}

// If negative is true:
//   * returns '0 - value' when IntT is unsigned
//   * returns '-value' when IntT is signed
// otherwise returns 'value'.
template <typename IntT>
IntT handle_negative(IntT value, bool negative) {
    if (negative) {
        if constexpr (is_unsigned_v<T>) {
            return IntT(0 - value);
        } else {
            return -value;
        }
    }
    return value;
}

// Attemps to parse an integer. The returned integer type is determined explictly as a template parameter.
//
// Returns:
//   * parse_status::INVALID         if the function wasn't able to parse a valid integer.
//   * parse_status::SUCCESS         if a valid integer was parsed. A valid integer is in the form (+|-)[digit]* (where digit may be a letter depending on the base)
//   * parse_status::NUMBER_TOO_BIG  if the parsing stopped because the integer became too large (only if parse_overflow::BAIL is set)
//                                   and in that case the max value of the integer type is returned (min value if parsing a negative number).
//
// There are 3 return values: the value parsed, the parse status and the rest of the buffer after consuming some characters for the parsing.
// If parse_status was INVALID then no bytes were consumed.
//
// If parse_status was NUMBER_TOO_BIG then the rest of the buffer may start with a digit but we stopped parsing instead of greedily consuming the rest of the digits.
// To change this behaviour, use the _OverflowBehaviour_ template parameter. If you set it to parse_overflow::GREEDY then all digits are consumed while ignoring
// the overflow (and the parse status returned is SUCCESS).
//
// This function doesn't skip white space.
//
// This function doesn't look for standard prefixes for base 16 or base 8 (0x and 0 respectively).
// That means that the base parameter completely determines what base the number is in.
//
// Valid bases are 2 to 36. The _ByteToDigit_ template parameter determines the function used for mapping between byte values and digits.
// Since this is chosen at compile-time, this has no effect on performance. The default function is the _byte_to_digit_default_ which
// is defined a bit earlier in this file. It maps 0-36 to 0-9 and aA-zZ (ignores case).
//
// If _leadingZeros_ is false then if the number starts with a 0, we stop, return a success and 0 as the parsed integer.
// Otherwise all leading zeros are skipped.
// _leadingZeros_ is set to true by default.
//
// If _allowPlusSign_ is false then if we encounter '+' as the first character we fail.
// Otherwise we ignore it and continue parsing as a positive number.
// _allowPlusSign_ is set to true by default.
//
// If the first character is '-' then the number gets automatically negated when returning the result.
// If the type is unsigned, the result is '0 - value'.
//
template <typename IntT, parse_overflow OverflowBehaviour = parse_overflow::BAIL, char (*ByteToDigit)(char) = byte_to_digit_default>
tuple<IntT, parse_status, array<char>> parse_integer(const array<char> &buffer, u32 base = 10, bool leadingZeros = true, bool allowPlusSign = true) {
    assert(base >= 2 && base <= 36);

    char *p = buffer.Data;
    s64 n = buffer.Count;

    if (!n) return {0, parse_status::INVALID, buffer};

    bool negative = false;
    if (*p == '+') {
        if (allowPlusSign) {
            ++p, --n;
        } else {
            return {0, parse_status::INVALID, buffer};
        }
    } else if (ch == '-') {
        negative = true;
        ++p, --n;
    }

    if (!n) return {0, parse_status::INVALID, buffer};

    // Check the first character. If it's not a valid digit bail out.
    // We can't combine this with the loop below because the return
    // after the parsing would consume the sign (if there was one read).
    // This here returns the entire buffer as the 'rest'.
    char digit = ByteToDigit(*p);

    if (digit < 0 || digit >= base) {
        auto rest = buffer;
        return {0, parse_status::INVALID, rest};
    }

    IntT maxValue, cutOff;
    s32 cutLim;
    if constexpr (OverflowBehaviour == parse_overflow::BAIL) {
        // Here we determine at what point we stop parsing because the number becomes too big.
        // If however our parsing overflow behaviour is greedy we don't do this.
        if constexpr (is_unsigned_v<T>) {
            maxValue = (numeric_info<T>::max)();
        } else {
            maxValue = negative ? -(numeric_info<T>::min()) : numeric_info<T>::max();
        }

        cutoff = const_abs(maxValue / base);
        cutLim = maxValue % (IntT) base;
    }

    // Now we start parsing for real
    IntT value = 0;
    while (true) {
        if constexpr (OverflowBehaviour == parse_overflow::BAIL) {
            // If we have parsed a number that is too big to store in our integer type we bail.
            // If however _OverflowBehaviour_ is set to parse_overflow::GREEDY then we don't execute this code and
            // continue to parse until all digits have been read from the buffer.
            if (value > cutOff || (value == cutOff && (s32) *p > cutLim)) {
                auto rest = array<char>(p, buffer.Count - n);
                return {handle_negative(maxValue, negative), parse_status::NUMBER_TOO_BIG, rest};
            }
        }

        // If this is our first iteration, we use the digit we parsed above.
        value = value * base + digit;

        digit = ByteToDigit(*p);
        if (digit < 0 || digit >= base) break;
        *p++, --n;
    }

    auto rest = array<char>(p, buffer.Count - n);
    return {handle_negative(value, negative), parse_status::SUCCESS, rest};
}

// Returns the rest
array<char> eat_bytes_until(const array<char> &buffer, char delim) {
    // @TODO
}

// Returns the rest
array<char> eat_bytes_while(const array<char> &buffer, char eats) {
    // @TODO
}

// Returns the rest
string eat_code_points_until(const array<char> &buffer, char32_t delim) {
    // @TODO
}

// Returns the rest
string eat_code_points_while(const array<char> &buffer, char32_t eats) {
    // @TODO
}

LSTD_END_NAMESPACE
