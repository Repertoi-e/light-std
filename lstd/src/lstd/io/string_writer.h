#pragma once

#include "../memory/stack_dynamic_buffer.h"
#include "../memory/string_builder.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

//
// @TODO: Optional utf8 validation would be good here?
//

void string_writer_write(writer *w, const byte *data, s64 size);

struct string_writer : writer {
    string *Str;

    string_writer(string *str) : writer(string_writer_write, writer_flush_do_nothing), Str(str) {}
};

inline void string_writer_write(writer *w, const byte *data, s64 size) {
    auto *sw = (string_writer *) w;
    sw->Str->append_pointer_and_size((const utf8 *) data, size);
}

void string_builder_writer_write(writer *w, const byte *data, s64 size);

struct string_builder_writer : writer {
    string_builder Builder;

    string_builder_writer() : writer(string_builder_writer_write, writer_flush_do_nothing) {}
};

inline void string_builder_writer_write(writer *w, const byte *data, s64 size) {
    auto *sw = (string_builder_writer *) w;
    append_pointer_and_size(sw->Builder, (const utf8 *) data, size);
}

}  // namespace io

LSTD_END_NAMESPACE
