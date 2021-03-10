#pragma once

#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

struct os_function_call {
    string Name;
    string File;
    u32 LineNumber;
};

inline os_function_call *clone(os_function_call *dest, os_function_call src) {
    clone(&dest->Name, src.Name);
    clone(&dest->File, src.File);
    dest->LineNumber = src.LineNumber;
    return dest;
}

LSTD_END_NAMESPACE