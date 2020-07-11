#include "../os.h"
#include "allocator.h"

LSTD_BEGIN_NAMESPACE

void *default_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 *) {
    switch (mode) {
        case allocator_mode::ALLOCATE:
            return os_allocate_block(size);
        case allocator_mode::RESIZE:
            assert(os_get_block_size(oldMemory) == oldSize);  // Sanity
            return os_resize_block(oldMemory, size);
        case allocator_mode::FREE:
            assert(os_get_block_size(oldMemory) == oldSize);  // Sanity
            os_free_block(oldMemory);
            return null;
        case allocator_mode::FREE_ALL:
            return (void *) -1;
        default:
            assert(false);
    }
    return null;
}

LSTD_END_NAMESPACE
