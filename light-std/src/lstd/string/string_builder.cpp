#include "string_builder.hpp"

LSTD_BEGIN_NAMESPACE

void string_builder::release() {
    // We don't need to free the base buffer, it is allocated on the stack
    buffer *b = BaseBuffer.Next;
    while (b) {
        buffer *old = b;
        b = b->Next;
        delete old;
    }
    CurrentBuffer = &BaseBuffer;
    BaseBuffer.Occupied = 0;
}

string_builder::~string_builder() { release(); }

void string_builder::reset() {
    buffer *buffer = &BaseBuffer;
    CurrentBuffer = buffer;

    while (buffer) {
        buffer->Occupied = 0;
        buffer = buffer->Next;
    }
}

void string_builder::append(char32_t codePoint) {
    byte encoded[4];
    encode_code_point(encoded, codePoint);
    append_pointer_and_size(encoded, get_size_of_code_point(codePoint));
}

void string_builder::append(const memory_view &view) { append_pointer_and_size(view.Data, view.ByteLength); }

void string_builder::append_pointer_and_size(const byte *data, size_t size) {
    buffer *currentBuffer = CurrentBuffer;

    size_t availableSpace = BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        buffer *b = new (&Allocator, ensure_allocator) buffer;

        CurrentBuffer->Next = b;
        CurrentBuffer = b;

        IndirectionCount++;

        append_pointer_and_size(data + availableSpace, size - availableSpace);
    }
}

LSTD_END_NAMESPACE
