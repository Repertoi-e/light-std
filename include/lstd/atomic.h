#pragma once

#include "lstd/math.h"

//
// Atomic operations: atomic_inc, atomic_add, atomic_swap,
// atomic_compare_and_swap
//

LSTD_BEGIN_NAMESPACE

template <typename T>
const bool is_appropriate_size_for_atomic_v = (sizeof(T) == 2) ||
                                              (sizeof(T) == 4) ||
                                              (sizeof(T) == 8);

template <typename T>
concept appropriate_for_atomic =
    (is_integral<T> || is_enum<T> ||
     is_pointer<T>)&&is_appropriate_size_for_atomic_v<T>;

#if COMPILER == MSVC
extern "C" {
short __cdecl _InterlockedIncrement16(short volatile *_Addend);
long __cdecl _InterlockedIncrement(long volatile *_Addend);
__int64 __cdecl _InterlockedIncrement64(__int64 volatile *_Addend);
short __cdecl _InterlockedExchangeAdd16(short volatile *_Addend, short _Value);
long __cdecl _InterlockedExchangeAdd(long volatile *_Addend, long _Value);
long long __cdecl _InterlockedExchangeAdd64(long long volatile *_Addend,
                                            long long _Value);
short __cdecl _InterlockedExchange16(short volatile *_Target, short _Value);
long __cdecl _InterlockedExchange(long volatile *_Target, long _Value);
long long __cdecl _InterlockedExchange64(long long volatile *_Target,
                                         long long _Value);

short __cdecl _InterlockedCompareExchange16(short volatile *_Destination,
                                            short _Exchange, short _Comparand);
long __cdecl _InterlockedCompareExchange(long volatile *_Destination,
                                         long _Exchange, long _Comparand);
long long __cdecl _InterlockedCompareExchange64(
    long long volatile *_Destination, long long _Exchange,
    long long _Comparand);
}

// Returns the initial value in _ptr_
template <appropriate_for_atomic T>
T atomic_inc(T *ptr) {
  if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedIncrement16((volatile short *)ptr);
  }
  if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedIncrement((volatile long *)ptr);
  }
#if BITS == 64
  if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedIncrement64((volatile long long *)ptr);
  }
#else
  assert(false && "Trying to atomic_inc on a 32 bit platform.");
#endif
}

// Returns the initial value in _ptr_
template <appropriate_for_atomic T>
T atomic_add(T *ptr, T value) {
  if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedExchangeAdd16((volatile short *)ptr, (short)value);
  }
  if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedExchangeAdd((volatile long *)ptr, (long)value);
  }
#if BITS == 64
  if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedExchangeAdd64((volatile long long *)ptr,
                                        (long long)value);
  }
#else
  assert(false && "Trying to atomic_add on a 32 bit platform.");
#endif
}

// Returns the old value in _ptr_
template <appropriate_for_atomic T>
T atomic_swap(T *ptr, T value) {
  if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedExchange16((volatile short *)ptr, (short)value);
  }
  if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedExchange((volatile long *)ptr, (long)value);
  }
#if BITS == 64
  if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedExchange64((volatile long long *)ptr,
                                     (long long)value);
#else
  assert(false && "Trying to atomic_swap on a 32 bit platform.");
#endif
  }
}

// Returns the old value in _ptr_, exchanges values only if the old value is
// equal to _oldValue_. You can use this for a safe way to read a value, e.g.
// atomic_compare_and_swap(&value, 0, 0).
//
// It's also exceedingly useful for implementing lock-free algorithms and data
// structures.
//
// Note: ABA problem. Check it out.
template <appropriate_for_atomic T>
T atomic_compare_and_swap(T *ptr, T oldValue, T newValue) {
  if constexpr (sizeof(T) == 2) {
    return (T)_InterlockedCompareExchange16((volatile short *)ptr,
                                            (short)newValue, (short)oldValue);
  }
  if constexpr (sizeof(T) == 4) {
    return (T)_InterlockedCompareExchange((volatile long *)ptr, (long)newValue,
                                          (long)oldValue);
  }
#if BITS == 64
  if constexpr (sizeof(T) == 8) {
    return (T)_InterlockedCompareExchange64(
        (volatile long long *)ptr, (long long)newValue, (long long)oldValue);
  }
#else
  assert(false && "Trying to atomic_swap on a 32 bit platform.");
#endif
}
#else
// Returns the initial value in _ptr_
template <appropriate_for_atomic T>
T atomic_inc(T *ptr) {
  return __sync_add_and_fetch(ptr, 1);
}

// Returns the initial value in _ptr_
template <appropriate_for_atomic T>
T atomic_add(T *ptr, T value) {
  return __sync_add_and_fetch(ptr, value);
}

// Returns the old value in _ptr_
template <appropriate_for_atomic T>
T atomic_swap(T *ptr, T value) {
  return __sync_lock_test_and_set(ptr, value);
}

// Returns the old value in _ptr_, exchanges values only if the old value is
// equal to _oldValue_. You can use this for a safe way to read a value, e.g.
// atomic_compare_and_swap(&value, 0, 0).
//
// It's also exceedingly useful for implementing lock-free algorithms and data
// structures.
//
// Note: ABA problem. Check it out.
template <appropriate_for_atomic T>
T atomic_compare_and_swap(T *ptr, T oldValue, T newValue) {
  return __sync_val_compare_and_swap(ptr, oldValue, newValue);
}
#endif

LSTD_END_NAMESPACE
