#include "common.h"
#include "lstd/common.h"

import lstd.os;

int __cdecl atexit(_PVFV function) {
    LSTD_NAMESPACE::exit_schedule(function);
    return 0;
}
