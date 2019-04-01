#pragma once

#include <lstd/containers.hpp>
#include <lstd/io.hpp>

#include <lstd/file/path.hpp>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      .../home/user/dev/sandbox-tests/src/tests/string.cpp ---> tests/string.cpp
//      .../home/user/dev/sandbox-tests/string.cpp           ---> string.cpp
//
constexpr string_view get_file_path_relative_to_src_or_just_file_name(const string_view &str) {
    char srcData[] = {'s', 'r', 'c', file::OS_PATH_SEPARATOR, '\0'};
    string_view src = srcData;

    size_t findResult = str.find_reverse(src);
    if (findResult == npos) {
        findResult = str.find_reverse(file::OS_PATH_SEPARATOR);
        assert(findResult != str.Length - 1);
        // Skip the slash
        findResult++;
    } else {
        // Skip the src directory
        findResult += src.Length;
    }

    string_view result = str;
    result.remove_prefix(findResult);
    return result;
}

typedef void (*test_func)();

struct test {
    string_view Name;
    test_func Function;
};

// !!! WORKAORUND
// Visual Studio compiler has a bug (suprise surpise) with inline variables getting initialized multiple times.
// So a workaround for now is to extern it instead.
// inline Table<string, Dynamic_Array<Test> *> g_TestTable;
//
// Definition of this in main.cpp
extern table<string_view, dynamic_array<test> *> g_TestTable;

struct asserts {
    inline static size_t GlobalCalledCount;
    inline static dynamic_array<string> GlobalFailed;
};

#define TEST(name)                                                                        \
    struct Test_Struct_##name {                                                           \
        Test_Struct_##name() {                                                            \
            string_view file = get_file_path_relative_to_src_or_just_file_name(__FILE__); \
                                                                                          \
            auto [testsArray, fileKeyFound] = g_TestTable.find(file);                     \
            if (!fileKeyFound) {                                                          \
                testsArray = new dynamic_array<test>;                                     \
                g_TestTable.put(file, testsArray);                                        \
            }                                                                             \
                                                                                          \
            testsArray->append(test{#name, &run});                                        \
        }                                                                                 \
        static void run();                                                                \
    };                                                                                    \
    static Test_Struct_##name g_TestStruct_##name;                                        \
    void Test_Struct_##name::run()

inline void test_assert_helper(const byte *fileName, u32 line, const byte *condition, bool eval, bool expected) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++asserts::GlobalCalledCount;
    if (expected && !eval) {
        asserts::GlobalFailed.append(sprint("{}:{} Expected {}true{}: {}{}{}", shortFile, line, fmt::fg::Yellow,
                                            fmt::fgb::Gray, fmt::fg::Yellow, condition, fmt::fg::Reset));
    }
    if (!expected && eval) {
        asserts::GlobalFailed.append(sprint("{}:{} Expected {}false{}: {}{}{}", shortFile, line, fmt::fg::Yellow,
                                            fmt::fgb::Gray, fmt::fg::Yellow, condition, fmt::fg::Reset));
    }
}

// Define helper assert macros

// We redefine the default context _assert_ macro that panics the program if the condition is false.
#undef assert
#define assert(x)                                                                                                     \
    ++asserts::GlobalCalledCount;                                                                                     \
    if (!(x)) {                                                                                                       \
        asserts::GlobalFailed.append(                                                                                 \
            fmt::sprint("{}:{} Expected {}true{}: {}{}{}", get_file_path_relative_to_src_or_just_file_name(__FILE__), \
                        __LINE__, fmt::fg::Yellow, fmt::fgb::Gray, fmt::fg::Yellow, u8## #x, fmt::fg::Reset));        \
    }

// Prefer these to just _assert_
#define assert_true(x) assert(x)
#define assert_false(x)                                                                                                \
    ++asserts::GlobalCalledCount;                                                                                      \
    if (!!(x)) {                                                                                                       \
        asserts::GlobalFailed.append(                                                                                  \
            fmt::sprint("{}:{} Expected {}false{}: {}{}{}", get_file_path_relative_to_src_or_just_file_name(__FILE__), \
                        __LINE__, fmt::fg::Yellow, fmt::fgb::Gray, fmt::fg::Yellow, u8## #x, fmt::fg::Reset));         \
    }

// x == y
#define assert_eq(x, y)                                                                                             \
    ++asserts::GlobalCalledCount;                                                                                   \
    {                                                                                                               \
        auto a##__LINE__ = x;                                                                                       \
        auto b##__LINE__ = y;                                                                                       \
        if (!(a##__LINE__ == b##__LINE__)) {                                                                        \
            asserts::GlobalFailed.append(fmt::sprint("{}:{} {}{} == {}{}, expected {}\"{}\"{}, got {}\"{}{}\"",     \
                                                     get_file_path_relative_to_src_or_just_file_name(__FILE__),     \
                                                     __LINE__, fmt::fg::Yellow, u8## #x, u8## #y, fmt::fgb::Gray,   \
                                                     fmt::fg::Yellow, a##__LINE__, fmt::fgb::Gray, fmt::fg::Yellow, \
                                                     b##__LINE__, fmt::fg::Reset));                                 \
        }                                                                                                           \
    }

// x != y
#define assert_nq(x, y)                                                                                             \
    ++asserts::GlobalCalledCount;                                                                                   \
    {                                                                                                               \
        auto a##__LINE__ = x;                                                                                       \
        auto b##__LINE__ = y;                                                                                       \
        if (!(a##__LINE__ != b##__LINE__)) {                                                                        \
            asserts::GlobalFailed.append(fmt::sprint("{}:{} {}{} != {}{}, got {}\"{}\"{}, and {}\"{}{}\"",          \
                                                     get_file_path_relative_to_src_or_just_file_name(__FILE__),     \
                                                     __LINE__, fmt::fg::Yellow, u8## #x, u8## #y, fmt::fgb::Gray,   \
                                                     fmt::fg::Yellow, a##__LINE__, fmt::fgb::Gray, fmt::fg::Yellow, \
                                                     b##__LINE__, fmt::fg::Reset));                                 \
        }                                                                                                           \
    }

// x < y
#define assert_lt(x, y)                                                                                             \
    ++asserts::GlobalCalledCount;                                                                                   \
    {                                                                                                               \
        auto a##__LINE__ = x;                                                                                       \
        auto b##__LINE__ = y;                                                                                       \
        if (!(a##__LINE__ < b##__LINE__)) {                                                                         \
            asserts::GlobalFailed.append(fmt::sprint("{}:{} {}{} < {}{}, got: {}\"{}\"{} and {}\"{}\"{}",           \
                                                     get_file_path_relative_to_src_or_just_file_name(__FILE__),     \
                                                     __LINE__, fmt::fg::Yellow, u8## #x, u8## #y, fmt::fgb::Gray,   \
                                                     fmt::fg::Yellow, a##__LINE__, fmt::fgb::Gray, fmt::fg::Yellow, \
                                                     b##__LINE__, fmt::fg::Reset));                                 \
        }                                                                                                           \
    }

// x <= y
#define assert_le(x, y)                                                                                             \
    ++asserts::GlobalCalledCount;                                                                                   \
    {                                                                                                               \
        auto a##__LINE__ = x;                                                                                       \
        auto b##__LINE__ = y;                                                                                       \
        if (!(a##__LINE__ <= b##__LINE__)) {                                                                        \
            asserts::GlobalFailed.append(fmt::sprint("{}:{} {}{} <= {}{}, got: {}\"{}\"{} and {}\"{}\"{}",          \
                                                     get_file_path_relative_to_src_or_just_file_name(__FILE__),     \
                                                     __LINE__, fmt::fg::Yellow, u8## #x, u8## #y, fmt::fgb::Gray,   \
                                                     fmt::fg::Yellow, a##__LINE__, fmt::fgb::Gray, fmt::fg::Yellow, \
                                                     b##__LINE__, fmt::fg::Reset));                                 \
        }                                                                                                           \
    }

// x > y
#define assert_gt(x, y)                                                                                             \
    ++asserts::GlobalCalledCount;                                                                                   \
    {                                                                                                               \
        auto a##__LINE__ = x;                                                                                       \
        auto b##__LINE__ = y;                                                                                       \
        if (!(a##__LINE__ > b##__LINE__)) {                                                                         \
            asserts::GlobalFailed.append(fmt::sprint("{}:{} {}{} > {}{}, got: {}\"{}\"{} and {}\"{}\"{}",           \
                                                     get_file_path_relative_to_src_or_just_file_name(__FILE__),     \
                                                     __LINE__, fmt::fg::Yellow, u8## #x, u8## #y, fmt::fgb::Gray,   \
                                                     fmt::fg::Yellow, a##__LINE__, fmt::fgb::Gray, fmt::fg::Yellow, \
                                                     b##__LINE__, fmt::fg::Reset));                                 \
        }                                                                                                           \
    }

// x >= y
#define assert_ge(x, y)                                                                                             \
    ++asserts::GlobalCalledCount;                                                                                   \
    {                                                                                                               \
        auto a##__LINE__ = x;                                                                                       \
        auto b##__LINE__ = y;                                                                                       \
        if (!(a##__LINE__ >= b##__LINE__)) {                                                                        \
            asserts::GlobalFailed.append(fmt::sprint("{}:{} {}{} >= {}{}, got: {}\"{}\"{} and {}\"{}\"{}",          \
                                                     get_file_path_relative_to_src_or_just_file_name(__FILE__),     \
                                                     __LINE__, fmt::fg::Yellow, u8## #x, u8## #y, fmt::fgb::Gray,   \
                                                     fmt::fg::Yellow, a##__LINE__, fmt::fgb::Gray, fmt::fg::Yellow, \
                                                     b##__LINE__, fmt::fg::Reset));                                 \
        }                                                                                                           \
    }
