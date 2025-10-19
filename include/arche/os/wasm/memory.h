#pragma once

#include "../../common.h"
#include "../../fmt.h"
#include "../../string.h"

#include <stdlib.h>

//
// Platform specific memory functions.
//

ARCHE_BEGIN_NAMESPACE

mark_as_leak inline void *os_allocate_block(s64 size) {
    assert(size < MAX_ALLOCATION_REQUEST);
    
    // Emscripten provides malloc/free that operate on the linear heap
    void *ptr = malloc(size);
    if (!ptr) {
        // Could happen if we run out of WASM memory
        return null;
    }
    return ptr;
}

inline void os_free_block(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

ARCHE_END_NAMESPACE
