#define GU_NO_CRT

#include <gu/memory/pool.h>
#include <gu/memory/stack.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

typedef void (*Test_Func)();
static Dynamic_Array<Test_Func> g_Tests;

#define TEST(name)                                   \
    struct Test_Struct_##name {                      \
        Test_Struct_##name() { add(g_Tests, &run); } \
        static void run();                           \
    };                                               \
    static Test_Struct_##name g_TestStruct_##name;   \
    void Test_Struct_##name::run()

TEST(Array) {
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

TEST(String) {
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

b32 g_CurrentTestFailed;

void run_tests() {
    auto testContext = *__context;
    testContext.AssertFailed = [](const char *file, int line, const char *failedCondition) {
        print("  %:% Assert failed: %\n", file, line, failedCondition);
        g_CurrentTestFailed = true;
    };
    PUSH_CONTEXT(testContext);

    int failedTests = 0;
    for (Test_Func func : g_Tests) {
        g_CurrentTestFailed = false;
        func();
        if (g_CurrentTestFailed) {
            print("  FAILED\n");
            failedTests = 0;
        } else {
            print("  PASSED\n");
        }
    }
    print("\n\n");

    if (failedTests) {
        print("run_tests finished with % failed tests.\n\n", failedTests);
    } else {
        print("run_tests finished with all tests passing.\n\n");
    }
}

int main() {
    // Setup the program's default context. Maybe move this to a utility
    // function so it is less error prone when we get to thread local storage?
    __context = New<Implicit_Context>(MALLOC);
    __context->Allocator = MALLOC;
    __context->Log = print_string_to_console;
    __context->AssertFailed = default_failed_assert;

    temporary_storage_init(4_MiB);

    auto tempContext = *__context;
    tempContext.Allocator = TEMPORARY_ALLOC;
    {
        PUSH_CONTEXT(tempContext);
        TEMPORARY_STORAGE_MARK_SCOPE;

        Table<string, s32> table;
        put(table, "TwentySomething", 20);
        put(table, "Thirty", 30);
        put(table, "TwentySomething", 25);

        auto [twenty, twentyFound] = find(table, "TwentySomething");
        auto [thirty, thirtyFound] = find(table, "Thirty");

        print("Found: %: %\n", twentyFound, twenty);
        print("Found: %: %\n", thirtyFound, thirty);

        run_tests();
    }
}
