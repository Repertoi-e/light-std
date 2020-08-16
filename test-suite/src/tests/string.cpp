#include <lstd/memory/string_builder.h>

#include "../test.h"

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

    string mixed;
    mixed.append_string(ascii)->append_string(cyrillic)->append_string(devanagari)->append_string(supplementary);

    assert_eq(mixed.ByteLength, 12 + 9 + 6 + 3);
    assert_eq(mixed.Length, 3 + 3 + 3 + 3);
}

TEST(substring) {
    string a = "Hello, world!";
    assert_eq(a.substring(2, 5), "llo");
    assert_eq(a.substring(7, a.Length), "world!");
    assert_eq(a.substring(0, -1), "Hello, world");
    assert_eq(a.substring(-6, -1), "world");
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

TEST(insert) {
    string a = "e";
    a.insert(1, 'l');
    a.insert(0, 'H');
    assert_eq(a, "Hel");

    a.insert_string(3, "lo");
    assert_eq(a, "Hello");

    a.insert_string(0, "Hello ");
    assert_eq(a, "Hello Hello");

    a.insert_string(5, " world");
    assert_eq(a, "Hello world Hello");
    a.release();
}

TEST(remove) {
    string a = "Hello world Hello";
    a.remove_range(-6, a.Length);
    assert_eq(a, "Hello world");
    a.remove(1);
    assert_eq(a, "Hllo world");
    a.remove(1);
    assert_eq(a, "Hlo world");
    a.remove(0);
    assert_eq(a, "lo world");
    a.remove(-1);
    assert_eq(a, "lo worl");
    a.remove(-2);
    assert_eq(a, "lo wol");
    a.release();

    a = "Hello world";
    a.remove_range(0, 5);
    assert_eq(a, " world");
    a.release();
}

TEST(trim) {
    string a = "\t\t    Hello, everyone!   \t\t   \n";
    assert_eq(a.trim_start(), "Hello, everyone!   \t\t   \n");
    assert_eq(a.trim_end(), "\t\t    Hello, everyone!");
    assert_eq(a.trim(), "Hello, everyone!");
}

TEST(begins_with) {
    string a = "Hello, world!";
    assert_true(a.begins_with("Hello"));
    assert_false(a.begins_with("Xello"));
    assert_false(a.begins_with("Hellol"));
}

TEST(ends_with) {
    string a = "Hello, world!";
    assert_true(a.ends_with("world!"));
    assert_false(a.ends_with("!world!"));
    assert_false(a.ends_with("world!!"));
}

TEST(set) {
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
    a.release();

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
    a.release();
}

TEST(iterator) {
    string a = "Hello";

    string result = "";
    for (auto ch : a) {
        result.append(ch);
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

TEST(append) {
    {
        string result = "Hello";
        result.append_pointer_and_size(",THIS IS GARBAGE", 1);
        result.append_string(" world!");

        assert_eq(result, "Hello, world!");
        result.release();
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";
        string result;
        result.append_string(a)->append_string(b)->append_string(c);

        assert_eq(result, "Hello, world!");
        result.release();
    }

    string result;
    For(range(10)) {
        result.append('i');
        assert_eq(result.ByteLength, it + 1);
        assert_eq(result.Length, it + 1);
    }
    result.release();
    For(range(10)) {
        result.append_string(u8"Д");
        assert_eq(result.ByteLength, 2 * (it + 1));
        assert_eq(result.Length, it + 1);
    }
    result.release();
}

TEST(count) {
    string a = "Hello";
    assert_eq(a.count('l'), 2);
    assert_eq(a.count('e'), 1);
    assert_eq(a.count('o'), 1);
}

TEST(builder) {
    string_builder builder;
    builder.append("Hello");
    builder.append_pointer_and_size(",THIS IS GARBAGE", 1);
    builder.append(string(" world"));
    builder.append('!');
    defer(builder.release());

    string result;
    result = builder.combine();
    defer(result.release());
    assert_eq(result, "Hello, world!");
}

TEST(remove_all) {
    string a = "Hello world!";
    string b = a;

    b.remove_all('l');
    assert_eq(b, "Heo word!");
    b.release();

    b = a;
    b.remove_all("ll");
    assert_eq(b, "Heo world!");
    b.release();

    b = a;
    a.remove_all("x");
    assert_eq(b, a);
    b.release();

    b = "llHello world!ll";
    b.remove_all('l');
    assert_eq(b, "Heo word!");
    b.release();

    b = "llHello world!ll";
    b.remove_all("ll");
    assert_eq(b, "Heo world!");
    b.release();
}

TEST(replace_all) {
    string a = "Hello world!";
    string b = a;

    b.replace_all("l", "ll");
    assert_eq(b, "Hellllo worlld!");
    b.release();

    b = a;
    b.replace_all("l", "");

    string c = a;
    c.remove_all('l');
    assert_eq(b, c);
    b.release();
    c.release();

    b = a;
    b.replace_all("x", "");
    assert_eq(b, a);
    b.release();

    b = a;
    b.replace_all("Hello", "olleH");
    assert_eq(b, "olleH world!");
    b.release();

    b = a = "llHello world!ll";
    b.replace_all("ll", "l");
    assert_eq(b, "lHelo world!l");
    b.release();

    b = a;
    b.replace_all("l", "ll");
    assert_eq(b, "llllHellllo worlld!llll");
    b.release();

    b = a;
    b.replace_all("l", "K");
    assert_eq(b, "KKHeKKo worKd!KK");
    b.release();
}

TEST(find) {
    string a = "This is a string";
    assert_eq(2, a.find("is"));
    assert_eq(5, a.find("is", 5));

    assert_eq(0, a.find("This"));
    assert_eq(0, a.find_reverse("This"));
    assert_eq(10, a.find("string"));
    assert_eq(10, a.find_reverse("string"));

    assert_eq(5, a.find_reverse("is", 6));
    assert_eq(2, a.find_reverse("is", 5));
    assert_eq(2, a.find_reverse("is", 3));

    assert_eq(1, a.find('h'));
    assert_eq(1, a.find('h', 1));
    assert_eq(1, a.find("h", 1));

    assert_eq(0, a.find('T'));
    assert_eq(0, a.find_reverse('T'));

    assert_eq(13, a.find_reverse('i'));
    assert_eq(5, a.find_reverse('i', 13));
    assert_eq(2, a.find_reverse('i', 5));

    assert_eq(a.Length - 1, a.find('g'));
    assert_eq(a.Length - 1, a.find_reverse('g'));

    assert_eq(1, a.find_not('T'));
    assert_eq(0, a.find_not('Q'));
    assert_eq(a.Length - 1, a.find_reverse_not('Q'));
    assert_eq(a.Length - 2, a.find_reverse_not('g'));

    assert_eq(-1, a.find('Q'));

    a = u8"Това е низ от букви";
    assert_eq(8, a.find(u8"и"));
    assert_eq(8, a.find(u8"и", 8));

    assert_eq(8, a.find(U'и'));
    assert_eq(8, a.find(U'и', 8));

    assert_eq(14, a.find(U'б'));
    assert_eq(14, a.find_reverse(U'б'));

    assert_eq(-1, a.find(U'я'));

    a = "aaabbbcccddd";
    assert_eq(3, a.find_any_of("DCb"));
    assert_eq(3, a.find_any_of("CbD"));
    assert_eq(0, a.find_any_of("PQa"));

    assert_eq(2, a.find_reverse_any_of("PQa"));
    assert_eq(1, a.find_reverse_any_of("PQa", 2));
    assert_eq(0, a.find_reverse_any_of("PQa", 1));

    assert_eq(a.find('d'), a.find_not_any_of("abc"));
    assert_eq(0, a.find_not_any_of("bcd"));
    assert_eq(a.find('b'), a.find_not_any_of("ac"));

    assert_eq(2, a.find_reverse_not_any_of("bcd"));
    assert_eq(2, a.find_reverse_not_any_of("bc", -3));
    assert_eq(2, a.find_reverse_not_any_of("bc", -4));
    assert_eq(0, a.find_reverse_not_any_of("bcd", 1));

    assert_eq(a.Length - 1, a.find_reverse_any_of("CdB"));

    assert_eq(-1, a.find_any_of("QRT"));
}
