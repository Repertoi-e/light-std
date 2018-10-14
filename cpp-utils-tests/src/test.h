#pragma once

#include <gu/memory/dynamic_array.h>
#include <gu/memory/table.h>

#include <gu/file/file_path.h>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      .../home/user/dev/sandbox-tests/src/tests/string.cpp ---> tests/string.cpp
//      .../home/user/dev/sandbox-tests/string.cpp           ---> string.cpp
//
constexpr string_view get_file_path_relative_to_src_or_just_file_name(string_view str) {
    char srcData[] = {'s', 'r', 'c', OS_PATH_SEPARATOR, '\0'};
    string_view src = srcData;

    size_t findResult = str.find_last(src);
    if (findResult == string_view::NPOS) {
        findResult = str.find_last(OS_PATH_SEPARATOR);
        assert(findResult != str.Length - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        for (auto ch : src) {
            findResult++;
        }
    }
    str.remove_prefix(findResult);
    return str;
}

typedef void (*Test_Func)();

struct Test {
    string_view Name;
    Test_Func Function;
};

// !!! WORKAORUND
// Visual Studio compiler has a bug (suprise surpise) with inline variables getting initialized multiple times.
// So a workaround for now is to extern it instead.
// inline Table<string, Dynamic_Array<Test> *> g_TestTable;
//
// Definition of this in main.cpp
extern Table<string_view, Dynamic_Array<Test> *> g_TestTable;

#define TEST(name)                                                                        \
    struct Test_Struct_##name {                                                           \
        Test_Struct_##name() {                                                            \
            string_view file = get_file_path_relative_to_src_or_just_file_name(__FILE__); \
                                                                                          \
            auto [testsArray, fileKeyFound] = g_TestTable.find(file);                     \
            if (!fileKeyFound) {                                                          \
                testsArray = New<Dynamic_Array<Test>>();                                  \
                g_TestTable.put(file, testsArray);                                        \
            }                                                                             \
                                                                                          \
            testsArray->add({#name, &run});                                               \
        }                                                                                 \
        static void run();                                                                \
    };                                                                                    \
    static Test_Struct_##name g_TestStruct_##name;                                        \
    void Test_Struct_##name::run()
