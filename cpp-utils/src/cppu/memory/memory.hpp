#pragma once

#include "../common.hpp"
#include "../context.hpp"

#include <new>

CPPU_BEGIN_NAMESPACE

// Windows.h defines copy_memory as memcpy, our version is compatible so it's safe to undefine it
#if defined copy_memory
#undef copy_memory
#endif
#if defined move_memory
#undef move_memory
#endif
#if defined fill_memory
#undef fill_memory
#endif
#if defined zero_memory
#undef zero_memory
#endif

#if !defined CPPU_NO_CRT
#include <cstring>

inline void *copy_memory(void *dest, void const *src, size_t num) { return memcpy(dest, src, num); }
inline void *move_memory(void *dest, void const *src, size_t num) { return memmove(dest, src, num); }
inline void *fill_memory(void *dest, int value, size_t num) { return memset(dest, value, num); }
inline s32 compare_memory(const void *ptr1, const void *ptr2, size_t num) { return memcmp(ptr1, ptr2, num); }

#else
// Defined in memory.cpp
void *copy_memory(void *dest, void const *src, size_t num);
void *move_memory(void *dest, void const *src, size_t num);
void *fill_memory(void *dest, int value, size_t num);
s32 compare_memory(const void *ptr1, const void *ptr2, size_t num);
CPPU_END_NAMESPACE
extern "C" {
// Defining intrinsic functions that the compiler may use to optimize.
inline void *memcpy(void *dest, void const *src, size_t num) {
    return CPPU_NAMESPACE_NAME ::copy_memory(dest, src, num);
}
inline void *memmove(void *dest, void const *src, size_t num) {
    return CPPU_NAMESPACE_NAME ::move_memory(dest, src, num);
}
inline void *memset(void *dest, int value, size_t num) { return CPPU_NAMESPACE_NAME ::fill_memory(dest, value, num); }
inline s32 memcmp(const void *ptr1, const void *ptr2, size_t num) {
    return CPPU_NAMESPACE_NAME ::compare_memory(ptr1, ptr2, num);
}
}
CPPU_BEGIN_NAMESPACE
#endif

// Helper function that fills memory with 0
inline void *zero_memory(void *dest, size_t num) { return fill_memory(dest, 0, num); }

// Helper template function for copying contents of arrays of the same type.
// This method does not handle overlapping arrays (for consistency with copy_memory).
template <typename T>
T *copy_elements(T *dest, T *src, size_t numberOfElements) {
    static_assert(!std::is_same_v<T, void>);

    for (size_t i = 0; i < numberOfElements; i++) {
        dest[i] = T(src[i]);
    }
    return dest;
}

// Helper template function for moving contents of one array to another,
// this function handles overlapping arrays (like move_memory does).
template <typename T>
T *move_elements(T *dest, T *src, size_t numberOfElements) {
    static_assert(!std::is_same_v<T, void>);

    T *d = dest;

    if (dest <= src || dest >= (src + numberOfElements)) {
        // Non-Overlapping Buffers
        while (numberOfElements--) {
            *dest++ = std::move(*src++);
        }
    } else {
        // Overlapping Buffers
        dest += numberOfElements - 1;
        src += numberOfElements - 1;

        while (numberOfElements--) {
            *dest-- = std::move(*src--);
        }
    }
    return d;
}

template <typename T>
T *New(Allocator_Closure allocator = {0, 0}) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    T *result = new (allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, sizeof(T), 0, 0, 0)) T;
    zero_memory(result, sizeof(T));
    return result;
}

template <typename T>
T *New(size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator) allocator = CONTEXT_ALLOC;

    T *result = (T *) allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, count * sizeof(T), 0, 0, 0);
    zero_memory(result, count * sizeof(T));
    for (size_t i = 0; i < count; i++) new (result + i) T;
    return result;
}

// This function is a wrapper around New<T>(allocator), but if the passed reference to allocator
// is null, it sets it to the context allocator and uses that one. This is different from the
// default New, because the default new just silently uses the context allocator. If you plan
// to use the memory allocated for longer time than just simple scopes (for example storage in
// data structures) and be robust to changes in the context allocator, you must save it in a
// variable that either the user can specify or just for your own internal purposes,
// and save it for later when you Delete the allocated storage.
//
// If you don't use the same allocator when you New and Delete something, you most probably
// will crash, because there is little guarantee that the two allocators are compatible.
template <typename T>
T *New_and_ensure_allocator(Allocator_Closure &allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return New<T>(allocator);
}

// See comment above this function. Actually important as to how this works!
template <typename T>
T *New_and_ensure_allocator(size_t count, Allocator_Closure &allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return New<T>(count, allocator);
}

template <typename T>
void Delete(T *memory, Allocator_Closure allocator = {0, 0}) {
    if (!allocator) allocator = CONTEXT_ALLOC;

    memory->~T();
    allocator.Function(Allocator_Mode::FREE, allocator.Data, 0, memory, sizeof(T), 0);
}

template <typename T>
void Delete(T *memory, size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator) allocator = CONTEXT_ALLOC;

    for (size_t i = 0; i < count; i++) (memory + i)->~T();
    allocator.Function(Allocator_Mode::FREE, allocator.Data, 0, memory, count * sizeof(T), 0);
}

// Used for rezising an array.
// _oldSize_ and _newSize_ are automatically multiplied by sizeof(T).
template <typename T>
T *Resize(T *memory, size_t oldSize, size_t newSize, Allocator_Closure allocator = {0, 0}) {
    if (!allocator) allocator = CONTEXT_ALLOC;

    oldSize *= sizeof(T);
    newSize *= sizeof(T);

    return (T *) allocator.Function(Allocator_Mode::RESIZE, allocator.Data, newSize, memory, oldSize, 0);
}

// See comment at New_and_ensure_allocator above this function. Actually important as to how this works!
template <typename T>
inline T *Resize_and_ensure_allocator(T *memory, size_t oldSize, size_t newSize, Allocator_Closure &allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return Resize(memory, oldSize, newSize, allocator);
}

CPPU_END_NAMESPACE
