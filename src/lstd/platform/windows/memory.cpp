#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/lstd.h"
#include "lstd/os/windows/api.h"     // Declarations of Win32 functions
#include "lstd/os/windows/common.h"  // Declarations of Win32 functions

LSTD_BEGIN_NAMESPACE

#define S ((win32_memory_state *)&Win32MemoryState[0])

void *win32_persistent_alloc(allocator_mode mode, void *context, s64 size,
                             void *oldMemory, s64 oldSize, u64 options) {
  auto *data = (tlsf_allocator_data *)context;

  lock(&S->PersistentAllocMutex);
  defer(unlock(&S->PersistentAllocMutex));

  if (mode == allocator_mode::ALLOCATE &&
      ((u64)(size * 2) > PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE)) {
    platform_report_warning(
        "Large allocation requested for the platform persistent allocator; "
        "querying the OS for memory directly");
    return create_persistent_alloc_page(size);
  }

  auto *result =
      tlsf_allocator(mode, context, size, oldMemory, oldSize, options);
  if (mode == allocator_mode::ALLOCATE && !result) {
    platform_report_warning(
        "Not enough memory in the persistent allocator; adding another pool");

    void *block =
        create_persistent_alloc_page(PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
    tlsf_allocator_add_pool(&S->PersistentAllocData, block,
                            PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);

    result = tlsf_allocator(allocator_mode::ALLOCATE, context, size, null, 0,
                            options);
    assert(result);
  }
  return result;
}

void platform_init_allocators() {
  S->TempAllocMutex = create_mutex();
  S->PersistentAllocMutex = create_mutex();

  S->TempAlloc = {win32_temp_alloc, &S->TempAllocData};

  S->TempAllocData.Block = null;

  create_new_temp_storage_block(PLATFORM_TEMPORARY_STORAGE_STARTING_SIZE);

  S->PersistentAllocBasePage = null;
  S->PersistentAlloc = {win32_persistent_alloc, &S->PersistentAllocData};

  void *block =
      create_persistent_alloc_page(PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
  tlsf_allocator_add_pool(&S->PersistentAllocData, block,
                          PLATFORM_PERSISTENT_STORAGE_STARTING_SIZE);
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
