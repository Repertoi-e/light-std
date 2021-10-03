#include "../test.h"

file_scope s32 my_callback(s32 a) { return a; }
file_scope s32 my_callback1(s32 a) { return a + 1; }
file_scope s32 my_callback2(s32 a) { return a + 2; }
file_scope s32 my_callback3(s32 a) { return a + 3; }

struct Member_Test {
    s32 value = 10;

    s32 member_callback(s32 i) const {
        s32 j = 42;
        return i + value;
    }
};

/*
TEST(member_function) {
    signal<void(s32)> s{};

    Member_Test myStruct;
    connect(&s, {&myStruct, &Member_Test::member_callback});

    emit(&s, 20);
    free_signal(&s);
}*/

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
