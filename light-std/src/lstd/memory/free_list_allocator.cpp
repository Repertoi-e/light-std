#include "free_list_allocator.h"

LSTD_BEGIN_NAMESPACE

struct free_list_header {
    size_t BlockSize;
    char Padding;
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

static size_t calculate_padding(size_t baseAddress, size_t align) {
    return (baseAddress / align + 1) * align - baseAddress;
}

static size_t calculate_padding_with_header(size_t baseAddress, size_t align, size_t headerSize) {
    size_t padding = calculate_padding(baseAddress, align);

    if (padding < headerSize) {
        headerSize -= padding;
        if (headerSize % align > 0) {
            padding += align * (1 + (headerSize / align));
        } else {
            padding += align * (headerSize / align);
        }
    }
    return padding;
}

size_t free_list_allocator_data::find_first(size_t size, size_t align, node **previousNode, node **foundNode) {
    size_t padding = 0;

    node *it = FreeListHead, *itPrev = null;

    while (it) {
        padding = calculate_padding_with_header((size_t) it, align, sizeof(free_list_header));

        size_t requiredSpace = size + padding;
        if (it->BlockSize >= requiredSpace) break;
        itPrev = it;
        it = it->Next;
    }
    *previousNode = itPrev;
    *foundNode = it;

    return padding;
}

size_t free_list_allocator_data::find_best(size_t size, size_t align, node **previousNode, node **foundNode) {
    size_t padding = 0;

    // Iterate the whole list while keeping a pointer to the best fit
    size_t smallestDiff = numeric_info<size_t>::max();

    node *it = FreeListHead, *itPrev = null, *bestBlock = null;
    while (it) {
        padding = calculate_padding_with_header((size_t) it, align, sizeof(free_list_header));
        size_t requiredSpace = size + padding;
        if (it->BlockSize >= requiredSpace && (it->BlockSize - requiredSpace < smallestDiff)) bestBlock = it;

        itPrev = it;
        it = it->Next;
    }
    *previousNode = itPrev;
    *foundNode = bestBlock;

    return padding;
}

void free_list_allocator_data::init(size_t totalSize, u8 policy) {
	Storage = new (Malloc) char[totalSize];
    Reserved = totalSize;

    PlacementPolicy = policy;

    // Initializes linked list
    allocator{free_list_allocator, this}.free_all();
}

void *free_list_allocator_data::allocate(size_t size, size_t align) {
    assert(size >= sizeof(node) && "Allocation size must be bigger");
    assert(align >= 8 && "Alignment must be 8 or more");

    // search through the free list for a free block that has enough space to allocate our Data
    node *affectedNode, *previousNode;
    size_t padding = 0;
    switch (PlacementPolicy) {
        case free_list_allocator_data::Find_First:
            padding = find_first(size, align, &previousNode, &affectedNode);
            break;
        case free_list_allocator_data::Find_Best:
            padding = find_best(size, align, &previousNode, &affectedNode);
            break;
        default:
            assert(false);
    }

    if (!affectedNode) return null;

    size_t alignmentPadding = padding - sizeof(free_list_header);
    size_t requiredSize = size + padding;

    size_t rest = affectedNode->BlockSize - requiredSize;

    if (rest > 0) {
        // We have to split the block into the Data block and a free block of size 'rest'
        auto *newFreeNode = (node *) ((size_t) affectedNode + requiredSize);
        newFreeNode->BlockSize = rest;

        if (!affectedNode) {
            newFreeNode->Next = FreeListHead;
            FreeListHead = newFreeNode;
        } else {
            if (!affectedNode->Next) {
                affectedNode->Next = newFreeNode;
                newFreeNode->Next = null;
            } else {
                newFreeNode->Next = affectedNode->Next;
                affectedNode->Next = newFreeNode;
            }
        }
    }

    if (!previousNode) {
        FreeListHead = affectedNode->Next;
    } else {
        previousNode->Next = affectedNode->Next;
    }

    // Setup data block
    size_t headerAddress = (size_t) affectedNode + alignmentPadding;
    size_t dataAddress = headerAddress + sizeof(free_list_header);
    ((free_list_header *) headerAddress)->BlockSize = requiredSize;
    ((free_list_header *) headerAddress)->Padding = (char) alignmentPadding;

    Used += requiredSize;
    PeakUsed = MAX(Used, PeakUsed);

    return (void *) dataAddress;
}

void free_list_allocator_data::free(void *memory) {
    size_t currentAddress = (size_t) memory;
    size_t headerAddress = currentAddress - sizeof(free_list_header);
    const free_list_header *allocationHeader{(free_list_header *) headerAddress};

    auto *freeNode = (node *) headerAddress;
    freeNode->BlockSize = allocationHeader->BlockSize + allocationHeader->Padding;
    freeNode->Next = null;

    node *it = FreeListHead, *itPrev = null;
    while (it) {
        if (memory < it) {
            if (!itPrev) {
                freeNode->Next = FreeListHead;
                FreeListHead = freeNode;
            } else {
                if (!itPrev->Next) {
                    itPrev->Next = freeNode;
                    freeNode->Next = null;
                } else {
                    // It is a middle node
                    freeNode->Next = itPrev->Next;
                    itPrev->Next = freeNode;
                }
            }
            break;
        }
        itPrev = it;
        it = it->Next;
    }
    Used -= freeNode->BlockSize;

    // Merge contiguous nodes
    coalescence(itPrev, freeNode);
}

void *free_list_allocator(allocator_mode mode, void *context, size_t size, void *oldMemory, size_t oldSize,
                          alignment align, u64) {
    auto *data = (free_list_allocator_data *) context;

    switch (mode) {
        case allocator_mode::ALLOCATE:
            return data->allocate(size);
        case allocator_mode::ALIGNED_ALLOCATE:
            return data->allocate(size, (size_t) align);
        case allocator_mode::REALLOCATE:
        case allocator_mode::ALIGNED_REALLOCATE: {
            auto *newMemory = data->allocate(size, (mode == allocator_mode::ALIGNED_REALLOCATE ? (size_t) align : 8));
            copy_memory(newMemory, oldMemory, oldSize);
            allocator{free_list_allocator, data}.free(oldMemory);
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
