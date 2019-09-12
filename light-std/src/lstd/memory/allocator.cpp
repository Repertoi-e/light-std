#include "allocator.h"

#include "../os.h"

#include "../context.h"
#include "../io/fmt.h"

#include "../../vendor/stb_malloc.h"

LSTD_BEGIN_NAMESPACE

void *os_alloc_wrapper(void *, size_t size, size_t *) { return os_alloc(size); }
void os_free_wrapper(void *, void *ptr) { os_free(ptr); }

static bool MallocInitted = false;
static char Heap[STBM_HEAP_SIZEOF];

void *default_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                        alignment align, u64) {
    if (!MallocInitted) {
        stbm_heap_config hc;
        zero_memory(&hc, sizeof(hc));
        {
            hc.system_alloc = os_alloc_wrapper;
            hc.system_free = os_free_wrapper;
            hc.user_context = null;
            hc.minimum_alignment = 8;

            hc.allocation_mutex = new ({os_allocator, null}) thread::mutex;
            hc.crossthread_free_mutex = new ({os_allocator, null}) thread::mutex;
        }
        stbm_heap_init(Heap, sizeof(Heap), &hc);

        MallocInitted = true;
    }

    switch (mode) {
        case allocator_mode::ALLOCATE:
            return stbm_alloc(null, (stbm_heap *) Heap, size, 0);
        case allocator_mode::ALIGNED_ALLOCATE:
            return stbm_alloc_align(null, (stbm_heap *) Heap, size, 0, (size_t) align, 0);
        case allocator_mode::REALLOCATE:
            return stbm_realloc(null, (stbm_heap *) Heap, oldMemory, size, 0);
        case allocator_mode::ALIGNED_REALLOCATE: {
            if (size <= oldSize) return oldMemory;

            void *newPtr = stbm_alloc_align(null, (stbm_heap *) Heap, size, 0, (size_t) align, 0);
            if (!newPtr) return null;

            copy_memory(newPtr, oldMemory, oldSize);
            stbm_free(null, (stbm_heap *) Heap, oldMemory);
            return newPtr;
        }
        case allocator_mode::FREE:
            stbm_free(null, (stbm_heap *) Heap, oldMemory);
            return null;
        case allocator_mode::FREE_ALL:
            return (void *) -1;
        default:
            assert(false);
    }
    return null;
}

void *os_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, alignment align,
                   u64) {
    switch (mode) {
        case allocator_mode::ALIGNED_ALLOCATE:
            size += (size_t) align;
            [[fallthrough]];
        case allocator_mode::ALLOCATE: {
            void *result = os_alloc(size);
            if (mode == allocator_mode::ALIGNED_ALLOCATE) result = get_aligned_pointer(result, (size_t) align);
            return result;
        }
        case allocator_mode::ALIGNED_REALLOCATE:
        case allocator_mode::REALLOCATE: {
            // @Speed: Make an _os_realloc_ and use that..
            if (size <= oldSize) return oldMemory;

            auto *newMemory = os_allocator(allocator_mode::ALIGNED_ALLOCATE, context, size, null, 0, align, 0);
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
            assert(false);
    }
    return null;
}

void *temporary_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                          alignment align, u64) {
#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

    auto *data = (temporary_allocator_data *) context;
    // The temporary allocator hasn't been initialized yet.
    if (!data->Base.Reserved) {
        size_t startingSize = (size * 2 + 8_KiB - 1) & -8_KiB;  // Round up to a multiple of 8 KiB
        data->Base.Storage = new (Malloc) char[startingSize];
        data->Base.Reserved = startingSize;
    }

    switch (mode) {
        case allocator_mode::ALIGNED_ALLOCATE:
            size += (size_t) align;
            [[fallthrough]];
        case allocator_mode::ALLOCATE: {
            auto *p = &data->Base;

            while (p->Next) {
                if (p->Used + size < p->Reserved) break;
                p = p->Next;
            }

            if (p->Used + size >= p->Reserved) {
                assert(!p->Next);
                p->Next = new (Malloc) temporary_allocator_data::page;

                // Random log-based growth thing I came up at the time, not real science.
                size_t loggedSize = (size_t) CEIL(p->Reserved * (LOG_2(p->Reserved * 10.0) / 3));
                size_t reserveTarget =
                    (MAX<size_t>(CEIL_POW_OF_2(size * 2), CEIL_POW_OF_2(loggedSize)) + 8_KiB - 1) & -8_KiB;

                p->Next->Storage = new (Malloc) char[reserveTarget];
                p->Next->Reserved = reserveTarget;
                p = p->Next;
            }

            void *result = (char *) p->Storage + p->Used;
            assert(result);

            p->Used += size;
            data->TotalUsed += size;

            if (mode == allocator_mode::ALIGNED_ALLOCATE) result = get_aligned_pointer(result, (size_t) align);

            return result;
        }
        // Reallocations aren't really viable with this allocator
        // so we just copy the old memory into a fresh block
        case allocator_mode::ALIGNED_REALLOCATE: {
            if (size <= oldSize) return oldMemory;
            void *result = temporary_allocator(allocator_mode::ALIGNED_ALLOCATE, context, size, null, 0, align, 0);
            copy_memory(result, oldMemory, oldSize);
            return result;
        }
        case allocator_mode::REALLOCATE: {
            if (size <= oldSize) return oldMemory;
            void *result = temporary_allocator(allocator_mode::ALLOCATE, context, size, null, 0, align, 0);
            copy_memory(result, oldMemory, oldSize);
            return result;
        }
        case allocator_mode::FREE:
            // We don't free individual allocations in the temporary allocator
            return null;
        case allocator_mode::FREE_ALL: {
            size_t targetSize = data->Base.Reserved;

            // Check if any overflow pages were used
            auto *page = data->Base.Next;
            while (page) {
                targetSize += page->Reserved;

                auto *next = page->Next;
                delete[] page->Storage;
                delete page;
                page = next;
            }
            data->Base.Next = null;

            // Resize _Storage_ to fit all allocations which previously required overflow pages
            if (targetSize != data->Base.Reserved) {
                delete[] data->Base.Storage;

                data->Base.Storage = new (Malloc) char[targetSize];
                data->Base.Reserved = targetSize;
            }

            data->TotalUsed = data->Base.Used = 0;
            // null means successful FREE_ALL
            // (void *) -1 means failure
            return null;
        }
        default:
            assert(false);
    }

#if COMPILER == MSVC
#pragma warning(pop)
#endif

    return null;
}

void implicit_context::release_temporary_allocator() const {
    if (!TemporaryAllocData.Base.Reserved) return;

    // Free any left-over overflow pages!
    TemporaryAlloc.free_all();

    delete[] TemporaryAllocData.Base.Storage;
    *const_cast<temporary_allocator_data *>(&TemporaryAllocData) = {};
}

LSTD_END_NAMESPACE

void *operator new(size_t size) { return operator new(size, null, 0); }
void *operator new[](size_t size) { return operator new(size, null, 0); }

void *operator new(size_t size, allocator *alloc, u64 userFlags) noexcept {
    if (!alloc) alloc = &const_cast<implicit_context *>(&Context)->Alloc;
    if (!*alloc) *alloc = Context.Alloc;
    assert(*alloc);
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
    if (!*alloc) *alloc = Context.Alloc;
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
