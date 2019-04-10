#pragma once

#include "../types.h"

#define INTEGRAL_FUNCTION_CONSTEXPR(return_type) \
    template <typename T>                        \
    constexpr enable_if_t<is_integral_v<T>, return_type>

INTEGRAL_FUNCTION_CONSTEXPR(bool) is_power_of_2(T number) { return (number & (number - 1)) == 0; }

INTEGRAL_FUNCTION_CONSTEXPR(T) round_up_to_multiple_of_power_of_2(T number, T pow2) {
    static_assert(is_power_of_2(pow2), "Argument is not a power of 2");
    return (number + pow2 - 1) & ~(pow2 - 1);
}

#undef INTEGRAL_FUNCTION_CONSTEXPR