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
// Any subclass needs to implement just write(string_view) and flush()
// All other overloads are implemented around those functions.
class Writer {
   protected:
    char *Buffer = null, *Current = null;
    size_t Available = 0;

   public:
    bool AlwaysFlush = true;

    virtual ~Writer() {}

    virtual Writer &write(const Memory_View &str) = 0;
    virtual void flush() = 0;

    Writer &write(const string_view &str) { return write(Memory_View((const byte *) str.Data, str.ByteLength)); }

    Writer &write(const string &str) { return write(str.get_view()); }
    Writer &write(const byte *data, size_t size) { return write(Memory_View(data, size)); }
    Writer &write(const char *data, size_t size) { return write((const byte *) data, size); }
    Writer &write(const char *data) { return write(string_view(data)); }

    Writer &write_char(char32_t ch) {
        char data[4];
        encode_code_point(data, ch);
        return write(string_view(data, get_size_of_code_point(data)));
    }

    template <typename... Args>
    Writer &write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
        return *this;
    }
};

// A Writer around String_Builder
struct String_Writer : Writer {
    String_Builder Builder;

    Writer &write(const Memory_View &str) override {
        Builder.append_pointer_and_size((const char *) str.Data, str.ByteLength);
        return *this;
    }

    void flush() override {}
};

struct Console_Writer : Writer, NonCopyable, NonMovable {
    Console_Writer();
    Writer &write(const Memory_View &str) override;
    void flush() override;

   private:
    // Needed for Windows to save the handle for cout
    size_t PlatformData = 0;
};

inline Console_Writer cout;
}  // namespace io

CPPU_END_NAMESPACE
