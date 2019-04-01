#include "../test.hpp"

#include <lstd/containers.hpp>
#include <lstd/io.hpp>

TEST(array) {
    array<s32, 5> a = {0, 1, 2, 3, 4};

    For(range(a.Count)) { assert_eq(a[it], it); }

#if !defined LSTD_NO_CRT
    size_t j = 0;
    For(a) { assert_eq(it, j++); }

    a.sort(std::greater<s32>());
    j = 4;
    For(a) { assert_eq(it, j--); }
    a.sort(std::less<s32>());
#endif

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

TEST(dynamic_array) {
    dynamic_array<s32> a;

    For(range(10)) { a.append(it); }
    For(range(10)) { assert_eq(a[it], it); }

    a.insert(a.begin() + 3, -3);
    assert_eq(a, to_array<s32>(0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9));

    a.remove(a.begin() + 4);
    assert_eq(a, to_array<s32>(0, 1, 2, -3, 4, 5, 6, 7, 8, 9));

    size_t count = a.Count;
    For(range(count)) { a.pop(); }
    assert_eq(a.Count, 0);

    For(range(10)) { a.insert_front(it); }
    assert_eq(a, to_array<s32>(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

    a.remove(a.end() - 1);
    assert_eq(a, to_array<s32>(9, 8, 7, 6, 5, 4, 3, 2, 1));

    a.remove(a.begin());
    assert_eq(a, to_array<s32>(8, 7, 6, 5, 4, 3, 2, 1));

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
    t.put("1", 1);
    t.put("4", 4);
    t.put("9", 10101);

    assert_eq(std::get<0>(t.find("1")), 1);
    assert_eq(std::get<0>(t.find("4")), 4);
    assert_eq(std::get<0>(t.find("9")), 10101);

    t.put("9", 20202);
    assert_eq(std::get<0>(t.find("9")), 20202);
    t.put("9", 9);

    for (auto [key, value] : t) {
        assert_eq(key, fmt::to_string(value));
    }

    table<string, s32> empty;
    for (auto [key, value] : empty) {
        assert(false);
    }
}

TEST(table_copy) {
    table<string, s32> t;
    t.put("1", 1);
    t.put("4", 4);
    t.put("9", 9);

    table<string, s32> copy = t;
    copy.put("11", 20);

    for (auto [key, value] : t) {
        assert_eq(key, fmt::to_string(value));
    }

    assert_eq(t.Count, 3);
    assert_eq(copy.Count, 4);
}

TEST(table_pointer_to_value) {
    table<string, dynamic_array<s32> *> t;

    dynamic_array<s32> a;
    a.append(0);
    a.append(1);
    a.append(2);

    t.put("1", &a);
    {
        auto [found, wasFound] = t.find("1");
        assert_true(wasFound);
        found->append(3);
        found->append(4);
    }
    {
        auto [found, wasFound] = t.find("1");
        assert_true(wasFound);
        assert_eq(found->Count, 5);
    }
}