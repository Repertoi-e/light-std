#include "string_builder.hpp"

LSTD_BEGIN_NAMESPACE

void String_Builder::release() {
    // We don't need to free the base buffer, it is allocated on the stack
    String_Builder::Buffer *buffer = _BaseBuffer.Next;
    while (buffer) {
        String_Builder::Buffer *toDelete = buffer;
        buffer = buffer->Next;
        delete toDelete;
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

void String_Builder::append(const string_view &str) { append_pointer_and_size(str.Data, str.ByteLength); }

void String_Builder::append(const string &str) { append_pointer_and_size(str.Data, str.ByteLength); }

void String_Builder::append(char32_t codePoint) {
    byte encoded[4];
    encode_code_point(encoded, codePoint);
    append_pointer_and_size(encoded, get_size_of_code_point(codePoint));
}

void String_Builder::append_cstring(const byte *str) { append_pointer_and_size(str, cstring_strlen(str)); }

void String_Builder::append_pointer_and_size(const byte *data, size_t size) {
    String_Builder::Buffer *currentBuffer = CurrentBuffer;

    size_t availableSpace = BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        String_Builder::Buffer *buffer = new (&Allocator, ensure_allocator) String_Builder::Buffer;

        CurrentBuffer->Next = buffer;
        CurrentBuffer = buffer;

        IndirectionCount++;

        append_pointer_and_size(data + availableSpace, size - availableSpace);
    }
}

LSTD_END_NAMESPACE
