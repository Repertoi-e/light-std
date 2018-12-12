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

// We redefine the default context `assert` macro.
// Note that this is a rare case.
#undef assert
#define assert(x) test_assert_helper(__FILE__, __LINE__, u8## #x, !!(x), true)

// Prefer these to just assert(x)
#define assert_true(x) assert(x)
#define assert_false(x) test_assert_helper(__FILE__, __LINE__, u8## #x, !!(x), false)

template <typename T, typename U>
inline void test_assert_eq_helper(const char *fileName, u32 line, const char *a, const char *b, const T &aValue,
                                  const U &bValue, bool expected) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++Asserts::GlobalCalledCount;
    if (expected && !(aValue == bValue)) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} == {}{}, expected {}\"{}\"{}, but got {}\"{}{}\"", shortFile,
                                              line, fmt::FG::Yellow, a, b, fmt::FGB::Gray, fmt::FG::Yellow, bValue,
                                              fmt::FGB::Gray, fmt::FG::Yellow, aValue, fmt::FG::Reset));
    }
    if (!expected && !(aValue != bValue)) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} != {}{}, got: {}\"{}\"{} and {}\"{}\"{}", shortFile, line,
                                              fmt::FG::Yellow, a, b, fmt::FGB::Gray, fmt::FG::Yellow, aValue,
                                              fmt::FGB::Gray, fmt::FG::Yellow, bValue, fmt::FG::Reset));
    }
}

// x == y
#define assert_eq(x, y) test_assert_eq_helper(__FILE__, __LINE__, u8## #x, u8## #y, x, y, true)

// x != y
#define assert_nq(x, y) test_assert_eq_helper(__FILE__, __LINE__, u8## #x, u8## #y, x, y, false)

template <typename T, typename U>
inline void test_assert_lt_helper(const char *fileName, u32 line, const char *a, const char *b, const T &aValue,
                                  const U &bValue) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++Asserts::GlobalCalledCount;
    if (!(aValue < bValue)) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} < {}{}, got: {}\"{}\"{} and {}\"{}\"{}", shortFile, line,
                                              fmt::FG::Yellow, a, b, fmt::FGB::Gray, fmt::FG::Yellow, aValue,
                                              fmt::FGB::Gray, fmt::FG::Yellow, bValue, fmt::FG::Reset));
    }
}

// x < y
#define assert_lt(x, y) test_assert_lt_helper(__FILE__, __LINE__, u8## #x, u8## #y, x, y)

template <typename T, typename U>
inline void test_assert_le_helper(const char *fileName, u32 line, const char *a, const char *b, const T &aValue,
                                  const U &bValue) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++Asserts::GlobalCalledCount;
    if (!(aValue <= bValue)) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} <= {}{}, got: {}\"{}\"{} and {}\"{}\"{}", shortFile, line,
                                              fmt::FG::Yellow, a, b, fmt::FGB::Gray, fmt::FG::Yellow, aValue,
                                              fmt::FGB::Gray, fmt::FG::Yellow, bValue, fmt::FG::Reset));
    }
}

// x <= y
#define assert_le(x, y) test_assert_le_helper(__FILE__, __LINE__, u8## #x, u8## #y, x, y)

template <typename T, typename U>
inline void test_assert_gt_helper(const char *fileName, u32 line, const char *a, const char *b, const T &aValue,
                                  const U &bValue) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++Asserts::GlobalCalledCount;
    if (!(aValue > bValue)) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} > {}{}, got: {}\"{}\"{} and {}\"{}\"{}", shortFile, line,
                                              fmt::FG::Yellow, a, b, fmt::FGB::Gray, fmt::FG::Yellow, aValue,
                                              fmt::FGB::Gray, fmt::FG::Yellow, bValue, fmt::FG::Reset));
    }
}

// x > y
#define assert_gt(x, y) test_assert_gt_helper(__FILE__, __LINE__, u8## #x, u8## #y, x, y)

template <typename T, typename U>
inline void test_assert_ge_helper(const char *fileName, u32 line, const char *a, const char *b, const T &aValue,
                                  const U &bValue) {
    auto shortFile = get_file_path_relative_to_src_or_just_file_name(fileName);
    ++Asserts::GlobalCalledCount;
    if (!(aValue >= bValue)) {
        Asserts::GlobalFailed.add(fmt::sprint("{}:{} {}{} >= {}{}, got: {}\"{}\"{} and {}\"{}\"{}", shortFile, line,
                                              fmt::FG::Yellow, a, b, fmt::FGB::Gray, fmt::FG::Yellow, aValue,
                                              fmt::FGB::Gray, fmt::FG::Yellow, bValue, fmt::FG::Reset));
    }
}

// x >= y
#define assert_ge(x, y) test_assert_ge_helper(__FILE__, __LINE__, u8## #x, u8## #y, x, y)
