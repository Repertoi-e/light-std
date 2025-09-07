#include "../test.h"

#include "lstd/variant.h"

TEST(variant_basic_construction) {
    // Test default construction (empty variant)
    variant<int, float, string> v;
    assert_false(v);  // Empty variant should be false

    // Test construction with int
    variant<int, float> v1(42);
    assert_true(v1);
    assert_true(v1.is<int>());
    assert_false(v1.is<float>());
    assert_eq(v1.strict_get<int>(), 42);

    // Test construction with float
    variant<int, float> v2(3.14f);
    assert_true(v2);
    assert_true(v2.is<float>());
    assert_false(v2.is<int>());
    assert_eq(v2.strict_get<float>(), 3.14f);
}

TEST(variant_assignment) {
    variant<int, float, string> v;
    
    // Assign int
    v = 42;
    assert_true(v.is<int>());
    assert_eq(v.strict_get<int>(), 42);
    
    // Assign float
    v = 3.14f;
    assert_true(v.is<float>());
    assert_eq(v.strict_get<float>(), 3.14f);
    
    // Assign string
    v = string("hello");
    assert_true(v.is<string>());
    assert_eq_str(v.strict_get<string>(), "hello");
}

TEST(variant_copy_constructor) {
    variant<int, string> v1(42);
    variant<int, string> v2 = v1;
    
    assert_true(v2.is<int>());
    assert_eq(v2.strict_get<int>(), 42);
    
    // Original should still be valid
    assert_true(v1.is<int>());
    assert_eq(v1.strict_get<int>(), 42);
}

TEST(variant_emplace) {
    variant<int, string> v;
    
    // Emplace int
    v.emplace<int>(100);
    assert_true(v.is<int>());
    assert_eq(v.strict_get<int>(), 100);
    
    // Emplace string
    v.emplace<string>("world");
    assert_true(v.is<string>());
    assert_eq_str(v.strict_get<string>(), "world");
}

TEST(variant_visit_pattern_matching) {
    variant<int, float, string> v;
    
    // Test with int
    v = 42;
    string result;
    v.visit(match {
        [&result](int x) { result = sprint("int: {}", x); },
        [&result](float x) { result = sprint("float: {}", x); },
        [&result](string x) { result = sprint("string: {}", x); },
        [&result](auto) { result = "empty"; }
    });
    assert_eq_str(result, "int: 42");
    
    // Test with float
    v = 3.14f;
    v.visit(match {
        [&result](int x) { result = sprint("int: {}", x); },
        [&result](float x) { result = sprint("float: {}", x); },
        [&result](string x) { result = sprint("string: {}", x); },
        [&result](auto) { result = "empty"; }
    });
    assert_eq_str(result, "float: 3.14");
    
    // Test with string
    v = string("hello");
    v.visit(match {
        [&result](int x) { result = sprint("int: {}", x); },
        [&result](float x) { result = sprint("float: {}", x); },
        [&result](string x) { result = sprint("string: {}", x); },
        [&result](auto) { result = "empty"; }
    });
    assert_eq_str(result, "string: hello");
    
    // Test empty variant
    v = variant<int, float, string>{};
    v.visit(match {
        [&result](int x) { result = sprint("int: {}", x); },
        [&result](float x) { result = sprint("float: {}", x); },
        [&result](string x) { result = sprint("string: {}", x); },
        [&result](auto) { result = "empty"; }
    });
    assert_eq_str(result, "empty");
}

TEST(variant_const_operations) {
    const variant<int, string> v(42);
    
    assert_true(v.is<int>());
    assert_false(v.is<string>());
    assert_eq(v.strict_get<int>(), 42);
    
    // Test const visit
    string result;
    v.visit(match {
        [&result](int x) { result = sprint("const int: {}", x); },
        [&result](string x) { result = sprint("const string: {}", x); },
        [&result](auto) { result = "const empty"; }
    });
    assert_eq_str(result, "const int: 42");
}

TEST(optional_basic) {
    // optional<T> is just variant<T>
    optional<int> opt;
    assert_false(opt);
    assert_true(opt.is<optional<int>::nil>());
    
    // Assign value
    opt = 42;
    assert_true(opt);
    assert_true(opt.is<int>());
    assert_eq(opt.strict_get<int>(), 42);
    
    // Reset to empty
    opt = optional<int>{};
    assert_false(opt);
}

TEST(optional_with_string) {
    optional<string> opt;
    assert_false(opt);
    
    opt = string("hello world");
    assert_true(opt);
    assert_eq_str(opt.strict_get<string>(), "hello world");
    
    // Test pattern matching with optional
    string result;
    opt.visit(match {
        [&result](string s) { result = sprint("got: {}", s); },
        [&result](auto) { result = "empty"; }
    });
    assert_eq_str(result, "got: hello world");
    
    // Clear and test again
    opt = optional<string>{};
    opt.visit(match {
        [&result](string s) { result = sprint("got: {}", s); },
        [&result](auto) { result = "empty"; }
    });
    assert_eq_str(result, "empty");
}

TEST(variant_complex_types) {
    struct point {
        int x, y;
        point(int x, int y) : x(x), y(y) {}
        bool operator==(const point& other) const {
            return x == other.x && y == other.y;
        }
    };
    
    variant<int, point, string> v;
    
    // Test with custom struct
    v.emplace<point>(10, 20);
    assert_true(v.is<point>());
    
    auto p = v.strict_get<point>();
    assert_eq(p.x, 10);
    assert_eq(p.y, 20);
    
    // Test pattern matching with custom type
    bool found_point = false;
    v.visit(match {
        [&found_point](point p) { 
            found_point = (p.x == 10 && p.y == 20);
        },
        [](auto) { }
    });
    assert_true(found_point);
}

TEST(variant_type_safety) {
    variant<int, string> v(42);
    
    // These should work
    assert_true(v.is<int>());
    assert_false(v.is<string>());
    
    // strict_get should work for correct type
    int val = v.strict_get<int>();
    assert_eq(val, 42);
    
    // Note: strict_get with wrong type would panic/crash,
    // but we can't easily test that in a unit test
}

TEST(variant_with_array_and_string) {
    // string and array are POD in this library
    variant<array<s32>, string> v;

    // Start empty
    assert_false(v);

    // Assign array
    array<s32> a;
    defer(free(a));
    For(range(3)) { a += {(s32)it}; }

    v = a; // shallow copy of POD struct
    assert_true(v);
    assert_true(v.is<array<s32>>());
    auto ar = v.strict_get<array<s32>>();
    assert_eq(ar.Count, 3);
    assert_eq(ar[0], 0);
    assert_eq(ar[1], 1);
    assert_eq(ar[2], 2);

    // Switch to string
    v = string("abc");
    assert_true(v.is<string>());
    assert_eq_str(v.strict_get<string>(), "abc");

    // Visit both cases
    string res;
    v.visit(match{
        [&res](array<s32> ar2){ res = sprint("arr:{}", ar2.Count); },
        [&res](string s){ res = sprint("str:{}", s); },
        [&res](auto){ res = "empty"; }
    });
    assert_eq_str(res, "str:abc");

    // Make variant empty and visit default branch
    v = variant<array<s32>, string>{};
    v.visit(match{
        [](array<s32>){}, [](string){}, [&res](auto){ res = "empty"; }
    });
    assert_eq_str(res, "empty");
}

TEST(optional_array_basic) {
    optional<array<s32>> o;
    assert_false(o);

    array<s32> a;
    defer(free(a));
    For(range(5)) { a += {(s32)(it * 2)}; }

    o = a;
    assert_true(o);
    assert_true(o.is<array<s32>>());
    auto ar = o.strict_get<array<s32>>();
    assert_eq(ar.Count, 5);
    assert_eq(ar[0], 0);
    assert_eq(ar[1], 2);
    assert_eq(ar[2], 4);
    assert_eq(ar[3], 6);
    assert_eq(ar[4], 8);

    // Reset
    o = optional<array<s32>>{};
    assert_false(o);
}
