#include "test.h"

import lstd.os;

void run_tests() {
	reserve(asserts::GlobalFailed);

    print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        print("{}:\n", *fileName);

        u32 sucessfulProcs = 0;
        For(*tests) {
            auto len = min(length(it.Name), 30);
            print("        {:.{}} {:.^{}} ", it.Name, len, "", 35 - len);

            auto failedAssertsStart = asserts::GlobalFailed.Count;

            // Run the test
            if (it.Function) {
                it.Function();
            } else {
                print("{!RED}FAILED {!GRAY}(Function pointer is null){!}\n");
                continue;
            }

            // Check if test has failed asserts
            if (failedAssertsStart == asserts::GlobalFailed.Count) {
                print("{!GREEN}OK{!}\n");
                sucessfulProcs++;
            } else {
                print("{!RED}FAILED{!}\n");

                For(range(failedAssertsStart, asserts::GlobalFailed.Count)) {
                    print("          {!GRAY}>>> {}{!}\n", asserts::GlobalFailed[it]);
                }
                print("\n");
            }
        }

        f32 successRate = (f32) sucessfulProcs / (f32) tests->Count;
        print("{!GRAY}{:.2%} success ({} out of {} procs)\n{!}\n", successRate, sucessfulProcs, tests->Count);
    }
    print("\n\n");

    s64 calledCount  = asserts::GlobalCalledCount;
    s64 failedCount  = asserts::GlobalFailed.Count;
    s64 successCount = calledCount - failedCount;

    f32 successRate = calledCount ? (f32) successCount / (f32) calledCount : 0.0f;
    print("[Test Suite] {:.3%} success ({}/{} test asserts)\n", successRate, successCount, calledCount);

    if (failedCount) {
        print("[Test Suite] Failed asserts:\n");
        For(asserts::GlobalFailed) { print("    >>> {!RED}FAILED:{!GRAY} {}{!}\n", it); }
    }
    print("\n{!}");

    // These need to be reset in case we rerun the tests (we may spin this function up in a while loop a bunch of times when looking for rare bugs).
    asserts::GlobalCalledCount = 0;
    free(asserts::GlobalFailed);
}

const bool LOG_TO_FILE = false;

string_builder_writer g_Logger;

void write_output_to_file() {
    auto newContext = Context;
    newContext.Log  = &cout;
    OVERRIDE_CONTEXT(newContext);

    os_write_to_file("output.txt", builder_to_string(g_Logger.Builder), file_write_mode::Overwrite_Entire);  // @Leak
}

s32 main() {
    time_t start = os_get_time();

    TemporaryAllocatorData.Block = os_allocate_block(1_MiB);
    TemporaryAllocatorData.Size  = 1_MiB;

    auto newContext           = Context;
    newContext.Alloc          = TemporaryAllocator;
    newContext.AllocAlignment = 16;
#if defined DEBUG_MEMORY
    // newContext.DebugMemoryHeapVerifyFrequency = 1;
#endif

    if (LOG_TO_FILE) {
        newContext.LogAllAllocations = true;
        newContext.Log               = &g_Logger;

        newContext.FmtDisableAnsiCodes = true;
    }

    OVERRIDE_CONTEXT(newContext);

    build_test_table();
    run_tests();

    print("\nFinished tests, time taken: {:f} seconds, bytes used: {}\n\n", os_time_to_seconds(os_get_time() - start), TemporaryAllocatorData.Used);

    if (LOG_TO_FILE) {
        write_output_to_file();
    }

#if defined DEBUG_MEMORY
    // debug_memory_report_leaks();
#endif

    return 0;
}
