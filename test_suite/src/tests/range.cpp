#include "../test.h"

template <typename U, s64 N>
void test_expected(stack_array<U, N> expected, s64 start, s64 stop, s64 step = 1) {
    array<s64> result;
    make_dynamic(&result, 8);

    For(range(start, stop, step)) add(&result, it);
    assert_eq(result, expected);
    free(result.Data);
}

TEST(basic) {
    test_expected(make_stack_array<s32>(0, 1, 2, 3, 4), 0, 5);
    test_expected(make_stack_array<s32>(-3, -2, -1, 0, 1), -3, 2);
}

TEST(variable_steps) {
    array<s64> result;
    make_dynamic(&result, 8);

    For(range(2, -3, 2)) add(&result, it);
    assert_eq(result.Count, 0);
    free(result.Data);

    test_expected(make_stack_array<s32>(-3, -1, 1), -3, 2, 2);
    test_expected(make_stack_array<s32>(10, 13), 10, 15, 3);

    test_expected(make_stack_array<s32>(2, 4, 6, 8), 2, 10, 2);
}

TEST(reverse) {
    test_expected(make_stack_array<s32>(5, 4, 3, 2, 1), 5, 0, -1);
    test_expected(make_stack_array<s32>(2, 1, 0, -1, -2), 2, -3, -1);

    test_expected(make_stack_array<s32>(2, 0, -2), 2, -3, -2);
    test_expected(make_stack_array<s32>(15, 12), 15, 10, -3);

    test_expected(make_stack_array<s32>(10, 8, 6, 4), 10, 2, -2);
}
