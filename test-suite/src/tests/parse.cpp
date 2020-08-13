#include <lstd/parse.h>

#include "test.h"

#define test_parse_integer(IntT, options, base, buffer, expectedValue, expectedStatus, expectedRest) \
{                                                                                                \
auto [value, status, rest] = parse_integer<IntT, options>((string) buffer, base);            \
assert_eq(value, expectedValue);                                                             \
assert_eq(status, expectedStatus);                                                           \
assert_eq(rest, (array<char>) (string) expectedRest);                                        \
}

constexpr auto signs_not_allowed = parse_int_options(byte_to_digit_default, false, true, false, parse_int_options::BAIL);
constexpr auto plus_not_allowed = parse_int_options(byte_to_digit_default, true, false, false, parse_int_options::BAIL);
constexpr auto base_prefixes = parse_int_options(byte_to_digit_default, true, true, true, parse_int_options::BAIL);
constexpr auto ignore_overflow_or_underflow = parse_int_options(byte_to_digit_default, true, true, false, parse_int_options::CONTINUE);

TEST(integers) {
    test_parse_integer(s32, &parse_int_options_default, 10, "", 0, PARSE_EXHAUSTED, "");
    
    test_parse_integer(u64, &parse_int_options_default, 10, "+", 0, PARSE_EXHAUSTED, "+");
    test_parse_integer(u64, &parse_int_options_default, 10, "-", 0, PARSE_EXHAUSTED, "-");
    
    test_parse_integer(s32, &parse_int_options_default, 10, "-10101aaa", -10101, PARSE_SUCCESS, "aaa");
    test_parse_integer(s32, &parse_int_options_default, 10, "+00001aaa", +1, PARSE_SUCCESS, "aaa");
    test_parse_integer(u32, &parse_int_options_default, 16, "-cafeF00D-", 0 - 0xcafef00d, PARSE_SUCCESS, "-");
    test_parse_integer(u32, &parse_int_options_default, 16, "ffffffff", 0xffffffff, PARSE_SUCCESS, "");
    
    test_parse_integer(u64, &parse_int_options_default, 16, "fedCBA0123456789++", 0xfedcba0123456789, PARSE_SUCCESS, "++");
    
    test_parse_integer(s32, &signs_not_allowed, 10, "+2", 0, PARSE_INVALID, "2");
    test_parse_integer(s32, &signs_not_allowed, 10, "-2", 0, PARSE_INVALID, "2");
    
    test_parse_integer(s32, &plus_not_allowed, 10, "+01aaa", 0, PARSE_INVALID, "01aaa");
    
    test_parse_integer(s32, &base_prefixes, 10, "0x", 0, PARSE_EXHAUSTED, "0x");
    test_parse_integer(s32, &base_prefixes, 10, "0", 0, PARSE_EXHAUSTED, "0");
    
    test_parse_integer(s32, &base_prefixes, 10, "+0xff", 0xff, PARSE_SUCCESS, "");
    test_parse_integer(s32, &base_prefixes, 10, "-0712", -0712, PARSE_SUCCESS, "");
    
    test_parse_integer(s32, &parse_int_options_default, 10, "1000000000000000000000000", numeric_info<s32>::max(), PARSE_TOO_MANY_DIGITS, "00000000000000");
    test_parse_integer(s32, &parse_int_options_default, 10, "-1000000000000000000000000", numeric_info<s32>::min(), PARSE_TOO_MANY_DIGITS, "00000000000000");
    
    test_parse_integer(s32, &ignore_overflow_or_underflow, 10, "1000000000000000000000000", -1593835520, PARSE_SUCCESS, "");
    test_parse_integer(s32, &ignore_overflow_or_underflow, 10, "-1000000000000000000000000", 1593835520, PARSE_SUCCESS, "");
}

#define test_parse_bool(options, buffer, expectedValue, expectedStatus, expectedRest) \
{                                                                                 \
auto [value, status, rest] = parse_bool<options>((string) buffer);            \
assert_eq(value, expectedValue);                                              \
assert_eq(status, expectedStatus);                                            \
assert_eq(rest, (array<char>) (string) expectedRest);                         \
}

constexpr auto words_not_allowed = parse_bool_options(true, false, false);
constexpr auto numbers_not_allowed = parse_bool_options(false, true, false);
constexpr auto ignore_case = parse_bool_options(false, true, true);

TEST(bools) {
    test_parse_bool(&parse_bool_options_default, "", false, PARSE_EXHAUSTED, "");
    
    test_parse_bool(&parse_bool_options_default, "0", false, PARSE_SUCCESS, "");
    test_parse_bool(&parse_bool_options_default, "1", true, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "t", false, PARSE_EXHAUSTED, "t");
    test_parse_bool(&parse_bool_options_default, "tr", false, PARSE_EXHAUSTED, "tr");
    test_parse_bool(&parse_bool_options_default, "tru", false, PARSE_EXHAUSTED, "tru");
    test_parse_bool(&parse_bool_options_default, "true", true, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "tRuE", false, PARSE_INVALID, "RuE");
    test_parse_bool(&ignore_case, "tRuE", true, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "trff", false, PARSE_INVALID, "ff");
    
    test_parse_bool(&parse_bool_options_default, "f", false, PARSE_EXHAUSTED, "f");
    test_parse_bool(&parse_bool_options_default, "fa", false, PARSE_EXHAUSTED, "fa");
    test_parse_bool(&parse_bool_options_default, "fal", false, PARSE_EXHAUSTED, "fal");
    test_parse_bool(&parse_bool_options_default, "fals", false, PARSE_EXHAUSTED, "fals");
    test_parse_bool(&parse_bool_options_default, "false", false, PARSE_SUCCESS, "");
    
    test_parse_bool(&parse_bool_options_default, "falff", false, PARSE_INVALID, "ff");
    
    test_parse_bool(&parse_bool_options_default, "falSe", false, PARSE_INVALID, "Se");
    test_parse_bool(&ignore_case, "falSe", false, PARSE_SUCCESS, "");
    
    test_parse_bool(&numbers_not_allowed, "0", false, PARSE_INVALID, "0");
    test_parse_bool(&numbers_not_allowed, "1", false, PARSE_INVALID, "1");
    
    test_parse_bool(&words_not_allowed, "true", false, PARSE_INVALID, "true");
    test_parse_bool(&words_not_allowed, "false", false, PARSE_INVALID, "false");
}
