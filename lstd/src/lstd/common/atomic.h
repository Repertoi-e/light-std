#pragma once

#include <intrin.h>

#include "type_info.h"

//
// Atomic operations:
//

LSTD_BEGIN_NAMESPACE

template <typename T>
constexpr bool is_appropriate_size_for_atomic_v = (sizeof(T) == 2) || (sizeof(T) == 4) || (sizeof(T) == 8);

template <typename T>
concept appropriate_for_atomic = (types::is_integral<T> || types::is_enum<T> || types::is_pointer<T>) &&is_appropriate_size_for_atomic_v<T>;

#if COMPILER == MSVC

// Returns the initial value in _ptr_
template <appropriate_for_atomic T>
always_inline constexpr T atomic_inc(T *ptr) {
    if constexpr (sizeof(T) == 2) return (T) _InterlockedIncrement16((volatile short *) ptr);
    if constexpr (sizeof(T) == 4) return (T) _InterlockedIncrement((volatile long *) ptr);
    if constexpr (sizeof(T) == 8) return (T) _InterlockedIncrement64((volatile long long *) ptr);
}

// Returns the initial value in _ptr_
template <appropriate_for_atomic T>
always_inline constexpr T atomic_add(T *ptr, T value) {
    if constexpr (sizeof(T) == 2) return (T) _InterlockedExchangeAdd16((volatile short *) ptr, (short) value);
    if constexpr (sizeof(T) == 4) return (T) _InterlockedExchangeAdd((volatile long *) ptr, (long) value);
    if constexpr (sizeof(T) == 8) return (T) _InterlockedExchangeAdd64((volatile long long *) ptr, (long long) value);
}

// Returns the old value in _ptr_
template <appropriate_for_atomic T>
always_inline constexpr T atomic_swap(T *ptr, T value) {
    if constexpr (sizeof(T) == 2) return (T) _InterlockedExchange16((volatile short *) ptr, (short) value);
    if constexpr (sizeof(T) == 4) return (T) _InterlockedExchange((volatile long *) ptr, (long) value);
    if constexpr (sizeof(T) == 8) return (T) _InterlockedExchange64((volatile long long *) ptr, (long long) value);
}

// Returns the old value in _ptr_, exchanges values only if the old value is equal to _oldValue_.
// You can use this for a safe way to read a value, e.g. atomic_compare_and_swap(&value, 0, 0).
//
// It's also exceedingly useful for implementing lock-free algorithms and data structures.
//
// Note: ABA problem. Check it out.
template <appropriate_for_atomic T>
always_inline constexpr T atomic_compare_and_swap(T *ptr, T oldValue, T newValue) {
    if constexpr (sizeof(T) == 2) return (T) _InterlockedCompareExchange16((volatile short *) ptr, (short) newValue, (short) oldValue);
    if constexpr (sizeof(T) == 4) return (T) _InterlockedCompareExchange((volatile long *) ptr, (long) newValue, (long) oldValue);
    if constexpr (sizeof(T) == 8) return (T) _InterlockedCompareExchange64((volatile long long *) ptr, (long long) newValue, (long long) oldValue);
}
#else
#define atomic_inc(ptr) __sync_add_and_fetch((ptr), 1)
#define atomic_inc_64(ptr) __sync_add_and_fetch((ptr), 1)
#error Some atomic operations not implemented
#endif

// Function for swapping endianness. You can check for the endianness by using #if ENDIAN = LITTLE_ENDIAN, etc.
always_inline constexpr void byte_swap_2(void *ptr) {
    u16 x        = *(u16 *) ptr;
    *(u16 *) ptr = x << 8 & 0xFF00 | x >> 8 & 0x00FF;
}

// Function for swapping endianness. You can check for the endianness by using #if ENDIAN = LITTLE_ENDIAN, etc.
always_inline constexpr void byte_swap_4(void *ptr) {
    u32 x        = *(u32 *) ptr;
    *(u32 *) ptr = x << 24 & 0xFF000000 | x << 8 & 0x00FF0000 | x >> 8 & 0x0000FF00 | x >> 24 & 0x000000FF;
}

// Function for swapping endianness. You can check for the endianness by using #if ENDIAN = LITTLE_ENDIAN, etc.
always_inline constexpr void byte_swap_8(void *ptr) {
    u64 x        = *(u64 *) ptr;
    x            = ((x << 8) & 0xFF00FF00FF00FF00ULL) | ((x >> 8) & 0x00FF00FF00FF00FFULL);
    x            = ((x << 16) & 0xFFFF0000FFFF0000ULL) | ((x >> 16) & 0x0000FFFF0000FFFFULL);
    *(u64 *) ptr = (x << 32) | (x >> 32);
}

LSTD_END_NAMESPACE
