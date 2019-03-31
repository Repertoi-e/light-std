#include "memory.hpp"

#include "allocator.hpp"

#include "../thread.hpp"

#include "../../vendor/stb/stb_malloc.hpp"

static size_t g_AllocationId = 0;

#if COMPILER == MSVC
static __forceinline void *allocate(size_t size, const Allocator_Closure &allocator, uptr_t userData = 0) {
#else
static void *allocate(size_t size, const Allocator_Closure &allocator, uptr_t userData = 0) __attribute__((always_inline)) {
#endif
    Allocation_Info info = {++g_AllocationId, allocator, size};
    if (!info.Allocator) info.Allocator = CONTEXT_ALLOC;
    if (!info.Allocator) {
        info.Allocator = OS_ALLOC;
    }

    size_t actualSize = size + sizeof(Allocation_Info);

    void *data = info.Allocator.Function(Allocator_Mode::ALLOCATE, info.Allocator.Data, actualSize, 0, 0, userData);
    copy_memory(data, &info, sizeof(Allocation_Info));
    return (byte *) data + sizeof(Allocation_Info);
}

void *operator new(size_t size) { return allocate(size, CONTEXT_ALLOC); }
void *operator new[](size_t size) { return allocate(size, CONTEXT_ALLOC); }

// We ignore alignment for now ??
void *operator new(size_t size, std::align_val_t align) { return allocate(size, CONTEXT_ALLOC); }
// We ignore alignment for now ??
void *operator new[](size_t size, std::align_val_t align) { return allocate(size, CONTEXT_ALLOC); }

void *operator new(size_t size, Allocator_Closure allocator) { return allocate(size, allocator); }
void *operator new[](size_t size, Allocator_Closure allocator) { return allocate(size, allocator); }

void *operator new(size_t size, Allocator_Closure allocator, uptr_t userData) {
    return allocate(size, allocator, userData);
}
void *operator new[](size_t size, Allocator_Closure allocator, uptr_t userData) {
    return allocate(size, allocator, userData);
}

// This operator is a wrapper around new (allocator) T, but if the passed pointer to allocator
// points to a null allocator, it makes it point to the context allocator and uses that one.
void *operator new(size_t size, Allocator_Closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator);
}

void *operator new[](size_t size, Allocator_Closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator);
}

void *operator new(size_t size, Allocator_Closure *allocator, uptr_t userData, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator, userData);
}

void *operator new[](size_t size, Allocator_Closure *allocator, uptr_t userData, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator, userData);
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

static bool g_MallocInitted = false;
static byte g_Heap[STBM_HEAP_SIZEOF];

void *malloc_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, uptr_t) {
    if (!g_MallocInitted) {
        stbm_heap_config hc;
        zero_memory(&hc, sizeof(hc));
        {
            hc.system_alloc = os_memory_alloc;
            hc.system_free = os_memory_free;
            hc.user_context = null;
            hc.minimum_alignment = 8;

            hc.allocation_mutex = new (OS_ALLOC) thread::Mutex;
            hc.crossthread_free_mutex = new (OS_ALLOC) thread::Mutex;
        }
        stbm_heap_init(g_Heap, sizeof(g_Heap), &hc);

        g_MallocInitted = true;
    }

    switch (mode) {
        case Allocator_Mode::ALLOCATE: {
            void *memory = stbm_alloc(0, (stbm_heap *) g_Heap, size, 0);
            zero_memory(memory, size);
            return memory;
        }
        case Allocator_Mode::RESIZE: {
            void *memory = stbm_realloc(0, (stbm_heap *) g_Heap, oldMemory, size, 0);
            if (size > oldSize) {
                zero_memory((byte *) memory + oldSize, size - oldSize);
            }
            return memory;
        }
        case Allocator_Mode::FREE:
            stbm_free(0, (stbm_heap *) g_Heap, oldMemory);
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

#if !defined LSTD_NAMESPACE_NAME
#define LSTD_NAMESPACE_NAME
#endif

// Defining intrinsic functions that the compiler may use to optimize.
#pragma function(memcpy)
void *memcpy(void *dest, void const *src, size_t num) { return LSTD_NAMESPACE_NAME::copy_memory(dest, src, num); }

#pragma function(memset)
void *memset(void *dest, s32 value, size_t num) { return LSTD_NAMESPACE_NAME::fill_memory(dest, value, num); }
}
#endif
