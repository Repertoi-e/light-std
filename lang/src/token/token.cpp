#include "../../lang.h"
#include "../../snipffi.h"

#include "lstd/fmt.h"
#include "lstd/parse.h"
#include "lstd/string.h"

#include <cstdio>

inline bool ascii_is_identifier_start(char x) { return ascii_is_alpha(x) || x == '_'; }
inline bool ascii_is_identifier_cont(char x) { return ascii_is_alphanumeric(x) || x == '_' || x == '-'; }

inline bool is_ident_start_cp(code_point cp) { return cp == '_' || unicode_is_letter(cp); }
inline bool is_ident_continue_cp(code_point cp) { 
    unicode_general_category cat = unicode_get_general_category(cp);
    return unicode_is_letter(cat) || unicode_is_number(cat) || unicode_is_mark(cat) || cp == '_' || cp == '-'; 
}

#include "token_generated.inc"

inline bool ascii_is_digit_based(char c, int base)
{
    if (base <= 10)
        return c >= '0' && c < '0' + base;
    return (c >= '0' && c <= '9') || (c >= 'a' && c < 'a' + base - 10) || (c >= 'A' && c < 'A' + base - 10);
}

inline s32 ascii_digit_value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

inline code_point unicode_parse_escape(const char **s) {
    char c = **s;
    switch (c) {
        case 'n': return '\n';
        case 'a': return '\a';
        case 'e': return '\e';
        case 'r': return '\r';
        case 't': return '\t';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'v': return '\v';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case '?': return '?';
    }

    if (c == 'x' || c == 'u' || c == 'U' || ascii_is_digit_based(c, 8))
    {
        s32 base = ascii_is_digit_based(c, 8) ? 8 : 16;

        // Determine the number of digits to expect based on the escape sequence
        s32 digits = (base == 8) ? 3 : (c == 'x' ? 2 : (c == 'u' ? 4 : 8));
        code_point cp = 0;

        const char *p = *s;
        while ((p - *s) < digits && ascii_is_digit_based(*p, base))
        {
            cp = cp * base + ascii_digit_value(*p);
            p++;
        }
        *s = p;
        return cp;
    }

    // Fallback: not an escape sequence we handle specially, treat literally
    (*s)++;
    WARN_ANNOTATED("Unknown escape sequence", *s - 1, *s, "Unknown escape sequence, treating literally");
    return c;
}

token tokenizer_next_string_literal(tokenizer ref tz) {
    const char *start_offset = tz.Current;

    if (*start_offset == '\0') {
        return (token){TOKEN_INVALID, 0};
    }

    atom *a = atom_new();
    const char *s = tz.Current;

    if (*s != '"' || *s != '\'') return (token){TOKEN_INVALID, 0};
    char quote = *s;

    s++; // skip opening quote
    const char *chunk_start = s;

    while (*s) {
        if (*s == quote) {
            if (s > chunk_start)
                a = atom_push(a, string(chunk_start, s - chunk_start));
            s++; // skip closing quote
            break;
        }

        if (*s == '\n' || *s == '\r') {
            ERR_ANNOTATED_CONTEXT(
                "Unterminated string",
                start_offset,
                tz.Start + (s - tz.Start),
                "The literal started here",
                s,
                s + 1,
                "Newline encountered before closing quote"
            );
            tz.Current = s;
            return (token){TOKEN_POISONED, start_offset - tz.Start};
        }

        if (*s == '\\') {
            if (s > chunk_start)
                a = atom_push(a, string(chunk_start, s - chunk_start));
            const char *start_escape = s;
            s++; // skip backslash
            if (!*s) break;

            code_point cp = unicode_parse_escape(&s);
            char buf[4];
            utf8_encode_cp(buf, cp);
            a = atom_push(a, string(buf, utf8_get_size_of_cp(buf)));
            chunk_start = s;
        } else {
            s++;
        }
    }

    tz.Current = s;
    atom *final_atom = atom_put(a);
    return (token){TOKEN_STRING, start_offset - tz.Start, final_atom };
}

void tokenizer_skip_trivia(tokenizer ref tz) {
    const char *s = tz.Current;

    while (*s) {
        byte c = (byte)*s;

        // Track newlines for line counting
        if (c == '\n') {
            s++;
            tz.CurrentLine++;
            tz.CurrentLineStart = s;
            continue;
        }

        // ASCII whitespace
        if (c < 0x80 && ascii_is_space((char)c)) {
            s++;
            continue;
        }

        // Comments
        if (c == '/' && s[1]) {
            if (s[1] == '/') {
                // Line comment
                s += 2;
                while (*s && *s != '\n' && *s != '\r') s++;
                continue;
            }
            if (s[1] == '*') {
                // Block comment
                const char *comment_start = s;
                s += 2;
                bool found_end = false;
                while (*s) {
                    if (*s == '*' && s[1] && s[1] == '/') {
                        s += 2;
                        found_end = true;
                        break;
                    }
                    s++;
                }
                if (!found_end) {
                    ERR_ANNOTATED_CONTEXT(
                        "Unterminated block comment",
                        comment_start, 
                        comment_start+1,             
                        "Block comment started here",
                        comment_start,
                        s,
                        "End of input reached before closing comment"
                    );
                }
                continue;
            }
        }

        // Unicode whitespace
        if (c >= 0x80) {
            code_point cp = utf8_decode_cp(s);
            s64 sz = utf8_get_size_of_cp(s);
            if (unicode_is_whitespace(cp)) {
                s += (usize)sz;
                continue;
            }
        }

        if (c == '#' && s == tz.CurrentLineStart) {
            // Attempt to parse: #line <number> "optional-file"
            const char* p = s + 1;
            while (*p == ' ' || *p == '\t') ++p;
            if (memcmp(p, "line", 4) == 0) {
                p += 4;
                while (*p == ' ' || *p == '\t') ++p;
                s64 num = 0;
                while (*p >= '0' && *p <= '9') { num = num * 10 + (*p - '0'); ++p; }
                if (num > 0) tz.CurrentLine = num; // set next line number
                while (*p == ' ' || *p == '\t') ++p;
                if (*p == '"') {
                    ++p; const char* fname_begin = p; while (*p && *p != '"') ++p; const char* fname_end = p;
                    if (*p == '"') {
                        string fn = make_string(fname_begin, fname_end - fname_begin);
                        tz.FileName = to_c_string(fn); // lives in arena (ARENA_TOKEN)
                        ++p;
                    }
                }
                // Skip rest of line
                while (*p && *p != '\n') ++p;
                if (*p == '\n') { tz.CurrentLine++; p++; }
                s = p; // continue scanning
                continue;
            }
        }

        // No trivia left
        break;
    }
    tz.Current = s;
}

token tokenizer_next_token_unicode(tokenizer ref tz)
{
    const char *start = tz.Current;
    const char *s = tz.Current;

    s64 sz = utf8_get_size_of_cp(s); // We have already validated utf-8, so we don't worry.
    code_point cp = utf8_decode_cp(s);
    s += sz;

    // Consume Unicode identifier, only possible scenario here
    if (is_ident_start_cp(cp))
    {
        while (*s)
        {
            code_point c2 = utf8_decode_cp(s);
            if (!is_ident_continue_cp(c2))
                break;
            s += utf8_get_size_of_cp(s);
        }
        tz.Current = s;
        return {TOKEN_IDENTIFIER, start - tz.Start};
    }
    // Fallback: Treat this cp as a single unknown token; advance one cp
    tz.Current = s;
    return {TOKEN_INVALID, start - tz.Start};
}

token tokenizer_next_token(tokenizer ref tz)
{
    tokenizer_skip_trivia(tz);
    const char* start = tz.Current;
    if (!*start) {
        return {TOKEN_INVALID, start - tz.Start};
    }

    if ((byte) *start >= 0x80) {
        return tokenizer_next_token_unicode(mut tz);
    }

    if (*start == '"' || *start == '\'') {
        return tokenizer_next_string_literal(mut tz);
    }

    token t = token_switch(tz);
    if (t.Type != TOKEN_INVALID) {
        if (t.Type != TOKEN_IDENTIFIER) return t;

        // Check if the identifier contains any non-ASCII characters, then push atom and return
        const char *s = tz.Current;
        if ((byte)*s > 0x80) {
            while (*s)
            {
                s64 sz = utf8_get_size_of_cp(s);
                if (sz <= 0)
                    break;
                code_point cp = utf8_decode_cp(s);
                if (!is_ident_continue_cp(cp))
                    break;
                s += sz;
            }
        }
        tz.Current = s;
        t.Atom = atom_put(string(start, tz.Current - start));
        return t;
    }

    // Unknown: consume one and return invalid to avoid stalling
    tz.Current++;
    return {TOKEN_INVALID, start - tz.Start};
}

static s128 parse_int128(const char *s, size_t len, s32 base)
{
    s128 result = 0;
    int negative = 0;
    size_t i = 0;

    if (s[i] == '+' || s[i] == '-')
    {
        negative = s[i] == '-';
        i++;
    }

    for (; i < len; i++)
    {
        char c = s[i];
        int digit = -1;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'f')
            digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            digit = c - 'A' + 10;
        else
        {
            break;
        }

        if (digit >= base)
        {
            break;
        }

        result = result * base + digit;
    }
    if (negative)
        result.lo = -result.lo;
    return result;
}

/*
 * strtod implementation from minlibc.
 * https://github.com/GaloisInc/minlibc Here is a copy of the license:
 *
 * Copyright (c) 2014 Galois Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *   * Neither the name of Galois, Inc. nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
f64 parse_double(const char *s)
{
    // This function stolen from either Rolf Neugebauer or Andrew Tolmach.
    // Probably Rolf.
    const char *begin = s;

    f64 a = 0.0;
    int e = 0;
    int c;
    while ((c = *s++) != '\0' && ascii_is_digit(c))
    {
        a = a * 10.0 + (c - '0');
    }
    if (c == '.')
    {
        while ((c = *s++) != '\0' && ascii_is_digit(c))
        {
            a = a * 10.0 + (c - '0');
            e = e - 1;
        }
    }
    if (c == 'e' || c == 'E')
    {
        int sign = 1;
        int i = 0;
        c = *s++;
        if (c == '+')
            c = *s++;
        else if (c == '-')
        {
            c = *s++;
            sign = -1;
        }
        while (ascii_is_digit(c))
        {
            i = i * 10 + (c - '0');
            c = *s++;
        }
        e += i * sign;
    }
    while (e > 0)
    {
        a *= 10.0;
        e--;
    }
    while (e < 0)
    {
        a *= 0.1;
        e++;
    }
    return a;
}

token tokenizer_next_number_literal(tokenizer ref tz)
{
    const char *s = tz.Current;
    const char *start = s;

    token res = {TOKEN_INVALID, start - tz.Start };

    // Optional sign (only if followed by digit or 0x/0b style prefix)
    if ((*s == '+' || *s == '-') && (ascii_is_digit(s[1]) || (s[1] == '0' && (s[2] == 'x' || s[2] == 'X' || s[2] == 'b' || s[2] == 'B')))) {
        s++;
    }

    // Base detection
    int base = 10;
    if (*s == '0')
    {
        if (s[1] == 'x' || s[1] == 'X')
        {
            base = 16;
            s += 2;
        }
        else if (s[1] == 'b' || s[1] == 'B')
        {
            base = 2;
            s += 2;
        }
        else
        {
            base = 8;
            s += 1;
        }
    }

    const char *int_start = s;
    while (ascii_is_digit_based(*s, base))
        s++;
    size_t int_len = s - int_start;

    if ((base == 16 || base == 2) && int_len == 0)
    {
        ERR_ANNOTATED("Invalid integer", start, s, mprint("No digits after base {} prefix", base));
        tz.Current = s;
        res.Type = TOKEN_POISONED;
        return res;
    }

    bool is_float = false;

    // Check for fractional / exponent part (only for base 10)
    if (base == 10 && (*s == '.' || *s == 'e' || *s == 'E'))
    {
        is_float = true;
        bool seen_dot = false;
        if (*s == '.') {
            seen_dot = true;
            s++;
            while (ascii_is_digit(*s)) s++;
        }
        if (*s == 'e' || *s == 'E')
        {
            char exp = *s;
            s++;
            if (*s == '+' || *s == '-')
                s++;
            const char *exp_start = s;
            while (ascii_is_digit(*s))
                s++;
            if (s == exp_start)
            {
                ERR_ANNOTATED("Invalid real number in scientific notation", start, s, mprint("Missing digits after '{:c}'", exp));
                tz.Current = s;
                res.Type = TOKEN_POISONED;
                return res;
            }
        }
        // Detect second '.' -> error (e.g., 12.34.56)
        if (*s == '.') {
            ERR_ANNOTATED("Invalid real number", start, s + 1, "Multiple '.'");
            // Consume the extra '.' and associated digits to avoid cascade
            s++;
            while (ascii_is_digit(*s)) s++;

            tz.Current = s;
            res.Type = TOKEN_POISONED;
            return res;
        }
    }

    // No digits parsed, somehow
    assert(s != start);

    size_t len = s - start;
    res.Atom = atom_put(string(start, len));

    if (is_float)
    {
        res.FloatValue = parse_double(start); // parse from beginning of literal
        res.Type = TOKEN_FLOAT;
    }
    else
    {
        res.IntValue = parse_int128(start, len, base);
        res.Type = TOKEN_INTEGER;
    }

    tz.Current = s;
    return res;
}