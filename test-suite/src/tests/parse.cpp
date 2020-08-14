#include <lstd/parse.h>

#include "test.h"

#define test_parse_int(IntT, options, base, buffer, expectedValue, expectedStatus, expectedRest) \
{                                                                                            \
auto [value, status, rest] = parse_int<IntT, options>((string) buffer, base);            \
assert_eq(value, expectedValue);                                                         \
assert_eq(status, expectedStatus);                                                       \
assert_eq(rest, (array<char>) (string) expectedRest);                                    \
}

constexpr auto signs_not_allowed = parse_int_options(byte_to_digit_default, false, true, false);
constexpr auto plus_not_allowed = parse_int_options(byte_to_digit_default, true, false, false);
constexpr auto base_prefixes = parse_int_options(byte_to_digit_default, true, true, true);
constexpr auto ignore_overflow_or_underflow = parse_int_options(byte_to_digit_default, true, true, false, parse_int_options::CONTINUE);

TEST(int) {
    test_parse_int(s32, &parse_int_options_default, 10, "", 0, PARSE_EXHAUSTED, "");
    
    test_parse_int(u64, &parse_int_options_default, 10, "+", 0, PARSE_EXHAUSTED, "+");
    test_parse_int(u64, &parse_int_options_default, 10, "-", 0, PARSE_EXHAUSTED, "-");
    
    test_parse_int(s32, &parse_int_options_default, 10, "-10101aaa", -10101, PARSE_SUCCESS, "aaa");
    test_parse_int(s32, &parse_int_options_default, 10, "+00001aaa", +1, PARSE_SUCCESS, "aaa");
    test_parse_int(u32, &parse_int_options_default, 16, "-cafeF00D-", 0 - 0xcafef00d, PARSE_SUCCESS, "-");
    test_parse_int(u32, &parse_int_options_default, 16, "ffffffff", 0xffffffff, PARSE_SUCCESS, "");
    
    test_parse_int(u64, &parse_int_options_default, 16, "fedCBA0123456789++", 0xfedcba0123456789, PARSE_SUCCESS, "++");
    
    test_parse_int(s32, &signs_not_allowed, 10, "+2", 0, PARSE_INVALID, "2");
    test_parse_int(s32, &signs_not_allowed, 10, "-2", 0, PARSE_INVALID, "2");
    
    test_parse_int(s32, &plus_not_allowed, 10, "+01aaa", 0, PARSE_INVALID, "01aaa");
    
    test_parse_int(s32, &base_prefixes, 10, "0x", 0, PARSE_EXHAUSTED, "0x");
    test_parse_int(s32, &base_prefixes, 10, "0", 0, PARSE_EXHAUSTED, "0");
    
    test_parse_int(s32, &base_prefixes, 10, "+0xff", 0xff, PARSE_SUCCESS, "");
    test_parse_int(s32, &base_prefixes, 10, "-0712", -0712, PARSE_SUCCESS, "");
    
    test_parse_int(s32, &parse_int_options_default, 10, "1000000000000000000000000", numeric_info<s32>::max(), PARSE_TOO_MANY_DIGITS, "00000000000000");
    test_parse_int(s32, &parse_int_options_default, 10, "-1000000000000000000000000", numeric_info<s32>::min(), PARSE_TOO_MANY_DIGITS, "00000000000000");
    
    test_parse_int(s32, &ignore_overflow_or_underflow, 10, "1000000000000000000000000", -1593835520, PARSE_SUCCESS, "");
    test_parse_int(s32, &ignore_overflow_or_underflow, 10, "-1000000000000000000000000", 1593835520, PARSE_SUCCESS, "");
}

#define test_parse_bool(options, buffer, expectedValue, expectedStatus, expectedRest) \
{                                                                                 \
auto [value, status, rest] = parse_bool<options>((string) buffer);            \
assert_eq(value, expectedValue);                                              \
assert_eq(status, expectedStatus);                                            \
assert_eq(rest, (array<char>) (string) expectedRest);                         \
}

constexpr auto words_not_allowed = parse_bool_options(true, false);
constexpr auto numbers_not_allowed = parse_bool_options(false, true);
constexpr auto dont_ignore_case = parse_bool_options(false, true, false);

TEST(bool) {
    test_parse_bool(&parse_bool_options_default, "", false, PARSE_EXHAUSTED, "");
    
    test_parse_bool(&parse_bool_options_default, "0", false, PARSE_SUCCESS, "");
    test_parse_bool(&parse_bool_options_default, "1", true, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "t", false, PARSE_EXHAUSTED, "t");
    test_parse_bool(&parse_bool_options_default, "tr", false, PARSE_EXHAUSTED, "tr");
    test_parse_bool(&parse_bool_options_default, "tru", false, PARSE_EXHAUSTED, "tru");
    test_parse_bool(&parse_bool_options_default, "true", true, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "tRuE", true, PARSE_SUCCESS, "");
    test_parse_bool(&dont_ignore_case, "tRuE", false, PARSE_INVALID, "RuE");
    
    test_parse_bool(&parse_bool_options_default, "trff", false, PARSE_INVALID, "ff");
    
    test_parse_bool(&parse_bool_options_default, "f", false, PARSE_EXHAUSTED, "f");
    test_parse_bool(&parse_bool_options_default, "fa", false, PARSE_EXHAUSTED, "fa");
    test_parse_bool(&parse_bool_options_default, "fal", false, PARSE_EXHAUSTED, "fal");
    test_parse_bool(&parse_bool_options_default, "fals", false, PARSE_EXHAUSTED, "fals");
    test_parse_bool(&parse_bool_options_default, "false", false, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "falff", false, PARSE_INVALID, "ff");
    
    test_parse_bool(&parse_bool_options_default, "falSe", false, PARSE_SUCCESS, "");
    test_parse_bool(&dont_ignore_case, "falSe", false, PARSE_INVALID, "Se");
    
    test_parse_bool(&numbers_not_allowed, "0", false, PARSE_INVALID, "0");
    test_parse_bool(&numbers_not_allowed, "1", false, PARSE_INVALID, "1");
    
    test_parse_bool(&words_not_allowed, "true", false, PARSE_INVALID, "true");
    test_parse_bool(&words_not_allowed, "false", false, PARSE_INVALID, "false");
}

TEST(guid) {
    // 2d043ca6-06cd-44c0-8009-e0b1ea72ebf9
    guid theGuid = {0x2d, 0x04, 0x3c, 0xa6, 0x06, 0xcd, 0x44, 0xc0, 0x80, 0x09, 0xe0, 0xb1, 0xea, 0x72, 0xeb, 0xf9};
    
    array<char> formats = {
        'n', 'N', 'd', 'D', 'b', 'B', 'p', 'P', 'x', 'X'};
    
    // Random stuff we will append after the string to check _rest_
    array<string> garbage = {
        "", "--", ")()-", "0xff", "cafef00d", "deadbeef"};
    
    For_as(f, formats) {
        For_as(g, garbage) {
            string format = fmt::sprint("{{:{:c}}}{}", f, g);
            defer(format.release());
            
            string guidFormatted = fmt::sprint(format, theGuid);
            defer(guidFormatted.release());
            
            auto [parsedGuid, status, rest] = parse_guid(guidFormatted);
            
            if (theGuid != parsedGuid) {
                int a = 42;
            }
            
            assert_eq(theGuid, parsedGuid);
            assert_eq(status, PARSE_SUCCESS);
            assert_eq(rest, (array<char>) g);
        }
    }
}
