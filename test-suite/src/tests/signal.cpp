#include <lstd/memory/signal.h>

#include "../test.h"

file_scope s32 my_callback(s32 a) { return a; }
file_scope s32 my_callback1(s32 a) { return a + 1; }
file_scope s32 my_callback2(s32 a) { return a + 2; }
file_scope s32 my_callback3(s32 a) { return a + 3; }

TEST(global_function) {
    signal<s32(s32), collector_array<s32>> signal;
    signal.connect(my_callback);
    signal.connect(my_callback1);
    signal.connect(my_callback2);
    signal.connect(my_callback3);

    array<s32> result = signal.emit(20);
    assert_eq(result, to_stack_array<s32>(20, 21, 22, 23));
    free(result);

    signal.release();
}

struct Member_Test {
    s32 value = 10;
    s32 member_callback(s32 i) const {
        s32 j = 42;
        return i + value;
    }
};

TEST(member_function) {
    signal<s32(s32)> signal;

    Member_Test myStruct;
    signal.connect({&myStruct, &Member_Test::member_callback});

    s32 result = signal.emit(20);

    assert_eq(result, myStruct.value + 20);
    signal.release();
}

TEST(global_function_delegate) {
    delegate<s32(s32)> delegate0 = my_callback;
    delegate<s32(s32)> delegate1 = my_callback1;
    delegate<s32(s32)> delegate2 = my_callback2;
    delegate<s32(s32)> delegate3 = my_callback3;

    assert_eq(delegate0(20), 20);
    assert_eq(delegate1(20), 21);
    assert_eq(delegate2(20), 22);
    assert_eq(delegate3(20), 23);
}

TEST(member_function_delegate) {
    Member_Test myStruct;
    auto delegate0 = delegate<s32(s32)>(&myStruct, &Member_Test::member_callback);
    assert_eq(delegate0(20), myStruct.value + 20);
}

struct functor_test {
    s32 i = 0;

    s32 operator()() {
        i = 20;
        return i;
    }
};

TEST(functor_delegate) {
    functor_test functor;

    auto delegate0 = delegate<s32()>(&functor);
    assert_eq(delegate0(), functor.i);
}
