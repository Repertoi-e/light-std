#include <lstd/parse.h>

#include "test.h"

#define test_parse_int(IntT, options, base, buffer, expectedValue, expectedStatus, expectedRest) \
    {                                                                                            \
        auto [value, status, rest] = parse_int<IntT, options>((string) buffer, base);            \
        assert_eq(value, expectedValue);                                                         \
        assert_eq(status, expectedStatus);                                                       \
        assert_eq(rest, (bytes)(string) expectedRest);                                           \
    }

TEST(int) {
    test_parse_int(s32, parse_int_options{}, 10, "", 0, PARSE_EXHAUSTED, "");

    test_parse_int(u64, parse_int_options{}, 10, "+", 0, PARSE_EXHAUSTED, "+");
    test_parse_int(u64, parse_int_options{}, 10, "-", 0, PARSE_EXHAUSTED, "-");

    test_parse_int(s32, parse_int_options{}, 10, "-10101aaa", -10101, PARSE_SUCCESS, "aaa");
    test_parse_int(s32, parse_int_options{}, 10, "+00001aaa", +1, PARSE_SUCCESS, "aaa");
    test_parse_int(u32, parse_int_options{}, 16, "-cafeF00D-", 0 - 0xcafef00d, PARSE_SUCCESS, "-");
    test_parse_int(u32, parse_int_options{}, 16, "ffffffff", 0xffffffff, PARSE_SUCCESS, "");

    test_parse_int(u64, parse_int_options{}, 16, "fedCBA0123456789++", 0xfedcba0123456789, PARSE_SUCCESS, "++");

    test_parse_int(s32, parse_int_options{.ParseSign = false}, 10, "+2", 0, PARSE_INVALID, "2");
    test_parse_int(s32, parse_int_options{.ParseSign = false}, 10, "-2", 0, PARSE_INVALID, "2");

    test_parse_int(s32, parse_int_options{.AllowPlusSign = false}, 10, "+01aaa", 0, PARSE_INVALID, "01aaa");

    test_parse_int(s32, parse_int_options{.LookForBasePrefix = true}, 10, "0x", 0, PARSE_EXHAUSTED, "0x");
    test_parse_int(s32, parse_int_options{.LookForBasePrefix = true}, 10, "0", 0, PARSE_EXHAUSTED, "0");

    test_parse_int(s32, parse_int_options{.LookForBasePrefix = true}, 10, "+0xff", 0xff, PARSE_SUCCESS, "");
    test_parse_int(s32, parse_int_options{.LookForBasePrefix = true}, 10, "-0712", -0712, PARSE_SUCCESS, "");

    test_parse_int(s32, parse_int_options{}, 10, "1000000000000000000000000", type::numeric_info<s32>::max(), PARSE_TOO_MANY_DIGITS, "00000000000000");
    test_parse_int(s32, parse_int_options{}, 10, "-1000000000000000000000000", type::numeric_info<s32>::min(), PARSE_TOO_MANY_DIGITS, "00000000000000");

    test_parse_int(s32, parse_int_options{.TooManyDigitsBehaviour = parse_int_options::CONTINUE}, 10, "1000000000000000000000000", -1593835520, PARSE_SUCCESS, "");
    test_parse_int(s32, parse_int_options{.TooManyDigitsBehaviour = parse_int_options::CONTINUE}, 10, "-1000000000000000000000000", 1593835520, PARSE_SUCCESS, "");
}

#define test_parse_bool(options, buffer, expectedValue, expectedStatus, expectedRest) \
    {                                                                                 \
        auto [value, status, rest] = parse_bool<options>((string) buffer);            \
        assert_eq(value, expectedValue);                                              \
        assert_eq(status, expectedStatus);                                            \
        assert_eq(rest, (bytes)(string) expectedRest);                                \
    }

TEST(bool) {
    test_parse_bool(parse_bool_options{}, "", false, PARSE_EXHAUSTED, "");

    test_parse_bool(parse_bool_options{}, "0", false, PARSE_SUCCESS, "");
    test_parse_bool(parse_bool_options{}, "1", true, PARSE_SUCCESS, "");

    test_parse_bool(parse_bool_options{}, "t", false, PARSE_EXHAUSTED, "t");
    test_parse_bool(parse_bool_options{}, "tr", false, PARSE_EXHAUSTED, "tr");
    test_parse_bool(parse_bool_options{}, "tru", false, PARSE_EXHAUSTED, "tru");
    test_parse_bool(parse_bool_options{}, "true", true, PARSE_SUCCESS, "");

    test_parse_bool(parse_bool_options{}, "tRuE", true, PARSE_SUCCESS, "");
    test_parse_bool(parse_bool_options{.IgnoreCase = false}, "tRuE", false, PARSE_INVALID, "RuE");

    test_parse_bool(parse_bool_options{}, "trff", false, PARSE_INVALID, "ff");

    test_parse_bool(parse_bool_options{}, "f", false, PARSE_EXHAUSTED, "f");
    test_parse_bool(parse_bool_options{}, "fa", false, PARSE_EXHAUSTED, "fa");
    test_parse_bool(parse_bool_options{}, "fal", false, PARSE_EXHAUSTED, "fal");
    test_parse_bool(parse_bool_options{}, "fals", false, PARSE_EXHAUSTED, "fals");
    test_parse_bool(parse_bool_options{}, "false", false, PARSE_SUCCESS, "");

    test_parse_bool(parse_bool_options{}, "falff", false, PARSE_INVALID, "ff");

    test_parse_bool(parse_bool_options{}, "falSe", false, PARSE_SUCCESS, "");
    test_parse_bool(parse_bool_options{.IgnoreCase = false}, "falSe", false, PARSE_INVALID, "Se");

    test_parse_bool(parse_bool_options{.ParseNumbers = false}, "0", false, PARSE_INVALID, "0");
    test_parse_bool(parse_bool_options{.ParseNumbers = false}, "1", false, PARSE_INVALID, "1");

    test_parse_bool(parse_bool_options{.ParseWords = false}, "true", false, PARSE_INVALID, "true");
    test_parse_bool(parse_bool_options{.ParseWords = false}, "false", false, PARSE_INVALID, "false");
}

TEST(guid) {
    guid guid = guid_new();

    auto formats = to_stack_array('n', 'N', 'd', 'D', 'b', 'B', 'p', 'P', 'x', 'X');

    // Random stuff we will append after the string to check _rest_
    auto garbage = to_stack_array<string>("", "--", ")()-", "0xff and cafef00d and deadbeef");

    For_as(f, formats) {
        For_as(g, garbage) {
            string format = fmt::sprint("{{:{:c}}}{}", f, g);
            defer(format.release());

            string guidFormatted = fmt::sprint(format, guid);
            defer(guidFormatted.release());

            auto [parsed, status, rest] = parse_guid(guidFormatted);
            assert_eq(guid, parsed);
            assert_eq(status, PARSE_SUCCESS);
            assert_eq(rest, (bytes) g);
        }
    }
}
