#include "../../lang.h"
#include "../../snipffi.h"

#include "lstd/fmt.h"
#include "token_generated.inc"

#include <cstdio>

// Error reporting with single annotation
static void tokenizer_error(tokenizer no_copy tz, const char* title, usize error_pos, s64 error_len, const char* annotation_message) {
    SnippetHandle snippet = snippet_new(to_c_string_temp(tz.Source), 1);
    AnnotationHandle annotation = annotation_new_primary((int)error_pos, (int)(error_pos + error_len), annotation_message);
    snippet_add_annotation(snippet, annotation);
    char* error_output = render_error(title, snippet);
    print("{}\n", error_output);
}

// Error reporting with two annotations (primary + context)
static void tokenizer_error_2(tokenizer no_copy tz, const char* title, 
                              usize error_pos, s64 error_len, const char* primary_msg,
                              usize context_pos, s64 context_len, const char* context_msg) {
    SnippetHandle snippet = snippet_new(to_c_string_temp(tz.Source), 1);
    AnnotationHandle primary = annotation_new_primary((int)error_pos, (int)(error_pos + error_len), primary_msg);
    AnnotationHandle context = annotation_new_context((int)context_pos, (int)(context_pos + context_len), context_msg);
    snippet_add_annotation(snippet, primary);
    snippet_add_annotation(snippet, context);
    char* error_output = render_error(title, snippet);
    print("{}\n", error_output);
}

// Error reporting with no annotations (just title)
static void tokenizer_error_simple(tokenizer no_copy tz, const char* title) {
    SnippetHandle snippet = snippet_new(to_c_string_temp(tz.Source), 1);
    char* error_output = render_error(title, snippet);
    print("{}\n", error_output);
}

// String literal parsing result
struct string_literal_result {
    token_type type;
    bool is_wide;    // L prefix
    bool is_char;    // single quotes vs double quotes
    s64 consumed;
};

// Parse string and character literals with proper C99/C11 handling
static string_literal_result parse_string_literal(const tokenizer& tz, usize start_pos, const char* s, s64 length) {
    if (length <= 0) return {TOKEN_INVALID, false, false, 0};
    
    s64 pos = 0;
    bool is_wide = false;
    
    // Check for L prefix
    if (s[pos] == 'L' && pos + 1 < length) {
        is_wide = true;
        pos++;
    }
    
    if (pos >= length) return {TOKEN_INVALID, false, false, 0};
    
    char quote = s[pos];
    if (quote != '"' && quote != '\'') {
        return {TOKEN_INVALID, false, false, 0};
    }
    
    bool is_char = (quote == '\'');
    s64 quote_start = pos;
    pos++; // Skip opening quote
    
    // Scan until closing quote, handling escapes
    bool found_closing = false;
    while (pos < length) {
        char c = s[pos];
        if (c == '\n' || c == '\r') {
            // Unterminated string/character literal - consume up to this point
            const char* title = is_char ? "unterminated character literal" : "unterminated string literal";
            const char* annotation = is_char ? "character literal started here" : "string literal started here";
            tokenizer_error(tz, title, start_pos + quote_start, 1, annotation);
            return {TOKEN_INVALID, false, false, pos};
        }
        if (c == quote) {
            pos++; // Include closing quote
            found_closing = true;
            break;
        }
        if (c == '\\' && pos + 1 < length) {
            pos += 2; // Skip escape sequence
        } else {
            pos++;
        }
    }
    
    // Check if we reached end without finding closing quote
    if (!found_closing) {
        const char* title = is_char ? "unterminated character literal" : "unterminated string literal";
        const char* annotation = is_char ? "character literal started here" : "string literal started here";
        tokenizer_error(tz, title, start_pos + quote_start, 1, annotation);
        return {TOKEN_INVALID, false, false, pos};
    }
    
    // Determine token type
    token_type type;
    if (is_wide) {
        type = is_char ? TOKEN_STRING_WIDE_SINGLE_QUOTE : TOKEN_STRING_WIDE_DOUBLE_QUOTE;
    } else {
        type = is_char ? TOKEN_STRING_SINGLE_QUOTE : TOKEN_STRING_DOUBLE_QUOTE;
    }
    
    return {type, is_wide, is_char, pos};
}

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
                    usize comment_start = tz.Position;
                    tz.Position += 2;
                    bool found_end = false;
                    while (!tokenizer_at_end(tz))
                    {
                        if (tz.Source.Data[tz.Position] == '*' && tz.Position + 1 < tz.Source.Count && tz.Source.Data[tz.Position + 1] == '/')
                        {
                            tz.Position += 2;
                            found_end = true;
                            break;
                        }
                        tokenizer_advance(tz, 1);
                    }
                    if (!found_end) {
                        tokenizer_error(tz, "unterminated block comment", comment_start, 2, "block comment started here");
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

bool is_ident_start_cp(code_point cp)
{
    return cp == '_' || unicode_is_letter(cp);
}

bool is_ident_continue_cp(code_point cp)
{
    return unicode_is_letter(cp) || unicode_is_number(cp) || unicode_is_mark(cp) || cp == '_';
}

token tokenizer_next_token_unicode(tokenizer ref tokenizer)
{
    usize start = tokenizer.Position;
    const char *p = tokenizer.Source.Data + tokenizer.Position;
    const char *end = tokenizer.Source.Data + tokenizer.Source.Count;
    if (p >= end)
        return {TOKEN_INVALID, start};

    s64 sz = utf8_get_size_of_cp(p); // We have already validated utf-8, so we don't worry.
    code_point cp = utf8_decode_cp(p);

    // Consume Unicode identifier, only possible scenario here
    if (is_ident_start_cp(cp))
    {
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

    // Try the generated token switch function first
    const char *s = tokenizer.Source.Data + tokenizer.Position;
    s64 remaining = (s64)tokenizer.Source.Count - (s64)tokenizer.Position;
    s64 consumed = 0;
    token_type tt = token_switch(s, remaining, &consumed);
    if (tt != TOKEN_INVALID)
    {
        if (tt != TOKEN_IDENTIFIER) {
            tokenizer_advance(tokenizer, consumed);
            return {tt, start};
        }

        // Generated function found ASCII identifier, but we need to check for Unicode continuation
        tokenizer_advance(tokenizer, consumed);
        while (!tokenizer_at_end(tokenizer))
        {
            char ch = tokenizer_peek_ascii(tokenizer);
            if ((unsigned char)ch >= 0x80)
            {
                // Unicode character - need to decode and check
                const char *p = tokenizer.Source.Data + tokenizer.Position;
                s64 sz = utf8_get_size_of_cp(p);
                if (sz <= 0) break;
                code_point cp = utf8_decode_cp(p);
                if (!is_ident_continue_cp(cp))
                    break;
                tokenizer_advance(tokenizer, sz);
            } 
            else {
                break;
            }
        }
        return {TOKEN_IDENTIFIER, start};
    }

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
                {
                    // Multiple decimal points in number
                    tokenizer_error(tokenizer, "invalid number literal", tokenizer.Position, 1, "second decimal point found here");
                    tokenizer_advance(tokenizer, 1); // Skip the invalid dot
                    continue;
                }
                is_float = true;
                tokenizer_advance(tokenizer, 1);
                continue;
            }
            break;
        }
        return {is_float ? TOKEN_FLOAT : TOKEN_INTEGER, start};
    }

    // String / char literals with optional L prefix
    if (c == 'L' || c == '"' || c == '\'')
    {
        const char *s = tokenizer.Source.Data + tokenizer.Position;
        s64 remaining = (s64)tokenizer.Source.Count - (s64)tokenizer.Position;
        auto result = parse_string_literal(tokenizer, start, s, remaining);
        if (result.type != TOKEN_INVALID)
        {
            tokenizer_advance(tokenizer, result.consumed);
            
            // Only concatenate string literals (not character literals)
            if (result.type == TOKEN_STRING_DOUBLE_QUOTE || result.type == TOKEN_STRING_WIDE_DOUBLE_QUOTE) {
                while (!tokenizer_at_end(tokenizer)) {
                    tokenizer_skip_trivia(mut tokenizer);
                    if (tokenizer_at_end(tokenizer))
                        break;
                        
                    byte next_c = tokenizer_peek_ascii(tokenizer);
                    if (next_c == 'L' || next_c == '"') {
                        const char *next_s = tokenizer.Source.Data + tokenizer.Position;
                        s64 next_remaining = (s64)tokenizer.Source.Count - (s64)tokenizer.Position;
                        auto next_result = parse_string_literal(tokenizer, tokenizer.Position, next_s, next_remaining);
                        
                        // Only concatenate compatible string literals (same wideness, both strings not chars)
                        if ((next_result.type == TOKEN_STRING_DOUBLE_QUOTE && result.type == TOKEN_STRING_DOUBLE_QUOTE) ||
                            (next_result.type == TOKEN_STRING_WIDE_DOUBLE_QUOTE && result.type == TOKEN_STRING_WIDE_DOUBLE_QUOTE)) {
                            tokenizer_advance(tokenizer, next_result.consumed);
                            continue;
                        } else if (next_result.type != TOKEN_INVALID) {
                            // Found a string-like token but not compatible for concatenation
                            if (next_result.is_char) {
                                tokenizer_error(tokenizer, "cannot concatenate string and character literals", tokenizer.Position, 1, "character literal cannot be concatenated with string literal");
                            } else if (next_result.is_wide != result.is_wide) {
                                tokenizer_error(tokenizer, "cannot concatenate wide and narrow string literals", tokenizer.Position, 1, "incompatible string literal type");
                            }
                            break;
                        } else {
                            // Not compatible for concatenation, stop
                            break;
                        }
                    } else {
                        // No more string literals to concatenate
                        break;
                    }
                }
            }
            return {result.type, start};
        }
    }

    // Unknown: consume one and return invalid to avoid stalling
    tokenizer_advance(tokenizer, 1);
    return {TOKEN_INVALID, start};
}
