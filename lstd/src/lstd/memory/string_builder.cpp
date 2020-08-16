#include "string_builder.h"

LSTD_BEGIN_NAMESPACE

void string_builder::release() {
    // We don't need to free the base buffer, it is allocated on the stack
    auto *b = BaseBuffer.Next;
    while (b) {
        auto *old = b;
        b = b->Next;
        free(old);
    }

    CurrentBuffer = null;  // null means BaseBuffer
    BaseBuffer.Occupied = 0;
}

void string_builder::reset() {
    CurrentBuffer = null;  // null means BaseBuffer

    auto *b = &BaseBuffer;
    while (b) {
        b->Occupied = 0;
        b = b->Next;
    }
}

void string_builder::append(char32_t cp) {
    char encoded[4];
    encode_cp(encoded, cp);
    append_pointer_and_size(encoded, get_size_of_cp(cp));
}

void string_builder::append(const string &str) { append_pointer_and_size(str.Data, str.ByteLength); }

void string_builder::append_pointer_and_size(const char *data, s64 size) {
    buffer *currentBuffer = get_current_buffer();

    s64 availableSpace = BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        const_copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        const_copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        buffer *b = allocate(buffer, Alloc, XXX_AVOID_RECURSION);

        currentBuffer->Next = b;
        CurrentBuffer = b;

        IndirectionCount++;

        append_pointer_and_size(data + availableSpace, size - availableSpace);
    }
}

string_builder::buffer *string_builder::get_current_buffer() {
    if (CurrentBuffer == null) return &BaseBuffer;
    return CurrentBuffer;
}

string string_builder::combine() const {
    string result;
    result.reserve((IndirectionCount + 1) * BUFFER_SIZE);
    auto *b = &BaseBuffer;
    while (b) {
        result.append_pointer_and_size(b->Data, b->Occupied);
        b = b->Next;
    }
    return result;
}

string_builder *clone(string_builder *dest, const string_builder &src) {
    *dest = {};
    src.traverse([&](const string &str) { dest->append(str); });
    return dest;
}

LSTD_END_NAMESPACE