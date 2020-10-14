#pragma once

#include "../memory/stack_dynamic_buffer.h"
#include "../memory/string_builder.h"
#include "writer.h"

LSTD_BEGIN_NAMESPACE

struct string_builder_writer : writer {
    string_builder Builder;

    void write(const byte *data, s64 size) override {
        //
        // @TODO: Optional utf8 validation would be good here?
        //
        append_pointer_and_size(Builder, (const utf8 *) data, size);
    }
};

LSTD_END_NAMESPACE
