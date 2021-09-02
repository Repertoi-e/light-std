#include "../common/context.h"
#include "allocator.h"
#include "string.h"

import os;

LSTD_BEGIN_NAMESPACE

#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)
#endif

void *arena_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (arena_allocator_data *) context;

    switch (mode) {
        case allocator_mode::ADD_POOL: {
            auto *pool = (allocator_pool *) oldMemory; // _oldMemory_ is the parameter which should contain the block to be added
            // the _size_ parameter contains the size of the block

            if (!allocator_pool_initialize(pool, size)) return null;
            allocator_pool_add_to_linked_list(&data->Base, pool);
            if (pool) {
                ++data->PoolsCount;
                return pool;
            }
            return null;
        }
        case allocator_mode::REMOVE_POOL: {
            auto *pool = (allocator_pool *) oldMemory;

            void *result = allocator_pool_remove_from_linked_list(&data->Base, pool);
            if (result) {
                --data->PoolsCount;
                assert(data->PoolsCount >= 0);
                return result;
            }
            return null;
        }
        case allocator_mode::ALLOCATE: {
            auto *p = data->Base;
            while (p->Next) {
                if (p->Used + size < p->Size) break;
                p = p->Next;
            }

            if (p->Used + size >= p->Size) return null; // Not enough space

            void *usableBlock = p + 1;
            void *result      = (byte *) usableBlock + p->Used;

            p->Used += size;
            data->TotalUsed += size;

            return result;
        }
        case allocator_mode::RESIZE: {
            // Implementing a fast RESIZE requires finding in which block the memory is in.
            // We might store a header which tells us that but right now I don't think it's worth it.
            // We simply return null and let the reallocate function allocate a new block and copy the contents.
            //
            // If you are dealing with very very large blocks and copying is expensive, you should
            // implement a specialized allocator. If you are dealing with appending to strings
            // (which causes string to try to reallocate), we provide a string_builder utility which will help with that.
            return null;
        }
        case allocator_mode::FREE: {
            // We don't free individual allocations in the arena allocator

            // null means success FREE
            return null;
        }
        case allocator_mode::FREE_ALL: {
            auto *p = data->Base;
            while (p) {
                p->Used = 0;
                p       = p->Next;
            }

            data->TotalUsed = 0;

            // null means successful FREE_ALL
            // (void *) -1 means that the allocator doesn't support FREE_ALL (by design)
            return null;
        }
        default:
            assert(false);
    }
    return null;
}

void *default_temp_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    auto *data = (arena_allocator_data *) context;

    if (!data->Base) {
        s64 startingPoolSize = 8_KiB;

        // Make sure the starting pool has enough space for the allocation we are about to do
        if (mode == allocator_mode::ALLOCATE) {
            if (startingPoolSize < size) startingPoolSize = ceil_pow_of_2(size * 2);
        } else if (mode != allocator_mode::ADD_POOL) {
            // If we called with ADD_POOL, don't add the starting pool.
            allocator_add_pool({arena_allocator, data}, os_allocate_block(startingPoolSize), startingPoolSize);
        }
    }

    auto *result = arena_allocator(mode, context, size, oldMemory, oldSize, options);
    if (mode == allocator_mode::ALLOCATE && !result) {
        // If we tried to allocate a block but didn't have enough space, we make a new, larger pool and print a warning.
        // This is default behaviour which you can override by providing your own custom allocator extension.
        //
        // You can avoid this by freeing all periodically or by manually adding a large enough pool at the beginning of your program.
        internal::platform_report_warning("Not enough space in temporary allocator; adding a pool");

        s64 poolSize = 8_KiB;
        if (poolSize < size) poolSize = ceil_pow_of_2(size * 2);

        allocator_add_pool({arena_allocator, data}, os_allocate_block(poolSize), poolSize);

        result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0, options);
    }

    return result;
}

#if COMPILER == MSVC
#pragma warning(pop)
#endif

LSTD_END_NAMESPACE
