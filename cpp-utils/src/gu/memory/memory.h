#pragma once

#include "../common.h"
#include "../context.h"

#include <new>

GU_BEGIN_NAMESPACE

template <typename T>
inline T *New(Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) allocator = CONTEXT_ALLOC;
    return new (allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, sizeof(T), 0, 0, 0)) T;
}

template <typename T>
inline T *New(size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) allocator = CONTEXT_ALLOC;

    T *result = (T *) allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, count * sizeof(T), 0, 0, 0);
    for (size_t i = 0; i < count; i++) new (result + i) T;
    return result;
}

template <typename T>
inline void Delete(T *memory, Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) allocator = CONTEXT_ALLOC;

    memory->~T();
    allocator.Function(Allocator_Mode::FREE, allocator.Data, 0, memory, sizeof(T), 0);
}

template <typename T>
inline void Delete(T *memory, size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) allocator = CONTEXT_ALLOC;

    for (size_t i = 0; i < count; i++) (memory + i)->~T();
    allocator.Function(Allocator_Mode::FREE, allocator.Data, 0, memory, count * sizeof(T), 0);
}

// Windows.h defines CopyMemory as memcpy, our version is compatible so it's safe to undefine it
#if defined CopyMemory
#undef CopyMemory
#endif
#if defined MoveMemory
#undef MoveMemory
#endif
#if defined FillMemory
#undef FillMemory
#endif
#if defined ZeroMemory
#undef ZeroMemory
#endif

#if !defined GU_NO_CRT
#include <cstring>

inline void *CopyMemory(void *dest, void const *src, size_t num) { return memcpy(dest, src, num); }
inline void *MoveMemory(void *dest, void const *src, size_t num) { return memmove(dest, src, num); }
inline void *FillMemory(void *dest, int value, size_t num) { return memset(dest, value, num); }

#else
// Defined in memory.cpp
void *CopyMemory(void *dest, void const *src, size_t num);
void *MoveMemory(void *dest, void const *src, size_t num);
void *FillMemory(void *dest, int value, size_t num);
extern "C" {
// Defining intrinsic functions that the compiler may use to optimize even though we aren't linking with CRT :thunk:
inline void *memcpy(void *dest, void const *src, size_t num) { return CopyMemory(dest, src, num); }
inline void *memmove(void *dest, void const *src, size_t num) { return MoveMemory(dest, src, num); }
inline void *memset(void *dest, int value, size_t num) { return FillMemory(dest, value, num); }
}
#endif

// Helper function that fills memory with 0
inline void *ZeroMemory(void *dest, size_t num) { return FillMemory(dest, 0, num); }

// Helper template function for copying contents of arrays of the same type
template <typename T>
inline T *CopyElements(T *dest, T *src, size_t numberOfElements) {
    static_assert(!std::is_same_v<T, void>);

    CopyMemory(dest, src, sizeof(T) * numberOfElements);
    return dest;
}

GU_END_NAMESPACE
