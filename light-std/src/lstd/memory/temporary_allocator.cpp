#include "../internal/context.h"
#include "allocator.h"
#include "array.h"

LSTD_BEGIN_NAMESPACE

void *temporary_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64) {
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
                size_t loggedSize = (size_t) ceil(p->Reserved * (log2(p->Reserved * 10.0) / 3));
                size_t reserveTarget =
                    (max<size_t>(ceil_pow_of_2(size * 2), ceil_pow_of_2(loggedSize)) + 8_KiB - 1) & -8_KiB;

                p->Next->Storage = new (Malloc) char[reserveTarget];
                p->Next->Reserved = reserveTarget;
                p = p->Next;
            }

            void *result = (char *) p->Storage + p->Used;
            assert(result);

            p->Used += size;
            data->TotalUsed += size;
            return result;
        }
        case allocator_mode::RESIZE: {
            auto *p = &data->Base;
            while (p->Next) {
                if (p->Used + size < p->Reserved) break;
                p = p->Next;
            }

            s64 diff = (s64) size - (s64) oldSize;

            void *possiblyThisBlock = (char *) p->Storage + p->Used - oldSize;

            // We support resizing only on the last allocation (this still covers lots of cases,
            // e.g. constructing a string and then immediately appending to it!).
            if (possiblyThisBlock == oldMemory) {
                if (p->Used + diff >= p->Reserved) return null;  // Not enough space
                p->Used += diff;
                return oldMemory;
            }
            return null;
        }
        case allocator_mode::FREE:
            // We don't free individual allocations in the temporary allocator
            return null;
        case allocator_mode::FREE_ALL: {
#if defined DEBUG_MEMORY
            // Remove our allocations from the linked list
            WITH_ALLOC(Malloc) {
                array<allocation_header *> toUnlink;
                auto *h = allocator::DEBUG_Head;
                while (h) {
                    if (h->Function == temporary_allocator && h->Context == data) {
                        toUnlink.append(h);
                    }
                    h = h->DEBUG_Next;
                }
                For(toUnlink) allocator::DEBUG_unlink_header(it);
            }
#endif

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

LSTD_END_NAMESPACE
