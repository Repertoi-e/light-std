#pragma once

#include "../storage/array.h"
#include "../storage/string.h"

#if defined EOF
#undef EOF
#endif

LSTD_BEGIN_NAMESPACE

namespace io {

// Special constant to signify end of file.
constexpr char eof = (char) -1;

struct reader;

template <typename T>
bool deserialize(T *dest, reader *r);

// Provides a way to parse types and any bytes with a simple extension API.
// Holds a pointer to a _request_byte_t_. Every other function
// in this class is implemented by calling that function.
// @TODO: Tests tests tests!
struct reader {
    using request_byte_t = char (*)(reader *r);

    // This is the only method function required for the reader to work, it is called only
    // when there are no more bytes available.
    // If you want to supply a buffer of bytes (not just one), use _Buffer_, _Current_, and _Available_.
    request_byte_t RequestByteFunction = null;

    const char *Buffer = null, *Current = null;
    size_t Available = 0;

    // Whether this reader has reached "end of file"
    bool EOF = false;
    // If the last call to any parse function has resulted in an error
    bool LastFailed = false;

    // By default, when reading code points, integers, floats, etc. any white space is ignored.
    // If you don't want that, set this flag to false.
    bool SkipWhitespace = true;

    reader() = default;
    reader(request_byte_t requestByteFunction);

    reader *read(char32_t *out);

    // Assumes there is enough space in _out_.
    reader *read(char *out, size_t n);
    reader *read(array<char> *out, size_t n);

    // Assumes there is enough space in _out_.
    // _delim_ is not included in the string.
    reader *read_until(char *out, char32_t delim);

    // _delim_ is not included in the string.
    reader *read_until(array<char> *out, char32_t delim);

    // Assumes there is enough space in _out_.
    // The delim is not included in the string.
    reader *read_until(char *out, string delims);

    // The delim is not included in the string.
    reader *read_until(array<char> *out, string delims);

    // Assumes there is enough space in _out_.
    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(char *out, char32_t eat);

    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(array<char> *out, char32_t eat);

    // Assumes there is enough space in _out_.
    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(char *out, string eats);

    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(array<char> *out, string eats);

    // Reads _n_ code points and appends to _str_
    reader *read(string *str, size_t n);

    // _delim_ is not included in the string.
    reader *read_until(string *str, char32_t delim);

    // The delim is not included in the string.
    reader *read_until(string *str, string delims);

    // Doesn't include the terminating code point in the string.
    reader *read_while(string *str, char32_t eat);

    // Doesn't include the terminating code point in the string.
    reader *read_while(string *str, string eats);

    // Reads bytes until a newline character and puts them in _str_.
    // '\n' is not included in the string.
    reader *read_line(string *str);

    // Ignore available characters, read until a newline character and don't return the result
    reader *ignore();

    // Parse an integer from the reader
    // You can supply a custom base the integer is encoded in.
    // base 0 means this function tries to automatically determine the base by looking for a prefix:
    //     0x - hex, 0 - oct, otherwise - decimal
    //
    // If the parsing fails:
    // - the integer is outside range:               the value returned is the min/max value for that integer type
    // - the buffer doesn't contain a valid integer: the value returned is '0'
    // In both cases the _LastFailed_ flag is set to true (the flag gets reset before any parse function)
    //
    // Note:
    // If T is unsigned, but the buffer contains a '-', the value returned is underflowed
    template <typename T>
    enable_if_t<is_integral_v<T>> read(T *value, s32 base = 0) {
        auto [parsed, success] = parse_int<T>(base);
        LastFailed = !success;
        *value = parsed;
    }

    // Read a bool
    // Valid strings are: "0" "1" "true" "false" (ignoring case)
    void read(bool *value);

    // Read a float
    // If the parsing fails the _LastFailed_ flag is set to true (gets reset before any parsing operation)
    void reader::read(f32 *value) {
        if (!value) return;
        auto [parsed, success] = parse_float();
        LastFailed = !success;
        *value = (f32) parsed;
    }

    // Read a float
    // If the parsing fails the _LastFailed_ flag is set to true (gets reset before any parsing operation)
    void reader::read(f64 *value) {
        if (!value) return;
        auto [parsed, success] = parse_float();
        LastFailed = !success;
        *value = parsed;
    }

    template <typename T>
    enable_if_t<!is_arithmetic_v<T> && !is_same_v<T, string>> read(T *value) {
        LastFailed = !deserialize(value, this);
    }

   private:
    pair<bool, bool> parse_bool();
    pair<f64, bool> parse_float();

#define check_eof(x)       \
    if (x == eof) {        \
        EOF = true;        \
        return {0, false}; \
    }

    template <typename T>
    pair<T, bool> parse_int(s32 base) {
        if (!test_state_and_skip_ws()) return {0, false};

        char ch = bump_byte();
        check_eof(ch);

        bool negative = false;
        if (ch == '+') {
            ch = bump_byte();
        } else if (ch == '-') {
            negative = true;
            ch = bump_byte();
        }
        check_eof(ch);

        char next = peek_byte();
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

        // @Locale This doesn't parse commas
        T maxValue;
        if constexpr (is_unsigned_v<T>) {
            maxValue = (numeric_info<T>::max)();
        } else {
            maxValue = negative ? -(numeric_info<T>::min()) : numeric_info<T>::max();
        }
        T cutoff = abs(maxValue / base);
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
                if constexpr (is_unsigned_v<T>) {
                    return {negative ? (0 - maxValue) : maxValue, false};
                } else {
                    return {(negative ? -1 : 1) * maxValue, false};
                }
            }
            value = value * base + ch;

            if (!is_alphanumeric(peek_byte())) break;
            ch = bump_byte();
        }
        if constexpr (is_unsigned_v<T>) {
            return {negative ? (0 - value) : value, true};
        } else {
            return {(negative ? -1 : 1) * value, true};
        }
    }
#undef check_eof

    void read_byte(char *value, bool noSkipWhitespaceSingleTime = false) {
        if (!test_state_and_skip_ws(noSkipWhitespaceSingleTime)) {
            LastFailed = true;
            *value = eof;
            return;
        }
        *value = bump_byte();
        if (*value == eof) {
            LastFailed = true;
            EOF = true;
        }
    }

    bool test_state_and_skip_ws(bool noSkipSingleTime = false);

    const char *incr() { return --Available, Current++; }
    const char *pre_incr() { return --Available, ++Current; }

    char peek_byte() {
        if (Available == 0) {
            return RequestByteFunction(this);
        }
        return *Current;
    }

    char request_byte_and_incr() {
        if (RequestByteFunction(this) == eof) return eof;
        return *incr();
    }

    char bump_byte() {
        if (Available == 0) {
            return RequestByteFunction(this);
        }
        return *incr();
    }

    char next_byte() {
        if (Available <= 1) {
            if (bump_byte() == eof) return eof;
            return peek_byte();
        }
        return *pre_incr();
    }
};

// Specialize this for custom types that may not be POD or have data that isn't serialized, e.g. pointers
template <typename T>
bool deserialize(T *dest, reader *r) {
    r->read((char *) dest, sizeof(T));
    return true;
}

}  // namespace io

LSTD_END_NAMESPACE
