#include "pool.hpp"

LSTD_BEGIN_NAMESPACE

void pool::resize_blocks(size_t blockSize) {
    BlockSize = blockSize;

    if (_CurrentMemblock) _ObsoletedMemblocks.add(_CurrentMemblock);

    For(_UsedMemblocks) _ObsoletedMemblocks.add(it);

    _CurrentMemblock = null;
    _UsedMemblocks.Count = 0;
}

void pool::cycle_new_block() {
    if (_CurrentMemblock) _UsedMemblocks.add(_CurrentMemblock);

    u8 *newBlock;
    if (_UnusedMemblocks.Count) {
        newBlock = *(_UnusedMemblocks.end() - 1);
        _UnusedMemblocks.pop();
    } else {
        newBlock = new (&BlockAllocator, ensure_allocator) u8[BlockSize];
    }

    _BytesLeft = BlockSize;
    _CurrentPosition = newBlock;
    _CurrentMemblock = newBlock;
}

void pool::ensure_memory_exists(size_t size) {
    size_t bs = BlockSize;

    while (bs < size) {
        bs *= 2;
    }

    if (bs > BlockSize) resize_blocks(bs);
    cycle_new_block();
}

void pool::reset() {
    if (_CurrentMemblock) {
        _UnusedMemblocks.add(_CurrentMemblock);
        _CurrentMemblock = null;
    }

    For(_UsedMemblocks) { _UnusedMemblocks.add(it); }
    _UsedMemblocks.Count = 0;

    For(_ObsoletedMemblocks) { delete it; }
    _ObsoletedMemblocks.Count = 0;

    cycle_new_block();
}

void pool::release() {
    reset();

    For(_UnusedMemblocks) { delete it; }
}

void *pool::get(size_t size) {
    size_t extra = Alignment - (size % Alignment);
    size += extra;

    if (_BytesLeft < size) ensure_memory_exists(size);

    void *ret = _CurrentPosition;
    _CurrentPosition += size;
    _BytesLeft -= size;
    return ret;
}

void *pool_allocator(allocator_mode mode, void *allocatorData, size_t size, void *oldMemory, size_t oldSize, uptr_t) {
    auto *p = (pool *) allocatorData;

    switch (mode) {
        case allocator_mode::ALLOCATE:
            return p->get(size);
        case allocator_mode::RESIZE: {
            // Don't bother with resizing, get a new block and copy the memory to it.
            void *newMemory = p->get(size);
            copy_memory(oldMemory, newMemory, oldSize);
            return newMemory;
        }
        case allocator_mode::FREE:
            // This allocator only supports FREE_ALL
            return null;
        case allocator_mode::FREE_ALL:
            p->reset();
            return null;
    }
    return null;
}

LSTD_END_NAMESPACE
