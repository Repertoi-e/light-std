#include <cppu/memory/pool.h>
#include <cppu/memory/table.h>

#include <cppu/format/fmt.h>

#include "../test.h"

TEST(code_point_size) {
    string ascii = "abc";
    assert(ascii.ByteLength == 3 && ascii.Length == 3);

    string cyrillic = u8"абв";
    assert(cyrillic.ByteLength == 6 && cyrillic.Length == 3);

    string devanagari = u8"\u0904\u0905\u0906";
    assert(devanagari.ByteLength == 9 && devanagari.Length == 3);

    string supplementary = u8"\U0002070E\U00020731\U00020779";
    assert(supplementary.ByteLength == 12 && supplementary.Length == 3);

    string mixed = ascii + cyrillic + devanagari + supplementary;
    assert(mixed.ByteLength == 12 + 9 + 6 + 3 && mixed.Length == 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    assert(a.substring(2, 5) == "llo");
    assert(a.substring(7, a.Length) == "world!");
    assert(a.substring(0, -1) == "Hello, world");
    assert(a.substring(-6, -1) == "world");

    assert(a(2, 5) == "llo");
    assert(a(7, a.Length) == "world!");
    assert(a(0, -1) == "Hello, world");
    assert(a(-6, -1) == "world");
}

TEST(substring_mixed_sizes) {
    string a = u8"Хеllo, уоrлd!";
    assert(a.substring(2, 5) == "llo");
    assert(a.substring(7, a.Length) == u8"уоrлd!");
    assert(a.substring(0, -1) == u8"Хеllo, уоrлd");
    assert(a.substring(-6, -1) == u8"уоrлd");
}

TEST(index) {
    string a = "Hello";
    assert(a[0] == 'H');
    assert(a[1] == 'e');
    assert(a[2] == 'l');
    assert(a[3] == 'l');
    assert(a[4] == 'o');

    a[0] = 'X';
    assert(a[0] == 'X');
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

TEST(modify) {
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
    string a = "Hello";

    string result = "";
    for (auto ch : a) {
        result += ch;
    }
    assert(result == a);

    string b = "HeLLo";
    // In order to modify a character, use a string::code_point
    // This will be same as writing "for (string::code_point ch : b)", since b is non-const.
    // Note that when b is const, the type of ch is just char32_t (you can't take a code point reference)
    for (auto ch : b) {
        ch = to_lower(ch);
    }
    assert(b == "hello");
    for (auto ch : b) {
        ch = U'Д';
    }
    assert(b == u8"ДДДДД");

    // for (char32_t &ch : b) { .. }
    // doesn't work since string isn't
    // actually an array of char32_t
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
        assert(result.ByteLength == i + 1 && result.Length == i + 1);
    }
    result.release();
    for (s32 i = 0; i < 10; i++) {
        result += u8"Д";
        assert(result.ByteLength == 2 * (i + 1) && result.Length == i + 1);
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

    string result = fmt::to_string(builder);
    assert(result == "Hello, world!");
}
