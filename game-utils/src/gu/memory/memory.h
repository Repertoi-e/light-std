#pragma once

#include "../common.h"
#include "../context.h"

#include <new>

GU_BEGIN_NAMESPACE

template <typename T>
inline T *New(Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) {
        allocator = CONTEXT_ALLOC;
    }

    void *result = allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, sizeof(T), 0, 0, 0);
    return new (result) T;
}

template <typename T>
inline T *New(size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) {
        allocator = CONTEXT_ALLOC;
    }

    T *result = (T *) allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, count * sizeof(T), 0, 0, 0);
    for (size_t i = 0; i < count; i++) {
        new ((void *) (result + i * sizeof(T))) T;
    }
    return result;
}

template <typename T>
inline void Delete(T *memory, Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) {
        allocator = CONTEXT_ALLOC;
    }

    memory->~T();
    allocator.Function(Allocator_Mode::FREE, allocator.Data, 0, memory, sizeof(T), 0);
}

template <typename T>
inline void Delete(T *memory, size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator.Function) {
        allocator = CONTEXT_ALLOC;
    }

    for (size_t i = 0; i < count; i++) {
        (memory + i * sizeof(T))->~T();
    }
    allocator.Function(Allocator_Mode::FREE, allocator.Data, 0, memory, count * sizeof(T), 0);
}

// Windows.h defines CopyMemory as memcpy, our version is compatible so it's
// safe to undefine it
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
inline void *CopyMemory(void *dest, void const *src, size_t num) {
    byte *d = (byte *) dest;
    byte const *s = (byte const *) src;

    if ((uptr_t) dest % sizeof(u32) == 0 && (uptr_t) src % sizeof(u32) == 0 && num % sizeof(u32) == 0) {
        for (size_t i = 0; i < num / sizeof(u32); i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = 0; i < num; i++) {
            d[i] = s[i];
        }
    }
    return dest;
}

inline void *MoveMemory(void *dest, void const *src, size_t num) {
    byte *d = (byte *) dest;
    byte const *s = (byte const *) src;

    if (d <= s || d >= (s + num)) {
        // Non-Overlapping Buffers
        while (num--) {
            *d++ = *s++;
        }
    } else {
        // Overlapping Buffers
        d += num - 1;
        s += num - 1;

        while (num--) {
            *d-- = *s--;
        }
    }
    return dest;
}

inline void *FillMemory(void *dest, int value, size_t num) {
    byte *ptr = (byte *) dest;
    while (num-- > 0) {
        *ptr++ = value;
    }
    return dest;
}
extern "C" {
inline void *memcpy(void *dest, void const *src, size_t num) { return CopyMemory(dest, src, num); }
inline void *memmove(void *dest, void const *src, size_t num) { return MoveMemory(dest, src, num); }
inline void *memset(void *dest, int value, size_t num) { return FillMemory(dest, value, num); }
}
#endif

// Fills memory with 0, helper function
inline void *ZeroMemory(void *dest, size_t num) { return FillMemory(dest, 0, num); }

GU_END_NAMESPACE
