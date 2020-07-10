#pragma once

#include "string.h"

LSTD_BEGIN_NAMESPACE

// This is good for large strings because it doesn't have to constantly reallocate
struct string_builder {
    static constexpr s64 BUFFER_SIZE = 1_KiB;

    struct buffer {
        char Data[BUFFER_SIZE]{};
        s64 Occupied = 0;
        buffer *Next = null;

        string_builder *Owner = null;
    };

    // Counts how many buffers have been dynamically allocated.
    s64 IndirectionCount = 0;

    buffer BaseBuffer;
    buffer *CurrentBuffer = null;  // null means BaseBuffer. We don't point directly to BaseBuffer because if we copy this object by value then the copy has the base buffer of the original buffer.

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
    // This value is null until this object allocates memory (in which case it sets it to the Context's allocator)
    // or the user sets it manually.
    allocator Alloc;

    string_builder() = default;
    // ~string_builder() { release(); }

    // Free any memory allocated by this object and reset cursor
    void release();

    // Don't free the buffers, just reset cursor
    void reset();

    // Append a code point to the builder
    void append(char32_t codePoint);

    // Append a string to the builder
    void append(const string &str);

    // Append _size_ bytes from _data_ to the builder
    void append_pointer_and_size(const char *data, s64 size);

    // Execute f on every buffer where f should have the signature: void f(string)
    template <typename Lambda>
    void traverse(Lambda f) const {
        const buffer *buffer = &BaseBuffer;
        while (buffer) {
            f(string(buffer->Data, buffer->Occupied));
            buffer = buffer->Next;
        }
    }

    buffer *get_current_buffer();

    // Merges all buffers in one string. The caller is responsible for freeing.
    string combine() const;
};

string_builder *clone(string_builder *dest, const string_builder &src);
// string_builder *move(string_builder *dest, string_builder *src);

LSTD_END_NAMESPACE
