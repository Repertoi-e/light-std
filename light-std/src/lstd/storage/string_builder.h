#pragma once

#include "string.h"

LSTD_BEGIN_NAMESPACE

struct string_builder {
    static constexpr size_t BUFFER_SIZE = 1_KiB;

    struct buffer {
        char Data[BUFFER_SIZE]{};
        size_t Occupied = 0;
        buffer *Next = null;

        string_builder *Owner = null;
    };

    // Counts how many buffers have been dynamically allocated.
    size_t IndirectionCount = 0;

    buffer BaseBuffer;
    buffer *CurrentBuffer = &BaseBuffer;

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
    // This value is null until this object allocates memory (in which case it sets it to the Context's allocator)
    // or the user sets it manually.
    allocator Alloc;

    string_builder() = default;
    ~string_builder() { release(); }

    // Free any memory allocated by this object and reset cursor
    void release();

    // Don't free the buffers, just reset cursor
    void reset();

    // Append a code point to the builder
    void append(char32_t codePoint);

    // Append a string to the builder
    void append(string str);

    // Append _size_ bytes from _data_ to the builder
    void append_pointer_and_size(const char *data, size_t size);

    // Execute f on every buffer where f should have the signature: void f(string)
    template <typename Lambda>
    void traverse(Lambda f) const {
        const buffer *buffer = &BaseBuffer;
        while (buffer) {
            f(string(buffer->Data, buffer->Occupied));
            buffer = buffer->Next;
        }
    }

    // Merges all buffers and returns a single string.
    string combine() const;
};

string_builder *clone(string_builder *dest, const string_builder &src);
string_builder *move(string_builder *dest, string_builder *src);

LSTD_END_NAMESPACE
