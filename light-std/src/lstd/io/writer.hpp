#pragma once

#include "../common.hpp"

#include "../string/string_builder.hpp"
#include "../string/string_view.hpp"

LSTD_BEGIN_NAMESPACE

struct string;
namespace io {
class Writer;
}

namespace fmt::internal {
template <typename... Args>
void to_writer(io::Writer &writer, const string_view &formatString, Args &&... args);
}

namespace io {

// Provides a simple API to write stuff.
// Any subclass needs to implement just write(Memory_View) and flush()
// All other overloads are implemented around those functions.
class Writer {
   protected:
    byte *Buffer = null, *Current = null;
    size_t Available = 0;

   public:
    virtual ~Writer() {}

    virtual void write(const Memory_View &str) = 0;
    virtual void flush() = 0;

    void write(const string_view &str) { write(Memory_View(str.Data, str.ByteLength)); }
    void write(const string &str) { write(str.get_view()); }
    void write(const byte *data, size_t size) { write(Memory_View(data, size)); }
    void write(const byte *data) { write(Memory_View(data, cstring_strlen(data))); }
    void write(const char *data, size_t size) { write(Memory_View(data, size)); }
    void write(const char *data) { write(Memory_View(data, cstring_strlen((const byte *) data))); }

    void write_codepoint(char32_t ch) {
        byte data[4];
        encode_code_point(data, ch);
        write(Memory_View(data, get_size_of_code_point(data)));
    }

    template <typename... Args>
    void write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
    }
};

// A Writer around String_Builder
struct String_Writer : Writer {
    String_Builder Builder;

    void write(const Memory_View &str) override { Builder.append_pointer_and_size(str.Data, str.ByteLength); }
    void flush() override {}
};

struct Console_Writer : Writer, NonCopyable, NonMovable {
    Console_Writer();
    void write(const Memory_View &str) override;
    void flush() override;
};

inline Console_Writer cout;
}  // namespace io

LSTD_END_NAMESPACE
