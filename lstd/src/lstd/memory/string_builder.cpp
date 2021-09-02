#include "string_builder.h"

LSTD_BEGIN_NAMESPACE

void free(string_builder &builder) {
    // We don't need to free the base buffer, it is allocated on the stack
    auto *b = builder.BaseBuffer.Next;
    while (b) {
        auto *old = b;
        b         = b->Next;
        free(old);
    }

    builder.CurrentBuffer       = null; // null means BaseBuffer
    builder.BaseBuffer.Occupied = 0;
}

void reset(string_builder &builder) {
    builder.CurrentBuffer = null; // null means BaseBuffer

    auto *b = &builder.BaseBuffer;
    while (b) {
        b->Occupied = 0;
        b           = b->Next;
    }
}

void string_append(string_builder &builder, utf32 cp) {
    utf8 encoded[4];
    encode_cp(encoded, cp);
    string_append(builder, encoded, get_size_of_cp(cp));
}

void string_append(string_builder &builder, const string &str) { string_append(builder, str.Data, str.Count); }

void string_append(string_builder &builder, const utf8 *data, s64 size) {
    auto *currentBuffer = string_builder_get_current_buffer(builder);

    s64 availableSpace = builder.BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        if (!builder.Alloc) builder.Alloc = Context.Alloc;
        auto *b = allocate<string_builder::buffer>({.Alloc = builder.Alloc});

        currentBuffer->Next   = b;
        builder.CurrentBuffer = b;

        builder.IndirectionCount++;

        string_append(builder, data + availableSpace, size - availableSpace);
    }
}

string_builder::buffer *string_builder_get_current_buffer(string_builder &builder) {
    if (builder.CurrentBuffer == null) return &builder.BaseBuffer;
    return builder.CurrentBuffer;
}

string string_builder_combine(const string_builder &builder) {
    string result;
    string_reserve(result, (builder.IndirectionCount + 1) * builder.BUFFER_SIZE);
    auto *b = &builder.BaseBuffer;
    while (b) {
        string_append(result, b->Data, b->Occupied);
        b = b->Next;
    }
    return result;
}

// @API Remove this, iterators? Literally anything else..
void string_builder_traverse(const string_builder &builder, const delegate<void(const string &)> &func) {
    auto *buffer = &builder.BaseBuffer;
    while (buffer) {
        func(string(buffer->Data, buffer->Occupied));
        buffer = buffer->Next;
    }
}

string_builder *clone(string_builder *dest, const string_builder &src) {
    *dest         = {};
    auto appender = [&](const string &str) { string_append(*dest, str); };
    string_builder_traverse(src, &appender);
    return dest;
}

LSTD_END_NAMESPACE
