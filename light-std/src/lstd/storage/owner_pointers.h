#pragma once

/// This file provides functions to manage pointers to allocated memory and objects than own them.

#include "../types.h"

LSTD_BEGIN_NAMESPACE

// Encodes _owner_ and returns (char *) _data_ + POINTER_SIZE, i.e. the char where storage begins.
template <typename T, typename U>
T *encode_owner(T *data, U *owner) {
    *((U **) data) = owner;
    return (T *) ((char *) data + POINTER_SIZE);
}

// Changes the encoded owner. Acesses (char *) data - POINTER_SIZE
template <typename T, typename U>
void change_owner(T *data, U *newOwner) {
    *((U **) data - 1) = newOwner;
}

// Returns the pointer encoded in data
template <typename U, typename T>
U *decode_owner(T *data) {
    return *((U **) ((char *) data - POINTER_SIZE));
}

LSTD_END_NAMESPACE