#pragma once

#include "../storage/stack_dynamic_buffer.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

void string_writer_write(writer *w, const byte *data, size_t count);

struct string_writer : writer {
    string *Str;

    string_writer(string *str) : writer(string_writer_write, writer_flush_do_nothing), Str(str) {}
};

inline void string_writer_write(writer *w, const byte *data, size_t count) {
    auto *sw = (string_writer *) w;
    sw->Str->append_pointer_and_size(data, count);
}

}  // namespace io

LSTD_END_NAMESPACE
