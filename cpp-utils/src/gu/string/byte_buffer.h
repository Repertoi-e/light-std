#pragma once

#include "../memory/memory.h"

GU_BEGIN_NAMESPACE

struct Byte_Buffer {
    byte *Data = null;
    size_t Used = 0, _Reserved = 0;

    static const size_t MINIMUM_SIZE = 32;

    // The allocator used for expanding the buffer.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    Byte_Buffer() {}
    Byte_Buffer(Byte_Buffer const &other);
    Byte_Buffer(Byte_Buffer &&other);
    ~Byte_Buffer();

    Byte_Buffer &operator=(Byte_Buffer const &other);
    Byte_Buffer &operator=(Byte_Buffer &&other);
};

// Frees all memory allocated by the buffer.
void release(Byte_Buffer &buffer);

// Makes sure the buffer has atleast _size_ bytes reserved.
// Reserved bytes include the ones in use.
void reserve(Byte_Buffer &buffer, size_t size);

// This function accepts any type and appends its byte
// representation to the byte buffer.
template <typename T>
void append_reinterpreted(Byte_Buffer &buffer, T const &value) {
    if (buffer.Used + sizeof(T) > buffer._Reserved) {
        size_t newSize = buffer._Reserved * 2;
        if (newSize < buffer.MINIMUM_SIZE) newSize = buffer.MINIMUM_SIZE;

        reserve(buffer, newSize);
    }

    CopyMemory(buffer.Data + buffer.Used, reinterpret_cast<byte *>(&value), sizeof(T));
    buffer.Used += sizeof(T);
}

GU_END_NAMESPACE
