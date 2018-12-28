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
// Any subclass needs to implement just _write(string_view)_
// All other overloads are implemented around that function.
class Writer {
   public:
    virtual ~Writer() {}

    virtual Writer &write(const string_view &str) = 0;
    Writer &write(const string &str) { return write(str.get_view()); }
    Writer &write(const char *data, size_t size) { return write(string_view(data, size)); }
    Writer &write(char32_t ch) {
        char data[4];
        encode_code_point(data, ch);
        return write(string_view(data, get_size_of_code_point(data)));
    }
    Writer &write(const char *data) { return write(string_view(data)); }

    template <typename... Args>
    Writer &write_fmt(const string_view &formatString, Args &&... args) {
        fmt::internal::to_writer(*this, formatString, std::forward<Args>(args)...);
        return *this;
    }
};

// A Writer around String_Builder
struct String_Writer : Writer {
    String_Builder Builder;

    Writer &write(const string_view &str) override {
        Builder.append(str);
        return *this;
    }
};

struct Console_Writer : Writer, NonCopyable, NonMovable {
    Console_Writer();
    Writer &write(const string_view &str) override;

   private:
    // Needed for Windows to save the handle for cout
    size_t PlatformData = 0;
};

inline Console_Writer cout;
}  // namespace io

CPPU_END_NAMESPACE
