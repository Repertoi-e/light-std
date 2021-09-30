module;

#include "../common.h"

export module lstd.string_writer;

export import lstd.writer;
export import lstd.string;
export import lstd.string_builder;

LSTD_BEGIN_NAMESPACE

export {
    struct string_builder_writer : writer {
        string_builder *Builder;

        void write(const char *data, s64 count) override { append(Builder, data, count); }
        void flush() override {}
    };
}

LSTD_END_NAMESPACE
