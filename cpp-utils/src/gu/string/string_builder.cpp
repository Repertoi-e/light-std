#include "string_builder.h"

GU_BEGIN_NAMESPACE

void release(String_Builder &builder) {
    // We don't need to free the base buffer, it is allocated on the stack
    String_Builder::Buffer *buffer = builder._BaseBuffer.Next;
    while (buffer) {
        String_Builder::Buffer *toDelete = buffer;
        buffer = buffer->Next;
        Delete(toDelete, builder.Allocator);
    }
    builder.CurrentBuffer = &builder._BaseBuffer;
    builder._BaseBuffer.Occupied = 0;
}

String_Builder::~String_Builder() { release(*this); }

void reset(String_Builder &builder) {
    String_Builder::Buffer *buffer = &builder._BaseBuffer;
    builder.CurrentBuffer = buffer;

    while (buffer) {
        buffer->Occupied = 0;
        buffer = buffer->Next;
    }
}

void append(String_Builder &builder, string const &str) { append_pointer_and_size(builder, str.Data, str.BytesUsed); }

void append(String_Builder &builder, char32_t codePoint) {
    char encoded[4];
    encode_code_point(encoded, codePoint);
    append_pointer_and_size(builder, encoded, get_size_of_code_point(codePoint));
}

void append_cstring(String_Builder &builder, const char *str) {
    append_pointer_and_size(builder, str, cstyle_strlen(str));
}

void append_pointer_and_size(String_Builder &builder, const char *data, size_t size) {
    String_Builder::Buffer *currentBuffer = builder.CurrentBuffer;

    size_t availableSpace = STRING_BUILDER_BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        CopyMemory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        CopyMemory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        String_Builder::Buffer *buffer = New<String_Builder::Buffer>(builder.Allocator);

        builder.CurrentBuffer->Next = buffer;
        builder.CurrentBuffer = buffer;

        append_pointer_and_size(builder, data + availableSpace, size - availableSpace);
    }
}

string to_string(String_Builder &builder) {
    string result;

    String_Builder::Buffer *buffer = &builder._BaseBuffer;
    while (buffer) {
        append_pointer_and_size(result, buffer->Data, buffer->Occupied);
        buffer = buffer->Next;
    }
    return result;
}

GU_END_NAMESPACE
