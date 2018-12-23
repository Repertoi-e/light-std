#pragma once

#include <cppu/memory/dynamic_array.h>
#include <cppu/memory/table.h>

#include <cppu/format/fmt.h>

#include <cppu/file/file_path.h>

// This is a helper function to shorten the name of test files.
// We check if the path contains src/ and use the rest after that.
// Otherwise we just take the file name. Possible results are:
//
//      .../home/user/dev/sandbox-tests/src/tests/string.cpp ---> tests/string.cpp
//      .../home/user/dev/sandbox-tests/string.cpp           ---> string.cpp
//
constexpr string_view get_file_path_relative_to_src_or_just_file_name(const string_view &str) {
    char srcData[] = {'s', 'r', 'c', OS_PATH_SEPARATOR, '\0'};
    string_view src = srcData;

    size_t findResult = str.find_last(src);
    if (findResult == npos) {
        findResult = str.find_last(OS_PATH_SEPARATOR);
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

struct Asserts {
    static u32 GlobalCalledCount;
    static Dynamic_Array<string> GlobalFailed;
};

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

inline void test_assert_helper(const char *fileName, u32 line, const char *condition, bool eval, bool expected) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++Asserts::GlobalCalledCount;
    if (expected && !eval) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} Expected {}true{}: {}{}{}", shortFile, line, fmt::FG::Yellow,
                                              fmt::FGB::Gray, fmt::FG::Yellow, condition, fmt::FG::Reset));
    }
    if (!expected && eval) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} Expected {}false{}: {}{}{}", shortFile, line, fmt::FG::Yellow,
                                              fmt::FGB::Gray, fmt::FG::Yellow, condition, fmt::FG::Reset));
    }
}

// Define helper assert macros

// We redefine the default context _assert_ macro that panics the program if the condition is false.
#undef assert
#define assert(x)                                                                                                     \
    ++Asserts::GlobalCalledCount;                                                                                     \
    if (!(x)) {                                                                                                       \
        Asserts::GlobalFailed.add(                                                                                    \
            fmt::sprint("{}:{} Expected {}true{}: {}{}{}", get_file_path_relative_to_src_or_just_file_name(__FILE__), \
                        __LINE__, fmt::FG::Yellow, fmt::FGB::Gray, fmt::FG::Yellow, u8## #x, fmt::FG::Reset));        \
    }

// Prefer these to just _assert_
#define assert_true(x) assert(x)
#define assert_false(x)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    if (!!(x)) {                                                                                                       \
        Asserts::GlobalFailed.add(                                                                                     \
            fmt::sprint("{}:{} Expected {}false{}: {}{}{}", get_file_path_relative_to_src_or_just_file_name(__FILE__), \
                        __LINE__, fmt::FG::Yellow, fmt::FGB::Gray, fmt::FG::Yellow, u8## #x, fmt::FG::Reset));         \
    }

// x == y
#define assert_eq(x, y)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ == b##__LINE__)) {                                                                           \
            Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} == {}{}, expected {}\"{}\"{}, got {}\"{}{}\"",           \
                                                  get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                  fmt::FG::Yellow, u8## #x, u8## #y, fmt::FGB::Gray, fmt::FG::Yellow,  \
                                                  b##__LINE__, fmt::FGB::Gray, fmt::FG::Yellow, a##__LINE__,           \
                                                  fmt::FG::Reset));                                                    \
        }                                                                                                              \
    }

// x != y
#define assert_nq(x, y)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ != b##__LINE__)) {                                                                           \
            Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} != {}{}, got {}\"{}\"{}, and {}\"{}{}\"",                \
                                                  get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                  fmt::FG::Yellow, u8## #x, u8## #y, fmt::FGB::Gray, fmt::FG::Yellow,  \
                                                  a##__LINE__, fmt::FGB::Gray, fmt::FG::Yellow, b##__LINE__,           \
                                                  fmt::FG::Reset));                                                    \
        }                                                                                                              \
    }

// x < y
#define assert_lt(x, y)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ < b##__LINE__)) {                                                                            \
            Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} < {}{}, got: {}\"{}\"{} and {}\"{}\"{}",                 \
                                                  get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                  fmt::FG::Yellow, u8## #x, u8## #y, fmt::FGB::Gray, fmt::FG::Yellow,  \
                                                  a##__LINE__, fmt::FGB::Gray, fmt::FG::Yellow, b##__LINE__,           \
                                                  fmt::FG::Reset));                                                    \
        }                                                                                                              \
    }

// x <= y
#define assert_le(x, y)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ <= b##__LINE__)) {                                                                           \
            Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} <= {}{}, got: {}\"{}\"{} and {}\"{}\"{}",                \
                                                  get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                  fmt::FG::Yellow, u8## #x, u8## #y, fmt::FGB::Gray, fmt::FG::Yellow,  \
                                                  a##__LINE__, fmt::FGB::Gray, fmt::FG::Yellow, b##__LINE__,           \
                                                  fmt::FG::Reset));                                                    \
        }                                                                                                              \
    }

// x > y
#define assert_gt(x, y)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ > b##__LINE__)) {                                                                            \
            Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} > {}{}, got: {}\"{}\"{} and {}\"{}\"{}",                 \
                                                  get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                  fmt::FG::Yellow, u8## #x, u8## #y, fmt::FGB::Gray, fmt::FG::Yellow,  \
                                                  a##__LINE__, fmt::FGB::Gray, fmt::FG::Yellow, b##__LINE__,           \
                                                  fmt::FG::Reset));                                                    \
        }                                                                                                              \
    }

// x >= y
#define assert_ge(x, y)                                                                                                \
    ++Asserts::GlobalCalledCount;                                                                                      \
    {                                                                                                                  \
        auto a##__LINE__ = x;                                                                                          \
        auto b##__LINE__ = y;                                                                                          \
        if (!(a##__LINE__ >= b##__LINE__)) {                                                                           \
            Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} >= {}{}, got: {}\"{}\"{} and {}\"{}\"{}",                \
                                                  get_file_path_relative_to_src_or_just_file_name(__FILE__), __LINE__, \
                                                  fmt::FG::Yellow, u8## #x, u8## #y, fmt::FGB::Gray, fmt::FG::Yellow,  \
                                                  a##__LINE__, fmt::FGB::Gray, fmt::FG::Yellow, b##__LINE__,           \
                                                  fmt::FG::Reset));                                                    \
        }                                                                                                              \
    }
