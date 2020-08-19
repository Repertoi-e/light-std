#include "../test.h"

TEST(stack_array) {
    auto stackArray = to_stack_array(0, 1, 2, 3, 4);
    array<s32> a = stackArray;

    For(range(a.Count)) { assert_eq(a[it], it); }

    assert_true(has(a, 3));
    assert_true(has(a, 4));
    assert_true(has(a, 0));

    assert_false(has(a, 10));
    assert_false(has(a, 20));

    assert_eq(find(a, 3, -1, true), 3);
    assert_eq(find(a, 4, -1, true), 4);
    assert_eq(find(a, 0, -1, true), 0);
    assert_eq(find(a, 3), 3);
    assert_eq(find(a, 4), 4);
    assert_eq(find(a, 0), 0);
}

TEST(array) {
    array<s64> a;
    defer(free(a));

    For(range(10)) { append(a, it); }
    For(range(10)) { assert_eq(a[it], it); }

    insert(a, 3, -3);
    assert_eq(a, to_stack_array<s64>(0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9));

    remove(a, 4);
    assert_eq(a, to_stack_array<s64>(0, 1, 2, -3, 4, 5, 6, 7, 8, 9));

    s64 count = a.Count;
    For(range(count)) { remove(a, -1); }
    assert_eq(a.Count, 0);

    For(range(10)) { insert(a, 0, it); }
    assert_eq(a, to_stack_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

    remove(a, -1);
    assert_eq(a, to_stack_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1));

    remove(a, 0);
    assert_eq(a, to_stack_array<s64>(8, 7, 6, 5, 4, 3, 2, 1));

    s64 f = find(a, 9);
    assert_eq(f, -1);
    f = find(a, 8);
    assert_eq(f, 0);
    f = find(a, 1);
    assert_eq(f, 7);
    f = find(a, 3);
    assert_eq(f, 5);
    f = find(a, 5);
    assert_eq(f, 3);
}

TEST(hash_table) {
    hash_table<string, s32> t;
    defer(free(t));

    set(t, "1", 1);
    set(t, "4", 4);
    set(t, "9", 10101);

    assert((void *) find(t, "1").second);
    assert_eq(*find(t, "1").second, 1);
    assert((void *) find(t, "4").second);
    assert_eq(*find(t, "4").second, 4);
    assert((void *) find(t, "9").second);
    assert_eq(*find(t, "9").second, 10101);

    set(t, "9", 20202);
    assert((void *) find(t, "9").second);
    assert_eq(*find(t, "9").second, 20202);
    set(t, "9", 9);

    s64 loopIterations = 0;
    for (auto [key, value] : t) {
        string str = fmt::sprint("{}", *value);
        assert_eq(*key, str);
        str.release();

        ++loopIterations;
    }
    assert_eq(loopIterations, t.Count);

    hash_table<string, s32> empty;
    for (auto [key, value] : empty) {
        assert(false);
    }
}

TEST(hash_table_clone) {
    hash_table<string, s32> t;
    defer(free(t));

    set(t, "1", 1);
    set(t, "4", 4);
    set(t, "9", 9);

    hash_table<string, s32> copy;
    clone(&copy, t);
    defer(free(copy));

    set(copy, "11", 20);

    s64 loopIterations = 0;
    for (auto [key, value] : t) {
        string str = fmt::sprint("{}", *value);
        assert_eq(*key, str);
        str.release();

        ++loopIterations;
    }
    assert_eq(loopIterations, t.Count);

    assert_eq(t.Count, 3);
    assert_eq(copy.Count, 4);
}

TEST(hash_table_alignment) {
    // This test uses SIMD types which require a 16 byte alignment or otherwise crash.
    // It tests if the block allocation in the table handles alignment of key and value arrays.

    hash_table<v2, v3> simdTable;
    reserve(simdTable, 0, 16);

    add(simdTable, {1, 2}, {1, 2, 3});
    add(simdTable, {1, 3}, {4, 7, 9});
}