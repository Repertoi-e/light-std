#include "free_list_allocator.h"

LSTD_BEGIN_NAMESPACE

struct free_list_header {
    size_t BlockSize;
    size_t Padding;
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

static size_t calculate_padding_with_header(void *ptr, size_t align) {
    size_t padding = calculate_padding(ptr, align);

    auto headerSize = sizeof(free_list_header);
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
        padding = calculate_padding_with_header(it, align);

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
        padding = calculate_padding_with_header(it, align);
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
    Storage = new (Malloc) char[totalSize];
    Reserved = totalSize;
    PlacementPolicy = policy;
    allocator{free_list_allocator, this}.free_all();  // Initializes linked list
}

void *free_list_allocator_data::allocate(size_t size, size_t align) {
    assert(size >= sizeof(node) && "Allocation size must be bigger");
    assert(IS_POW_OF_2(align));
    align = align < sizeof(free_list_header) ? sizeof(free_list_header) : align;

    // Search through the free list for a free block that has enough space to allocate our Data
    node *previousNode, *foundNode;
    size_t padding = PlacementPolicy == free_list_allocator_data::Find_First
                         ? find_first(size, align, &previousNode, &foundNode)
                         : find_best(size, align, &previousNode, &foundNode);
    if (!foundNode) return null;

    size_t alignmentPadding = padding - sizeof(free_list_header);
    size_t required = size + padding;

    size_t rest = foundNode->BlockSize - required;
    if (rest > 0) {
        // We have to split the block into the Data block and a free block of size 'rest'
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

    Used += required;
    PeakUsed = MAX(Used, PeakUsed);

    auto *header = (free_list_header *) ((char *) foundNode + alignmentPadding);
    header->BlockSize = required;
    header->Padding = (char) alignmentPadding;

    return (void *) (header + 1);
}

void free_list_allocator_data::free(void *memory) {
    auto *header = (free_list_header *) memory - 1;

    auto newBlockSize = header->BlockSize + header->Padding;
    auto *freeNode = (node *) header;
    freeNode->BlockSize = newBlockSize;
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
            free_list_allocator(allocator_mode::FREE, context, 0, oldMemory, 0, alignment(0), 0);
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
