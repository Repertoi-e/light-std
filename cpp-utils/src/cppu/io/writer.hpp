#pragma once

#include "../common.hpp"

#include "../string/string_builder.hpp"
#include "../string/string_view.hpp"

CPPU_BEGIN_NAMESPACE

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
    char *Buffer = null, *Current = null;
    size_t Available = 0;

   public:
    bool AlwaysFlush = true;

    virtual ~Writer() {}

    virtual void write(const Memory_View &str) = 0;
    virtual void flush() = 0;

    void write(const string_view &str) { write(Memory_View((const byte *) str.Data, str.ByteLength)); }
    void write(const string &str) { write(str.get_view()); }
    void write(const byte *data, size_t size) { write(Memory_View(data, size)); }
    void write(const char *data, size_t size) { write((const byte *) data, size); }
    void write(const char *data) { write(string_view(data)); }

    void write_codepoint(char32_t ch) {
        char data[4];
        encode_code_point(data, ch);
        write(string_view(data, get_size_of_code_point(data)));
    }

    template <typename... Args>
    void write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
    }
};

// A Writer around String_Builder
struct String_Writer : Writer {
    String_Builder Builder;

    void write(const Memory_View &str) override {
        Builder.append_pointer_and_size((const char *) str.Data, str.ByteLength);
    }

    void flush() override {}
};

struct Console_Writer : Writer, NonCopyable, NonMovable {
    Console_Writer();
    void write(const Memory_View &str) override;
    void flush() override;

   private:
    // Needed for Windows to save the handle for cout
    size_t PlatformData = 0;
};

inline Console_Writer cout;
}  // namespace io

CPPU_END_NAMESPACE
