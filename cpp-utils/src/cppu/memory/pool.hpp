#pragma once

#include "dynamic_array.hpp"

CPPU_BEGIN_NAMESPACE

struct Pool {
    size_t BlockSize = 65536;
    size_t Alignment = 8;

    Dynamic_Array<u8 *> _UnusedMemblocks;
    Dynamic_Array<u8 *> _UsedMemblocks;
    Dynamic_Array<u8 *> _ObsoletedMemblocks;

    u8 *_CurrentMemblock = 0;
    u8 *_CurrentPosition = 0;
    size_t _BytesLeft = 0;

    // The allocator used for reserving the initial memory block
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure BlockAllocator;

    Pool() {}
    Pool(const Pool &other) = delete;
    Pool(Pool &&other) = delete;
    Pool &operator=(const Pool &other) = delete;
    Pool &operator=(Pool &&other) = delete;

    // Resets and frees the pool
    void release();

    // Resets the pool without releasing the allocated memory.
    void reset();

    // Gets _size_ bytes of memory from the pool.
    // Handles running out of memory in the current block.
    void *get(size_t size);

    void _resize_blocks(size_t blockSize);
    void _cycle_new_block();
    void _ensure_memory_exists(size_t size);
};

// The allocator function that works with a pool.
// As you can see, there is no "free" function defined above,
// that's because Pool doesn't manage freeing of invidual pieces
// of memory. So calling pool_allocator with Allocator_Mode::FREE,
// doesn't do anything. Allocator_Mode::FREE_ALL does tho.
void *pool_allocator(Allocator_Mode mode, void *allocatorData, size_t size, void *oldMemory, size_t oldSize,
                       s32 options);

CPPU_END_NAMESPACE