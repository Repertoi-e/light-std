#include <lstd/memory/string_builder.h>

#include "../test.h"

TEST(code_point_size) {
    string ascii = "abc";
    assert_eq(ascii.Count, 3);
    assert_eq(ascii.Length, 3);

    string cyrillic = u8"абв";
    assert_eq(cyrillic.Count, 6);
    assert_eq(cyrillic.Length, 3);

    string devanagari = u8"\u0904\u0905\u0906";
    assert_eq(devanagari.Count, 9);
    assert_eq(devanagari.Length, 3);

    string supplementary = u8"\U0002070E\U00020731\U00020779";
    assert_eq(supplementary.Count, 12);
    assert_eq(supplementary.Length, 3);

    string mixed;
    string_append(mixed, ascii);
    string_append(mixed, cyrillic);
    string_append(mixed, devanagari);
    string_append(mixed, supplementary);

    assert_eq(mixed.Count, 12 + 9 + 6 + 3);
    assert_eq(mixed.Length, 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    assert_eq((a[{2, 5}]), "llo");
    assert_eq((a[{7, a.Length}]), "world!");
    assert_eq((a[{0, -1}]), "Hello, world");
    assert_eq((a[{-6, -1}]), "world");
}

TEST(substring_mixed_sizes) {
    string a = u8"Хеllo, уоrлd!";
    assert_eq((a[{2, 5}]), "llo");
    assert_eq((a[{7, a.Length}]), u8"уоrлd!");
    assert_eq((a[{0, -1}]), u8"Хеllo, уоrлd");
    assert_eq((a[{-6, -1}]), u8"уоrлd");
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

TEST(insert) {
    string a = "e";
    string_insert_at(a, 1, 'l');
    string_insert_at(a, 0, 'H');
    assert_eq(a, "Hel");

    string_insert_at(a, 3, "lo");
    assert_eq(a, "Hello");

    string_insert_at(a, 0, "Hello ");
    assert_eq(a, "Hello Hello");

    string_insert_at(a, 5, " world");
    assert_eq(a, "Hello world Hello");
    free(a);
}

TEST(remove) {
    string a = "Hello world Hello";
    string_remove_range(a, -6, a.Length);
    assert_eq(a, "Hello world");
    string_remove_at(a, 1);
    assert_eq(a, "Hllo world");
    string_remove_at(a, 1);
    assert_eq(a, "Hlo world");
    string_remove_at(a, 0);
    assert_eq(a, "lo world");
    string_remove_at(a, -1);
    assert_eq(a, "lo worl");
    string_remove_at(a, -2);
    assert_eq(a, "lo wol");
    free(a);

    a = "Hello world";
    string_remove_range(a, 0, 5);
    assert_eq(a, " world");
    free(a);
}

TEST(trim) {
    string a = "\t\t    Hello, everyone!   \t\t   \n";
    assert_eq(trim_start(a), "Hello, everyone!   \t\t   \n");
    assert_eq(trim_end(a), "\t\t    Hello, everyone!");
    assert_eq(trim(a), "Hello, everyone!");
}

TEST(match_beginning) {
    string a = "Hello, world!";
    assert_true(match_beginning(a, "Hello"));
    assert_false(match_beginning(a, "Xello"));
    assert_false(match_beginning(a, "Hellol"));
}

TEST(match_end) {
    string a = "Hello, world!";
    assert_true(match_end(a, "world!"));
    assert_false(match_end(a, "!world!"));
    assert_false(match_end(a, "world!!"));
}

TEST(set) {
    string a = "aDc";
    string_set(a, 1, 'b');
    assert_eq(a, "abc");
    string_set(a, 1, U'Д');
    assert_eq(a, u8"aДc");
    string_set(a, 1, 'b');
    assert_eq(a, "abc");
    assert_eq(a[0], 'a');
    assert_eq(a[1], 'b');
    assert_eq(a[2], 'c');
    free(a);

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
    free(a);
}

TEST(iterator) {
    string a = "Hello";

    string result = "";
    for (auto ch : a) {
        string_append(result, ch);
    }
    assert_eq(result, a);

    string b = "HeLLo";
    // In order to modify a character, use a string::code_point
    // This will be same as writing "for (string::code_point ch : b)", since b is non-const.
    // Note that when b is const, the type of ch is just utf32 (you can't take a code point reference)
    for (auto ch : b) {
        ch = to_lower(ch);
    }
    assert_eq(b, "hello");
    for (auto ch : b) {
        ch = U'Д';
    }
    assert_eq(b, u8"ДДДДД");

    // for (utf32 &ch : b) { .. }
    // doesn't work since string isn't
    // actually an array of utf32
}

TEST(append) {
    {
        string result = "Hello";
        string_append(result, ",THIS IS GARBAGE", 1);
        string_append(result, " world!");

        assert_eq(result, "Hello, world!");
        free(result);
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";
        string result;
        string_append(result, a);
        string_append(result, b);
        string_append(result, c);

        assert_eq(result, "Hello, world!");
        free(result);
    }

    string result;
    For(range(10)) {
        string_append(result, 'i');
        assert_eq(result.Count, it + 1);
        assert_eq(result.Length, it + 1);
    }
    free(result);
    For(range(10)) {
        string_append(result, u8"Д");
        assert_eq(result.Count, 2 * (it + 1));
        assert_eq(result.Length, it + 1);
    }
    free(result);
}

TEST(count) {
    string a = "Hello";
    assert_eq(count(a, 'l'), 2);
    assert_eq(count(a, 'e'), 1);
    assert_eq(count(a, 'o'), 1);
}

TEST(builder) {
    string_builder builder;
    string_append(builder, "Hello");
    string_append(builder, ",THIS IS GARBAGE", 1);
    string_append(builder, string(" world"));
    string_append(builder, '!');
    defer(free(builder));

    string result;
    result = combine(builder);
    defer(free(result));
    assert_eq(result, "Hello, world!");
}

TEST(remove_all) {
    string a = "Hello world!";
    string b = a;

    string_remove_all(b, 'l');
    assert_eq(b, "Heo word!");
    free(b);

    b = a;
    string_remove_all(b, "ll");
    assert_eq(b, "Heo world!");
    free(b);

    b = a;
    string_remove_all(a, "x");
    assert_eq(b, a);
    free(b);

    b = "llHello world!ll";
    string_remove_all(b, 'l');
    assert_eq(b, "Heo word!");
    free(b);

    b = "llHello world!ll";
    string_remove_all(b, "ll");
    assert_eq(b, "Heo world!");
    free(b);
}

TEST(replace_all) {
    string a = "Hello world!";
    string b = a;

    string_replace_all(b, "l", "ll");
    assert_eq(b, "Hellllo worlld!");
    free(b);

    b = a;
    string_replace_all(b, "l", "");

    string c = a;
    string_remove_all(c, 'l');
    assert_eq(b, c);
    free(b);
    free(c);

    b = a;
    string_replace_all(b, "x", "");
    assert_eq(b, a);
    free(b);

    b = a;
    string_replace_all(b, "Hello", "olleH");
    assert_eq(b, "olleH world!");
    free(b);

    b = a = "llHello world!ll";
    string_replace_all(b, "ll", "l");
    assert_eq(b, "lHelo world!l");
    free(b);

    b = a;
    string_replace_all(b, "l", "ll");
    assert_eq(b, "llllHellllo worlld!llll");
    free(b);

    b = a;
    string_replace_all(b, "l", "K");
    assert_eq(b, "KKHeKKo worKd!KK");
    free(b);
}

TEST(find) {
    string a = "This is a string";
    assert_eq(2, find_substring(a, "is"));
    assert_eq(5, find_substring(a, "is", 5));

    assert_eq(0, find_substring(a, "This"));
    assert_eq(0, find_substring_reverse(a, "This"));
    assert_eq(10, find_substring(a, "string"));
    assert_eq(10, find_substring_reverse(a, "string"));

    assert_eq(5, find_substring_reverse(a, "is", 6));
    assert_eq(2, find_substring_reverse(a, "is", 5));
    assert_eq(2, find_substring_reverse(a, "is", 3));

    assert_eq(1, find_cp(a, 'h'));
    assert_eq(1, find_cp(a, 'h', 1));
    assert_eq(1, find_substring(a, "h", 1));

    assert_eq(0, find_cp(a, 'T'));
    assert_eq(0, find_cp_reverse(a, 'T'));

    assert_eq(13, find_cp_reverse(a, 'i'));
    assert_eq(5, find_cp_reverse(a, 'i', 13));
    assert_eq(2, find_cp_reverse(a, 'i', 5));

    assert_eq(a.Length - 1, find_cp(a, 'g'));
    assert_eq(a.Length - 1, find_cp_reverse(a, 'g'));

    assert_eq(1, find_cp_not(a, 'T'));
    assert_eq(0, find_cp_not(a, 'Q'));
    assert_eq(a.Length - 1, find_cp_reverse_not(a, 'Q'));
    assert_eq(a.Length - 2, find_cp_reverse_not(a, 'g'));

    assert_eq(-1, find_cp(a, 'Q'));

    a = u8"Това е низ от букви";
    assert_eq(8, find_substring(a, u8"и"));
    assert_eq(8, find_substring(a, u8"и", 8));

    assert_eq(8, find_cp(a, U'и'));
    assert_eq(8, find_cp(a, U'и', 8));

    assert_eq(14, find_cp(a, U'б'));
    assert_eq(14, find_cp_reverse(a, U'б'));

    assert_eq(-1, find_cp(a, U'я'));

    a = "aaabbbcccddd";
    assert_eq(3, find_any_of(a, "DCb"));
    assert_eq(3, find_any_of(a, "CbD"));
    assert_eq(0, find_any_of(a, "PQa"));

    assert_eq(2, find_reverse_any_of(a, "PQa"));
    assert_eq(1, find_reverse_any_of(a, "PQa", 2));
    assert_eq(0, find_reverse_any_of(a, "PQa", 1));

    assert_eq(find_cp(a, 'd'), find_not_any_of(a, "abc"));
    assert_eq(0, find_not_any_of(a, "bcd"));
    assert_eq(find_cp(a, 'b'), find_not_any_of(a, "ac"));

    assert_eq(2, find_reverse_not_any_of(a, "bcd"));
    assert_eq(2, find_reverse_not_any_of(a, "bc", -3));
    assert_eq(2, find_reverse_not_any_of(a, "bc", -4));
    assert_eq(0, find_reverse_not_any_of(a, "bcd", 1));

    assert_eq(a.Length - 1, find_reverse_any_of(a, "CdB"));

    assert_eq(-1, find_any_of(a, "QRT"));
}
