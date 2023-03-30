#include "../test.h"

#define CHECK_WRITE(expected, fmtString, ...)           \
    {                                                   \
        string t = sprint(fmtString, ##__VA_ARGS__);    \
        assert_eq_str(t, expected);     \
        free(t);                                        \
    }

file_scope string LAST_ERROR;

file_scope void test_parse_error_handler(string message, string formatString, s64 position) {
    LAST_ERROR = message;

    // We test visually for the correctness of the ^.
#if 0
    string str = formatString;
    str.replace_all('\"', "\\\"")
        ->replace_all('\\', "\\\\")
        ->replace_all('\a', "\\a")
        ->replace_all('\b', "\\b")
        ->replace_all('\f', "\\f")
        ->replace_all('\n', "\\n")
        ->replace_all('\r', "\\r")
        ->replace_all('\t', "\\t")
        ->replace_all('\v', "\\v");

    print("\n\n>>> {!GRAY}An error during formatting occured: {!YELLOW}{}{!GRAY}\n", message);
    print("    ... the error happened here:\n");
    print("        {!}{}{!GRAY}\n", str);
    print("        {: >{}} {!} \n\n", "^", position + 1);
#endif
}

template <typename... Args>
void format_test_error(string fmtString, Args &&...arguments) {
    auto newContext                 = Context;
    newContext.FmtParseErrorHandler = test_parse_error_handler;
    PUSH_CONTEXT(newContext) {
        static const s64 NUM_ARGS = sizeof...(Args);
        stack_array<fmt_arg, NUM_ARGS> args;

        args = {fmt_make_arg(arguments)...};

        counting_writer dummy;
        auto f = fmt_context(&dummy, fmtString, args);

        fmt_parse_and_format(&f);
    }
}

#define EXPECT_ERROR(expected, fmtString, ...)              \
    format_test_error(fmtString, ##__VA_ARGS__);            \
    assert_eq_str(LAST_ERROR, expected);    \
    LAST_ERROR = "";

TEST(write_bool) {
    CHECK_WRITE("true", "{}", true);
    CHECK_WRITE("false", "{}", false);
    CHECK_WRITE("1", "{:d}", true);
    CHECK_WRITE("true ", "{:5}", true);
}

TEST(write_integer_16) {
    s16 s = 42;
    CHECK_WRITE("42", "{0:d}", s);
    u16 us = 42;
    CHECK_WRITE("42", "{0:d}", us);
}

TEST(write_integer_32) {
    CHECK_WRITE("42", "{}", 42);
    CHECK_WRITE("-42", "{}", -42);
    CHECK_WRITE("12", "{}", static_cast<u16>(12));
    CHECK_WRITE("34", "{}", 34u);
    CHECK_WRITE("56", "{}", 56l);
    CHECK_WRITE("78", "{}", 78ul);
    CHECK_WRITE("-2147483648", "{}", numeric<s32>::min());
    CHECK_WRITE("2147483647", "{}", numeric<s32>::max());
    CHECK_WRITE("4294967295", "{}", numeric<u32>::max());
    CHECK_WRITE("-2147483648", "{}", numeric<long>::min());
    CHECK_WRITE("2147483647", "{}", numeric<long>::max());
    CHECK_WRITE("4294967295", "{}", numeric<unsigned long>::max());
}

TEST(write_integer_64) {
    CHECK_WRITE("56", "{}", 56ll);
    CHECK_WRITE("78", "{}", 78ull);
    CHECK_WRITE("-9223372036854775808", "{}", numeric<s64>::min());
    CHECK_WRITE("9223372036854775807", "{}", numeric<s64>::max());
    CHECK_WRITE("18446744073709551615", "{}", numeric<u64>::max());
}

TEST(write_f64) {
    CHECK_WRITE("4.2", "{}", 4.2);
    CHECK_WRITE("-4.2", "{}", -4.2);
    CHECK_WRITE("2.2250738585072014e-308", "{}", numeric<f64>::min());
    CHECK_WRITE("1.7976931348623157e+308", "{}", numeric<f64>::max());
}

TEST(write_code_point) {
    CHECK_WRITE("X", "{:c}", 'X');
}

template <typename T>
void check_unknown_types(T value, string types, string expectedMessage) {
    string special = ".0123456789}";

    For(range(1, numeric<s8>::max())) {
        if (string_has(special, it) || string_has(types, it)) continue;

        string fmtString = sprint("{{0:10{:c}}}", it);
        EXPECT_ERROR(expectedMessage, fmtString, value);
        free(fmtString);
    }
}

TEST(format_int) {
    EXPECT_ERROR("\"}\" expected", "{0:v", 42);
    // check_unknown_types(42, "bBdoxXnc", "Invalid type specifier for an integer");
}

TEST(format_int_binary) {
    CHECK_WRITE("0", "{0:b}", 0);
    CHECK_WRITE("101010", "{0:b}", 42);
    CHECK_WRITE("101010", "{0:b}", 42u);
    CHECK_WRITE("-101010", "{0:b}", -42);
    CHECK_WRITE("11000000111001", "{0:b}", 12345);
    CHECK_WRITE("10010001101000101011001111000", "{0:b}", 0x12345678);
    CHECK_WRITE("10010000101010111100110111101111", "{0:b}", 0x90ABCDEF);
    CHECK_WRITE("11111111111111111111111111111111", "{0:b}", numeric<u32>::max());
}

TEST(format_int_octal) {
    CHECK_WRITE("0", "{0:o}", 0);
    CHECK_WRITE("42", "{0:o}", 042);
    CHECK_WRITE("42", "{0:o}", 042u);
    CHECK_WRITE("-42", "{0:o}", -042);
    CHECK_WRITE("12345670", "{0:o}", 012345670);
}

TEST(format_int_decimal) {
    CHECK_WRITE("0", "{0}", 0);
    CHECK_WRITE("42", "{0}", 42);
    CHECK_WRITE("42", "{0:d}", 42);
    CHECK_WRITE("42", "{0}", 42u);
    CHECK_WRITE("-42", "{0}", -42);
    CHECK_WRITE("12345", "{0}", 12345);
    CHECK_WRITE("67890", "{0}", 67890);
}

TEST(format_int_hexadecimal) {
    CHECK_WRITE("0", "{0:x}", 0);
    CHECK_WRITE("42", "{0:x}", 0x42);
    CHECK_WRITE("42", "{0:x}", 0x42u);
    CHECK_WRITE("-42", "{0:x}", -0x42);
    CHECK_WRITE("12345678", "{0:x}", 0x12345678);
    CHECK_WRITE("90abcdef", "{0:x}", 0x90abcdef);
    CHECK_WRITE("12345678", "{0:X}", 0x12345678);
    CHECK_WRITE("90ABCDEF", "{0:X}", 0x90ABCDEF);
}

// format\(([^)]*)\)

// @Locale
TEST(format_int_localeish) {
    CHECK_WRITE("123", "{:n}", 123);
    CHECK_WRITE("1,234", "{:n}", 1234);
    CHECK_WRITE("1,234,567", "{:n}", 1234567);
    CHECK_WRITE("4,294,967,295", "{:n}", numeric<u32>::max());
}

TEST(format_f32) {
    CHECK_WRITE("0", "{}", 0.0f);
    CHECK_WRITE("392.500000", "{0:f}", 392.5f);
    CHECK_WRITE("12.500000%", "{0:%}", 0.125f);
}

TEST(format_f64) {
    // check_unknown_types(1.2, "eEfFgGaAn%", "Invalid type specifier for a float");

    CHECK_WRITE("0", "{}", 0.0);

    CHECK_WRITE("0", "{:}", 0.0);
    CHECK_WRITE("0.000000", "{:f}", 0.0);
    CHECK_WRITE("0", "{:g}", 0.0);
    CHECK_WRITE("392.65", "{:}", 392.65);
    CHECK_WRITE("392.65", "{:g}", 392.65);
    CHECK_WRITE("392.65", "{:G}", 392.65);
    CHECK_WRITE("4.9014e+06", "{:g}", 4.9014e6);
    CHECK_WRITE("392.650000", "{:f}", 392.65);
    CHECK_WRITE("392.650000", "{:F}", 392.65);

    CHECK_WRITE("12.500000%", "{:%}", 0.125);
    CHECK_WRITE("12.34%", "{:.2%}", 0.1234432);

    CHECK_WRITE("3.926490e+02", "{0:e}", 392.649);
    CHECK_WRITE("3.926490E+02", "{0:E}", 392.649);
    CHECK_WRITE("+0000392.6", "{0:+010.4g}", 392.649);

    // @TODO: Hex floats
    // CHECK_WRITE("-0x1.500000p+5", "{:a}", -42.0);
    // CHECK_WRITE("-0x1.500000P+5", "{:A}", -42.0);
}

TEST(format_nan) {
    auto nan = numeric<f64>::quiet_NaN();
    CHECK_WRITE("nan", "{}", nan);
    CHECK_WRITE("+nan", "{:+}", nan);
    CHECK_WRITE(" nan", "{: }", nan);
    CHECK_WRITE("NAN", "{:F}", nan);
    CHECK_WRITE("nan    ", "{:<7}", nan);
    CHECK_WRITE("  nan  ", "{:^7}", nan);
    CHECK_WRITE("    nan", "{:>7}", nan);
    CHECK_WRITE("nan%", "{:%}", nan);
}

TEST(format_inf) {
    auto inf = numeric<f64>::infinity();
    CHECK_WRITE("inf", "{}", inf);
    CHECK_WRITE("+inf", "{:+}", inf);
    CHECK_WRITE("-inf", "{}", -inf);
    CHECK_WRITE(" inf", "{: }", inf);
    CHECK_WRITE("INF", "{:F}", inf);
    CHECK_WRITE("inf    ", "{:<7}", inf);
    CHECK_WRITE("  inf  ", "{:^7}", inf);
    CHECK_WRITE("    inf", "{:>7}", inf);
    CHECK_WRITE("inf%", "{:%}", inf);
}

struct Answer {
};

void write_custom(fmt_context *f, const Answer *) { write(f, 42); }

TEST(format_custom) {
    Answer a;

    CHECK_WRITE("42", "{0}", a);
    CHECK_WRITE("0042", "{:04}", a);
}

TEST(precision_rounding) {
    CHECK_WRITE("0", "{:.0f}", 0.0);
    CHECK_WRITE("0", "{:.0f}", 0.01);
    CHECK_WRITE("0", "{:.0f}", 0.1);

    CHECK_WRITE("0.000", "{:.3f}", 0.00049);
    CHECK_WRITE("0.001", "{:.3f}", 0.0005);
    CHECK_WRITE("0.001", "{:.3f}", 0.00149);
    CHECK_WRITE("0.002", "{:.3f}", 0.0015);
    CHECK_WRITE("1.000", "{:.3f}", 0.9999);
    CHECK_WRITE("0.00123", "{:.3}", 0.00123);
    CHECK_WRITE("0.1", "{:.16g}", 0.1);
    CHECK_WRITE("1", "{:.0}", 1.0);
    CHECK_WRITE("225.51575035152063720", "{:.17f}", 225.51575035152064);
    CHECK_WRITE("-761519619559038.3", "{:.1f}", -761519619559038.2);
    CHECK_WRITE("1.9156918820264798e-56", "{}", 1.9156918820264798e-56);
    CHECK_WRITE("0.0000", "{:.4f}", 7.2809479766055470e-15);
    CHECK_WRITE("3788512123356.985352", "{:f}", 3788512123356.985352);
}

TEST(prettify_float) {
    CHECK_WRITE("0.0001", "{}", 1e-4);
    CHECK_WRITE("1e-05", "{}", 1e-5);
    CHECK_WRITE("1000000000000000", "{}", 1e15);
    CHECK_WRITE("1e+16", "{}", 1e16);
    CHECK_WRITE("9.999e-05", "{}", 9.999e-5);
    CHECK_WRITE("10000000000", "{}", 1e10);
    CHECK_WRITE("100000000000", "{}", 1e11);
    CHECK_WRITE("12340000000", "{}", 1234e7);
    CHECK_WRITE("12.34", "{}", 1234e-2);
    CHECK_WRITE("0.001234", "{}", 1234e-6);
    CHECK_WRITE("0.1", "{}", 0.1f);
    CHECK_WRITE("0.1", "{}", 0.1);
    CHECK_WRITE("1.3563156e-19", "{}", 1.35631564e-19f);
}

TEST(escape_brackets) {
    CHECK_WRITE("{", "{{");
    CHECK_WRITE("before {", "before {{");
    CHECK_WRITE("{ after", "{{ after");
    CHECK_WRITE("before { after", "before {{ after");

    CHECK_WRITE("}", "}}");
    CHECK_WRITE("before }", "before }}");
    CHECK_WRITE("} after", "}} after");
    CHECK_WRITE("before } after", "before }} after");

    CHECK_WRITE("{}", "{{}}");
    CHECK_WRITE("{42}", "{{{0}}}", 42);
}

TEST(args_in_different_positions) {
    CHECK_WRITE("42", "{0}", 42);
    CHECK_WRITE("before 42", "before {0}", 42);
    CHECK_WRITE("42 after", "{0} after", 42);
    CHECK_WRITE("before 42 after", "before {0} after", 42);
    CHECK_WRITE("answer = 42", "{0} = {1}", "answer", 42);
    CHECK_WRITE("42 is the answer", "{1} is the {0}", "answer", 42);
    CHECK_WRITE("abracadabra", "{0}{1}{0}", "abra", "cad");
}

TEST(args_errors) {
    EXPECT_ERROR("Invalid format string", "{");
    EXPECT_ERROR("Format string ended abruptly", "{0");
    EXPECT_ERROR("Argument index out of range", "{0}");

    EXPECT_ERROR("Invalid format string", "{");  //-V1002
    EXPECT_ERROR("Unmatched \"}\" in format string - if you want to print it use \"}}\" to escape", "}");
    EXPECT_ERROR("Expected \":\" or \"}\"", "{0{}");
}

TEST(many_args) {
    CHECK_WRITE("1234567891011121314151617181920", "{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}{}", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
}

TEST(auto_arg_index) {
    CHECK_WRITE("abc", "{}{}{}", "a", "b", "c");

    EXPECT_ERROR("Cannot switch from manual to automatic argument indexing", "{0}{}", 'a', 'b');
    EXPECT_ERROR("Cannot switch from automatic to manual argument indexing", "{}{0}", 'a', 'b');

    CHECK_WRITE("1.2", "{:.{}}", 1.2345, 2);

    EXPECT_ERROR("Cannot switch from manual to automatic argument indexing", "{0}:.{}", 1.2345, 2);
    EXPECT_ERROR("Cannot switch from automatic to manual argument indexing", "{:.{1}}", 1.2345, 2);
}

TEST(empty_specs) {
    CHECK_WRITE("42", "{0:}", 42);
}

TEST(left_align) {
    CHECK_WRITE("42  ", "{0:<4}", 42);
    CHECK_WRITE("42  ", "{0:<4o}", 042);
    CHECK_WRITE("42  ", "{0:<4x}", 0x42);
    CHECK_WRITE("-42  ", "{0:<5}", -42);
    CHECK_WRITE("42   ", "{0:<5}", 42u);
    CHECK_WRITE("-42  ", "{0:<5}", -42l);
    CHECK_WRITE("42   ", "{0:<5}", 42ul);
    CHECK_WRITE("-42  ", "{0:<5}", -42ll);
    CHECK_WRITE("42   ", "{0:<5}", 42ull);
    CHECK_WRITE("-42  ", "{0:<5}", -42.0);
    CHECK_WRITE("c    ", "{0:<5}", "c");
    CHECK_WRITE("abc  ", "{0:<5}", "abc");
    CHECK_WRITE("0xface  ", "{0:<8}", (void *) 0xface);
}

TEST(right_align) {
    CHECK_WRITE("  42", "{0:>4}", 42);
    CHECK_WRITE("  42", "{0:>4o}", 042);
    CHECK_WRITE("  42", "{0:>4x}", 0x42);
    CHECK_WRITE("  -42", "{0:>5}", -42);
    CHECK_WRITE("   42", "{0:>5}", 42u);
    CHECK_WRITE("  -42", "{0:>5}", -42l);
    CHECK_WRITE("   42", "{0:>5}", 42ul);
    CHECK_WRITE("  -42", "{0:>5}", -42ll);
    CHECK_WRITE("   42", "{0:>5}", 42ull);
    CHECK_WRITE("  -42", "{0:>5}", -42.0);
    CHECK_WRITE("    c", "{0:>5}", "c");
    CHECK_WRITE("  abc", "{0:>5}", "abc");
    CHECK_WRITE("  0xface", "{0:>8}", (void *) 0xface);
}

TEST(numeric_align) {
    CHECK_WRITE("  42", "{0:=4}", 42);
    CHECK_WRITE("+ 42", "{0:=+4}", 42);
    CHECK_WRITE("  42", "{0:=4o}", 042);
    CHECK_WRITE("+ 42", "{0:=+4o}", 042);
    CHECK_WRITE("  42", "{0:=4x}", 0x42);
    CHECK_WRITE("+ 42", "{0:=+4x}", 0x42);
    CHECK_WRITE("-  42", "{0:=5}", -42);
    CHECK_WRITE("   42", "{0:=5}", 42u);
    CHECK_WRITE("-  42", "{0:=5}", -42l);
    CHECK_WRITE("   42", "{0:=5}", 42ul);
    CHECK_WRITE("-  42", "{0:=5}", -42ll);
    CHECK_WRITE("   42", "{0:=5}", 42ull);
    CHECK_WRITE("-  42", "{0:=5}", -42.0);

    EXPECT_ERROR("\"}\" expected", "{0:=5", 'a');
    EXPECT_ERROR("Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", "{0:=5c}", 'a');
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:=5}", "abc");
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:=8}", (void *) 0xface);

    CHECK_WRITE(" 1", "{:= }", 1.0);
}

TEST(center_align) {
    CHECK_WRITE(" 42  ", "{0:^5}", 42);
    CHECK_WRITE(" 42  ", "{0:^5o}", 042);
    CHECK_WRITE(" 42  ", "{0:^5x}", 0x42);
    CHECK_WRITE(" -42 ", "{0:^5}", -42);
    CHECK_WRITE(" 42  ", "{0:^5}", 42u);
    CHECK_WRITE(" -42 ", "{0:^5}", -42l);
    CHECK_WRITE(" 42  ", "{0:^5}", 42ul);
    CHECK_WRITE(" -42 ", "{0:^5}", -42ll);
    CHECK_WRITE(" 42  ", "{0:^5}", 42ull);
    CHECK_WRITE(" -42 ", "{0:^5}", -42.0);
    CHECK_WRITE("  c  ", "{0:^5}", "c");
    CHECK_WRITE(" abc  ", "{0:^6}", "abc");
    CHECK_WRITE(" 0xface ", "{0:^8}", (void *) 0xface);
}

TEST(fill) {
    EXPECT_ERROR("Invalid fill character \"{\"", "{0:{<5}", 'c');

    CHECK_WRITE("**42", "{0:*>4}", 42);
    CHECK_WRITE("**-42", "{0:*>5}", -42);
    CHECK_WRITE("***42", "{0:*>5}", 42u);
    CHECK_WRITE("**-42", "{0:*>5}", -42l);
    CHECK_WRITE("***42", "{0:*>5}", 42ul);
    CHECK_WRITE("**-42", "{0:*>5}", -42ll);
    CHECK_WRITE("***42", "{0:*>5}", 42ull);
    CHECK_WRITE("**-42", "{0:*>5}", -42.0);
    CHECK_WRITE("c****", "{0:*<5}", "c");
    CHECK_WRITE("abc**", "{0:*<5}", "abc");
    CHECK_WRITE("**0xface", "{0:*>8}", (void *) 0xface);
    CHECK_WRITE("foo=", "{:}=", "foo");

    CHECK_WRITE(u8"ФФ42", u8"{0:Ф>4}", 42);
    CHECK_WRITE(u8"\u0904\u090442", u8"{0:\u0904>4}", 42);
    CHECK_WRITE(u8"\U0002070E\U0002070E42", u8"{0:\U0002070E>4}", 42);
}

TEST(plus_sign) {
    CHECK_WRITE("+42", "{0:+}", 42);
    CHECK_WRITE("-42", "{0:+}", -42);
    CHECK_WRITE("+42", "{0:+}", 42);
    CHECK_WRITE("+42", "{0:+}", 42l);
    CHECK_WRITE("+42", "{0:+}", 42ll);
    CHECK_WRITE("+42", "{0:+}", 42.0);

    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0:+}", 42u);
    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0:+}", 42ul);
    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0:+}", 42ull);
    EXPECT_ERROR("\"}\" expected", "{0:+", 'c');
    EXPECT_ERROR("Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", "{0:+c}", 'c');
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:+}", "abc");
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:+}", (void *) 0x42);
}

TEST(minus_sign) {
    CHECK_WRITE("42", "{0:-}", 42);
    CHECK_WRITE("-42", "{0:-}", -42);
    CHECK_WRITE("42", "{0:-}", 42);
    CHECK_WRITE("42", "{0:-}", 42l);
    CHECK_WRITE("42", "{0:-}", 42ll);
    CHECK_WRITE("42", "{0:-}", 42.0);

    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0:-}", 42u);
    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0:-}", 42ul);
    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0:-}", 42ull);
    EXPECT_ERROR("\"}\" expected", "{0:-", 'c');
    EXPECT_ERROR("Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", "{0:-c}", 'c');
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:-}", "abc");
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:-}", (void *) 0x42);
}

TEST(space_sign) {
    CHECK_WRITE(" 42", "{0: }", 42);
    CHECK_WRITE("-42", "{0: }", -42);
    CHECK_WRITE(" 42", "{0: }", 42);
    CHECK_WRITE(" 42", "{0: }", 42l);
    CHECK_WRITE(" 42", "{0: }", 42ll);
    CHECK_WRITE(" 42", "{0: }", 42.0);

    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0: }", 42u);
    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0: }", 42ul);
    EXPECT_ERROR("Format specifier requires a signed integer argument (got unsigned)", "{0: }", 42ull);
    EXPECT_ERROR("\"}\" expected", "{0: ", 'c');
    EXPECT_ERROR("Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", "{0: c}", 'c');
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0: }", "abc");
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0: }", (void *) 0x42);
}

TEST(hash_flag) {
    CHECK_WRITE("42", "{0:#}", 42);
    CHECK_WRITE("-42", "{0:#}", -42);
    CHECK_WRITE("0b101010", "{0:#b}", 42);
    CHECK_WRITE("0B101010", "{0:#B}", 42);
    CHECK_WRITE("-0b101010", "{0:#b}", -42);
    CHECK_WRITE("0x42", "{0:#x}", 0x42);
    CHECK_WRITE("0X42", "{0:#X}", 0x42);
    CHECK_WRITE("-0x42", "{0:#x}", -0x42);
    CHECK_WRITE("042", "{0:#o}", 042);
    CHECK_WRITE("-042", "{0:#o}", -042);
    CHECK_WRITE("42", "{0:#}", 42u);
    CHECK_WRITE("0x42", "{0:#x}", 0x42u);
    CHECK_WRITE("042", "{0:#o}", 042u);

    CHECK_WRITE("-42", "{0:#}", -42l);
    CHECK_WRITE("0x42", "{0:#x}", 0x42l);
    CHECK_WRITE("-0x42", "{0:#x}", -0x42l);
    CHECK_WRITE("042", "{0:#o}", 042l);
    CHECK_WRITE("-042", "{0:#o}", -042l);
    CHECK_WRITE("42", "{0:#}", 42ul);
    CHECK_WRITE("0x42", "{0:#x}", 0x42ul);
    CHECK_WRITE("042", "{0:#o}", 042ul);

    CHECK_WRITE("-42", "{0:#}", -42ll);
    CHECK_WRITE("0x42", "{0:#x}", 0x42ll);
    CHECK_WRITE("-0x42", "{0:#x}", -0x42ll);
    CHECK_WRITE("042", "{0:#o}", 042ll);
    CHECK_WRITE("-042", "{0:#o}", -042ll);
    CHECK_WRITE("42", "{0:#}", 42ull);
    CHECK_WRITE("0x42", "{0:#x}", 0x42ull);
    CHECK_WRITE("042", "{0:#o}", 042ull);

    CHECK_WRITE("-42.0", "{0:#}", -42.0);
    CHECK_WRITE("-42.01", "{0:#}", -42.01);
    CHECK_WRITE("4.e+01", "{0:#.0e}", 42.0);

    CHECK_WRITE("0.", "{:#.0f}", 0.01);
    CHECK_WRITE("0.50", "{:#.2g}", 0.5);
    CHECK_WRITE("1.", "{:#.0f}", 0.5);
    CHECK_WRITE("0.", "{:#.0f}", 0.2);
    CHECK_WRITE("1.", "{:#.0f}", 0.51);
    CHECK_WRITE("1.e+01", "{:#.0e}", 9.5);
    CHECK_WRITE("9.e+00", "{:#.0e}", 9.1);

    EXPECT_ERROR("\"}\" expected", "{0:#", 'c');
    EXPECT_ERROR("Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", "{0:#c}", 'c');
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:#}", "abc");
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:#}", (void *) 0x42);
}

TEST(zero_flag) {
    CHECK_WRITE("42", "{0:0}", 42);
    CHECK_WRITE("-0042", "{0:05}", -42);
    CHECK_WRITE("00042", "{0:05}", 42u);
    CHECK_WRITE("-0042", "{0:05}", -42l);
    CHECK_WRITE("00042", "{0:05}", 42ul);
    CHECK_WRITE("-0042", "{0:05}", -42ll);
    CHECK_WRITE("00042", "{0:05}", 42ull);
    CHECK_WRITE("-0042", "{0:05}", -42.0);

    EXPECT_ERROR("\"}\" expected", "{0:0", 'c');
    EXPECT_ERROR("Invalid format specifier(s) for code point - code points can't have numeric alignment, signs or #", "{0:0c}", 'c');
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:0}", "abc");
    EXPECT_ERROR("Format specifier requires an arithmetic argument", "{0:0}", (void *) 0x42);
}

TEST(width) {
    EXPECT_ERROR("We parsed an integer width which was too large", "{0:999999999999999999}", 0);

    CHECK_WRITE(" -42", "{0:4}", -42);
    CHECK_WRITE("   42", "{0:5}", 42u);
    CHECK_WRITE("   -42", "{0:6}", -42l);
    CHECK_WRITE("     42", "{0:7}", 42ul);
    CHECK_WRITE("   -42", "{0:6}", -42ll);
    CHECK_WRITE("     42", "{0:7}", 42ull);
    CHECK_WRITE("   -0.25", "{0:8}", -0.25);
    CHECK_WRITE("    -0.25", "{0:9}", -0.25);
    CHECK_WRITE("    0xcafe", "{0:10}", (void *) 0xcafe);
    CHECK_WRITE("x          ", "{0:11}", "x");
    CHECK_WRITE("str         ", "{0:12}", "str");
}

TEST(dynamic_width) {
    EXPECT_ERROR("Expected a closing \"}\" after parsing an argument ID for a dynamic width", "{0:{", 0);
    EXPECT_ERROR("\"}\" expected", "{0:{}", 0);
    EXPECT_ERROR("Expected a number - an index to an argument", "{0:{?}}", 0);
    EXPECT_ERROR("Argument index out of range", "{0:{1}}", 0);

    EXPECT_ERROR("Expected a closing \"}\" after parsing an argument ID for a dynamic width", "{0:{0:}}", 0);

    EXPECT_ERROR("Negative width", "{0:{1}}", 0, -1);
    EXPECT_ERROR("Width value is too big", "{0:{1}}", 0, (numeric<s32>::max() + 1u));
    EXPECT_ERROR("Negative width", "{0:{1}}", 0, -1l);
    EXPECT_ERROR("Width value is too big", "{0:{1}}", 0, (numeric<s32>::max() + 1ul));

    EXPECT_ERROR("Width was not an integer", "{0:{1}}", 0, "0");
    EXPECT_ERROR("Width was not an integer", "{0:{1}}", 0, 0.0);

    CHECK_WRITE(" -42", "{0:{1}}", -42, 4);
    CHECK_WRITE("   42", "{0:{1}}", 42u, 5);
    CHECK_WRITE("   -42", "{0:{1}}", -42l, 6);
    CHECK_WRITE("     42", "{0:{1}}", 42ul, 7);
    CHECK_WRITE("   -42", "{0:{1}}", -42ll, 6);
    CHECK_WRITE("     42", "{0:{1}}", 42ull, 7);
    CHECK_WRITE("   -0.25", "{0:{1}}", -0.25, 8);
    CHECK_WRITE("    -0.25", "{0:{1}}", -0.25, 9);
    CHECK_WRITE("    0xcafe", "{0:{1}}", (void *) 0xcafe, 10);
    CHECK_WRITE("x          ", "{0:{1}}", "x", 11);
    CHECK_WRITE("str         ", "{0:{1}}", "str", 12);

    CHECK_WRITE(u8"**🤡**", "{:*^5}", u8"🤡");
    CHECK_WRITE(u8"**🤡**", "{:*^5c}", U'🤡');
    CHECK_WRITE(u8"**你好**", "{:*^6}", u8"你好");
    CHECK_WRITE("  42.0", "{:#6}", 42.0);
    CHECK_WRITE("x     ", "{:6c}", 'x');
    CHECK_WRITE("000000", "{:>06.0f}", 0.00884311);
}

TEST(precision) {
    EXPECT_ERROR("We parsed an integer precision which was too large", "{0:.999999999999999999}", 0);

    EXPECT_ERROR("Missing precision specifier (we parsed a dot but nothing valid after that)", "{0:.", 0);
    EXPECT_ERROR("Missing precision specifier (we parsed a dot but nothing valid after that)", "{0:.}", 0);

    EXPECT_ERROR("\"}\" expected", "{0:.2", 0);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2f}", 42);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2f}", 42u);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2f}", 42l);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2f}", 42ul);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2f}", 42ll);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2f}", 42ull);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.2%}", 42);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.2}", 42);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.2}", 42u);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.2}", 42l);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.2}", 42ul);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.2}", 42ll);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.2}", 42ull);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:3.0c}", 'c');

    CHECK_WRITE("1.2", "{0:.2}", 1.2345);

    CHECK_WRITE("1.2e+56", "{:.2}", 1.234e56);
    CHECK_WRITE("1.1", "{0:.3}", 1.1);
    CHECK_WRITE("  0.0e+00", "{:9.1e}", 0.0);

    CHECK_WRITE(
        "4.9406564584124654417656879286822137236505980261432476442558568250067550"
        "727020875186529983636163599237979656469544571773092665671035593979639877"
        "479601078187812630071319031140452784581716784898210368871863605699873072"
        "305000638740915356498438731247339727316961514003171538539807412623856559"
        "117102665855668676818703956031062493194527159149245532930545654440112748"
        "012970999954193198940908041656332452475714786901472678015935523861155013"
        "480352649347201937902681071074917033322268447533357208324319361e-324",
        "{:.494}",
        4.9406564584124654E-324);

    CHECK_WRITE("123.", "{:#.0f}", 123.0);
    CHECK_WRITE("1.23", "{:.02f}", 1.234);
    CHECK_WRITE("0.001", "{:.1g}", 0.001);
    CHECK_WRITE("1019666400", "{}", 1019666432.0f);
    CHECK_WRITE("1e+01", "{:.0e}", 9.5);
    CHECK_WRITE("9e+00", "{:.0e}", 9.1);
    CHECK_WRITE("1.0e-34", "{:.1e}", 1e-34);

    EXPECT_ERROR("Precision is not allowed for pointer type", "{0:.2}", (void *) 0xcafe);
    EXPECT_ERROR("Invalid type specifier for a pointer", "{0:.2f}", (void *) 0xcafe);

    CHECK_WRITE("st", "{0:.2}", "str");
}

TEST(benchmark_string) {
    CHECK_WRITE("0.1250000000:0042:+0.25:str:0x3e8:X:%", "{0:0.10f}:{1:04}:{2:+g}:{3}:{4}:{5:c}:%", 0.125, 42, 0.25, "str", (void *) 1000, 'X');
}

TEST(dynamic_precision) {
    EXPECT_ERROR("Expected a closing \"}\" after parsing an argument ID for a dynamic precision", "{0:.{", 0);
    EXPECT_ERROR("\"}\" expected", "{0:.{}", 0);
    EXPECT_ERROR("Expected a number - an index to an argument", "{0:.{?}}", 0);
    EXPECT_ERROR("\"}\" expected", "{0:.{1}", 0, 0);
    EXPECT_ERROR("Argument index out of range", "{0:.{1}}", 0);

    EXPECT_ERROR("Expected a closing \"}\" after parsing an argument ID for a dynamic precision", "{0:.{0:}}", 0);

    EXPECT_ERROR("Negative precision", "{0:.{1}}", 0, -1);
    EXPECT_ERROR("Precision value is too big", "{0:.{1}}", 0, (numeric<s32>::max() + 1u));
    EXPECT_ERROR("Negative precision", "{0:.{1}}", 0, -1l);
    EXPECT_ERROR("Precision value is too big", "{0:.{1}}", 0, (numeric<s32>::max() + 1ul));

    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}c}", 0, '0');
    EXPECT_ERROR("Precision was not an integer", "{0:.{1}}", 0, 0.0);

    EXPECT_ERROR("\"}\" expected", "{0:.{1}", 0, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}f}", 42, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}f}", 42u, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}f}", 42l, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}f}", 42ul, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}f}", 42ll, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}f}", 42ull, 2);
    EXPECT_ERROR("Invalid type specifier for an integer", "{0:.{1}%}", 42, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}}", 42, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}}", 42u, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}}", 42l, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}}", 42ul, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}}", 42ll, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:.{1}}", 42ull, 2);
    EXPECT_ERROR("Precision is not allowed for integer types", "{0:3.{1}c}", 'c', 0);

    CHECK_WRITE("1.2", "{0:.{1}}", 1.2345, 2);

    EXPECT_ERROR("Precision is not allowed for pointer type", "{0:.{1}}", (void *) 0xcafe, 2);
    EXPECT_ERROR("Invalid type specifier for a pointer", "{0:.{1}f}", (void *) 0xcafe, 2);

    CHECK_WRITE("st", "{0:.{1}}", "str", 2);
}

TEST(colors_and_emphasis) {
    if (Context.FmtDisableAnsiCodes) return;

    EXPECT_ERROR(
        "Invalid emphasis character - "
        "valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)",
        "{!L}");
    EXPECT_ERROR(
        "Invalid emphasis character - "
        "valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)",
        "{!BLUE;BL}");
    EXPECT_ERROR(
        "Invalid emphasis character - "
        "valid ones are: B (bold), I (italic), U (underline) and S (strikethrough)",
        "{!BG}");

    EXPECT_ERROR("Channel value too big - it must be in the range [0-255]", "{!256;0;0}");
    EXPECT_ERROR("Channel value too big - it must be in the range [0-255]", "{!0;300;0}");
    EXPECT_ERROR("\";\" expected followed by the next channel value", "{!0.0}");
    EXPECT_ERROR("\";\" expected followed by the next channel value", "{!0;0}");
    EXPECT_ERROR("Expected an integer specifying a channel value (3 channels required)", "{!0;0;}");
    EXPECT_ERROR("\"}\" expected (or \";\" for BG specifier or emphasis)", "{!0;0;0.}");

    EXPECT_ERROR("Invalid color name - it must be a valid identifier (without digits)", "{!BL9UE}");

    CHECK_WRITE("\x1b[38;2;255;020;030m", "{!255;20;30}");
    CHECK_WRITE("\x1b[38;2;000;000;255m", "{!BLUE}");
    CHECK_WRITE("\x1b[38;2;000;000;255m\x1b[48;2;255;000;000m", "{!BLUE}{!RED;BG}");
    CHECK_WRITE("\x1b[1m", "{!B}");
    CHECK_WRITE("\x1b[3m", "{!I}");
    CHECK_WRITE("\x1b[4m", "{!U}");
    CHECK_WRITE("\x1b[9m", "{!S}");
    CHECK_WRITE("\x1b[38;2;000;000;255m\x1b[1m", "{!BLUE;B}");
    CHECK_WRITE("\x1b[31m", "{!tRED}");
    CHECK_WRITE("\x1b[46m", "{!tCYAN;BG}");
    CHECK_WRITE("\x1b[92m", "{!tBRIGHT_GREEN}");
    CHECK_WRITE("\x1b[105m", "{!tBRIGHT_MAGENTA;BG}");
}
