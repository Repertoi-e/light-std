#pragma once

/// This file provides functions to manage pointers to allocated memory and objects than own them.

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Encodes _owner_ and returns (byte *) _data_ + POINTER_SIZE, i.e. the byte where storage begins.
template <typename T, typename U>
T *encode_owner(T *data, U *owner) {
    *((U **) data) = owner;
    return (T *) ((byte *) data + POINTER_SIZE);
}

// Changes the encoded owner. Acesses (byte *) data - POINTER_SIZE
template <typename T, typename U>
void change_owner(T *data, U *newOwner) {
    *((U **) data - 1) = newOwner;
}

// Returns the pointer encoded in data
template <typename U, typename T>
U *decode_owner(T *data) {
    return *((U **) ((byte *) data - POINTER_SIZE));
}

LSTD_END_NAMESPACE