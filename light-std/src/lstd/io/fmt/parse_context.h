#pragma once

#include "../../intrin.h"
#include "../../storage/string_utils.h"

#include "error_handler.h"
#include "specs.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct parse_context {
    string FmtString;
    const char *It = null, *End = null;

    s32 NextArgID = 0;

    error_handler_t ErrorHandlerFunc = default_error_handler;

    parse_context(string fmtString, error_handler_t errorHandlerFunc)
        : FmtString(fmtString), ErrorHandlerFunc(errorHandlerFunc) {
        It = FmtString.Data;
        End = FmtString.Data + fmtString.ByteLength;
    }

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

    arg_ref parse_arg_id();

    // _argType_ is the type of the argument for which we are parsing the specs.
    // It is used, for example, to check if it's numeric when we encounter numeric-only specs.
    bool parse_fmt_specs(type argType, dynamic_format_specs *specs);
    bool parse_text_style(text_style *textStyle);

    // @TODO: Have a way to specify position to be more accurate in the error message.
    void on_error(string message) const {
        if (ErrorHandlerFunc) ErrorHandlerFunc(message, {FmtString, (size_t)(It - FmtString.Data)});
    }

   private:
    void require_numeric_arg(type argType);
    void require_signed_arg(type argType);
    void check_precision_for_arg(type argType);

    // Assumes the first byte is a digit.
    u32 parse_nonnegative_int();

    // Parses _fill_ code point as well
    bool parse_align(type argType, format_specs *specs);
    bool parse_width(dynamic_format_specs *specs);
    bool parse_precision(type argType, dynamic_format_specs *specs);

    u32 parse_rgb_channel(bool last);
    bool handle_emphasis(text_style *textStyle);
};
}  // namespace fmt

LSTD_END_NAMESPACE
