#pragma once

#include <lstd/basic.h>

#include <lstd/io.h>
#include <lstd/io/fmt.h>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      /home/.../lstd-tests/src/tests/string.cpp ---> tests/string.cpp
//      /home/.../lstd-tests/string.cpp           ---> string.cpp
//
constexpr string_view get_short_file_name(string_view str) {
    // @Platform '\\' is Windows-specific
    char srcData[] = {'s', 'r', 'c', '\\', '\0'};
    string_view src = srcData;

    size_t findResult = str.find_reverse(src);
    if (findResult == npos) {
        findResult = str.find_reverse('\\');  // @Platform
        assert(findResult != str.Length - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        findResult += src.Length;
    }

    string_view result = str;
    return result.substring(findResult, result.Length);
}

struct asserts {
    inline static size_t GlobalCalledCount;
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

template <typename T, typename U>
inline void test_assert_failed_binary_operator(const byte *var1, const byte *op, const byte *var2, const byte *file,
                                               s32 line, T *value1, U *value2) {
    constexpr auto *fmtString = "{}:{} {!YELLOW}{} {} {}{!GRAY}, LHS: {!YELLOW}{}{!GRAY}, RHS: {!YELLOW}{}{!}";

    string_view shortFile = get_short_file_name(__FILE__);
    fmt::sprint(asserts::GlobalFailed.append(), fmtString, shortFile, line, var1, op, var2, *value1, *value2);
}

#define assert_helper(x, y, condition, op)                                                                          \
    ++asserts::GlobalCalledCount;                                                                                   \
    auto LINE_NAME(a) = x;                                                                                          \
    auto LINE_NAME(b) = y;                                                                                          \
    if (!(condition)) {                                                                                             \
        test_assert_failed_binary_operator(u8## #x, op, u8## #y, __FILE__, __LINE__, &LINE_NAME(a), &LINE_NAME(b)); \
    }

//
// Define test stuff:
//

using test_func = void (*)();

struct test {
    string Name;
    test_func Function = null;

    test(string name, test_func function) : Name(name), Function(function) {}

    // Required by _array_
    bool operator==(test other) const { return Name == other.Name && Function == other.Function; }
};

inline table<string_view, array<test>> g_TestTable;

#define TEST(name)                                                 \
    struct test_##name {                                           \
        test_##name() {                                            \
            string_view shortFile = get_short_file_name(__FILE__); \
            g_TestTable[shortFile]->append(test(#name, &run));     \
        }                                                          \
        static void run();                                         \
    };                                                             \
    static test_##name g_TestStruct_##name;                        \
    void test_##name::run()
