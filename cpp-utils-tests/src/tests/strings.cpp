#include <cppu/memory/pool.hpp>
#include <cppu/memory/table.hpp>

#include <cppu/format/fmt.hpp>

#include "../test.hpp"

TEST(code_point_size) {
    string ascii = "abc";
    assert_eq(ascii.ByteLength, 3);
    assert_eq(ascii.Length, 3);

    string cyrillic = u8"абв";
    assert_eq(cyrillic.ByteLength, 6);
    assert_eq(cyrillic.Length, 3);

    string devanagari = u8"\u0904\u0905\u0906";
    assert_eq(devanagari.ByteLength, 9);
    assert_eq(devanagari.Length, 3);

    string supplementary = u8"\U0002070E\U00020731\U00020779";
    assert_eq(supplementary.ByteLength, 12);
    assert_eq(supplementary.Length, 3);

    string mixed = ascii + cyrillic + devanagari + supplementary;
    assert_eq(mixed.ByteLength, 12 + 9 + 6 + 3);
    assert_eq(mixed.Length, 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    assert_eq(a.substring(2, 5), "llo");
    assert_eq(a.substring(7, a.Length), "world!");
    assert_eq(a.substring(0, -1), "Hello, world");
    assert_eq(a.substring(-6, -1), "world");

    assert_eq(a(2, 5), "llo");
    assert_eq(a(7, a.Length), "world!");
    assert_eq(a(0, -1), "Hello, world");
    assert_eq(a(-6, -1), "world");
}

TEST(substring_mixed_sizes) {
    string a = u8"Хеllo, уоrлd!";
    assert_eq(a.substring(2, 5), "llo");
    assert_eq(a.substring(7, a.Length), u8"уоrлd!");
    assert_eq(a.substring(0, -1), u8"Хеllo, уоrлd");
    assert_eq(a.substring(-6, -1), u8"уоrлd");
}

TEST(index) {
    string a = "Hello";
    assert_eq(a[0], 'H');
    assert_eq(a[1], 'e');
    assert_eq(a[2], 'l');
    assert_eq(a[3], 'l');
    assert_eq(a[4], 'o');

    a[0] = 'X';
    assert_eq(a[0], 'X');
}

TEST(add_and_remove) {
    string a = "e";
    a.add(1, 'l');
    a.add(0, 'H');
    assert_eq(a, "Hel");

    a.remove(1);
    assert_eq(a, "Hl");
    a.remove(1);
    assert_eq(a, "H");
    a.remove(0);
    assert_eq(a, "");
}

TEST(utility_functions) {
    string a = "\t\t    Hello, everyone!   \t\t   \n";
    assert_eq(a.trim_start(), "Hello, everyone!   \t\t   \n");
    assert_eq(a.trim_end(), "\t\t    Hello, everyone!");
    assert_eq(a.trim(), "Hello, everyone!");

    string b = "Hello, world!";
    assert_true(b.begins_with("Hello"));
    assert_false(b.begins_with("Xello"));
    assert_false(b.begins_with("Hellol"));

    assert_true(b.ends_with("world!"));
    assert_false(b.ends_with("!world!"));
    assert_false(b.ends_with("world!!"));
}

TEST(modify) {
    string a = "aDc";
    a.set(1, 'b');
    assert_eq(a, "abc");
    a.set(1, U'Д');
    assert_eq(a, u8"aДc");
    a.set(1, 'b');
    assert_eq(a, "abc");
    assert_eq(a.get(0), 'a');
    assert_eq(a.get(1), 'b');
    assert_eq(a.get(2), 'c');

    a = "aDc";
    a[-2] = 'b';
    assert_eq(a, "abc");
    a[1] = U'Д';
    assert_eq(a, u8"aДc");
    a[1] = 'b';
    assert_eq(a, "abc");
    assert_eq(a[0], 'a');
    assert_eq(a[1], 'b');
    assert_eq(a[2], 'c');

    a[-3] = U'\U0002070E';
    a[-2] = U'\U00020731';
    a[-1] = U'\U00020779';
    assert_eq(a, u8"\U0002070E\U00020731\U00020779");
}

TEST(iterator) {
    string a = "Hello";

    string result = "";
    for (auto ch : a) {
        result += ch;
    }
    assert_eq(result, a);

    string b = "HeLLo";
    // In order to modify a character, use a string::code_point
    // This will be same as writing "for (string::code_point ch : b)", since b is non-const.
    // Note that when b is const, the type of ch is just char32_t (you can't take a code point reference)
    for (auto ch : b) {
        ch = to_lower(ch);
    }
    assert_eq(b, "hello");
    for (auto ch : b) {
        ch = U'Д';
    }
    assert_eq(b, u8"ДДДДД");

    // for (char32_t &ch : b) { .. }
    // doesn't work since string isn't
    // actually an array of char32_t
}

TEST(concat) {
    {
        string result = "Hello";
        result.append_pointer_and_size(",THIS IS GARBAGE", 1);
        result.append_cstring(" world!");

        assert_eq(result, "Hello, world!");
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";
        string result = a + b + c;

        assert_eq(result, "Hello, world!");
    }

    string result;
    For(range(10)) {
        result += 'i';
        assert_eq(result.ByteLength, it + 1);
        assert_eq(result.Length, it + 1);
    }
    result.release();
    For(range(10)) {
        result += u8"Д";
        assert_eq(result.ByteLength, 2 * (it + 1));
        assert_eq(result.Length, it + 1);
    }
}

TEST(string_find) {
    string a = "Hello";
    assert_eq(a.find('e'), 1);
    assert_eq(a.find('l'), 2);
    assert_eq(a.find('l', 3), 3);
    assert_eq(a.find_last('l'), 3);
    assert_eq(a.find_last('l', 4), npos);

    a = u8"Здрello";
    assert_eq(a.find('e'), 3);
    assert_eq(a.find('l'), 4);
    assert_eq(a.find_last('l'), 5);
    assert_eq(a.find_last('o'), 6);
}

TEST(string_count) {
    string a = "Hello";
    assert_eq(a.count('l'), 2);
    assert_eq(a.count('e'), 1);
    assert_eq(a.count('o'), 1);
}

TEST(string_builder) {
    String_Builder builder;
    builder.append_cstring("Hello");
    builder.append_pointer_and_size(",THIS IS GARBAGE", 1);
    builder.append(string(" world"));
    builder.append('!');

    string result = builder.combine();
    assert_eq(result, "Hello, world!");
}
