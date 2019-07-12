#pragma once

#include "../storage/array.h"
#include "../storage/string.h"

#if defined EOF
#undef EOF
#endif

LSTD_BEGIN_NAMESPACE

namespace io {

// Special constant to signify end of file.
constexpr byte eof = (byte) -1;

struct reader;

template <typename T>
bool deserialize(T *dest, reader *r);

// Provides a way to parse types and any bytes with a simple extension API.
// Holds a pointer to a _request_byte_t_. Every other function
// in this class is implemented by calling that function.
// @TODO: Tests tests tests!
struct reader {
    using request_byte_t = byte (*)(reader *r);

    // This is the only method function required for the reader to work, it is called only
    // when there are no more bytes available.
    // If you want to supply a buffer of bytes (not just one), use _Buffer_, _Current_, and _Available_.
    request_byte_t RequestByteFunction = null;

    const byte *Buffer = null, *Current = null;
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

    // Reads _n_ bytes and put them in _out_.
    // Assumes there is enough space in _out_.
    reader *read(byte *out, size_t n);

    // Reads bytes until _delim_ code point is encountered and puts them in _out_.
    // This function automatically reserves space in the buffer.
    reader *read(array<byte> *out, size_t n);

    // Reads bytes until _delim_ code point is encountered and puts them in _out_.
    // Assumes there is enough space in _out_.
    // _delim_ is not included in the string.
    reader *read_until(byte *out, char32_t delim);

    // Reads bytes until _delim_ code point is encountered and puts them in _out_.
    // This function automatically reserves space in the buffer.
    // _delim_ is not included in the string.
    reader *read_until(array<byte> *out, char32_t delim);

    // Reads bytes until any code point in _delims_ is encountered and puts them in _out_.
    // Assumes there is enough space in _out_.
    // The delim is not included in the string.
    reader *read_until(byte *out, string delims);

    // Reads bytes until any code point in _delims_ is encountered and puts them in _out_.
    // This function automatically reserves space in the buffer.
    // The delim is not included in the string.
    reader *read_until(array<byte> *out, string delims);

    // Reads bytes while anything else but the _eat_ code point is encountered and puts them in _out_.
    // Assumes there is enough space in _out_.
    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(byte *out, char32_t eat);

    // Reads bytes while anything else but the _eat_ code point is encountered and puts them in _out_.
    // This function automatically reserves space in the buffer.
    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(array<byte> *out, char32_t eat);

    // Reads bytes while anything else but any of the code points in _eats_ is encountered and puts them in _out_.
    // Assumes there is enough space in _out_.
    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(byte *out, string eats);

    // Reads bytes while anything else but any of the code points in _eats_ is encountered and puts them in _out_.
    // This function automatically reserves space in the buffer.
    // Doesn't put the terminating byte/s in the buffer.
    reader *read_while(array<byte> *out, string eats);

    // Reads _n_ code points and appends to _str_
    reader *read(string *str, size_t n);

    // Reads codepoints until _delim_ is reached and appends to _str_
    // _delim_ is not included in the string.
    reader *read_until(string *str, char32_t delim);

    // Reads codepoints until _delim_ is reached and appends to _str_
    // The delim is not included in the string.
    reader *read_until(string *str, string delims);

    // Reads codepoints while anything else but the _eat_ code point is encountered and appends to _str_
    // Doesn't include the terminating code point in the string.
    reader *read_while(string *str, char32_t eat);

    // Reads codepoints while anything else but any of the code points in _eats_ is encountered
    // and appends to _str_.
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
    void read(f32 *value);

    // Read a float
    // If the parsing fails the _LastFailed_ flag is set to true (gets reset before any parsing operation)
    void read(f64 *value);

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

        byte ch = bump_byte();
        check_eof(ch);

        bool negative = false;
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
        if constexpr (is_unsigned_v<T>) {
            maxValue = (numeric_info<T>::max)();
        } else {
            maxValue = negative ? -(numeric_info<T>::min()) : numeric_info<T>::max();
        }
        T cutoff = ABS(maxValue / base);
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

    void read_byte(byte *value, bool noSkipWhitespaceSingleTime = false) {
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

    const byte *incr() { return --Available, Current++; }
    const byte *pre_incr() { return --Available, ++Current; }

    byte peek_byte() {
        if (Available == 0) {
            return RequestByteFunction(this);
        }
        return *Current;
    }

    byte request_byte_and_incr() {
        if (RequestByteFunction(this) == eof) return eof;
        return *incr();
    }

    byte bump_byte() {
        if (Available == 0) {
            return RequestByteFunction(this);
        }
        return *incr();
    }

    byte next_byte() {
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
    r->read((byte *) dest, sizeof(T));
    return true;
}

}  // namespace io

LSTD_END_NAMESPACE

