#define GU_NO_CRT

#include <gu/memory/pool.h>
#include <gu/memory/stack.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "test.h"

string g_CurrentTestFile;
u32 g_TotalAsserts = 0, g_TotalFailedAsserts = 0;
Dynamic_Array<string> g_CurrentTestFailedAsserts, g_AllFailedAsserts;
u32 g_MaxFailedAsserts;

void run_tests() {
    // Copy the current context
    auto testContext = __context;

    // Add our own assert handler so we can save some stats and logs
    testContext.AssertHandler = [](bool failed, const char *file, int line, const char *failedCondition) {
        string shortFileName = file_name_relative_to_src(file);

        // Avoid counting assert calls from other sources (like print.h, etc.)
        if (g_CurrentTestFile == shortFileName) {
            if (failed) {
                add(g_CurrentTestFailedAsserts, sprint("%:% Assert failed: %", shortFileName, line, failedCondition));
                g_TotalFailedAsserts++;
            }
            g_TotalAsserts++;
        } else {
            default_assert_handler(failed, file, line, failedCondition);
        }
    };
    {
        PUSH_CONTEXT(testContext);

        print("\n");

        for (auto [fileName, tests] : g_TestTable) {
            g_CurrentTestFile = fileName;

            u32 failedProcs = 0;

            print("%:\n", fileName);
            for (Test &test : *tests) {
                size_t numberOfDots = 35 - length(test.Name);

                string dots;
                reserve(dots, numberOfDots);
                while (numberOfDots--) {
                    append_cstring(dots, ".");
                }
                print("        % % ", test.Name, dots);

                test.Function();

                if (g_CurrentTestFailedAsserts.Count != 0) {
                    print("\e[38;5;160mFAILED\e[0m\n");
                    for (string &fail : g_CurrentTestFailedAsserts) {
                        print("          \e[38;5;246m>>> %\e[0m\n", fail);
                    }
                    print("\n");
                    failedProcs++;
                } else {
                    print("\e[38;5;28mOK\e[0m\n");
                }

                // Add the failed asserts to the array of all faile asserts
                // that gets printed at the end of the test suite.
                for (string &fail : g_CurrentTestFailedAsserts) {
                    add(g_AllFailedAsserts, fail);
                }
                release(g_CurrentTestFailedAsserts);
            }
            print("\e[38;5;246m%1%% success (% out of % procs)\n\e[0m\n",
                  to_string(100.0f * (f32)(tests->Count - failedProcs) / (f32) tests->Count, 0, 1),
                  tests->Count - failedProcs, tests->Count);
        }
    }  // testContext
    print("\n\n");

    u32 successfulAsserts = g_TotalAsserts - g_TotalFailedAsserts;

    f32 percentage = 0.0f;
    // Avoid dividing zero by zero o_O
    if (g_TotalAsserts) {
        percentage = 100.0f * (f32) successfulAsserts / (f32) g_TotalAsserts;
    }

    print("[Test Suite] %1%% success (%/% test asserts)\n", to_string(percentage, 0, 3), successfulAsserts,
          g_TotalAsserts);

    if (g_AllFailedAsserts.Count) {
        print("[Test Suite] Failed asserts:\n");
        String_Builder failedAssertsLog;
        for (string &fail : g_AllFailedAsserts) {
            append_cstring(failedAssertsLog, "        >>> \e[38;5;160mFAILED:\e[38;5;246m ");
            append_string(failedAssertsLog, fail);
            append_cstring(failedAssertsLog, "\e[0m\n");
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
}
