#pragma once

#include <lstd/basic.h>
#include <lstd/io.h>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      /home/.../lstd-tests/src/tests/string.cpp ---> tests/string.cpp
//      /home/.../lstd-tests/string.cpp           ---> string.cpp
//
constexpr string_view get_file_path_relative_to_src_or_just_file_name(string_view str) {
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

using test_func = void (*)();

struct test {
    string_view Name;
    test_func Function;
};

inline table<string_view, array<test>> g_TestTable;

struct asserts {
    inline static size_t GlobalCalledCount;
    inline static array<string> GlobalFailed;
};

#define TEST(name)                                                                 \
    struct test_struct_##name {                                                    \
        test_struct_##name() {                                                     \
            auto file = get_file_path_relative_to_src_or_just_file_name(__FILE__); \
            g_TestTable[file]->append(test{#name, &run});                          \
        }                                                                          \
        static void run();                                                         \
    };                                                                             \
    static test_struct_##name g_TestStruct_##name;                                 \
    void test_struct_##name::run()

// Define helper assert macros

// We redefine the default _assert_ macro that panics the program if the condition is false.
#undef assert
#define assert(x)                                                                                                     \
    ++asserts::GlobalCalledCount;                                                                                     \
    if (!(x)) {                                                                                                       \
        asserts::GlobalFailed.append(fmt::sprint("{}:{} Expected {!YELLOW}true{!GRAY}: {!YELLOW}{}{!}",               \
                                                 get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                 u8## #x));                                                           \
    }

// Prefer these to just _assert_
#define assert_true(x) assert(x)
#define assert_false(x)                                                                                               \
    ++asserts::GlobalCalledCount;                                                                                     \
    if (!!(x)) {                                                                                                      \
        asserts::GlobalFailed.append(fmt::sprint("{}:{} Expected {!YELLOW}false{!GRAY}: {!YELLOW}{}{!}",              \
                                                 get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                 u8## #x));                                                           \
    }

// x == y
#define assert_eq(x, y)                                                                                                \
    ++asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ == b##__LINE__)) {                                                                           \
            asserts::GlobalFailed.append(                                                                              \
                fmt::sprint("{}:{} {!YELLOW}{} == {}{!GRAY}, expected {!YELLOW}\"{}\"{!GRAY}, got {!YELLOW}\"{}{!}\"", \
                            get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, u8## #x, u8## #y,     \
                            a##__LINE__, b##__LINE__));                                                                \
        }                                                                                                              \
    }

// x != y
#define assert_nq(x, y)                                                                                            \
    ++asserts::GlobalCalledCount;                                                                                  \
    {                                                                                                              \
        auto a##__LINE__ = x;                                                                                      \
        auto b##__LINE__ = y;                                                                                      \
        if (!(a##__LINE__ != b##__LINE__)) {                                                                       \
            asserts::GlobalFailed.append(                                                                          \
                fmt::sprint("{}:{} {!YELLOW}{} != {}{!GRAY}, got {!YELLOW}\"{}\"{!GRAY}, and {!YELLOW}\"{}{!}\"",  \
                            get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, u8## #x, u8## #y, \
                            a##__LINE__, b##__LINE__));                                                            \
        }                                                                                                          \
    }

// x < y
#define assert_lt(x, y)                                                                                            \
    ++asserts::GlobalCalledCount;                                                                                  \
    {                                                                                                              \
        auto a##__LINE__ = x;                                                                                      \
        auto b##__LINE__ = y;                                                                                      \
        if (!(a##__LINE__ < b##__LINE__)) {                                                                        \
                fmt::sprint("{}:{} {!YELLOW}{} < {}{!GRAY}, got {!YELLOW}\"{}\"{!GRAY}, and {!YELLOW}\"{}{!}\"",  \
                            get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, u8## #x, u8## #y, \
                            a##__LINE__, b##__LINE__));                                                            \
        }                                                                                                          \
    }

// x <= y
#define assert_le(x, y)                                                                                            \
    ++asserts::GlobalCalledCount;                                                                                  \
    {                                                                                                              \
        auto a##__LINE__ = x;                                                                                      \
        auto b##__LINE__ = y;                                                                                      \
        if (!(a##__LINE__ <= b##__LINE__)) {                                                                       \
                fmt::sprint("{}:{} {!YELLOW}{} <= {}{!GRAY}, got {!YELLOW}\"{}\"{!GRAY}, and {!YELLOW}\"{}{!}\"",  \
                            get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, u8## #x, u8## #y, \
                            a##__LINE__, b##__LINE__));                                                            \
        }                                                                                                          \
    }

// x > y
#define assert_gt(x, y)                                                                                            \
    ++asserts::GlobalCalledCount;                                                                                  \
    {                                                                                                              \
        auto a##__LINE__ = x;                                                                                      \
        auto b##__LINE__ = y;                                                                                      \
        if (!(a##__LINE__ > b##__LINE__)) {                                                                        \
                fmt::sprint("{}:{} {!YELLOW}{} > {}{!GRAY}, got {!YELLOW}\"{}\"{!GRAY}, and {!YELLOW}\"{}{!}\"",  \
                            get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, u8## #x, u8## #y, \
                            a##__LINE__, b##__LINE__));                                                            \
        }                                                                                                          \
    }

// x >= y
#define assert_ge(x, y)                                                                                            \
    ++asserts::GlobalCalledCount;                                                                                  \
    {                                                                                                              \
        auto a##__LINE__ = x;                                                                                      \
        auto b##__LINE__ = y;                                                                                      \
        if (!(a##__LINE__ >= b##__LINE__)) {                                                                       \
                fmt::sprint("{}:{} {!YELLOW}{} >= {}{!GRAY}, got {!YELLOW}\"{}\"{!GRAY}, and {!YELLOW}\"{}{!}\"",  \
                            get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, u8## #x, u8## #y, \
                            a##__LINE__, b##__LINE__));                                                            \
        }                                                                                                          \
    }

