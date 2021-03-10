#include "free_list_allocator.h"

#include "../types/numeric_info.h"
#include "array.h"

LSTD_BEGIN_NAMESPACE

using node = free_list_allocator_data::node;

file_scope void coalescence(node *previousNode, node *freeNode) {
    if (freeNode->Next && (s64) freeNode + freeNode->BlockSize == (s64) freeNode->Next) {
        freeNode->BlockSize += freeNode->Next->BlockSize;
        freeNode->Next = freeNode->Next->Next;
    }

    if (previousNode && (s64) previousNode + previousNode->BlockSize == (s64) freeNode) {
        previousNode->BlockSize += freeNode->BlockSize;
        previousNode->Next = freeNode->Next;
    }
}

u16 free_list_allocator_data::find_first(s64 size, node **previousNode, node **foundNode) {
    u16 padding = 0;

    node *it = FreeListHead, *itPrev = null;
    while (it) {
        padding = calculate_padding_for_pointer_with_header(it, 16, sizeof(block_header));

        s64 requiredSpace = size + padding;
        if (it->BlockSize >= requiredSpace) break;
        itPrev = it;
        it = it->Next;
    }
    *previousNode = itPrev;
    *foundNode = it;

    return padding;
}

u16 free_list_allocator_data::find_best(s64 size, node **previousNode, node **foundNode) {
    u16 padding = 0;

    // Iterate the whole list while keeping a pointer to the best fit
    s64 smallestDiff = numeric_info<s64>::max();

    node *it = FreeListHead, *itPrev = null, *bestBlock = null;
    while (it) {
        padding = calculate_padding_for_pointer_with_header(it, 16, sizeof(block_header));
        s64 requiredSpace = size + padding;
        s64 diff = it->BlockSize - requiredSpace;
        if (it->BlockSize >= requiredSpace && (diff < smallestDiff)) {
            bestBlock = it;
            smallestDiff = diff;
        }

        itPrev = it;
        it = it->Next;
    }
    *previousNode = itPrev;
    *foundNode = bestBlock;

    return padding;
}

void free_list_allocator_data::init(s64 totalSize, u8 policy) {
    Storage = allocate_array<byte>(totalSize, {.Alloc = DefaultAlloc});
    Allocated = totalSize;
    PlacementPolicy = policy;
    free_all(allocator{free_list_allocator, this});  // Initializes the linked list
}

void free_list_allocator_data::release() {
    free(Storage);
    Allocated = 0;
    FreeListHead = null;
    Storage = null;
}

void *free_list_allocator_data::allocate_block(s64 size) {
    auto userSize = size;

    assert(userSize >= sizeof(node) && "Allocation size must be bigger");

    // Search through the free list for a free block that has enough space to allocate our Data
    node *previousNode, *foundNode;
    u16 padding = PlacementPolicy == free_list_allocator_data::Find_First
                      ? find_first(userSize, &previousNode, &foundNode)
                      : find_best(userSize, &previousNode, &foundNode);
    if (!foundNode) return null;

    u16 alignmentPadding = padding - sizeof(block_header);
    s64 required = userSize + padding;

    s64 rest = foundNode->BlockSize - required;
    if (rest > 0) {
        // We have to split the block into the Data block and a free block of size _rest_
        auto *newFreeNode = (node *) ((char *) foundNode + required);
        newFreeNode->BlockSize = rest;
        newFreeNode->Next = foundNode->Next;
        foundNode->Next = newFreeNode;
    }
    if (previousNode) {
        previousNode->Next = foundNode->Next;
    } else {
        FreeListHead = foundNode->Next;
    }

#if defined DEBUG_MEMORY
    sanity();
#endif

    Used += required;
    PeakUsed = max(Used, PeakUsed);

    auto *header = (block_header *) ((char *) foundNode + alignmentPadding);
    header->Size = userSize;
    header->AlignmentPadding = alignmentPadding;

    return (void *) (header + 1);
}

/*
void *free_list_allocator_data::resize_block(void *block, s64 newSize) {
    auto *header = (block_header *) block - 1;

#if defined DEBUG_MEMORY
    auto *p = (u32 *) ((char *) header - header->AlignmentPadding);
    assert(*p == USED_BLOCK_FLAG);  // Sanity
#endif

    s64 diff = (s64) newSize - (s64) header->Size;

    auto *nextBlock = (char *) block + header->Size;
    auto *flag = (u32 *) nextBlock;

    // There is a guarantee to be no padding if the block is a free block, so the flag must be at that location.
    if (*flag == FREE_BLOCK_FLAG) {
        auto *freeNode = (node *) nextBlock;

        if (diff > 0) {
            if ((s64) freeNode->BlockSize >= diff) {
                freeNode->BlockSize -= (s64) diff;
            } else {
                return null;  // Not enough space in next free node
            }
        } else {
            freeNode->BlockSize += (s64) -diff;  // If we are shrinking, tell the free node it has more space now.
        }

        auto *moved = (char *) freeNode + diff;
        copy_memory(moved, freeNode, sizeof(node));

        // Find the previous free node
        node *it = FreeListHead, *itPrev = null;
        while (it) {
            if (it == freeNode) break;
            itPrev = it;
            it = it->Next;
        }
        itPrev->Next = (node *) moved;

        return block;
    } else {
        return null;  // We can't resize it, next node is not free
    }
}
*/

void free_list_allocator_data::free_block(void *block) {
    auto *header = (block_header *) block - 1;

    s64 alignmentPadding = header->AlignmentPadding;

    auto *freeNode = (node *) ((char *) header - alignmentPadding);
    freeNode->BlockSize = header->Size + header->AlignmentPadding + sizeof(block_header);
    freeNode->Next = null;

    node *it = FreeListHead, *itPrev = null;
    while (it) {
        if (block < (void *) it) {
            if (!itPrev) {
                freeNode->Next = FreeListHead;
                FreeListHead = freeNode;
            } else {
                freeNode->Next = itPrev->Next;
                itPrev->Next = freeNode;
            }
            break;
        }
        itPrev = it;
        it = it->Next;
    }
    Used -= freeNode->BlockSize;

    // Merge contiguous nodes
    coalescence(itPrev, freeNode);

#if defined DEBUG_MEMORY
    sanity();
#endif
}

void free_list_allocator_data::sanity() {
    node *it = FreeListHead;
    while (it) {
        if (it->Next) {
            assert((s64) it + it->BlockSize <= (s64) it->Next);
        }
        it = it->Next;
    }
}

void *free_list_allocator(allocator_mode mode, void *context, s64 size, void *oldMemory, s64 oldSize, u64 *) {
    auto *data = (free_list_allocator_data *) context;

    switch (mode) {
        case allocator_mode::ALLOCATE: {
            return data->allocate_block(size);
        }
        case allocator_mode::RESIZE: {
            return null;
            // return data->resize(oldMemory, size);
        }
        case allocator_mode::FREE: {
            data->free_block(oldMemory);
            return null;
        }
        case allocator_mode::FREE_ALL: {
            data->Used = data->PeakUsed = 0;

            auto *firstNode = (node *) data->Storage;
            firstNode->BlockSize = data->Allocated;
            firstNode->Next = null;
            data->FreeListHead = firstNode;

            return null;
        }
        default:
            assert(false);
    }
    return null;
}

LSTD_END_NAMESPACE
