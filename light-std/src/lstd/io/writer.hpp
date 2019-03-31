#pragma once

#include "../common.hpp"

#include "../memory/memory_buffer.hpp"
#include "../string/string_builder.hpp"

#include "../thread.hpp"

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

    // Remove all trailing bytes that equal _ch_.
    // This function does not notify any implementations
    // so for example Counter_Writer doesn't account for these "unwritten" bytes
    void remove_trailing_bytes(byte ch) {
        while (Current != Buffer && *(Current - 1) == ch) {
            --Current;
            Available++;
        }
    }

    template <typename... Args>
    void write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
    }
};

// A Writer around String_Builder
void string_writer_write(void *data, const Memory_View &writeData);

struct String_Writer : Writer {
    String_Builder Builder;

    String_Writer() { write_function = string_writer_write; }
};

void console_writer_write(void *data, const Memory_View &writeData);
void console_writer_flush(void *data);

struct Console_Writer : Writer, NonCopyable, NonMovable {
    // By default, we are thread-safe.
    // If you don't use seperate threads and aim for max
    // performance, set this to false.
    bool LockMutex = true;

    Console_Writer();
    Console_Writer(bool cerr);

   private:
    bool Err = false;
    thread::Recursive_Mutex *Mutex = null;

    friend void console_writer_write(void *, const Memory_View &);
    friend void console_writer_flush(void *);
};

inline auto cout = Console_Writer(false);
inline auto cerr = Console_Writer(true);

// Writer that does nothing but count how many bytes would have been written
void counter_writer_write(void *data, const Memory_View &writeData);

struct Counter_Writer : Writer {
    size_t Count = 0;

    Counter_Writer() { write_function = counter_writer_write; }
};

// Writer around a Memory_Buffer
template <size_t S>
void memory_buffer_write(void *data, const Memory_View &writeData);

template <size_t S>
struct Memory_Buffer_Writer : Writer {
    Memory_Buffer<S> &Buffer;

    Memory_Buffer_Writer(Memory_Buffer<S> &buffer) : Buffer(buffer) { write_function = memory_buffer_write<S>; }
};

template <size_t S>
void memory_buffer_write(void *data, const Memory_View &writeData) {
    auto *writer = (Memory_Buffer_Writer<S> *) data;
    writer->Buffer.append(writeData);
}

}  // namespace io

LSTD_END_NAMESPACE
