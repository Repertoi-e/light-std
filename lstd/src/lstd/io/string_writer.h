#pragma once

#include "../memory/stack_dynamic_buffer.h"
#include "../memory/string_builder.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

namespace io {

void string_writer_write(writer *w, const char *data, s64 count);

struct string_writer : writer {
    string *Str;

    string_writer(string *str) : writer(string_writer_write, writer_flush_do_nothing), Str(str) {}
};

inline void string_writer_write(writer *w, const char *data, s64 count) {
    auto *sw = (string_writer *) w;
    sw->Str->append_pointer_and_size(data, count);
}

void string_builder_writer_write(writer *w, const char *data, s64 count);

struct string_builder_writer : writer {
    string_builder Builder;

    string_builder_writer() : writer(string_builder_writer_write, writer_flush_do_nothing) {}
};

inline void string_builder_writer_write(writer *w, const char *data, s64 count) {
    auto *sw = (string_builder_writer *) w;
    sw->Builder.append_pointer_and_size(data, count);
}

}  // namespace io

LSTD_END_NAMESPACE
