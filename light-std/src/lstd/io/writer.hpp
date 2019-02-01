#pragma once

#include "../common.hpp"

#include "../string/string_builder.hpp"
#include "../string/string_view.hpp"

LSTD_BEGIN_NAMESPACE

struct string;
namespace io {
struct Writer;
}

namespace fmt::internal {
template <typename... Args>
void to_writer(io::Writer &writer, const string_view &formatString, Args &&... args);
}

namespace io {

inline void writer_flush_do_nothing(void *data) {}

// Provides a simple API to write stuff.
// Any subclass needs to provide pointers for the functions write(const Memory_View &) and flush()
// All other overloads are implemented around those functions.
// Note: A default flush function which does nothing is provided by default.
struct Writer {
    byte *Buffer = null, *Current = null;
    size_t Available = 0;

    using write_type = void (*)(void *data, const Memory_View &writeData);
    using flush_type = void (*)(void *data);

    write_type write_function = null;
    flush_type flush_function = writer_flush_do_nothing;

    void flush() { flush_function(this); }
    void write(const Memory_View &data) { write_function(this, data); }
    void write(const string_view &str) { write_function(this, Memory_View(str.Data, str.ByteLength)); }
    void write(const string &str) { write_function(this, str.get_view()); }
    void write(const byte *data, size_t size) { write_function(this, Memory_View(data, size)); }
    void write(const byte *data) { write_function(this, Memory_View(data, cstring_strlen(data))); }
    void write(const char *data, size_t size) { write_function(this, Memory_View(data, size)); }
    void write(const char *data) { write_function(this, Memory_View(data, cstring_strlen((const byte *) data))); }

    void write_codepoint(char32_t ch) {
        byte data[4];
        encode_code_point(data, ch);
        write_function(this, Memory_View(data, get_size_of_code_point(data)));
    }

    template <typename... Args>
    void write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
    }
};

void string_writer_write(void *data, const Memory_View &writeData);

// A Writer around String_Builder
struct String_Writer : Writer {
    String_Builder Builder;

    String_Writer() { write_function = string_writer_write; }
};

void console_writer_write(void *data, const Memory_View &writeData);
void console_writer_flush(void *data);

struct Console_Writer : Writer, NonCopyable, NonMovable {
    Console_Writer();
};

inline Console_Writer cout;
}  // namespace io

LSTD_END_NAMESPACE
