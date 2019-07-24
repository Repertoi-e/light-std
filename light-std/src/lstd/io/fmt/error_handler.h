#pragma once

#include "../../common.h"

#include "../../storage/string_utils.h"

LSTD_BEGIN_NAMESPACE

namespace fmt {

struct error_context {
    string_view FmtString;
    size_t Position;
};

using error_handler_t = void (*)(const byte *message, error_context errorContext);

extern void default_error_handler(const byte *message, error_context errorContext);
}  // namespace fmt

LSTD_END_NAMESPACE
