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

    auto a = signal.emit(20);

    // Signals are emmitted to connections backwards
    assert_eq(signal.emit(20), to_array<s32>(23, 22, 21, 20));
}

struct Member_Test {
    s32 value = 10;
    s32 member_callback(s32 i) {
        s32 j = 42;
        return i + value;
    }
};

TEST(member_function) {
    Signal<s32(s32)> signal;

    Member_Test myStruct;
    signal.connect({&myStruct, &Member_Test::member_callback});
    assert_eq(signal.emit(20), myStruct.value + 20);
}

TEST(global_function_delegate) {
    Delegate<s32(s32)> delegate = my_callback;
    Delegate<s32(s32)> delegate1 = my_callback1;
    Delegate<s32(s32)> delegate2 = my_callback2;
    Delegate<s32(s32)> delegate3 = my_callback3;

    assert_eq(delegate(20), 20);
    assert_eq(delegate1(20), 21);
    assert_eq(delegate2(20), 22);
    assert_eq(delegate3(20), 23);
}

TEST(member_function_delegate) {
    Member_Test myStruct;
    auto delegate = Delegate<s32(s32)>(&myStruct, &Member_Test::member_callback);
    assert_eq(delegate(20), myStruct.value + 20);
}

TEST(lambda_delegate) {
    s32 i = 0;
    auto delegate = Delegate<s32()>([&]() {
        i = 20;
        return i;
    });
    assert_eq(delegate(), i);
}