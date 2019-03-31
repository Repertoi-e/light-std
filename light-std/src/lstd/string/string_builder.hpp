#pragma once

#include "../memory/memory_buffer.hpp"
#include "string.hpp"

LSTD_BEGIN_NAMESPACE

struct string_builder {
    static constexpr size_t BUFFER_SIZE = 1_KiB;

    struct buffer {
        byte Data[BUFFER_SIZE];
        size_t Occupied = 0;
        buffer *Next = null;
    };

    // Counts how many buffers have been dynamically allocated.
    size_t IndirectionCount = 0;

    buffer BaseBuffer;
    buffer *CurrentBuffer = &BaseBuffer;

    string_builder() = default;
    ~string_builder();

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
    // This value is null until this object allocates memory or the user sets it manually.
    allocator_closure Allocator;

    // Append a non encoded character to a string
    void append(char32_t codePoint);

    // Append a string to the builder
    void append(const memory_view &memory);

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const byte *data, size_t size);

    // Execute void f(string_view) on every buffer
    template <typename Lambda>
    void traverse(Lambda f) const {
        const buffer *buffer = &BaseBuffer;
        while (buffer) {
            f(string_view(buffer->Data, buffer->Occupied));
            buffer = buffer->Next;
        }
    }

    // Merges all buffers and returns a single string.
    string combine() const {
        string result;
        result.reserve((IndirectionCount + 1) * BUFFER_SIZE);

        const buffer *buffer = &BaseBuffer;
        while (buffer) {
            result.append_pointer_and_size(buffer->Data, buffer->Occupied);
            buffer = buffer->Next;
        }
        return result;
    }

    // Don't deallocate, just move cursor to 0
    void reset();

    // Free the entire builder
    void release();
};

LSTD_END_NAMESPACE
