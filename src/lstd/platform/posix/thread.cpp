#include "lstd/common.h"

#if OS == MACOS || OS == LINUX 

#include "lstd/memory.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

thread create_and_launch_thread(delegate<void(void *)> function,
                                void *userData) {
  thread t;

  // Passed to the thread wrapper, which will eventually free it
  // @TODO @Speed @Memory Fragmentation! We should have a dedicated pool
  // allocator for thread_start_info when threads could be created/destroyed very often.
  auto *ti = malloc<thread_start_info>({.Alloc = platform_get_persistent_allocator(), .Options = LEAK /*TEMP*/});
  ti->Function = function;
  ti->UserData = userData;
  ti->ContextPtr = &Context;
  ti->ParentWasUsingTemporaryAllocator = Context.Alloc == TemporaryAllocator;

   // Create the thread
    int result = pthread_create((pthread_t *) &t.Handle, null, thread_wrapper_function, (void *)ti);
    if (result != 0) {
        // Error handling: Handle failure to create thread
        report_warning_no_allocations("Failed pthread_create");
        
        // We free this directly since thread wrapper never even ran
        free(ti);
        t.Handle = null;
    }

    t.ThreadID = (u64) t.Handle;

  return t;
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
