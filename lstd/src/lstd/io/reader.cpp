#include "reader.h"

#include "../memory/array.h"

LSTD_BEGIN_NAMESPACE

namespace io {

reader::reader(give_me_buffer_t giveMeBuffer) : GiveMeBuffer(giveMeBuffer) {}

void reader::request_next_buffer() {
    char status = GiveMeBuffer(this);
    if (status == eof) EOF = true;
}

pair<char, bool> reader::read_byte() {
    if (EOF) return {0, false};

    assert(Buffer.Data && "Didn't call request_next_buffer?");

    if (Buffer.Count) {
        char ch = *Buffer.Data;
        Buffer.Data += 1;
        Buffer.Count -= 1;
        return {ch, true};
    }
    return {0, false};
}

pair<array<char>, s64> reader::read_bytes(s64 n) {
    if (EOF) return {{}, n};

    assert(Buffer.Data && "Didn't call request_next_buffer?");

    if (Buffer.Count >= n) {
        return {read_bytes_unsafe(n), 0};
    } else {
        s64 diff = n - Buffer.Count;
        return {read_bytes_unsafe(Buffer.Count), diff};
    }
}

pair<array<char>, bool> reader::read_bytes_until(char delim) {
    if (EOF) return {{}, false};

    assert(Buffer.Data && "Didn't call request_next_buffer?");

    char *p = Buffer.Data;
    s64 n = Buffer.Count;
    while (n >= 4) {
        if (U32_HAS_BYTE(*(u32 *) p, delim)) break;
        p += 4;
        n -= 4;
    }

    while (n > 0) {
        if (*p == delim) return {array<char>(Buffer.Data, p - Buffer.Data), true};
        ++p, --n;
    }
    return {array<char>(Buffer.Data, p - Buffer.Data), false};
}

pair<array<char>, bool> reader::read_bytes_until(const array<char> &delims) {
    if (EOF) return {{}, false};

    assert(Buffer.Data && "Didn't call request_next_buffer?");

    char *p = Buffer.Data;
    s64 n = Buffer.Count;
    while (n > 0) {
        if (find(delims, *p) != -1) return {array<char>(Buffer.Data, p - Buffer.Data), true};
        ++p, --n;
    }
    return {array<char>(Buffer.Data, p - Buffer.Data), false};
}

pair<array<char>, bool> reader::read_bytes_while(char eats) {
    if (EOF) return {{}, false};

    assert(Buffer.Data && "Didn't call request_next_buffer?");

    char *p = Buffer.Data;
    s64 n = Buffer.Count;
    while (n >= 4) {
        if (!U32_HAS_BYTE(*(u32 *) p, eats)) break;
        p += 4;
        n -= 4;
    }

    while (n > 0) {
        if (*p != eats) return {array<char>(Buffer.Data, p - Buffer.Data), true};
        ++p, --n;
    }
    return {array<char>(Buffer.Data, p - Buffer.Data), false};
}

pair<array<char>, bool> reader::read_bytes_while(const array<char> &anyOfThese) {
    if (EOF) return {{}, false};

    assert(Buffer.Data && "Didn't call request_next_buffer?");

    char *p = Buffer.Data;
    s64 n = Buffer.Count;
    while (n > 0) {
        if (find(anyOfThese, *p) == -1) return {array<char>(Buffer.Data, p - Buffer.Data), true};
        ++p, --n;
    }
    return {array<char>(Buffer.Data, p - Buffer.Data), false};
}

array<char> reader::read_bytes_unsafe(s64 n) {
    auto result = array<char>(Buffer.Data, n);
    Buffer.Data += n;
    Buffer.Count -= n;
    return result;
}

void reader::go_backwards(s64 n) {
    Buffer.Data -= n;
    Buffer.Count += n;
}

/*    
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
                } else if (ch == '.' /*@Locale*/
/*) { 
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
    
    */
}  // namespace io

LSTD_END_NAMESPACE