#pragma once

/// This file provides functions to manage pointers to allocated memory and objects than own them.

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Encodes _owner_ in the allocation header
template <typename T, typename U, s32 Slot = 1>
T *encode_owner(T *data, U *owner) {
    static_assert(Slot == 1 || Slot == 2);

    auto *header = (allocation_header *) data - 1;
    assert(header->Pointer == data &&
           "Pointer doesn't have a header (probably isn't dynamic memory or"
           "it wasn't allocated with an allocator from this library)");
    if (Slot == 2) {
        header->UserData2 = owner;
    } else {
        header->UserData1 = owner;
    }
    return data;
}

// Changes the encoded owner
template <typename T, typename U, s32 Slot = 1>
void change_owner(T *data, U *newOwner) {
    static_assert(Slot == 1 || Slot == 2);

    auto *header = (allocation_header *) data - 1;
    assert(header->Pointer == data &&
           "Pointer doesn't have a header (probably isn't dynamic memory or"
           "it wasn't allocated with an allocator from this library)");
    if (Slot == 2) {
        header->UserData2 = newOwner;
    } else {
        header->UserData1 = newOwner;
    }
}

// Returns the pointer encoded in the header
template <typename U, typename T, s32 Slot = 1>
U *decode_owner(T *data) {
    static_assert(Slot == 1 || Slot == 2);

    auto *header = (allocation_header *) data - 1;
    if (header->Pointer != data) return null;
    if (Slot == 2) {
        return (U *) header->UserData2;
    } else {
        return (U *) header->UserData1;
    }
}

LSTD_END_NAMESPACE
