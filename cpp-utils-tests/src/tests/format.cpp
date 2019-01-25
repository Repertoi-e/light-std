#include "../test.hpp"

#include <cppu/io.hpp>

#include <limits.h>

struct Custom_Type {};

template <>
struct fmt::Formatter<Custom_Type> {
    void format(Custom_Type, Format_Context &f) { f.write("foo"); }
};

TEST(custom_types_and_to_string) {
    assert_eq(fmt::sprint("{}", Custom_Type{}), "foo");

    assert_eq(fmt::to_string(42), "42");
    assert_eq(fmt::to_string(string("foo")), "foo");
    assert_eq(fmt::to_string(string_view("foo")), "foo");

    assert_eq(fmt::to_string(false), "false");
    assert_eq(fmt::to_string(true), "true");

    assert_eq(fmt::to_string(Custom_Type{}), "foo");
}

TEST(positional_arguments) {
    assert_eq("42", fmt::sprint("{0}", 42));
    assert_eq("before 42", fmt::sprint("before {0}", 42));
    assert_eq("42 after", fmt::sprint("{0} after", 42));
    assert_eq("before 42 after", fmt::sprint("before {0} after", 42));
    assert_eq("answer = 42", fmt::sprint("{0} = {1}", "answer", 42));
    assert_eq("42 is the answer", fmt::sprint("{1} is the {0}", "answer", 42));
    assert_eq("abracadabra", fmt::sprint("{0}{1}{0}", "abra", "cad"));
}

TEST(named_arguments) {
    assert_eq("abracadabra", fmt::sprint("{0}{1}{0}", "abra", "cad"));
    assert_eq("1/97/A", fmt::sprint("{_1}/{a_}/{A_}", "a_"_a = 'a', "A_"_a = "A", "_1"_a = 1));
    assert_eq("abracadabra", fmt::sprint("{0}{1}{0}", "abra", "cad"));
    assert_eq(" -42", fmt::sprint("{0:{width}}", -42, "width"_a = 4));
    assert_eq("abracadabra", fmt::sprint("{0}{1}{0}", "abra", "cad"));
    assert_eq("st", fmt::sprint("{0:.{precision}}", "str", "precision"_a = 2));
    assert_eq("abracadabra", fmt::sprint("{0}{1}{0}", "abra", "cad"));
    assert_eq("1 2", fmt::sprint("{} {two}", 1, "two"_a = 2));
    assert_eq("abracadabra", fmt::sprint("{0}{1}{0}", "abra", "cad"));
    assert_eq("42", fmt::sprint("{c}", "a"_a = 0, "b"_a = 0, "c"_a = 42, "d"_a = 0, "e"_a = 0, "f"_a = 0, "g"_a = 0,
                                "h"_a = 0, "i"_a = 0, "j"_a = 0, "k"_a = 0, "l"_a = 0, "m"_a = 0, "n"_a = 0, "o"_a = 0,
                                "p"_a = 0));
}

TEST(automatic_argument_indexing) {
    assert_eq("abc", fmt::sprint("{:c}{:c}{:c}", 'a', 'b', 'c'));
    assert_eq("1.23", fmt::sprint("{:.{}}", 1.2345, 2));
}

TEST(left_align) {
    assert_eq("42  ", fmt::sprint("{0:<4}", 42));
    assert_eq("42  ", fmt::sprint("{0:<4o}", 042));
    assert_eq("42  ", fmt::sprint("{0:<4x}", 0x42));
    assert_eq("-42  ", fmt::sprint("{0:<5}", -42));
    assert_eq("42   ", fmt::sprint("{0:<5}", 42u));
    assert_eq("-42  ", fmt::sprint("{0:<5}", -42l));
    assert_eq("42   ", fmt::sprint("{0:<5}", 42ul));
    assert_eq("-42  ", fmt::sprint("{0:<5}", -42ll));
    assert_eq("42   ", fmt::sprint("{0:<5}", 42ull));
    assert_eq("-42  ", fmt::sprint("{0:<5.0}", -42.0));
    assert_eq("c    ", fmt::sprint("{0:<5c}", 'c'));
    assert_eq("abc  ", fmt::sprint("{0:<5}", "abc"));
    assert_eq("0xface  ", fmt::sprint("{0:<8}", (void *) 0xface));
}

TEST(right_align) {
    assert_eq("  42", fmt::sprint("{0:>4}", 42));
    assert_eq("  42", fmt::sprint("{0:>4o}", 042));
    assert_eq("  42", fmt::sprint("{0:>4x}", 0x42));
    assert_eq("  -42", fmt::sprint("{0:>5}", -42));
    assert_eq("   42", fmt::sprint("{0:>5}", 42u));
    assert_eq("  -42", fmt::sprint("{0:>5}", -42l));
    assert_eq("   42", fmt::sprint("{0:>5}", 42ul));
    assert_eq("  -42", fmt::sprint("{0:>5}", -42ll));
    assert_eq("   42", fmt::sprint("{0:>5}", 42ull));
    assert_eq("  -42", fmt::sprint("{0:>5.0}", -42.0));
    assert_eq("    c", fmt::sprint("{0:>5c}", 'c'));
    assert_eq("  abc", fmt::sprint("{0:>5}", "abc"));
    assert_eq("  0xface", fmt::sprint("{0:>8}", (void *) 0xface));
}

TEST(numeric_align) {
    assert_eq("  42", fmt::sprint("{0:=4}", 42));
    assert_eq("+ 42", fmt::sprint("{0:=+4}", 42));
    assert_eq("  42", fmt::sprint("{0:=4o}", 042));
    assert_eq("+ 42", fmt::sprint("{0:=+4o}", 042));
    assert_eq("  42", fmt::sprint("{0:=4x}", 0x42));
    assert_eq("+ 42", fmt::sprint("{0:=+4x}", 0x42));
    assert_eq("-  42", fmt::sprint("{0:=5}", -42));
    assert_eq("   42", fmt::sprint("{0:=5}", 42u));
    assert_eq("-  42", fmt::sprint("{0:=5}", -42l));
    assert_eq("   42", fmt::sprint("{0:=5}", 42ul));
    assert_eq("-  42", fmt::sprint("{0:=5}", -42ll));
    assert_eq("   42", fmt::sprint("{0:=5}", 42ull));
    assert_eq("-  42", fmt::sprint("{0:=5.0}", -42.0));
    assert_eq(" 1", fmt::sprint("{:= .0}", 1.0));
}

TEST(center_align) {
    assert_eq(" 42  ", fmt::sprint("{0:^5}", 42));
    assert_eq(" 42  ", fmt::sprint("{0:^5o}", 042));
    assert_eq(" 42  ", fmt::sprint("{0:^5x}", 0x42));
    assert_eq(" -42 ", fmt::sprint("{0:^5}", -42));
    assert_eq(" 42  ", fmt::sprint("{0:^5}", 42u));
    assert_eq(" -42 ", fmt::sprint("{0:^5}", -42l));
    assert_eq(" 42  ", fmt::sprint("{0:^5}", 42ul));
    assert_eq(" -42 ", fmt::sprint("{0:^5}", -42ll));
    assert_eq(" 42  ", fmt::sprint("{0:^5}", 42ull));
    assert_eq(" -42  ", fmt::sprint("{0:^6.0}", -42.0));
    assert_eq("  c  ", fmt::sprint("{0:^5c}", 'c'));
    assert_eq(" abc  ", fmt::sprint("{0:^6}", "abc"));
    assert_eq(" 0xface ", fmt::sprint("{0:^8}", (void *) 0xface));
}

TEST(fill) {
    assert_eq("**42", fmt::sprint("{0:*>4}", 42));
    assert_eq("**-42", fmt::sprint("{0:*>5}", -42));
    assert_eq("***42", fmt::sprint("{0:*>5}", 42u));
    assert_eq("**-42", fmt::sprint("{0:*>5}", -42l));
    assert_eq("***42", fmt::sprint("{0:*>5}", 42ul));
    assert_eq("**-42", fmt::sprint("{0:*>5}", -42ll));
    assert_eq("***42", fmt::sprint("{0:*>5}", 42ull));
    assert_eq("**-42", fmt::sprint("{0:*>5.0}", -42.0));
    assert_eq("c****", fmt::sprint("{0:*<5c}", 'c'));
    assert_eq("abc**", fmt::sprint("{0:*<5}", "abc"));
    assert_eq("**0xface", fmt::sprint("{0:*>8}", (void *) 0xface));
    assert_eq("foo=", fmt::sprint("{:}=", "foo"));
}

TEST(plus_sign) {
    assert_eq("+42", fmt::sprint("{0:+}", 42));
    assert_eq("-42", fmt::sprint("{0:+}", -42));
    assert_eq("+42", fmt::sprint("{0:+}", 42));
    assert_eq("+42", fmt::sprint("{0:+}", 42l));
    assert_eq("+42", fmt::sprint("{0:+}", 42ll));
    assert_eq("+42", fmt::sprint("{0:+.0}", 42.0));
}

TEST(minus_sign) {
    assert_eq("42", fmt::sprint("{0:-}", 42));
    assert_eq("-42", fmt::sprint("{0:-}", -42));
    assert_eq("42", fmt::sprint("{0:-}", 42));
    assert_eq("42", fmt::sprint("{0:-}", 42l));
    assert_eq("42", fmt::sprint("{0:-}", 42ll));
    assert_eq("42", fmt::sprint("{0:-.0}", 42.0));
}

TEST(space_sign) {
    assert_eq(" 42", fmt::sprint("{0: }", 42));
    assert_eq("-42", fmt::sprint("{0: }", -42));
    assert_eq(" 42", fmt::sprint("{0: }", 42));
    assert_eq(" 42", fmt::sprint("{0: }", 42l));
    assert_eq(" 42", fmt::sprint("{0: }", 42ll));
    assert_eq(" 42", fmt::sprint("{0: .0}", 42.0));
}

TEST(hash_flag) {
    assert_eq("42", fmt::sprint("{0:#}", 42));
    assert_eq("-42", fmt::sprint("{0:#}", -42));
    assert_eq("0b101010", fmt::sprint("{0:#b}", 42));
    assert_eq("0B101010", fmt::sprint("{0:#B}", 42));
    assert_eq("-0b101010", fmt::sprint("{0:#b}", -42));
    assert_eq("0x42", fmt::sprint("{0:#x}", 0x42));
    assert_eq("0X42", fmt::sprint("{0:#X}", 0x42));
    assert_eq("-0x42", fmt::sprint("{0:#x}", -0x42));
    assert_eq("042", fmt::sprint("{0:#o}", 042));
    assert_eq("-042", fmt::sprint("{0:#o}", -042));
    assert_eq("42", fmt::sprint("{0:#}", 42u));
    assert_eq("0x42", fmt::sprint("{0:#x}", 0x42u));
    assert_eq("042", fmt::sprint("{0:#o}", 042u));

    assert_eq("-42", fmt::sprint("{0:#}", -42l));
    assert_eq("0x42", fmt::sprint("{0:#x}", 0x42l));
    assert_eq("-0x42", fmt::sprint("{0:#x}", -0x42l));
    assert_eq("042", fmt::sprint("{0:#o}", 042l));
    assert_eq("-042", fmt::sprint("{0:#o}", -042l));
    assert_eq("42", fmt::sprint("{0:#}", 42ul));
    assert_eq("0x42", fmt::sprint("{0:#x}", 0x42ul));
    assert_eq("042", fmt::sprint("{0:#o}", 042ul));

    assert_eq("-42", fmt::sprint("{0:#}", -42ll));
    assert_eq("0x42", fmt::sprint("{0:#x}", 0x42ll));
    assert_eq("-0x42", fmt::sprint("{0:#x}", -0x42ll));
    assert_eq("042", fmt::sprint("{0:#o}", 042ll));
    assert_eq("-042", fmt::sprint("{0:#o}", -042ll));
    assert_eq("42", fmt::sprint("{0:#}", 42ull));
    assert_eq("0x42", fmt::sprint("{0:#x}", 0x42ull));
    assert_eq("042", fmt::sprint("{0:#o}", 042ull));

    assert_eq("-42.0", fmt::sprint("{0:#.1}", -42.0));
}

TEST(zero_flag) {
    assert_eq("42", fmt::sprint("{0:0}", 42));
    assert_eq("-0042", fmt::sprint("{0:05}", -42));
    assert_eq("00042", fmt::sprint("{0:05}", 42u));
    assert_eq("-0042", fmt::sprint("{0:05}", -42l));
    assert_eq("00042", fmt::sprint("{0:05}", 42ul));
    assert_eq("-0042", fmt::sprint("{0:05}", -42ll));
    assert_eq("00042", fmt::sprint("{0:05}", 42ull));
    assert_eq("-0042", fmt::sprint("{0:05.0}", -42.0));
}

TEST(width) {
    assert_eq(" -42", fmt::sprint("{0:4}", -42));
    assert_eq("   42", fmt::sprint("{0:5}", 42u));
    assert_eq("   -42", fmt::sprint("{0:6}", -42l));
    assert_eq("     42", fmt::sprint("{0:7}", 42ul));
    assert_eq("   -42", fmt::sprint("{0:6}", -42ll));
    assert_eq("     42", fmt::sprint("{0:7}", 42ull));
    assert_eq("   -1.23", fmt::sprint("{0:8.2}", -1.23));
    assert_eq("    0xcafe", fmt::sprint("{0:10}", (void *) 0xcafe));
    assert_eq("x          ", fmt::sprint("{0:11c}", 'x'));
    assert_eq("str         ", fmt::sprint("{0:12}", "str"));

    assert_eq(" -42", fmt::sprint("{0:{1}}", -42, 4));
    assert_eq("   42", fmt::sprint("{0:{1}}", 42u, 5));
    assert_eq("   -42", fmt::sprint("{0:{1}}", -42l, 6));
    assert_eq("     42", fmt::sprint("{0:{1}}", 42ul, 7));
    assert_eq("   -42", fmt::sprint("{0:{1}}", -42ll, 6));
    assert_eq("     42", fmt::sprint("{0:{1}}", 42ull, 7));
    assert_eq("   -1.23", fmt::sprint("{0:{1}.2}", -1.23, 8));
    assert_eq("    0xcafe", fmt::sprint("{0:{1}}", (void *) 0xcafe, 10));
    assert_eq("x          ", fmt::sprint("{0:{1}c}", 'x', 11));
    assert_eq("str         ", fmt::sprint("{0:{1}}", "str", 12));
}

TEST(precision) {
    assert_eq("1.23", fmt::sprint("{0:.2}", 1.2345));
    assert_eq("st", fmt::sprint("{0:.2}", "str"));

    assert_eq("1.23", fmt::sprint("{0:.{1}}", 1.2345, 2));
    assert_eq("st", fmt::sprint("{0:.{1}}", "str", 2));
}

TEST(bool_and_short) {
    assert_eq("true", fmt::sprint("{}", true));
    assert_eq("false", fmt::sprint("{}", false));
    assert_eq("1", fmt::sprint("{:d}", true));
    assert_eq("true ", fmt::sprint("{:5}", true));

    s16 s = 42;
    assert_eq("42", fmt::sprint("{0:d}", s));

    u16 us = 42;
    assert_eq("42", fmt::sprint("{0:d}", us));
}

TEST(binary) {
    assert_eq("0", fmt::sprint("{0:b}", 0));
    assert_eq("101010", fmt::sprint("{0:b}", 42));
    assert_eq("101010", fmt::sprint("{0:b}", 42u));
    assert_eq("-101010", fmt::sprint("{0:b}", -42));
    assert_eq("11000000111001", fmt::sprint("{0:b}", 12345));
    assert_eq("10010001101000101011001111000", fmt::sprint("{0:b}", 0x12345678));
    assert_eq("10010000101010111100110111101111", fmt::sprint("{0:b}", 0x90ABCDEF));
    assert_eq("11111111111111111111111111111111", fmt::sprint("{0:b}", std::numeric_limits<u32>::max()));
}

TEST(decimal) {
    assert_eq("0", fmt::sprint("{0}", 0));
    assert_eq("42", fmt::sprint("{0}", 42));
    assert_eq("42", fmt::sprint("{0:d}", 42));
    assert_eq("42", fmt::sprint("{0}", 42u));
    assert_eq("-42", fmt::sprint("{0}", -42));
    assert_eq("12345", fmt::sprint("{0}", 12345));
    assert_eq("67890", fmt::sprint("{0}", 67890));
}

TEST(hexadecimal) {
    assert_eq("0", fmt::sprint("{0:x}", 0));
    assert_eq("42", fmt::sprint("{0:x}", 0x42));
    assert_eq("42", fmt::sprint("{0:x}", 0x42u));
    assert_eq("-42", fmt::sprint("{0:x}", -0x42));
    assert_eq("12345678", fmt::sprint("{0:x}", 0x12345678));
    assert_eq("90abcdef", fmt::sprint("{0:x}", 0x90abcdef));
    assert_eq("12345678", fmt::sprint("{0:X}", 0x12345678));
    assert_eq("90ABCDEF", fmt::sprint("{0:X}", 0x90ABCDEF));
}

TEST(octal) {
    assert_eq("0", fmt::sprint("{0:o}", 0));
    assert_eq("42", fmt::sprint("{0:o}", 042));
    assert_eq("42", fmt::sprint("{0:o}", 042u));
    assert_eq("-42", fmt::sprint("{0:o}", -042));
    assert_eq("12345670", fmt::sprint("{0:o}", 012345670));
}

TEST(int_locale) {
    assert_eq("123", fmt::sprint("{:n}", 123));
    assert_eq("1,234", fmt::sprint("{:n}", 1234));
    assert_eq("1,234,567", fmt::sprint("{:n}", 1234567));
    assert_eq("4,294,967,295", fmt::sprint("{:n}", std::numeric_limits<u32>::max()));
}

TEST(floating_point) {
    assert_eq("392.500000", fmt::sprint("{0:f}", 392.5f));

    assert_eq("0", fmt::sprint("{:.0}", 0.0));
    assert_eq("0.000000", fmt::sprint("{:f}", 0.0));
    assert_eq("0", fmt::sprint("{:g}", 0.0));
    assert_eq("392.65", fmt::sprint("{:.2}", 392.65));
    assert_eq("392.65", fmt::sprint("{:g}", 392.65));
    assert_eq("392.65", fmt::sprint("{:G}", 392.65));
    assert_eq("392.650000", fmt::sprint("{:f}", 392.65));
    assert_eq("392.650000", fmt::sprint("{:F}", 392.65));

    assert_eq(fmt::sprint("0.{:0<1000}", ""), fmt::sprint("{:.1000f}", 0.0));

    f64 nan = std::numeric_limits<f64>::quiet_NaN();
    assert_eq("nan", fmt::sprint("{}", nan));
    assert_eq("+nan", fmt::sprint("{:+}", nan));
    assert_eq(" nan", fmt::sprint("{: }", nan));
    assert_eq("NAN", fmt::sprint("{:F}", nan));
    assert_eq("nan    ", fmt::sprint("{:<7}", nan));
    assert_eq("  nan  ", fmt::sprint("{:^7}", nan));
    assert_eq("    nan", fmt::sprint("{:>7}", nan));

    f64 inf = std::numeric_limits<f64>::infinity();
    assert_eq("inf", fmt::sprint("{}", inf));
    assert_eq("+inf", fmt::sprint("{:+}", inf));
    assert_eq("-inf", fmt::sprint("{}", -inf));
    assert_eq(" inf", fmt::sprint("{: }", inf));
    assert_eq("INF", fmt::sprint("{:F}", inf));
    assert_eq("inf    ", fmt::sprint("{:<7}", inf));
    assert_eq("  inf  ", fmt::sprint("{:^7}", inf));
    assert_eq("    inf", fmt::sprint("{:>7}", inf));
}

TEST(bytes_chars_and_strings) {
    assert_eq("97", fmt::sprint("{0}", 'a'));
    assert_eq("z", fmt::sprint("{0:c}", 'z'));

    s32 n = 'x';
    const char types[] = "cbBdoxXn";
    for (const char *type = types + 1; *type; ++type) {
        string formatStr = fmt::sprint("{{:{}}}", *type);
        assert_eq(fmt::sprint(string_view(formatStr), n), fmt::sprint(string_view(formatStr), 'x'));
    }
    assert_eq(fmt::sprint("{:02X}", n), fmt::sprint("{:02X}", 'x'));

    assert_eq("42", fmt::sprint("{}", (byte) 42));
    assert_eq("42", fmt::sprint("{}", (byte) 42));

    char nonconst[] = "nonconst";
    assert_eq("nonconst", fmt::sprint("{0}", nonconst));

    assert_eq("test", fmt::sprint("{0}", "test"));
    assert_eq("test", fmt::sprint("{0:s}", "test"));
}

TEST(pointer) {
    assert_eq("0x0", fmt::sprint("{0}", (void *) null));
    assert_eq("0x1234", fmt::sprint("{0}", (void *) 0x1234));
    assert_eq("0x1234", fmt::sprint("{0:p}", (void *) 0x1234));
    assert_eq("0x" + string("f").repeated(sizeof(void *) * 2), fmt::sprint("{0}", (void *) (~uptr_t())));
    assert_eq("0x0", fmt::sprint("{}", null));
}
