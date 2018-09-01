#include "../test.h"

#include <gu/memory/array.h>
#include <gu/memory/dynamic_array.h>

#include <gu/string/print.h>

#include <array>

TEST(static_array) {
    Array<s32, 5> ints = {0, 1, 2, 3, 4};

    for (s32 i = 0; i < ints.Count; i++) {
        assert(ints[i] == i);
    }

    size_t j = 0;
    for (s32 i : ints) {
        assert(i == j++);
    }
}

TEST(dynamic_array) {
    Dynamic_Array<s32> integers;
    for (s32 i = 0; i < 10; i++) {
        add(integers, i);
    }

    for (s32 i = 0; i < 10; i++) {
        assert(integers[i] == i);
    }

    {
        insert(integers, begin(integers) + 3, -3);

        Array<s32, 11> expected = {0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9};
        size_t j = 0;
        for (s32 i : integers) {
            assert(i == expected[j++]);
        }
    }
    {
        remove(integers, begin(integers) + 4);

        Array<s32, 10> expected = {0, 1, 2, -3, 4, 5, 6, 7, 8, 9};

        String_Builder builder;
        for (size_t i = 0; i < expected.Count; i++) {
            assert(integers[i] == expected[i]);
        }
    }
    {
        size_t count = integers.Count;
        for (size_t i = 0; i < count; i++) {
            pop(integers);
        }

        assert(integers.Count == 0);
    }
    {
        for (s32 i = 0; i < 10; i++) {
            add_front(integers, i);
        }
        Array<s32, 10> expected = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
        for (size_t i = 0; i < integers.Count; i++) {
            assert(integers[i] == expected[i]);
        }
    }
    {
        remove(integers, last(integers));
        Array<s32, 9> expected = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        for (size_t i = 0; i < integers.Count; i++) {
            assert(integers[i] == expected[i]);
        }
    }
    {
        remove(integers, first(integers));
        Array<s32, 8> expected = {8, 7, 6, 5, 4, 3, 2, 1};
        for (size_t i = 0; i < integers.Count; i++) {
            assert(integers[i] == expected[i]);
        }
    }
    {
        s32 findResult = find(integers, 9);
        assert(findResult == -1);
        findResult = find(integers, 8);
        assert(findResult == 0);
        findResult = find(integers, 1);
        assert(findResult == 7);
        findResult = find(integers, 3);
        assert(findResult == 5);
        findResult = find(integers, 5);
        assert(findResult == 3);
    }
}

TEST(table) {
    Table<string, int> table;
    put(table, "1", 1);
    put(table, "4", 4);
    put(table, "9", 10101);

    assert(std::get<0>(find(table, "1")) == 1);
    assert(std::get<0>(find(table, "4")) == 4);
    assert(std::get<0>(find(table, "9")) == 10101);

    put(table, "9", 20202);
    assert(std::get<0>(find(table, "9")) == 20202);
    put(table, "9", 9);

    for (auto [key, value] : table) {
        assert(key == to_string(value));
    }

    Table<string, int> emptyTable;
    for (auto [key, value] : emptyTable) {
        // Just to make sure this even compiles
        print(">> !!! This shouldn't be printed !!! <<\n");
    }
}

TEST(table_reference_to_value) {
    Table<string, Dynamic_Array<int>*> table;

    Dynamic_Array<int> array;
    add(array, 0);
    add(array, 1);
    add(array, 2);

    put(table, "1", &array);

    {
        auto [found, wasFound] = find(table, "1");
        assert(wasFound);
        add(*found, 3);
        add(*found, 4);
    }
    {
        auto [found, wasFound] = find(table, "1");
        assert(wasFound);
        assert(found->Count == 5);
    }
}