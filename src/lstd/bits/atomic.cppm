module;

#include "../common/namespace.h"

#include <intrin.h>

export module lstd.atomic;

export import lstd.math;

//
// Atomic operations: atomic_inc, atomic_add, atomic_swap, atomic_compare_and_swap
//

LSTD_BEGIN_NAMESPACE

template <typename T>
const bool is_appropriate_size_for_atomic_v = (sizeof(T) == 2) || (sizeof(T) == 4) || (sizeof(T) == 8);

template <typename T>
concept appropriate_for_atomic = (is_integral<T> || is_enum<T> || is_pointer<T>) &&is_appropriate_size_for_atomic_v<T>;

export {
#if COMPILER == MSVC

    // Returns the initial value in _ptr_
    template <appropriate_for_atomic T>
    T atomic_inc(T* ptr) {
        if constexpr (sizeof(T) == 2) return (T)_InterlockedIncrement16((volatile short*)ptr);
        if constexpr (sizeof(T) == 4) return (T)_InterlockedIncrement((volatile long*)ptr);
        if constexpr (sizeof(T) == 8) return (T)_InterlockedIncrement64((volatile long long*)ptr);
    }

    // Returns the initial value in _ptr_
    template <appropriate_for_atomic T>
    T atomic_add(T* ptr, T value) {
        if constexpr (sizeof(T) == 2) return (T)_InterlockedExchangeAdd16((volatile short*)ptr, (short)value);
        if constexpr (sizeof(T) == 4) return (T)_InterlockedExchangeAdd((volatile long*)ptr, (long)value);
        if constexpr (sizeof(T) == 8) return (T)_InterlockedExchangeAdd64((volatile long long*)ptr, (long long)value);
    }

    // Returns the old value in _ptr_
    template <appropriate_for_atomic T>
    T atomic_swap(T* ptr, T value) {
        if constexpr (sizeof(T) == 2) return (T)_InterlockedExchange16((volatile short*)ptr, (short)value);
        if constexpr (sizeof(T) == 4) return (T)_InterlockedExchange((volatile long*)ptr, (long)value);
        if constexpr (sizeof(T) == 8) return (T)_InterlockedExchange64((volatile long long*)ptr, (long long)value);
    }

    // Returns the old value in _ptr_, exchanges values only if the old value is equal to _oldValue_.
    // You can use this for a safe way to read a value, e.g. atomic_compare_and_swap(&value, 0, 0).
    //
    // It's also exceedingly useful for implementing lock-free algorithms and data structures.
    //
    // Note: ABA problem. Check it out.
    template <appropriate_for_atomic T>
     T atomic_compare_and_swap(T* ptr, T oldValue, T newValue) {
        if constexpr (sizeof(T) == 2) return (T)_InterlockedCompareExchange16((volatile short*)ptr, (short)newValue, (short)oldValue);
        if constexpr (sizeof(T) == 4) return (T)_InterlockedCompareExchange((volatile long*)ptr, (long)newValue, (long)oldValue);
        if constexpr (sizeof(T) == 8) return (T)_InterlockedCompareExchange64((volatile long long*)ptr, (long long)newValue, (long long)oldValue);
    }
#else
#error Implement.
#endif
}

LSTD_END_NAMESPACE
