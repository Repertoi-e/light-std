#include "../test.h"

#include <cppu/format/fmt.h>

struct Custom_Type {};

template <>
struct fmt::Formatter<Custom_Type> {
    string_view::Iterator parse(const Parse_Context &parseContext) { return parseContext.It; }

    void format(Custom_Type, Format_Context &f) { f.Out.append("foo"); }
};

TEST(core) {
    assert(fmt::sprint("{}", 42) == "42");
    assert(fmt::sprint("{}", string("foo")) == "foo");
    assert(fmt::sprint("{}", string_view("foo")) == "foo");

    assert(fmt::sprint("{}", false) == "false");
    assert(fmt::sprint("{}", true) == "true");

    assert(fmt::sprint("{}", Custom_Type{}) == "foo");

    string result = fmt::sprint("{}, {}, {}", 1, 2, 6);
    assert(result == "1, 2, 6");
}

TEST(positional_arguments) {
    assert(fmt::sprint("{0}, {1}, {2}", 1, 2, 6) == "1, 2, 6");
    assert(fmt::sprint("{2}, {0}, {1}", 1, 2, 6) == "6, 1, 2");
    assert(fmt::sprint("{1}, {2}, {0}", 1, 2, 6) == "2, 6, 1");
}

TEST(named_arguments) {
    assert(fmt::sprint("{first}, {second}, {third}", "first"_a = 1, "second"_a = 2, "third"_a = 6) == "1, 2, 6");
    assert(fmt::sprint("{second}, {first}, {third}", "first"_a = 1, "second"_a = 2, "third"_a = 6) == "2, 1, 6");
    assert(fmt::sprint("{third}, {second}, {first}", "first"_a = 1, "second"_a = 2, "third"_a = 6) == "6, 2, 1");
}

TEST(numeric) {
    string result = fmt::sprint("int: {0:d};  hex: {0:x};  oct: {0:o}; bin: {0:b}", 42);
    assert(result == "int: 42;  hex: 2a;  oct: 52; bin: 101010");
    result = fmt::sprint("int: {0:d};  hex: {0:#x};  oct: {0:#o};  bin: {0:#b}", 42);
    assert(result == "int: 42;  hex: 0x2a;  oct: 052;  bin: 0b101010");
}

TEST(to_string) {
    assert(fmt::to_string(42) == "42");
    assert(fmt::to_string(string("foo")) == "foo");
    assert(fmt::to_string(string_view("foo")) == "foo");

    assert(fmt::to_string(false) == "false");
    assert(fmt::to_string(true) == "true");

    assert(fmt::to_string(Custom_Type{}) == "foo");
}
