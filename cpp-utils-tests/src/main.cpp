
#include <cppu/memory/pool.h>
#include <cppu/memory/table.h>

#include "test.h"

// VS has a bug which prevents inline variables in headers constructing properly...
Table<string_view, Dynamic_Array<Test> *> g_TestTable;
u32 Asserts::GlobalCalledCount = 0;
Dynamic_Array<string> Asserts::GlobalFailed;

void run_tests() {
    fmt::print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        fmt::print("{}:\n", fileName);

        u32 sucessfulProcs = 0;
        for (const auto &test : *tests) {
            s32 length = Min(30, (s32) test.Name.Length);
            fmt::print("        {:.{}} {:.{}} ", test.Name, length, string(".").repeated(35), 35 - length);

            size_t failedIndexStart = Asserts::GlobalFailed.Count;

            // Run the test
            test.Function();

            // Check if test has failed asserts
            if (failedIndexStart == Asserts::GlobalFailed.Count) {
                // No failed asserts!
                // TODO: We should have a console text formatting library...
                fmt::print("\033[38;5;28mOK\033[0m\n");
                sucessfulProcs++;
            } else {
                // TODO: We should have a console text formatting library...
                fmt::print("\033[38;5;160mFAILED\033[0m\n");

                auto it = Asserts::GlobalFailed.begin() + failedIndexStart;
                for (; it != Asserts::GlobalFailed.end(); ++it) {
                    fmt::print("          \033[38;5;246m>>> {}\033[0m\n", *it);
                }
                fmt::print("\n");
            }
        }

        f32 successRate = (f32) sucessfulProcs / (f32) tests->Count * 100.0f;
        // TODO: We should have a console text formatting library...
        fmt::print("\033[38;5;246m{:.2}% success ({} out of {} procs)\n\033[0m\n", successRate, sucessfulProcs,
                   tests->Count);
    }
    fmt::print("\n\n");

    u32 calledCount = Asserts::GlobalCalledCount;
    size_t failedCount = Asserts::GlobalFailed.Count;
    size_t successCount = calledCount - failedCount;

    fmt::print("[Test Suite] {:.3}% success ({}/{} test asserts)\n",
               calledCount ? 100.0f * (f32) successCount / (f32) calledCount : 0.0f, successCount, calledCount);

    if (failedCount) {
        fmt::print("[Test Suite] Failed asserts:\n");
        for (const string &message : Asserts::GlobalFailed) {
            fmt::print("    >>> \033[38;5;160mFAILED:\033[38;5;246m {}\033[0m\n", message);
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
}
