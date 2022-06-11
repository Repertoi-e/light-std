#pragma once

import "lstd.h";
import lstd.fmt;
import lstd.path;

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      /home/.../lstd-tests/src/tests/string.cpp ---> tests/string.cpp
//      /home/.../lstd-tests/string.cpp           ---> string.cpp
//
constexpr string get_short_file_path(string str) {
    char srcData[] = {'s', 'r', 'c', OS_PATH_SEPARATOR, '\0'};
    string src     = srcData;

    s64 findResult = string_find(str, src, -1, true);
    if (findResult == -1) {
        findResult = string_find(str, OS_PATH_SEPARATOR, -1, true);
        assert(findResult != string_length(str) - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        findResult += string_length(src);
    }

    string result = str;
    return substring(result, findResult, string_length(result));
}

struct asserts {
    inline static s64 GlobalCalledCount;
    inline static array<string> GlobalFailed;
};

//
// Define assert macros
//

// We redefine the default _assert_ macro that panics the program if the condition is false.
#undef assert
#define assert(x) assert_helper(x, true, (!!(LINE_NAME(a))), "==")
#define assert_true(x) assert(x)
#define assert_false(x) assert_helper(x, false, (!(LINE_NAME(a))), "==")

#define assert_eq(x, y) assert_helper(x, y, LINE_NAME(a) == LINE_NAME(b), "==")
#define assert_nq(x, y) assert_helper(x, y, LINE_NAME(a) != LINE_NAME(b), "!=")
#define assert_lt(x, y) assert_helper(x, y, LINE_NAME(a) < LINE_NAME(b), "<")
#define assert_le(x, y) assert_helper(x, y, LINE_NAME(a) <= LINE_NAME(b), "<=")
#define assert_gt(x, y) assert_helper(x, y, LINE_NAME(a) > LINE_NAME(b), ">")
#define assert_ge(x, y) assert_helper(x, y, LINE_NAME(a) >= LINE_NAME(b), ">=")

#define assert_helper(x, y, condition, op)                        \
    {                                                             \
        ++asserts::GlobalCalledCount;                             \
        auto LINE_NAME(a) = x;                                    \
        auto LINE_NAME(b) = y;                                    \
        if (!(condition)) {                                       \
            string message = sprint(                              \
                "{}:{} {!YELLOW}{} {} {}{!GRAY},\n"               \
                "                LHS : {!YELLOW}\"{}\"{!GRAY},\n" \
                "                RHS: {!YELLOW}\"{}\"{!}",        \
                get_short_file_path(__FILE__),                    \
                __LINE__,                                         \
                u8## #x,                                          \
                op,                                               \
                u8## #y,                                          \
                LINE_NAME(a),                                     \
                LINE_NAME(b));                                    \
            add(&asserts::GlobalFailed, message);                 \
        }                                                         \
    }

//
// Define test stuff:
//

using test_func = void (*)();

struct test {
    string Name;
    test_func Function = null;
};

inline bool strings_match_for_table(string ref a, string ref b) {
    return strings_match(a, b);
}

// Gets filled out by a function "build_test_table"
// and that function gets generated by a python script.
//
// Key is a file, and array<test> contains all tests.
inline hash_table<string, array<test>> g_TestTable;

void build_test_table();
#define TEST(name) void test_##name()

// #define _TEST(name)                                                     \
//     struct _MACRO_CONCAT(test_, __LINE__)##_##name {                    \
//         _MACRO_CONCAT(test_, __LINE__)                                  \
//         ##_##name() {                                                   \
//             string shortFile = get_short_file_path(__FILE__);           \
//             array_append(*g_TestTable[shortFile], {#name, &run});       \
//         }                                                               \
//         static void run();                                              \
//     };                                                                  \
//     static _MACRO_CONCAT(test_, __LINE__)##_##name g_TestStruct_##name; \
//     void _MACRO_CONCAT(test_, __LINE__)##_##name::run()
//
// #define TEST(name) _TEST(name)