#pragma once

#include "../memory/memory.h"

#include "string.h"

GU_BEGIN_NAMESPACE

inline constexpr size_t STRING_BUILDER_BUFFER_SIZE = 4_KiB;

struct String_Builder {
    struct Buffer {
        char Data[STRING_BUILDER_BUFFER_SIZE];
        size_t Occupied = null;
        Buffer *Next = null;
    };

    Buffer BaseBuffer;
    Buffer *CurrentBuffer;

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    String_Builder() {
        Allocator = CONTEXT_ALLOC;

        CurrentBuffer = &BaseBuffer;
    }

    ~String_Builder();
};

namespace private_string_builder {
inline void allocate_next_buffer(String_Builder &builder) {
    String_Builder::Buffer *buffer = New<String_Builder::Buffer>(builder.Allocator);

    builder.CurrentBuffer->Next = buffer;
    builder.CurrentBuffer = buffer;
}
}  // namespace private_string_builder

inline void append_cstring_and_size(String_Builder &builder, const char *str, size_t size) {
    String_Builder::Buffer *currentBuffer = builder.CurrentBuffer;

    size_t availableSpace = STRING_BUILDER_BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        CopyMemory(currentBuffer->Data + currentBuffer->Occupied, str, size);
        currentBuffer->Occupied += size;
    } else {
        CopyMemory(currentBuffer->Data + currentBuffer->Occupied, str, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        private_string_builder::allocate_next_buffer(builder);
        append_cstring_and_size(builder, str + availableSpace, size - availableSpace);
    }
}

inline void append_cstring(String_Builder &builder, const char *str) {
    append_cstring_and_size(builder, str, utf8size(str) - 1);
}

inline void append_string(String_Builder &builder, string const &str) {
    append_cstring_and_size(builder, str.Data, str.Size);
}

inline string to_string(String_Builder &builder) {
    string result;

    String_Builder::Buffer *buffer = &builder.BaseBuffer;
    while (buffer) {
        append_cstring_and_size(result, buffer->Data, buffer->Occupied);
        buffer = buffer->Next;
    }
    return result;
}

// Don't deallocate, just move cursor to 0
inline void reset(String_Builder &builder) {
    String_Builder::Buffer *buffer = &builder.BaseBuffer;
    builder.CurrentBuffer = buffer;

    while (buffer) {
        buffer->Occupied = 0;
        buffer = buffer->Next;
    }
}

// Free the entire builder
inline void release(String_Builder &builder) {
    // We don't need to free the base buffer, it is allocated on the stack
    String_Builder::Buffer *buffer = builder.BaseBuffer.Next;
    while (buffer) {
        String_Builder::Buffer *toDelete = buffer;
        buffer = buffer->Next;
        Delete(toDelete, builder.Allocator);
    }
    builder.CurrentBuffer = &builder.BaseBuffer;
    builder.BaseBuffer.Occupied = 0;
}

inline String_Builder::~String_Builder() { release(*this); }

GU_END_NAMESPACE
