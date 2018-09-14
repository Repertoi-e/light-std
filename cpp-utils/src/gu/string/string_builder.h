#pragma once

#include "../memory/memory.h"

#include "string.h"

GU_BEGIN_NAMESPACE

inline constexpr size_t STRING_BUILDER_BUFFER_SIZE = 4_KiB;

struct String_Builder {
    struct Buffer {
        char Data[STRING_BUILDER_BUFFER_SIZE];
        size_t Occupied = 0;
        Buffer *Next = null;
    };

    Buffer _BaseBuffer;
    Buffer *CurrentBuffer = &_BaseBuffer;

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    ~String_Builder();
};

// Append a string to the builder
void append(String_Builder &builder, string const &str);

// Append a non encoded character to a string
void append(String_Builder &builder, char32_t codePoint);

// Append a null terminated utf-8 cstyle string.
void append_cstring(String_Builder &builder, const char *str);

// Append _size_ bytes of string contained in _data_
void append_pointer_and_size(String_Builder &builder, const char *data, size_t size);

string to_string(String_Builder &builder);

// Don't deallocate, just move cursor to 0
void reset(String_Builder &builder);

// Free the entire builder
void release(String_Builder &builder);

GU_END_NAMESPACE
