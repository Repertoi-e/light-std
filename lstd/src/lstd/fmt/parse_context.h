#pragma once

#include "specs.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

using parse_error_handler_t = void (*)(const string &message, const string &formatString, s64 position);
void default_parse_error_handler(const string &message, const string &formatString, s64 position);

struct parse_context {
    string FormatString;
    string It;  // How much left we have to parse from the format string

    s32 NextArgID = 0;

    parse_error_handler_t ErrorHandlerFunc = default_parse_error_handler;

    parse_context(const string &formatString, parse_error_handler_t errorHandlerFunc) : FormatString(formatString), It(formatString), ErrorHandlerFunc(errorHandlerFunc) {}
};

u32 next_arg_id(parse_context *p);

// Note: When parsing, if we reach the end before } or : or whatever we don't report an error.
// The caller of this should handle that. Returns -1 if an error occured (the error is reported).
s64 parse_arg_id(parse_context *p);

// _argType_ is the type of the argument for which we are parsing the specs.
// It is used for error checking, e.g, to check if it's numeric when we encounter numeric-only specs.
//
// Note: When parsing, if we reach the end before } we don't report an error. The caller of this should handle that.
bool parse_fmt_specs(parse_context *p, type argType, dynamic_format_specs *specs);

// @TODO: Return text_style with the bool in a pair
bool parse_text_style(parse_context *p, text_style *textStyle);

// The position tells where to point the caret in the format string, so it is clear where exactly the error happened.
// If left as -1 we calculate using the current It.
// We may want to pass a different position if we are in the middle of parsing and the It is not pointing at the right place.
inline void on_error(parse_context *p, const string &message, s64 position = -1) {
    if (position == -1) position = p->It.Data - p->FormatString.Data;
    if (p->ErrorHandlerFunc) p->ErrorHandlerFunc(message, p->FormatString, position);
}

}  // namespace fmt

LSTD_END_NAMESPACE
