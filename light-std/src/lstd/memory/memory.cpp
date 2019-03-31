#include "memory.hpp"

#include "allocator.hpp"

#include "../thread.hpp"

#include "../../vendor/stb/stb_malloc.hpp"

static size_t g_AllocationId = 0;

#if COMPILER == MSVC
static __forceinline void *allocate(size_t size, const allocator_closure &allocator, uptr_t userData = 0) {
#else
static __attribute__((always_inline)) void *allocate(size_t size, const allocator_closure &allocator,
                                                     uptr_t userData = 0) {
#endif
    allocation_info info = {++g_AllocationId, allocator, size};
    if (!info.Allocator) info.Allocator = CONTEXT_ALLOC;
    if (!info.Allocator) {
        info.Allocator = OS_ALLOC;
    }

    size_t actualSize = size + sizeof(allocation_info);

    void *data = info.Allocator.Function(allocator_mode::ALLOCATE, info.Allocator.Data, actualSize, null, 0, userData);
    copy_memory(data, &info, sizeof(allocation_info));
    return (byte *) data + sizeof(allocation_info);
}

void *operator new(size_t size) { return allocate(size, CONTEXT_ALLOC); }
void *operator new[](size_t size) { return allocate(size, CONTEXT_ALLOC); }

// We ignore alignment for now ??
void *operator new(size_t size, std::align_val_t align) { return allocate(size, CONTEXT_ALLOC); }
// We ignore alignment for now ??
void *operator new[](size_t size, std::align_val_t align) { return allocate(size, CONTEXT_ALLOC); }

void *operator new(size_t size, allocator_closure allocator) { return allocate(size, allocator); }
void *operator new[](size_t size, allocator_closure allocator) { return allocate(size, allocator); }

void *operator new(size_t size, allocator_closure allocator, uptr_t userData) {
    return allocate(size, allocator, userData);
}
void *operator new[](size_t size, allocator_closure allocator, uptr_t userData) {
    return allocate(size, allocator, userData);
}

// This operator is a wrapper around new (allocator) T, but if the passed pointer to allocator
// points to a null allocator, it makes it point to the context allocator and uses that one.
void *operator new(size_t size, allocator_closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator);
}

void *operator new[](size_t size, allocator_closure *allocator, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator);
}

void *operator new(size_t size, allocator_closure *allocator, uptr_t userData, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator, userData);
}

void *operator new[](size_t size, allocator_closure *allocator, uptr_t userData, const ensure_allocator_t) {
    if (!*allocator) *allocator = CONTEXT_ALLOC;
    if (!*allocator) *allocator = OS_ALLOC;
    return allocate(size, *allocator, userData);
}

static void deallocate(void *ptr) {
    if (!ptr) return;

    auto *info = (allocation_info *) ptr - 1;
    auto allocSize = info->Size + sizeof(allocation_info);
    info->Allocator.Function(allocator_mode::FREE, info->Allocator.Data, 0, info, allocSize, 0);
}

void operator delete(void *ptr) { deallocate(ptr); }
void operator delete[](void *ptr) { deallocate(ptr); }
void operator delete(void *ptr, size_t) { deallocate(ptr); }
void operator delete[](void *ptr, size_t) { deallocate(ptr); }

LSTD_BEGIN_NAMESPACE

static bool g_MallocInitted = false;
static byte g_Heap[STBM_HEAP_SIZEOF];

void *malloc_allocator(allocator_mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, uptr_t) {
    if (!g_MallocInitted) {
        stbm_heap_config hc;
        zero_memory(&hc, sizeof(hc));
        {
            hc.system_alloc = os_memory_alloc;
            hc.system_free = os_memory_free;
            hc.user_context = null;
            hc.minimum_alignment = 8;

            hc.allocation_mutex = new (OS_ALLOC) thread::mutex;
            hc.crossthread_free_mutex = new (OS_ALLOC) thread::mutex;
        }
        stbm_heap_init(g_Heap, sizeof(g_Heap), &hc);

        g_MallocInitted = true;
    }

    switch (mode) {
        case allocator_mode::ALLOCATE: {
            void *memory = stbm_alloc(null, (stbm_heap *) g_Heap, size, 0);
            zero_memory(memory, size);
            return memory;
        }
        case allocator_mode::RESIZE: {
            void *memory = stbm_realloc(null, (stbm_heap *) g_Heap, oldMemory, size, 0);
            if (size > oldSize) {
                zero_memory((byte *) memory + oldSize, size - oldSize);
            }
            return memory;
        }
        case allocator_mode::FREE:
            stbm_free(null, (stbm_heap *) g_Heap, oldMemory);
            return null;
        default:
            assert(false);  // We shouldn't get here
    }
    return null;
}

allocator_func g_DefaultAllocator = malloc_allocator;

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
