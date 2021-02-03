#include "../test.h"
#include "math.h"

TEST(ctor_and_index) {
    matf<3, 3> m = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    matf<3, 3> n = {no_init};
    n(0, 0) = 1;
    n(0, 1) = 2;
    n(0, 2) = 3;
    n(1, 0) = 4;
    n(1, 1) = 5;
    n(1, 2) = 6;
    n(2, 0) = 7;
    n(2, 1) = 8;
    n(2, 2) = 9;

    assert_eq(m, n);
}

TEST(thin_mat_from_vec) {
    vec<f32, 3> v(1, 2, 3);

    matf<3, 1> m1 = v;
    matf<1, 3> m2 = v;

    assert_eq(m1(0, 0), 1);
    assert_eq(m1(1, 0), 2);
    assert_eq(m1(2, 0), 3);

    assert_eq(m2(0, 0), 1);
    assert_eq(m2(0, 1), 2);
    assert_eq(m2(0, 2), 3);
}

TEST(thin_mat_to_vec) {
    vec<f32, 3> vexp(1, 2, 3);

    matf<3, 1> m1 = {1, 2, 3};
    matf<1, 3> m2 = {1, 2, 3};

    vec<f32, 3> v1 = m1;
    vec<f32, 3> v2 = m2;

    assert_eq(v1, vexp);
    assert_eq(v2, vexp);
}

TEST(thin_mat_short_index) {
    matf<3, 1> m1 = {1, 2, 3};
    matf<1, 3> m2 = {1, 2, 3};

    assert_eq(m1(0), 1);
    assert_eq(m1(1), 2);
    assert_eq(m1(2), 3);

    assert_eq(m2(0), 1);
    assert_eq(m2(1), 2);
    assert_eq(m2(2), 3);
}

TEST(view) {
    mat<utf8, 5, 5> m1 = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                          'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y'};

    mat<utf8, 5, 5> m2 = {'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z',
                          'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z', 'z'};

    mat<utf8, 5, 5> r = {'z', 'z', 'z', 'p', 'q', 'z', 'z', 'z', 'u', 'v', 'c', 'd', 'e',
                         'z', 'z', 'h', 'i', 'j', 'z', 'z', 'm', 'n', 'o', 'z', 'z'};

    mat<utf8, 2, 2> sm = m1.get_view<2, 2>(3, 0);
    m2.get_view<3, 3>(2, 0) = m1.get_view<3, 3>(0, 2);
    m2.get_view<2, 2>(0, 3) = sm;
    assert_eq(m2, r);

    m2.col(4) = vec<f32, 5>('0');
    r(0, 4) = r(1, 4) = r(2, 4) = r(3, 4) = r(4, 4) = '0';
    assert_eq(m2, r);

    vec<utf8, 3> v = m1.get_view<3, 1>(0, 0);
    vec<utf8, 3> vr = {'a', 'f', 'k'};
    assert_eq(v, vr);
    v = m1.get_view<1, 3>(0, 0);
    vr = {'a', 'b', 'c'};
    assert_eq(v, vr);
}

TEST(mat_add) {
    matf<3, 3> m1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    matf<3, 3> m2 = {7, 6, 5, 4, 3, 2, 1, 0, -1};
    decltype(m1 + m2) rexp1 = {8, 8, 8, 8, 8, 8, 8, 8, 8};

    matf<4, 5> m3 = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
    matf<4, 5> m4 = {4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1};
    decltype(m3 + m4) rexp2 = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

    matf<2, 2> m5 = {1, 2, 3, 4};
    matf<2, 2> m6 = {4, 3, 2, 1};
    decltype(m5 + m6) rexp3 = {5, 5, 5, 5};

    assert_eq(m1 + m2, rexp1);
    assert_eq(m3 + m4, rexp2);
    assert_eq(m5 + m6, rexp3);
}

TEST(mat_subtract) {
    matf<3, 3> m1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    matf<3, 3> m2 = {2, 3, 4, 5, 6, 7, 8, 9, 10};
    decltype(m1 - m2) rexp1 = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

    matf<2, 2> m3 = {1, 2, 3, 4};
    matf<2, 2> m4 = {2, 3, 4, 5};
    decltype(m3 - m4) rexp2 = {-1, -1, -1, -1};

    assert_eq(m1 - m2, rexp1);
    assert_eq(m3 - m4, rexp2);
}

TEST(mat_multiply_square) {
    matf<2, 2> m2 = {1, 2, 3, 4};
    matf<2, 2> n2 = {5, 6, 7, 8};
    decltype(dot(m2, n2)) exp2 = {19, 22, 43, 50};

    assert_eq(dot(m2, n2), exp2);

    matf<3, 3> m = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    matf<3, 3> n = {5, 6, 8, 1, 3, 5, 7, 8, 4};
    decltype(dot(m, n)) exp = {28, 36, 30, 67, 87, 81, 106, 138, 132};

    assert_eq(dot(m, n), exp);

    matf<5, 5> m5 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
    matf<5, 5> n5 = {9, 8, 7, 6, 5, 4, 2, 7, 3, 5, 3, 6, 2, 7, 2, 9, 4, 1, 4, 7, 5, 7, 5, 5, 1};
    decltype(dot(m5, n5)) exp5 = {87, 81, 56, 74, 54, 237, 216, 166, 199, 154, 387, 351, 276,
                                  324, 254, 537, 486, 386, 449, 354, 687, 621, 496, 574, 454};

    assert_eq(dot(m5, n5), exp5);
}

TEST(mat_multiply_arbitrary) {
    matf<2, 4> m2 = {1, 2, 3, 4, 3, 4, 5, 6};
    matf<4, 2> n2 = {
        5,
        6,
        7,
        8,
        6,
        4,
        4,
        9,
    };
    decltype(dot(m2, n2)) exp21 = {53, 70, 97, 124};
    decltype(dot(n2, m2)) exp22 = {23, 34, 45, 56, 31, 46, 61, 76, 18, 28, 38, 48, 31, 44, 57, 70};

    assert_eq(dot(m2, n2), exp21);
    assert_eq(dot(n2, m2), exp22);
}

#define TEST_MAT_SCALAR_OP(NAME, OPERATOR)                                                                   \
    TEST(mat_scalar_##NAME) {                                                                                \
        const matf<2, 2> sm = {1, 2, 3, 4};                                                                  \
        const matf<2, 5> m = {1, 2, 5, 6, 9, 3, 4, 7, 8, 10};                                                \
                                                                                                             \
        const f32 b = 27;                                                                                    \
        auto smr = sm OPERATOR b;                                                                            \
        auto mr = m OPERATOR b;                                                                              \
                                                                                                             \
        For_as(i, range(sm.R)) { For_as(j, range(sm.C)) assert_eq(approx(sm(i, j) OPERATOR b), smr(i, j)); } \
        For_as(i, range(m.R)) { For_as(j, range(m.C)) assert_eq(approx(m(i, j) OPERATOR b), mr(i, j)); }     \
    }

#define TEST_SCALAR_MAT_OP(NAME, OPERATOR)                                                                   \
    TEST(scalar_mat_##NAME) {                                                                                \
        const matf<2, 2> sm = {1, 2, 3, 4};                                                                  \
        const matf<2, 5> m = {1, 2, 5, 6, 9, 3, 4, 7, 8, 10};                                                \
                                                                                                             \
        const f32 b = 27;                                                                                    \
        auto smr = b OPERATOR sm;                                                                            \
        auto mr = b OPERATOR m;                                                                              \
                                                                                                             \
        For_as(i, range(sm.R)) { For_as(j, range(sm.C)) assert_eq(approx(b OPERATOR sm(i, j)), smr(i, j)); } \
        For_as(i, range(m.R)) { For_as(j, range(m.C)) assert_eq(approx(b OPERATOR m(i, j)), mr(i, j)); }     \
    }

#define TEST_MAT_SCALAR_COMPOUND_OP(NAME, OPERATOR)           \
    TEST(scalar_compound_##NAME) {                            \
        const matf<2, 2> sm = {1, 2, 3, 4};                   \
        const matf<2, 5> m = {1, 2, 5, 6, 9, 3, 4, 7, 8, 10}; \
                                                              \
        const f32 b = 27;                                     \
        auto smr = sm;                                        \
        auto mr = m;                                          \
        smr OPERATOR b;                                       \
        mr OPERATOR b;                                        \
                                                              \
        For_as(i, range(sm.R)) {                              \
            For_as(j, range(sm.C)) {                          \
                auto elem = sm(i, j);                         \
                elem OPERATOR b;                              \
                assert_eq(approx(elem), smr(i, j));           \
            }                                                 \
        }                                                     \
        For_as(i, range(m.R)) {                               \
            For_as(j, range(m.C)) {                           \
                auto elem = m(i, j);                          \
                elem OPERATOR b;                              \
                assert_eq(approx(elem), mr(i, j));            \
            }                                                 \
        }                                                     \
    }

// A note to tell our script to do special work because of the macros we use.
//
// :build_tests: mat_scalar_##NAME      -> mat_scalar_multiply       mat_scalar_divide
// :build_tests: scalar_mat_##NAME      -> scalar_mat_multiply       scalar_mat_divide
// :build_tests: scalar_compound_##NAME -> scalar_compound_multiply  scalar_compound_divide

TEST_MAT_SCALAR_OP(multiply, *)
TEST_MAT_SCALAR_OP(divide, /)

TEST_SCALAR_MAT_OP(multiply, *)
TEST_SCALAR_MAT_OP(divide, /)

TEST_MAT_SCALAR_COMPOUND_OP(multiply, *=)
TEST_MAT_SCALAR_COMPOUND_OP(divide, /=)

TEST(vec_square_multiply) {
    matf<3, 3> m = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    vec<f32, 3> v(5, 7, 11);
    auto p = dot(v, m);

    vec<f32, 3> exp(110, 133, 156);
    assert_eq(p, approx_vec(exp));
}

TEST(vec_non_square_multiply) {
    matf<4, 3> m = {1, 2, 3, 4, 5, 6, 7, 8, 9, 6, 7, 8};

    vec<f32, 4> v(5, 7, 11, 1);
    auto p = dot(v, m);

    vec<f32, 3> exp(116, 140, 164);
    assert_eq(p, approx_vec(exp));
}

TEST(vec_implicit_affine_multiply) {
    matf<4, 3> m = {1, 2, 3, 4, 5, 6, 7, 8, 9, 6, 7, 8};

    vec<f32, 3> v(5, 7, 11);
    auto p = dot(v, m);

    vec<f32, 3> exp(116, 140, 164);
    assert_eq(p, approx_vec(exp));
}

TEST(vec_implicit_homogeneous_multiply) {
    matf<4, 4> m = {1, 2, 3, 3, 4, 5, 6, 7, 7, 8, 9, 2, 6, 7, 8, 3};

    vec<f32, 3> v(5, 7, 11);
    auto p = dot(v, m);

    vec<f32, 3> exp(116.f / 89.f, 140.f / 89.f, 164.f / 89.f);
    assert_eq(p, approx_vec(exp));
}

TEST(trace) {
    matf<3, 3> m = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    auto t = trace(m);

    assert_eq(approx(t), 15.f);

    matf<5, 5> m5 = {5, 7, 3, 6, 4, 4, 7, 4, 6, 3, 6, 2, 8, 9, 7, 1, 2, 7, 4, 8, 5, 9, 7, 1, 5};
    t = trace(m5);
    assert_eq(approx(t), 29);
}

TEST(transpose) {
    matf<4, 2> m = {1, 2, 3, 4, 5, 6, 7, 8};
    matf<2, 4> mT = T(m);
    matf<2, 4> mexp = {1, 3, 5, 7, 2, 4, 6, 8};

    assert_eq(mT, mexp);
}

TEST(det_small) {
    matf<2, 2> m2 = {1, 3, 4, 5};
    assert_eq(approx(det(m2)), -7);

    matf<4, 4> m4 = {1, 3, 2, 1, 4, 5, 6, 2, 7, 8, 9, 3, 1, 2, 3, 4};
    assert_eq(approx(det(m4)), 27);

    matf<3, 3> m3 = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    assert_eq(approx(det(m3)), 9);
}

TEST(det) {
    matf<5, 5> m5 = {5, 7, 3, 6, 4, 4, 7, 4, 6, 3, 6, 2, 8, 9, 7, 1, 2, 7, 4, 8, 5, 9, 7, 1, 5};
    assert_eq(approx(det(m5)), 4134);
}

TEST(inverse_small) {
    matf<2, 2> m2 = {1, 3, 4, 5};
    matf<2, 2> mI2 = inverse(m2);
    matf<2, 2> mexp2 = {-0.714286, 0.428571, 0.571429, -0.142857};

    assert_eq(approx_vec(mI2), mexp2);

    matf<3, 3> m3 = {1, 3, 2, 4, 5, 6, 7, 8, 9};
    matf<3, 3> mI3 = inverse(m3);
    matf<3, 3> mexp3 = {-0.333333, -1.222222, 0.888889, 0.666667, -0.555556, 0.222222, -0.333333, 1.444444, -0.777778};

    assert_eq(approx_vec(mI3), mexp3);

    matf<4, 4> m4 = {1, 3, 2, 1, 4, 5, 6, 2, 7, 8, 9, 3, 1, 2, 3, 4};
    matf<4, 4> mI4 = inverse(m4);
    matf<4, 4> mexp4 = {-0.333333, -1.296296, 0.925926, 0.037037, 0.666667, -0.407407, 0.148148, -0.074074,
                        -0.333333, 1.592593, -0.851852, -0.074074, 0, -0.666667, 0.333333, 0.333333};

    assert_eq(approx_vec(mI4), mexp4);
}

TEST(inverse) {
    matf<5, 5> n = {1, 56, 8, 4, 3, 4, 2, 7, 8, 4, 1, 5, 7, 4, 3, 9, 5, 3, 8, 4, 7, 2, 83, 46, 4};
    matf<5, 5> nI = inverse(n);
    matf<5, 5> iden = dot(n, nI);
    matf<5, 5> idenexp = identity();

    assert_eq(approx_vec(idenexp), iden);
}

TEST(norm) {
    vec<f32, 8> v(1, 2, 3, 4, 5, 6, 7, 8);
    matf<2, 4> m = {1, 2, 3, 4, 5, 6, 7, 8};
    assert_eq(approx(len(v)), norm(m));
}

TEST(lu_decomposition) {
    matf<3, 3> A = {3, -0.1, -0.2, 0.1, 7, -0.3, 0.3, -0.2, 10};

    auto [L, U] = decompose_lu(A);

    For_as(i, range(A.R)) {
        For_as(j, range(i - 1)) {
            assert_eq(U(i, j), approx(0.0));
            assert_eq(L(j, i), approx(0.0));
        }
    }

    auto Mprod = dot(L, U);
    assert_eq(approx_vec(A), Mprod);
}

TEST(lu_solve) {
    matf<3, 3> A = {3, -0.1f, -0.2f, 0.1f, 7, -0.3f, 0.3f, -0.2f, 10};
    vec<f32, 3> b(7.85, -19.3, 71.4);
    vec<f32, 3> x(no_init);
    vec<f32, 3> xexp(3, -2.5, 7);

    x = decompose_lu(A).solve(b);
    assert_eq(approx_vec(x), xexp);
}

TEST(lup_decomposition) {
    matf<3, 3> A = {3, -0.1f, -0.2f, 0.3f, -0.2f, 10, 0.1f, 7, -0.3f};

    auto [L, U, P] = decompose_lup(A);

    For_as(i, range(A.R)) {
        For_as(j, range(i - 1)) {
            assert_eq(U(i, j), approx(0.0f));
            assert_eq(L(j, i), approx(0.0f));
        }
    }

    matf<3, 3> Pm = zero();
    For(P) Pm(it, P[it]) = 1.0f;

    auto Mprod = dot(dot(T(Pm), L), U);
    assert_eq(approx_vec(A), Mprod);
}

TEST(lup_solve) {
    matf<4, 4> A = {1, 3, 4, 6, 3, 6, 2, 6, 9, 2, 6, 7, 6, 2, 7, 5};
    vec<f32, 4> b(3, 4, 2, 8);
    vec<f32, 4> x(no_init);
    vec<f32, 4> xexp(-94.f / 497, 895.f / 497, 1000.f / 497, -850.f / 497);

    x = decompose_lup(A).solve(b);

    assert_eq(approx_vec(x), xexp);
}

TEST(lup_decomposition_singular) {
    matf<3, 3> A = {1, 0, 0, 0, 0, 1, 0, -1, 0};

    auto [L, U, P] = decompose_lup(A);

    For_as(i, range(A.R)) {
        For_as(j, range(i - 1)) {
            assert_eq(U(i, j), approx(0.0f));
            assert_eq(L(j, i), approx(0.0f));
        }
    }

    matf<3, 3> Pm = zero();
    For(P) Pm(it, P[it]) = 1.0f;

    auto Mprod = dot(dot(T(Pm), L), U);
    assert_eq(approx_vec(A), Mprod);
}

TEST(qr_decomposition) {
    // example from wikipedia SVD article
    matf<5, 4> A1 = T(matf<4, 5>{1, 0, 0, 1, 2, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0});
    auto [Q1, R1] = decompose_qr(A1);
    matf<5, 4> A1assembled = dot(Q1, R1);
    assert_eq(approx_vec(A1assembled), A1);

    // the same matrix as the LU
    matf<3, 3> A2 = {3, -0.1f, -0.2f, 0.1f, 7, -0.3f, 0.3f, -0.2f, 10};

    auto [Q2, R2] = decompose_qr(A2);

    matf<3, 3> A2assembled = dot(Q2, R2);
    assert_eq(approx_vec(A2assembled), A2);
}

TEST(transform_identity) {
    matf<3, 3> m = identity();
    matf<3, 3> mexp = {1, 0, 0, 0, 1, 0, 0, 0, 1};

    assert_eq(m, mexp);

    matf<3, 5> m5 = identity();
    matf<3, 5> mexp5 = {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0};

    assert_eq(m5, mexp5);
}

TEST(transform_zero) {
    matf<3, 4> m = zero();
    matf<3, 4> mexp = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    assert_eq(m, mexp);
}

TEST(transform_rotation_2d) {
    matf<2, 2> m22 = rotation(1.f);
    matf<3, 2> m32 = rotation(1.f);
    matf<3, 3> m33 = rotation(1.f);

    matf<2, 2> m22exp = {0.54030, 0.84147, -0.84147, 0.54030};
    matf<3, 2> m32exp = {0.54030, 0.84147, -0.84147, 0.54030, 0, 0};
    matf<3, 3> m33exp = {0.54030, 0.84147, 0, -0.84147, 0.54030, 0, 0, 0, 1};
    assert_eq(approx_vec(m22), m22exp);
    assert_eq(approx_vec(m32), m32exp);
    assert_eq(approx_vec(m33), m33exp);
}

TEST(transform_rotation_principal) {
    matf<3, 3> m33 = rotation_x(1.f);
    matf<3, 3> m33exp = {1.000000, 0.000000, 0.000000, 0.000000, 0.540302, 0.841471, 0.000000, -0.841471, 0.540302};
    assert_eq(approx_vec(m33), m33exp);

    matf<4, 3> m43 = rotation_y(1.f);
    matf<4, 3> m43exp = {0.540302, 0.000000, -0.841471, 0.000000, 1.000000, 0.000000,
                         0.841471, 0.000000, 0.540302, 0, 0, 0};
    assert_eq(approx_vec(m43), m43exp);

    matf<4, 4> m44 = rotation_z(1.f);
    matf<4, 4> m44exp = {0.540302, 0.841471, 0.000000, 0, -0.841471, 0.540302, 0.000000, 0,
                         0.000000, 0.000000, 1.000000, 0, 0, 0, 0, 1};
    assert_eq(approx_vec(m44), m44exp);
}

TEST(transform_rotation_tri_axis) {
    matf<3, 3> m33 = rotation_axis_3<0, 1, 1>(1.f, 1.0f, -1.0f);
    matf<3, 3> m33exp = {1.000000, 0.000000, 0.000000, 0.000000, 0.540302, 0.841471, 0.000000, -0.841471, 0.540302};
    assert_eq(approx_vec(m33), m33exp);

    matf<4, 3> m43 = rotation_axis_3<0, 1, 2>(0.0f, 1.f, 0.0f);
    matf<4, 3> m43exp = {0.540302, 0.000000, -0.841471, 0.000000, 1.000000, 0.000000,
                         0.841471, 0.000000, 0.540302, 0, 0, 0};
    assert_eq(approx_vec(m43), m43exp);

    matf<4, 4> m44 = rotation_axis_3<0, 0, 2>(-1.0f, 1.0f, 1.f);
    matf<4, 4> m44exp = {0.540302, 0.841471, 0.000000, 0, -0.841471, 0.540302, 0.000000, 0,
                         0.000000, 0.000000, 1.000000, 0, 0, 0, 0, 1};
    assert_eq(approx_vec(m44), m44exp);
}

TEST(transform_rotation_axis_angle) {
    matf<3, 3> m33 = rotation_axis_angle(normalize(vec<f32, 3>(1, 2, 3)), 1.0f);
    matf<3, 3> m33exp = {0.573138, 0.740349, -0.351279, -0.609007, 0.671645, 0.421906, 0.548292, -0.027879, 0.835822};
    assert_eq(approx_vec(m33), m33exp);

    matf<4, 3> m43 = rotation_axis_angle(normalize(vec<f32, 3>(1, 2, 3)), 1.0f);
    matf<4, 3> m43exp = {0.573138, 0.740349, -0.351279, -0.609007, 0.671645, 0.421906,
                         0.548292, -0.027879, 0.835822, 0, 0, 0};
    assert_eq(approx_vec(m43), m43exp);

    matf<4, 4> m44 = rotation_axis_angle(normalize(vec<f32, 3>(1, 2, 3)), 1.0f);
    matf<4, 4> m44exp = {0.573138, 0.740349, -0.351279, 0, -0.609007, 0.671645, 0.421906, 0,
                         0.548292, -0.027879, 0.835822, 0, 0, 0, 0, 1};
    assert_eq(approx_vec(m44), m44exp);
}

TEST(transform_scale) {
    matf<5, 5> m = scale(1, 2, 3, 4, 5);
    vec<f32, 5> v(2, 6, 3, 7, 5);
    matf<3, 3> m3 = scale(vec<f32, 3>{1, 2, 3});

    auto vt1 = v * vec<f32, 5>{1, 2, 3, 4, 5};
    auto vt2 = dot(v, m);

    assert_eq(vt1, vt2);

    matf<5, 5> mp = scale(1, 2, 3, 4, 5);
    assert_eq(approx_vec(mp), m);
}

TEST(transform_translation) {
    matf<3, 3> m2d_33a = translation(1, 2);
    matf<3, 3> m2d_33b = translation(vec<f32, 2, false>(1, 2));
    matf<3, 3> m2d_33exp = {1, 0, 0, 0, 1, 0, 1, 2, 1};
    assert_eq(approx_vec(m2d_33a), m2d_33exp);
    assert_eq(approx_vec(m2d_33b), m2d_33exp);

    matf<3, 2> m2d_32 = translation(1, 2);
    matf<3, 2> m2d_32exp = {1, 0, 0, 1, 1, 2};
    assert_eq(approx_vec(m2d_32), m2d_32exp);

    vec<f32, 5> t(1, 2, 3, 4, 5);
    matf<6, 5> m = translation(t);
    vec<f32, 5> v(1, 2, 3, 4, 5);
    v = dot(v, m);

    vec<f32, 5> vexp(2, 4, 6, 8, 10);
    assert_eq(v, vexp);
}

TEST(transform_orthographic) {
    using Vec = vec<f32, 3>;
    using VecF = vec<f32, 3, false>;
    Vec worldFrustum[2] = {{-0.25f, -0.44444444f, 0.5f}, {5.0f, 8.8888888f, 10.f}};
    Vec ndcFrustum[2] = {no_init, no_init};

    // Z forward
    matf<4, 4> m = orthographic(VecF(worldFrustum[0]), VecF(worldFrustum[1]), 0.f, 1.f);
    ndcFrustum[0] = dot(worldFrustum[0], m);
    ndcFrustum[1] = dot(worldFrustum[1], m);

    assert_eq(approx_vec(ndcFrustum[0]), Vec(-1, -1, 0));
    assert_eq(approx_vec(ndcFrustum[1]), Vec(1, 1, 1));
}

template <typename T>
struct Basis {
    vec<T, 3> Basis1 = {no_init};
    vec<T, 3> Basis2 = {no_init};
    vec<T, 3> Basis3 = {no_init};
    vec<T, 3> Center = {no_init};

    Basis() {
        Basis1 = normalize(vec<T, 3>(-1, 3, 0));
        Basis2 = normalize(vec<T, 3>(3, 1, 0));
        Basis3 = normalize(vec<T, 3>(0, 0, 1));
        Center = {6, 5, 8};
        assert(approx(0) == dot(Basis1, Basis2));
        assert(approx(0) == dot(Basis1, Basis3));
        assert(approx(0) == dot(Basis3, Basis2));
    }

    vec<T, 3> express(vec<T, 3> v) { return v[0] * Basis1 + v[1] * Basis2 + v[2] * Basis3 + Center; }
};

TEST(transform_view) {
    Basis<f32> basis;
    using Vec = vec<f32, 3>;

    stack_array<Vec, 6> viewVecs = {
        Vec(1, 2, 3),
        Vec(5, -5, 3),
        Vec(1, 7, -1),
        Vec(9, 3, -2),
        Vec(9, 3, 4),
        Vec(-4, -3, 4),
    };
    stack_array<Vec, 6> worldVecs = make_stack_array_of_uninitialized_math_type<Vec, 6>();
    For(range(6)) worldVecs[it] = basis.express(viewVecs[it]);

    vec<f32, 3, false> eye = basis.Center;
    vec<f32, 3, false> target = basis.Center + 2 * basis.Basis1;
    vec<f32, 3, false> up = normalize(basis.Basis3 + f32(0.1) * basis.Basis1);

    matf<4, 4> m = look_at(eye, target, up, true, false, false);
    matf<4, 4> mfff = look_at(eye, target, up, false, false, false);
    matf<4, 4> mftf = look_at(eye, target, up, false, true, false);
    matf<4, 4> mftt = look_at(eye, target, up, false, true, true);

    assert_eq(dot(basis.Center + basis.Basis1, m), approx_vec(Vec(0, 0, 1)));
    assert_eq(dot(basis.Center + basis.Basis2, m), approx_vec(Vec(1, 0, 0)));
    assert_eq(dot(basis.Center + basis.Basis3, m), approx_vec(Vec(0, 1, 0)));

    assert_eq(dot(basis.Center + basis.Basis1, mfff), approx_vec(Vec(0, 0, -1)));
    assert_eq(dot(basis.Center + basis.Basis2, mfff), approx_vec(Vec(1, 0, 0)));
    assert_eq(dot(basis.Center + basis.Basis3, mfff), approx_vec(Vec(0, 1, 0)));

    assert_eq(dot(basis.Center + basis.Basis1, mftf), approx_vec(Vec(0, 0, -1)));
    assert_eq(dot(basis.Center + basis.Basis2, mftf), approx_vec(Vec(-1, 0, 0)));
    assert_eq(dot(basis.Center + basis.Basis3, mftf), approx_vec(Vec(0, 1, 0)));

    assert_eq(dot(basis.Center + basis.Basis1, mftt), approx_vec(Vec(0, 0, -1)));
    assert_eq(dot(basis.Center + basis.Basis2, mftt), approx_vec(Vec(-1, 0, 0)));
    assert_eq(dot(basis.Center + basis.Basis3, mftt), approx_vec(Vec(0, -1, 0)));

    For(range(6)) assert_eq(dot(worldVecs[it], m), approx_vec(Vec(viewVecs[it].yzx)));
}

TEST(transform_view_2d) {
    using Vec = vec<f32, 2>;
    vec<f32, 2, false> eye(3, 4);
    vec<f32, 2, false> target(6, 5);
    vec<f32, 2, false> test(4, 4);

    matf<3, 3> m = look_at(eye, target, true, false);

    assert_eq(dot(Vec(eye), m), approx_vec(Vec(0, 0)));
    assert_eq(normalize(dot(Vec(target), m)), approx_vec(Vec(0, 1)));
    assert_gt(dot(Vec(test), m).x, 0);
    assert_gt(dot(Vec(test), m).y, 0);
}
