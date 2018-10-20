#pragma once

#include "format_core.h"

#if !defined COMPILER_MSVC
#define CLZ(n) __builtin_clz(n)
#define CLZLL(n) __builtin_clzll(n)
#endif

#if defined COMPILER_MSVC
#include <intrin.h>
GU_BEGIN_NAMESPACE
namespace fmt::internal {
#pragma intrinsic(_BitScanReverse)

inline u32 clz(u32 x) {
    assert(x != 0);

    unsigned long r = 0;
    _BitScanReverse(&r, x);
    return 31 - r;
}
#define CLZ(n) internal::clz(n)

#if defined PROCESSOR_X64
#pragma intrinsic(_BitScanReverse64)
#endif

inline u32 clzll(u64 x) {
    assert(x != 0);

    unsigned long r = 0;
#ifdef PROCESSOR_X64
    _BitScanReverse64(&r, x);
#else
    // Scan the high 32 bits.
    if (_BitScanReverse(&r, (u32)(x >> 32))) return 63 - (r + 32);
    // Scan the low 32 bits.
    _BitScanReverse(&r, (u32) x);
#endif
    return 63 - r;
}
#define CLZLL(n) internal::clzll(n)
}  // namespace fmt::internal
GU_END_NAMESPACE
#endif

namespace fmt::internal {
// An equivalent of `*(Dest*) (&source)` that doesn't produce
// undefined behavior (e.g. due to type aliasing).
template <typename T, typename U>
inline T bit_cast(const U& source) {
    static_assert(sizeof(T) == sizeof(U), "size mismatch");
    T result;
    CopyMemory(&result, &source, sizeof(T));
    return result;
}

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
constexpr typename std::enable_if_t<std::numeric_limits<T>::is_signed, bool> is_negative(T value) {
    return value < 0;
}

template <typename T>
constexpr typename std::enable_if_t<!std::numeric_limits<T>::is_signed, bool> is_negative(T) {
    return false;
}

// Casts nonnegative integer to unsigned.
template <typename T>
constexpr typename std::make_unsigned_t<T> to_unsigned(T value) {
	assert(value >= 0);
	return (typename std::make_unsigned_t<T>) value;
}

// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline unsigned count_digits(u64 n) {
	s32 t = (64 - CLZLL(n | 1)) * 1233 >> 12;
	return to_unsigned(t) - (n < data::ZERO_OR_POWERS_OF_10_64[t]) + 1;
}

}  // namespace fmt::internal

#undef CLZ(n)
#undef CLZLL(n)
