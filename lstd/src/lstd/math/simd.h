#pragma once

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Default implementation without SSE.
// Additional implementation are included as template specializations.

// @Volatile Currently:
// 2,4 or 8 dimension f32 or f64 parameters accepted.
// Uses SSE2 or AVX acceleration if enabled in the compiler.
template <typename T, s64 Dim>
union alignas(16) simd {
    // @Cleanup: This looks messy
    static_assert(Dim == 2 || Dim == 4 || Dim == 8, "Dimension must be 2, 4, or 8.");
    static_assert(types::is_same<T, f32> || types::is_same<T, f64> || types::is_same<T, s32> || types::is_same<T, s64>, "Type must be f32, f64, s32 or s64.");

    T reg[Dim];

    static simd mul(const simd &lhs, const simd &rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] * rhs.reg[i];
        return result;
    }

    static simd div(const simd &lhs, const simd &rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] / rhs.reg[i];
        return result;
    }

    static simd add(const simd &lhs, const simd &rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] + rhs.reg[i];
        return result;
    }

    static simd sub(const simd &lhs, const simd &rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] - rhs.reg[i];
        return result;
    }

    static simd mul(const simd &lhs, T rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] * rhs;
        return result;
    }

    static simd div(const simd &lhs, T rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] / rhs;
        return result;
    }

    static simd add(const simd &lhs, T rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] + rhs;
        return result;
    }

    static simd sub(const simd &lhs, T rhs) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = lhs.reg[i] - rhs;
        return result;
    }

    static simd spread(T value) {
        simd result;
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = value;
        return result;
    }

    template <typename... Args>
    static simd set(Args ... args) {
        simd result;

        static_assert(sizeof...(Args) == Dim, "Number of arguments must be equal to dimension.");

        T table[] = {T(args)...};
        for (s64 i = 0; i < Dim; ++i) result.reg[i] = table[i];
        return result;
    }

    template <s64 Count = Dim>
    static T dot(const simd &lhs, const simd &rhs) {
        static_assert(Count <= Dim, "Number of elements to dot must be smaller or equal to dimension.");
        static_assert(Count > 0, "Count must not be zero.");

        T sum = lhs.reg[0] * rhs.reg[0];
        for (s64 i = 1; i < Count; ++i) sum += lhs.reg[i] * rhs.reg[i];
        return sum;
    }

    template <s64 i0, s64 i1>
    static simd shuffle(simd arg) {
        static_assert(Dim == 2, "Only for 2-way simd.");

        simd result;
        result.reg[1] = arg.reg[i0];
        result.reg[0] = arg.reg[i1];
        return result;
    }

    template <s64 i0, s64 i1, s64 i2, s64 i3>
    static simd shuffle(simd arg) {
        static_assert(Dim == 4, "Only for 4-way simd.");

        simd result;
        result.reg[3] = arg.reg[i0];
        result.reg[2] = arg.reg[i1];
        result.reg[1] = arg.reg[i2];
        result.reg[0] = arg.reg[i3];
        return result;
    }

    template <s64 i0, s64 i1, s64 i2, s64 i3, s64 i4, s64 i5, s64 i6, s64 i7>
    static simd shuffle(simd arg) {
        static_assert(Dim == 8, "Only for 8-way simd.");

        simd result;
        result.reg[7] = arg.reg[i0];
        result.reg[6] = arg.reg[i1];
        result.reg[5] = arg.reg[i2];
        result.reg[4] = arg.reg[i3];
        result.reg[3] = arg.reg[i4];
        result.reg[2] = arg.reg[i5];
        result.reg[1] = arg.reg[i6];
        result.reg[0] = arg.reg[i7];
        return result;
    }
};

LSTD_END_NAMESPACE

#include "simd_sse2.h"
