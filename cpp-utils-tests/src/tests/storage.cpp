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
		integers.add(i);
    }

    for (s32 i = 0; i < 10; i++) {
        assert(integers[i] == i);
    }

	integers.insert(integers.begin() + 3, -3);
    assert(integers == (Array<s32, 11>{0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9}));

	integers.remove(integers.begin() + 4);
    assert(integers == (Array<s32, 10>{0, 1, 2, -3, 4, 5, 6, 7, 8, 9}));

    size_t count = integers.Count;
    for (size_t i = 0; i < count; i++) {
		integers.pop();
    }
    assert(integers.Count == 0);

    for (s32 i = 0; i < 10; i++) {
		integers.add_front(i);
    }
    assert(integers == (Array<s32, 10>{9, 8, 7, 6, 5, 4, 3, 2, 1, 0}));

	integers.remove(integers.end() - 1);
    assert(integers == (Array<s32, 9>{9, 8, 7, 6, 5, 4, 3, 2, 1}));

	integers.remove(integers.begin());
    assert(integers == (Array<s32, 8>{8, 7, 6, 5, 4, 3, 2, 1}));

    s64 findResult = integers.find(9);
    assert(findResult == -1);
    findResult = integers.find(8);
    assert(findResult == 0);
    findResult = integers.find(1);
    assert(findResult == 7);
    findResult = integers.find(3);
    assert(findResult == 5);
    findResult = integers.find(5);
    assert(findResult == 3);
}

TEST(table) {
    Table<string, int> table;
    table.put("1", 1);
    table.put("4", 4);
    table.put("9", 10101);

    assert(std::get<0>(table.find("1")) == 1);
    assert(std::get<0>(table.find("4")) == 4);
    assert(std::get<0>(table.find("9")) == 10101);

    table.put("9", 20202);
    assert(std::get<0>(table.find("9")) == 20202);
    table.put("9", 9);

    for (auto [key, value] : table) {
		if (key != to_string(value)) {
			int a = 2;
		}
        //assert();
    }

    Table<string, int> emptyTable;
    for (auto [key, value] : emptyTable) {
        // Just to make sure this even compiles
        print(">> !!! This shouldn't be printed !!! <<\n");
    }
}

TEST(table_copy) {
    Table<string, int> table;
    table.put("1", 1);
    table.put("4", 4);
    table.put("9", 9);

    Table<string, int> tableCopy = table;
	tableCopy.put("11", 20);

    for (auto [key, value] : table) {
        assert(key == to_string(value));
    }

    assert(table.Count == 3);
    assert(tableCopy.Count == 4);
}

TEST(table_pointer_to_value) {
    Table<string, Dynamic_Array<int> *> table;

	// FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME 
    //Dynamic_Array<int> array;
    //array.add(0);
    //array.add(1);
    //array.add(2);
	//
    //table.put("1", &array);
	//
    //{
    //    auto [found, wasFound] = table.find("1");
    //    assert(wasFound);
	//	found->add(3);
	//	found->add(4);
    //}
    //{
    //    auto [found, wasFound] = table.find("1");
    //    assert(wasFound);
    //    assert(found->Count == 5);
    //}
}