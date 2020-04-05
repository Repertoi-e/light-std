#pragma once

#include <lstd/io/fmt.h>
#include <lstd/math.h>

template <s32 Dim, bool Packed = false>
using vecf = vec<f32, Dim, Packed>;

/*
REQUIRE\((.*?)== (.*?)\);
assert_eq($1, $2);


TEST_CASE_VEC_VARIANT\("(.*?)", "\[Vector\]", TypesAll, PackedAll\) {\s*\r\n\s*SECTION\(SECTIONNAMEVEC\)
TEST($1)
*/

template <typename T>
struct approx_helper {
    T Value;

    approx_helper() {}
    approx_helper(T value) { Value = value; }
};

template <typename T>
struct fmt::formatter<approx_helper<T>> {
    void format(approx_helper<T> src, format_context *f) { to_writer(f, "{}", src.Value); }
};

template <typename T, typename U>
bool operator==(approx_helper<T> lhs, U rhs) {
    return almost_equal(lhs.Value, rhs);
}

template <typename T, typename U>
bool operator==(T lhs, approx_helper<U> rhs) {
    return almost_equal(rhs.Value, lhs);
}

template <typename R, typename U>
bool operator==(approx_helper<R> lhs, approx_helper<U> rhs) {
    return almost_equal(lhs.Value, rhs.Value);
}

template <typename T>
approx_helper<T> approx(T arg) {
    return {arg};
}

template <typename Linalg>
struct approx_helper2 {
    Linalg Object = {no_init};

    approx_helper2() {}
    approx_helper2(Linalg object) { Object = object; }
};

template <typename T>
struct fmt::formatter<approx_helper2<T>> {
    void format(approx_helper2<T> src, format_context *f) { to_writer(f, "{}", src.Object); }
};

template <typename Linalg1, typename Linalg2>
bool operator==(approx_helper2<Linalg1> lhs, Linalg2 rhs) {
    return almost_equal(lhs.Object, rhs);
}

template <typename Linalg1, typename Linalg2>
bool operator==(Linalg1 lhs, approx_helper2<Linalg2> rhs) {
    return almost_equal(rhs.Object, lhs);
}

template <typename Linalg1, typename Linalg2>
bool operator==(approx_helper2<Linalg1> lhs, approx_helper2<Linalg2> rhs) {
    return almost_equal(lhs.Object, rhs.Object);
}

template <typename Linalg>
approx_helper2<Linalg> approx_vec(const Linalg &arg) {
    return {arg};
}
