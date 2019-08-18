#include "allocator.h"

#include "../os.h"

#include "../context.h"
#include "../io/fmt.h"

#include "../../vendor/stb_malloc.hpp"

LSTD_BEGIN_NAMESPACE

void *os_alloc_wrapper(void *, size_t size, size_t *) { return os_alloc(size); }
void os_free_wrapper(void *, void *ptr) { os_free(ptr); }

static bool g_MallocInitted = false;
static char g_Heap[STBM_HEAP_SIZEOF];

void *default_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                        alignment align, u64) {
    if (!g_MallocInitted) {
        stbm_heap_config hc = {};
        {
            hc.system_alloc = os_alloc_wrapper;
            hc.system_free = os_free_wrapper;
            hc.user_context = null;
            hc.minimum_alignment = 8;

            hc.allocation_mutex = new ({os_allocator, null}) thread::mutex;
            hc.crossthread_free_mutex = new ({os_allocator, null}) thread::mutex;
        }
        stbm_heap_init(g_Heap, sizeof(g_Heap), &hc);

        g_MallocInitted = true;
    }

    switch (mode) {
        case allocator_mode::ALLOCATE:
            return stbm_alloc(null, (stbm_heap *) g_Heap, size, 0);
        case allocator_mode::ALIGNED_ALLOCATE:
            return stbm_alloc_align(null, (stbm_heap *) g_Heap, size, 0, (size_t) align, 0);
        case allocator_mode::REALLOCATE:
            return stbm_realloc(null, (stbm_heap *) g_Heap, oldMemory, size, 0);
        case allocator_mode::ALIGNED_REALLOCATE: {
            if (!oldMemory) return stbm_alloc_align(null, (stbm_heap *) g_Heap, size, 0, (size_t) align, 0);
            if (size <= oldSize && oldSize < size * 2) return oldMemory;

            void *newPtr = stbm_alloc_align(null, (stbm_heap *) g_Heap, size, 0, (size_t) align, 0);
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

void *os_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, alignment align,
                   u64) {
    switch (mode) {
        case allocator_mode::ALLOCATE:
        case allocator_mode::ALIGNED_ALLOCATE:
            return os_alloc(size);
        case allocator_mode::REALLOCATE:
            return stbm_realloc(null, (stbm_heap *) g_Heap, oldMemory, size, 0);
        case allocator_mode::ALIGNED_REALLOCATE: {
            if (!oldMemory) return os_alloc(size);
            if (size <= oldSize && oldSize < size * 2) return oldMemory;

            auto *newMemory = os_alloc(size);
            copy_memory(newMemory, oldMemory, oldSize);
            os_free(oldMemory);
            return newMemory;
        }
        case allocator_mode::FREE:
            os_free(oldMemory);
            return null;
        case allocator_mode::FREE_ALL:
            return (void *) -1;
        default:
            assert(false && "What");
    }
    return null;
}

void *temporary_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                          alignment align, u64) {
    auto *data = (temporary_allocator_data *) context;
    // The temporary allocator hasn't been initialized yet.
    if (!data->Reserved) {
        size_t startingSize = (size_t)(size * 1.5);
        data->Storage = new (Malloc) char[startingSize];
        data->Reserved = startingSize;
    }

    switch (mode) {
        case allocator_mode::ALIGNED_ALLOCATE:
            size += (size_t) align < POINTER_SIZE ? POINTER_SIZE : (size_t) align;
            [[fallthrough]];
        case allocator_mode::ALLOCATE: {
            void *result = null;
            if (data->Used + size < data->Reserved) {
                result = (char *) data->Storage + data->Used;
                data->Used += size;
            } else {
                // Out of space.. find an overflow page
                temporary_allocator_data::overflow_page *page = data->OverflowPageList;

                auto *last = page;
                while (page) {
                    if (page->Used + size < page->Reserved) break;
                    last = page;
                    page = page->Next;
                }

                if (!page) {
                    page = new (Malloc) temporary_allocator_data::overflow_page;

                    size_t lastReserved;
                    if (!data->OverflowPageList) {
                        data->OverflowPageList = page;
                        lastReserved = data->Reserved;
                    } else {
                        last->Next = page;
                        lastReserved = last->Reserved;
                    }

                    // Random log-based growth thing I came up at the time, not real science.
                    size_t reserveTarget =
                        MAX<size_t>(CEIL_POW_OF_2(size * 2),
                                    CEIL_POW_OF_2((size_t) CEIL(lastReserved * (LOG_2(lastReserved * 10.0) / 3))));

                    page->Storage = new (Malloc) char[reserveTarget];
                    page->Reserved = reserveTarget;
                }

                result = (char *) page->Storage + page->Used;
                page->Used += size;
            }
            if (result && mode == allocator_mode::ALIGNED_ALLOCATE) {
                result = get_aligned_pointer(result, (size_t) align);
            }

            data->OverallUsed += size;

            return result;
        }
        // Reallocations aren't really viable with this allocator
        // so we just copy the old memory into a fresh block
        case allocator_mode::ALIGNED_REALLOCATE: {
            void *result = temporary_allocator(allocator_mode::ALIGNED_ALLOCATE, context, size, null, 0, align, 0);
            copy_memory(result, oldMemory, oldSize);
            return result;
        }
        case allocator_mode::REALLOCATE: {
            void *result = temporary_allocator(allocator_mode::ALLOCATE, context, size, null, 0, align, 0);
            copy_memory(result, oldMemory, oldSize);
            return result;
        }
        case allocator_mode::FREE:
            // We don't free individual allocations in the temporary allocator
            return null;
        case allocator_mode::FREE_ALL: {
            size_t targetSize = data->Reserved;

            // Check if any overflow pages were used
            auto *page = data->OverflowPageList;
            while (page) {
                targetSize += page->Reserved;

                auto *next = page->Next;
                delete[] page->Storage;
                delete page;
                page = next;
            }
            data->OverflowPageList = null;

            // Resize _Storage_ to fit all allocations which previously required overflow pages
            if (targetSize != data->Reserved) {
                delete[] data->Storage;

                size_t prev = data->Reserved;

                data->Storage = new (Malloc) char[targetSize];
                data->Reserved = targetSize;
            }

            data->OverallUsed = data->Used = 0;
            // null means successful FREE_ALL
            // (void *) -1 means failure
            return null;
        }
        default:
            assert(false && "What");
    }
    return null;
}

void implicit_context::release_temporary_allocator() const {
    if (!TemporaryAllocData.Reserved) return;

    // Free any left-over overflow pages!
    TemporaryAlloc.free_all();

    delete[] TemporaryAllocData.Storage;
    zero_memory(const_cast<temporary_allocator_data *>(&TemporaryAllocData), sizeof(TemporaryAllocData));
}

LSTD_END_NAMESPACE

void *operator new(size_t size) { return operator new(size, null, 0); }
void *operator new[](size_t size) { return operator new(size, null, 0); }

void *operator new(size_t size, allocator *alloc, u64 userFlags) noexcept {
    if (!alloc) alloc = &const_cast<implicit_context *>(&Context)->Alloc;
    if (!(*alloc)) *alloc = Context.Alloc;
    if (!(*alloc)) {
        const_cast<implicit_context *>(&Context)->Alloc = Malloc;
        *alloc = Malloc;
    }
    return alloc->allocate(size, userFlags);
}

void *operator new[](size_t size, allocator *alloc, u64 userFlags) noexcept {
    return operator new(size, alloc, userFlags);
}

void *operator new(size_t size, allocator alloc, u64 userFlags) noexcept {
    return operator new(size, &alloc, userFlags);
}
void *operator new[](size_t size, allocator alloc, u64 userFlags) noexcept {
    return operator new(size, &alloc, userFlags);
}

void *operator new(size_t size, alignment align, allocator *alloc, u64 userFlags) noexcept {
    if (!alloc) alloc = &const_cast<implicit_context *>(&Context)->Alloc;
    if (!(*alloc)) *alloc = Context.Alloc; 
    assert(*alloc);
    return alloc->allocate_aligned(size, align, userFlags);
}

void *operator new[](size_t size, alignment align, allocator *alloc, u64 userFlags) noexcept {
    return operator new(size, align, alloc, userFlags);
}

void *operator new(size_t size, alignment align, allocator alloc, u64 userFlags) noexcept {
    return operator new(size, align, &alloc, userFlags);
}
void *operator new[](size_t size, alignment align, allocator alloc, u64 userFlags) noexcept {
    return operator new(size, align, &alloc, userFlags);
}

void operator delete(void *ptr) noexcept { allocator::free(ptr); }
void operator delete[](void *ptr) noexcept { allocator::free(ptr); }
