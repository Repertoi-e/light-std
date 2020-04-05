#pragma once

#include <emmintrin.h>

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Specialization for 4f32, using SSE
template <>
struct alignas(16) simd<f32, 4> {
    __m128 reg;

    static inline simd mul(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_mul_ps(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd div(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_div_ps(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd add(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_add_ps(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd sub(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_sub_ps(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd mul(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg = _mm_mul_ps(lhs.reg, temp);
        return result;
    }

    static inline simd div(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg = _mm_div_ps(lhs.reg, temp);
        return result;
    }

    static inline simd add(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg = _mm_add_ps(lhs.reg, temp);
        return result;
    }

    static inline simd sub(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg = _mm_sub_ps(lhs.reg, temp);
        return result;
    }

    static inline simd mad(const simd &a, const simd &b, const simd &c) { return add(mul(a, b), c); }

    static inline simd spread(f32 value) {
        simd result;
        result.reg = _mm_set1_ps(value);
        return result;
    }

    static inline simd set(f32 x, f32 y, f32 z, f32 w) {
        simd result;
        result.reg = _mm_setr_ps(x, y, z, w);
        return result;
    }

    template <s64 Count>
    static inline f32 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 4, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(0 < Count, "Count must not be zero.");

        f32 sum;
        simd m = mul(lhs, rhs);

        auto *v = m.reg.m128_f32;
        sum = v[0];
        for (s64 i = 1; i < Count; ++i) {
            sum += v[i];
        }
        return sum;
    }

    template <s64 i0, s64 i1, s64 i2, s64 i3>
    static inline simd shuffle(const simd &arg) {
        simd result;
        auto r = _mm_shuffle_epi32(*((__m128i *) &arg.reg), _MM_SHUFFLE(i0, i1, i2, i3));
        copy_memory(&result.reg, &r, sizeof(r));
        return result;
    }
};

// Specialization for 8f32, using SSE
template <>
struct alignas(16) simd<f32, 8> {
    __m128 reg[2];

    static inline simd mul(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_mul_ps(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd div(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_div_ps(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_div_ps(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd add(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_add_ps(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_add_ps(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd sub(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_sub_ps(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_sub_ps(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd mul(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg[0] = _mm_mul_ps(lhs.reg[0], temp);
        result.reg[1] = _mm_mul_ps(lhs.reg[1], temp);
        return result;
    }

    static inline simd div(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg[0] = _mm_div_ps(lhs.reg[0], temp);
        result.reg[1] = _mm_div_ps(lhs.reg[1], temp);
        return result;
    }

    static inline simd add(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg[0] = _mm_add_ps(lhs.reg[0], temp);
        result.reg[1] = _mm_add_ps(lhs.reg[1], temp);
        return result;
    }

    static inline simd sub(const simd &lhs, f32 rhs) {
        simd result;
        __m128 temp = _mm_set1_ps(rhs);
        result.reg[0] = _mm_sub_ps(lhs.reg[0], temp);
        result.reg[1] = _mm_sub_ps(lhs.reg[1], temp);
        return result;
    }

    static inline simd mad(const simd &a, const simd &b, const simd &c) { return add(mul(a, b), c); }

    static inline simd spread(f32 value) {
        simd result;
        result.reg[0] = _mm_set1_ps(value);
        result.reg[1] = _mm_set1_ps(value);
        return result;
    }

    static inline simd set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) {
        simd result;
        result.reg[0] = _mm_setr_ps(a, b, c, d);
        result.reg[1] = _mm_setr_ps(e, f, g, h);
        return result;
    }

    template <s64 Count>
    static inline f32 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 8, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(0 < Count, "Count must not be zero.");

        __m128 reg1, reg2;
        reg1 = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
        reg2 = _mm_mul_ps(lhs.reg[1], rhs.reg[1]);

        for (s64 i = 7; i >= Count && i >= 4; --i) {
            ((f32 *) &reg2)[i] = 0.0f;
        }
        for (s64 i = 3; i >= Count && i >= 0; --i) {
            ((f32 *) &reg1)[i] = 0.0f;
        }

        f32 sum;
        reg1 = _mm_add_ps(reg1, reg2);
        sum = ((f32 *) &reg1)[0] + ((f32 *) &reg1)[1] + ((f32 *) &reg1)[2] + ((f32 *) &reg1)[3];
        return sum;
    }

    template <s64 i0, s64 i1, s64 i2, s64 i3, s64 i4, s64 i5, s64 i6, s64 i7>
    static inline simd shuffle(const simd &arg) {
        simd result;

        auto *v1 = result.reg[0].m128_f32;
        auto *v2 = arg.reg[0].m128_f32;
        v1[7] = v2[i0];
        v1[6] = v2[i1];
        v1[5] = v2[i2];
        v1[4] = v2[i3];
        v1[3] = v2[i4];
        v1[2] = v2[i5];
        v1[1] = v2[i6];
        v1[0] = v2[i7];
        return result;
    }
};

// Specialization for 2f64, using SSE
template <>
struct alignas(16) simd<f64, 2> {
    __m128d reg;

    static inline simd mul(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_mul_pd(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd div(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_div_pd(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd add(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_add_pd(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd sub(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg = _mm_sub_pd(lhs.reg, rhs.reg);
        return result;
    }

    static inline simd mul(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg = _mm_mul_pd(lhs.reg, temp);
        return result;
    }

    static inline simd div(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg = _mm_div_pd(lhs.reg, temp);
        return result;
    }

    static inline simd add(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg = _mm_add_pd(lhs.reg, temp);
        return result;
    }

    static inline simd sub(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg = _mm_sub_pd(lhs.reg, temp);
        return result;
    }

    static inline simd mad(const simd &a, const simd &b, const simd &c) { return add(mul(a, b), c); }

    static inline simd spread(f64 value) {
        simd result;
        result.reg = _mm_set1_pd(value);
        return result;
    }

    static inline simd set(f64 x, f64 y) {
        simd result;
        result.reg = _mm_setr_pd(x, y);
        return result;
    }

    template <s64 Count>
    static inline f64 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 2, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(0 < Count, "Count must not be zero.");

        f64 sum;
        simd m = mul(lhs, rhs);
        auto *v = m.reg.m128d_f64;
        sum = v[0];
        for (s64 i = 1; i < Count; ++i) {
            sum += v[i];
        }
        return sum;
    }

    template <s64 i0, s64 i1>
    static inline simd shuffle(const simd &arg) {
        simd result;
        result.reg = _mm_shuffle_pd(arg.reg, arg.reg, _MM_SHUFFLE2(i0, i1));
        return result;
    }
};

// Specialization for 4f64, using SSE
//*
template <>
struct alignas(16) simd<f64, 4> {
    __m128d reg[2];

    static inline simd mul(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_mul_pd(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_mul_pd(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd div(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_div_pd(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_div_pd(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd add(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_add_pd(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_add_pd(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd sub(const simd &lhs, const simd &rhs) {
        simd result;
        result.reg[0] = _mm_sub_pd(lhs.reg[0], rhs.reg[0]);
        result.reg[1] = _mm_sub_pd(lhs.reg[1], rhs.reg[1]);
        return result;
    }

    static inline simd mul(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg[0] = _mm_mul_pd(lhs.reg[0], temp);
        result.reg[1] = _mm_mul_pd(lhs.reg[1], temp);
        return result;
    }

    static inline simd div(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg[0] = _mm_div_pd(lhs.reg[0], temp);
        result.reg[1] = _mm_div_pd(lhs.reg[1], temp);
        return result;
    }

    static inline simd add(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg[0] = _mm_add_pd(lhs.reg[0], temp);
        result.reg[1] = _mm_add_pd(lhs.reg[1], temp);
        return result;
    }

    static inline simd sub(const simd &lhs, f64 rhs) {
        simd result;
        __m128d temp = _mm_set1_pd(rhs);
        result.reg[0] = _mm_sub_pd(lhs.reg[0], temp);
        result.reg[1] = _mm_sub_pd(lhs.reg[1], temp);
        return result;
    }

    static inline simd mad(const simd &a, const simd &b, const simd &c) { return add(mul(a, b), c); }

    static inline simd spread(f64 value) {
        simd result;
        result.reg[0] = _mm_set1_pd(value);
        result.reg[1] = _mm_set1_pd(value);
        return result;
    }

    static inline simd set(f64 x, f64 y, f64 z, f64 w) {
        simd result;
        result.reg[0] = _mm_setr_pd(x, y);
        result.reg[1] = _mm_setr_pd(z, w);
        return result;
    }

    template <s64 Count>
    static inline f64 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 4, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(0 < Count, "Count must not be zero.");

        __m128d regs[2];
        regs[0] = _mm_mul_pd(lhs.reg[0], rhs.reg[0]);
        regs[1] = _mm_mul_pd(lhs.reg[1], rhs.reg[1]);

        for (s64 i = 3; i >= Count; --i) {
            (f64 *) (&regs)[i] = 0.0;
        }

        f64 sum;
        regs[0] = _mm_add_pd(regs[0], regs[1]);
        sum = (f64 *) (&regs[0])[0] + (f64 *) (&regs[0])[1];

        return sum;
    }

    template <s64 i0, s64 i1, s64 i2, s64 i3>
    static inline simd shuffle(const simd &arg) {
        simd result;

        auto *v1 = result.reg[0].m128d_f64;
        auto *v2 = arg.reg[0].m128d_f64;
        v1[3] = v2[i0];
        v1[2] = v2[i1];
        v1[1] = v2[i2];
        v1[0] = v2[i3];
        return result;
    }
};

LSTD_END_NAMESPACE
