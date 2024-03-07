#pragma once

#include "../../common.h"
#include "../../fmt.h"
#include "../../string.h"
#include "thread.h"

#include <unistd.h>
#include <sys/mman.h>

//
// Platform specific memory functions.
//

LSTD_BEGIN_NAMESPACE

mark_as_leak inline void *os_allocate_block(s64 size) {
  assert(size < MAX_ALLOCATION_REQUEST);

  void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  return ptr != MAP_FAILED ? ptr : nullptr;
}

inline void os_free_block(void *ptr) {
  if (munmap(ptr, 0) == -1) {
    // Handle error 
  }
}

LSTD_END_NAMESPACE
