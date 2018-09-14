#include "pool.h"

GU_BEGIN_NAMESPACE

static void resize_blocks(Pool &pool, size_t blockSize) {
    pool.BlockSize = blockSize;

    if (pool._CurrentMemblock) add(pool._ObsoletedMemblocks, pool._CurrentMemblock);

    for (u8 *it : pool._UsedMemblocks) add(pool._ObsoletedMemblocks, it);

    pool._CurrentMemblock = 0;
    pool._UsedMemblocks.Count = 0;
}

static void cycle_new_block(Pool &pool) {
    if (pool._CurrentMemblock) add(pool._UsedMemblocks, pool._CurrentMemblock);

    u8 *newBlock;
    if (pool._UnusedMemblocks.Count) {
        newBlock = *(end(pool._UnusedMemblocks) - 1);
        pop(pool._UnusedMemblocks);
    } else {
        newBlock = New<u8>(pool.BlockSize, pool.BlockAllocator);
    }

    pool._BytesLeft = pool.BlockSize;
    pool._CurrentPosition = newBlock;
    pool._CurrentMemblock = newBlock;
}

static void ensure_memory_exists(Pool &pool, size_t size) {
    size_t bs = pool.BlockSize;

    while (bs < size) {
        bs *= 2;
    }

    if (bs > pool.BlockSize) resize_blocks(pool, bs);
    cycle_new_block(pool);
}

void *get(Pool &pool, size_t size) {
    size_t extra = pool.Alignment - (size % pool.Alignment);
    size += extra;

    if (pool._BytesLeft < size) ensure_memory_exists(pool, size);

    void *ret = pool._CurrentPosition;
    pool._CurrentPosition += size;
    pool._BytesLeft -= size;
    return ret;
}

void reset(Pool &pool) {
    if (pool._CurrentMemblock) {
        add(pool._UnusedMemblocks, pool._CurrentMemblock);
        pool._CurrentMemblock = 0;
    }

    for (u8 *it : pool._UsedMemblocks) {
        add(pool._UnusedMemblocks, it);
    }
    pool._UsedMemblocks.Count = 0;

    for (u8 *it : pool._ObsoletedMemblocks) {
        Delete(it, pool.BlockAllocator);
    }
    pool._ObsoletedMemblocks.Count = 0;

    cycle_new_block(pool);
}

void release(Pool &pool) {
    reset(pool);

    for (u8 *it : pool._UnusedMemblocks) {
        Delete(it, pool.BlockAllocator);
    }
}

void *__pool_allocator(Allocator_Mode mode, void *allocatorData, size_t size, void *oldMemory, size_t oldSize,
                       s32 options) {
    Pool &pool = *((Pool *) allocatorData);

    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            return get(pool, size);
        case Allocator_Mode::RESIZE: {
            // Don't bother with resizing, get a new block and copy the memory to it.
            void *newMemory = get(pool, size);
            CopyMemory(oldMemory, newMemory, oldSize);
            return newMemory;
        }
        case Allocator_Mode::FREE:
            // This allocator only supports FREE_ALL
            return 0;
        case Allocator_Mode::FREE_ALL:
            reset(pool);
            return 0;
    }
    return 0;
}

GU_END_NAMESPACE
