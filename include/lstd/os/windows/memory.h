#pragma once

#include "api.h"

//
// Platform specific memory functions.
//

LSTD_BEGIN_NAMESPACE

inline void *os_allocate_block(s64 size) {
  assert(size < MAX_ALLOCATION_REQUEST);
  return HeapAlloc(GetProcessHeap(), 0, size);
}

inline void os_free_block(void *ptr) {
  WIN32_CHECK_BOOL(r, HeapFree(GetProcessHeap(), 0, ptr));
}

LSTD_END_NAMESPACE
