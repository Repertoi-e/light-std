#pragma once

#include "../storage/stack_dynamic_buffer.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

void string_writer_write(writer *w, const byte *data, size_t count);
void string_writer_flush(writer *w);

struct string_writer : writer {
    string *String;

    string_writer(string *string) : writer(string_writer_write, string_writer_flush), String(string) {
        Buffer = Current = const_cast<byte *>(string->Data);
        BufferSize = Available = String->Reserved;
    }
};

inline void string_writer_write(writer *w, const byte *data, size_t count) {
    auto *sw = (string_writer *) w;

    if (count > sw->Available) {
        w->write(data, sw->Available);
        data += sw->Available;
        count -= sw->Available;

        sw->flush();
    }

    copy_memory(sw->Current, data, count);
    sw->Current += count;
    sw->Available -= count;
}

inline void string_writer_flush(writer *w) {
    auto *sw = (string_writer *) w;

    auto *string = sw->String;
    string->append_pointer_and_size(sw->Buffer, sw->BufferSize - sw->Available);
    sw->Buffer = sw->Current = const_cast<byte *>(string->Data) + string->ByteLength;

    if (!string->Reserved) {
        string->reserve(0);
    }
    sw->BufferSize = sw->Available = string->Reserved - string->ByteLength;
}

}  // namespace io

LSTD_END_NAMESPACE
