#pragma once

#include "../memory/memory_buffer.hpp"
#include "string.hpp"

LSTD_BEGIN_NAMESPACE

struct String_Builder {
    static constexpr size_t BUFFER_SIZE = 1_KiB;

    struct Buffer {
        byte Data[BUFFER_SIZE];
        size_t Occupied = 0;
        Buffer *Next = null;
    };

    // Counts how many extra buffers have been dynamically allocated.
    size_t IndirectionCount = 0;

    Buffer _BaseBuffer;
    Buffer *CurrentBuffer = &_BaseBuffer;

    String_Builder() {
        int a = 52;
    }

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

    // Append a string to the builder
    void append(const string_view &str);
    void append(const string &str);

    // Append a non encoded character to a string
    void append(char32_t codePoint);

    // Append a null terminated utf-8 cstyle string.
    void append_cstring(const byte *str);
    void append_cstring(const char *str) { append_cstring((const byte *) str); }

    // Append _size_ bytes of string contained in _data_
    void append_pointer_and_size(const byte *data, size_t size);
    void append_pointer_and_size(const char *data, size_t size) { append_pointer_and_size((const byte *) data, size); }

    template <size_t S>
    void append(const Memory_Buffer<S> &other) {
        append_pointer_and_size(other.Data, other.ByteLength);
    }
    void append(const Memory_View &view) { append_pointer_and_size(view.Data, view.ByteLength); }

    // Execute void f(string_view) on every buffer
    template <typename Lambda>
    inline void traverse(Lambda f) const {
        const String_Builder::Buffer *buffer = &_BaseBuffer;
        while (buffer) {
            f(string_view(buffer->Data, buffer->Occupied));
            buffer = buffer->Next;
        }
    }

    // Merges all buffers and returns a single string.
    inline string combine() const {
        string result;
        result.reserve((IndirectionCount + 1) * BUFFER_SIZE);

        const String_Builder::Buffer *buffer = &_BaseBuffer;
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

    ~String_Builder();
};

LSTD_END_NAMESPACE
