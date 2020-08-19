#pragma once

#include "allocator.h"

LSTD_BEGIN_NAMESPACE

struct free_list_allocator_data : non_copyable, non_movable, non_assignable {
    void *Storage = null;
    s64 Allocated = 0;

    struct node {
        s64 BlockSize = 0;
        node *Next = null;
    };

    struct block_header {
        s64 Size;
        u16 AlignmentPadding;  // This might move _Flag_ but its ok, we save it before the padding anyways
    };

    node *FreeListHead = null;

    s64 Used = 0, PeakUsed = 0;

    enum placement_policy : u8 { Find_First = 0,
                                 Find_Best };
    u8 PlacementPolicy;

    void init(s64 totalSize, u8 policy);
    void release();

   private:
    void *allocate_block(s64 size);
    void *resize_block(void *block, s64 newSize);
    void free_block(void *block);

    void sanity();

    u16 find_first(s64 size, node **previousNode, node **foundNode);
    u16 find_best(s64 size, node **previousNode, node **foundNode);

    friend void *free_list_allocator(allocator_mode, void *, s64, void *, s64, u64 *);
};

// This allocator uses a pre-allocated large block and book keeps free nodes with a linked list.
// It's suited for general use.
// Supports two ways of finding free blocks - FIND_FIRST (faster), FIND_BEST (less memory fragmentation).
void *free_list_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 *);

LSTD_END_NAMESPACE
