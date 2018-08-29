#define GU_NO_CRT

#include <gu/memory/pool.h>
#include <gu/memory/stack.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "test.h"

TEST(arrays) {
    Dynamic_Array<int> integers;
    for (int i = 0; i < 10; i++) {
        add(integers, i);
    }

    {
        String_Builder builder;
        for (int a : integers) {
            append_string(builder, tprint("%, ", a));
        }
        assert(to_string(builder) == "0, 1, 2, 3, 4, 5, 6, 7, 8, 9, ");
    }
    {
        insert(integers, begin(integers) + 3, -3);

        String_Builder builder;
        for (int a : integers) {
            append_string(builder, tprint("%, ", a));
        }
        assert(to_string(builder) == "0, 1, 2, -3, 3, 4, 5, 6, 7, 8, 9, ");
    }
    {
        remove(integers, begin(integers) + 4);

        String_Builder builder;
        for (int a : integers) {
            append_string(builder, tprint("%, ", a));
        }
        assert(to_string(builder) == "0, 1, 2, -3, 4, 5, 6, 7, 8, 9, ");
    }
}

TEST(strings) {
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";
        string result = a + b + c;

        assert(result == "Hello, world!");
    }
    {
        String_Builder builder;
        append_cstring(builder, "Hello");
        append_cstring_and_size(builder, ",THIS IS GARBAGE", 1);
        append_string(builder, string(" world!"));
        string result = to_string(builder);

        assert(result == "Hello, world!");
    }

    assert(to_string(2.40) == "2.400000");
    assert(to_string(2.12359012385, 0, 3) == "2.124");
    assert(to_string(123512.1241242222222222, 8, 9) == "123512.124124222");
    assert(to_string(22135.42350, 20, 1) == "             22135.4");
    assert(to_string(2.40, 21, 2) == "                 2.40");
    assert(to_string(2.40, 10, 0) == "         2");
    assert(to_string(2.40, 10, 1) == "       2.4");

    assert(to_string(1024, Base(2)) == "10000000000");
    assert(to_string(std::numeric_limits<u8>::max(), Base(10), 30) == "000000000000000000000000000255");
    assert(to_string(std::numeric_limits<u8>::max(), Base(2)) == "11111111");

    assert(to_string(std::numeric_limits<s64>::min(), Base(10)) == "-9223372036854775808");
    assert(to_string(std::numeric_limits<s64>::min(), Base(2), 30) ==
           "-1000000000000000000000000000000000000000000000000000000000000000");

    assert(to_string(std::numeric_limits<u64>::max(), Base(10)) == "18446744073709551615");
    assert(to_string(std::numeric_limits<u64>::max(), Base(2), 30) ==
           "1111111111111111111111111111111111111111111111111111111111111111");

    assert(to_string("Hello, world!", 0) == "Hello, world!");

    assert(to_string("Hello, world!", 20) == "Hello, world!       ");
    assert(to_string("Hello, world!", 13) == "Hello, world!");
    assert(to_string("Hello, world!", 12) == "Hello, wo...");
    assert(to_string("Hello, world!", 4) == "H...");
    assert(to_string("Hello, world!", 3) == "...");
    assert(to_string("Hello, world!", 2) == "..");
    assert(to_string("Hello, world!", 1) == ".");
    assert(to_string("Hello, world!", -1) == ".");
    assert(to_string("Hello, world!", -2) == "..");
    assert(to_string("Hello, world!", -3) == "...");
    assert(to_string("Hello, world!", -4) == "...!");
    assert(to_string("Hello, world!", -12) == "...o, world!");
    assert(to_string("Hello, world!", -13) == "Hello, world!");
    assert(to_string("Hello, world!", -20) == "       Hello, world!");

    string print1 = tprint("My name is %2 and my bank interest is %1%%.", to_string(152.29385, 0, 2), "Dotra");
    string print2 = tprint("My name is % and my bank interest is %2%%.", "Dotra", to_string(152.29385, 0, 2));

    assert(print1 == "My name is Dotra and my bank interest is 152.29%.");
    assert(print2 == "My name is Dotra and my bank interest is 152.29%.");
}

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
                add(g_CurrentTestFailedAsserts, sprint("%:% Assert failed: %\n", shortFileName, line, failedCondition));
                g_TotalFailedAsserts++;
            }
            g_TotalAsserts++;
        } else {
            // If
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
                // Add the failed asserts to the array of all faile asserts
                // that gets printed at the end.
                for (string &fail : g_CurrentTestFailedAsserts) {
                    add(g_AllFailedAsserts, fail);
                }
                release(g_CurrentTestFailedAsserts);

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
                    failedProcs++;
                } else {
                    print("\e[38;5;28mOK\e[0m\n");
                }
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

    String_Builder failedAssertsLog;
    for (string &fail : g_CurrentTestFailedAsserts) {
        append_cstring(failedAssertsLog, "        >>> \e[38;5;160mFAILED:\e[38;5;246m ");
        append_string(failedAssertsLog, fail);
        append_cstring(failedAssertsLog, "\n");
    }
    print(to_string(failedAssertsLog));
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
