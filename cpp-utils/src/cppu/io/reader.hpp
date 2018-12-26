#pragma once

#include "../common.hpp"

#include "../format/parse.hpp"
#include "../string/string_view.hpp"

CPPU_BEGIN_NAMESPACE

#undef EOF

namespace io {

constexpr char eof = -1;

class Reader {
   private:
    bool _ReachedEOF = false, _ParseError = false;

   protected:
    char *Buffer = null, *Current = null;
    size_t Available = 0;

   public:
    virtual ~Reader() {}

    virtual char request_byte() = 0;

    // Use the parameter if you don't want to skip whitespace
    char32_t read_codepoint(bool noSkipWS = false) {
        if (!test_state_and_skip_ws(noSkipWS)) {
            return eof;
        }

        char byte = peek_byte();
        if (byte == eof) {
            _ReachedEOF = true;
            return eof;
        }

        char data[4] = {0};
        for (auto i : range(get_size_of_code_point(Current))) {
            byte = bump_byte();
            if (byte == eof) {
                _ReachedEOF = true;
                return eof;
            }
            data[i] = byte;
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
            _ReachedEOF = true;
        }
        return *this;
    }

    // Reads bytes until _delim_ codepoint is encountered and put them in _buffer_
    // This function automatically reserves space in the buffer
    Reader &read(Dynamic_Array<char> &buffer, size_t n) {
        buffer.reserve(n);
        return read(buffer.Data, n);
    }

    // Reads bytes until _delim_ codepoint is encountered and put them in _buffer_
    Reader &read(char *buffer, char32_t delim) {
        if (!test_state_and_skip_ws()) {
            _ReachedEOF = true;
            return *this;
        }

        char byte = peek_byte();
        if (byte == eof) {
            _ReachedEOF = true;
            return *this;
        }

        char32_t cp = read_codepoint();
        while (true) {
            if (cp == eof) {
                _ReachedEOF = true;
                break;
            }
            cp = read_codepoint(true);
            encode_code_point(buffer, cp);
            buffer += get_size_of_code_point(cp);
            if (cp == delim) break;
        }
        return *this;
    }

    // Reads bytes until _delim_ codepoint is encountered and put them in _buffer_
    // This function automatically reserves space in the buffer
    Reader &read(Dynamic_Array<char> &buffer, char32_t delim) {
        if (!test_state_and_skip_ws()) {
            _ReachedEOF = true;
            return *this;
        }

        char byte = peek_byte();
        if (byte == eof) {
            _ReachedEOF = true;
            return *this;
        }

        char *bufferData = buffer.Data;

        char32_t cp = read_codepoint();
        while (true) {
            if (cp == eof) {
                _ReachedEOF = true;
                break;
            }
            cp = read_codepoint(true);
            size_t cpSize = get_size_of_code_point(cp);

            if (!buffer.has_space_for(cpSize)) {
                uptr_t diff = bufferData - buffer.Data;
                buffer.reserve(buffer.Reserved + cpSize);
                bufferData = buffer.Data + diff;
            }
            encode_code_point(bufferData, cp);
            bufferData += get_size_of_code_point(bufferData);
            if (cp == delim) break;
        }
        return *this;
    }

    // Reads a given number of codepoints and puts them in str
    Reader &read(string &str, size_t codepoints) {
        str.reserve(codepoints * 4);
        for (char32_t cp = read_codepoint();; cp = read_codepoint(true)) {
            if (cp == eof) {
                _ReachedEOF = true;
                break;
            }
            str.append(cp);
        }
        return *this;
    }

    // Reads codepoints until _delim_ is reached and puts them in str
    // Includes _delim_ in string
    Reader &read(string &str, char32_t delim) {
        Dynamic_Array<char> buffer;
        read(buffer, delim);
        str = string(buffer.Data, buffer.Count);
        return *this;
    }

    // Reads codepoints until a newline and puts them in str
    // The newline is included in the string
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
        _ParseError = !success;
        value = parsed;
        return *this;
    }

    // Whether this reader has reached the end of file
    const bool &EOF = _ReachedEOF;

    // If the last call to any parse function has resulted in an error
    const bool &ParseError = _ParseError;

   protected:
    template <typename T>
    constexpr std::pair<T, bool> parse_int(s32 base = 0) {
        if (!test_state_and_skip_ws()) {
            return {0, false};
        }

        bool negative = false;
        char32_t cp = read_codepoint(true);
        if (cp == '+') {
            cp = read_codepoint(true);
        } else if (cp == '-') {
            negative = true;
            cp = read_codepoint(true);
        } else if (cp == eof) {
            return {0, false};
        }

        bool revert = true;
        char32_t next = read_codepoint(true);
        if (next == eof) {
            return {0, false};
        }

        if ((base == 0 || base == 16) && cp == '0' && (next == 'x' || next == 'X')) {
            base = 16;
            cp = read_codepoint(true);
            revert = false;
        }
        if (base == 0) {
            base = cp == '0' ? 8 : 10;
        }

        if (!is_alphanumeric(cp)) {
            return {0, false};
        }

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
            if (is_digit(cp)) {
                cp -= '0';
            } else if (is_alpha(cp)) {
                cp -= to_upper(cp) == cp ? 'A' - 10 : 'a' - 10;
            }

            if ((s32) cp >= base) break;
            if (value > cutoff || (value == cutoff && (s32) cp > cutlim)) {
                if constexpr (std::is_unsigned_v<T>) {
                    return {negative ? (0 - maxValue) : maxValue, false};
                } else {
                    return {(negative ? -1 : 1) * maxValue, false};
                }
            } else {
                value = value * base + cp;
            }

            if (revert) {
                cp = next;
                revert = false;
                continue;
            }

            // We are only interested in one byte, because a valid integer only contains ascii chars...
            char32_t byte = peek_byte();
            if (!is_alphanumeric(byte)) break;

            cp = read_codepoint(true);
        }
        if constexpr (std::is_unsigned_v<T>) {
            return {negative ? (0 - value) : value, true};
        } else {
            return {(negative ? -1 : 1) * value, true};
        }
    }

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
                    _ReachedEOF = true;
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

struct Console_Reader : Reader {
    Console_Reader();
    char request_byte() override;

    Console_Reader(const Console_Reader &) = delete;
    Console_Reader(Console_Reader &&) = delete;

    Console_Reader &operator=(const Console_Reader &) = delete;
    Console_Reader &operator=(Console_Reader &&) = delete;

   private:
    // Needed for Windows to save the handle for cin
    size_t PlatformData = 0;
};

inline Console_Reader cin;

}  // namespace io

CPPU_END_NAMESPACE