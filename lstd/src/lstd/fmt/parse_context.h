#pragma once

#include "specs.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct parse_context {
    using error_handler_t = void (*)(const string &message, const string &formatString, s64 position);
    static void default_error_handler(const string &message, const string &formatString, s64 position);

    string FormatString;
    string It;  // How much left we have to parse from the format string

    s32 NextArgID = 0;

    error_handler_t ErrorHandlerFunc = default_error_handler;

    parse_context(const string &formatString, error_handler_t errorHandlerFunc) : FormatString(formatString), It(formatString), ErrorHandlerFunc(errorHandlerFunc) {}

    u32 next_arg_id() {
        if (NextArgID >= 0) return (u32) NextArgID++;
        on_error("Cannot switch from manual to automatic argument indexing");
        return 0;
    }

    bool check_arg_id(u32) {
        if (NextArgID > 0) {
            on_error("Cannot switch from automatic to manual argument indexing");
            return false;
        }
        NextArgID = -1;
        return true;
    }

    // Note: When parsing, if we reach the end before } or : or whatever we don't report an error.
    // The caller of this should handle that. Returns -1 if an error occured (the error is reported).
    s64 parse_arg_id();

    // _argType_ is the type of the argument for which we are parsing the specs.
    // It is used for error checking, e.g, to check if it's numeric when we encounter numeric-only specs.
    //
    // Note: When parsing, if we reach the end before } we don't report an error. The caller of this should handle that.
    bool parse_fmt_specs(type argType, dynamic_format_specs *specs);

    // @TODO: Return text_style with the bool in a pair
    bool parse_text_style(text_style *textStyle);

    // The position tells where to point the caret in the format string, so it is clear where exactly the error happened.
    // If left as -1 we calculate using the current It.
    // We may want to pass a different position if we are in the middle of parsing and the It is not pointing at the right place.
    void on_error(const string &message, s64 position = -1) const {
        if (position == -1) position = It.Data - FormatString.Data;
        if (ErrorHandlerFunc) ErrorHandlerFunc(message, FormatString, position);
    }

   private:
    // @TODO: Have a way to hook into these for custom arguments.
    // Right now we parse any specs and pass them to the custom formatter.
    // Would it be useful to have a way to restrict certain specifiers so you can't format an integer vector with precision for example...

    // Some specifiers require numeric arguments and we do error checking, CUSTOM arguments don't get checked
    void require_arithmetic_arg(type argType, s64 errorPosition = -1);

    // Some specifiers require signed numeric arguments and we do error checking, CUSTOM arguments don't get checked
    void require_signed_arithmetic_arg(type argType, s64 errorPosition = -1);

    // Integer values and pointers aren't allowed to get precision. CUSTOM argument is again, not checked.
    void check_precision_for_arg(type argType, s64 errorPosition = -1);

    // Parses _fill_ code point as well
    bool parse_fill_and_align(type argType, format_specs *specs);
    bool parse_width(dynamic_format_specs *specs);
    bool parse_precision(type argType, dynamic_format_specs *specs);

    u32 parse_rgb_channel(bool last);
    bool handle_emphasis(text_style *textStyle);
};
}  // namespace fmt

LSTD_END_NAMESPACE
