#pragma once

#include "allocator.h"

LSTD_BEGIN_NAMESPACE

struct free_list_allocator_data {
    void *Storage = null;
    size_t Reserved = 0;

    struct node {
        size_t BlockSize = 0;
        node *Next = null;
    };
    node *FreeListHead = null;

    size_t Used = 0, PeakUsed = 0;

    enum placement_policy : u8 { Find_First = 0, Find_Best };
    u8 PlacementPolicy;

    void init(size_t totalSize, u8 policy);

   private:
    void *allocate(size_t size);
    void free(void *memory);

    void sanity();

    size_t find_first(size_t size, node **previousNode, node **foundNode);
    size_t find_best(size_t size, node **previousNode, node **foundNode);

    friend void *free_list_allocator(allocator_mode, void *, size_t, void *, size_t, u64);
};

// This allocator uses a pre-allocated large block and book keeps free nodes with a linked list.
// It's suited for general use.
// Supports two ways of finding free blocks - FIND_FIRST (faster), FIND_BEST (less memory fragmentation).
void *free_list_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64);

LSTD_END_NAMESPACE
