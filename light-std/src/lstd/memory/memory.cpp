#include "memory.hpp"

#include "allocator.hpp"

void *operator new(size_t size) {
    Allocation_Info info = {CONTEXT_ALLOC, size};
    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *) data + sizeof(Allocation_Info);
}

void *operator new[](size_t size) {
    Allocation_Info info = {CONTEXT_ALLOC, size};
    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *) data + sizeof(Allocation_Info);
}

// We ignore alignment for now ??
void *operator new(size_t size, std::align_val_t align) {
    Allocation_Info info = { CONTEXT_ALLOC, size };
    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *)data + sizeof(Allocation_Info);
}

// We ignore alignment for now ??
void *operator new[](size_t size, std::align_val_t align) {
    Allocation_Info info = { CONTEXT_ALLOC, size };
    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *)data + sizeof(Allocation_Info);
}

void *operator new(size_t size, Allocator_Closure allocator) {
    Allocation_Info info = {allocator, size};
    if (!info.Allocator) info.Allocator = CONTEXT_ALLOC;

    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *) data + sizeof(Allocation_Info);
}

void *operator new[](size_t size, Allocator_Closure allocator) {
    Allocation_Info info = {allocator, size};
    if (!info.Allocator) info.Allocator = CONTEXT_ALLOC;

    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *) data + sizeof(Allocation_Info);
}

// This operator is a wrapper around new (allocator) T, but if the passed pointer to allocator
// points to a null allocator, it makes it point to the context allocator and uses that one.
void *operator new(size_t size, Allocator_Closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;

    Allocation_Info info = { *allocator, size };
    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *)data + sizeof(Allocation_Info);
}

void *operator new[](size_t size, Allocator_Closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;

    Allocation_Info info = { *allocator, size };
    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *)data + sizeof(Allocation_Info);
}

void operator delete(void *ptr) {
    auto *info = (Allocation_Info *) ptr - 1;
    auto allocSize = info->Size + sizeof(Allocation_Info);
    info->Allocator.Function(Allocator_Mode::FREE, info->Allocator.Data, 0, info, allocSize, 0);
}

void operator delete[](void *ptr) {
    auto *info = (Allocation_Info *) ptr - 1;
    auto allocSize = info->Size + sizeof(Allocation_Info);
    info->Allocator.Function(Allocator_Mode::FREE, info->Allocator.Data, 0, info, allocSize, 0);
}

void operator delete(void *ptr, size_t size) {
    auto *info = (Allocation_Info *) ptr - 1;
    auto allocSize = info->Size + sizeof(Allocation_Info);
    info->Allocator.Function(Allocator_Mode::FREE, info->Allocator.Data, 0, info, allocSize, 0);
}

void operator delete[](void *ptr, size_t size) {
    auto *info = (Allocation_Info *) ptr - 1;
    auto allocSize = info->Size + sizeof(Allocation_Info);
    info->Allocator.Function(Allocator_Mode::FREE, info->Allocator.Data, 0, info, allocSize, 0);
}

LSTD_BEGIN_NAMESPACE

#if defined LSTD_NO_CRT
void *copy_memory(void *dest, void const *src, size_t num) {
    byte *d = (byte *) dest;
    byte const *s = (byte const *) src;

    if ((uptr_t) dest % sizeof(u32) == 0 && (uptr_t) src % sizeof(u32) == 0 && num % sizeof(u32) == 0) {
        for (size_t i = 0; i < num / sizeof(u32); i++) {
            ((u32 *) d)[i] = ((u32 *) s)[i];
        }
    } else {
        for (size_t i = 0; i < num; i++) {
            d[i] = s[i];
        }
    }
    return dest;
}

void *move_memory(void *dest, void const *src, size_t num) {
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

void *fill_memory(void *dest, int value, size_t num) {
    byte *ptr = (byte *) dest;
    while (num-- > 0) {
        *ptr++ = value;
    }
    return dest;
}

s32 compare_memory(const void *ptr1, const void *ptr2, size_t num) {
    const byte *s1 = (const byte *) ptr1;
    const byte *s2 = (const byte *) ptr2;

    while (num-- > 0) {
        if (*s1++ != *s2++) return s1[-1] < s2[-1] ? -1 : 1;
    }

    // The memory regions match
    return 0;
}
#else
void *crt_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, s32) {
    switch (mode) {
        case Allocator_Mode::ALLOCATE: {
            void *memory = malloc(size);
            zero_memory(memory, size);
            return memory;
        }
        case Allocator_Mode::RESIZE: {
            void *memory = realloc(oldMemory, size);
            if (oldSize > size) {
                zero_memory((byte *) memory + oldSize, oldSize - size);
            }
            return memory;
        }
        case Allocator_Mode::FREE:
            free(oldMemory);
            return null;
        case Allocator_Mode::FREE_ALL:
            return null;
        default:
            assert(false);  // We shouldn't get here
    }
    return null;
}

Allocator_Func DefaultAllocator = crt_allocator;
#endif

LSTD_END_NAMESPACE
