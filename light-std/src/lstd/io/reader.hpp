#pragma once

#include "../common.hpp"

#include "../format/parse.hpp"
#include "../string/string_view.hpp"

#include "../memory/dynamic_array.hpp"

#include "../thread.hpp"

#include "../intrinsics/intrin.hpp"

LSTD_BEGIN_NAMESPACE

#undef EOF

namespace io {

constexpr byte eof = -1;

struct reader;

template <typename T, typename Enable = void>
struct deserializer {
    // Return false on failure, true if the read completed successfully
    bool read(T &, reader &) {
        assert(false);
        // static_assert(false, "Deserializer<T> not specialized");
        return false;
    }
};

// Provides a way to parse types and any bytes with a simple extension API.
// Holds a pointer to _request_byte_. Every other function
// in this class is implemented around that function.

struct reader {
    using request_byte_type = byte (*)(void *data);

    // This is the only method function required for the Reader to work, it is called only
    // when there are no more bytes available.
    // If you want to supply more bytes, use _Buffer_, _Current_, and _Available_.
    request_byte_type request_byte_function = null;

    // Whether this reader has reached the end of file
    bool EOF = false;
    // If the last call to any parse function has resulted in an error
    bool FailedParse = false;
    const byte *Buffer = null, *Current = null;
    size_t Available = 0;

    // By default, when reading codepoints, integers, etc. any white space is disregarded.
    // If you don't want that, set this flag to false.
    bool SkipWhitespace = true;

    // Use the parameter if you don't want to skip whitespace
    char32_t read_codepoint(bool noSkipWS = false);

    void read(char32_t &out);

    // Reads _n_ bytes and put them in _buffer_
    void read(byte *buffer, size_t n);

    // Reads bytes until _delim_ code point is encountered and put them in _buffer_
    // This function automatically reserves space in the buffer
    void read(dynamic_array<byte> &buffer, size_t n);

    // Reads bytes until _delim_ code point is encountered and put them in _buffer_
    // Assumes there is enough space in _buffer_
    void read(byte *buffer, char32_t delim);

    // Reads codepoints until any of _delims_ is reached and appends them to buffer
    // This function automatically reserves space in the buffer
    // Doesn't include the delimeter in the buffer
    void read(dynamic_array<byte> &buffer, const string_view &delims);

    // Reads bytes until _delim_ code point is encountered and put them in _buffer_
    // This function automatically reserves space in the buffer
    // The encountered _delim_ is not going to be part of the buffer
    void read(dynamic_array<byte> &buffer, char32_t delim);

    // Reads a given number of codepoints and overwrites _str_
    void read(string &str, size_t codepoints);

    // Reads codepoints until _delim_ is reached and overwrites _str_
    // Doesn't include _delim_ in string
    void read(string &str, char32_t delim);

    // Reads codepoints until any of _delims_ is reached and overwrites _str_
    // Doesn't include _delim_ in string
    void read(string &str, const string_view &delims);

    // Reads codepoints until a newline and puts them in str
    // The newline is NOT included in the string
    void read(string &str);

    // Read until newline and ignore it
    void read();

    // Parse an integer from the stream
    // You can supply a custom base the integer is encoded in.
    // 0 means it is automatically determined: 0x prefix - hex, 0 prefix - oct, otherwise - decimal
    //
    // If the parsing fails:
    // - the integer is outside range:               the value returned is the min/max value for that integer type
    // - the buffer doesn't contain a valid integer: the value returned is '0'
    // In both cases the FailedParse flag is set to true (gets reset before any parsing operation)
    //
    // Note:
    // If value type is unsigned, but the buffer contains a '-', the value returned is underflowed
    template <typename T>
    std::enable_if_t<std::is_integral_v<T>> read(T &value, s32 base = 0) {
        auto [parsed, success] = parse_int<T>(base);
        FailedParse = !success;
        value = parsed;
    }

    // Read a bool
    // Valid strings are: "0" "1" "true" "false" (ignoring case)
    void read(bool &value) {
        auto [parsed, success] = parse_bool();
        FailedParse = !success;
        value = parsed;
    }

    // Read a float
    // If the parsing fails the FailedParse flag is set to true (gets reset before any parsing operation)
    void read(f32 &value) {
        auto [parsed, success] = parse_float();
        FailedParse = !success;
        value = (f32) parsed;
    }

    // Read a float
    // If the parsing fails the FailedParse flag is set to true (gets reset before any parsing operation)
    void read(f64 &value) {
        auto [parsed, success] = parse_float();
        FailedParse = !success;
        value = parsed;
    }

    // Read a byte
    void read(byte &value, bool noSkipWs = false) {
        if (!test_state_and_skip_ws(noSkipWs)) {
            FailedParse = true;
            value = eof;
            return;
        }
        value = bump_byte();
        if (value == eof) {
            FailedParse = true;
            EOF = true;
        }
    }

    template <typename T>
    std::enable_if_t<!std::is_arithmetic_v<T> && !std::is_same_v<T, string>> read(T &value) {
        deserializer<T> deserializer;
        FailedParse = !deserializer.read(value, *this);
    }

   protected:
#define check_eof(x)       \
    if (x == eof) {        \
        EOF = true;        \
        return {0, false}; \
    }

    template <typename T>
    std::pair<T, bool> parse_int(s32 base) {
        if (!test_state_and_skip_ws()) {
            return {0, false};
        }

        bool negative = false;
        byte ch = bump_byte();
        check_eof(ch);

        if (ch == '+') {
            ch = bump_byte();
        } else if (ch == '-') {
            negative = true;
            ch = bump_byte();
        }
        check_eof(ch);

        byte next = peek_byte();
        check_eof(next);

        if ((base == 0 || base == 16) && ch == '0' && (next == 'x' || next == 'X')) {
            base = 16;
            bump_byte();
            ch = bump_byte();
        }
        if (base == 0) {
            base = ch == '0' ? 8 : 10;
        }
        check_eof(ch);

        T maxValue;
        if constexpr (std::is_unsigned_v<T>) {
            maxValue = (std::numeric_limits<T>::max)();
        } else {
            maxValue = negative ? -(std::numeric_limits<T>::min()) : std::numeric_limits<T>::max();
        }
        T cutoff = absolute_value(maxValue / base);
        s32 cutlim = maxValue % (T) base;

        T value = 0;
        while (true) {
            if (is_digit(ch)) {
                ch -= '0';
            } else if (is_alpha(ch)) {
                ch -= to_upper(ch) == ch ? 'A' - 10 : 'a' - 10;
            }

            if ((s32) ch >= base) break;
            if (value > cutoff || (value == cutoff && (s32) ch > cutlim)) {
                if constexpr (std::is_unsigned_v<T>) {
                    return {negative ? (0 - maxValue) : maxValue, false};
                } else {
                    return {(negative ? -1 : 1) * maxValue, false};
                }
            }
            value = value * base + ch;

            if (!is_alphanumeric(peek_byte())) break;
            ch = bump_byte();
        }
        if constexpr (std::is_unsigned_v<T>) {
            return {negative ? (0 - value) : value, true};
        } else {
            return {(negative ? -1 : 1) * value, true};
        }
    }
#undef check_eof

    std::pair<f64, bool> parse_float();

    // Second bool is success
    std::pair<bool, bool> parse_bool();

    size_t read_bytes(byte *buffer, size_t n);
    bool test_state_and_skip_ws(bool noSkip = false);

    const byte *incr();
    const byte *pre_incr();

    byte peek_byte();
    byte request_byte_and_incr();
    byte bump_byte();
    byte next_byte();
};

template <typename T>
struct deserializer<T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, string>>> {
    bool read(T &value, reader &reader) {
        reader.read(value);
        return !reader.FailedParse;
    }
};

struct string_reader : reader {
    string_view View;
    bool Exhausted = false;

    explicit string_reader(const string_view &view);
};

inline byte string_reader_request_byte(void *data) {
    auto *reader = (string_reader *) data;

    if (reader->Exhausted) return eof;
    reader->Buffer = reader->View.Data;
    reader->Current = reader->Buffer;
    reader->Available = reader->View.ByteLength;
    reader->Exhausted = true;
    return *reader->Current;
}

// Defined in *platform*.cpp
byte console_reader_request_byte(void *data);

struct console_reader : reader, non_copyable, non_movable {
    // By default, we are thread-safe.
    // If you don't use seperate threads and aim for max
    // performance, set this to false.
    bool LockMutex = true;

    console_reader();

   private:
    thread::mutex *_Mutex = null;

    friend byte console_reader_request_byte(void *);
};

inline console_reader cin;

}  // namespace io

LSTD_END_NAMESPACE