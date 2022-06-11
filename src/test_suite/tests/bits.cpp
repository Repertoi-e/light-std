#include "../test.h"

// u32
static_assert(msb(0b11101010100000000100000010001011ul) == 31);
static_assert(msb(0b01101010100000000100000010001011ul) == 30);
static_assert(msb(0b00101010100000000100000010001011ul) == 29);
static_assert(msb(0b00001010100000000100000010001011ul) == 27);
static_assert(msb(0b00000000000000000000000000000001ul) == 0);

static_assert(msb(12ul) == 3);

static_assert(lsb(0b11101010100000000100000000000000ul) == 14);
static_assert(lsb(0b01101010100000000100000010000000ul) == 7);
static_assert(lsb(0b00101010100000000100000010001000ul) == 3);
static_assert(lsb(0b00001010100000000100000010001010ul) == 1);
static_assert(lsb(0b00000000000000000000000000000001ul) == 0);

static_assert(lsb(12ul) == 2);

// u64 and u128
static_assert(msb(u128(0b1110101010000000010000001000101100000000000000000000000000000000ull, 0b0000000000000000000000000000000000000000000000000000000000000001ull)) == 127);
static_assert(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000001ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)) == 64);
static_assert(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000011ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)) == 65);
static_assert(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000000ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)) == 63);
static_assert(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000000ull, 0b0000101010000000010000001000101100000000000000000000000000000000ull)) == 59);

static_assert(lsb(u128(0b1110101010000000010000001000101100000000000000000000000000000000ull, 0b0000000000000000000000000000000000000000000000000000000000000001ull)) == 0);
static_assert(lsb(u128(0b0000000000000000000000000000000000000000000000000000000000000001ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)) == 32);
static_assert(lsb(u128(0b0000000000000000000000000000000000000000000000000000000000000011ull, 0b0000000000000000000000000000000000000000000000000000000000000000ull)) == 64);
static_assert(lsb(u128(0b1000000000000000000000000000000000000000000000000000000000000000ull, 0b0000000000000000000000000000000000000000000000000000000000000000ull)) == 127);

TEST(msb) {
    assert_eq(msb(0b11101010100000000100000010001011ul), 31);
    assert_eq(msb(0b01101010100000000100000010001011ul), 30);
    assert_eq(msb(0b00101010100000000100000010001011ul), 29);
    assert_eq(msb(0b00001010100000000100000010001011ul), 27);
    assert_eq(msb(0b00000000000000000000000000000001ul), 0);

    assert_eq(msb(12ul), 3);

    assert_eq(msb(u128(0b1110101010000000010000001000101100000000000000000000000000000000ull, 0b0000000000000000000000000000000000000000000000000000000000000001ull)), 127);
    assert_eq(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000001ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)), 64);
    assert_eq(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000011ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)), 65);
    assert_eq(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000000ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)), 63);
    assert_eq(msb(u128(0b0000000000000000000000000000000000000000000000000000000000000000ull, 0b0000101010000000010000001000101100000000000000000000000000000000ull)), 59);
}

TEST(lsb) {
    assert_eq(lsb(0b11101010100000000100000000000000ul), 14);
    assert_eq(lsb(0b01101010100000000100000010000000ul), 7);
    assert_eq(lsb(0b00101010100000000100000010001000ul), 3);
    assert_eq(lsb(0b00001010100000000100000010001010ul), 1);
    assert_eq(lsb(0b00000000000000000000000000000001ul), 0);

    assert_eq(lsb(12ul), 2);

    assert_eq(lsb(u128(0b1110101010000000010000001000101100000000000000000000000000000000ull, 0b0000000000000000000000000000000000000000000000000000000000000001ull)), 0);
    assert_eq(lsb(u128(0b0000000000000000000000000000000000000000000000000000000000000001ull, 0b1110101010000000010000001000101100000000000000000000000000000000ull)), 32);
    assert_eq(lsb(u128(0b0000000000000000000000000000000000000000000000000000000000000011ull, 0b0000000000000000000000000000000000000000000000000000000000000000ull)), 64);
    assert_eq(lsb(u128(0b1000000000000000000000000000000000000000000000000000000000000000ull, 0b0000000000000000000000000000000000000000000000000000000000000000ull)), 127);
}