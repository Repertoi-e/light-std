#include "../test.h"

TEST(code_point_size) {
    string ascii = "abc";
    assert_eq(ascii.Count, 3);
    assert_eq(string_length(ascii), 3);

    string cyrillic = u8"абв";
    assert_eq(cyrillic.Count, 6);
    assert_eq(string_length(cyrillic), 3);

    string devanagari = u8"\u0904\u0905\u0906";
    assert_eq(devanagari.Count, 9);
    assert_eq(string_length(devanagari), 3);

    string supplementary = u8"\U0002070E\U00020731\U00020779";
    assert_eq(supplementary.Count, 12);
    assert_eq(string_length(supplementary), 3);

    string mixed;
    make_dynamic(&mixed, 50);
    defer(free(mixed.Data));

    string_append(&mixed, ascii);
    string_append(&mixed, cyrillic);
    string_append(&mixed, devanagari);
    string_append(&mixed, supplementary);

    assert_eq(mixed.Count, 12 + 9 + 6 + 3);
    assert_eq(string_length(mixed), 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    assert_eq(substring(a, 2, 5), string("llo"));
    assert_eq(substring(a, 7, string_length(a)), string("world!"));
    assert_eq(substring(a, 0, -1), string("Hello, world"));
    assert_eq(substring(a, -6, -1), string("world"));
}

TEST(substring_mixed_sizes) {
    string a = u8"Хеllo, уоrлd!";
    assert_eq(substring(a, 2, 5), string("llo"));
    assert_eq(substring(a, 7, string_length(a)), string(u8"уоrлd!"));
    assert_eq(substring(a, 0, -1), string(u8"Хеllo, уоrлd"));
    assert_eq(substring(a, -6, -1), string(u8"уоrлd"));
}

TEST(index) {
    string a = "Hello";
    make_dynamic(&a, 8);
    defer(free(a.Data));

    assert_eq(a[0], 'H');
    assert_eq(a[1], 'e');
    assert_eq(a[2], 'l');
    assert_eq(a[3], 'l');
    assert_eq(a[4], 'o');

    string_get(&a, 0) = 'X';
    assert_eq(a[0], 'X');
}

TEST(insert) {
    string a = "e";
    make_dynamic(&a, 8);
    defer(free(a.Data));

    string_insert_at_index(&a, 1, 'l');
    string_insert_at_index(&a, 0, 'H');
    assert_eq(a, string("Hel"));

    string_insert_at_index(&a, 3, "lo");
    assert_eq(a, string("Hello"));

    string_insert_at_index(&a, 0, "Hello ");
    assert_eq(a, string("Hello Hello"));

    string_insert_at_index(&a, 5, " world");
    assert_eq(a, string("Hello world Hello"));
}

TEST(remove) {
    string a = "Hello world Hello";
    make_dynamic(&a, 20);

    string_remove_range(&a, -6, string_length(a));
    assert_eq(a, string("Hello world"));
    string_remove_at_index(&a, 1);
    assert_eq(a, string("Hllo world"));
    string_remove_at_index(&a, 1);
    assert_eq(a, string("Hlo world"));
    string_remove_at_index(&a, 0);
    assert_eq(a, string("lo world"));
    string_remove_at_index(&a, -1);
    assert_eq(a, string("lo worl"));
    string_remove_at_index(&a, -2);
    assert_eq(a, string("lo wol"));
    free(a.Data);

    a = "Hello world";
    make_dynamic(&a, 20);

    string_remove_range(&a, 0, 5);
    assert_eq(a, string(" world"));
    free(a.Data);
}

TEST(trim) {
    string a = "\t\t    Hello, everyone!   \t\t   \n";
    assert_eq(trim_start(a), string("Hello, everyone!   \t\t   \n"));
    assert_eq(trim_end(a), string("\t\t    Hello, everyone!"));
    assert_eq(trim(a), string("Hello, everyone!"));
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
    make_dynamic(&a, 20);

    string_set(&a, 1, 'b');
    assert_eq(a, string("abc"));
    string_set(&a, 1, U'Д');
    assert_eq(a, string(u8"aДc"));
    string_set(&a, 1, 'b');
    assert_eq(a, string("abc"));
    assert_eq(a[0], 'a');
    assert_eq(a[1], 'b');
    assert_eq(a[2], 'c');
    free(a.Data);

    a = "aDc";
    make_dynamic(&a, 8);

    string_get(&a, -2) = 'b';
    assert_eq(a, string("abc"));
    string_get(&a, 1) = U'Д';
    assert_eq(a, string(u8"aДc"));
    string_get(&a, 1) = 'b';
    assert_eq(a, string("abc"));
    assert_eq(a[0], 'a');
    assert_eq(a[1], 'b');
    assert_eq(a[2], 'c');

    string_get(&a, -3) = U'\U0002070E';
    string_get(&a, -2) = U'\U00020731';
    string_get(&a, -1) = U'\U00020779';
    assert_eq(a, string(u8"\U0002070E\U00020731\U00020779"));
    free(a.Data);
}

TEST(iterator) {
    string a = "Hello";
    make_dynamic(&a, 10);

    string result = "";
    make_dynamic(&result, 10);
    for (auto ch : a) {
        string_append(&result, ch);
    }
    assert_eq(result, a);

    string b = "HeLLo";
    make_dynamic(&b, 10);

    // In order to modify a character, use a string::code_point
    // This will be same as writing "for (string::code_point ch : b)", since b is non-const.
    // Note that when b is const, the type of ch is just utf32 (you can't take a code point reference)
    for (auto ch : b) {
        ch = to_lower(ch);
    }
    assert_eq(b, string("hello"));
    for (auto ch : b) {
        ch = U'Д';
    }
    assert_eq(b, string(u8"ДДДДД"));

    // for (utf32 &ch : b) { .. }
    // doesn't work since string isn't
    // actually an array of utf32
}

TEST(append) {
    {
        string result = "Hello";
        make_dynamic(&result, 20);

        string_append(&result, ",THIS IS GARBAGE", 1);
        string_append(&result, " world!");

        assert_eq(result, string("Hello, world!"));
        free(result.Data);
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";

        string result;
        make_dynamic(&result, 50);

        string_append(&result, a);
        string_append(&result, b);
        string_append(&result, c);

        assert_eq(result, string("Hello, world!"));
        free(result.Data);
    }

    {
        string result;
        make_dynamic(&result, 10);

        For(range(10)) {
            string_append(&result, 'i');
            assert_eq(result.Count, it + 1);
            assert_eq(string_length(result), it + 1);
        }
        free(result.Data);
    }

    {
        string result;
        make_dynamic(&result, 20);
        For(range(10)) {
            string_append(&result, u8"Д");
            assert_eq(result.Count, 2 * (it + 1));
            assert_eq(string_length(result), it + 1);
        }
        free(result.Data);
    }
}

TEST(builder) {
    string_builder builder;
    append(&builder, "Hello");
    append(&builder, ",THIS IS GARBAGE", 1);
    append(&builder, string(" world"));
    append(&builder, '!');
    defer(free_buffers(&builder));

    string result = builder_to_string(&builder);
    defer(free(result.Data));
    assert_eq(result, string("Hello, world!"));
}

TEST(remove_all) {
    string a = "Hello world!";

    string b = a;
    make_dynamic(&b, 20);

    remove_all(&b, 'l');
    assert_eq(b, string("Heo word!"));
    free(b.Data);

    b = a;
    make_dynamic(&b, 20);

    remove_all(&b, "ll");
    assert_eq(b, string("Heo world!"));
    free(b.Data);

    b = a;
    make_dynamic(&b, 20);

    make_dynamic(&a, 20);
    remove_all(&a, "x");
    assert_eq(b, a);
    free(b.Data);
    free(a.Data);

    b = "llHello world!ll";
    make_dynamic(&b, 20);

    remove_all(&b, 'l');
    assert_eq(b, string("Heo word!"));
    free(b.Data);

    b = "llHello world!ll";
    make_dynamic(&b, 20);

    remove_all(&b, "ll");
    assert_eq(b, string("Heo world!"));
    free(b.Data);
}

TEST(replace_all) {
    string a = "Hello world!";

    string b = a;
    make_dynamic(&b, 20);

    replace_all(&b, string("l"), string("ll"));
    assert_eq(b, string("Hellllo worlld!"));
    free(b.Data);

    b = a;
    make_dynamic(&b, 20);

    replace_all(&b, string("l"), string(""));

    string c = a;
    make_dynamic(&c, 20);

    remove_all(&c, 'l');
    assert_eq(b, c);
    free(b.Data);
    free(c.Data);

    b = a;
    make_dynamic(&b, 20);

    replace_all(&b, string("x"), string(""));
    assert_eq(b, a);
    free(b.Data);

    b = a;
    make_dynamic(&b, 20);

    replace_all(&b, string("Hello"), string("olleH"));
    assert_eq(b, string("olleH world!"));
    free(b.Data);

    b = a = "llHello world!ll";
    make_dynamic(&b, 20);

    replace_all(&b, string("ll"), string("l"));
    assert_eq(b, string("lHelo world!l"));
    free(b.Data);

    b = a;
    make_dynamic(&b, 20);

    replace_all(&b, string("l"), string("ll"));
    assert_eq(b, string("llllHellllo worlld!llll"));
    free(b.Data);

    b = a;
    make_dynamic(&b, 20);

    replace_all(&b, string("l"), string("K"));
    assert_eq(b, string("KKHeKKo worKd!KK"));
    free(b.Data);
}

TEST(find) {
    string a = "This is a string";
    assert_eq(2, string_find(a, string("is")));
    assert_eq(5, string_find(a, string("is"), 5));

    assert_eq(0, string_find(a, string("This")));
    assert_eq(0, string_find(a, string("This"), -1, true));
    assert_eq(10, string_find(a, string("string")));
    assert_eq(10, string_find(a, string("string"), -1, true));

    assert_eq(5, string_find(a, string("is"), 6, true));
    assert_eq(5, string_find(a, string("is"), 5, true));
    assert_eq(2, string_find(a, string("is"), 3, true));
    assert_eq(2, string_find(a, string("is"), 2, true));
    assert_eq(-1, string_find(a, string("is"), 1, true));

    assert_eq(1, string_find(a, 'h'));
    assert_eq(1, string_find(a, 'h', 1));
    assert_eq(1, string_find(a, string("h"), 1));

    assert_eq(0, string_find(a, 'T'));
    assert_eq(0, string_find(a, 'T', -1, true));

    assert_eq(13, string_find(a, 'i', -1, true));
    assert_eq(13, string_find(a, 'i', 13, true));
    assert_eq(5, string_find(a, 'i', 12, true));
    assert_eq(5, string_find(a, 'i', 5, true));
    assert_eq(2, string_find(a, 'i', 4, true));

    assert_eq(string_length(a) - 1, string_find(a, 'g'));
    assert_eq(string_length(a) - 1, string_find(a, 'g', -1, true));

    assert_eq(1, string_find_not(a, 'T'));
    assert_eq(0, string_find_not(a, 'Q'));
    assert_eq(string_length(a) - 1, string_find_not(a, 'Q', -1, true));
    assert_eq(string_length(a) - 2, string_find_not(a, 'g', -1, true));

    assert_eq(-1, string_find(a, 'Q'));

    a = u8"Това е низ от букви";
    assert_eq(8, string_find(a, string(u8"и")));
    assert_eq(8, string_find(a, string(u8"и"), 8));

    assert_eq(8, string_find(a, U'и'));
    assert_eq(8, string_find(a, U'и', 8));

    assert_eq(14, string_find(a, U'б'));
    assert_eq(14, string_find(a, U'б', -1, true));

    assert_eq(-1, string_find(a, U'я'));

    a = "aaabbbcccddd";
    assert_eq(3, string_find_any_of(a, "DCb"));
    assert_eq(3, string_find_any_of(a, "CbD"));
    assert_eq(0, string_find_any_of(a, "PQa"));

    assert_eq(2, string_find_any_of(a, "PQa", -1, true));
    assert_eq(2, string_find_any_of(a, "PQa", 2, true));
    assert_eq(1, string_find_any_of(a, "PQa", 1, true));
    assert_eq(0, string_find_any_of(a, "PQa", 0, true));

    assert_eq(string_find(a, 'd'), string_find_not_any_of(a, "abc"));
    assert_eq(0, string_find_not_any_of(a, "bcd"));
    assert_eq(string_find(a, 'b'), string_find_not_any_of(a, "ac"));

    assert_eq(2, string_find_not_any_of(a, "bcd", -1, true));
    assert_eq(9, string_find_not_any_of(a, "bc", -3, true));
    assert_eq(2, string_find_not_any_of(a, "bc", -4, true));
    assert_eq(1, string_find_not_any_of(a, "bcd", 1, true));
    assert_eq(0, string_find_not_any_of(a, "bcd", 0, true));

    assert_eq(string_length(a) - 1, string_find_any_of(a, "CdB", -1, true));

    assert_eq(-1, string_find_any_of(a, "QRT"));
}
