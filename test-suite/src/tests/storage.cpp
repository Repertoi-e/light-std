#include "../test.h"

TEST(stack_array) {
    stack_array<s32, 5> a = {0, 1, 2, 3, 4};

    For(range(a.Count)) { assert_eq(a[it], it); }

    assert_true(a.has(3));
    assert_true(a.has(4));
    assert_true(a.has(0));

    assert_false(a.has(10));
    assert_false(a.has(20));

    assert_eq(a.find_reverse(3), 3);
    assert_eq(a.find_reverse(4), 4);
    assert_eq(a.find_reverse(0), 0);
    assert_eq(a.find(3), 3);
    assert_eq(a.find(4), 4);
    assert_eq(a.find(0), 0);
}

TEST(array) {
    array<s64> a;

    For(range(10)) { a.append(it); }
    For(range(10)) { assert_eq(a[it], it); }

    a.insert(3, -3);
    assert_eq(a, to_array<s64>(0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9));

    a.remove(4);
    assert_eq(a, to_array<s64>(0, 1, 2, -3, 4, 5, 6, 7, 8, 9));

    size_t count = a.Count;
    For(range(count)) { a.remove(-1); }
    assert_eq(a.Count, 0);

    For(range(10)) { a.insert(0, it); }
    assert_eq(a, to_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

    a.remove(-1);
    assert_eq(a, to_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1));

    a.remove(0);
    assert_eq(a, to_array<s64>(8, 7, 6, 5, 4, 3, 2, 1));

    s64 f = a.find(9);
    assert_eq(f, -1);
    f = a.find(8);
    assert_eq(f, 0);
    f = a.find(1);
    assert_eq(f, 7);
    f = a.find(3);
    assert_eq(f, 5);
    f = a.find(5);
    assert_eq(f, 3);
}

TEST(table) {
    table<string, s32> t;
    t.set("1", 1);
    t.set("4", 4);
    t.set("9", 10101);

    assert((void *) t.find("1"));
    assert_eq(*t.find("1"), 1);
    assert((void *) t.find("4"));
    assert_eq(*t.find("4"), 4);
    assert((void *) t.find("9"));
    assert_eq(*t.find("9"), 10101);

    t.set("9", 20202);
    assert((void *) t.find("9"));
    assert_eq(*t.find("9"), 20202);
    t.set("9", 9);

	size_t loopIterations = 0;
    for (auto [key, value] : t) {
        string str;
        fmt::sprint(&str, "{}", *value);
        assert_eq(*key, str);
        ++loopIterations;
    }
    assert_eq(loopIterations, t.Count)

    table<string, s32> empty;
    for (auto [key, value] : empty) {
        assert(false);
    }
}

TEST(table_clone) {
    table<string, s32> t;
    t.set("1", 1);
    t.set("4", 4);
    t.set("9", 9);

    table<string, s32> copy;
    clone(&copy, t);
    copy.set("11", 20);

	size_t loopIterations = 0;
    for (auto [key, value] : t) {
        string str;
        fmt::sprint(&str, "{}", *value);
        assert_eq(*key, str);
        ++loopIterations;
    }
    assert_eq(loopIterations, t.Count)

    assert_eq(t.Count, 3);
    assert_eq(copy.Count, 4);
}
