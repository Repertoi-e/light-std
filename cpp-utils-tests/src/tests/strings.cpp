#include <gu/memory/pool.h>
#include <gu/memory/stack.h>
#include <gu/memory/table.h>

#include <gu/string/print.h>

#include "../test.h"

TEST(string_concat) {
    {
        string a = "Hello";
        append_pointer_and_size(a, ",THIS IS GARBAGE", 1);
        append_cstring(a, " world!");

        assert(a == "Hello, world!");
    }
    {
        string a = "Hello";
        string b = ",";
        string c = " world!";
        string result = a + b + c;

        assert(result == "Hello, world!");
    }
}

TEST(string_builder) {
    String_Builder builder;
    append_cstring(builder, "Hello");
    append_pointer_and_size(builder, ",THIS IS GARBAGE", 1);
    append(builder, string(" world!"));

    string result = to_string(builder);
    assert(result == "Hello, world!");
}

TEST(format_string) {
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
}

TEST(format_float) {
    assert(to_string(2.40) == "2.400000");
    assert(to_string(2.12359012385, 0, 3) == "2.124");
    assert(to_string(123512.1241242222222222, 8, 9) == "123512.124124222");
    assert(to_string(22135.42350, 20, 1) == "             22135.4");
    assert(to_string(2.40, 21, 2) == "                 2.40");
    assert(to_string(2.40, 10, 0) == "         2");
    assert(to_string(2.40, 10, 1) == "       2.4");
}

TEST(format_integer) {
    assert(to_string(1024, Base(2)) == "10000000000");
    assert(to_string(std::numeric_limits<u8>::max(), Base(10), 30) == "000000000000000000000000000255");
    assert(to_string(std::numeric_limits<u8>::max(), Base(2)) == "11111111");

    assert(to_string(std::numeric_limits<s64>::min(), Base(10)) == "-9223372036854775808");
    assert(to_string(std::numeric_limits<s64>::min(), Base(2), 30) ==
           "-1000000000000000000000000000000000000000000000000000000000000000");

    assert(to_string(std::numeric_limits<u64>::max(), Base(10)) == "18446744073709551615");
    assert(to_string(std::numeric_limits<u64>::max(), Base(2), 30) ==
           "1111111111111111111111111111111111111111111111111111111111111111");
}

TEST(sprint) {
    string print1 = tprint("My name is %2 and my bank interest is %1%%.", to_string(152.29385, 0, 2), "Dotra");
    string print2 = tprint("My name is % and my bank interest is %2%%.", "Dotra", to_string(152.29385, 0, 2));

    assert(print1 == "My name is Dotra and my bank interest is 152.29%.");
    assert(print2 == "My name is Dotra and my bank interest is 152.29%.");
}
