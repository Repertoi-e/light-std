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
constexpr const char *get_file_path_relative_to_src_or_just_file_name(const char *str) {
    char srcData[] = {'s', 'r', 'c', OS_PATH_SEPARATOR, '\0'};
    char *src = srcData;

    const char *result = find_cstring_last(str, src);
    if (!result) {
        result = find_cstring_last(str, OS_PATH_SEPARATOR);
        // Skip the slash
        result++;
    } else {
        // Skip the src directory
        while (*src++) {
            result++;
        }
    }

    return result;
}

typedef void (*Test_Func)();

struct Test {
    string Name;
    Test_Func Function;
};

// !!! WORKAORUND
// Visual Studio compiler has a bug (suprise surpise) with inline variables getting initialized multiple times.
// So a workaround for now is to extern it instead.
// inline Table<string, Dynamic_Array<Test> *> g_TestTable;
//
// Definition of this in main.cpp
extern Table<string, Dynamic_Array<Test> *> g_TestTable;

#define TEST(name)                                                                        \
    struct Test_Struct_##name {                                                           \
        Test_Struct_##name() {                                                            \
            const char *file = get_file_path_relative_to_src_or_just_file_name(__FILE__); \
                                                                                          \
            auto [testsArray, fileKeyFound] = find(g_TestTable, file);                    \
            if (!fileKeyFound) {                                                          \
                testsArray = New<Dynamic_Array<Test>>();                                  \
                put(g_TestTable, file, testsArray);                                       \
            }                                                                             \
                                                                                          \
            add(*testsArray, {#name, &run});                                              \
        }                                                                                 \
        static void run();                                                                \
    };                                                                                    \
    static Test_Struct_##name g_TestStruct_##name;                                        \
    void Test_Struct_##name::run()
