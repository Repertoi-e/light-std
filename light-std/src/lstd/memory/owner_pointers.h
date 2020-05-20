#pragma once

/// This file provides functions to manage pointers to allocated memory and objects than own them.

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Encodes _owner_ in the allocation header
template <typename T, typename U>
T *encode_owner(T *data, U *owner) {
    auto *header = (allocation_header *) data - 1;
    allocator::verify_header(header);
    header->Owner = owner;
    return data;
}

// Returns the pointer encoded in the header
template <typename U, typename T>
U *decode_owner(T *data) {
    auto *header = (allocation_header *) data - 1;
    allocator::verify_header(header);
    return (U *) header->Owner;
}

LSTD_END_NAMESPACE
