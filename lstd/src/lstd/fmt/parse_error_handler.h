#pragma once

#include "../memory/string.h"

using parse_error_handler_t = void (*)(const string &message, const string &formatString, s64 position);
void default_parse_error_handler(const string &message, const string &formatString, s64 position);

