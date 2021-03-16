#include "test.h"

void run_tests() {
    print("\n");
    for (auto [fileName, tests] : g_TestTable) {
        print("{}:\n", *fileName);

        u32 sucessfulProcs = 0;
        For(*tests) {
            auto length = min(it.Name.Length, 30);
            print("        {:.{}} {:.^{}} ", it.Name, length, "", 35 - length);

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

                auto it = asserts::GlobalFailed.begin() + failedAssertsStart;
                for (; it != asserts::GlobalFailed.end(); ++it) {
                    print("          {!GRAY}>>> {}{!}\n", *it);
                }
                print("\n");
            }
        }

        f32 successRate = (f32) sucessfulProcs / (f32) tests->Count;
        print("{!GRAY}{:.2%} success ({} out of {} procs)\n{!}\n", successRate, sucessfulProcs, tests->Count);
    }
    print("\n\n");

    s64 calledCount = asserts::GlobalCalledCount;
    s64 failedCount = asserts::GlobalFailed.Count;
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

#define LOG_TO_FILE 0

#if LOG_TO_FILE
string_builder_writer logger;
void write_output_to_file() {
    Context.Log = &cout;

    auto out = file::handle("output.txt");
    out.write_to_file(combine(logger.Builder), file::handle::Overwrite_Entire);
}
#endif

s32 main() {
#if LOG_TO_FILE
    // logger.Builder.Alloc = Malloc;  // Otherwise we use the temporary alloc which gets freed after we run the tests

    // Context.LogAllAllocations = true;
    // Context.Log = &logger;

    // Context.FmtDisableAnsiCodes = true;
#endif
    time_t start = os_get_time();

#if defined DEBUG_MEMORY
    DEBUG_memory->MemoryVerifyHeapFrequency = 1;
#endif

    auto newContext = Context;
    newContext.AllocAlignment = 16;
    newContext.Alloc = Context.TempAlloc;

    allocator_add_pool(Context.TempAlloc, os_allocate_block(1_MiB), 1_MiB);

    PUSH_CONTEXT(newContext) {
        build_test_table();
        run_tests();
    }
    print("\nFinished tests, time taken: {:f} seconds, bytes used: {}, pools used: {}\n\n", os_time_to_seconds(os_get_time() - start), __TempAllocData.TotalUsed, __TempAllocData.PoolsCount);

#if LOG_TO_FILE
    // exit_schedule(write_output_to_file);
    write_output_to_file();
#endif

#if defined DEBUG_MEMORY
    DEBUG_memory->report_leaks();
#endif

    return 0;
}