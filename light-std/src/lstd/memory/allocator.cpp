#include "allocator.h"

#include "../../vendor/stb_malloc.hpp"
#include "../context.h"
#include "../io/fmt.h"

LSTD_BEGIN_NAMESPACE

// OS specific functions, defined in *platform*_common.cpp files
extern void *os_alloc(size_t size);
extern void os_free(void *ptr);

void *os_alloc_wrapper(void *, size_t size, size_t *) { return os_alloc(size); }
void os_free_wrapper(void *, void *ptr) { os_free(ptr); }

static bool g_MallocInitted = false;
static byte g_Heap[STBM_HEAP_SIZEOF];

void *default_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                        size_t alignment, uptr_t) {
    if (!g_MallocInitted) {
        stbm_heap_config hc = {};
        {
            hc.system_alloc = os_alloc_wrapper;
            hc.system_free = os_free_wrapper;
            hc.user_context = null;
            hc.minimum_alignment = 8;

            // @Thread
            // hc.allocation_mutex = new (OS_ALLOC) thread::mutex;
            // hc.crossthread_free_mutex = new (OS_ALLOC) thread::mutex;
        }
        stbm_heap_init(g_Heap, sizeof(g_Heap), &hc);

        g_MallocInitted = true;
    }

    switch (mode) {
        case allocator_mode::ALLOCATE:
            return stbm_alloc(null, (stbm_heap *) g_Heap, size, 0);
        case allocator_mode::ALIGNED_ALLOCATE:
            return stbm_alloc_align(null, (stbm_heap *) g_Heap, size, 0, alignment, 0);
        case allocator_mode::REALLOCATE:
            return stbm_realloc(null, (stbm_heap *) g_Heap, oldMemory, size, 0);
        case allocator_mode::ALIGNED_REALLOCATE: {
            if (!oldMemory) return stbm_alloc_align(null, (stbm_heap *) g_Heap, size, 0, alignment, 0);
            if (size <= oldSize && oldSize < size * 2) return oldMemory;

            void *newPtr = stbm_alloc_align(null, (stbm_heap *) g_Heap, size, 0, alignment, 0);
            if (!newPtr) return null;

            copy_memory(newPtr, oldMemory, oldSize);
            stbm_free(null, (stbm_heap *) g_Heap, oldMemory);
            return newPtr;
        }
        case allocator_mode::FREE:
            stbm_free(null, (stbm_heap *) g_Heap, oldMemory);
            return null;
        case allocator_mode::FREE_ALL:
            return (void *) -1;
        default:
            assert(false && "What");
    }
    return null;
}

void *temporary_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                          size_t alignment, uptr_t) {
    auto *data = (temporary_allocator_data *) context;
    assert(data && "Temporary allocator probably not initialized");

    switch (mode) {
        case allocator_mode::ALIGNED_ALLOCATE:
            size += alignment < POINTER_SIZE ? POINTER_SIZE : alignment;
            [[fallthrough]];
        case allocator_mode::ALLOCATE: {
            void *result = null;
            if (data->Used + size < data->Reserved) {
                result = (byte *) data->Storage + data->Used;
                data->Used += size;
            } else {
                // Out of storage.
                bool switched = false;
                if (Context.Alloc == Context.TemporaryAlloc) {
                    switched = true;
                    // @Cleanup Maybe we shouldn't do this...
                    // If the context uses the temporary allocator, switch it forcefully to malloc
                    const_cast<implicit_context *>(&Context)->Alloc = Malloc;
                }
                fmt::print(">> TemporaryAlloc ran out of storage. Using Malloc...\n");
                if (switched) {
                    fmt::print("   Note: Context's default allocator is TemporaryAlloc. Switching it to Malloc...\n");
                }
                result = Malloc.allocate(size);
            }
            if (result && mode == allocator_mode::ALIGNED_ALLOCATE) {
                result = get_aligned_pointer(result, alignment);
            }
            return result;
        }
        // Reallocations aren't really viable with this allocator
        // so we just copy the old memory into a fresh block
        case allocator_mode::ALIGNED_REALLOCATE: {
            void *result = temporary_allocator(allocator_mode::ALIGNED_ALLOCATE, context, size, null, 0, alignment, 0);
            copy_memory(result, oldMemory, oldSize);
            return result;
        }
        case allocator_mode::REALLOCATE: {
            void *result = temporary_allocator(allocator_mode::ALLOCATE, context, size, null, 0, 0, 0);
            copy_memory(result, oldMemory, oldSize);
            return result;
        }
        case allocator_mode::FREE:
            // We don't free individual allocations
            return null;
        case allocator_mode::FREE_ALL:
            data->Used = 0;
            // null means successful FREE_ALL
            // (void *) -1 means failure
            return null;
            // return (void *) -1;
    }
    return null;
}

void implicit_context::init_temporary_allocator(size_t storageSize) const {
    assert(!TemporaryAlloc.Context &&
           "Temporary allocator already initialized. Destroy it with release_temporary_allocator() first.");

    auto *data = new (Malloc) temporary_allocator_data;
    data->Storage = new (Malloc) byte[storageSize];
    data->Reserved = storageSize;

    const_cast<allocator *>(&TemporaryAlloc)->Context = data;
}

void implicit_context::release_temporary_allocator() const {
    assert(TemporaryAlloc.Context && "Temporary allocator not initialized");

    auto *tempAlloc = const_cast<allocator *>(&TemporaryAlloc);
    delete[](byte *)((temporary_allocator_data *) tempAlloc->Context)->Storage;
    delete (temporary_allocator_data *) tempAlloc->Context;

    tempAlloc->Context = null;
}

LSTD_END_NAMESPACE

void *operator new(size_t size) {
    auto *alloc = (allocator *) &Context.Alloc;
    if (!alloc) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        alloc = &Malloc;
    }
    return alloc->allocate(size, 0);
}

void *operator new[](size_t size) { return operator new(size); }

void *operator new(size_t size, allocator alloc) {
    if (!alloc) alloc = Context.Alloc;
    if (!alloc) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        alloc = Malloc;
    }
    return alloc.allocate(size, 0);
}

void *operator new[](size_t size, allocator alloc) { return operator new(size, alloc); }

void *operator new(size_t size, allocator *alloc) {
    assert(alloc);
    if (!(*alloc)) *alloc = Context.Alloc;
    if (!(*alloc)) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        *alloc = Malloc;
    }
    return alloc->allocate(size, 0);
}

void *operator new[](size_t size, allocator *alloc) { return operator new(size, alloc); }

void *operator new(size_t size, size_t alignment) {
    auto *alloc = (allocator *) &Context.Alloc;
    if (!alloc) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        alloc = &Malloc;
    }
    return alloc->allocate_aligned(size, alignment);
}

void *operator new[](size_t size, size_t alignment) { return operator new(size, alignment); }

void *operator new(size_t size, size_t alignment, allocator alloc) {
    if (!alloc) alloc = Context.Alloc;
    if (!alloc) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        alloc = Malloc;
    }
    return alloc.allocate_aligned(size, alignment);
}

void *operator new[](size_t size, size_t alignment, allocator alloc) { return operator new(size, alignment, alloc); }

void *operator new(size_t size, size_t alignment, allocator *alloc) {
    assert(alloc);
    if (!(*alloc)) *alloc = Context.Alloc;
    if (!(*alloc)) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        *alloc = Malloc;
    }
    return alloc->allocate_aligned(size, alignment, 0);
}

void *operator new[](size_t size, size_t alignment, allocator *alloc) { return operator new(size, alignment, alloc); }

void operator delete(void *ptr) noexcept { allocator::free(ptr); }
void operator delete[](void *ptr) noexcept { allocator::free(ptr); }
