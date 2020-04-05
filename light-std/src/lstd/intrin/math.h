#pragma once

#include "general.h"

LSTD_BEGIN_NAMESPACE

template <typename T>
constexpr T const_exp10(s32 exponent) {
    return exponent == 0 ? T(1) : T(10) * const_exp10<T>(exponent - 1);
}

LSTD_END_NAMESPACE
