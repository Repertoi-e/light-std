#include "string_builder.h"

GU_BEGIN_NAMESPACE

void String_Builder::release() {
    // We don't need to free the base buffer, it is allocated on the stack
    String_Builder::Buffer *buffer = _BaseBuffer.Next;
    while (buffer) {
        String_Builder::Buffer *toDelete = buffer;
        buffer = buffer->Next;
        Delete(toDelete, Allocator);
    }
    CurrentBuffer = &_BaseBuffer;
    _BaseBuffer.Occupied = 0;
}

String_Builder::~String_Builder() { release(); }

void String_Builder::reset() {
    String_Builder::Buffer *buffer = &_BaseBuffer;
    CurrentBuffer = buffer;

    while (buffer) {
        buffer->Occupied = 0;
        buffer = buffer->Next;
    }
}

void String_Builder::append(const string &str) {
    append_pointer_and_size(str.Data, str.BytesUsed);
}

void String_Builder::append(char32_t codePoint) {
    char encoded[4];
    encode_code_point(encoded, codePoint);
    append_pointer_and_size(encoded, get_size_of_code_point(codePoint));
}

void String_Builder::append_cstring(const char *str) {
    append_pointer_and_size(str, cstyle_strlen(str));
}

void String_Builder::append_pointer_and_size(const char *data, size_t size) {
    String_Builder::Buffer *currentBuffer = CurrentBuffer;

    size_t availableSpace = STRING_BUILDER_BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        CopyMemory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        CopyMemory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        String_Builder::Buffer *buffer = New_And_Set_Allocator<String_Builder::Buffer>(Allocator);

        CurrentBuffer->Next = buffer;
        CurrentBuffer = buffer;

        append_pointer_and_size(data + availableSpace, size - availableSpace);
    }
}

string to_string(String_Builder &builder) {
    string result;

    String_Builder::Buffer *buffer = &builder._BaseBuffer;
    while (buffer) {
        result.append_pointer_and_size(buffer->Data, buffer->Occupied);
        buffer = buffer->Next;
    }
    return result;
}

GU_END_NAMESPACE
