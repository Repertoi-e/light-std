#pragma once

#include "../common.hpp"
#include "../context.hpp"

#include "../../vendor/apex_memmove/apex_memmove.hpp"

enum class ensure_allocator_t : byte { YES };
constexpr auto ensure_allocator = ensure_allocator_t::YES;

void *operator new(size_t count, Allocator_Closure allocator);
void *operator new[](size_t count, Allocator_Closure allocator);

// This operator is a wrapper around new (allocator) T, but if the passed pointer to allocator
// points to a null allocator, it makes it point to the context allocator and uses that one.
void *operator new(size_t count, Allocator_Closure *allocator, ensure_allocator_t);
void *operator new[](size_t count, Allocator_Closure *allocator, ensure_allocator_t);

LSTD_BEGIN_NAMESPACE

// constexpr slow copy memory
constexpr void *copy_memory_constexpr(void *dest, const void *src, size_t num) {
    auto *d = (byte *) dest;
    auto *s = (const byte *) src;

    for (size_t i = 0; i < num; ++i) d[i] = s[i];
    return dest;
}

// constexpr slow move memory
constexpr void *move_memory_constexpr(void *dest, void const *src, size_t num) {
    auto *d = (byte *) dest;
    auto *s = (const byte *) src;

    if (d <= s || d >= (s + num)) {
        // non-overlapping
        while (num--) {
            *d++ = *s++;
        }
    } else {
        // overlapping
        d += num - 1;
        s += num - 1;

        while (num--) {
            *d-- = *s--;
        }
    }
    return dest;
}

// constexpr fill memory
constexpr void *fill_memory_constexpr(void *dest, s32 value, size_t num) {
    auto ptr = (byte *) dest;
    while (num-- > 0) {
        *ptr++ = value;
    }
    return dest;
}

// constexpr compare memory
constexpr s32 compare_memory_constexpr(const void *ptr1, const void *ptr2, size_t num) {
    auto *s1 = (const byte *) ptr1;
    auto *s2 = (const byte *) ptr2;

    while (num-- > 0) {
        if (*s1++ != *s2++) return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

inline void *fill_memory(void *dest, s32 value, size_t num) { return fill_memory_constexpr(dest, value, num); }
inline s32 compare_memory(const void *ptr1, const void *ptr2, size_t num) {
    return compare_memory_constexpr(ptr1, ptr2, num);
}

// Helper function that fills memory with 0
inline void *zero_memory(void *dest, size_t num) { return fill_memory(dest, 0, num); }

// Helper template function for copying contents of arrays of the same type.
// This method does not handle overlapping arrays (for consistency with copy_memory).
template <typename T>
T *copy_elements(T *dest, T *src, size_t numberOfElements) {
    static_assert(!std::is_same_v<T, void>);

    For(range(numberOfElements)) { new (&dest[it]) T(src[it]); }
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

// OS ALLOCATOR (not quite malloc)

// Used by stb_malloc
void *os_memory_alloc(void *context, size_t size, size_t *outsize);

// Used by stb_malloc
void os_memory_free(void *context, void *ptr);

void *os_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, uptr_t);

#define OS_ALLOC \
    Allocator_Closure { os_allocator, null }

struct Allocation_Info {
    size_t Id = 0;
    Allocator_Closure Allocator = {null, null};
    size_t Size = 0;
};

// Used for rezising an array.
// _newSize_ is automatically multiplied by sizeof(T).
// The old size is stored in the header of every allocation's memory pointer (Allocation_Info)
template <typename T>
T *resize(T *memory, size_t newSize, uptr_t userData = 0) {
    auto *info = (Allocation_Info *) memory - 1;
    size_t oldSize = info->Size + sizeof(Allocation_Info);

    newSize = newSize * sizeof(T) + sizeof(Allocation_Info);

    // Note: We don't change info->Id here
    // I don't think we should...
    auto *newMemory = (Allocation_Info *) info->Allocator.Function(Allocator_Mode::RESIZE, info->Allocator.Data,
                                                                   newSize, info, oldSize, userData);
    newMemory->Size = newSize;
    return (T *) (newMemory + 1);
}

// BIT CAST

template <class T, class U>
inline T bit_cast(const U &source) {
    static_assert(sizeof(T) == sizeof(U), "trying to bit_cast types of different size");

    T dest;
    copy_memory(&dest, &source, sizeof(dest));
    return dest;
}
LSTD_END_NAMESPACE
