#define GU_NO_CRT

#include <gu/memory/pool.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "test.h"

// !!! WORKAORUND
// Look in test.h
Table<string_view, Dynamic_Array<Test> *> g_TestTable;

// Returns an array of failed assert messages for this file.
std::pair<size_t, Dynamic_Array<string>> run_tests_in_file(const string_view &fileName,
                                                           const Dynamic_Array<Test> &tests) {
    size_t assertsCalledCount = 0;
    Dynamic_Array<string> allFailedAsserts;

    print("%:\n", string(fileName));

    size_t sucessfulProcs = tests.Count;
    for (const Test &test : tests) {
        // 1. Print the test's name.
        s32 length = Min(30, (s32) test.Name.Length);
        print("        % % ", to_string(string(test.Name), (s32) length), string(".") * (35 - length));

        // 2. Run the actual test function
        Dynamic_Array<string> failedAsserts;

        auto testContext = __context;
        Assert_Function defaultAssert = testContext.AssertFailed;
        testContext.AssertFailed = [&](const char *file, int line, const char *condition) {
            if (fileName == get_file_path_relative_to_src_or_just_file_name(file)) {
                failedAsserts.add(sprint("%:% Assert failed: %", string(fileName), line, condition));
                assertsCalledCount++;
            } else {
                defaultAssert(file, line, condition);
            }
        };
        PUSH_CONTEXT(testContext) { test.Function(); }

        // 3. Print information about the failed asserts
        if (failedAsserts.Count) {
            // TODO: We should have a console text formatting library...
            print("\033[38;5;160mFAILED\033[0m\n");
            for (string &message : failedAsserts) print("          \033[38;5;246m>>> %\033[0m\n", message);
            print("\n");
            sucessfulProcs--;
            allFailedAsserts.insert(allFailedAsserts.end(), failedAsserts.begin(), failedAsserts.end());
        } else {
            // TODO: We should have a console text formatting library...
            print("\033[38;5;28mOK\033[0m\n");
        }
    }

    string percentage = to_string(100.0f * (f32) sucessfulProcs / (f32) tests.Count, 0, 1);
    // TODO: We should have a console text formatting library...
    print("\033[38;5;246m%1%% success (% out of % procs)\n\033[0m\n", percentage, sucessfulProcs, tests.Count);

    return {assertsCalledCount, allFailedAsserts};
}

void run_tests() {
    size_t totalAssertsCount = 0;
    Dynamic_Array<string> allFailedAsserts;

    print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        auto [assertsCalled, failedAsserts] = run_tests_in_file(fileName, *tests);
        allFailedAsserts.insert(allFailedAsserts.end(), failedAsserts.begin(), failedAsserts.end());
        totalAssertsCount += assertsCalled;
    }
    print("\n\n");

    size_t successfulAsserts = totalAssertsCount - allFailedAsserts.Count;

    string percentage = "0.000";
    if (totalAssertsCount) {
        percentage = to_string(100.0f * (f32) successfulAsserts / (f32) totalAssertsCount, 0, 3);
    }
    print("[Test Suite] %1%% success (%/% test asserts)\n", percentage, successfulAsserts, totalAssertsCount);

    if (allFailedAsserts.Count) {
        print("[Test Suite] Failed asserts:\n");
        for (string &message : allFailedAsserts) {
            print("        >>> \033[38;5;160mFAILED:\033[38;5;246m %\033[0m\n", message);
        }
    }
    print("\n");
}

int main() {
    temporary_storage_init(4_MiB);

    auto tempContext = __context;
    tempContext.Allocator = TEMPORARY_ALLOC;
    PUSH_CONTEXT(tempContext) {
        TEMPORARY_STORAGE_MARK_SCOPE;
        run_tests();
    }
}
