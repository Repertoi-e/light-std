#pragma once

#include "../common.hpp"

#include "../format/parse.hpp"
#include "../string/string_view.hpp"

CPPU_BEGIN_NAMESPACE

#undef EOF

namespace io {

constexpr char eof = -1;

// Provides a way to parse types and any bytes with a simple extension API.
// Any subclass needs to implement just _request_byte_. Every other function
// in this class is implemented around that.
class Reader {
   protected:
    bool ReachedEOF = false, ParseError = false;
    char *Buffer = null, *Current = null;
    size_t Available = 0;

   public:
    // Whether this reader has reached the end of file
    const bool &EOF = ReachedEOF;

    // If the last call to any parse function has resulted in an error
    const bool &FailedParse = ParseError;

    virtual ~Reader() {}

    // This is the only method required to be implemented by subclasses, it is called only
    // when there are no more characters available.
    // If more than one byte is available, use _Buffer_, _Current_, and _Available_ member variables.
    virtual char request_byte() = 0;

    // Use the parameter if you don't want to skip whitespace
    char32_t read_codepoint(bool noSkipWS = false) {
        if (!test_state_and_skip_ws(noSkipWS)) {
            return eof;
        }

        char byte = peek_byte();
        if (byte == eof) {
            ReachedEOF = true;
            return eof;
        }

        char data[4] = {0};
        For(range(get_size_of_code_point(Current))) {
            byte = bump_byte();
            if (byte == eof) {
                ReachedEOF = true;
                return eof;
            }
            data[it] = byte;
        }
        return decode_code_point(data);
    }

    Reader &read(char32_t &out) {
        out = read_codepoint();
        return *this;
    }

    // Reads _n_ bytes and put them in _buffer_
    Reader &read(char *buffer, size_t n) {
        size_t read = read_bytes(buffer, n);
        if (read != n) {
            ReachedEOF = true;
        }
        return *this;
    }

    // Reads bytes until _delim_ codepoint is encountered and put them in _buffer_
    // This function automatically reserves space in the buffer
    Reader &read(Dynamic_Array<char> &buffer, size_t n) {
        if (!buffer.has_space_for(n)) buffer.expand(n);
        return read(buffer.Data, n);
    }

    // Reads bytes until _delim_ codepoint is encountered and put them in _buffer_
    // Assumes there is enough space in _buffer_
    Reader &read(char *buffer, char32_t delim) {
        if (!test_state_and_skip_ws()) {
            ReachedEOF = true;
            return *this;
        }

        char32_t cp = 0;
        for (char32_t cp = read_codepoint(); cp != eof; cp = read_codepoint(true)) {
            if (cp == delim) break;

            encode_code_point(buffer, cp);
            buffer += get_size_of_code_point(cp);
        }
        return *this;
    }

    // Reads codepoints until any of _delims_ is reached and appends them to buffer
    // This function automatically reserves space in the buffer
    // Doesn't include the delimeter in the buffer
    Reader &read(Dynamic_Array<char> &buffer, const string_view &delims) {
        if (!test_state_and_skip_ws()) {
            ReachedEOF = true;
            return *this;
        }

        char *bufferData = buffer.Data;

        char32_t cp = 0;
        for (char32_t cp = read_codepoint(); cp != eof; cp = read_codepoint(true)) {
            if (delims.has(cp)) break;

            size_t cpSize = get_size_of_code_point(cp);

            if (!buffer.has_space_for(cpSize)) {
                uptr_t diff = bufferData - buffer.Data;
                buffer.expand(cpSize);
                bufferData = buffer.Data + diff;
            }
            encode_code_point(bufferData, cp);
            bufferData += get_size_of_code_point(bufferData);
            ++buffer.Count;
        }
        return *this;
    }

    // Reads bytes until _delim_ codepoint is encountered and put them in _buffer_
    // This function automatically reserves space in the buffer
    // The encountered _delim_ is not going to be part of the buffer
    Reader &read(Dynamic_Array<char> &buffer, char32_t delim) {
        char data[4];
        encode_code_point(data, delim);
        return read(buffer, string_view(data, get_size_of_code_point(delim)));
    }

    // Reads a given number of codepoints and overwrites _str_
    Reader &read(string &str, size_t codepoints) {
        str = "";
        str.reserve(codepoints * 4);
        for (char32_t cp = read_codepoint(); cp != eof; cp = read_codepoint(true)) {
            str.append(cp);
        }
        return *this;
    }

    // Reads codepoints until _delim_ is reached and overwrites _str_
    // Doesn't include _delim_ in string
    Reader &read(string &str, char32_t delim) {
        Dynamic_Array<char> buffer;
        read(buffer, delim);
        str = string(buffer.Data, buffer.Count);
        return *this;
    }

    // Reads codepoints until any of _delims_ is reached and overwrites _str_
    // Doesn't include _delim_ in string
    Reader &read(string &str, const string_view &delims) {
        Dynamic_Array<char> buffer;
        read(buffer, delims);
        str = string(buffer.Data, buffer.Count);
        return *this;
    }

    // Reads codepoints until a newline and puts them in str
    // The newline is NOT included in the string
    Reader &read(string &str) { return read(str, U'\n'); }

    // Parse an integer from the stream
    // You can supply a custom base the integer is encoded in.
    // 0 means it is automatically determined: 0x prefix - hex, 0 prefix - oct, otherwise - decimal
    //
    // If the parsing fails:
    // - the integer is outside range:               the value returned is the min/max value for that integer type
    // - the buffer doesn't contain a valid integer: the value returned is '0'
    // In both cases the ParseError flag is set to true (gets reset before any parsing operation)
    //
    // Note:
    // If value type is unsigned, but the buffer contains a '-', the value returned is underflowed
    template <typename T>
    std::enable_if_t<std::is_integral_v<T>, Reader &> read(T &value, s32 base = 0) {
        auto [parsed, success] = parse_int<T>(base);
        ParseError = !success;
        value = parsed;
        return *this;
    }

    // Read a bool
    // Valid strings are: "0" "1" "true" "false" (ignoring case)
    Reader &read(bool &value) {
        auto [parsed, success] = parse_bool();
        ParseError = !success;
        value = parsed;
        return *this;
    }

    // Read a float
    // If the parsing fails the ParseError flag is set to true (gets reset before any parsing operation)
    Reader &read(f32 &value) {
        auto [parsed, success] = parse_float();
        ParseError = !success;
        value = (f32) parsed;
        return *this;
    }

    // Read a float
    // If the parsing fails the ParseError flag is set to true (gets reset before any parsing operation)
    Reader &read(f64 &value) {
        auto [parsed, success] = parse_float();
        ParseError = !success;
        value = parsed;
        return *this;
    }

    // Read a byte
    Reader &read(char &value, bool noSkipWs = false) {
        if (!test_state_and_skip_ws(noSkipWs)) {
            ParseError = true;
            value = eof;
            return *this;
        }
        value = bump_byte();
        if (value == eof) {
            ParseError = true;
            ReachedEOF = true;
        }
        return *this;
    }

#define check_eof(x)       \
    if (x == eof) {        \
        ReachedEOF = true; \
        return {0, false}; \
    }

   protected:
    template <typename T>
    std::pair<T, bool> parse_int(s32 base = 0) {
        if (!test_state_and_skip_ws()) {
            return {0, false};
        }

        bool negative = false;
        char ch = bump_byte();
        check_eof(ch);

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

        T maxValue;
        if constexpr (std::is_unsigned_v<T>) {
            maxValue = (std::numeric_limits<T>::max)();
        } else {
            maxValue = negative ? -(std::numeric_limits<T>::min()) : std::numeric_limits<T>::max();
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
                if constexpr (std::is_unsigned_v<T>) {
                    return {negative ? (0 - maxValue) : maxValue, false};
                } else {
                    return {(negative ? -1 : 1) * maxValue, false};
                }
            } else {
                value = value * base + ch;
            }

            if (!is_alphanumeric(peek_byte())) break;
            ch = bump_byte();
        }
        if constexpr (std::is_unsigned_v<T>) {
            return {negative ? (0 - value) : value, true};
        } else {
            return {(negative ? -1 : 1) * value, true};
        }
    }

    f64 pow10(s32 n) {
        f64 result = 1.0;
        f64 r = 10.0;
        if (n < 0) {
            n = -n;
            r = 0.1;
        }

        while (n) {
            if (n & 1) result *= r;
            r *= r;
            n >>= 1;
        }
        return result;
    }

    std::pair<f64, bool> parse_float() {
        if (!test_state_and_skip_ws()) {
            return {0.0, false};
        }

        bool negative = false;
        char ch = bump_byte();
        check_eof(ch);

        if (ch == '+') {
            ch = bump_byte();
        } else if (ch == '-') {
            negative = true;
            ch = bump_byte();
        }
        check_eof(ch);

        f64 integerPart = 0.0;
        f64 fractionPart = 0.0;
        bool hasFraction = false;
        bool hasExponent = false;

        while (true) {
            if (ch >= '0' && ch <= '9') {
                integerPart = integerPart * 10 + (ch - '0');
            } else if (ch == '.') {
                hasFraction = true;
                ch = bump_byte();
                break;
            } else if (ch == 'e') {
                hasExponent = true;
                ch = bump_byte();
                break;
            } else {
                return {(negative ? -1 : 1) * integerPart, true};
            }

            char byte = peek_byte();
            if (!is_alphanumeric(byte) && byte != '.' && byte != 'e') break;
            ch = bump_byte();
        }
        check_eof(ch);

        if (hasFraction) {
            f64 fractionExponent = 0.1;

            while (true) {
                if (ch >= '0' && ch <= '9') {
                    fractionPart += fractionExponent * (ch - '0');
                    fractionExponent *= 0.1;
                } else if (ch == 'e') {
                    hasExponent = true;
                    ch = bump_byte();
                    break;
                } else {
                    return {(negative ? -1 : 1) * (integerPart + fractionPart), true};
                }

                char byte = peek_byte();
                if (!is_digit(byte) && byte != '.' && byte != 'e') break;
                ch = bump_byte();
            }
        }
        check_eof(ch);

        f64 exponentPart = 1.0;
        if (hasExponent) {
            s32 exponentSign = 1;
            if (ch == '-') {
                exponentSign = -1;
                ch = bump_byte();
            } else if (ch == '+') {
                ch = bump_byte();
            }
            check_eof(ch);

            s32 e = 0;
            while (ch >= '0' && ch <= '9') {
                e = e * 10 + ch - '0';
                if (!is_digit(peek_byte())) break;
                ch = bump_byte();
            }
            exponentPart = pow10(exponentSign * e);
        }

        return {(negative ? -1 : 1) * (integerPart + fractionPart) * exponentPart, true};
    }

    // Second bool is success
    std::pair<bool, bool> parse_bool() {
        if (!test_state_and_skip_ws()) {
            return {false, false};
        }

        char ch = bump_byte();
        check_eof(ch);
        if (ch == '0') {
            return {false, true};
        }
        if (ch == '1') {
            return {true, true};
        }

        // "true", "false"
        if (Available >= 4) {
            if (string_view(Current - 1, 4).compare_ignore_case("true")) {
                For(range(3)) bump_byte();
                return {true, true};
            }
        }

        if (Available >= 5) {
            if (string_view(Current - 1, 5).compare_ignore_case("false")) {
                For(range(4)) bump_byte();
                return {true, true};
            }
        }

        // Not a bool
        return {false, false};
    }
#undef check_eof

    size_t read_bytes(char *buffer, size_t n) {
        size_t copyN = n;
        while (n > 0) {
            size_t size = Available;
            if (size > 0) {
                if (n < size) {
                    size = n;
                }
                copy_memory(buffer, Current, size);
                buffer += size;
                n -= size;
                Current += size;
                Available -= size;
            } else {
                char byte = request_byte_and_incr();
                if (byte == eof) break;

                *buffer++ = byte;
                --n;
            }
        }
        return copyN - n;
    }

    bool test_state_and_skip_ws(bool noSkip = false) {
        if (EOF) return false;

        if (!noSkip && SkipWhitespace) {
            for (char ch = peek_byte(); is_space(ch); ch = next_byte()) {
                if (ch == eof) {
                    ReachedEOF = true;
                    return false;
                }
            }
        }
        return true;
    }

    char *incr() { return --Available, Current++; }
    char *pre_incr() { return --Available, ++Current; }

    char peek_byte() {
        if (Available == 0) {
            return request_byte();
        }
        return *Current;
    }

    char request_byte_and_incr() {
        if (request_byte() == eof) return eof;
        return *incr();
    }

    char bump_byte() {
        if (Available == 0) {
            return request_byte_and_incr();
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

   public:
    // By default, when reading codepoints, integers, etc. any white space is disregarded.
    // If you don't want that, set this flag to false.
    bool SkipWhitespace = true;
};

struct Console_Reader : Reader, NonCopyable, NonMovable {
    Console_Reader();
    char request_byte() override;

   private:
    // Needed for Windows to save the handle for cin
    size_t PlatformData = 0;
};

inline Console_Reader cin;

}  // namespace io

CPPU_END_NAMESPACE