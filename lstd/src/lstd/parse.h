#pragma once

#include "memory/array.h"
#include "memory/guid.h"
#include "memory/string.h"
#include "types/numeric_info.h"

LSTD_BEGIN_NAMESPACE

enum parse_status : u32 {
    PARSE_SUCCESS = 0,

    // We have ran out of buffer. The returned _rest_ is actually the whole buffer which was passed.
    // The caller should get more bytes and concatenate it with the original buffer and call the function again (usually that is what you want).
    PARSE_EXHAUSTED,

    PARSE_INVALID,  // Means the input was malformed/in the wrong format

    PARSE_TOO_MANY_DIGITS  // Used in _parse_int_ when the resulting value overflowed or underflowed
};

// Used in parse functions for some special behaviour/special return values
constexpr static byte BYTE_NOT_VALID = (byte) -1;
constexpr static byte IGNORE_THIS_BYTE = '\x7f';  // This is the non-printable DEL char in ascii, arbritralily chosen..

typedef s32 (*byte_to_digit_t)(byte);

// Maps 0-36 to 0-9 and aA-zZ (ignores case).
// If we parse the 'feb10cafEBA' as hex number, the parsed result is 'feb10cafEBA'.
inline s32 byte_to_digit_default(byte value) {
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
inline s32 byte_to_digit_force_lower(byte value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'a' && value <= 'z') {
        return value - 'a' + 10;
    }
    return BYTE_NOT_VALID;
}

// Allows only characters which are in upper case.
// If we parse the 'FEB10CAFeba' as hex number, the parsed result is 'FEB10CAF'.
inline s32 byte_to_digit_force_upper(byte value) {
    if (value >= '0' && value <= '9') {
        return value - '0';
    } else if (value >= 'A' && value <= 'Z') {
        return value - 'A' + 10;
    }
    return BYTE_NOT_VALID;
}

//
// This struct is used to conditionally compile _parse_int_.
// Normally writing a function that has these as arguments means that it will become quite large and slow because it has to handle all cases.
// These options determine which code paths of _parse_int_ get compiled and thus don't have effect on runtime performance.
//
// This is done in such way so you can specify a custom struct as a template parameter which,
// e.g. disables all unnecessary parsing and you get a really barebones and fast integer to string function.
//
// The result is that we have written an extremely general parse function which can be tweaked to any imaginable non-weird use case,
// and the user didn't have to write his own fast version because ours was too slow, which means that this library is a lot more usable.
//
// We could only wish C++ had more facilities which helped with these kind of options (e.g. Jai's #bake_arguments), but this seems to work well enough.
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
    //     s32 byte_to_digit_which_ignores_underscores(byte value) {
    //         if (value >= '0' && value <= '9') {
    //             return value - '0';
    //         } else if (value == '_') {
    //             return IGNORE_THIS_BYTE;
    //         }
    //         return BYTE_NOT_VALID;
    //     }
    //
    //     // Allows parsing a base-64 integer [0-9a-zA-Z#_]* and doesn't support = as padding because we treat the input as an integer.
    //     // To parse actual data encoded in base-64 you might want to use another function and not parse_int..
    //     s32 byte_to_digit_base_64(byte value) {
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
    // Remember that this function is actually specified at compile time!!! The compiler inlines it.
    byte_to_digit_t ByteToDigit = byte_to_digit_default;

    bool ParseSign = true;           // If true, looks for +/- before trying to parse any digits. If '-' the result is negated.
    bool AllowPlusSign = true;       // If true, allows explicit + as a sign, if false, results in parse failure.
    bool LookForBasePrefix = false;  // If true, looks for 0x and 0 for hex and oct respectively and if found, overwrites the base parameter.

    enum too_many_digits : s32 {
        BAIL,     // Stop parsing when an overflow happens and bail out of the function.
        CONTINUE  // Parse as much digits as possible while ignoring the overflow/underflow.
    };

    too_many_digits TooManyDigitsBehaviour = BAIL;

    // By default we return the min/max value of the integer type when we bail because we parsed too many digits.
    // If false, the returned value is the value we have parsed so far.
    bool ReturnLimitOnTooManyDigits = true;

    // Set maximum amount of digits which should be parsed. -1 means unspecified (no limit).
    // Note: The return value is PARSE_SUCCESS and not PARSE_TOO_MANY_DIGITS.
    // Note: IGNORE_THIS_BYTE from ByteToDigit is counted towards this.
    s64 MaxDigits = -1;

    // @TODO: Potentially useful?
    // bool BasePrefixCountedTowardsMaxDigits = true;                 // By default we count the base prefix towards _MaxDigits_.
    // bool ByteToDigitIgnoreThisByteCountedTowardsMaxDigits = true;  // _ByteToDigit_ might return IGNORE_THIS_BYTE, by default we count it towards _MaxDigits_.
};

// With C++20 aggregate dot initialization you can do this!!
// constexpr parse_int_options parse_int_options_no_signs_but_base_prefix = {.ParseSign = false, .AllowPlusSign = false, .LookForBasePrefix = true};

// If negative is true:
//   * returns '0 - value' when IntT is unsigned
//   * returns '-value' when IntT is signed
// otherwise returns 'value'.
template <typename IntT>
IntT handle_negative(IntT value, bool negative) {
    if (negative) {
        if constexpr (types::is_unsigned_integral<IntT>) {
            return IntT(0 - value);
        } else {
            return -value;
        }
    }
    return value;
}

// Unsafe
inline void advance_bytes(bytes *p, s64 count) {
    p->Data += count;
    p->Count -= count;
}

// Unsafe
inline void advance_cp(string *p, s64 count) {
    auto *t = get_cp_at_index(p->Data, p->Length, count, true);
    p->Count -= t - p->Data;
    p->Data = (utf8 *) t;
    p->Length -= count;
}

template <typename T>
struct parse_result {
    T Value;
    parse_status Status;  // If the status was PARSE_INVALID then some bytes could have been consumed (for example +/- or the base prefix).
    bytes Rest;
};

// Attemps to parse an integer. The integer size returned is determined explictly as a template parameter.
// This is a very general and light function.
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
//   * PARSE_EXHAUSTED        if an empty buffer was passed or we parsed a sign or a base prefix but we ran ouf ot bytes,
//   * PARSE_INVALID          if the function wasn't able to parse a valid integer but
//                            note that if the integer starts with +/-, that could be considered invalid (depending on the options),
//   * PARSE_TOO_MANY_DIGITS  if the parsing stopped because the integer became too large (only if TooManyDigitsBehaviour == BAIL in options)
//                            and in that case the max value of the integer type is returned (min value if parsing a negative number).
//
template <typename IntT, parse_int_options Options = parse_int_options{}>
parse_result<IntT> parse_int(bytes buffer, u32 base = 10) {
    assert(base >= 2 && base <= 36);

    if (!buffer.Count) return {0, PARSE_EXHAUSTED, buffer};

    bytes p = buffer;

    bool negative = false;
    if constexpr (Options.ParseSign) {
        if (p[0] == '+') {
            advance_bytes(&p, 1);
            if constexpr (!Options.AllowPlusSign) {
                return {0, PARSE_INVALID, p};
            }
        } else if (p[0] == '-') {
            negative = true;
            advance_bytes(&p, 1);
        }
        if (!p) return {0, PARSE_EXHAUSTED, buffer};
    }

    if constexpr (Options.LookForBasePrefix) {
        if (p[0] == '0') {
            if ((p.Count - 1) && (p[1] == 'x' || p[1] == 'X')) {
                base = 16;
                advance_bytes(&p, 2);
            } else {
                base = 8;
                advance_bytes(&p, 1);
            }
        }
        if (!p) return {0, PARSE_EXHAUSTED, buffer};
    }

    IntT maxValue, cutOff;
    s32 cutLim;
    if constexpr (Options.TooManyDigitsBehaviour == parse_int_options::BAIL) {
        // Here we determine at what point we stop parsing because the number becomes too big.
        // If however our parsing overflow behaviour is greedy we don't do this.
        if constexpr (types::is_unsigned_integral<IntT>) {
            maxValue = (numeric_info<IntT>::max)();
            cutOff = maxValue / base;
        } else {
            maxValue = negative ? -(numeric_info<IntT>::min()) : numeric_info<IntT>::max();
            cutOff = maxValue / base; 
            if (cutOff < 0) cutOff = -cutOff; // abs complains when passing unsigned integer type (doesn't make sense)
        }
        cutLim = maxValue % (IntT) base;
    }

    s64 maxDigits = Options.MaxDigits;
    if constexpr (Options.MaxDigits != -1) {
        assert(maxDigits > 0);
    }

    bool firstDigit = true;

    // Now we start parsing for real
    IntT value = 0;
    while (true) {
        if constexpr (Options.MaxDigits != -1) {
            if (!maxDigits) break;
            --maxDigits;
        }

        s32 digit;
        if (p) {
            // If we still have bytes we read the next one..
            digit = Options.ByteToDigit(p[0]);
            advance_bytes(&p, 1);
        } else {
            // .. if not we don't read p[0] because that's an overflow and
            // we just set an invalid byte which we catch in the condition below.
            digit = BYTE_NOT_VALID;

            // We advance in both cases because later we roll back one byte! It's fine because we are not reading or writing anything just bumping the pointer and count
            advance_bytes(&p, 1);
        }

        if (digit == IGNORE_THIS_BYTE) continue;

        if (digit < 0 || digit >= (s32) base) {
            if (firstDigit) {
                // We have a special case for when we look for prefix and we are base 8 but the whole valid integer is just one 0,
                // then we return 0 and don't treat it as an oct value (because in that case we require more digits).
                if (Options.LookForBasePrefix) {
                    advance_bytes(&p, -1);
                    if (base == 8) return {0, PARSE_SUCCESS, p};
                }
                return {0, PARSE_INVALID, p};
            }

            // Roll back the invalid byte we consumed.
            // After the break we return PARSE_SUCCESS.
            // We only consume the invalid byte if we return PARSE_INVALID.
            advance_bytes(&p, -1);
            break;
        }
        firstDigit = false;

        if constexpr (Options.TooManyDigitsBehaviour == parse_int_options::BAIL) {
            // If we have parsed a number that is too big to store in our integer type we bail.
            // If however _OverflowBehaviour_ is set to integer_parse_too_many_digits::IGNORE then we don't execute this code and
            // continue to parse until all digits have been read from the buffer.
            if (value > cutOff || (value == cutOff && (s32) digit > cutLim)) {
                // If we haven't parsed a sign there is no need to handle negative

                if constexpr (Options.ReturnLimitOnTooManyDigits) value = maxValue;

                if constexpr (Options.ParseSign) {
                    return {handle_negative(value, negative), PARSE_TOO_MANY_DIGITS, p};
                } else {
                    return {value, PARSE_TOO_MANY_DIGITS, p};
                }
            }
        }

        value = value * base + digit;
    }

    // If we haven't parsed a sign there is no need to handle negative
    if constexpr (Options.ParseSign) {
        return {handle_negative(value, negative), PARSE_SUCCESS, p};
    } else {
        return {value, PARSE_SUCCESS, p};
    }
}

// If _IgnoreCase_ is true, then _value_ must be lower case (to save on performance)
template <bool IgnoreCase = false>
inline parse_status expect_byte(bytes *p, byte value) {
    if (!*p) return PARSE_EXHAUSTED;

    byte ch = (*p)[0];
    if constexpr (IgnoreCase) ch = (byte) to_lower(ch);

    if (ch == value) {
        advance_bytes(p, 1);
        return PARSE_SUCCESS;
    } else {
        return PARSE_INVALID;
    }
}

// If _IgnoreCase_ is true, then _sequence_ must be lower case (to save on performance)
template <bool IgnoreCase = false>
inline parse_status expect_sequence(bytes *p, bytes sequence) {
    For(sequence) {
        parse_status status = expect_byte<IgnoreCase>(p, it);
        if (status != PARSE_SUCCESS) return status;
    }
    return PARSE_SUCCESS;
}

// Similar to parse_int, these options compile different versions of parse_bool and turn off certain code paths.
struct parse_bool_options {
    bool ParseNumbers = true;  // Attemps to parse 0/1.
    bool ParseWords = true;    // Attemps to parse the words "true" and "false".
    bool IgnoreCase = true;    // Ignores case when parsing the words.
};

// Attemps to parse a bool.
// This is a very general and light function.
//
// There are 3 return values: the value parsed, the parse status and the rest of the buffer after consuming some characters for the parsing.
// If the status was PARSE_INVALID then some bytes could have been consumed (for example when parsing words and the buffer was "truFe").
//
// Allows for compilation of different code paths using a template parameter which is a pointer to a struct (parse_bool_options) with constants.
// The options are described there (a bit earlier in this file). But in short if provides options for parse integers or words and case handling.
//
// This function doesn't eat white space from the beginning.
//
// Returns:
//   * PARSE_SUCCESS          if a valid bool was parsed (0/1 or  "true"/"false", depending on the options)
//   * PARSE_EXHAUSTED        if an empty buffer was passed or we started parsing a word but ran out of bytes
//   * PARSE_INVALID          if the function wasn't able to parse a valid bool
//
template <parse_bool_options Options = parse_bool_options{}>
parse_result<bool> parse_bool(bytes buffer) {
    static_assert(Options.ParseNumbers || Options.ParseWords);  // Sanity

    if (!buffer) return {0, PARSE_EXHAUSTED, buffer};

    bytes p = buffer;
    if constexpr (Options.ParseNumbers) {
        if (p[0] == '0') {
            advance_bytes(&p, 1);
            return {false, PARSE_SUCCESS, p};
        }
        if (p[0] == '1') {
            advance_bytes(&p, 1);
            return {true, PARSE_SUCCESS, p};
        }
    }

    if constexpr (Options.ParseWords) {
        if (p[0] == 't') {
            parse_status status = expect_sequence<Options.IgnoreCase>(&p, (string) "true");
            if (status == PARSE_INVALID) return {false, status, p};
            if (status == PARSE_EXHAUSTED) return {false, status, buffer};

            return {true, PARSE_SUCCESS, p};
        }

        if (p[0] == 'f') {
            parse_status status = expect_sequence<Options.IgnoreCase>(&p, (string) "false");
            if (status == PARSE_INVALID) return {false, status, p};
            if (status == PARSE_EXHAUSTED) return {false, status, buffer};

            return {false, PARSE_SUCCESS, p};
        }
    }

    return {false, PARSE_INVALID, p};
}

struct eat_hex_byte_result {
    byte Value;
    parse_status Status;
};

// Tries to parse exactly two hex digits as a byte.
// We don't eat if parsing fails.
template <byte_to_digit_t ByteToDigit = byte_to_digit_default>
eat_hex_byte_result eat_hex_byte(bytes *p) {
    auto [value, status, rest] = parse_int<u8, parse_int_options{.ParseSign = false, .ReturnLimitOnTooManyDigits = false, .MaxDigits = 2}>(*p, 16);

    if (status == PARSE_TOO_MANY_DIGITS) status = PARSE_SUCCESS;
    if (status == PARSE_SUCCESS) advance_bytes(p, 2);
    return {(byte) value, status};
}

// Similar to parse_int, these options compile different versions of parse_guid and turn off certain code paths.
struct parse_guid_options {
    // The function used for parsing the actual hex byte values.
    // We call parse_int with it.
    byte_to_digit_t ByteToDigit = byte_to_digit_default;

    // Do we handle formats starting with parentheses - ( or {.
    bool Parentheses = true;

    // Doesn't pay attention to the position or the number of hyphens in the input, just ignores them.
    // This makes parsing go faster when you don't care if the input is partially incorrect or you know it is not!
    bool RelaxHyphens = false;
};

// Parse a guid
// Parses the following representations:
// - 81a130d2502f4cf1a37663edeb000e9f
// - 81a130d2-502f-4cf1-a376-63edeb000e9f
// - {81a130d2-502f-4cf1-a376-63edeb000e9f}
// - (81a130d2-502f-4cf1-a376-63edeb000e9f)
// - {0x81a130d2,0x502f,0x4cf1,{0xa3,0x76,0x63,0xed,0xeb,0x00,0x0e,0x9f}}
//
// Doesn't pay attention to capitalization (both uppercase/lowercase/mixed are valid).
//
// Returns: the guid parsed, a status, and the rest of the buffer
template <parse_guid_options Options = parse_guid_options{}>
parse_result<guid> parse_guid(bytes buffer) {
    guid empty;
    if (!buffer) return {empty, PARSE_EXHAUSTED, buffer};

    bytes p = buffer;

    bool parentheses = false, curly = false;
    if constexpr (Options.Parentheses) {
        if (p[0] == '(' || p[0] == '{') {
            parentheses = true;
            curly = p[0] == '{';
            advance_bytes(&p, 1);
            if (!p) return {empty, PARSE_EXHAUSTED, buffer};
        }
    }

    guid result;

    if ((p.Count - 1) && p[0] == '0') {
        // Parse following format:
        // {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
        // We detect that if the first byte begins with 0x.
        if (p[1] == 'x' || p[1] == 'X') {
            if (!parentheses || !curly) {
                // Here we don't return "rest" after consuming the chars,
                // because the parse functions should return the buffer where
                // the parse error happened (e.g. for logging errors with context).
                // In this cases the error is that there is 0x but we didn't start with a {.
                return {empty, PARSE_INVALID, buffer};
            }

            parse_status status;

            auto *resultBuffer = &result.Data[0];

#define EAT_HEX_BYTES(count)                                           \
    For(range(count)) {                                                \
        auto [value, status] = eat_hex_byte<Options.ByteToDigit>(&p);  \
        if (status == PARSE_INVALID) return {empty, status, p};        \
        if (status == PARSE_EXHAUSTED) return {empty, status, buffer}; \
        *resultBuffer++ = value;                                       \
    }
#define EXPECT_SEQUENCE(sequence)                           \
    status = expect_sequence<true>(&p, (string) sequence);  \
    if (status == PARSE_INVALID) return {empty, status, p}; \
    if (status == PARSE_EXHAUSTED) return {empty, status, buffer};

            EXPECT_SEQUENCE("0x");
            EAT_HEX_BYTES(4);
            EXPECT_SEQUENCE(",0x");

            EAT_HEX_BYTES(2);
            EXPECT_SEQUENCE(",0x");

            EAT_HEX_BYTES(2);
            EXPECT_SEQUENCE(",{0x");

            For(range(7)) {
                EAT_HEX_BYTES(1);
                EXPECT_SEQUENCE(",0x");
            }
            EAT_HEX_BYTES(1);
            EXPECT_SEQUENCE("}}");

#undef EAT_HEX_BYTES
#undef EXPECT_SEQUENCE

            return {result, PARSE_SUCCESS, p};
        }
    }

    bool hyphens = false;

    auto *resultBuffer = &result.Data[0];

    s32 counter = 0;
    while (true) {
        // We expect hyphens at positions 4, 6, 8 and 10.
        // Unless Options.RelaxHyphens was specified.
        if constexpr (!Options.RelaxHyphens) {
            if (counter == 4) {
                if (!hyphens) {
                    if (p[0] == '-') {
                        hyphens = true;
                        advance_bytes(&p, 1);
                        if (!p) return {empty, PARSE_EXHAUSTED, buffer};
                    }
                }
            }

            if (hyphens && (counter == 6 || counter == 8 || counter == 10)) {
                if (p[0] == '-') {
                    advance_bytes(&p, 1);
                    if (!p) return {empty, PARSE_EXHAUSTED, buffer};
                } else {
                    return {empty, PARSE_INVALID, p};
                }
            }
        } else {
            if (p[0] == '-') {
                advance_bytes(&p, 1);
                if (!p) return {empty, PARSE_EXHAUSTED, buffer};
            }
        }

        auto [value, status] = eat_hex_byte<Options.ByteToDigit>(&p);
        if (status == PARSE_INVALID) return {empty, status, p};
        if (status == PARSE_EXHAUSTED) return {empty, status, buffer};
        *resultBuffer++ = value;

        ++counter;

        // We have eaten 16 hex bytes.
        if (counter == 16) break;
    }

    if constexpr (Options.Parentheses) {
        if (parentheses) {
            parse_status status = expect_byte(&p, (curly ? '}' : ')'));
            if (status == PARSE_INVALID) return {empty, PARSE_INVALID, p};
            if (status == PARSE_EXHAUSTED) return {empty, PARSE_EXHAUSTED, buffer};
        }
    }
    return {result, PARSE_SUCCESS, p};
}

struct eat_bytes_result {
    bytes Value;   // The stuff read
    bool Success;  // False if the buffer was exhausted
    bytes Rest;    // The rest of the buffer
};

// Returns: the bytes read, a success flag (false if buffer was exhausted), and the rest of the buffer
inline eat_bytes_result eat_bytes_until(bytes buffer, byte delim) {
    bytes p = buffer;
    while (p.Count >= 4) {
        if (U32_HAS_BYTE(*(u32 *) p.Data, delim)) break;
        advance_bytes(&p, 4);
    }
    while (p.Count) {
        if (p[0] == delim) return {bytes(buffer.Data, p.Data - buffer.Data), true, p};
        advance_bytes(&p, 1);
    }
    return {{}, false, buffer};
}

// Returns: the bytes read, a success flag (false if buffer was exhausted), and the rest of the buffer
inline eat_bytes_result eat_bytes_until_any_of(bytes buffer, bytes anyOfTheseDelims) {
    // @Speed Benchmark this!
    byte minb = 255, maxb = 0;
    For(anyOfTheseDelims) {
        if (it < minb) minb = it;
        if (it > maxb) maxb = it;
    }

    bytes p = buffer;
    while (p.Count >= 4) {
        if (U32_LIKELY_HAS_BYTE_BETWEEN(*(u32 *) p.Data, minb, maxb)) {
            if (U32_HAS_BYTE_BETWEEN(*(u32 *) p.Data, minb, maxb)) {
                break;
            }
        }
        advance_bytes(&p, 4);
    }

    while (p.Count) {
        if (has(anyOfTheseDelims, p[0])) return {bytes(buffer.Data, p.Data - buffer.Data), true, p};
        advance_bytes(&p, 1);
    }
    return {{}, false, buffer};
}

// Returns: the bytes read, a success flag (false if buffer was exhausted), and the rest of the buffer
inline eat_bytes_result eat_bytes_while(bytes buffer, byte eats) {
    bytes p = buffer;
    while (p.Count >= 4) {
        if (!(U32_HAS_BYTE(*(u32 *) p.Data, eats))) break;
        advance_bytes(&p, 4);
    }
    while (p.Count) {
        if (p[0] != eats) return {bytes(buffer.Data, p.Data - buffer.Data), true, p};
        advance_bytes(&p, 1);
    }
    return {{}, false, buffer};
}

// Returns: the bytes read, a success flag (false if buffer was exhausted), and the rest of the buffer
inline eat_bytes_result eat_bytes_while_any_of(bytes buffer, bytes anyOfTheseEats) {
    // @Speed Benchmark this!
    byte minb = 255, maxb = 0;
    For(anyOfTheseEats) {
        if (it < minb) minb = it;
        if (it > maxb) maxb = it;
    }

    bytes p = buffer;
    while (p.Count >= 4) {
        if (!(U32_LIKELY_HAS_BYTE_BETWEEN(*(u32 *) p.Data, minb, maxb))) {
            if (!(U32_HAS_BYTE_BETWEEN(*(u32 *) p.Data, minb, maxb))) {
                break;
            }
        }
        advance_bytes(&p, 4);
    }

    while (p.Count) {
        if (!has(anyOfTheseEats, p[0])) return {bytes(buffer.Data, p.Data - buffer.Data), true, p};
        advance_bytes(&p, 1);
    }
    return {{}, false, buffer};
}

// Returns the code point, a status, and the rest of the buffer.
// Note: This checks for validity of the input!
//
// Status is: PARSE_SUCCESS, PARSE_INVALID (buffer contained invalid utf8), PARSE_EXHAUSTED (we ran out of bytes)
//
// :ParseInvalidConsumesByte
// Read the doc in _eat_code_points_until_!
inline parse_result<utf32> eat_code_point(bytes buffer) {
    if (!buffer) return {0, PARSE_EXHAUSTED, buffer};

    s64 sizeOfCp = get_size_of_cp((utf8 *) buffer.Data);
    if (buffer.Count < sizeOfCp) return {0, PARSE_EXHAUSTED, buffer};

    utf8 data[4]{};
    For(range(sizeOfCp)) {
        data[it] = buffer[0];
        advance_bytes(&buffer, 1);
    }
    if (!is_valid_utf8(data)) {
        advance_bytes(&buffer, 1);
        return {0, PARSE_INVALID, buffer};
    }
    return {decode_cp(data), PARSE_SUCCESS, buffer};
}

struct parse_string_result {
    string Value;
    parse_status Status;
    string Rest;
};

// Same as the buffer version (eat_bytes_until) but pays attention to utf8.
// Returns: the code points read, a status, and the rest of the buffer.
// Status is: PARSE_SUCCESS, PARSE_INVALID (buffer contained invalid utf8), PARSE_EXHAUSTED (we ran out of bytes)
//
// :ParseInvalidConsumesByte
// When PARSE_INVALID we consume one byte even though the code point might be multiple bytes.
// We do this to comforn with the rest of the standard of our parse functions of using the return value
// as a pointer to exactly where things went wrong. e.g. an handler at the all site might report an error like this:
//
//                                         .. here is a sequence that began with a byte that told us we are looking
//                                         at a 4 byte code point for example but somewhere in there there was an invalid byte
//      This was a valid utf8 string until XXXX
//                                         ^ error happened here. This is the last byte in the returned value. Rest of string in this case is "XXX".
//
inline parse_string_result eat_code_points_until(bytes buffer, utf32 delim) {
    bytes p = buffer;
    while (true) {
        auto [cp, status, rest] = eat_code_point(p);
        if (status == PARSE_EXHAUSTED) return {{}, PARSE_EXHAUSTED, (string) buffer};

        if (status == PARSE_INVALID) {
            advance_bytes(&p, 1);
            return {{string(buffer.Data, p.Data - buffer.Data + 1)}, PARSE_INVALID, (string) p};
        }

        if (cp == delim) break;

        p = rest;
    }
    return {string(buffer.Data, p.Data - buffer.Data), PARSE_SUCCESS, (string) p};
}

// Works like _eat_code_points_until_ but with multiple delimeters.
inline parse_string_result eat_code_points_until_any_of(bytes buffer, const string &anyOfTheseDelims) {
    bytes p = buffer;
    while (true) {
        auto [cp, status, rest] = eat_code_point(p);
        if (status == PARSE_EXHAUSTED) return {{}, PARSE_EXHAUSTED, (string) buffer};

        if (status == PARSE_INVALID) {
            advance_bytes(&p, 1);
            return {{string(buffer.Data, p.Data - buffer.Data + 1)}, PARSE_INVALID, (string) p};
        }

        if (has(anyOfTheseDelims, cp)) break;

        p = rest;
    }
    return {string(buffer.Data, p.Data - buffer.Data), PARSE_SUCCESS, (string) p};
}

// Same as the buffer version (eat_bytes_while) but pays attention to utf8.
// Returns: the code points read, a status, and the rest of the buffer.
// Status is: PARSE_SUCCESS, PARSE_INVALID (buffer contained invalid utf8), PARSE_EXHAUSTED (we ran out of bytes)
//
// :ParseInvalidConsumesByte
// Read the doc in _eat_code_points_until_!
inline parse_string_result eat_code_points_while(bytes buffer, utf32 eats) {
    bytes p = buffer;
    while (true) {
        auto [cp, status, rest] = eat_code_point(p);
        if (status == PARSE_EXHAUSTED) return {{}, PARSE_EXHAUSTED, (string) buffer};

        if (status == PARSE_INVALID) {
            advance_bytes(&p, 1);
            return {{string(buffer.Data, p.Data - buffer.Data + 1)}, PARSE_INVALID, (string) p};
        }

        if (cp != eats) break;

        p = rest;
    }
    return {string(buffer.Data, p.Data - buffer.Data), PARSE_SUCCESS, (string) p};
}

// Works like _eat_code_points_while_ but with multiple allowed code points.
inline parse_string_result eat_code_points_while_any_of(bytes buffer, const string &anyOfTheseEats) {
    bytes p = buffer;
    while (true) {
        auto [cp, status, rest] = eat_code_point(p);
        if (status == PARSE_EXHAUSTED) return {{}, PARSE_EXHAUSTED, (string) buffer};

        if (status == PARSE_INVALID) {
            advance_bytes(&p, 1);
            return {{string(buffer.Data, p.Data - buffer.Data + 1)}, PARSE_INVALID, (string) p};
        }

        if (!has(anyOfTheseEats, cp)) break;

        p = rest;
    }
    return {string(buffer.Data, p.Data - buffer.Data), PARSE_SUCCESS, (string) p};
}

struct eat_white_space_result {
    parse_status Status;
    string Rest;
};

// Returns: the status, and the rest of the buffer.
// Status is: PARSE_SUCCESS, PARSE_INVALID (buffer contained invalid utf8), PARSE_EXHAUSTED (we ran out of bytes)
//
// :ParseInvalidConsumesByte
// Read the doc in _eat_code_points_until_!
inline eat_white_space_result eat_white_space(const string &str) {
    bytes p = (bytes) str;
    while (true) {
        auto [cp, status, rest] = eat_code_point(p);
        if (status == PARSE_EXHAUSTED) return {PARSE_EXHAUSTED, str};
        if (status == PARSE_INVALID) {
            advance_bytes(&p, 1);
            return {PARSE_INVALID, (string) p};
        }

        if (!is_space(cp)) break;

        p = rest;
    }
    return {PARSE_SUCCESS, (string) p};
}

LSTD_END_NAMESPACE
