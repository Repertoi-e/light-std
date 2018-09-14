#include <gu/memory/pool.h>
#include <gu/memory/stack.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "../test.h"

TEST(code_point_size) {
    string ascii = "abc";
    assert(ascii.BytesUsed == 3 && ascii.Length == 3);

    string cyrillic = u8"\u0410\u0431\u0432";
    assert(cyrillic.BytesUsed == 6 && cyrillic.Length == 3);

    string devanagari = u8"\u0904\u0905\u0906";
    assert(devanagari.BytesUsed == 9 && devanagari.Length == 3);

    string supplementary = u8"\U0002070E\U00020731\U00020779";
    assert(supplementary.BytesUsed == 12 && supplementary.Length == 3);

    string mixed = ascii + cyrillic + devanagari + supplementary;
    assert(mixed.BytesUsed == 12 + 9 + 6 + 3 && mixed.Length == 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    string b = substring(a, 2, 5);
    assert(b == "llo");
    b = substring(a, 7, a.Length);
    assert(b == "world!");
    b = substring(a, 0, -1);
    assert(b == "Hello, world");
    b = substring(a, -6, -1);
    assert(b == "world");
}

TEST(substring_operator) {
    string a = "Hello, world!";
    string b = a(2, 5);
    assert(b == "llo");
    b = a(7, a.Length);
    assert(b == "world!");
    b = a(0, -1);
    assert(b == "Hello, world");
    b = a(-6, -1);
    assert(b == "world");
}

TEST(string_modify_and_index) {
    string sample = u8"a\u0431c";

    string a = "aDc";
    set(a, 1, 'b');
    assert(a == "abc");
    set(a, 1, U'\u0431');
    assert(a == sample);
    set(a, 1, 'b');
    assert(a == "abc");
    assert(get(a, 0) == 'a' && get(a, 1) == 'b' && get(a, 2) == 'c');

    a = "aDc";
    a[-2] = 'b';
    assert(a == "abc");
    a[1] = U'\u0431';
    assert(a == sample);
    a[1] = 'b';
    assert(a == "abc");
    assert(a[0] == 'a' && a[1] == 'b' && a[2] == 'c');

    a[-3] = U'\U0002070E';
    a[-2] = U'\U00020731';
    a[-1] = U'\U00020779';
    assert(a == u8"\U0002070E\U00020731\U00020779");
}

TEST(string_concat) {
    {
        string result = "Hello";
        append_pointer_and_size(result, ",THIS IS GARBAGE", 1);
        append_cstring(result, " world!");

        assert(result == "Hello, world!");
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";
        string result = a + b + c;

        assert(result == "Hello, world!");
    }

    string result;
    for (s32 i = 0; i < 10; i++) {
        result += 'i';
        assert(result.BytesUsed == i + 1 && result.Length == i + 1);
    }
    release(result);
    for (s32 i = 0; i < 10; i++) {
        result += u8"\u0434";
        assert(result.BytesUsed == 2 * (i + 1) && result.Length == i + 1);
    }
}

TEST(string_builder) {
    String_Builder builder;
    append_cstring(builder, "Hello");
    append_pointer_and_size(builder, ",THIS IS GARBAGE", 1);
    append(builder, string(" world"));
    append(builder, '!');

    string result = to_string(builder);
    assert(result == "Hello, world!");
}

TEST(format_string) {
    assert(to_string("Hello, world!", 0) == "Hello, world!");
    assert(to_string("Hello, world!", 20) == "Hello, world!       ");
    assert(to_string("Hello, world!", 13) == "Hello, world!");
    assert(to_string("Hello, world!", 12) == "Hello, wo...");
    assert(to_string("Hello, world!", 4) == "H...");
    assert(to_string("Hello, world!", 3) == "...");
    assert(to_string("Hello, world!", 2) == "..");
    assert(to_string("Hello, world!", 1) == ".");
    assert(to_string("Hello, world!", -1) == ".");
    assert(to_string("Hello, world!", -2) == "..");
    assert(to_string("Hello, world!", -3) == "...");
    assert(to_string("Hello, world!", -4) == "...!");
    assert(to_string("Hello, world!", -12) == "...o, world!");
    assert(to_string("Hello, world!", -13) == "Hello, world!");
    assert(to_string("Hello, world!", -20) == "       Hello, world!");
}

TEST(format_float) {
    assert(to_string(2.40) == "2.400000");
    assert(to_string(2.12359012385, 0, 3) == "2.124");
    assert(to_string(123512.1241242222222222, 8, 9) == "123512.124124222");
    assert(to_string(22135.42350, 20, 1) == "             22135.4");
    assert(to_string(2.40, 21, 2) == "                 2.40");
    assert(to_string(2.40, 10, 0) == "         2");
    assert(to_string(2.40, 10, 1) == "       2.4");
}

TEST(format_integer) {
    assert(to_string(1024, Base(2)) == "10000000000");
    assert(to_string(std::numeric_limits<u8>::max(), Base(10), 30) == "000000000000000000000000000255");
    assert(to_string(std::numeric_limits<u8>::max(), Base(2)) == "11111111");

    assert(to_string(std::numeric_limits<s64>::min(), Base(10)) == "-9223372036854775808");
    assert(to_string(std::numeric_limits<s64>::min(), Base(2), 30) ==
           "-1000000000000000000000000000000000000000000000000000000000000000");

    assert(to_string(std::numeric_limits<u64>::max(), Base(10)) == "18446744073709551615");
    assert(to_string(std::numeric_limits<u64>::max(), Base(2), 30) ==
           "1111111111111111111111111111111111111111111111111111111111111111");
}

TEST(sprint) {
    string print1 = tprint("My name is %2 and my bank interest is %1%%.", to_string(152.29385, 0, 2), "Dotra");
    string print2 = tprint("My name is % and my bank interest is %2%%.", "Dotra", to_string(152.29385, 0, 2));

    assert(print1 == "My name is Dotra and my bank interest is 152.29%.");
    assert(print2 == "My name is Dotra and my bank interest is 152.29%.");
}
