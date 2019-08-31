#pragma once

#include "../../storage/string.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct error_context {
    string FmtString;
    size_t Position;
};

using error_handler_t = void (*)(string, error_context);

extern void default_error_handler(string message, error_context errorContext);
}  // namespace fmt

LSTD_END_NAMESPACE