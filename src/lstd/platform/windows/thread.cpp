#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/memory.h"
#include "lstd/os.h"              // Declarations of Win32 functions
#include "lstd/os/windows/api.h"  // Declarations of Win32 functions

extern "C" IMAGE_DOS_HEADER __ImageBase;

LSTD_BEGIN_NAMESPACE

thread create_and_launch_thread(delegate<void(void *)> function,
                                void *userData) {
  thread t;

  // Passed to the thread wrapper, which will eventually free it
  // @TODO @Speed @Memory Fragmentation! We should have a dedicated pool
  // allocator for thread_start_info because threads could be created/destroyed
  // very often.
  auto *ti =
      malloc<thread_start_info>({.Alloc = platform_get_persistent_allocator()});
  ti->Function = function;
  ti->UserData = userData;
  ti->ContextPtr = &Context;
  ti->ParentWasUsingTemporaryAllocator = Context.Alloc == TemporaryAllocator;

  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                     (LPCWSTR)&__ImageBase, &ti->Module);
  auto handle =
      CreateThread(null, 0, (LPTHREAD_START_ROUTINE)thread_wrapper_function, ti,
                   0, &t.ThreadID);
  t.Handle = (void *)handle;

  if (!handle || (void *)handle == INVALID_HANDLE_VALUE) {
    // We free this directly since thread wrapper never even ran
    free(ti);
  }

  return t;
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
