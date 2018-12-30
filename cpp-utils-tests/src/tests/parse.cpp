#include <cppu/io/reader.hpp>

#include "../test.hpp"

TEST(bytes_and_codepoints) {
    io::String_Reader in(" 1 2   3");
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
    io::String_Reader in("0 1 true false TRUE fALsE tRue");
    Dynamic_Array<bool> results;
    while (!in.EOF) {
        bool value;
        in.read(value);
        if (!in.FailedParse) results.add(value);
    }
    assert_eq(results, to_array(false, true, true, false, true, false, true));
}

TEST(integers) {
    io::String_Reader in("-2305 2050 10 -0xff 0xff 0202 -240");
    Dynamic_Array<s32> results;
    while (!in.EOF) {
        s32 value;
        in.read(value);
        if (!in.FailedParse) results.add(value);
    }
    assert_eq(results, to_array(-2305, 2050, 10, -0xff, 0xff, 0202, -240));
}

TEST(floats) {
    io::String_Reader in("-2305.02 2050.02502 10e10 -520.20501 5.2e2");
    Dynamic_Array<f64> results;
    while (!in.EOF) {
        f64 value;
        in.read(value);
        if (!in.FailedParse) results.add(value);
    }
    assert_eq(results, to_array(-2305.02, 2050.02502, 10e10, -520.20501, 5.2e2));
}