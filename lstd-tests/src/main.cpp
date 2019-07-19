#include "test.h"

#include <ctime>

void run_tests() {
    fmt::print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        fmt::print("{}:\n", *fileName);

        u32 sucessfulProcs = 0;
        For(*tests) {
            auto length = MIN<size_t>(30, it.Name.Length);
            fmt::print("        {:.{}} {:.^{}} ", it.Name, length, "", 35 - length);

            size_t failedIndexStart = asserts::GlobalFailed.Count;

            // Run the test
            if (it.Function) {
                it.Function();
            } else {
                fmt::print("{!RED}FAILED {!GRAY}(Function pointer is null){!}\n");
                continue;
            }

            // Check if test has failed asserts
            if (failedIndexStart == asserts::GlobalFailed.Count) {
                // No failed asserts!
                fmt::print("{!GREEN}OK{!}\n");
                sucessfulProcs++;
            } else {
                fmt::print("{!RED}FAILED{!}\n");

                auto it = asserts::GlobalFailed.begin() + failedIndexStart;
                for (; it != asserts::GlobalFailed.end(); ++it) {
                    fmt::print("          {!GRAY}>>> {}{!}\n", *it);
                }
                fmt::print("\n");
            }
        }

        f32 successRate = (f32) sucessfulProcs / (f32) tests->Count;
        fmt::print("{!GRAY}{:.2%} success ({} out of {} procs)\n{!}\n", successRate, sucessfulProcs, tests->Count);
    }
    fmt::print("\n\n");

    size_t calledCount = asserts::GlobalCalledCount;
    size_t failedCount = asserts::GlobalFailed.Count;
    size_t successCount = calledCount - failedCount;

    f32 successRate = calledCount ? (f32) successCount / (f32) calledCount : 0.0f;
    fmt::print("[Test Suite] {:.3%} success ({}/{} test asserts)\n", successRate, successCount, calledCount);

    if (failedCount) {
        fmt::print("[Test Suite] Failed asserts:\n");
        For(asserts::GlobalFailed) { fmt::print("    >>> {!RED}FAILED:{!GRAY} {}{!}\n", it); }
    }
    fmt::print("\n");

    asserts::GlobalCalledCount = 0;
    asserts::GlobalFailed.release();
}

int main() {
    Context.init_temporary_allocator(4_MiB);

    while (true) {
        run_tests();
        break;
        // Context.TemporaryAlloc.free_all();
    }
}