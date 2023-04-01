#include "../test.h"

TEST(stack_array) {
    auto stackArray = make_stack_array(0, 1, 2, 3, 4);
    array<s32> a    = stackArray;

    For(range(a.Count)) { assert_eq(a[it], it); }

    assert_true(has(a, 3));
    assert_true(has(a, 4));
    assert_true(has(a, 0));

    assert_false(has(a, 10));
    assert_false(has(a, 20));

    assert_eq(search(a, 3, search_options {.Start = -1, .Reversed = true}), 3);
    assert_eq(search(a, 4, search_options{ .Start = -1, .Reversed = true }), 4);
    assert_eq(search(a, 0, search_options{ .Start = -1, .Reversed = true }), 0);
    assert_eq(search(a, 3), 3);
    assert_eq(search(a, 4), 4);
    assert_eq(search(a, 0), 0);
}

TEST(array) {
    array<s64> a;
    reserve(a);
    defer(free(a.Data));

    For(range(10)) { a += {it}; }
    For(range(10)) { assert_eq(a[it], it); }

    insert_at_index(a, 3, -3);
    assert_eq(a, make_stack_array<s64>(0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9));

    remove_ordered_at_index(a, 4);
    assert_eq(a, make_stack_array<s64>(0, 1, 2, -3, 4, 5, 6, 7, 8, 9));

    s64 count = a.Count;
    For(range(count)) { remove_ordered_at_index(a, -1); }
    assert_eq(a.Count, 0);

    For(range(10)) { insert_at_index(a, 0, it); }
    assert_eq(a, make_stack_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

    remove_ordered_at_index(a, -1);
    assert_eq(a, make_stack_array<s64>(9, 8, 7, 6, 5, 4, 3, 2, 1));

    remove_ordered_at_index(a, 0);
    assert_eq(a, make_stack_array<s64>(8, 7, 6, 5, 4, 3, 2, 1));

    s64 f = search(a, 9);
    assert_eq(f, -1);
    f = search(a, 8);
    assert_eq(f, 0);
    f = search(a, 1);
    assert_eq(f, 7);
    f = search(a, 3);
    assert_eq(f, 5);
    f = search(a, 5);
    assert_eq(f, 3);
}

TEST(hash_table) {
    hash_table<string, s32> t;
    defer(free(t));

    set(t, "1", 1);
    set(t, "4", 4);
    set(t, "9", 10101);

    assert((void *) search(t, "1").Value);
    assert_eq(*search(t, "1").Value, 1);
    assert((void *) search(t, "4").Value);
    assert_eq(*search(t, "4").Value, 4);
    assert((void *) search(t, "9").Value);
    assert_eq(*search(t, "9").Value, 10101);

    set(t, "9", 20202);
    assert((void *) search(t, "9").Value);
    assert_eq(*search(t, "9").Value, 20202);
    set(t, "9", 9);

    s64 loopIterations = 0;
    for (auto [key, value] : t) {
        string str = sprint("{}", *value);
        assert_eq_str(*key, str);
        free(str);

        ++loopIterations;
    }
    assert_eq(loopIterations, t.Count);

    hash_table<string, s32> empty;
    for (auto [key, value] : empty) {
        (void) key, value;  // Unused variable
        assert(false);
    }
}

TEST(hash_table_clone) {
    hash_table<string, s32> t;
    defer(free(t));

    set(t, "1", 1);
    set(t, "4", 4);
    set(t, "9", 9);

    hash_table<string, s32> copy = clone(t);
    defer(free(copy));

    set(copy, "11", 20);

    s64 loopIterations = 0;
    for (auto [key, value] : t) {
        string str = sprint("{}", *value);
        assert_eq_str(*key, str);
        free(str.Data);

        ++loopIterations;
    }
    assert_eq(loopIterations, t.Count);

    assert_eq(t.Count, 3);
    assert_eq(copy.Count, 4);
}

struct v2 {
    f32 x, y;
};

struct v3 {
    f32 x, y, z;
};

u64 get_hash(v2) {
    return 10; // Ha-hA
}

TEST(hash_table_alignment) {
    // This test uses SIMD types which require a 16 byte alignment or otherwise crash.
    // It tests if the block allocation in the table handles alignment of key and value arrays.

    hash_table<v2, v3> simdTable;
    resize(simdTable, 0, 16);

    add(simdTable, {1, 2}, {1, 2, 3});
    add(simdTable, {1, 3}, {4, 7, 9});
}
