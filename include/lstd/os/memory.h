#pragma once

#include "../common.h"
#include "../fmt.h"
#include "../string.h"
#include "thread.h"

//
// Platform specific memory functions.
//

LSTD_BEGIN_NAMESPACE

// Allocates memory by calling the OS directly
mark_as_leak void *os_allocate_block(s64 size);

// Frees a memory block allocated by os_allocate_block()
void os_free_block(void *ptr);

struct platform_memory_state {
  // Used to store global state (e.g. cached command-line arguments/env
  // variables or directories), a tlsf allocator
  allocator PersistentAlloc;
  tlsf_allocator_data PersistentAllocData;

  // Stores blocks that have been added as pools for the tlsf allocator or a
  // large allocation which was handled by _os_allocate_block()_.
  struct persistent_alloc_page {
    persistent_alloc_page *Next;
  };
  persistent_alloc_page *PersistentAllocBasePage;

  mutex PersistentAllocMutex;

  //
  // We don't use the default thread-local temporary allocator because we don't
  // want to mess with the user's memory.
  //
  // Used for temporary storage (e.g. converting strings from utf8 to wchar for
  // windows calls or null-terminated for posix calls). Memory returned is only 
  // guaranteed to be valid until the next temporary alloc call, because we call 
  // free_all if we don't have enough space for the
  // allocation. See note above _platform_temp_alloc()_.
  allocator TempAlloc;
  arena_allocator_data TempAllocData;

  mutex TempAllocMutex;  // @TODO: Remove
};

// :GlobalStateNoConstructors:
alignas(64) inline byte PlatformMemoryState[sizeof(platform_memory_state)];

// Short-hand macro for sanity
#define S ((platform_memory_state *)&PlatformMemoryState[0])

// @TODO: Print call stack
inline void platform_report_warning(
    string message, source_location loc = source_location::current()) {
  print(">>> {!YELLOW}Platform warning{!} {}:{} (in function: {}): {}.\n",
        loc.file_name(), loc.line(), loc.function_name(), message);
}

inline void platform_report_error(
    string message, source_location loc) {
  print(">>> {!RED}Platform error{!} {}:{} (in function: {}): {}.\n", loc.file_name(),
        loc.line(), loc.function_name(), message);
}

inline void create_new_temp_storage_block(s64 size) {
  if (S->TempAllocData.Block) {
    os_free_block(S->TempAllocData.Block);
  } 
  S->TempAllocData.Block = os_allocate_block(size);

  S->TempAllocData.Size = size;
  S->TempAllocData.Used = 0;
}

// An extension to the arena allocator. Calls free_all when not enough space.
// Because we are not running e.g. a game there is no clear point at which to
// free_all the temporary allocator, that's why we assume that no allocation
// made with TempAlloc should persist beyond the next allocation.
//
// Note: This allocator doesn't work 100% in a multithreaded situation because
// free_all could be called at an arbitrary point in time.
//
// @TODO: Replace calls to the temporary alloc with calls to the persient alloc.
// They are both are fast. Obviously this allocator is faster, but there is no
// clear point as to when we can safely call free_all.. In a game free_all is
// called at the end of each frame which means there is no problem there since
// temporary allocations shouldn't persist until the next frame.
inline void *platform_temp_alloc(allocator_mode mode, void *context, s64 size,
                              void *oldMemory, s64 oldSize, u64 options) {
  auto *data = (arena_allocator_data *)context;

  lock(&S->TempAllocMutex);
  defer(unlock(&S->TempAllocMutex));

  auto *result =
      arena_allocator(mode, context, size, oldMemory, oldSize, options);
  if (!result && mode == allocator_mode::ALLOCATE) {
    if (size < S->TempAllocData.Size) {
      // If we couldn't allocate but the temporary storage block has enough
      // space, we just call free_all
      free_all(S->TempAlloc);
      result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0,
                               options);
      // Problem in multithreaded! See note above.
    } else {
      // If we try to allocate a block with size bigger than the temporary
      // storage block, we make a new, larger temporary storage block.

      platform_report_warning(
          "Not enough memory in the temporary allocator "
          "block; reallocating the pool");

      os_free_block(S->TempAllocData.Block);
      create_new_temp_storage_block(size * 2);

      result = arena_allocator(allocator_mode::ALLOCATE, context, size, null, 0,
                               options);
    }
  }

  return result;
}

// Returns a pointer to the usable memory
inline void *create_persistent_alloc_page(s64 size) {
  void *result = os_allocate_block(
      size + sizeof(platform_memory_state::persistent_alloc_page));

  auto *p = (platform_memory_state::persistent_alloc_page *)result;

  p->Next = S->PersistentAllocBasePage;
  if (!S->PersistentAllocBasePage) {
    S->PersistentAllocBasePage = p;
  }

  return (void *)(p + 1);
}

inline void *platform_persistent_alloc(allocator_mode mode, void *context,
                                    s64 size, void *oldMemory, s64 oldSize,
                                    u64 options);

inline allocator platform_get_persistent_allocator() { return S->PersistentAlloc; }
inline allocator platform_get_temporary_allocator() { return S->TempAlloc; }

void platform_init_allocators();

inline void platform_uninit_allocators() {
  lock(&S->TempAllocMutex);
  lock(&S->PersistentAllocMutex);

  // Free all pages (pools and big allocations)
  auto *p = S->PersistentAllocBasePage;
  while (p) {
    auto *o = p;
    p = p->Next;
    os_free_block(o);
  }
  S->PersistentAllocBasePage = null;

  // Free temporary storage arena
  os_free_block(S->TempAllocData.Block);
  S->TempAllocData.Size = 0;
  S->TempAllocData.Used = 0;

  // Release mutexes

  unlock(&S->TempAllocMutex);
  unlock(&S->PersistentAllocMutex);

  free_mutex(&S->TempAllocMutex);
  free_mutex(&S->PersistentAllocMutex);
}

LSTD_END_NAMESPACE

#if OS == WINDOWS
#include "windows/memory.h"
#elif OS == MACOS || OS == LINUX 
#include "posix/memory.h"
#elif OS == NO_OS
// No OS (e.g. programming on baremetal).
// Let the user define interfacing with hardware.
#else
#error Implement.
#endif

#undef S
