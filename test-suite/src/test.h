#pragma once

#include <lstd/file.h>
#include <lstd/fmt.h>
#include <lstd/io.h>
#include <lstd/memory/array.h>
#include <lstd/memory/table.h>
#include <lstd/os.h>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      /home/.../lstd-tests/src/tests/string.cpp ---> tests/string.cpp
//      /home/.../lstd-tests/string.cpp           ---> string.cpp
//
constexpr string get_short_file_name(const string &str) {
    char srcData[] = {'s', 'r', 'c', file::OS_PATH_SEPARATORS[0], '\0'};
    string src = srcData;

    s64 findResult = str.find_reverse(src);
    if (findResult == -1) {
        findResult = str.find_reverse(file::OS_PATH_SEPARATORS[0]);
        assert(findResult != str.Length - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        findResult += src.Length;
    }

    string result = str;
    return result.substring(findResult, result.Length);
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

#define assert_helper(x, y, condition, op)                                                                  \
    {                                                                                                       \
        ++asserts::GlobalCalledCount;                                                                       \
        auto LINE_NAME(a) = x;                                                                              \
        auto LINE_NAME(b) = y;                                                                              \
        if (!(condition)) {                                                                                 \
            string message = fmt::sprint(                                                                   \
                "{}:{} {!YELLOW}{} {} {}{!GRAY},\n"                                                         \
                "                LHS : {!YELLOW}\"{}\"{!GRAY},\n"                                           \
                "                RHS: {!YELLOW}\"{}\"{!}",                                                  \
                get_short_file_name(__FILE__), __LINE__, u8## #x, op, u8## #y, LINE_NAME(a), LINE_NAME(b)); \
            asserts::GlobalFailed.append(message);                                                          \
        }                                                                                                   \
    }

//
// Define test stuff:
//

using test_func = void (*)();

struct test {
    string Name;
    test_func Function = null;
};

inline hash_table<string, array<test>> g_TestTable;

#define _TEST(name)                                                     \
    struct _MACRO_CONCAT(test_, __LINE__)##_##name {                    \
        _MACRO_CONCAT(test_, __LINE__)                                  \
        ##_##name() {                                                   \
            string shortFile = get_short_file_name(__FILE__);           \
            g_TestTable[shortFile]->append({#name, &run});              \
        }                                                               \
        static void run();                                              \
    };                                                                  \
    static _MACRO_CONCAT(test_, __LINE__)##_##name g_TestStruct_##name; \
    void _MACRO_CONCAT(test_, __LINE__)##_##name::run()

#define TEST(name) _TEST(name)
