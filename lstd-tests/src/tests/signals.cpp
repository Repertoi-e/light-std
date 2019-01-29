#include <lstd/signal/signal.hpp>

#include "../test.hpp"

static s32 my_callback(s32 a) { return a; }
static s32 my_callback1(s32 a) { return a + 1; }
static s32 my_callback2(s32 a) { return a + 2; }
static s32 my_callback3(s32 a) { return a + 3; }

TEST(global_function) {
    Signal<s32(s32), Collector_Array<s32>> signal;
    signal.connect(my_callback);
    signal.connect(my_callback1);
    signal.connect(my_callback2);
    signal.connect(my_callback3);

    assert_eq(signal.emit(20), to_array(20, 21, 22, 23));
}

struct Member_Test {
    s32 value = 10;
    s32 member_callback() { return value; }
};

TEST(member_function) {
    Signal<s32()> signal;

    Member_Test myStruct;
    signal.connect({&myStruct, &Member_Test::member_callback});
    assert_eq(signal.emit(), 10);
}
