#include "memory.hpp"

#include "allocator.hpp"

#include "../../vendor/stb/stb_malloc.hpp"

static size_t g_AllocationId = 0;

static void *allocate(size_t size, const Allocator_Closure &allocator) {
    Allocation_Info info = {++g_AllocationId, allocator, size};
    if (!info.Allocator) info.Allocator = MALLOC;

    size += sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, size, 0, 0, 0);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *) data + sizeof(Allocation_Info);
}

void *operator new(size_t size) { return allocate(size, CONTEXT_ALLOC); }

void *operator new[](size_t size) { return allocate(size, CONTEXT_ALLOC); }

// We ignore alignment for now ??
void *operator new(size_t size, std::align_val_t align) { return allocate(size, CONTEXT_ALLOC); }

// We ignore alignment for now ??
void *operator new[](size_t size, std::align_val_t align) { return allocate(size, CONTEXT_ALLOC); }

void *operator new(size_t size, Allocator_Closure allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return allocate(size, allocator);
}

void *operator new[](size_t size, Allocator_Closure allocator) {
    if (!allocator) allocator = CONTEXT_ALLOC;
    return allocate(size, allocator);
}

// This operator is a wrapper around new (allocator) T, but if the passed pointer to allocator
// points to a null allocator, it makes it point to the context allocator and uses that one.
void *operator new(size_t size, Allocator_Closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = MALLOC;
    return allocate(size, *allocator);
}

void *operator new[](size_t size, Allocator_Closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = MALLOC;
    return allocate(size, *allocator);
}

static void deallocate(void *ptr) {
    auto *info = (Allocation_Info *) ptr - 1;
    auto allocSize = info->Size + sizeof(Allocation_Info);
    info->Allocator.Function(Allocator_Mode::FREE, info->Allocator.Data, 0, info, allocSize, 0);
}

void operator delete(void *ptr) { deallocate(ptr); }
void operator delete[](void *ptr) { deallocate(ptr); }
void operator delete(void *ptr, size_t size) { deallocate(ptr); }
void operator delete[](void *ptr, size_t size) { deallocate(ptr); }

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
#endif

static bool g_MallocInitted = false;
static byte g_HeapStorage[STBM_HEAP_SIZEOF];
static stbm_heap *g_Heap = null;

static void init_malloc() {
    stbm_heap_config hc = { 0 };
    hc.system_alloc = os_memory_alloc;
    hc.system_free = os_memory_free;
    hc.user_context = null;
    hc.minimum_alignment = 8;

    g_Heap = stbm_heap_init(g_HeapStorage, sizeof(g_HeapStorage), &hc);
}

void *malloc_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, s32) {
    if (!g_MallocInitted) {
        g_MallocInitted = true;
        init_malloc();
    }

    switch (mode) {
        case Allocator_Mode::ALLOCATE: {
            void *memory = stbm_alloc(0, g_Heap, size, 0);
            zero_memory(memory, size);
            return memory;
        }
        case Allocator_Mode::RESIZE: {
            void *memory = stbm_realloc(0, g_Heap, oldMemory, size, 0);
            if (oldSize > size) {
                zero_memory((byte *) memory + oldSize, oldSize - size);
            }
            return memory;
        }
        case Allocator_Mode::FREE:
            stbm_free(0, g_Heap, oldMemory);
            return null;
        case Allocator_Mode::FREE_ALL:
            return null;
        default:
            assert(false);  // We shouldn't get here
    }
    return null;
}

Allocator_Func DefaultAllocator = malloc_allocator;

LSTD_END_NAMESPACE

#if defined LSTD_NO_CRT
extern "C" {

// Defining intrinsic functions that the compiler may use to optimize.
#pragma function(memcpy)
void *memcpy(void *dest, void const *src, size_t num) { return LSTD_NAMESPACE_NAME::copy_memory(dest, src, num); }

#pragma function(memset)
void *memset(void *dest, s32 value, size_t num) { return LSTD_NAMESPACE_NAME::fill_memory(dest, value, num); }
}
#endif
