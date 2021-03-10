#include "../test.h"
#include "math.h"

TEST(quat_ctor) {
    quat q1(1, 2, 3, 4);
    assert_eq(q1.w, 1);
    assert_eq(q1.x, 2);
    assert_eq(q1.y, 3);
    assert_eq(q1.z, 4);

    quat q2(1.f, vecf<3>{2, 3, 4});
    assert_eq(q2.w, 1);
    assert_eq(q2.x, 2);
    assert_eq(q2.y, 3);
    assert_eq(q2.z, 4);

    quat q3(vecf<3>{2, 3, 4});
    assert_eq(q3.w, 0);
    assert_eq(q3.x, 2);
    assert_eq(q3.y, 3);
    assert_eq(q3.z, 4);
}

TEST(axis_angle) {
    quat q = rotation_axis_angle(normalize(vecf<3>{1, 2, 3}), 0.83f);
    quat qexp(0.9151163f, 0.107757f, 0.2155141f, 0.3232711f);
    assert_eq(approx_vec(q), qexp);
}

TEST(tri_axis) {
    quat q = rotation_axis_3<1, 2, 0>(1.0f, 0.8f, 1.2f);

    quat q1 = rotation_axis_angle(vecf<3>{1, 0, 0}, 1.2f);
    quat q2 = rotation_axis_angle(vecf<3>{0, 0, 1}, 0.8f);
    quat q3 = rotation_axis_angle(vecf<3>{0, 1, 0}, 1.0f);

    quat qexp = qmul(q1, qmul(q2, q3));  // Quaternion multipliation is associative (but not commutative)
    assert_eq(approx_vec(q), qexp);
}

TEST(query_axis_angle) {
    vecf<3> axis(1, 2, 3);
    axis = normalize(axis);
    f32 angle = 0.83f;

    quat q = rotation_axis_angle(axis, angle);

    assert_eq(approx_vec(axis), q.axis());
    assert_eq(approx(angle), q.angle());

    q = {1, 0, 0, 0};
    axis = {1, 0, 0};
    auto qaxis = q.axis();
    assert_eq(approx_vec(axis), q.axis());
    assert_eq(approx(0.0f), q.angle());
}

TEST(to_mat) {
    quat q(0.9151163f, 0.107757f, 0.2155141f, 0.3232711f);
    mat<f32, 3, 3> m33 = (decltype(m33)) q;
    mat<f32, 4, 4> m44 = (decltype(m44)) q;

    mat<f32, 3, 3> m33exp = {0.6980989, -0.5452151, 0.4641104, 0.6381077, 0.7677684,
                             -0.0578815, -0.3247714, 0.3365594, 0.8838842};
    mat<f32, 4, 4> m44exp = {0.6980989, -0.5452151, 0.4641104, 0, 0.6381077, 0.7677684, -0.0578815, 0,
                             -0.3247714, 0.3365594, 0.8838842, 0, 0, 0, 0, 1};
    assert_eq(approx_vec(m33), m33exp);
    assert_eq(approx_vec(m44), m44exp);
}

// Only works if _to_mat_ works.
TEST(from_mat) {
    quat q(0.9151163f, 0.107757f, 0.2155141f, 0.3232711f);
    mat<f32, 3, 3> m331 = (decltype(m331)) q;
    mat<f32, 3, 3> m332 = (decltype(m332)) q;
    mat<f32, 4, 3> m34 = (decltype(m34)) q;
    mat<f32, 4, 4> m441 = (decltype(m441)) q;
    mat<f32, 4, 4> m442 = (decltype(m442)) q;

    assert_eq(approx_vec(q), quat(m331));
    assert_eq(approx_vec(q), quat(m332));
    assert_eq(approx_vec(q), quat(m34));
    assert_eq(approx_vec(q), quat(m441));
    assert_eq(approx_vec(q), quat(m442));
}

TEST(add_subtract) {
    quat q1(1, 2, 3, 4);
    quat q2(4, 5, 6, 3);
    quat q3 = q1 + q2;
    quat q4 = q1 - q2;
    quat q3exp(5, 7, 9, 7);
    quat q4exp(-3, -3, -3, 1);

    assert_eq(approx_vec(q3exp), q3);
    assert_eq(approx_vec(q4exp), q4);
}

TEST(product) {
    quat q1(1, 2, 3, 4);
    quat q2(4, 5, 6, 3);
    quat q3 = qmul(q1, q2);
    quat q3exp(-36, -2, 32, 16);

    assert_eq(approx_vec(q3exp), q3);
}

template <bool Packed>
void vec_rotation_test() {
    quat q = rotation_axis_angle(normalize(vec<f32, 3, Packed>{1, 2, 3}), 0.83f);
    mat<f32, 3, 3, Packed> M = rotation_axis_angle(normalize(vec<f32, 3, Packed>{1, 2, 3}), 0.83f);

    vec<f32, 3, Packed> v(3, 2, 6);

    auto r1 = rotate_vec(v, q);
    auto r2 = dot(v, M);

    assert_eq(approx_vec(r1), r2);
}

TEST(vec_rotation) {
    vec_rotation_test<true>();
    vec_rotation_test<false>();
}

template <bool Packed>
void chaining_test() {
    vec<f32, 3, Packed> axis1(1, 2, 3);
    vec<f32, 3, Packed> axis2(3, 1, 2);
    axis1 = normalize(axis1);
    axis2 = normalize(axis2);
    f32 angle1 = 0.83f;
    f32 angle2 = 1.92f;

    tquat<f32, Packed> q1 = rotation_axis_angle(axis1, angle1);
    tquat<f32, Packed> q2 = rotation_axis_angle(axis2, angle2);
    mat<f32, 3, 3, Packed> M2 = rotation_axis_angle(axis2, angle2);
    mat<f32, 3, 3, Packed> M1 = rotation_axis_angle(axis1, angle1);

    vec<f32, 3, Packed> v(3, 2, 6);

    auto r1 = rotate_vec(v, qmul(q2, q1));
    auto r2 = dot(v, (dot(M1, M2)));

    assert_eq(approx_vec(r1), r2);
}

TEST(chaining) {
    chaining_test<true>();
    chaining_test<false>();
}

TEST(exp_ln) {
    quat q(1.0f, 2.0f, 0.5f, -0.7f);
    quat p = exp(ln(q));
    assert_eq(approx_vec(q), p);
}

TEST(pow) {
    quat q(1.0f, 2.0f, 0.5f, -0.7f);

    quat p = pow(q, 3.0f);
    quat pexp = qmul(q, qmul(q, q));  // Quaternion multipliation is associative (but not commutative)

    assert_eq(approx_vec(p), pexp);
}
