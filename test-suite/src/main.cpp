#include "test.h"

void run_tests() {
    fmt::print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        fmt::print("{}:\n", *fileName);

        u32 sucessfulProcs = 0;
        For(*tests) {
            auto length = min<size_t>(30, it.Name.Length);
            fmt::print("        {:.{}} {:.^{}} ", it.Name, length, "", 35 - length);

            auto failedAssertsStart = asserts::GlobalFailed.Count;

            // Run the test
            if (it.Function) {
                it.Function();
            } else {
                fmt::print("{!RED}FAILED {!GRAY}(Function pointer is null){!}\n");
                continue;
            }

            // Check if test has failed asserts
            if (failedAssertsStart == asserts::GlobalFailed.Count) {
                fmt::print("{!GREEN}OK{!}\n");
                sucessfulProcs++;
            } else {
                fmt::print("{!RED}FAILED{!}\n");

                auto it = asserts::GlobalFailed.begin() + failedAssertsStart;
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
    fmt::print("\n{!}");

    asserts::GlobalCalledCount = 0;
    asserts::GlobalFailed.release();
}

s32 main() {
    time_t start = os_get_time();

    PUSH_CONTEXT(Alloc, Context.TemporaryAlloc) {
        while (true) {
            run_tests();
			Context.TemporaryAlloc.free_all();
            break;
        }
    }

    fmt::print("\nFinished tests, time taken: {:f} seconds\n\n", os_time_to_seconds(os_get_time() - start));

    return 0;
}