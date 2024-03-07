#pragma once

#include "lstd/math.h"

LSTD_BEGIN_NAMESPACE

//
// Shortcut macros for "for each" loops (really up to personal style if you want
// to use this)
//
//  For(array) print(it);
//
#define For_as(x, in) for (auto &&x : in)
#define For(in) For_as(it, in)

//
// Loop that gets unrolled at compile-time, this avoids copy-pasta
// or using macros in order to be sure the code gets unrolled properly.
//
template <s64 First, s64 Last, typename Lambda>
void static_for(Lambda f) {
  if constexpr (First < Last) {
    f(integral_constant<s64, First>{});
    static_for<First + 1, Last>(f);
  }
}

LSTD_END_NAMESPACE
