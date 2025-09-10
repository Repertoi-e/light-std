#!/usr/bin/env python3
# Generates token_generated.inc with helper functions for tokenizer.
# Parses the token_type enum from lang.h to extract keywords and punctuators.

import os
import sys
import re
from collections import defaultdict

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LANG_H = os.path.join(SCRIPT_DIR, 'lang.h')
OUT_TOKEN_GEN = os.path.join(SCRIPT_DIR, 'src', 'token', 'token_generated.inc')


def parse_token_enum(path):
    """Parse the token_type enum from lang.h and extract tokens with their string literals."""
    keywords = []
    punctuators = []
    token_strings = {}
    
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find the enum token_type definition
    enum_match = re.search(r'enum token_type\s*\{(.*?)\};', content, re.DOTALL)
    if not enum_match:
        raise ValueError("Could not find token_type enum in lang.h")
    
    enum_content = enum_match.group(1)
    lines = enum_content.split('\n')
    
    for line in lines:
        original_line = line  # Store original line for comment parsing
        line = line.strip()
        if not line or line.startswith('//') or line.startswith('#'):
            continue
            
        if '=' in line:
            # Handle assignment lines carefully to preserve full value
            parts = line.split('=', 1)
            token_name = parts[0].strip()
            value_part = parts[1].strip()
            # Remove trailing comma if present
            if value_part.endswith(','):
                value_part = value_part[:-1].strip()
            
            # Handle single character tokens like TOKEN_DOT = '.'
            if value_part.startswith("'") and value_part.endswith("'") and len(value_part) == 3:
                char = value_part[1]
                punctuators.append((char, token_name))
                token_strings[token_name] = char
            
            # Handle keywords - they start at 0x10000000
            elif token_name.startswith('TOKEN_KW_'):
                # Check if there's a comment with the actual keyword in the original line
                comment_match = re.search(r'//\s*(\S+)', original_line)
                if comment_match:
                    kw = comment_match.group(1)
                    keywords.append((kw, token_name))
                    token_strings[token_name] = kw
                else:
                    # Fallback: extract keyword from token name
                    kw = token_name[9:]  # Remove TOKEN_KW_ prefix
                    keywords.append((kw, token_name))
                    token_strings[token_name] = kw
            
            # Handle multi-character operators
            elif 'TKN2' in value_part or 'TKN3' in value_part:
                # Extract characters from TKN2('a', 'b') or TKN3('a', 'b', 'c')
                chars_match = re.findall(r"'(.)'", value_part)
                if chars_match:
                    op = ''.join(chars_match)
                    punctuators.append((op, token_name))
                    token_strings[token_name] = op
        else:
            # Handle tokens without explicit values
            original_line_for_comment = line  # Save original line for comment parsing
            
            # Remove trailing comma and comments for token name extraction
            if ',' in line:
                line = line.split(',')[0].strip()
            
            if line.startswith('TOKEN_'):
                token_name = line
                # Check if there's a comment with the actual keyword
                comment_match = re.search(r'//\s*(\S+)', original_line_for_comment)
                
                if token_name.startswith('TOKEN_KW_'):
                    if comment_match:
                        kw = comment_match.group(1)
                        keywords.append((kw, token_name))
                        token_strings[token_name] = kw
                    else:
                        # Fallback: extract keyword from token name
                        kw = token_name[9:]  # Remove TOKEN_KW_ prefix
                        keywords.append((kw, token_name))
                        token_strings[token_name] = kw
                elif token_name in ['TOKEN_IDENTIFIER', 'TOKEN_INTEGER', 'TOKEN_FLOAT']:
                    # Add default representations for special tokens
                    if token_name == 'TOKEN_IDENTIFIER':
                        token_strings[token_name] = '<identifier>'
                    elif token_name == 'TOKEN_INTEGER':
                        token_strings[token_name] = '<int>'
                    elif token_name == 'TOKEN_FLOAT':
                        token_strings[token_name] = '<float>'
    
    return keywords, punctuators, token_strings


def c_string_literal(s: str) -> str:
    """Escape string for inclusion inside C double quotes."""
    out = '"'
    for ch in s:
        if ch == '"':
            out += '\\"'
        elif ch == '\\':
            out += '\\\\'
        elif ch == '\n':
            out += '\\n'
        elif ch == '\t':
            out += '\\t'
        else:
            out += ch
    out += '"'
    return out


def emit_token_to_string(token_strings):
    """Generate the token_to_string function."""
    lines = []
    lines.append('// Generated by gen_tokenizer.py. Do not edit by hand.')
    lines.append('const char* token_to_string(token_type t) {')
    lines.append('  switch (t) {')
    for token_enum, text in sorted(token_strings.items()):
        lines.append(f'    case {token_enum}: return {c_string_literal(text)};')
    lines.append('    default: return "<unknown>";')
    lines.append('  }')
    lines.append('}')
    return '\n'.join(lines)


def emit_token_switch(keywords, punctuators):
    """Generate a switch statement for tokenizing based on characters."""
    lines = []
    lines.append('// Generated by gen_tokenizer.py. Do not edit by hand.')
    lines.append('// Always inline token switch for character-by-character matching')
    lines.append('static inline token_type token_switch(const char* s, s64 length, s64* consumed) {')
    lines.append('  *consumed = 0;')
    lines.append('  if (length <= 0) return TOKEN_INVALID;')
    lines.append('')
    
    # Group keywords by first character for efficient switching
    kw_groups = defaultdict(list)
    for kw, token_name in keywords:
        if kw:
            kw_groups[kw[0]].append((kw, token_name))
    
    # Group punctuators by first character, sort by length desc for longest match
    punct_groups = defaultdict(list)
    for punct, token_name in punctuators:
        punct_groups[punct[0]].append((punct, token_name))
    for key in punct_groups:
        punct_groups[key].sort(key=lambda x: -len(x[0]))
    
    # Combine all first characters
    all_chars = set(kw_groups.keys()) | set(punct_groups.keys())
    
    lines.append('  switch (s[0]) {')
    
    for ch in sorted(all_chars):
        lines.append(f"    case '{ch}': {{")
        
        # Handle punctuators first (they have higher priority in most cases)
        if ch in punct_groups:
            # Process multi-character punctuators first (longest match)
            multi_char = [p for p in punct_groups[ch] if len(p[0]) > 1]
            single_char = [p for p in punct_groups[ch] if len(p[0]) == 1]
            
            for punct, token_name in multi_char:
                conditions = [f'length >= {len(punct)}']
                for i, c in enumerate(punct[1:], 1):
                    conditions.append(f"s[{i}] == '{c}'")
                cond_str = ' && '.join(conditions)
                lines.append(f'      if ({cond_str}) {{ *consumed = {len(punct)}; return {token_name}; }}')
            
            # Then single character punctuators
            for punct, token_name in single_char:
                lines.append(f'      *consumed = 1; return {token_name};')
        
        # Handle keywords
        if ch in kw_groups:
            for kw, token_name in kw_groups[ch]:
                if len(kw) == 1:
                    # Single character keyword (unlikely but handle it)
                    lines.append(f'      if (length == 1 || (!ascii_is_alphanumeric(s[1]) && s[1] != \'_\')) {{ *consumed = 1; return {token_name}; }}')
                else:
                    # Multi-character keyword - check each character and ensure word boundary
                    conditions = [f'length >= {len(kw)}']
                    for i, c in enumerate(kw[1:], 1):
                        conditions.append(f"s[{i}] == '{c}'")
                    # Add word boundary check - be conservative with non-ASCII
                    conditions.append(f'(length == {len(kw)} || ((!ascii_is_alphanumeric(s[{len(kw)}]) && s[{len(kw)}] != \'_\')))')
                    cond_str = ' && '.join(conditions)
                    lines.append(f'      if ({cond_str}) {{ *consumed = {len(kw)}; return {token_name}; }}')
        
        lines.append('      break;')
        lines.append('    }')
    
    lines.append('    default: break;')
    lines.append('  }')
    lines.append('')
    lines.append("""  // If we get here, check if it starts with ASCII identifier character
  // Note: Unicode identifiers that continue are handled in the main next_token function
  if (ascii_is_identifier_start(s[0])) {
    s64 i = 1;
    while (i < length && (ascii_is_identifier_cont(s[i]))) i++;
    *consumed = i;
    return TOKEN_IDENTIFIER;
  }""")
    lines.append('')
    lines.append('  return TOKEN_INVALID;')
    lines.append('}')
    
    return '\n'.join(lines)


def generate_token_inc(keywords, punctuators, token_strings):
    """Generate the complete token_generated.inc file."""
    lines = []
    lines.append('/* ')
    lines.append(' * Generated by gen_tokenizer.py. Do not edit by hand.')
    lines.append(' * This file contains auto-generated token helper functions')
    lines.append(' * ')
    lines.append(f' * Keywords: {len(keywords)}')
    lines.append(f' * Punctuators: {len(punctuators)}')
    lines.append(f' * Total tokens: {len(token_strings)}')
    lines.append(' */')
    lines.append('')
    
    # Add the token to string function
    lines.append(emit_token_to_string(token_strings))
    lines.append('')
    
    # Add the token switch function
    lines.append(emit_token_switch(keywords, punctuators))
    
    return '\n'.join(lines)


def main():
    try:
        keywords, punctuators, token_strings = parse_token_enum(LANG_H)
        
        # Ensure output directory exists
        os.makedirs(os.path.dirname(OUT_TOKEN_GEN), exist_ok=True)
        
        # Generate the token_generated.inc file
        token_inc_content = generate_token_inc(keywords, punctuators, token_strings)
        
        with open(OUT_TOKEN_GEN, 'w', encoding='utf-8') as f:
            f.write(token_inc_content)
        
        print(f'Generated {OUT_TOKEN_GEN}')
        print(f'  Keywords: {len(keywords)}')
        print(f'  Punctuators: {len(punctuators)}')
        print(f'  Total tokens: {len(token_strings)}')
        
        return 0
        
    except Exception as e:
        print(f'Error: {e}', file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
