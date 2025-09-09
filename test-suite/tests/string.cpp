#include "../test.h"

TEST(code_point_size)
{
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

TEST(empty_and_views)
{
  string e;
  assert_eq(length(e), 0);
  assert_eq(e.Count, 0);

  // slice on empty
  assert_eq_str(slice(e, 0, 0), "");
  assert_eq_str(slice(e, 0, -1), "");

  // insert/remove on empty
  insert_at_index(e, 0, "");
  assert_eq_str(e, "");
  remove_range(e, 0, 0);
  assert_eq_str(e, "");

  // trim/search on empty
  assert_eq_str(trim(e), "");
  assert_eq(search(e, 'a'), -1);
  assert_eq(search(e, string("a")), -1);
}

TEST(unicode_boundaries)
{
  // 1-byte, 2-byte, 4-byte code points
  string s = u8"A\u00C4\U0001D11E"; // A, Ä, 𝄞
  reserve(s);
  defer(free(s));
  assert_eq(length(s), 3);

  // set at first/last
  set(s, 0, U'Z');
  set(s, -1, U'X');
  assert_eq(length(s), 3);
  assert_eq(get(s, 0), U'Z');
  assert_eq(get(s, -1), U'X');

  // insert at boundaries
  insert_at_index(s, 0, U'!');
  insert_at_index(s, length(s), U'?');
  assert_eq(get(s, 0), U'!');
  assert_eq(get(s, -1), U'?');

  // remove middle code point
  remove_range(s, 2, 3);
  assert_eq(length(s), 4);
}

TEST(bounds_and_indices)
{
  string s = u8"абв";
  assert_eq(length(s), 3);

  // begin==end
  assert_eq_str(slice(s, 1, 1), "");
  // begin>end returns empty
  assert_eq_str(slice(s, 2, 1), "");

  // negative indexing boundaries
  assert_eq(get(s, -1), U'в');
  assert_eq(get(s, -length(s)), U'а');

  // set using -length
  set(s, -length(s), U'X');
  assert_eq(get(s, 0), U'X');
}

TEST(search_corner_cases)
{
  {
    string s = "banana";
    // start beyond length
    assert_eq(search(s, string("na"), .Start = 10), -1);
    // reversed search from end
    assert_eq(search(s, string("na"), .Start = -1, .Reversed = true), 4);
    // reversed search from 0 should only find at 0 if match
    assert_eq(search(s, string("ba"), .Start = 0, .Reversed = true), 0);
  }
  {
    string s = u8"абаб";
    assert_eq(search(s, string(u8"аб")), 0);
    assert_eq(search(s, string(u8"аб"), .Start = -1, .Reversed = true), 2);
  }
  {
    // overlapping replace/remove
    string s = "aaaa";
    replace_all(s, string("aa"), string("a"));
    assert_eq_str(s, "aa");

    replace_all(s, string("aa"), string("aaa"));
    assert_eq_str(s, "aaa"); 

    remove_all(s, string("aa"));
    assert_eq_str(s, "a");
  }
}

TEST(trim_whitespace_cases)
{
  string only_ws = "\t \r\n\t";
  assert_eq_str(trim(only_ws), "");

  string mixed = "\t\n a \r\n";
  assert_eq_str(trim_start(mixed), "a \r\n");
  assert_eq_str(trim_end(mixed), "\t\n a");
  assert_eq_str(trim(mixed), "a");
}

TEST(builder_edges)
{
  string_builder b;
  defer(free(b));

  // empty builder
  {
    string res = builder_to_string(b);
    defer(free(res));
    assert_eq_str(res, "");
  }

  // large append over multiple buffers
  For(range(3000)) add(b, 'x');
  string res = builder_to_string(b);
  defer(free(res));
  assert_eq(res.Count, 3000);
  assert_eq(length(res), 3000);

  // reuse/reset
  b.Count = 0;
  add(b, "ok");
  string res2 = builder_to_string(b);
  defer(free(res2));
  assert_eq_str(res2, "ok");
}

TEST(substring)
{
  string a = "Hello, world!";
  assert_eq_str(slice(a, 2, 5), "llo");
  assert_eq_str(slice(a, 7, length(a)), "world!");
  assert_eq_str(slice(a, 0, -1), "Hello, world");
  assert_eq_str(slice(a, -6, -1), "world");
}

TEST(substring_mixed_sizes)
{
  string a = u8"Хеllo, уоrлd!";
  assert_eq_str(slice(a, 2, 5), "llo");
  assert_eq_str(slice(a, 7, length(a)), u8"уоrлd!");
  assert_eq_str(slice(a, 0, -1), u8"Хеllo, уоrлd");
  assert_eq_str(slice(a, -6, -1), u8"уоrлd");
}

TEST(index)
{
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

TEST(insert)
{
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

TEST(remove)
{
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

TEST(trim)
{
  string a = "\t\t    Hello, everyone!   \t\t   \n";
  assert_eq_str(trim_start(a), "Hello, everyone!   \t\t   \n");
  assert_eq_str(trim_end(a), "\t\t    Hello, everyone!");
  assert_eq_str(trim(a), "Hello, everyone!");
}

TEST(match_beginning)
{
  string a = "Hello, world!";
  assert_true(match_beginning(a, "Hello"));
  assert_false(match_beginning(a, "Xello"));
  assert_false(match_beginning(a, "Hellol"));
}

TEST(match_end)
{
  string a = "Hello, world!";
  assert_true(match_end(a, "world!"));
  assert_false(match_end(a, "!world!"));
  assert_false(match_end(a, "world!!"));
}

TEST(set)
{
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

TEST(iterator)
{
  string a = make_string("Hello");

  string result = make_string("");
  for (auto ch : a)
  {
    add(result, ch);
  }
  assert_eq_str(result, a);

  string b = make_string("HeLLo");

  // In order to modify a character, use a string::code_point_ref
  // This will be same as writing "for (string::code_point_ref ch : b)", since b
  // is non-const. Note that when b is const, the type of ch is just utf32 (you
  // can't take a code point reference)
  for (auto ch : b)
  {
    ch = unicode_to_lower(ch);
  }
  assert_eq_str(b, "hello");
  for (auto ch : b)
  {
    ch = U'Д';
  }
  assert_eq_str(b, u8"ДДДДД");

  // for (utf32 &ch : b) { .. }
  // doesn't work since string isn't
  // actually an array of utf32
}

TEST(lower_upper) 
{
  string a = "aBcДЕЖ";
  defer(free(a));

  string lower = clone(a);
  defer(free(lower));

  for (auto ch : lower)
  {
    ch = unicode_to_lower(ch);
  }
  assert_eq_str(lower, u8"abcдеж");

  string upper = clone(a);
  defer(free(upper));

  for (auto ch : upper)
  {
    ch = unicode_to_upper(ch);
  }
  // Latin 'c' should uppercase to 'C'; Cyrillic letters remain as provided.
  assert_eq_str(upper, u8"ABCДЕЖ");
}

TEST(add)
{
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
    result += a;
    result += b;
    result += c;

    assert_eq_str(result, "Hello, world!");
    free(result);
  }

  {
    string result;

    For(range(10))
    {
      add(result, 'i');
      assert_eq(result.Count, it + 1);
      assert_eq(length(result), it + 1);
    }
    free(result);
  }

  {
    string result;

    For(range(10))
    {
      add(result, u8"Д");
      assert_eq(result.Count, 2 * (it + 1));
      assert_eq(length(result), it + 1);
    }
    free(result);
  }
}

TEST(builder)
{
  string_builder builder;
  add(builder, "Hello");
  add(builder, ",THIS IS GARBAGE", 1);
  add(builder, string(" world"));
  add(builder, '!');
  defer(free(builder));

  string result = builder_to_string(builder);
  defer(free(result));
  assert_eq_str(result, "Hello, world!");
}

TEST(remove_all)
{
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

TEST(replace_all)
{
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

TEST(find)
{
  string a = "This is a string";
  assert_eq(2, search(a, string("is")));
  assert_eq(5, search(a, string("is"), .Start = 5));

  assert_eq(0, search(a, string("This")));
  assert_eq(0, search(a, string("This"),
                      .Start = -1, .Reversed = true));
  assert_eq(10, search(a, string("string")));
  assert_eq(10, search(a, string("string"),
                       .Start = -1, .Reversed = true));

  assert_eq(
      5, search(a, string("is"), .Start = 6, .Reversed = true));
  assert_eq(
      5, search(a, string("is"), .Start = 5, .Reversed = true));
  assert_eq(
      2, search(a, string("is"), .Start = 3, .Reversed = true));
  assert_eq(
      2, search(a, string("is"), .Start = 2, .Reversed = true));
  assert_eq(-1, search(a, string("is"),
                       .Start = 1, .Reversed = true));

  assert_eq(1, search(a, 'h'));
  assert_eq(1, search(a, 'h', .Start = 1));
  assert_eq(1, search(a, string("h"), .Start = 1));

  assert_eq(0, search(a, 'T'));
  assert_eq(0, search(a, 'T', .Start = -1, .Reversed = true));

  assert_eq(13, search(a, 'i', .Start = -1, .Reversed = true));
  assert_eq(13, search(a, 'i', .Start = 13, .Reversed = true));
  assert_eq(5, search(a, 'i', .Start = 12, .Reversed = true));
  assert_eq(5, search(a, 'i', .Start = 5, .Reversed = true));
  assert_eq(2, search(a, 'i', .Start = 4, .Reversed = true));

  assert_eq(length(a) - 1, search(a, 'g'));
  assert_eq(length(a) - 1,
            search(a, 'g', .Start = -1, .Reversed = true));

  auto matchNotT = [](code_point cp)
  { return cp != 'T'; };
  auto matchNotQ = [](code_point cp)
  { return cp != 'Q'; };
  auto matchNotg = [](code_point cp)
  { return cp != 'g'; };
  assert_eq(1, search(a, &matchNotT));
  assert_eq(0, search(a, &matchNotQ));
  assert_eq(
      length(a) - 1,
      search(a, &matchNotQ, .Start = -1, .Reversed = true));
  assert_eq(
      length(a) - 2,
      search(a, &matchNotg, .Start = -1, .Reversed = true));

  assert_eq(-1, search(a, 'Q'));

  a = u8"Това е низ от букви";
  assert_eq(8, search(a, string(u8"и")));
  assert_eq(8, search(a, string(u8"и"), .Start = 8));

  assert_eq(8, search(a, U'и'));
  assert_eq(8, search(a, U'и', .Start = 8));

  assert_eq(14, search(a, U'б'));
  assert_eq(14, search(a, U'б', .Start = -1, .Reversed = true));

  assert_eq(-1, search(a, U'я'));

  auto matchAnyOf1 = [](code_point cp)
  { return has("DCb", cp); };
  auto matchAnyOf2 = [](code_point cp)
  { return has("CbD", cp); };
  auto matchAnyOf3 = [](code_point cp)
  { return has("PQa", cp); };

  a = "aaabbbcccddd";
  assert_eq(3, search(a, &matchAnyOf1));
  assert_eq(3, search(a, &matchAnyOf2));
  assert_eq(0, search(a, &matchAnyOf3));

  assert_eq(2, search(a, &matchAnyOf3,
                      .Start = -1, .Reversed = true));
  assert_eq(
      2, search(a, &matchAnyOf3, .Start = 2, .Reversed = true));
  assert_eq(
      1, search(a, &matchAnyOf3, .Start = 1, .Reversed = true));
  assert_eq(
      0, search(a, &matchAnyOf3, .Start = 0, .Reversed = true));

  auto matchNoneOf1 = [](code_point cp)
  { return !has("abc", cp); };
  auto matchNoneOf2 = [](code_point cp)
  { return !has("bcd", cp); };
  auto matchNoneOf3 = [](code_point cp)
  { return !has("ac", cp); };
  auto matchNoneOf4 = [](code_point cp)
  { return !has("bc", cp); };

  assert_eq(search(a, 'd'), search(a, &matchNoneOf1));
  assert_eq(0, search(a, &matchNoneOf2));
  assert_eq(search(a, 'b'), search(a, &matchNoneOf3));

  assert_eq(2, search(a, &matchNoneOf2,
                      .Start = -1, .Reversed = true));
  assert_eq(9, search(a, &matchNoneOf4, .Start = 3));
  assert_eq(2, search(a, &matchNoneOf4,
                      .Start = 4, .Reversed = true));
  assert_eq(1, search(a, &matchNoneOf2,
                      .Start = 1, .Reversed = true));
  assert_eq(0, search(a, &matchNoneOf2,
                      .Start = 0, .Reversed = true));

  auto matchAnyOf4 = [](code_point cp)
  { return has("CdB", cp); };
  auto matchAnyOf5 = [](code_point cp)
  { return has("QRT", cp); };

  assert_eq(
      length(a) - 1,
      search(a, &matchAnyOf4, .Start = -1, .Reversed = true));

  assert_eq(-1, search(a, &matchAnyOf5));
}


TEST(utf8_find_invalid_cases)
{
  // Valid ASCII
  {
    string s = "Hello";
    assert_eq(-1, utf8_find_invalid(s.Data, s.Count));
  }

  // Valid multi-byte sequences
  {
    string s = u8"абв"; // 2-byte sequences
    assert_eq(-1, utf8_find_invalid(s.Data, s.Count));

    string s2 = u8"𝄞"; // U+1D11E (4-byte)
    assert_eq(-1, utf8_find_invalid(s2.Data, s2.Count));
  }

  // Invalid: overlong/illegal starts
  {
    const char overlong1[] = "\xC0\x80"; // overlong U+0000
    string s = make_string(overlong1, 2);
    defer(free(s));
    assert_eq(0, utf8_find_invalid(s.Data, s.Count));

    const char overlong2[] = "\xC1\x81"; // illegal 0xC1 start
    s = make_string(overlong2, 2);
    assert_eq(0, utf8_find_invalid(s.Data, s.Count));
    free(s);
  }

  // Invalid: stray continuation byte
  {
    const char stray[] = "\x80";
    string s = make_string(stray, 1);
    defer(free(s));
    assert_eq(0, utf8_find_invalid(s.Data, s.Count));
  }

  // Invalid: surrogate encoded in UTF-8 (disallowed)
  {
    const char surrogate[] = "\xED\xA0\x80"; // U+D800
    string s = make_string(surrogate, 3);
    defer(free(s));
    assert_eq(0, utf8_find_invalid(s.Data, s.Count));
  }

  // Invalid: truncated sequence
  {
    const char trunc2[] = "\xC2"; // missing continuation
    string s = make_string(trunc2, 1);
    defer(free(s));
    assert_eq(0, utf8_find_invalid(s.Data, s.Count));

    const char trunc3[] = "\xE2\x82"; // incomplete 3-byte
    string s2 = make_string(trunc3, 2);
    defer(free(s2));
    assert_eq(0, utf8_find_invalid(s2.Data, s2.Count));
  }
}

TEST(unicode_normalize_nfc)
{
  // NFC should compose A + COMBINING ACUTE to precomposed Á
  {
    string s = u8"A\u0301";
    string n = make_string_normalized_nfc(s);
    defer(free(n));
    assert_eq_str(n, u8"\u00C1");
  }

  // Idempotence: already NFC remains unchanged
  {
    string s = u8"\u00C1";
    string n = make_string_normalized_nfc(s);
    defer(free(n));
    assert_eq_str(n, s);
  }

  // Ensure canonical reordering by CCC (dot below before acute)
  {
    string s = u8"a\u0301\u0323"; // a + acute (230) + dot-below (220)
    string n = make_string_normalized_nfc(s);
    defer(free(n));

    // Verify CCC sequence after first code point is non-decreasing
    // (NFC guarantees canonical order)
    u8 last = 0; bool first = true; bool ok = true;
    for (auto cp : n) {
      u8 cc = unicode_combining_class(cp);
      if (first) { first = false; last = 0; continue; }
      if (cc < last) { ok = false; break; }
      last = cc;
    }
    assert_true(ok);
  }

  // Composition exclusions/compatibility: U+2126 (OHM SIGN) stays as-is in NFC
  {
    string s = u8"\u2126";
    string n = make_string_normalized_nfc(s);
    defer(free(n));
    assert_eq_str(n, s);
  }

  // Normalization does not increase byte length for NFC
  {
    string s = u8"A\u0301"; // 3 bytes -> 2 bytes
    string n = make_string_normalized_nfc(s);
    defer(free(n));
    assert_true(n.Count <= s.Count);
  }

  // Invalid input returns null string {}
  {
    const char bad[] = "\xC0\x80";
    string s = make_string(bad, 2);
    defer(free(s));
    string n = make_string_normalized_nfc(s);
    defer(free(n));
    assert_eq((void *) n.Data, (void *) 0);
    assert_eq(n.Count, 0);
  }
}

TEST(unicode_casing_full)
{
    // 1. Default locale (Context.Locale)
    {
        // ASCII
        assert_eq(unicode_to_lower('A'), 'a');
        assert_eq(unicode_to_upper('a'), 'A');
        assert_eq(unicode_to_lower('Z'), 'z');
        assert_eq(unicode_to_upper('z'), 'Z');

        // Non-ASCII BMP letters
        assert_eq(unicode_to_lower(0x0130), 0x0069); // İ -> i (non-Turkic uses default simple folding)
        assert_eq(unicode_to_upper(0x00E5), 0x00C5); // å -> Å

        // Characters already lowercase/uppercase
        assert_eq(unicode_to_lower('a'), 'a');
        assert_eq(unicode_to_upper('A'), 'A');
    }

    // 2. Turkic locale
    {
        auto newCtx = Context;
        newCtx.Locale = text_locale::Turkic;
        PUSH_CONTEXT(newCtx)
        {
            // Special Turkish casing
            assert_eq(unicode_to_lower('I'), (code_point)0x0131); // I -> ı
            assert_eq(unicode_to_upper('i'), (code_point)0x0130); // i -> İ

            // Other letters unchanged
            assert_eq(unicode_to_lower('A'), 'a');
            assert_eq(unicode_to_upper('Z'), 'Z');
        }
    }

#ifdef LSTD_UNICODE_FULL_RANGE
    // 3. Supplementary-plane letters (outside BMP)
    {
        code_point smp_upper = 0x10400; // DESERET CAPITAL LETTER LONG I
        code_point smp_lower = 0x10428; // DESERET SMALL LETTER LONG I
        assert_eq(unicode_to_lower(smp_upper), smp_lower);
        assert_eq(unicode_to_upper(smp_lower), smp_upper);
    }
#endif

    // 4. Non-letter / digits / symbols remain unchanged
    {
        assert_eq(unicode_to_lower('1'), '1');
        assert_eq(unicode_to_upper('#'), '#');
        assert_eq(unicode_to_lower(0x2603), 0x2603); // SNOWMAN
    }

    // 5. Round-trip check
    {
        for (code_point cp : make_stack_array('A', 'a', 'Z', 'z', 0x0130, 0x0131, 0x10400, 0x10428)) {
            auto upper = unicode_to_upper(cp);
            auto lower = unicode_to_lower(upper);
            if (cp != 0x0130 && cp != 0x0131) // Turkish exceptions
                assert_eq(lower, unicode_to_lower(cp));
        }
    }
}
