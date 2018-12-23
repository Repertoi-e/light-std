#include "../test.hpp"

#include <cppu/memory/array.hpp>
#include <cppu/memory/dynamic_array.hpp>

#include <cppu/format/fmt.hpp>

#include <array>

TEST(static_array) {
    Array<s32, 5> ints = {0, 1, 2, 3, 4};

    for (s32 i = 0; i < ints.Count; i++) {
        assert_eq(ints[i], i);
    }

    size_t j = 0;
    for (s32 i : ints) {
        assert_eq(i, j++);
    }

    ints.sort(std::greater<s32>());
    j = 4;
    for (s32 i : ints) {
        assert_eq(i, j--);
    }
    ints.sort(std::less<s32>());

    assert_true(ints.has(3));
    assert_true(ints.has(4));
    assert_true(ints.has(0));

    assert_false(ints.has(10));
    assert_false(ints.has(20));

    assert_eq(ints.find_last(3), 3);
    assert_eq(ints.find_last(4), 4);
    assert_eq(ints.find_last(0), 0);
    assert_eq(ints.find(3), 3);
    assert_eq(ints.find(4), 4);
    assert_eq(ints.find(0), 0);
}

TEST(dynamic_array) {
    Dynamic_Array<s32> integers;
    for (s32 i = 0; i < 10; i++) {
        integers.add(i);
    }

    for (s32 i = 0; i < 10; i++) {
        assert_eq(integers[i], i);
    }

    integers.insert(integers.begin() + 3, -3);
    assert_eq(integers, to_array<s32>(0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9));

    integers.remove(integers.begin() + 4);
    assert_eq(integers, to_array<s32>(0, 1, 2, -3, 4, 5, 6, 7, 8, 9));

    size_t count = integers.Count;
    for (size_t i = 0; i < count; i++) {
        integers.pop();
    }
    assert_eq(integers.Count, 0);

    for (s32 i = 0; i < 10; i++) {
        integers.insert_front(i);
    }
    assert_eq(integers, to_array<s32>(9, 8, 7, 6, 5, 4, 3, 2, 1, 0));

    integers.remove(integers.end() - 1);
    assert_eq(integers, to_array<s32>(9, 8, 7, 6, 5, 4, 3, 2, 1));

    integers.remove(integers.begin());
    assert_eq(integers, to_array<s32>(8, 7, 6, 5, 4, 3, 2, 1));

    s64 findResult = integers.find(9);
    assert_eq(findResult, -1);
    findResult = integers.find(8);
    assert_eq(findResult, 0);
    findResult = integers.find(1);
    assert_eq(findResult, 7);
    findResult = integers.find(3);
    assert_eq(findResult, 5);
    findResult = integers.find(5);
    assert_eq(findResult, 3);
}

TEST(table) {
    Table<string, s32> table;
    table.put("1", 1);
    table.put("4", 4);
    table.put("9", 10101);

    assert_eq(std::get<0>(table.find("1")), 1);
    assert_eq(std::get<0>(table.find("4")), 4);
    assert_eq(std::get<0>(table.find("9")), 10101);

    table.put("9", 20202);
    assert_eq(std::get<0>(table.find("9")), 20202);
    table.put("9", 9);

    for (auto [key, value] : table) {
        assert_eq(key, fmt::to_string(value));
    }

    Table<string, s32> emptyTable;
    for (auto [key, value] : emptyTable) {
        // Just to make sure this even compiles
        fmt::print(">> !!! This shouldn't be printed !!! <<\n");
    }
}

TEST(table_copy) {
    Table<string, s32> table;
    table.put("1", 1);
    table.put("4", 4);
    table.put("9", 9);

    Table<string, s32> tableCopy = table;
    tableCopy.put("11", 20);

    for (auto [key, value] : table) {
        assert_eq(key, fmt::to_string(value));
    }

    assert_eq(table.Count, 3);
    assert_eq(tableCopy.Count, 4);
}

TEST(table_pointer_to_value) {
    Table<string, Dynamic_Array<s32> *> table;

    Dynamic_Array<s32> array;
    array.add(0);
    array.add(1);
    array.add(2);

    table.put("1", &array);
    {
        auto [found, wasFound] = table.find("1");
        assert_true(wasFound);
        found->add(3);
        found->add(4);
    }
    {
        auto [found, wasFound] = table.find("1");
        assert_true(wasFound);
        assert_eq(found->Count, 5);
    }
}