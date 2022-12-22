#include "../test.h"

TEST(code_point_size) {
    string ascii = "abc";
    assert_eq(ascii.Count, 3);
    assert_eq(length(ascii), 3);

    string cyrillic = u8"абв";
    assert_eq(cyrillic.Count, 6);
    assert_eq(length(cyrillic), 3);

    string devanagari = u8"\u0904\u0905\u0906";
    assert_eq(devanagari.Count, 9);
    assert_eq(length(devanagari), 3);

    string supplementary = u8"\U0002070E\U00020731\U00020779";
    assert_eq(supplementary.Count, 12);
    assert_eq(length(supplementary), 3);

    string mixed;
    reserve(mixed, 12 + 9 + 6 + 3);
    defer(free(mixed));

    mixed += ascii;
    mixed += cyrillic;
    mixed += devanagari;
    mixed += supplementary;

    assert_eq(mixed.Count, 12 + 9 + 6 + 3);
    assert_eq(length(mixed), 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    assert_eq_str(slice(a, 2, 5), "llo");
    assert_eq_str(slice(a, 7, length(a)), "world!");
    assert_eq_str(slice(a, 0, -1), "Hello, world");
    assert_eq_str(slice(a, -6, -1), "world");
}

TEST(substring_mixed_sizes) {
    string a = u8"Хеllo, уоrлd!";
	assert_eq_str(slice(a, 2, 5), "llo");
	assert_eq_str(slice(a, 7, length(a)), u8"уоrлd!");
	assert_eq_str(slice(a, 0, -1), u8"Хеllo, уоrлd");
	assert_eq_str(slice(a, -6, -1), u8"уоrлd");
}

TEST(index) {
    string a = make_string("Hello");
    defer(free(a));

    assert_eq(a[0], 'H');
    assert_eq(a[1], 'e');
    assert_eq(a[2], 'l');
    assert_eq(a[3], 'l');
    assert_eq(a[4], 'o');

    a[0] = 'X';
    assert_eq(a[0], 'X');
}

TEST(insert) {
    string a = make_string("e");
    defer(free(a));

    insert_at_index(a, 1, 'l');
    insert_at_index(a, 0, 'H');
    assert_eq_str(a, "Hel");

    insert_at_index(a, 3, "lo");
    assert_eq_str(a, "Hello");

    insert_at_index(a, 0, "Hello ");
    assert_eq_str(a, "Hello Hello");

    insert_at_index(a, 5, " world");
    assert_eq_str(a, "Hello world Hello");
}

TEST(remove) {
    string a = make_string("Hello world Hello");
    
    remove_range(a, -6, length(a));
    assert_eq_str(a, "Hello world");
    remove_at_index(a, 1);
    assert_eq_str(a, "Hllo world");
    remove_at_index(a, 1);
    assert_eq_str(a, "Hlo world");
    remove_at_index(a, 0);
    assert_eq_str(a, "lo world");
    remove_at_index(a, -1);
    assert_eq_str(a, "lo worl");
    remove_at_index(a, -2);
    assert_eq_str(a, "lo wol");
    free(a);

    a = make_string("Hello world");

    remove_range(a, 0, 5);
    assert_eq_str(a, " world");
    free(a);
}

TEST(trim) {
    string a = "\t\t    Hello, everyone!   \t\t   \n";
    assert_eq_str(trim_start(a), "Hello, everyone!   \t\t   \n");
    assert_eq_str(trim_end(a), "\t\t    Hello, everyone!");
	assert_eq_str(trim(a), "Hello, everyone!");
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
    string a = make_string("aDc");

    set(a, 1, 'b');
    assert_eq_str(a, "abc");
    set(a, 1, U'Д');
    assert_eq_str(a, u8"aДc");
    set(a, 1, 'b');
    assert_eq_str(a, "abc");
    assert_eq(a[0], 'a');
    assert_eq(a[1], 'b');
    assert_eq(a[2], 'c');
    free(a);

    a = make_string("aDc");

    a[-2] = 'b';
    assert_eq_str(a, "abc");
    a[1] = U'Д';
    assert_eq_str(a, u8"aДc");
    a[1] = 'b';
    assert_eq_str(a, "abc");
    assert_eq(a[0], 'a');
    assert_eq(a[1], 'b');
    assert_eq(a[2], 'c');

    a[-3] = U'\U0002070E';
    a[-2] = U'\U00020731';
    a[-1] = U'\U00020779';
    assert_eq_str(a, u8"\U0002070E\U00020731\U00020779");
    free(a);
}

TEST(iterator) {
    string a = make_string("Hello");

    string result = make_string("");
    for (auto ch : a) {
        add(result, ch);
    }
    assert_eq_str(result, a);

    string b = make_string("HeLLo");

    // In order to modify a character, use a string::code_point_ref
    // This will be same as writing "for (string::code_point_ref ch : b)", since b is non-const.
    // Note that when b is const, the type of ch is just utf32 (you can't take a code point reference)
    for (auto ch : b) {
        ch = to_lower(ch);
    }
    assert_eq_str(b, "hello");
    for (auto ch : b) {
        ch = U'Д';
    }
    assert_eq_str(b, u8"ДДДДД");

    // for (utf32 &ch : b) { .. }
    // doesn't work since string isn't
    // actually an array of utf32
}

TEST(append) {
    {
        string result = make_string("Hello");

        add(result, ",THIS IS GARBAGE", 1);
        result += " world!";

        assert_eq_str(result, "Hello, world!");
        free(result);
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";

        string result;
        reserve(result);
        result += a;
        result += b;
        result += c;

        assert_eq_str(result, "Hello, world!");
        free(result);
    }

    {
        string result;
		reserve(result);

        For(range(10)) {
            add(result, 'i');
            assert_eq(result.Count, it + 1);
            assert_eq(length(result), it + 1);
        }
        free(result);
    }

    {
        string result;
		reserve(result);

        For(range(10)) {
            add(result, u8"Д");
            assert_eq(result.Count, 2 * (it + 1));
            assert_eq(length(result), it + 1);
        }
        free(result);
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
    defer(free(result));
    assert_eq_str(result, "Hello, world!");
}

TEST(remove_all) {
    string a = "Hello world!";

    string b = clone(a);

    remove_all(b, 'l');
    assert_eq_str(b, "Heo word!");
    free(b);

    b = clone(a);

    remove_all(b, "ll");
    assert_eq_str(b, "Heo world!");
    free(b);

    b = clone(a);
    
    reserve(a);
    remove_all(a, "x");
    assert_eq_str(b, a);
    free(b);
    free(a);

    b = make_string("llHello world!ll");

    remove_all(b, 'l');
    assert_eq_str(b, "Heo word!");
    free(b);

    b = make_string("llHello world!ll");

    remove_all(b, "ll");
    assert_eq_str(b, "Heo world!");
    free(b);
}

TEST(replace_all) {
    string a = "Hello world!";

    string b = clone(a);

    replace_all(b, string("l"), string("ll"));
    assert_eq_str(b, "Hellllo worlld!");
    free(b);

    b = clone(a);

    replace_all(b, string("l"), string(""));

    string c = clone(a);

    remove_all(c, 'l');
    assert_eq_str(b, c);
    free(b);
    free(c);

    b = clone(a);

    replace_all(b, string("x"), string(""));
    assert_eq_str(b, a);
    free(b);

    b = clone(a);

    replace_all(b, string("Hello"), string("olleH"));
    assert_eq_str(b, "olleH world!");
    free(b);

    a = "llHello world!ll";
    b = clone(a);

    replace_all(b, string("ll"), string("l"));
    assert_eq_str(b, "lHelo world!l");
    free(b);

    b = clone(a);

    replace_all(b, string("l"), string("ll"));
    assert_eq_str(b, "llllHellllo worlld!llll");
    free(b);

    b = clone(a);

    replace_all(b, string("l"), string("K"));
    assert_eq_str(b, "KKHeKKo worKd!KK");
    free(b);
}

TEST(find) {
    string a = "This is a string";
    assert_eq(2, search(a, string("is")));
    assert_eq(5, search(a, string("is"), search_options{ .Start = 5 }));

    assert_eq(0, search(a, string("This")));
    assert_eq(0, search(a, string("This"), search_options {.Start = -1, .Reversed = true}));
    assert_eq(10, search(a, string("string")));
    assert_eq(10, search(a, string("string"), search_options {.Start = -1, .Reversed = true}));

    assert_eq(5, search(a, string("is"), search_options {.Start = 6, .Reversed = true}));
    assert_eq(5, search(a, string("is"), search_options {.Start = 5, .Reversed = true}));
    assert_eq(2, search(a, string("is"), search_options {.Start = 3, .Reversed = true}));
    assert_eq(2, search(a, string("is"), search_options {.Start = 2, .Reversed = true}));
    assert_eq(-1, search(a, string("is"), search_options {.Start = 1, .Reversed = true}));

    assert_eq(1, search(a, 'h'));
    assert_eq(1, search(a, 'h', search_options{ .Start = 1 }));
    assert_eq(1, search(a, string("h"), search_options{ .Start = 1 }));

    assert_eq(0, search(a, 'T'));
    assert_eq(0, search(a, 'T', search_options {.Start = -1, .Reversed = true}));

    assert_eq(13, search(a, 'i', search_options {.Start = -1, .Reversed = true}));
    assert_eq(13, search(a, 'i', search_options {.Start = 13, .Reversed = true}));
    assert_eq(5, search(a, 'i', search_options {.Start = 12, .Reversed = true}));
    assert_eq(5, search(a, 'i', search_options {.Start = 5, .Reversed = true}));
    assert_eq(2, search(a, 'i', search_options {.Start = 4, .Reversed = true}));

    assert_eq(length(a) - 1, search(a, 'g'));
    assert_eq(length(a) - 1, search(a, 'g', search_options {.Start = -1, .Reversed = true}));

    auto matchNotT = [](code_point cp) { return cp != 'T'; };
    auto matchNotQ = [](code_point cp) { return cp != 'Q'; };
    auto matchNotg = [](code_point cp) { return cp != 'g'; };
    assert_eq(1, search(a, &matchNotT));
    assert_eq(0, search(a, &matchNotQ));
    assert_eq(length(a) - 1, search(a, &matchNotQ, search_options {.Start = -1, .Reversed = true}));
    assert_eq(length(a) - 2, search(a, &matchNotg, search_options {.Start = -1, .Reversed = true}));

    assert_eq(-1, search(a, 'Q'));

    a = u8"Това е низ от букви";
    assert_eq(8, search(a, string(u8"и")));
    assert_eq(8, search(a, string(u8"и"), search_options{ .Start = 8 }));

    assert_eq(8, search(a, U'и'));
    assert_eq(8, search(a, U'и', search_options{ .Start = 8 }));

    assert_eq(14, search(a, U'б'));
    assert_eq(14, search(a, U'б', search_options {.Start = -1, .Reversed = true}));

    assert_eq(-1, search(a, U'я'));

    auto matchAnyOf1 = [](code_point cp) { return has("DCb", cp); };
    auto matchAnyOf2 = [](code_point cp) { return has("CbD", cp); };
    auto matchAnyOf3 = [](code_point cp) { return has("PQa", cp); };

    a = "aaabbbcccddd";
    assert_eq(3, search(a, &matchAnyOf1));
    assert_eq(3, search(a, &matchAnyOf2));
    assert_eq(0, search(a, &matchAnyOf3));

    assert_eq(2, search(a, &matchAnyOf3, search_options {.Start = -1, .Reversed = true}));
    assert_eq(2, search(a, &matchAnyOf3, search_options {.Start = 2, .Reversed = true}));
    assert_eq(1, search(a, &matchAnyOf3, search_options {.Start = 1, .Reversed = true}));
    assert_eq(0, search(a, &matchAnyOf3, search_options {.Start = 0, .Reversed = true}));

    auto matchNoneOf1 = [](code_point cp) { return !has("abc", cp); };
    auto matchNoneOf2 = [](code_point cp) { return !has("bcd", cp); };
    auto matchNoneOf3 = [](code_point cp) { return !has("ac", cp); };
    auto matchNoneOf4 = [](code_point cp) { return !has("bc", cp); };

    assert_eq(search(a, 'd'), search(a, &matchNoneOf1));
    assert_eq(0, search(a, &matchNoneOf2));
    assert_eq(search(a, 'b'), search(a, &matchNoneOf3));

    assert_eq(2, search(a, &matchNoneOf2, search_options {.Start = -1, .Reversed = true}));
    assert_eq(9, search(a, &matchNoneOf4, search_options {.Start = 3}));
    assert_eq(2, search(a, &matchNoneOf4, search_options {.Start = 4, .Reversed = true}));
    assert_eq(1, search(a, &matchNoneOf2, search_options {.Start = 1, .Reversed = true}));
    assert_eq(0, search(a, &matchNoneOf2, search_options {.Start = 0, .Reversed = true}));

    auto matchAnyOf4 = [](code_point cp) { return has("CdB", cp); };
    auto matchAnyOf5 = [](code_point cp) { return has("QRT", cp); };

    assert_eq(length(a) - 1, search(a, &matchAnyOf4, search_options {.Start = -1, .Reversed = true}));

    assert_eq(-1, search(a, &matchAnyOf5));
}
