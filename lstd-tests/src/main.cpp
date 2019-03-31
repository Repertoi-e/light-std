#include <lstd/io/reader.hpp>

#include <lstd/memory/pool.hpp>
#include <lstd/memory/table.hpp>

#include "test.hpp"

// VS has a bug which prevents inline variables in headers constructing properly...
table<string_view, dynamic_array<test> *> g_TestTable;
size_t asserts::GlobalCalledCount = 0;
dynamic_array<string> asserts::GlobalFailed;

void run_tests() {
    fmt::print("\n");
    for (const auto &[fileName, tests] : g_TestTable) {
        fmt::print("{}:\n", fileName);

        u32 sucessfulProcs = 0;
        For(*tests) {
            s32 length = min(30, (s32) it.Name.Length);
            fmt::print("        {:.{}} {:.{}} ", it.Name, length, string(".").repeated(35), 35 - length);

            size_t failedIndexStart = asserts::GlobalFailed.Count;

            // Run the test
            it.Function();

            // Check if test has failed asserts
            if (failedIndexStart == asserts::GlobalFailed.Count) {
                // No failed asserts!
                print("{}OK{}\n", fmt::fgb::Green, fmt::fg::Reset);
                sucessfulProcs++;
            } else {
                print("{}FAILED{}\n", fmt::fgb::Red, fmt::fg::Reset);

                auto it = asserts::GlobalFailed.begin() + failedIndexStart;
                for (; it != asserts::GlobalFailed.end(); ++it) {
                    print("          {}>>> {}{}\n", fmt::fgb::Gray, *it, fmt::fg::Reset);
                }
                fmt::print("\n");
            }
        }

        f32 successRate = (f32) sucessfulProcs / (f32) tests->Count * 100.0f;
        print("{}{:.2}% success ({} out of {} procs)\n{}\n", fmt::fg::Gray, successRate, sucessfulProcs, tests->Count,
              fmt::fg::Reset);
    }
    fmt::print("\n\n");

    size_t calledCount = asserts::GlobalCalledCount;
    size_t failedCount = asserts::GlobalFailed.Count;
    size_t successCount = calledCount - failedCount;

    fmt::print("[Test Suite] {:.3}% success ({}/{} test asserts)\n",
               calledCount ? 100.0f * (f32) successCount / (f32) calledCount : 0.0f, successCount, calledCount);

    if (failedCount) {
        fmt::print("[Test Suite] Failed asserts:\n");
        For(asserts::GlobalFailed) {
            print("    >>> {}FAILED:{} {}{}\n", fmt::fg::Red, fmt::fgb::Gray, it, fmt::fg::Reset);
        }
    }
    fmt::print("\n");
}

int main() {
    temporary_storage_init(4_MiB);

    PUSH_CONTEXT(Allocator, TEMPORARY_ALLOC) {
        // while (true) {
        //     run_tests();
        //     temporary_storage_reset();
        // }
        run_tests();
    }
}
