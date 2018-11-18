#pragma once

#include "../common.h"
#include "../context.h"

#include <new>

CPPU_BEGIN_NAMESPACE

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
inline s32 CompareMemory(const void *ptr1, const void *ptr2, size_t num) { return memcmp(ptr1, ptr2, num); }

#else
// Defined in memory.cpp
void *CopyMemory(void *dest, void const *src, size_t num);
void *MoveMemory(void *dest, void const *src, size_t num);
void *FillMemory(void *dest, int value, size_t num);
s32 CompareMemory(const void *ptr1, const void *ptr2, size_t num);
CPPU_END_NAMESPACE
extern "C" {
// Defining intrinsic functions that the compiler may use to optimize.
inline void *memcpy(void *dest, void const *src, size_t num) { return CPPU_NAMESPACE_NAME ::CopyMemory(dest, src, num); }
inline void *memmove(void *dest, void const *src, size_t num) { return CPPU_NAMESPACE_NAME ::MoveMemory(dest, src, num); }
inline void *memset(void *dest, int value, size_t num) { return CPPU_NAMESPACE_NAME ::FillMemory(dest, value, num); }
inline s32 memcmp(const void *ptr1, const void *ptr2, size_t num) {
    return CPPU_NAMESPACE_NAME ::CompareMemory(ptr1, ptr2, num);
}
}
CPPU_BEGIN_NAMESPACE
#endif

// Helper function that fills memory with 0
inline void *ZeroMemory(void *dest, size_t num) { return FillMemory(dest, 0, num); }

// Helper template function for copying contents of arrays of the same type.
// This method does not handle overlapping arrays (for consistency with CopyMemory).
template <typename T>
T *CopyElements(T *dest, T *src, size_t numberOfElements) {
    static_assert(!std::is_same_v<T, void>);

    for (size_t i = 0; i < numberOfElements; i++) {
        dest[i] = T(src[i]);
    }
    return dest;
}

// Helper template function for moving contents of one array to another,
// this function handles overlapping arrays (like MoveMemory does).
template <typename T>
T *MoveElements(T *dest, T *src, size_t numberOfElements) {
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
    ZeroMemory(result, sizeof(T));
    return result;
}

template <typename T>
T *New(size_t count, Allocator_Closure allocator = {0, 0}) {
    if (!allocator) allocator = CONTEXT_ALLOC;

    T *result = (T *) allocator.Function(Allocator_Mode::ALLOCATE, allocator.Data, count * sizeof(T), 0, 0, 0);
    ZeroMemory(result, count * sizeof(T));
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
T *New_And_Ensure_Allocator(Allocator_Closure &allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return New<T>(allocator);
}

// See comment above this function. Actually important as to how this works!
template <typename T>
T *New_And_Ensure_Allocator(size_t count, Allocator_Closure &allocator) {
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

// See comment at New_And_Ensure_Allocator above this function. Actually important as to how this works!
template <typename T>
inline T *Resize_And_Ensure_Allocator(T *memory, size_t oldSize, size_t newSize, Allocator_Closure &allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return Resize(memory, oldSize, newSize, allocator);
}

CPPU_END_NAMESPACE
