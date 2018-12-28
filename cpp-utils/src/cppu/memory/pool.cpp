#include "pool.hpp"

CPPU_BEGIN_NAMESPACE

void Pool::resize_blocks(size_t blockSize) {
    BlockSize = blockSize;

    if (CurrentMemblock) ObsoletedMemblocks.add(CurrentMemblock);

    for (u8 *it : UsedMemblocks) ObsoletedMemblocks.add(it);

    CurrentMemblock = 0;
    UsedMemblocks.Count = 0;
}

void Pool::cycle_new_block() {
    if (CurrentMemblock) UsedMemblocks.add(CurrentMemblock);

    u8 *newBlock;
    if (UnusedMemblocks.Count) {
        newBlock = *(UnusedMemblocks.end() - 1);
        UnusedMemblocks.pop();
    } else {
        newBlock = New_and_ensure_allocator<u8>(BlockSize, BlockAllocator);
    }

    _BytesLeft = BlockSize;
    CurrentPosition = newBlock;
    CurrentMemblock = newBlock;
}

void Pool::ensure_memory_exists(size_t size) {
    size_t bs = BlockSize;

    while (bs < size) {
        bs *= 2;
    }

    if (bs > BlockSize) resize_blocks(bs);
    cycle_new_block();
}

void Pool::reset() {
    if (CurrentMemblock) {
        UnusedMemblocks.add(CurrentMemblock);
        CurrentMemblock = 0;
    }

    for (u8 *it : UsedMemblocks) {
        UnusedMemblocks.add(it);
    }
    UsedMemblocks.Count = 0;

    for (u8 *it : ObsoletedMemblocks) {
        Delete(it, BlockAllocator);
    }
    ObsoletedMemblocks.Count = 0;

    cycle_new_block();
}

void Pool::release() {
    reset();

    for (u8 *it : UnusedMemblocks) {
        Delete(it, BlockAllocator);
    }
}

void *Pool::get(size_t size) {
    size_t extra = Alignment - (size % Alignment);
    size += extra;

    if (_BytesLeft < size) ensure_memory_exists(size);

    void *ret = CurrentPosition;
    CurrentPosition += size;
    _BytesLeft -= size;
    return ret;
}

void *pool_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, s32 options) {
    Pool *pool = (Pool *) data;

    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            return pool->get(size);
        case Allocator_Mode::RESIZE: {
            // Don't bother with resizing, get a new block and copy the memory to it.
            void *newMemory = pool->get(size);
            copy_memory(oldMemory, newMemory, oldSize);
            return newMemory;
        }
        case Allocator_Mode::FREE:
            // This allocator only supports FREE_ALL
            return 0;
        case Allocator_Mode::FREE_ALL:
            pool->reset();
            return 0;
    }
    return 0;
}

CPPU_END_NAMESPACE
