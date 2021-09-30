#include "../common.h"
#include "vendor/tlsf/tlsf.h"

import lstd.memory;

LSTD_BEGIN_NAMESPACE

void *tlsf_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 options) {
    assert(context);

    auto *data = (tlsf_allocator_data *) context;

    if (!data->State && mode != allocator_mode::ADD_POOL) {
        assert(false && "No pools have been added yet! Add the first one with allocator_add_pool().");
        return null;
    }

    switch (mode) {
        case allocator_mode::ADD_POOL: {
            auto *pool = (allocator_pool *) oldMemory;  // _oldMemory_ is the parameter which should contain the block to be added,
            // the _size_ parameter contains the size of the block

            if (!data->State) {
                data->State = tlsf_create_with_pool(pool, (u64) size);
                return data->State;
            }
            return tlsf_add_pool(data->State, pool, (u64) size);
        }
        case allocator_mode::REMOVE_POOL: {
            auto *pool = (allocator_pool *) oldMemory;
            tlsf_remove_pool(data->State, pool);  // This function assumes the block exists
            return pool;
        }
        case allocator_mode::ALLOCATE: {
            return tlsf_malloc(data->State, size);
        }
        case allocator_mode::RESIZE: {
            return tlsf_resize(data->State, oldMemory, size);
        }
        case allocator_mode::FREE: {
            tlsf_free(data->State, oldMemory);
            return null;
        }
        case allocator_mode::FREE_ALL: {
            assert(false);  // Some allocators can't support this by design
            return null;
        }
    }
    return null;
}

LSTD_END_NAMESPACE
