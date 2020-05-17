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

    char ch = peek_byte();
    if (ch == eof) {
        EOF = true;
        *out = eof;
        return this;
    }

    char data[4]{};
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

reader *reader::read(char *out, size_t n) {
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
            char ch = request_byte_and_incr();
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

reader *io::reader::read(array<char> *out, size_t n) {
    if (EOF) return this;
    out->reserve(n);
    return read(out->Data, n);
}

reader *io::reader::read_until(char *out, char32_t delim) {
    if (EOF) return this;

    char delimEnc[4]{};
    encode_cp(delimEnc, delim);

    size_t delimSize = get_size_of_cp(delim);
    size_t delimProgress = 0;

    char ch = peek_byte();
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

reader *io::reader::read_until(array<char> *out, char32_t delim) {
    if (EOF) return this;

    char delimEnc[4]{};
    encode_cp(delimEnc, delim);

    size_t delimSize = get_size_of_cp(delim);
    size_t delimProgress = 0;

    char ch = peek_byte();
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

reader *io::reader::read_until(char *out, string delims) {
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

reader *io::reader::read_until(array<char> *out, string delims) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof) break;

        if (delims.has(cp)) break;
        char cpData[4];
        encode_cp(cpData, cp);
        out->append_pointer_and_size(cpData, get_size_of_cp(cp));
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_while(char *out, char32_t eat) {
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

reader *io::reader::read_while(array<char> *out, char32_t eat) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof || cp != eat) break;
        char cpData[4];
        encode_cp(cpData, cp);
        out->append_pointer_and_size(cpData, get_size_of_cp(cp));
    }

    SkipWhitespace = skipWS;
    return this;
}

reader *io::reader::read_while(char *out, string eats) {
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

reader *io::reader::read_while(array<char> *out, string eats) {
    if (EOF) return this;

    bool skipWS = SkipWhitespace;
    SkipWhitespace = false;

    char32_t cp;
    while (true) {
        read(&cp);
        if (cp == eof || !eats.has(cp)) break;
        char cpData[4];
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
    array<char> buffer;
    read_until(&buffer, delim);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_until(string *str, string delims) {
    array<char> buffer;
    read_until(&buffer, delims);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_while(string *str, char32_t eat) {
    array<char> buffer;
    read_while(&buffer, eat);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_while(string *str, string eats) {
    array<char> buffer;
    read_while(&buffer, eats);
    str->append_pointer_and_size(buffer.Data, buffer.Count);
    return this;
}

reader *io::reader::read_line(string *str) { return read_until(str, "\n"); }

reader *io::reader::ignore() {
    if (EOF) return this;

    char ch = peek_byte();
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
        char ch = peek_byte();
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

    char ch = bump_byte();
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

    if (ch == '0' && next == 'x' || next == 'X') {
        // Parse hex float
        bump_byte();
        ch = bump_byte();

        // @TODO: Not parsing hex floats yet, but they are the most accurate in serialization!
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

            char next = peek_byte();
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

                char next = peek_byte();
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

#define fail                             \
    {                                    \
        fill_memory(result.Data, 0, 16); \
        return {result, false};          \
    }

#undef check_eof
#define check_eof(x) \
    if (x == eof) {  \
        EOF = true;  \
        fail;        \
    }

char hex_digit_from_char(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

pair<guid, bool> io::reader::parse_guid() {
    guid result;
    if (!test_state_and_skip_ws()) return {result, false};

    bool parenthesis = false;
    bool curly = false;

    char ch = peek_byte();
    check_eof(ch);

    if (ch == '(' || ch == '{') {
        parenthesis = true;
        curly = ch == '{';
        bump_byte();
    }

    ch = bump_byte();
    check_eof(ch);

    char next = peek_byte();
    if (ch == '0' && next == 'x' || next == 'X') {
        // :GuidParseError
        if (!parenthesis) fail;
        if (!curly) fail;

        //
        // Parse following format:
        // {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
        //
        union {
            char Data[16];
            struct {
                u32 D1;
                u16 D5, D7;
                char D9, D10, D11, D12, D13, D14, D15, D16;
            };
        } u;

// :GuidParseError
#define EAT_CHAR(c)   \
    ch = bump_byte(); \
    check_eof(ch);    \
    if (ch != c) fail

// :GuidParseError
#define HANDLE_SECTION(num, size, comma)                \
    {                                            \
        auto [d, success] = parse_int<size>(16); \
        if (!success) fail;                      \
        if (comma) EAT_CHAR(',');                           \
        u.D##num = d;                            \
    }
        bump_byte(); // We already consumed 0, so when parse_int fires the first char is x which is invalid.

        HANDLE_SECTION(1, u32, true);
        HANDLE_SECTION(5, u16, true);
        HANDLE_SECTION(7, u16, true);
        EAT_CHAR('{');
        HANDLE_SECTION(9, u8, true);
        HANDLE_SECTION(10, u8, true);
        HANDLE_SECTION(11, u8, true);
        HANDLE_SECTION(12, u8, true);
        HANDLE_SECTION(13, u8, true);
        HANDLE_SECTION(14, u8, true);
        HANDLE_SECTION(15, u8, true);
        HANDLE_SECTION(16, u8, false);
        EAT_CHAR('}');
        EAT_CHAR('}');

        copy_memory(result.Data, u.Data, 16);
        return {result, true};
    } else {
        char c1 = 0, c2 = 0;
        bool seek1 = true;
        bool hyphens = false;
        bool justSkippedHyphen = false;

        u32 p = 0;
        while (true) {
            if (!hyphens) {
                if (ch == '-' && p == 4) {
                    hyphens = true;
                    ch = bump_byte();
                    check_eof(ch);
                    continue;
                }
            } else if (!justSkippedHyphen && (p == 6 || p == 8 || p == 10)) {
                if (ch != '-') {
                    // @TODO: We should report parse errors like we do format errors
                    fail;
                } else {
                    justSkippedHyphen = true;
                    ch = bump_byte();
                    check_eof(ch);
                    continue;
                }
            }

            if (!is_hex_digit(to_lower(ch))) {
                fail;  // :GuidParseError
            }

            if (seek1) {
                c1 = hex_digit_from_char(ch);
                if (c1 == -1) {
                    fail;  // :GuidParseError
                }
                seek1 = false;
            } else {
                c2 = hex_digit_from_char(ch);
                if (c2 == -1) {
                    fail;  // :GuidParseError
                }
                u8 uc = c1 * 16 + c2;

                union {
                    u8 uc;
                    char sc;
                } u;
                u.uc = uc;

                result.Data[p++] = u.sc;
                seek1 = true;

                justSkippedHyphen = false;
            }

            if (p == 16) break;

            ch = bump_byte();
            check_eof(ch);
        }

        if (parenthesis) {
            ch = bump_byte();
            check_eof(ch);
            if (curly) {
                if (ch != '}') fail;  // :GuidParseError
            } else if (ch != ')') {
                fail;  // :GuidParseError
            }
        }
        return {result, true};
    }
}

}  // namespace io

LSTD_END_NAMESPACE