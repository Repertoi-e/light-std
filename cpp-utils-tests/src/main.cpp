#define GU_NO_CRT

#include <gu/memory/pool.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "test.h"

// !!! WORKAORUND
// Look in test.h
Table<string, Dynamic_Array<Test> *> g_TestTable;

void run_tests() {
    // Copy the current context
    auto testContext = __context;

	string currentTestFile;
	u32 totalAssertsCount = 0, totalFailedAssertsCount = 0;
	Dynamic_Array<string> currentTestFailedAsserts, allFailedAsserts;

    // Add our own assert handler so we can save some stats and logs
	testContext.AssertHandler = [&](bool failed, const char *file, int line, const char *failedCondition) {
        string shortFileName = get_file_path_relative_to_src_or_just_file_name(file);
	
        // Avoid counting assert calls from other sources (like print.h, etc.)
        if (currentTestFile == shortFileName) {
            if (failed) {
                currentTestFailedAsserts.add(sprint("%:% Assert failed: %", shortFileName, line, failedCondition));
                totalFailedAssertsCount++;
            }
            totalAssertsCount++;
        } else {
            default_assert_handler(failed, file, line, failedCondition);
        }
    };
    {
        PUSH_CONTEXT(testContext);

        print("\n");

        for (auto [fileName, tests] : g_TestTable) {
            currentTestFile = fileName;

            u32 failedProcs = 0;

            print("%:\n", fileName);
            for (Test &test : *tests) {
                size_t numberOfDots = 35 - test.Name.Length;

                string dots;
                dots.reserve(numberOfDots);
                while (numberOfDots--) {
                    dots.append_cstring(".");
                }
                print("        % % ", test.Name, dots);

                test.Function();

                if (currentTestFailedAsserts.Count != 0) {
                    print("\033[38;5;160mFAILED\033[0m\n");
                    for (string &fail : currentTestFailedAsserts) {
                        print("          \033[38;5;246m>>> %\033[0m\n", fail);
                    }
                    print("\n");
                    failedProcs++;
                } else {
                    print("\033[38;5;28mOK\033[0m\n");
                }

                // Add the failed asserts to the array of all faile asserts
                // that gets printed at the end of the test suite.
                for (string &fail : currentTestFailedAsserts) {
                    allFailedAsserts.add(fail);
                }
                currentTestFailedAsserts.release();
            }
            print("\033[38;5;246m%1%% success (% out of % procs)\n\033[0m\n",
                  to_string(100.0f * (f32)(tests->Count - failedProcs) / (f32) tests->Count, 0, 1),
                  tests->Count - failedProcs, tests->Count);
        }
    }  // testContext
    print("\n\n");

    u32 successfulAsserts = totalAssertsCount - totalFailedAssertsCount;

    f32 percentage = 0.0f;
    // Avoid dividing zero by zero o_O
    if (totalAssertsCount) {
        percentage = 100.0f * (f32) successfulAsserts / (f32) totalAssertsCount;
    }

    print("[Test Suite] %1%% success (%/% test asserts)\n", to_string(percentage, 0, 3), successfulAsserts,
          totalAssertsCount);

    if (allFailedAsserts.Count) {
        print("[Test Suite] Failed asserts:\n");
        String_Builder failedAssertsLog;
        for (string &fail : allFailedAsserts) {
            append_cstring(failedAssertsLog, "        >>> \033[38;5;160mFAILED:\033[38;5;246m ");
            append(failedAssertsLog, fail);
            append_cstring(failedAssertsLog, "\033[0m\n");
        }
        print(to_string(failedAssertsLog));
    }
    print("\n");
}

int main() {
    temporary_storage_init(4_MiB);

    auto tempContext = __context;
    tempContext.Allocator = TEMPORARY_ALLOC;
    {
        PUSH_CONTEXT(tempContext);
        TEMPORARY_STORAGE_MARK_SCOPE;
		run_tests();
	}
    run_tests();
}
