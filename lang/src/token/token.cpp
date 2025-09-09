#include "../../token.h"

#include "tokenizer_keywords_switch.inc"
#include "tokenizer_punct_switch.inc"
#include "token_print.inc"

#include "lstd/unicode.h"

void tokenizer_skip_trivia(tokenizer ref tz)
{
    for (;;)
    {
        if (tokenizer_at_end(tz))
            return;
        byte c = tokenizer_peek_ascii(tz);
        // ASCII whitespace
        if (c < 0x80 && ascii_is_space((char)c))
        {
            tokenizer_advance(tz, 1);
            continue;
        }
        // Comments
        if (c == '/')
        {
            if (tz.Position + 1 < tz.Source.Count)
            {
                char n = tz.Source.Data[tz.Position + 1];
                if (n == '/')
                { // line comment
                    tz.Position += 2;
                    while (!tokenizer_at_end(tz))
                    {
                        char cc = tokenizer_peek_ascii(tz);
                        if (cc == '\n' || cc == '\r')
                        {
                            tokenizer_advance(tz, 1);
                            break;
                        }
                        tokenizer_advance(tz, 1);
                    }
                    continue;
                }
                else if (n == '*')
                { // block comment
                    tz.Position += 2;
                    while (!tokenizer_at_end(tz))
                    {
                        if (tz.Source.Data[tz.Position] == '*' && tz.Position + 1 < tz.Source.Count && tz.Source.Data[tz.Position + 1] == '/')
                        {
                            tz.Position += 2;
                            break;
                        }
                        tokenizer_advance(tz, 1);
                    }
                    continue;
                }
            }
        }
        // Unicode whitespace: decode cp and skip
        if ((unsigned char)c >= 0x80)
        {
            const char *p = tz.Source.Data + tz.Position;
            s64 sz = utf8_get_size_of_cp(p);
            code_point cp = utf8_decode_cp(p);
            if (unicode_is_whitespace(cp))
            {
                tz.Position += (usize)sz;
                continue;
            }
        }
        return;
    }
}

token tokenizer_next_token_unicode(tokenizer ref tokenizer)
{
    usize start = tokenizer.Position;
    const char *p = tokenizer.Source.Data + tokenizer.Position;
    const char *end = tokenizer.Source.Data + tokenizer.Source.Count;
    if (p >= end)
        return {TOKEN_INVALID, start};

    s64 sz = utf8_get_size_of_cp(p); // We have already validated utf-8
    code_point cp = utf8_decode_cp(p);

    if (unicode_is_whitespace(cp))
    {
        tokenizer_advance(tokenizer, sz);
        return tokenizer_next_token(tokenizer);
    }

    auto is_ident_start_cp = [](code_point c)
    {
        return c == '_' || unicode_is_letter(c); // allow underscore and any letter
    };

    auto is_ident_continue_cp = [](code_point c)
    {
        // allow letters, numbers, marks and underscore
        if (c == '_' || unicode_is_letter(c) || unicode_is_number(c) || unicode_is_mark(c))
            return true;
        return false;
    };

    if (is_ident_start_cp(cp))
    {
        // consume identifier
        tokenizer_advance(tokenizer, (usize)sz);
        const char *q = tokenizer.Source.Data + tokenizer.Position;
        while (q < end)
        {
            s64 s2 = utf8_get_size_of_cp(q);
            if (s2 <= 0 || q + s2 > end || !utf8_is_valid_cp(q))
                break;
            code_point c2 = utf8_decode_cp(q);
            if (!is_ident_continue_cp(c2))
                break;
            tokenizer_advance(tokenizer, (usize)s2);
            q += s2;
        }
        return {TOKEN_IDENTIFIER, start};
    }

    // Fallback: treat this cp as a single unknown token; advance one cp
    tokenizer_advance(tokenizer, (usize)sz);
    return {TOKEN_INVALID, start};
}

token tokenizer_next_token(tokenizer ref tokenizer)
{
    tokenizer_skip_trivia(tokenizer);
    usize start = tokenizer.Position;
    if (tokenizer_at_end(tokenizer))
        return {TOKEN_INVALID, start};

    char c = tokenizer_peek_ascii(tokenizer);
    if ((unsigned char)c >= 0x80)
        return tokenizer_next_token_unicode(mut tokenizer);

    // Punctuators (1/2/3-char), longest-match
    {
        const char *s = tokenizer.Source.Data + tokenizer.Position;
        s64 remaining = (s64)tokenizer.Source.Count - (s64)tokenizer.Position;
        auto r = scan_punct(s, remaining);
        if (r.Type != TOKEN_INVALID)
        {
            tokenizer_advance(tokenizer, r.Length);
            return {r.Type, start};
        }
    }

    // Identifiers (ASCII)
    if (ascii_is_identifier_start(c))
    {
        tokenizer_advance(tokenizer, 1);
        while (!tokenizer_at_end(tokenizer) && ascii_is_alphanumeric(tokenizer_peek_ascii(tokenizer)))
            tokenizer_advance(tokenizer, 1);
        s64 len = (s64)tokenizer.Position - (s64)start;
        token_type kw = classify_identifier_ascii(tokenizer.Source.Data + start, len);
        return {kw, start};
    }

    // Numbers (simple integer/float with dot)
    if (ascii_is_digit(c))
    {
        bool is_float = false;
        tokenizer_advance(tokenizer, 1);
        while (!tokenizer_at_end(tokenizer))
        {
            char d = tokenizer_peek_ascii(tokenizer);
            if (ascii_is_digit(d))
            {
                tokenizer_advance(tokenizer, 1);
                continue;
            }
            if (d == '.')
            {
                if (is_float)
                    break;
                is_float = true;
                tokenizer_advance(tokenizer, 1);
                continue;
            }
            break;
        }
        return {is_float ? TOKEN_FLOAT : TOKEN_INTEGER, start};
    }

    // String / char literals with optional L prefix
    if (c == 'L')
    {
        if (tokenizer.Position + 1 < tokenizer.Source.Count)
        {
            char q = tokenizer.Source.Data[tokenizer.Position + 1];
            if (q == '"' || q == '\'')
            {
                tokenizer_advance(tokenizer, 2); // consume L and quote
                char quote = q;
                // scan until closing quote (handle escapes)
                while (!tokenizer_at_end(tokenizer))
                {
                    char ch = tokenizer_peek_ascii(tokenizer);
                    if (ch == '\\')
                    {
                        tokenizer_advance(tokenizer, 1);
                        if (!tokenizer_at_end(tokenizer))
                            tokenizer_advance(tokenizer, 1);
                        continue;
                    }
                    tokenizer_advance(tokenizer, 1);
                    if (ch == quote)
                        break;
                }
                token_type tt = (quote == '"') ? TOKEN_STRING_WIDE_DOUBLE_QUOTE : TOKEN_STRING_WIDE_SINGLE_QUOTE;
                return {tt, start};
            }
        }
    }
    if (c == '"' || c == '\'')
    {
        char quote = c;
        tokenizer_advance(tokenizer, 1);
        while (!tokenizer_at_end(tokenizer))
        {
            char ch = tokenizer_peek_ascii(tokenizer);
            if (ch == '\\')
            {
                tokenizer_advance(tokenizer, 1);
                if (!tokenizer_at_end(tokenizer))
                    tokenizer_advance(tokenizer, 1);
                continue;
            }
            tokenizer_advance(tokenizer, 1);
            if (ch == quote)
                break;
        }
        token_type tt = (quote == '"') ? TOKEN_STRING_DOUBLE_QUOTE : TOKEN_STRING_SINGLE_QUOTE;
        return {tt, start};
    }

    // 3-char punctuators
    auto match3 = [&](char a, char b, char d)
    {
        if (tokenizer.Position + 2 < tokenizer.Source.Count)
        {
            char c0 = tokenizer.Source.Data[tokenizer.Position];
            char c1 = tokenizer.Source.Data[tokenizer.Position + 1];
            char c2 = tokenizer.Source.Data[tokenizer.Position + 2];
            return c0 == a && c1 == b && c2 == d;
        }
        return false;
    };
    if (match3('<', '<', '='))
    {
        tokenizer_advance(tokenizer, 3);
        return {TOKEN_LEFT_SHIFT_EQUAL, start};
    }
    if (match3('>', '>', '='))
    {
        tokenizer_advance(tokenizer, 3);
        return {TOKEN_RIGHT_SHIFT_EQUAL, start};
    }
    if (match3('.', '.', '.'))
    {
        tokenizer_advance(tokenizer, 3);
        return {TOKEN_TRIPLE_DOT, start};
    }

    // 2-char punctuators
    auto match2 = [&](char a, char b)
    {
        if (tokenizer.Position + 1 < tokenizer.Source.Count)
        {
            char c0 = tokenizer.Source.Data[tokenizer.Position];
            char c1 = tokenizer.Source.Data[tokenizer.Position + 1];
            return c0 == a && c1 == b;
        }
        return false;
    };
    if (match2('-', '>'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_ARROW, start};
    }
    if (match2('#', '#'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_DOUBLE_HASH, start};
    }
    if (match2('&', '&'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_DOUBLE_AND, start};
    }
    if (match2('|', '|'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_DOUBLE_OR, start};
    }
    if (match2('+', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_PLUS_EQUAL, start};
    }
    if (match2('-', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_MINUS_EQUAL, start};
    }
    if (match2('*', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_TIMES_EQUAL, start};
    }
    if (match2('/', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_SLASH_EQUAL, start};
    }
    if (match2('%', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_PERCENT_EQUAL, start};
    }
    if (match2('|', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_OR_EQUAL, start};
    }
    if (match2('&', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_AND_EQUAL, start};
    }
    if (match2('^', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_XOR_EQUAL, start};
    }
    if (match2('!', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_NOT_EQUAL, start};
    }
    if (match2('=', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_EQUALITY, start};
    }
    if (match2('>', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_GREATER_EQUAL, start};
    }
    if (match2('<', '='))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_LESS_EQUAL, start};
    }
    if (match2('<', '<'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_LEFT_SHIFT, start};
    }
    if (match2('>', '>'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_RIGHT_SHIFT, start};
    }
    if (match2('+', '+'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_INCREMENT, start};
    }
    if (match2('-', '-'))
    {
        tokenizer_advance(tokenizer, 2);
        return {TOKEN_DECREMENT, start};
    }

    // Single-char tokens
    auto take1 = [&](token_type t)
    { tokenizer_advance(tokenizer,1); return token{t, start}; };
    switch (c)
    {
    case '.':
        return take1(TOKEN_DOT);
    case ',':
        return take1(TOKEN_COMMA);
    case '+':
        return take1(TOKEN_PLUS);
    case '-':
        return take1(TOKEN_MINUS);
    case '*':
        return take1(TOKEN_TIMES);
    case '/':
        return take1(TOKEN_SLASH);
    case '%':
        return take1(TOKEN_PERCENT);
    case '=':
        return take1(TOKEN_ASSIGN);
    case '&':
        return take1(TOKEN_AND);
    case '^':
        return take1(TOKEN_XOR);
    case '|':
        return take1(TOKEN_OR);
    case '#':
        return take1(TOKEN_HASH);
    case '@':
        return take1(TOKEN_AT);
    case '~':
        return take1(TOKEN_TILDE);
    case '!':
        return take1(TOKEN_EXCLAMATION);
    case ':':
        return take1(TOKEN_COLON);
    case ';':
        return take1(TOKEN_SEMICOLON);
    case '<':
        return take1(TOKEN_LESS);
    case '>':
        return take1(TOKEN_GREATER);
    case '[':
        return take1(TOKEN_BRACKET_OPEN);
    case ']':
        return take1(TOKEN_BRACKET_CLOSE);
    case '(':
        return take1(TOKEN_PAREN_OPEN);
    case ')':
        return take1(TOKEN_PAREN_CLOSE);
    case '{':
        return take1(TOKEN_BRACE_OPEN);
    case '}':
        return take1(TOKEN_BRACE_CLOSE);
    }

    // Unknown: consume one and return invalid to avoid stalling
    tokenizer_advance(tokenizer, 1);
    return {TOKEN_INVALID, start};
}
