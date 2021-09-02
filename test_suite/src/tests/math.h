#pragma once

template <typename T>
struct approx_helper {
    T Value;

    approx_helper() {
    }

    approx_helper(T value) { Value = value; }
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

template <typename T>
struct formatter<approx_helper<T>> {
    void format(approx_helper<T> src, fmt_context *f) { fmt_to_writer(f, "{}", src.Value); }
};

template <typename Linalg>
struct approx_helper2 {
    Linalg Object;

    approx_helper2() {
    }

    approx_helper2(Linalg object) { Object = object; }
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

template <typename T>
struct formatter<approx_helper2<T>> {
    void format(approx_helper2<T> src, fmt_context *f) { fmt_to_writer(f, "{}", src.Object); }
};
