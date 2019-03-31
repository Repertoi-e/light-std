#pragma once

#include "dynamic_array.hpp"

LSTD_BEGIN_NAMESPACE

struct pool {
    size_t BlockSize = 65536;
    size_t Alignment = 8;

    // The allocator used for reserving the initial memory block
    // This value is null until this object allocates memory or the user sets it manually.
    allocator_closure BlockAllocator;

    pool() {}
    pool(const pool &other) = delete;
    pool(pool &&other) = delete;
    pool &operator=(const pool &other) = delete;
    pool &operator=(pool &&other) = delete;

    // Resets and frees the pool
    void release();

    // Resets the pool without releasing the allocated memory.
    void reset();

    // Gets _size_ bytes of memory from the pool.
    // Handles running out of memory in the current block.
    void *get(size_t size);

   private:
    void resize_blocks(size_t blockSize);
    void cycle_new_block();
    void ensure_memory_exists(size_t size);

    dynamic_array<u8 *> _UnusedMemblocks;
    dynamic_array<u8 *> _UsedMemblocks;
    dynamic_array<u8 *> _ObsoletedMemblocks;

    u8 *_CurrentMemblock = null;
    u8 *_CurrentPosition = null;
    size_t _BytesLeft = 0;
};

// The allocator function that works with a pool.
// As you can see, there is no "free" function defined above,
// that's because Pool doesn't manage freeing of invidual pieces
// of memory. So calling pool_allocator with Allocator_Mode::FREE,
// doesn't do anything. Allocator_Mode::FREE_ALL does tho.
void *pool_allocator(allocator_mode mode, void *allocatorData, size_t size, void *oldMemory, size_t oldSize, uptr_t);

LSTD_END_NAMESPACE