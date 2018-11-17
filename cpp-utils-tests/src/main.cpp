
#include <gu/memory/pool.h>
#include <gu/memory/table.h>

#include <gu/format/fmt.h>

#include "test.h"

// !!! WORKAORUND
// Look in test.h
Table<string_view, Dynamic_Array<Test> *> g_TestTable;

// Returns an array of failed assert messages for this file.
std::pair<size_t, Dynamic_Array<string>> run_tests_in_file(const string_view &fileName,
                                                           const Dynamic_Array<Test> &tests) {
    size_t assertsCalledCount = 0;
    Dynamic_Array<string> allFailedAsserts;

    fmt::print("{}:\n", string(fileName));

    size_t sucessfulProcs = tests.Count;
    for (const Test &test : tests) {
        // 1. Print the test's name.
        s32 length = Min(30, (s32) test.Name.Length);
        fmt::print("        {:.{}} {} ", test.Name, length, string(".") * (35 - length));

        // 2. Run the actual test function
        Dynamic_Array<string> failedAsserts;

        auto testContext = __context;
        Assert_Function defaultAssert = testContext.AssertFailed;
        testContext.AssertFailed = [&](const char *file, int line, const char *condition) {
            if (fileName == get_file_path_relative_to_src_or_just_file_name(file)) {
                failedAsserts.add(fmt::sprint("{}:{} Assert failed: %", fileName, line, condition));
                assertsCalledCount++;
            } else {
                defaultAssert(file, line, condition);
            }
        };
        PUSH_CONTEXT(testContext) { test.Function(); }

        // 3. Print information about the failed asserts
        if (failedAsserts.Count) {
            // TODO: We should have a console text formatting library...
            fmt::print("\033[38;5;160mFAILED\033[0m\n");
            for (string &message : failedAsserts) fmt::print("          \033[38;5;246m>>> {}\033[0m\n", message);
            fmt::print("\n");
            sucessfulProcs--;
            allFailedAsserts.insert(allFailedAsserts.end(), failedAsserts.begin(), failedAsserts.end());
        } else {
            // TODO: We should have a console text formatting library...
            fmt::print("\033[38;5;28mOK\033[0m\n");
        }
    }

    // TODO: We don't have formatting for float numbers yet
    string percentage = "0.0";  // fmt::to_string(100.0f * (f32) sucessfulProcs / (f32) tests.Count);
                                // TODO: We should have a console text formatting library...
    fmt::print("\033[38;5;246m{}% success ({} out of {} procs)\n\033[0m\n", percentage, sucessfulProcs, tests.Count);

    return {assertsCalledCount, allFailedAsserts};
}

void run_tests() {
    size_t totalAssertsCount = 0;
    Dynamic_Array<string> allFailedAsserts;

    fmt::print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        auto [assertsCalled, failedAsserts] = run_tests_in_file(fileName, *tests);
        allFailedAsserts.insert(allFailedAsserts.end(), failedAsserts.begin(), failedAsserts.end());
        totalAssertsCount += assertsCalled;
    }
    fmt::print("\n\n");

    size_t successfulAsserts = totalAssertsCount - allFailedAsserts.Count;

    string percentage = "0.000";
    if (totalAssertsCount) {
        // TODO: We don't have formatting for float numbers yet
        // percentage = fmt::to_string(100.0f * (f32) successfulAsserts / (f32) totalAssertsCount, 0, 3);
    }
    fmt::print("[Test Suite] {}% success ({}/{} test asserts)\n", percentage, successfulAsserts, totalAssertsCount);

    if (allFailedAsserts.Count) {
        fmt::print("[Test Suite] Failed asserts:\n");
        for (string &message : allFailedAsserts) {
            fmt::print("        >>> \033[38;5;160mFAILED:\033[38;5;246m {}\033[0m\n", message);
        }
    }
    fmt::print("\n");
}

int main() {
    temporary_storage_init(4_MiB);

    auto tempContext = __context;
    tempContext.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(tempContext) {
        TEMPORARY_STORAGE_MARK_SCOPE;
        run_tests();
    }

    string result = fmt::sprint("{}, {}, {}", 1, 2, 6);
    assert(result == "1, 2, 6");
    result = fmt::sprint("{2}, {0}, {1}", 1, 2, 6);
    assert(result == "6, 1, 2");

    result = fmt::sprint("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    assert(result == "int: 42;  hex: 2a;  oct: 52; bin: 101010");
    result = fmt::sprint("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42);
    assert(result == "int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010");
}
