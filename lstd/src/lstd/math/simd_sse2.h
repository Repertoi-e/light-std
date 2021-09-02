#pragma once

#include <emmintrin.h>

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Specialization for 4xf32, using SSE
template <>
union alignas(16) simd<f32, 4> {
    __m128 reg;
    __m128i regi;
    f32 v[4];

    static simd mul(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_mul_ps(lhs.reg, rhs.reg);
        return r;
    }

    static simd div(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_div_ps(lhs.reg, rhs.reg);
        return r;
    }

    static simd add(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_add_ps(lhs.reg, rhs.reg);
        return r;
    }

    static simd sub(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_sub_ps(lhs.reg, rhs.reg);
        return r;
    }

    static simd mul(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg       = _mm_mul_ps(lhs.reg, temp);
        return r;
    }

    static simd div(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg       = _mm_div_ps(lhs.reg, temp);
        return r;
    }

    static simd add(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg       = _mm_add_ps(lhs.reg, temp);
        return r;
    }

    static simd sub(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg       = _mm_sub_ps(lhs.reg, temp);
        return r;
    }

    static simd spread(f32 value) {
        simd r;
        r.reg = _mm_set1_ps(value);
        return r;
    }

    static simd set(f32 x, f32 y, f32 z, f32 w) {
        simd r;
        r.reg = _mm_setr_ps(x, y, z, w);
        return r;
    }

    template <s32 Count>
    static f32 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 4, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(Count > 0, "Count must not be zero.");

        simd m  = mul(lhs, rhs);
        f32 sum = m.v[0];
        for (s32 i = 1; i < Count; ++i) {
            sum += m.v[i];
        }
        return sum;
    }

    template <s32 i0, s32 i1, s32 i2, s32 i3>
    static simd shuffle(const simd &arg) {
        simd r;
        r.regi = _mm_shuffle_epi32(arg.regi, _MM_SHUFFLE(i0, i1, i2, i3));
        return r;
    }
};

// Specialization for 8xf32, using SSE
template <>
union alignas(16) simd<f32, 8> {
    __m128 reg[2];
    f32 v[8];

    static simd mul(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_mul_ps(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd div(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_div_ps(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_div_ps(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd add(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_add_ps(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_add_ps(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd sub(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_sub_ps(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_sub_ps(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd mul(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg[0]    = _mm_mul_ps(lhs.reg[0], temp);
        r.reg[1]    = _mm_mul_ps(lhs.reg[1], temp);
        return r;
    }

    static simd div(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg[0]    = _mm_div_ps(lhs.reg[0], temp);
        r.reg[1]    = _mm_div_ps(lhs.reg[1], temp);
        return r;
    }

    static simd add(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg[0]    = _mm_add_ps(lhs.reg[0], temp);
        r.reg[1]    = _mm_add_ps(lhs.reg[1], temp);
        return r;
    }

    static simd sub(const simd &lhs, f32 rhs) {
        simd r;
        __m128 temp = _mm_set1_ps(rhs);
        r.reg[0]    = _mm_sub_ps(lhs.reg[0], temp);
        r.reg[1]    = _mm_sub_ps(lhs.reg[1], temp);
        return r;
    }

    static simd spread(f32 value) {
        simd r;
        r.reg[0] = _mm_set1_ps(value);
        r.reg[1] = _mm_set1_ps(value);
        return r;
    }

    static simd set(f32 a, f32 b, f32 c, f32 d, f32 e, f32 f, f32 g, f32 h) {
        simd r;
        r.reg[0] = _mm_setr_ps(a, b, c, d);
        r.reg[1] = _mm_setr_ps(e, f, g, h);
        return r;
    }

    template <s32 Count>
    static f32 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 8, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(Count > 0, "Count must not be zero.");
        __m128 reg1, reg2;
        reg1 = _mm_mul_ps(lhs.reg[0], rhs.reg[0]);
        reg2 = _mm_mul_ps(lhs.reg[1], rhs.reg[1]);

        for (s32 i = 7; i >= Count && i >= 4; --i) {
            reinterpret_cast<f32 *>(&reg2)[i] = 0.0f;
        }
        for (s32 i = 3; i >= Count && i >= 0; --i) {
            reinterpret_cast<f32 *>(&reg1)[i] = 0.0f;
        }

        reg1    = _mm_add_ps(reg1, reg2);
        f32 sum = reinterpret_cast<f32 *>(&reg1)[0] + reinterpret_cast<f32 *>(&reg1)[1] +
                  reinterpret_cast<f32 *>(&reg1)[2] + reinterpret_cast<f32 *>(&reg1)[3];

        return sum;
    }

    template <s32 i0, s32 i1, s32 i2, s32 i3, s32 i4, s32 i5, s32 i6, s32 i7>
    static simd shuffle(const simd &arg) {
        simd r;
        r.v[7] = arg.v[i0];
        r.v[6] = arg.v[i1];
        r.v[5] = arg.v[i2];
        r.v[4] = arg.v[i3];
        r.v[3] = arg.v[i4];
        r.v[2] = arg.v[i5];
        r.v[1] = arg.v[i6];
        r.v[0] = arg.v[i7];
        return r;
    }
};

// Specialization for 2xf64, using SSE
template <>
union alignas(16) simd<f64, 2> {
    __m128d reg;
    f64 v[4];

    static simd mul(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_mul_pd(lhs.reg, rhs.reg);
        return r;
    }

    static simd div(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_div_pd(lhs.reg, rhs.reg);
        return r;
    }

    static simd add(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_add_pd(lhs.reg, rhs.reg);
        return r;
    }

    static simd sub(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg = _mm_sub_pd(lhs.reg, rhs.reg);
        return r;
    }

    static simd mul(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg        = _mm_mul_pd(lhs.reg, temp);
        return r;
    }

    static simd div(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg        = _mm_div_pd(lhs.reg, temp);
        return r;
    }

    static simd add(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg        = _mm_add_pd(lhs.reg, temp);
        return r;
    }

    static simd sub(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg        = _mm_sub_pd(lhs.reg, temp);
        return r;
    }

    static simd spread(f64 value) {
        simd r;
        r.reg = _mm_set1_pd(value);
        return r;
    }

    static simd set(f64 x, f64 y) {
        simd r;
        r.reg = _mm_setr_pd(x, y);
        return r;
    }

    template <s32 Count>
    static f64 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 2, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(Count > 0, "Count must not be zero.");
        simd m  = mul(lhs, rhs);
        f64 sum = m.v[0];
        for (s32 i = 1; i < Count; ++i) {
            sum += m.v[i];
        }
        return sum;
    }

    template <s32 i0, s32 i1>
    static simd shuffle(const simd &arg) {
        simd r;
        r.reg = _mm_shuffle_pd(arg.reg, arg.reg, _MM_SHUFFLE2(i0, i1));
        return r;
    }
};

// Specialization for 4xf64, using SSE
template <>
union alignas(16) simd<f64, 4> {
    __m128d reg[2];
    f64 v[4];

    static simd mul(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_mul_pd(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_mul_pd(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd div(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_div_pd(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_div_pd(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd add(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_add_pd(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_add_pd(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd sub(const simd &lhs, const simd &rhs) {
        simd r;
        r.reg[0] = _mm_sub_pd(lhs.reg[0], rhs.reg[0]);
        r.reg[1] = _mm_sub_pd(lhs.reg[1], rhs.reg[1]);
        return r;
    }

    static simd mul(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg[0]     = _mm_mul_pd(lhs.reg[0], temp);
        r.reg[1]     = _mm_mul_pd(lhs.reg[1], temp);
        return r;
    }

    static simd div(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg[0]     = _mm_div_pd(lhs.reg[0], temp);
        r.reg[1]     = _mm_div_pd(lhs.reg[1], temp);
        return r;
    }

    static simd add(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg[0]     = _mm_add_pd(lhs.reg[0], temp);
        r.reg[1]     = _mm_add_pd(lhs.reg[1], temp);
        return r;
    }

    static simd sub(const simd &lhs, f64 rhs) {
        simd r;
        __m128d temp = _mm_set1_pd(rhs);
        r.reg[0]     = _mm_sub_pd(lhs.reg[0], temp);
        r.reg[1]     = _mm_sub_pd(lhs.reg[1], temp);
        return r;
    }

    static simd spread(f64 value) {
        simd r;
        r.reg[0] = _mm_set1_pd(value);
        r.reg[1] = _mm_set1_pd(value);
        return r;
    }

    static simd set(f64 x, f64 y, f64 z, f64 w) {
        simd r;
        r.reg[0] = _mm_setr_pd(x, y);
        r.reg[1] = _mm_setr_pd(z, w);
        return r;
    }

    template <s32 Count>
    static f64 dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= 4, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(Count > 0, "Count must not be zero.");
        __m128d regs[2];
        regs[0] = _mm_mul_pd(lhs.reg[0], rhs.reg[0]);
        regs[1] = _mm_mul_pd(lhs.reg[1], rhs.reg[1]);

        for (s32 i = 3; i >= Count; --i) {
            reinterpret_cast<f64 *>(&regs)[i] = 0.0;
        }

        regs[0] = _mm_add_pd(regs[0], regs[1]);
        f64 sum = reinterpret_cast<f64 *>(&regs[0])[0] + reinterpret_cast<f64 *>(&regs[0])[1];

        return sum;
    }

    template <s32 i0, s32 i1, s32 i2, s32 i3>
    static simd shuffle(const simd &arg) {
        simd r;
        r.v[3] = arg.v[i0];
        r.v[2] = arg.v[i1];
        r.v[1] = arg.v[i2];
        r.v[0] = arg.v[i3];
        return r;
    }
};

LSTD_END_NAMESPACE
