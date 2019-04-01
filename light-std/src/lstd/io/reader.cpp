#include "reader.hpp"

LSTD_BEGIN_NAMESPACE

namespace io {
char32_t reader::read_codepoint(bool noSkipWS) {
    if (!test_state_and_skip_ws(noSkipWS)) {
        return (char32_t) eof;
    }

    byte ch = peek_byte();
    if (ch == eof) {
        EOF = true;
        return (char32_t) eof;
    }

    byte data[4] = {0};
    For(range(get_size_of_code_point(Current))) {
        ch = bump_byte();
        if (ch == eof) {
            EOF = true;
            return (char32_t) eof;
        }
        data[it] = ch;
    }
    return decode_code_point(data);
}

void reader::read(char32_t &out) { out = read_codepoint(); }

void reader::read(byte *buffer, size_t n) {
    size_t read = read_bytes(buffer, n);
    if (read != n) {
        EOF = true;
    }
}

void reader::read(dynamic_array<byte> &buffer, size_t n) {
    if (!buffer.has_space_for(n)) buffer.grow(n);
    return read(buffer.Data.get(), n);
}

void reader::read(byte *buffer, char32_t delim) {
    if (!test_state_and_skip_ws()) {
        EOF = true;
        return;
    }

    char32_t cp = 0;
    for (char32_t cp = read_codepoint(); cp != (char32_t) eof; cp = read_codepoint(true)) {
        if (cp == delim) break;

        encode_code_point(buffer, cp);
        buffer += get_size_of_code_point(cp);
    }
}

void reader::read(dynamic_array<byte> &buffer, const string_view &delims) {
    if (!test_state_and_skip_ws(true)) {
        EOF = true;
        return;
    }

    byte *bufferData = buffer.Data.get();

    char32_t cp = 0;
    for (char32_t cp = read_codepoint(true); cp != (char32_t) eof; cp = read_codepoint(true)) {
        if (delims.has(cp)) break;

        size_t cpSize = get_size_of_code_point(cp);

        if (!buffer.has_space_for(cpSize)) {
            uptr_t diff = bufferData - buffer.Data.get();
            buffer.grow(cpSize);
            bufferData = buffer.Data.get() + diff;
        }
        encode_code_point(bufferData, cp);
        bufferData += get_size_of_code_point(bufferData);
        ++buffer.Count;
    }
}

void reader::read(dynamic_array<byte> &buffer, char32_t delim) {
    byte data[4];
    encode_code_point(data, delim);
    return read(buffer, string_view(data, get_size_of_code_point(delim)));
}

void reader::read(string &str, size_t codepoints) {
    str = "";
    str.Data.reserve(codepoints * 4);
    for (char32_t cp = read_codepoint(); cp != (char32_t) eof; cp = read_codepoint(true)) {
        str.append(cp);
    }
}

void reader::read(string &str, char32_t delim) {
    dynamic_array<byte> buffer;
    read(buffer, delim);
    str = string(buffer.Data.get(), buffer.Count);
}

void reader::read(string &str, const string_view &delims) {
    dynamic_array<byte> buffer;
    read(buffer, delims);
    str = string(buffer.Data.get(), buffer.Count);
}

void reader::read(string &str) { return read(str, U'\n'); }

void reader::read() {
    string ignored;
    read(ignored);
}

static f64 pow10(s32 n) {
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

#define check_eof(x)         \
    if (x == eof) {          \
        EOF = true;          \
        return {0.0, false}; \
    }

std::pair<f64, bool> reader::parse_float() {
    if (!test_state_and_skip_ws()) {
        return {0.0, false};
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
            return {(negative ? -1 : 1) * integerPart, false};
        }

        byte next = peek_byte();
        if (!is_alphanumeric(next) && next != '.' && next != 'e') break;
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

            byte next = peek_byte();
            if (!is_digit(next) && next != '.' && next != 'e') break;
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
std::pair<bool, bool> reader::parse_bool() {
    if (!test_state_and_skip_ws()) {
        return {false, false};
    }

    byte ch = bump_byte();
    check_eof(ch);
    if (ch == '0') {
        return {false, true};
    }
    if (ch == '1') {
        return {true, true};
    }

    // "true", "false"
    if (Available >= 3) {
        if (string_view(Current - 1, 4).compare_ignore_case("true") == 0) {
            For(range(3)) bump_byte();
            return {true, true};
        }
    }

    if (Available >= 4) {
        if (string_view(Current - 1, 5).compare_ignore_case("false") == 0) {
            For(range(4)) bump_byte();
            return {false, true};
        }
    }

    // Not a bool
    return {false, false};
}
#undef check_eof

size_t reader::read_bytes(byte *buffer, size_t n) {
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
            byte ch = request_byte_and_incr();
            if (ch == eof) break;

            *buffer++ = ch;
            --n;
        }
    }
    return copyN - n;
}

bool reader::test_state_and_skip_ws(bool noSkip) {
    if (EOF) return false;

    if (!noSkip && SkipWhitespace) {
        for (byte ch = peek_byte();; ch = next_byte()) {
            if (ch == eof) {
                EOF = true;
                return false;
            }
            if (!is_space(ch)) {
                break;
            }
        }
    }
    return true;
}

const byte *reader::incr() { return --Available, Current++; }
const byte *reader::pre_incr() { return --Available, ++Current; }

byte reader::peek_byte() {
    if (Available == 0) {
        return request_byte_function(this);
    }
    return *Current;
}

byte reader::request_byte_and_incr() {
    if (request_byte_function(this) == eof) return eof;
    return *incr();
}

byte reader::bump_byte() {
    if (Available == 0) {
        return request_byte_and_incr();
    }
    return *incr();
}

byte reader::next_byte() {
    if (Available <= 1) {
        if (bump_byte() == eof) return eof;
        return peek_byte();
    }
    return *pre_incr();
}

string_reader::string_reader(const string_view &view) : View(view) {
    request_byte_function = string_reader_request_byte;
}
}  // namespace io

LSTD_END_NAMESPACE