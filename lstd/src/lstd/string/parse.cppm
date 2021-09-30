module;

#include "../common.h"

export module lstd.parse;

export import lstd.string;
export import lstd.big_integer;
export import lstd.guid;

LSTD_BEGIN_NAMESPACE

export {
    enum parse_status : u32 {
        // We have ran out of buffer.
        // This is == to returning PARSE_INVALID with an empty _Rest_.
        // We used to treat it as a seperate status but this just complicates code.
        // PARSE_EXHAUSTED,

        // Means the input was malformed/in
        // the wrong format.
        PARSE_INVALID = 0,

        PARSE_SUCCESS = 1,

        // Used in _parse_int_ when the
        // resulting value overflowed or underflowed.
        PARSE_TOO_MANY_DIGITS = 2
    };

    // Used in parse functions for some special behaviour/special return values
    constexpr code_point CP_INVALID = 0xfffd;
    constexpr code_point CP_MAX     = 0x7fffffff;

    // End of file and ignore this byte don't exist as real unicode code points.
    // They are just symbolic, so we them a value outside the range of possible code points.
    constexpr code_point CP_EOF         = CP_MAX + 1;
    constexpr code_point CP_IGNORE_THIS = CP_MAX + 2;

    // This maps a code point to a numerical value for parsing numbers
    using cp_to_digit_t = s32 (*)(code_point, bool);

    // _first_ is true if _cp_ was the first parsed thing after +/- and base prefixes.
    // This can be used to disallow numbers starting with certain symbols (e.g. ,)
    s32 cp_to_digit_default(code_point cp, bool first = false) {
        if (cp >= '0' && cp <= '9') return cp - '0';
        if (cp >= 'a' && cp <= 'z') return cp - 'a' + 10;
        if (cp >= 'A' && cp <= 'Z') return cp - 'A' + 10;
        return CP_INVALID;
    }

    // Unsafe, doesn't check bounds
    void advance_bytes(string * p, s64 count) {
        p->Data += count;
        p->Count -= count;
    }

    // Unsafe, doesn't check bounds
    void advance_cp(string * p, s64 count) {
        while (count--) {
            s64 c = utf8_get_size_of_cp(p->Data);
            p->Data += c;
            p->Count -= c;
        }
    }

    // If negative is true:
    //   * returns '0 - value' when _value_ is an unsigned integral
    //   * returns '-value' when _value_ is a signed integral
    // otherwise returns 'value'.
    auto handle_negative(types::is_integral auto value, bool negative) {
        using T = decltype(value);
        if (negative) {
            if constexpr (types::is_unsigned_integral<T>) {
                return T(0 - value);
            } else {
                return T(-value);
            }
        }
        return T(value);
    }

    //
    // This struct is used to conditionally compile _parse_int_.
    // Normally writing a function that has these as arguments means that it will become quite large and slow because it has to handle all cases.
    // These options determine which code paths of _parse_int_ get compiled and thus don't have effect on runtime performance.
    //
    // :CodeReusability:
    // Hopefully this makes _parse_int_ a lot more usable since users don't have to write an optimized version for their own use cases.
    //
    struct parse_int_options {
        bool ParseSign     = true;  // If true, looks for +/- before trying to parse any digits. If '-' the result is negated.
        bool AllowPlusSign = true;  // If true, allows explicit + as a sign, if false, results in parse failure.

        //
        // We use _CodePointToDigit_ to determine the numerical value symbols (as well as which symbols to ignore/are invalid).
        // We support all bases up to base 16 (ignoring the case for letters). You can roll your own implementation for other bases.
        //
        // If _CodePointToDigit_ returns a value that's bigger than the _base_ the caller specified (or was
        // overriden by a base prefix) we break in the parse loop. So there's no need to pay attention to that in the routine.
        //
        // For any code point that doesn't correspond to a digit, return CP_INVALID
        // and if the parser should ignore the byte but not fail parsing, return CP_IGNORE_THIS.
        //
        // Here are two use cases which illustrate that:
        //
        //     // This cp to digit function supports only decimal and allows parsing '1_000_000' as 1 million because it tells the parser to ignore '_'.
        //     // You can also write a function which ignores commas, using this to parse numbers with a thousands separator: '1,000,000'.
        //     // This callback can also be any delegate (including lambdas with state) so you could check for example if commas are placed
        //     // after every third digit. The parameter _first_ tells whether this is the first code point (after +/- and base prefix)
        //     // which is parsed as a digit. Here we ignore underscores but don't allow a number to start with a _.
        //     u32 cp_to_digit_dec_and_ignore_underscores(code_point cp, u32 base, bool first) {
        //         assert(base == 10);
        //         if (cp >= '0' && cp <= '9') return cp - '0';
        //         if (cp == '_') return first ? CP_INVALID : CP_IGNORE_THIS;
        //         return CP_INVALID;
        //     }
        //
        //     // Allows parsing a base 64 integer [0-9a-zA-Z#_]* (doesn't support = as padding)
        //     u32 cp_to_digit_base_64(code_point cp, u32 base, bool first) {
        //         assert(base == 64);
        //         if (cp >= '0' && cp <= '9') return cp - '0';
        //         if (cp >= 'a' && cp <= 'z') return cp - 'a' + 10;
        //         if (cp >= 'A' && cp <= 'Z') return cp - 'A' + 10 + 26;
        //         if (cp == '#') return 63;
        //         if (cp == '_') return 64;
        //         return CP_INVALID;
        //     }
        //
        cp_to_digit_t CodePointToDigit = cp_to_digit_default;

        // By default the base is determined by _CodePointToDigit_ (since it returns CP_INVALID when encountering a banned value).
        // If _LookForBasePrefix_ is set, we look for 0x and 0 for hex and oct respectively and if found, we overwrite _CodePointToDigit_.
        bool LookForBasePrefix = false;

        // Set maximum amount of digits which should be parsed. -1 means unspecified (no limit).
        // Note: The return value is PARSE_SUCCESS and not PARSE_TOO_MANY_DIGITS if there were more digits left to parse.
        // Note: CP_IGNORE_THIS from CodePointToDigit is counted towards this.
        // Note: The base prefix (if parsed) is counted towards this.
        s64 MaxDigits = -1;
        // Potentially useful?
        // bool IgnoreThisCodePointCountedTowardsMaxDigits = false;
        // bool BasePrefixCountedTowardsMaxDigits = false;

        // If we should stop parsing when an overflow happens and bail out of the function.
        // Otherwise parse as much digits as possible while ignoring the overflow/underflow.
        bool BailOnTooManyDigits = true;

        // By default we return the min/max value of the integer type when we bail because we parsed too many digits.
        // If false, the returned value is the value we have parsed so far.
        bool ReturnLimitOnTooManyDigits = true;
    };

    template <typename IntT>
    struct parse_result {
        IntT Value;

        parse_status Status;

        // Returns the rest of the buffer after we ate some bytes.
        // If _Status_ was PARSE_INVALID:
        //    - If this is empty, it means that we ran out of bytes to parse;
        //    - otherwise it contains at least the invalid byte (we don't eat it).
        string Rest;
    };

#define FAIL \
    { 0, PARSE_INVALID, p }

    // Parses 8, 16, 32, 64 or 128 bit numbers after sign and base prefix has been handled. Called from parse_int.
    template <types::is_integral T, parse_int_options Options>
    parse_result<T> parse_int_small_integer(string p, u32 base, bool parsedNegative) {
        T maxValue, cutOff;
        s32 cutLim;
        if constexpr (Options.BailOnTooManyDigits) {
            // Determine at what point we stop parsing because the number becomes too big
            if constexpr (types::is_unsigned_integral<T>) {
                maxValue = (numeric_info<T>::max) ();
                cutOff   = maxValue / base;
            } else {
                maxValue = parsedNegative ? -numeric_info<T>::min() : numeric_info<T>::max();
                cutOff   = maxValue / base;
                cutOff   = abs(cutOff);
            }
            cutLim = maxValue % (T) base;
        }

        s64 maxDigits = Options.MaxDigits;
        if constexpr (Options.MaxDigits != -1) {
            assert(maxDigits > 0);
        }

        bool firstDigit = true;

        // Now start doing the real work
        T value = 0;
        while (true) {
            if constexpr (Options.MaxDigits != -1) {
                if (!maxDigits) break;
                --maxDigits;
            }

            s32 digit = p ? Options.CodePointToDigit(p[0], firstDigit) : CP_INVALID;
            advance_cp(&p, 1);

            if (digit == CP_IGNORE_THIS) continue;

            if (digit < 0 || digit >= (s32) base) {
                // We have CP_INVALID or a digit that's outside our base, break.

                if (firstDigit) {
                    // We have a special case for when we have parsed a base 8 prefix but the whole valid integer is just one 0,
                    // then we return 0 and don't treat it as an oct value (because in that case we require more digits).
                    if (Options.LookForBasePrefix) {
                        advance_bytes(&p, -1);
                        if (base == 8) return {0, PARSE_SUCCESS, p};
                    }
                    return FAIL;
                }

                // Roll back the invalid byte we consumed.
                // After the break we return PARSE_SUCCESS.
                // We only consume the invalid byte if we return PARSE_INVALID.
                advance_bytes(&p, -1);
                break;
            }

            firstDigit = false;  // @Cleanup

            if constexpr (Options.BailOnTooManyDigits) {
                // If we have parsed a number that is too big to store in our integer type we bail
                if (value > cutOff || value == cutOff && digit > cutLim) {
                    if constexpr (Options.ReturnLimitOnTooManyDigits) value = maxValue;
                    return {handle_negative(value, parsedNegative), PARSE_TOO_MANY_DIGITS, p};
                }

                // If _BailOnTooManyDigits_ is false then we don't execute the code above and
                // continue parsing (while overflowing) until all digits have been read from the buffer.
            }
            value = value * base + digit;
        }

        return {handle_negative(value, parsedNegative), PARSE_SUCCESS, p};
    }

    template <parse_int_options Options>
    parse_result<big_integer> parse_int_big_integer(string p, u32 base) {
        assert(false);  // @TODO
        return {0, PARSE_SUCCESS, p};
    }

    // Attemps to parse an integer of a type T (including big integers which have practically infinite range).
    // This is a very general and light function.
    //
    // Allows for compilation of different code paths using a template parameter which is a struct literal (parse_int_options).
    // It define the function which maps code points to digits as well as options for how we should handle signs,
    // base prefixes and integer overflow/underflow.
    //
    // This function doesn't eat white space from the beginning.
    //
    // By default we try to do the most sensible thing: valid integers start with either +/- and then a base prefix (0 or 0x for oct/hex)
    // and then a range of 0-9a-zA-Z which describes digits. We stop parsing when we encounter a byte which is not a valid digit.
    //
    // See comments in _parse_int_options_.
    //
    // By default we stop parsing when the resulting integer overflows/underflows instead of greedily consuming the rest of the digits.
    // In that case the returned status is PARSE_TOO_MANY_DIGITS. To change this behaviour, change the _BailOnTooManyDigits_ option.
    // If you set it to false then all digits are consumed while ignoring the overflow/underflow (and the parse status returned is SUCCESS).
    //
    // Returns:
    //   * PARSE_SUCCESS          if a valid integer was parsed. A valid integer is in the form (+|-)[digit]*
    //                            (where digit may be a letter depending on the base and Options.CodePointToDigit).
    //   * PARSE_INVALID          if the function wasn't able to parse a valid integer.
    //                            Note: If the integer starts with +/-, that could be considered invalid (depending on Options.ParseSign).
    //   * PARSE_TOO_MANY_DIGITS  if the parsing stopped because the integer became too large (only if Options.BailOnTooManyDigits and
    //                            we aren't parsing a big_integer which handles practically infinite digits). In that case the max value
    //                            of the integer type is returned (min value if parsing a negative number).
    //
    template <types::is_integral T, parse_int_options Options = parse_int_options{}>
    parse_result<T> parse_int(string buffer, u32 base = 10) {
        string p = buffer;
        if (!p) return FAIL;

        bool negative = false;
        if constexpr (Options.ParseSign) {
            if (p[0] == '+') {
                advance_bytes(&p, 1);
                if constexpr (!Options.AllowPlusSign) return FAIL;
            } else if (p[0] == '-') {
                negative = true;
                advance_bytes(&p, 1);
            }
            if (!p) return FAIL;
        }

        if constexpr (Options.LookForBasePrefix) {
            if (p[0] == '0') {
                if (p.Count - 1 && (p[1] == 'x' || p[1] == 'X')) {
                    base = 16;
                    advance_bytes(&p, 2);
                } else {
                    base = 8;
                    advance_bytes(&p, 1);
                }
            }
            if (!p) return FAIL;
        }

        if constexpr (types::is_same<T, big_integer>) {
            return parse_int_big_integer<T, Options>(p, base, negative);
        } else {
            return parse_int_small_integer<T, Options>(p, base, negative);
        }
    }

    template <bool IgnoreCase = false>
    bool expect_cp(string * p, code_point value) {
        if (!*p) return false;

        code_point ch = (*p)[0];
        if constexpr (IgnoreCase) ch = to_lower(ch);

        if (ch == value) {
            advance_cp(p, 1);
            return true;
        } else {
            return false;
        }
    }

    template <bool IgnoreCase = false>
    bool expect_sequence(string * p, string sequence) {
        For(sequence) {
            bool status = expect_cp<IgnoreCase>(p, it);
            if (!status) return false;
        }
        return true;
    }

    // Similar to parse_int, these options compile different versions of parse_bool and turn off certain code paths.
    struct parse_bool_options {
        bool ParseNumbers         = true;  // Attemps to parse 0/1.
        bool ParseWords           = true;  // Attemps to parse the words "true" and "false".
        bool ParseWordsIgnoreCase = true;  // Ignores case when parsing the words.
    };

    //
    // Attemps to parse a bool.
    //
    // There are 3 return values: the value parsed, the parse status and the rest of the buffer after consuming some characters for the parsing.
    // If the status was PARSE_INVALID then some bytes could have been consumed. E.g. when parsing words and the buffer was "truDD"
    // _rest_ is "DD".
    //
    // Allows for compilation of different code paths using a template parameter which is a struct literal (parse_bool_options).
    //
    // This function doesn't eat white space from the beginning.
    //
    // Returns:
    //   * PARSE_SUCCESS          if a valid bool was parsed (0/1 or  "true"/"false", depending on the options)
    //   * PARSE_EXHAUSTED        if an empty buffer was passed or we started parsing a word but ran out of bytes
    //   * PARSE_INVALID          if the function wasn't able to parse a valid bool
    //
    template <parse_bool_options Options = parse_bool_options{}>
    parse_result<bool> parse_bool(string buffer) {
        static_assert(Options.ParseNumbers || Options.ParseWords);  // Sanity, one of them must be set

#define SUCCESS(x) {x, PARSE_SUCCESS, p};

        string p = buffer;
        if (!p) return FAIL;

        if constexpr (Options.ParseNumbers) {
            if (p[0] == '0') {
                advance_cp(&p, 1);
                return SUCCESS(false);
            }
            if (p[0] == '1') {
                advance_cp(&p, 1);
                return SUCCESS(true);
            }
        }

        if constexpr (Options.ParseWords) {
            if (p[0] == 't') {
                parse_status status = expect_sequence<Options.ParseWordsIgnoreCase>(&p, (string) "true");
                if (!status) return FAIL;
                return SUCCESS(true);
            }

            if (p[0] == 'f') {
                parse_status status = expect_sequence<Options.ParseWordsIgnoreCase>(&p, (string) "false");
                if (!status) return FAIL;
                return SUCCESS(true);
            }
        }
        return FAIL;
    }

    struct eat_hex_byte_result {
        u8 Value;
        bool Status;
    };

    // Tries to parse exactly two hex digits as a byte.
    // We don't eat if parsing fails.
    eat_hex_byte_result eat_hex_byte(string * p) {
        auto [value, status, rest] = parse_int<u8, parse_int_options{.ParseSign = false, .MaxDigits = 2, .ReturnLimitOnTooManyDigits = false}>(*p, 16);
        *p                         = rest;
        return {value, (bool) status};
    }

    struct parse_guid_options {
        // Do we handle formats starting with parentheses - ( or {.
        bool Parentheses = true;

        // Doesn't pay attention to the position or the number of hyphens in the input, just ignores them.
        // This makes parsing go faster when you don't care if the input is partially incorrect or you know it is not!
        bool RelaxHyphens = false;
    };

    //
    // Parses the following GUID representations:
    // - 81a130d2502f4cf1a37663edeb000e9f
    // - 81a130d2-502f-4cf1-a376-63edeb000e9f
    // - {81a130d2-502f-4cf1-a376-63edeb000e9f}
    // - (81a130d2-502f-4cf1-a376-63edeb000e9f)
    // - {0x81a130d2,0x502f,0x4cf1,{0xa3,0x76,0x63,0xed,0xeb,0x00,0x0e,0x9f}}
    //
    // Doesn't pay attention to capitalization (both uppercase/lowercase/mixed are valid).
    //
    // Returns: the guid parsed, a status, and the rest of the buffer
    //
    template <parse_guid_options Options = parse_guid_options{}>
    parse_result<guid> parse_guid(string buffer) {
        guid empty;

#undef FAIL
#define FAIL {empty, PARSE_INVALID, p}

        string p = buffer;
        if (!p) return FAIL;

        bool parentheses = false, curly = false;
        if constexpr (Options.Parentheses) {
            if (p[0] == '(' || p[0] == '{') {
                parentheses = true;
                curly       = p[0] == '{';
                advance_cp(&p, 1);
                if (!p) return FAIL;
            }
        }

        guid result;

        if (p.Count - 1 && p[0] == '0') {
            // Parse following format:
            // {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
            // We choose that format if the first byte begins with 0x.
            if (p[1] == 'x' || p[1] == 'X') {
                if (!parentheses || !curly) {
                    // In this case the error is that there is 0x but we didn't start with a {
                    return FAIL;
                }

                parse_status status;

                auto *resultBuffer = &result.Data[0];

#define EAT_HEX_BYTES(count)                     \
    For(range(count)) {                          \
        auto [value, status] = eat_hex_byte(&p); \
        if (!status) return FAIL;                \
        *resultBuffer++ = value;                 \
    }
#define EXPECT_SEQUENCE(sequence)                          \
    status = expect_sequence<true>(&p, (string) sequence); \
    if (!status) return FAIL;

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

        // In the case above we the format with 0x and the commas:
        //   {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
        //
        // Below we parse every other format:
        //   81a130d2502f4cf1a37663edeb000e9f
        //   81a130d2-502f-4cf1-a376-63edeb000e9f
        //   {81a130d2-502f-4cf1-a376-63edeb000e9f}
        //   (81a130d2-502f-4cf1-a376-63edeb000e9f)

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
                            advance_cp(&p, 1);
                            if (!p) return FAIL;
                        }
                    }
                }

                if (hyphens && (counter == 6 || counter == 8 || counter == 10)) {
                    if (p[0] == '-') {
                        advance_cp(&p, 1);
                        if (!p) return FAIL;
                    } else {
                        return FAIL;
                    }
                }
            } else {
                if (p[0] == '-') {
                    advance_cp(&p, 1);
                    if (!p) return FAIL;
                }
            }

            auto [value, status] = eat_hex_byte(&p);
            if (!status) return FAIL;
            *resultBuffer++ = value;

            ++counter;

            // We have eaten 16 hex bytes.
            if (counter == 16) break;
        }

        // Expect a closing parenthesis
        if constexpr (Options.Parentheses) {
            if (parentheses) {
                bool status = expect_cp(&p, curly ? '}' : ')');
                if (!status) return FAIL;
            }
        }
        return {result, PARSE_SUCCESS, p};
    }
}

LSTD_END_NAMESPACE
