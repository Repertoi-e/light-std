#define GU_NO_CRT

#include <gu/memory/pool.h>
#include <gu/memory/stack.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "test.h"

// !!! WORKAORUND
// Look in test.h
Table<string, Dynamic_Array<Test> *> g_TestTable;


string g_CurrentTestFile;
u32 g_TotalAsserts = 0, g_TotalFailedAsserts = 0;
Dynamic_Array<string> g_CurrentTestFailedAsserts, g_AllFailedAsserts;
u32 g_MaxFailedAsserts;

void run_tests() {
    // Copy the current context
    auto testContext = __context;

    // Add our own assert handler so we can save some stats and logs
    testContext.AssertHandler = [](bool failed, const char *file, int line, const char *failedCondition) {
        string shortFileName = get_file_path_relative_to_src_or_just_file_name(file);

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
                    print("\033[38;5;160mFAILED\033[0m\n");
                    for (string &fail : g_CurrentTestFailedAsserts) {
                        print("          \033[38;5;246m>>> %\033[0m\n", fail);
                    }
                    print("\n");
                    failedProcs++;
                } else {
                    print("\033[38;5;28mOK\033[0m\n");
                }

                // Add the failed asserts to the array of all faile asserts
                // that gets printed at the end of the test suite.
                for (string &fail : g_CurrentTestFailedAsserts) {
                    add(g_AllFailedAsserts, fail);
                }
                release(g_CurrentTestFailedAsserts);
            }
            print("\033[38;5;246m%1%% success (% out of % procs)\n\033[0m\n",
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

	// These variables are used in run_tests, which uses another
	// allocator than the one these use when getting freed at the 
	// end of the program (which causes a crash because you can't
	// mix and match allocator functions) so we override that by
	// setting their internal allocator to the one that is used at
	// the end of the program to free them (malloc).
	g_CurrentTestFile.Allocator = CONTEXT_ALLOC;
	g_CurrentTestFailedAsserts.Allocator = CONTEXT_ALLOC;
	g_AllFailedAsserts.Allocator = CONTEXT_ALLOC;

    auto tempContext = __context;
    tempContext.Allocator = TEMPORARY_ALLOC;
    {
        PUSH_CONTEXT(tempContext);
        TEMPORARY_STORAGE_MARK_SCOPE;

        run_tests();
    }
}
