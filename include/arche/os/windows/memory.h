#pragma once

#include "api.h"

//
// Platform specific memory functions.
//

ARCHE_BEGIN_NAMESPACE

inline void *os_allocate_block(s64 size) {
    assert(size < MAX_ALLOCATION_REQUEST);
    return HeapAlloc(GetProcessHeap(), 0, size);
}

inline void os_free_block(void *ptr) { WIN32_CHECK_BOOL(r, HeapFree(GetProcessHeap(), 0, ptr)); }

ARCHE_END_NAMESPACE
