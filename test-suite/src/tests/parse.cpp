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
    test_parse_integer(u64, &parse_int_options_default, 10, "+", 0, PARSE_INVALID, "");
    test_parse_integer(u64, &parse_int_options_default, 10, "-", 0, PARSE_INVALID, "");

    test_parse_integer(s32, &parse_int_options_default, 10, "-10101aaa", -10101, PARSE_SUCCESS, "aaa");
    test_parse_integer(s32, &parse_int_options_default, 10, "+00001aaa", +1, PARSE_SUCCESS, "aaa");
    test_parse_integer(u32, &parse_int_options_default, 16, "-cafeF00D-", 0 - 0xcafef00d, PARSE_SUCCESS, "-");
    test_parse_integer(u32, &parse_int_options_default, 16, "ffffffff", 0xffffffff, PARSE_SUCCESS, "");

    test_parse_integer(u64, &parse_int_options_default, 16, "fedCBA0123456789++", 0xfedcba0123456789, PARSE_SUCCESS, "++");

    test_parse_integer(s32, &signs_not_allowed, 10, "+2", 0, PARSE_INVALID, "2");
    test_parse_integer(s32, &signs_not_allowed, 10, "-2", 0, PARSE_INVALID, "2");

    test_parse_integer(s32, &plus_not_allowed, 10, "+01aaa", 0, PARSE_INVALID, "01aaa");

    test_parse_integer(s32, &base_prefixes, 10, "0x", 0, PARSE_INVALID, "");
    test_parse_integer(s32, &base_prefixes, 10, "0", 0, PARSE_INVALID, "");

    test_parse_integer(s32, &base_prefixes, 10, "+0xff", 0xff, PARSE_SUCCESS, "");
    test_parse_integer(s32, &base_prefixes, 10, "-0712", -0712, PARSE_SUCCESS, "");

    test_parse_integer(s32, &parse_int_options_default, 10, "1000000000000000000000000", numeric_info<s32>::max(), PARSE_TOO_MANY_DIGITS, "00000000000000");
    test_parse_integer(s32, &parse_int_options_default, 10, "-1000000000000000000000000", numeric_info<s32>::min(), PARSE_TOO_MANY_DIGITS, "00000000000000");

    test_parse_integer(s32, &ignore_overflow_or_underflow, 10, "1000000000000000000000000", -1593835520, PARSE_SUCCESS, "");
    test_parse_integer(s32, &ignore_overflow_or_underflow, 10, "-1000000000000000000000000", 1593835520, PARSE_SUCCESS, "");
}
