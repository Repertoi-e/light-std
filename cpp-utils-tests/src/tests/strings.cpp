#include <gu/memory/pool.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "../test.h"

TEST(code_point_size) {
    string ascii = "abc";
    assert(ascii.BytesUsed == 3 && ascii.Length == 3);

    string cyrillic = u8"абв";
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
    string_view b = a.substring(2, 5);
    assert(b == "llo");
    b = a.substring(7, a.Length);
    assert(b == "world!");
    b = a.substring(0, -1);
    assert(b == "Hello, world");
    b = a.substring(-6, -1);
    assert(b == "world");
}

TEST(substring_operator) {
    string a = "Hello, world!";
    string_view b = a(2, 5);
    assert(b == "llo");
    b = a(7, a.Length);
    assert(b == "world!");
    b = a(0, -1);
    assert(b == "Hello, world");
    b = a(-6, -1);
    assert(b == "world");
}

TEST(substring_mixed_sizes) {
    string a = u8"Хеllo, уоrлd!";
    string_view b = a.substring(2, 5);
    assert(b == "llo");
    b = a.substring(7, a.Length);
    assert(b == u8"уоrлd!");
    b = a.substring(0, -1);
    assert(b == u8"Хеllo, уоrлd");
    b = a.substring(-6, -1);
    assert(b == u8"уоrлd");
}

TEST(utility_functions) {
    string a = "\t\t    Hello, everyone!   \t\t   \n";
    assert(a.trim_start() == "Hello, everyone!   \t\t   \n");
    assert(a.trim_end() == "\t\t    Hello, everyone!");
    assert(a.trim() == "Hello, everyone!");

    string b = "Hello, world!";
    assert(b.begins_with("Hello"));
    assert(!b.begins_with("Xello"));
    assert(!b.begins_with("Hellol"));

    assert(b.ends_with("world!"));
    assert(!b.ends_with("!world!"));
    assert(!b.ends_with("world!!"));
}

TEST(modify_and_index) {
    string a = "aDc";
    a.set(1, 'b');
    assert(a == "abc");
    a.set(1, U'Д');
    assert(a == u8"aДc");
    a.set(1, 'b');
    assert(a == "abc");
    assert(a.get(0) == 'a' && a.get(1) == 'b' && a.get(2) == 'c');

    a = "aDc";
    a[-2] = 'b';
    assert(a == "abc");
    a[1] = U'Д';
    assert(a == u8"aДc");
    a[1] = 'b';
    assert(a == "abc");
    assert(a[0] == 'a' && a[1] == 'b' && a[2] == 'c');

    a[-3] = U'\U0002070E';
    a[-2] = U'\U00020731';
    a[-1] = U'\U00020779';
    assert(a == u8"\U0002070E\U00020731\U00020779");
}

TEST(iterator) {
    const string a = "Hello";
    string result = "";
    for (char32_t ch : a) {
        result += ch;
    }
    assert(result == a);
    result.clear();

    string b = "Hello";
    // In order to modify a character, use a string::Code_Point_Ref variable
    // This will be same as writing "for (auto ch : b)", since b is non-const.
    for (string::Code_Point_Ref ch : b) {
        ch = to_lower(ch);
    }
    assert(b == "hello");

    // You can also do this type of loop with 
	// the non-const b when you don't plan on
	// modifying the character:
    // for (char32_t ch : b) { .. }
    //
    // But this doesn't work since string isn't
	// actually an array of char32_t's
    // for (char32_t &ch : b) { .. }
    
	for (string::Code_Point_Ref ch : b) {
		ch = U'Д';
    }
    assert(b == u8"ДДДДД");
}

TEST(concat) {
    {
        string result = "Hello";
        result.append_pointer_and_size(",THIS IS GARBAGE", 1);
        result.append_cstring(" world!");

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
    result.release();
    for (s32 i = 0; i < 10; i++) {
        result += u8"Д";
        assert(result.BytesUsed == 2 * (i + 1) && result.Length == i + 1);
    }
}

TEST(string_find) {
	string a = "Hello";
	assert(a.find('e') == 1);
	assert(a.find('l') == 2);
	assert(a.find_last('l') == 3);

	a = u8"Здрello";
	assert(a.find('e') == 3);
	assert(a.find('l') == 4);
	assert(a.find_last('l') == 5);
	assert(a.find_last('o') == 6);
}

TEST(string_builder) {
    String_Builder builder;
    builder.append_cstring("Hello");
    builder.append_pointer_and_size(",THIS IS GARBAGE", 1);
    builder.append(string(" world"));
    builder.append('!');

    string result = to_string(builder);
    assert(result == "Hello, world!");
}

TEST(format_string) {
    assert(to_string("Hello, world!", 0) == "Hello, world!");
    assert(to_string("Hello, world!", 20) == "Hello, world!       ");
    assert(to_string("Hello, world!", 13) == "Hello, world!");
    assert(to_string("Hello, world!", 12) == "Hello, wo...");
    assert(to_string("Hello, world!", 3) == "...");
    assert(to_string("Hello, world!", 2) == "..");
    assert(to_string("Hello, world!", 1) == ".");
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
