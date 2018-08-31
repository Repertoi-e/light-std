#pragma once

#include <gu/memory/dynamic_array.h>
#include <gu/memory/table.h>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      .../home/user/dev/sandbox-tests/src/tests/string.cpp ---> tests/string.cpp
//      .../home/user/dev/sandbox-tests/string.cpp           ---> string.cpp
//
constexpr const char *file_name_relative_to_src(const char *str) {
    // Note: / won't work on Windows
    char *src = (char *) "src/";

    const char *result = find_cstring_last(str, src);
    if (!result) {
        result = find_cstring_last(str, "/");
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

inline Table<string, Dynamic_Array<Test> *> g_TestTable;

#define TEST(name)                                                     \
    struct Test_Struct_##name {                                        \
        Test_Struct_##name() {                                         \
            const char *file = file_name_relative_to_src(__FILE__);    \
                                                                       \
            auto [testsArray, fileKeyFound] = find(g_TestTable, file); \
            if (!fileKeyFound) {                                       \
                testsArray = New<Dynamic_Array<Test>>();               \
                put(g_TestTable, file, testsArray);                    \
            }                                                          \
                                                                       \
            add(*testsArray, {#name, &run});                           \
        }                                                              \
        static void run();                                             \
    };                                                                 \
    static Test_Struct_##name g_TestStruct_##name;                     \
    void Test_Struct_##name::run()
