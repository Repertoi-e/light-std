#include <lstd/io/reader.hpp>

#include "../test.hpp"

TEST(bytes_and_codepoints) {
    io::string_reader in(" 1 2   3");
    assert_eq(in.read_codepoint(), '1');
    assert_eq(in.read_codepoint(), '2');
    assert_eq(in.read_codepoint(), '3');
    in.Exhausted = false;

    assert_eq(in.read_codepoint(true), ' ');
    assert_eq(in.read_codepoint(true), '1');
    assert_eq(in.read_codepoint(true), ' ');

    char byte;
    in.read(byte);
    assert_eq(byte, '2');
}

TEST(bools) {
    io::string_reader in("0 1 true false TRUE fALsE tRue");
    dynamic_array<bool> results;
    while (!in.EOF) {
        bool value;
        in.read(value);
        if (!in.FailedParse) results.append(value);
    }
    assert_eq(results, to_array(false, true, true, false, true, false, true));
}

TEST(integers) {
    io::string_reader in("-2305 2050 10 -0xff 0xff 0202 -240");
    dynamic_array<s32> results;
    while (!in.EOF) {
        s32 value;
        in.read(value);
        if (!in.FailedParse) results.append(value);
    }
    assert_eq(results, to_array(-2305, 2050, 10, -0xff, 0xff, 0202, -240));
}

TEST(floats) {
    io::string_reader in("-2305.02 2050.02502 10e10 -520.20501 5.2e2");
    dynamic_array<f64> results;
    while (!in.EOF) {
        f64 value;
        in.read(value);
        if (!in.FailedParse) results.append(value);
    }
    assert_eq(results, to_array(-2305.02, 2050.02502, 10e10, -520.20501, 5.2e2));
}

struct custom_int {
    s32 v = 0;
};

template <>
struct io::deserializer<custom_int> {
    bool read(custom_int &value, reader &reader) const {
        reader.read(value.v);
        return !reader.FailedParse;
    }
};

TEST(custom_types) {
    io::string_reader in("42");
    custom_int myType;
    in.read(myType);
    assert_eq(myType.v, 42);
}