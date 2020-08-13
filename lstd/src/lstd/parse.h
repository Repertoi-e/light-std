#pragma once

#include "io/reader.h"

LSTD_BEGIN_NAMESPACE

enum parse_status : u32 {
    PARSE_SUCCESS = 0,
    
    // We have ran out of buffer. The returned _rest_ is actually the whole buffer which was passed.
    // The caller should get more bytes and concatenate it with the original buffer and call the function again (usually that is what you want).
    PARSE_EXHAUSTED,
    
    PARSE_INVALID,  // Means the input was malformed/in the wrong format
    
    PARSE_TOO_MANY_DIGITS  // Used in _parse_integer_ when the resulting value overflowed or underflowed
};

// Used in parse functions for some special behaviour/special return values
constexpr static char BYTE_NOT_VALID = -1;
constexpr static char IGNORE_THIS_BYTE = '\x7f';  // This is the non-printable DEL char in ascii, arbritralily chosen..

// Maps 0-36 to 0-9 and aA-zZ (ignores case).
// If we parse the 'feb10cafEBA' as hex number, the parsed result is 'feb10cafEBA'.
inline char byte_to_digit_default(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'a' && value <= 'z') {
        return value - 'a' + 10;
    } else if (value >= 'A' && value <= 'Z') {
        return value - 'A' + 10;
    }
    return BYTE_NOT_VALID;
}

// Allows only characters which are in lower case.
// If we parse the 'feb10cafEBA' as hex number, the parsed result is 'feb10caf'.
inline char byte_to_digit_force_lower(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'a' && value <= 'z') {
        return value - 'a' + 10;
    }
    return BYTE_NOT_VALID;
}

// Allows only characters which are in upper case.
// If we parse the 'FEB10CAFeba' as hex number, the parsed result is 'FEB10CAF'.
inline char byte_to_digit_force_upper(char value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'A' && value <= 'Z') {
        return value - 'A' + 10;
    }
    return BYTE_NOT_VALID;
}

//
// This struct is used to conditionally compile _parse_integer_.
// Normally writing a function that has these as arguments means that it will become quite large and slow because it has to handle all cases.
// These options determine which code paths of _parse_integer_ get compiled and thus don't have effect on runtime performance.
//
// This is done in such way so you can specify a custom struct as a template parameter which,
// e.g. disables all unnecessary parsing and you get a really barebones and fast integer to string function.
//
// The result is that we have written an extremely general parse function which can be tweaked to any imaginable non-weird use case,
// and the user didn't have to write his own fast version because ours was too slow, which means that this library is a lot more usable.
//
// We could only wish C++ had more facilities which helped with these kind of options (e.g. Jai's #bake_arguments), but this seems to work well enough.
//
struct parse_int_options {
    // We use _ByteToDigit_ to convert from a byte value to a digit.
    // The spec is that if a byte is valid, the returned value is the value of the digit.
    // By default we ignore case so 0-9 and a-z/A-Z map to 0-9 and 10-36 which allows base-36 numbers to be parsed.
    // We don't handle more than that by default because we are not sure what characters should be assigned to which digit values.
    // e.g. there are many base-64 formats each using different digits. You can write a function that supports higher bases very easily!
    //
    // For any byte that doesn't correspond to a digit, return _BYTE_NOT_VALID_ and if the parser should ignore the byte but not fail parsing, return _IGNORE_THIS_BYTE.
    // Here are two use cases which illustrate that:
    //
    //     // This byte to digit function supports only decimal and allows parsing '1_000_000' as 1 million because it tells the parser to ignore '_'.
    //     // You can also write a function which ignores commas, using this to parse numbers with a thousands separator: '1,000,000'
    //     char byte_to_digit_which_ignores_underscores(char value) {
    //         if (value >= '0' && value <= '9') {
    //             return value - '0';
    //         } else if (value == '_') {
    //             return IGNORE_THIS_BYTE;
    //         }
    //         return BYTE_NOT_VALID;
    //     }
    //
    //     // Allows parsing a base-64 integer [0-9a-zA-Z#_]* and doesn't support = as padding because we treat the input as an integer.
    //     // To parse actual data encoded in base-64 you might want to use another function and not parse_integer..
    //     char byte_to_digit_base_64(char value) {
    //         if (value >= '0' && value <= '9') {
    //             return value - '0';
    //         } else if (value >= 'a' && value <= 'z') {
    //             return value - 'a' + 10;
    //         } else if (value >= 'A' && value <= 'Z') {
    //             return value - 'A' + 10 + 26;
    //         } else if (value == '#') {
    //             return 63;
    //         } else if (value == '_') {
    //             return 64;
    //         }
    //         return BYTE_NOT_VALID;
    //     }
    //
    // Remember that this function is actually specified at compile time!!!
    char (*ByteToDigit)(char) = byte_to_digit_default;
    
    bool ParseSign = true;           // If true, looks for +/- before trying to parse any digits. If '-' the result is negated.
    bool AllowPlusSign = true;       // If true, allows explicit + as a sign, if false, results in parse failure.
    bool LookForBasePrefix = false;  // If true, looks for 0x and 0 for hex and oct respectively and if found, overwrites the base parameter.
    
    enum too_many_digits : char {
        BAIL,     // Stop parsing when an overflow happens and bail out of the function.
        CONTINUE  // Parse as much digits as possible while ignoring the overflow/underflow.
    };
    
    too_many_digits TooManyDigitsBehaviour = BAIL;
    
    constexpr parse_int_options() {}
    constexpr parse_int_options(char (*byteToDigit)(char), bool parseSign, bool allowPlusSign, bool lookForBasePrefix, too_many_digits tooManyDigitsBehaviour)
        : ByteToDigit(byteToDigit), ParseSign(parseSign), AllowPlusSign(allowPlusSign), LookForBasePrefix(lookForBasePrefix), TooManyDigitsBehaviour(tooManyDigitsBehaviour) {}
};

constexpr parse_int_options parse_int_options_default;

// If negative is true:
//   * returns '0 - value' when IntT is unsigned
//   * returns '-value' when IntT is signed
// otherwise returns 'value'.
template <typename IntT>
IntT handle_negative(IntT value, bool negative) {
    if (negative) {
        if constexpr (is_unsigned_v<IntT>) {
            return IntT(0 - value);
        } else {
            return -value;
        }
    }
    return value;
}

// Attemps to parse an integer. The integer size returned is determined explictly as a template parameter.
// This is a very light-weight and performant function without any unnecessary bloat and cruft.
//
// There are 3 return values: the value parsed, the parse status and the rest of the buffer after consuming some characters for the parsing.
// If the status was PARSE_INVALID then some bytes could have been consumed (for example +/- or the base prefix).
//
// Allows for compilation of different code paths using a template parameter which is a pointer to a struct (parse_int_options) with constants.
// The options are described there (a bit earlier in this file). But in short, it contains a function which maps bytes to digits as well as options
// for how we should handle signs, base prefixes and integer overflow/underflow.
//
// By default we try to do the most sensible and useful thing. Valid integers start with either +/- and then a base prefix (0 or 0x for oct/hex)
// and then a range of bytes which describe digits. We stop parsing when we encounter a byte which is not a valid digit.
//
// Valid bases are 2 to 36. The _ByteToDigit_ template parameter determines the function used for mapping between byte values and digits.
// Since this is chosen at compile time, this has no effect on performance. The default function is the _byte_to_digit_default_ which
// is defined a bit earlier in this file. It maps 0-36 to 0-9 and aA-zZ (ignores case).
//
// By default we stop parsing when the resulting integer overflows/underflows instead of greedily consuming the rest of the digits.
// In that case the returned status is PARSE_TOO_MANY_DIGITS. To change this behaviour, change the _TooManyDigitsBehaviour_ option.
// If you set it to IGNORE then all digits are consumed while ignoring the overflow/underflow (and the parse status returned is SUCCESS).
//
// This function doesn't eat white space from the beginning.
//
// Returns:
//   * PARSE_SUCCESS          if a valid integer was parsed. A valid integer is in the form (+|-)[digit]* (where digit may be a letter depending on the base),
//   * PARSE_EXHAUSTED        if we parsed a sign or a base prefix but we ran ouf ot bytes,
//   * PARSE_INVALID          if the function wasn't able to parse a valid integer but
//                            note that if the integer starts with +/-, that could be considered invalid (depending on the options),
//   * PARSE_TOO_MANY_DIGITS  if the parsing stopped because the integer became too large (only if TooManyDigitsBehaviour == BAIL in options)
//                            and in that case the max value of the integer type is returned (min value if parsing a negative number).
//
template <typename IntT, const parse_int_options *Options = &parse_int_options_default>
tuple<IntT, parse_status, array<char>> parse_integer(const array<char> &buffer, u32 base = 10) {
    assert(base >= 2 && base <= 36);
    
    char *p = buffer.Data;
    s64 n = buffer.Count;
    
    if (!n) return {0, PARSE_EXHAUSTED, buffer};
    
    bool negative = false;
    if constexpr (Options->ParseSign) {
        if (*p == '+') {
            ++p, --n;
            if constexpr (!Options->AllowPlusSign) {
                return {0, PARSE_INVALID, array<char>(p, n)};
            }
        } else if (*p == '-') {
            negative = true;
            ++p, --n;
        }
        if (!n) return {0, PARSE_EXHAUSTED, buffer};
    }
    
    if constexpr (Options->LookForBasePrefix) {
        if (*p == '0') {
            if ((n - 1) && *(p + 1) == 'x') {
                base = 16;
                p += 2, n -= 2;
            } else {
                base = 8;
                ++p, --n;
            }
        }
        if (!n) return {0, PARSE_EXHAUSTED, buffer};
    }
    
    // Check the first character. If it's not a valid digit bail out.
    // We can't combine this with the loop below because the return
    // after the parsing would consume the sign (if there was one read).
    // This here returns the entire buffer as the 'rest'.
    char digit = Options->ByteToDigit(*p);
    ++p, --n;
    
    if (digit < 0 || digit >= (s32) base) return {0, PARSE_INVALID, array<char>(p, n)};
    
    IntT maxValue, cutOff;
    s32 cutLim;
    if constexpr (Options->TooManyDigitsBehaviour == parse_int_options::BAIL) {
        // Here we determine at what point we stop parsing because the number becomes too big.
        // If however our parsing overflow behaviour is greedy we don't do this.
        if constexpr (is_unsigned_v<IntT>) {
            maxValue = (numeric_info<IntT>::max)();
        } else {
            maxValue = negative ? -(numeric_info<IntT>::min()) : numeric_info<IntT>::max();
        }
        
        cutOff = const_abs(maxValue / base);
        cutLim = maxValue % (IntT) base;
    }
    
    // Now we start parsing for real
    IntT value = 0;
    while (true) {
        if constexpr (Options->TooManyDigitsBehaviour == parse_int_options::BAIL) {
            // If we have parsed a number that is too big to store in our integer type we bail.
            // If however _OverflowBehaviour_ is set to integer_parse_too_many_digits::IGNORE then we don't execute this code and
            // continue to parse until all digits have been read from the buffer.
            if (value > cutOff || (value == cutOff && (s32) *p > cutLim)) {
                // If we haven't parsed a sign there is no need to handle negative
                if constexpr (Options->ParseSign) {
                    return {handle_negative(maxValue, negative), PARSE_TOO_MANY_DIGITS, array<char>(p, n)};
                } else {
                    return {maxValue, PARSE_TOO_MANY_DIGITS, array<char>(p, n)};
                }
            }
        }
        
        // If this is our first iteration, we use the digit we parsed above
        value = value * base + digit;
        
        digit = Options->ByteToDigit(*p);
        if (digit < 0 || digit >= (s32) base) break;
        *p++, --n;
    }
    
    // If we haven't parsed a sign there is no need to handle negative
    if constexpr (Options->ParseSign) {
        return {handle_negative(value, negative), PARSE_SUCCESS, array<char>(p, n)};
    } else {
        return {value, PARSE_SUCCESS, array<char>(p, n)};
    }
}

// If _IgnoreCase_ is true, then _value_ must be lower case (to save on performance)
template <bool IgnoreCase>
inline parse_status eat_byte(char **p, s64 *n, char value) {
    if (!*n) return PARSE_EXHAUSTED;
    char ch = **p;
    if constexpr (IgnoreCase) ch = (char) to_lower(ch);
    if (ch == value) {
        ++*p, --*n;
        return PARSE_SUCCESS;
    } else {
        return PARSE_INVALID;
    }
}

// If _IgnoreCase_ is true, then _sequence_ must be lower case (to save on performance)
template <bool IgnoreCase>
inline parse_status eat_sequence(char **p, s64 *n, const array<char> &sequence) {
    For(sequence) {
        parse_status status = eat_byte<IgnoreCase>(p, n, it);
        if (status != PARSE_SUCCESS) return status;
    }
    return PARSE_SUCCESS;
}

struct parse_bool_options {
    bool ParseNumbers = true;  // Attemps to parse 0/1.
    bool ParseWords = true;    // Attemps to parse the words "true" and "false".
    bool IgnoreCase = false;   // Ignores case when parsing the words.
    
    constexpr parse_bool_options() {}
    constexpr parse_bool_options(bool parseNumbers, bool parseWords, bool ignoreCase) : ParseNumbers(parseNumbers), ParseWords(parseWords), IgnoreCase(ignoreCase) {}
};

constexpr parse_bool_options parse_bool_options_default;

//
//
//
template <const parse_bool_options *Options = &parse_bool_options_default>
tuple<bool, parse_status, array<char>> parse_bool(const array<char> &buffer) {
    static_assert(Options->ParseNumbers || Options->ParseWords);  // Sanity
    
    char *p = buffer.Data;
    s64 n = buffer.Count;
    
    if (!n) return {0, PARSE_EXHAUSTED, buffer};
    
    if constexpr (Options->ParseNumbers) {
        if (*p == '0') return {false, PARSE_SUCCESS, array<char>(p + 1, n - 1)};
        if (*p == '1') return {true, PARSE_SUCCESS, array<char>(p + 1, n - 1)};
    }
    
    if constexpr (Options->ParseWords) {
        if (*p == 't') {
            parse_status status = eat_sequence<Options->IgnoreCase>(&p, &n, (string) "true");
            if (status == PARSE_INVALID) return {false, PARSE_INVALID, array<char>(p, n)};
            if (status == PARSE_EXHAUSTED) return {false, PARSE_EXHAUSTED, buffer};
            
            return {true, PARSE_SUCCESS, array<char>(p, n)};
        }
        
        if (*p == 'f') {
            parse_status status = eat_sequence<Options->IgnoreCase>(&p, &n, (string) "false");
            if (status == PARSE_INVALID) return {false, PARSE_INVALID, array<char>(p, n)};
            if (status == PARSE_EXHAUSTED) return {false, PARSE_EXHAUSTED, buffer};
            
            return {false, PARSE_SUCCESS, array<char>(p, n)};
        }
    }
    
    return {false, PARSE_INVALID, array<char>(p, n)};
}

// Returns the rest after _delim_ is reached
array<char> eat_bytes_until(const array<char> &buffer, char delim) {
    // @TODO
    return buffer;
}

// Returns the rest after a byte that is not _eats_ is reached
array<char> eat_bytes_while(const array<char> &buffer, char eats) {
    // @TODO
    return buffer;
}

// Same as the buffer version but works on strings and pays attention to utf8
string eat_code_points_until(const string &str, char32_t delim) {
    // @TODO
    return "";
}

// Same as the buffer version but works on strings and pays attention to utf8
string eat_code_points_while(const string &str, char32_t eats) {
    // @TODO
    return "";
}

LSTD_END_NAMESPACE
