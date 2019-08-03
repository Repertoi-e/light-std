#include "reader.h"

LSTD_BEGIN_NAMESPACE

namespace io {

reader::reader(request_byte_t requestByteFunction)
    : Buffer(null), Current(null), Available(0), EOF(false), LastFailed(false), SkipWhitespace(true) {
    RequestByteFunction = requestByteFunction;
}

reader *reader::read(char32_t *out) {
    if (!test_state_and_skip_ws()) {
        *out = eof;
        return this;
    }

    byte ch = peek_byte();
    if (ch == eof) {
        EOF = true;
        *out = eof;
        return this;
    }

    byte data[4]{};
    For(range(get_size_of_cp(Current))) {
        ch = bump_byte();
        if (ch == eof) {
            EOF = true;
            *out = eof;
            return this;
        }
        data[it] = ch;
    }
    *out = decode_cp(data);
    return this;
}

reader *reader::read(byte *out, size_t n) {
    if (EOF) return this;

    while (n > 0) {
        size_t size = Available;
        if (size > 0) {
            if (n < size) {
                size = n;
            }
            copy_memory(out, Current, size);
            out += size;
            n -= size;
            Current += size;
            Available -= size;
        } else {
            byte ch = request_byte_and_incr();
            if (ch == eof) {
                EOF = true;
                break;
            }

            *out++ = ch;
            --n;
        }
    }
    return this;
}

reader *io::reader::read(array<byte> *out, size_t n) {
    if (EOF) return this;
    out->reserve(n);
    return read(out->Data, n);
}

reader *io::reader::read_until(byte *out, char32_t delim) {
    if (EOF) return this;

    byte delimEnc[4]{};
    encode_cp(delimEnc, delim);

    size_t delimSize = get_size_of_cp(delim);
    size_t delimProgress = 0;

    byte ch = peek_byte();
    while (true) {
        if (ch == eof) {
            EOF = true;
            break;
        }
        if (ch == delimEnc[delimProgress]) {
            ++delimProgress;
        } else {
            delimProgress = 0;
        }
        if (delimProgress == delimSize) break;
        *out++ = ch;

        ch = next_byte();
    }
    return this;
}

reader *io::reader::read_until(array<byte> *out, char32_t delim) {
    if (EOF) return this;

    byte delimEnc[4]{};
    encode_cp(delimEnc, delim);

    size_t delimSize = get_size_of_cp(delim);
    size_t delimProgress = 0;

    byte ch = peek_byte();
    while (true) {
        if (ch == eof) {
            EOF = true;
            break;
        }
        if (ch == delimEnc[delimProgress]) {
            ++delimProgress;
        } else {
            delimProgress = 0;
        }
        if (delimProgress == delimSize) break;
        out->append(ch);

        ch = next_byte();
    }
    return this;
}

reader *io::reader::read_until(byte *out, string delims) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof) break;

        if (delims.has(cp)) break;
        encode_cp(out, cp);
        out += get_size_of_cp(cp);
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_until(array<byte> *out, string delims) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof) break;

        if (delims.has(cp)) break;
        byte cpData[4];
        encode_cp(cpData, cp);
        out->append_pointer_and_size(cpData, get_size_of_cp(cp));
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_while(byte *out, char32_t eat) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof || cp != eat) break;
        encode_cp(out, cp);
        out += get_size_of_cp(cp);
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_while(array<byte> *out, char32_t eat) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof || cp != eat) break;
        byte cpData[4];
        encode_cp(cpData, cp);
        out->append_pointer_and_size(cpData, get_size_of_cp(cp));
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_while(byte *out, string eats) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof || !eats.has(cp)) break;
        encode_cp(out, cp);
        out += get_size_of_cp(cp);
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_while(array<byte> *out, string eats) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof || !eats.has(cp)) break;
        byte cpData[4];
        encode_cp(cpData, cp);
        out->append_pointer_and_size(cpData, get_size_of_cp(cp));
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read(string *str, size_t n) {
    if (EOF) return this;

    str->reserve(n);

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (n--) {
        read(&cp);
        if (cp == eof) break;
        str->append(cp);
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_until(string *str, char32_t delim) {
    array<byte> buffer;
    read_until(&buffer, delim);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_until(string *str, string delims) {
    array<byte> buffer;
    read_until(&buffer, delims);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_while(string *str, char32_t eat) {
    array<byte> buffer;
    read_while(&buffer, eat);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_while(string *str, string eats) {
    array<byte> buffer;
    read_while(&buffer, eats);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_line(string *str) { return read_until(str, "\n"); }

reader *io::reader::ignore() {
    if (EOF) return this;

    byte ch = peek_byte();
    if (ch == eof) {
        EOF = true;
        return this;
    }

    while (true) {
        ch = bump_byte();
        if (ch == eof || ch == '\n') {
            EOF = true;
            break;
        }
    }
    return this;
}

bool reader::test_state_and_skip_ws(bool noSkipSingleTime) {
    if (EOF) return false;

    if (!noSkipSingleTime && SkipWhitespace) {
        byte ch = peek_byte();
        while (true) {
            if (ch == eof) {
                EOF = true;
                return false;
            }
            if (!is_space(ch)) {
                break;
            }
            ch = next_byte();
        }
    }
    return true;
}

pair<bool, bool> reader::parse_bool() {
    if (!test_state_and_skip_ws()) return {false, false};

    byte ch = bump_byte();
    if (ch == eof) {
        EOF = true;
        return {false, false};
    }
    if (ch == '0') return {false, true};
    if (ch == '1') return {true, true};

    // "true", "false"
    if (Available >= 3) {
        if (string(Current - 1, 4).compare_ignore_case("true") == npos) {
            For(range(3)) bump_byte();
            return {true, true};
        }
    }

    if (Available >= 4) {
        if (string(Current - 1, 5).compare_ignore_case("false") == npos) {
            For(range(4)) bump_byte();
            return {false, true};
        }
    }

    // Not a bool
    return {false, false};
}

void reader::read(bool *value) {
    if (!value) return;
    auto [parsed, success] = parse_bool();
    LastFailed = !success;
    *value = parsed;
}

#define check_eof(x)         \
    if (x == eof) {          \
        EOF = true;          \
        return {0.0, false}; \
    }

static f64 pow_10(s32 n) {
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

// @Locale This doesn't parse commas
pair<f64, bool> reader::parse_float() {
    if (!test_state_and_skip_ws()) return {0.0, false};

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

    if (ch == '0' && next == 'x' || next == 'X') {
        // Parse hex float
        bump_byte();
        ch = bump_byte();

        assert(false && "Not parsing hex floats yet");
        return {};
    } else {
        // Parse fixed or in scientific notation
        f64 integerPart = 0.0, fractionPart = 0.0;
        bool hasFraction = false, hasExponent = false;

        while (true) {
            if (ch >= '0' && ch <= '9') {
                integerPart = integerPart * 10 + (ch - '0');
            } else if (ch == '.' /*@Locale*/) {
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
            exponentPart = pow_10(exponentSign * e);
        }

        return {(negative ? -1 : 1) * (integerPart + fractionPart) * exponentPart, true};
    }
}

}  // namespace io

LSTD_END_NAMESPACE