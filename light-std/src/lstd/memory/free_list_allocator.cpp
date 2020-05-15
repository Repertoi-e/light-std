#include "free_list_allocator.h"

LSTD_BEGIN_NAMESPACE

struct free_list_header {
    size_t BlockSize;
    size_t AlignmentPadding;
};

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

size_t free_list_allocator_data::find_first(size_t size, node **previousNode, node **foundNode) {
    size_t padding = 0;

    node *it = FreeListHead, *itPrev = null;
    while (it) {
        padding = calculate_padding_for_pointer_with_header(it, 16, sizeof(free_list_header));

        size_t requiredSpace = size + padding;
        if (it->BlockSize >= requiredSpace) break;
        itPrev = it;
        it = it->Next;
    }
    *previousNode = itPrev;
    *foundNode = it;

    return padding;
}

size_t free_list_allocator_data::find_best(size_t size, node **previousNode, node **foundNode) {
    size_t padding = 0;

    // Iterate the whole list while keeping a pointer to the best fit
    size_t smallestDiff = numeric_info<size_t>::max();

    node *it = FreeListHead, *itPrev = null, *bestBlock = null;
    while (it) {
        padding = calculate_padding_for_pointer_with_header(it, 16, sizeof(free_list_header));
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
    Storage = new (alignment(16), Malloc) char[totalSize];
    Reserved = totalSize;
    PlacementPolicy = policy;
    allocator{free_list_allocator, this}.free_all();  // Initializes linked list
}

void *free_list_allocator_data::allocate(size_t size) {
    if (allocator::AllocationCount == 72) {
        int k = 42;
    }

    assert(size >= sizeof(node) && "Allocation size must be bigger");

    // Search through the free list for a free block that has enough space to allocate our Data
    node *previousNode, *foundNode;
    size_t padding = PlacementPolicy == free_list_allocator_data::Find_First
                         ? find_first(size, &previousNode, &foundNode)
                         : find_best(size, &previousNode, &foundNode);
    if (!foundNode) return null;

    size_t alignmentPadding = padding - sizeof(free_list_header);
    size_t required = size + padding;

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
    // sanity();

    Used += required;
    PeakUsed = max(Used, PeakUsed);

    auto *header = (free_list_header *) ((char *) foundNode + alignmentPadding);
    header->BlockSize = required;
    header->AlignmentPadding = (char) alignmentPadding;

    return (void *) (header + 1);
}

void free_list_allocator_data::free(void *memory) {
    auto *header = (free_list_header *) memory - 1;

    size_t alignmentPadding = header->AlignmentPadding;

    auto *freeNode = (node *) ((char *) header - alignmentPadding);
    freeNode->BlockSize = header->BlockSize; // _BlockSize_ already includes padding
    freeNode->Next = null;

    node *it = FreeListHead, *itPrev = null;
    while (it) {
        if (memory < (void *) it) {
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
    
    // sanity();
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
        case allocator_mode::ALLOCATE:
            return data->allocate(size);
        case allocator_mode::REALLOCATE: {
            auto *newMemory = data->allocate(size);
            copy_memory(newMemory, oldMemory, oldSize);
            free_list_allocator(allocator_mode::FREE, context, 0, oldMemory, 0, 0);
            return newMemory;
        }
        case allocator_mode::FREE: {
            data->free(oldMemory);
            return null;
        }
        case allocator_mode::FREE_ALL: {
            data->Used = data->PeakUsed = 0;

            auto *firstNode = (node *) data->Storage;
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
