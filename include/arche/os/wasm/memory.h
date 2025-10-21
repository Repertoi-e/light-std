#pragma once

#include "../../common.h"
#include "../../fmt.h"
#include "../../string.h"

#include <emscripten/heap.h>

#ifdef __EMSCRIPTEN_TRACING__
void emscripten_memprof_sbrk_grow(intptr_t old, intptr_t new);
#else
#define emscripten_memprof_sbrk_grow(...) ((void)0)
#endif

extern size_t __heap_base;

inline static uintptr_t sbrk_val = (uintptr_t)&__heap_base;

inline uintptr_t* emscripten_get_sbrk_ptr() {
#ifdef __PIC__
  // In relocatable code we may call emscripten_get_sbrk_ptr() during startup,
  // potentially *before* the setup of the dynamically-linked __heap_base, when
  // using SAFE_HEAP. (SAFE_HEAP instruments *all* memory accesses, so even the
  // code doing dynamic linking itself ends up instrumented, which is why we can
  // get such an instrumented call before sbrk_val has its proper value.)
  if (sbrk_val == 0) {
    sbrk_val = (uintptr_t)&__heap_base;
  }
#endif
  return &sbrk_val;
}

// Enforce preserving a minimal alignof(maxalign_t) alignment for sbrk.
#define SBRK_ALIGNMENT (__alignof__(max_align_t))

#ifdef __EMSCRIPTEN_SHARED_MEMORY__
#define READ_SBRK_PTR(sbrk_ptr) (__c11_atomic_load((_Atomic(uintptr_t)*)(sbrk_ptr), __ATOMIC_SEQ_CST))
#else
#define READ_SBRK_PTR(sbrk_ptr) (*(sbrk_ptr))
#endif

inline void *_sbrk64(int64_t increment) {
  if (increment >= 0) {
    increment = (increment + (SBRK_ALIGNMENT-1)) & ~((int64_t)SBRK_ALIGNMENT-1);
  } else {
    increment = -(-increment & ~((int64_t)SBRK_ALIGNMENT-1));
  }

  uintptr_t *sbrk_ptr = (uintptr_t*)emscripten_get_sbrk_ptr();

  // To make sbrk thread-safe, implement a CAS loop to update the
  // value of sbrk_ptr.
  while (1) {
    uintptr_t old_brk = READ_SBRK_PTR(sbrk_ptr);
    int64_t new_brk64 = (int64_t)old_brk + increment;
    uintptr_t new_brk = (uintptr_t)new_brk64;
    // Check for a) an over/underflow, which would indicate that we are
    // allocating over maximum addressable memory. and b) if necessary,
    // increase the WebAssembly Memory size, and abort if that fails.
    if (new_brk < 0 || new_brk64 != (int64_t)new_brk
     || (new_brk > emscripten_get_heap_size() && !emscripten_resize_heap(new_brk))) {
      errno = ENOMEM;
      return (void*)-1;
    }
#ifdef __EMSCRIPTEN_SHARED_MEMORY__
    // Attempt to update the dynamic top to new value. Another thread may have
    // beat this one to the update, in which case we will need to start over
    // by iterating the loop body again.
    uintptr_t expected = old_brk;

    __c11_atomic_compare_exchange_strong((_Atomic(uintptr_t)*)sbrk_ptr,
      &expected, new_brk, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);

    if (expected != old_brk) continue; // CAS failed, another thread raced in between.
#else
    *sbrk_ptr = new_brk;
#endif

    emscripten_memprof_sbrk_grow(old_brk, new_brk);
    return (void*)old_brk;
  }
}

inline void *sbrk(ptrdiff_t increment_)
{
#if defined(__wasm64__) // TODO || !defined(wasm2gb)
  // In the correct https://linux.die.net/man/2/sbrk spec, sbrk() parameter is
  // intended to be treated as signed, meaning that it is not possible in a
  // 32-bit program to sbrk alloc (or dealloc) more than 2GB of memory at once.

  // Treat sbrk() parameter as signed.
  return _sbrk64((int64_t)increment_);
#else
  // BUG: Currently the Emscripten test suite codifies expectations that sbrk()
  // values passed to this function are to be treated as unsigned, which means
  // that in 2GB and 4GB build modes, it is not possible to shrink memory.
  // To satisfy that mode, treat sbrk() parameters in 32-bit builds as unsigned.
  // https://github.com/emscripten-core/emscripten/issues/25138

  // Treat sbrk() parameter as unsigned.
  return _sbrk64((int64_t)(uintptr_t)increment_);
#endif
}

//
// Platform specific memory functions.
//

ARCHE_BEGIN_NAMESPACE

mark_as_leak inline void *os_allocate_block(s64 size) {
    assert(size < MAX_ALLOCATION_REQUEST);
    
    // Emscripten provides malloc/free that operate on the linear heap
    void *ptr = sbrk(size);
    if (!ptr) {
        // Could happen if we run out of WASM memory
        return null;
    }
    return ptr;
}

inline void os_free_block(void *ptr) {
    // No-op on WASM
}

ARCHE_END_NAMESPACE
