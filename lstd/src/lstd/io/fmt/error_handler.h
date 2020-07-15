#pragma once

#include "../../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct error_context {
    string FmtString;
    s64 Position;
};

using error_handler_t = void (*)(const string &, error_context);

extern void default_error_handler(const string &message, error_context errorContext);
}  // namespace fmt

LSTD_END_NAMESPACE
