#include "common.h"
#include "lstd/internal/common.h"
#include "lstd/memory/delegate.h"

int __cdecl atexit(_PVFV function) {
    LSTD_NAMESPACE::exit_schedule(function);
    return 0;
}