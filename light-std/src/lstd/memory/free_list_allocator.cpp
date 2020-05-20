#include "free_list_allocator.h"

#include "array.h"

LSTD_BEGIN_NAMESPACE

using node = free_list_allocator_data::node;

static void coalescence(node *previousNode, node *freeNode) {
    if (freeNode->Next && (size_t) freeNode + freeNode->BlockSize == (size_t) freeNode->Next) {
        freeNode->BlockSize += freeNode->Next->BlockSize;
        freeNode->Next = freeNode->Next->Next;
    }

    if (previousNode && (size_t) previousNode + previousNode->BlockSize == (size_t) freeNode) {
        previousNode->BlockSize += freeNode->BlockSize;
        previousNode->Next = freeNode->Next;
    }
}

u16 free_list_allocator_data::find_first(size_t size, node **previousNode, node **foundNode) {
    u16 padding = 0;

    node *it = FreeListHead, *itPrev = null;
    while (it) {
        padding = calculate_padding_for_pointer_with_header(it, 16, sizeof(block_header));

        size_t requiredSpace = size + padding;
        if (it->BlockSize >= requiredSpace) break;
        itPrev = it;
        it = it->Next;
    }
    *previousNode = itPrev;
    *foundNode = it;

    return padding;
}

u16 free_list_allocator_data::find_best(size_t size, node **previousNode, node **foundNode) {
    u16 padding = 0;

    // Iterate the whole list while keeping a pointer to the best fit
    size_t smallestDiff = numeric_info<size_t>::max();

    node *it = FreeListHead, *itPrev = null, *bestBlock = null;
    while (it) {
        padding = calculate_padding_for_pointer_with_header(it, 16, sizeof(block_header));
        size_t requiredSpace = size + padding;
        size_t diff = it->BlockSize - requiredSpace;
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

void free_list_allocator_data::init(size_t totalSize, u8 policy) {
    Storage = operator new(totalSize, Malloc);
    Reserved = totalSize;
    PlacementPolicy = policy;
    allocator{free_list_allocator, this}.free_all();  // Initializes linked list
}

void *free_list_allocator_data::allocate(size_t size) {
    auto userSize = size;

    assert(userSize >= sizeof(node) && "Allocation size must be bigger");

    // Search through the free list for a free block that has enough space to allocate our Data
    node *previousNode, *foundNode;
    u16 padding = PlacementPolicy == free_list_allocator_data::Find_First
                      ? find_first(userSize, &previousNode, &foundNode)
                      : find_best(userSize, &previousNode, &foundNode);
    if (!foundNode) return null;

    u16 alignmentPadding = padding - sizeof(block_header);
    size_t required = userSize + padding;

    size_t rest = foundNode->BlockSize - required;
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

    auto *header = (block_header *) foundNode;
    // We write the flag before the padding!
    header->Flag = USED_BLOCK_FLAG;

    header = (block_header *) ((char *) foundNode + alignmentPadding);
    header->Size = userSize;
    header->AlignmentPadding = alignmentPadding;

    return (void *) (header + 1);
}

void *free_list_allocator_data::resize(void *block, size_t newSize) {
    auto *header = (block_header *) block - 1;

#if defined DEBUG_MEMORY
    auto *p = (u32 *) (char *) header - header->AlignmentPadding;
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
                freeNode->BlockSize -= (size_t) diff;
            } else {
                return null;  // Not enough space in next free node
            }
        } else {
            freeNode->BlockSize += (size_t) -diff;  // If we are shrinking, tell the free node it has more space now.
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

void free_list_allocator_data::free(void *block) {
    auto *header = (block_header *) block - 1;

    size_t alignmentPadding = header->AlignmentPadding;

    auto *freeNode = (node *) ((char *) header - alignmentPadding);
    freeNode->Flag = free_list_allocator_data::FREE_BLOCK_FLAG;
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
            assert((size_t) it + it->BlockSize <= (size_t) it->Next);
        }
        it = it->Next;
    }
}

void *free_list_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize, u64) {
    auto *data = (free_list_allocator_data *) context;

    switch (mode) {
        case allocator_mode::ALLOCATE: {
            return data->allocate(size);
        }
        case allocator_mode::RESIZE: {
            return data->resize(oldMemory, size);
        }
        case allocator_mode::FREE: {
            data->free(oldMemory);
            return null;
        }
        case allocator_mode::FREE_ALL: {
#if defined DEBUG_MEMORY
            // Remove our allocations from the linked list
            WITH_ALLOC(Malloc) {
                array<allocation_header *> toUnlink;
                auto *h = allocator::DEBUG_Head;
                while (h) {
                    if (h->Function == free_list_allocator && h->Context == data) {
                        toUnlink.append(h);
                    }
                    h = h->DEBUG_Next;
                }
                For(toUnlink) allocator::DEBUG_unlink_header(it);
            } 
#endif

            data->Used = data->PeakUsed = 0;

            auto *firstNode = (node *) data->Storage;
            firstNode->Flag = free_list_allocator_data::FREE_BLOCK_FLAG;
            firstNode->BlockSize = data->Reserved;
            firstNode->Next = null;
            data->FreeListHead = firstNode;

            return (void *) -1;
        }
        default:
            assert(false);
    }
    return null;
}

LSTD_END_NAMESPACE
