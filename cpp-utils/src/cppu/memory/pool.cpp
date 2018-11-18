#include "pool.h"

CPPU_BEGIN_NAMESPACE

void Pool::_resize_blocks(size_t blockSize) {
    BlockSize = blockSize;

    if (_CurrentMemblock) _ObsoletedMemblocks.add(_CurrentMemblock);

    for (u8 *it : _UsedMemblocks) _ObsoletedMemblocks.add(it);

    _CurrentMemblock = 0;
    _UsedMemblocks.Count = 0;
}

void Pool::_cycle_new_block() {
    if (_CurrentMemblock) _UsedMemblocks.add(_CurrentMemblock);

    u8 *newBlock;
    if (_UnusedMemblocks.Count) {
        newBlock = *(_UnusedMemblocks.end() - 1);
        _UnusedMemblocks.pop();
    } else {
        newBlock = New_And_Ensure_Allocator<u8>(BlockSize, BlockAllocator);
    }

    _BytesLeft = BlockSize;
    _CurrentPosition = newBlock;
    _CurrentMemblock = newBlock;
}

void Pool::_ensure_memory_exists(size_t size) {
    size_t bs = BlockSize;

    while (bs < size) {
        bs *= 2;
    }

    if (bs > BlockSize) _resize_blocks(bs);
    _cycle_new_block();
}

void Pool::reset() {
    if (_CurrentMemblock) {
        _UnusedMemblocks.add(_CurrentMemblock);
        _CurrentMemblock = 0;
    }

    for (u8 *it : _UsedMemblocks) {
        _UnusedMemblocks.add(it);
    }
    _UsedMemblocks.Count = 0;

    for (u8 *it : _ObsoletedMemblocks) {
        Delete(it, BlockAllocator);
    }
    _ObsoletedMemblocks.Count = 0;

    _cycle_new_block();
}

void Pool::release() {
    reset();

    for (u8 *it : _UnusedMemblocks) {
        Delete(it, BlockAllocator);
    }
}

void *Pool::get(size_t size) {
    size_t extra = Alignment - (size % Alignment);
    size += extra;

    if (_BytesLeft < size) _ensure_memory_exists(size);

    void *ret = _CurrentPosition;
    _CurrentPosition += size;
    _BytesLeft -= size;
    return ret;
}

void *__pool_allocator(Allocator_Mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, s32 options) {
    Pool *pool = (Pool *) data;

    switch (mode) {
        case Allocator_Mode::ALLOCATE:
            return pool->get(size);
        case Allocator_Mode::RESIZE: {
            // Don't bother with resizing, get a new block and copy the memory to it.
            void *newMemory = pool->get(size);
            CopyMemory(oldMemory, newMemory, oldSize);
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
