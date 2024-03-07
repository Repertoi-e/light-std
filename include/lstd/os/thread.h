#pragma once

#include "../atomic.h"
#include "../context.h"
#include "../delegate.h"
#include "../memory.h"

LSTD_BEGIN_NAMESPACE

// Blocks the calling thread for at least a given period of time in ms.
// sleep(0) supposedly tells the os to yield execution to another thread.

// @TODO: Rename back to sleep when not clashing with DARWIN api on Mac.
void thread_sleep(u32 ms);

//
// For synchronizing access to shared memory between several threads.
// The mutex can be recursive i.e. a program doesn't deadlock if the thread
// that owns a mutex object calls lock() again.
//
// For recursive mutexes, see _recursive_mutex_.
//
// Scoped lock can be simulated with a defer statement: defer(unlock(...));
//
struct mutex {
    #if OS == WINDOWS || OS == MACOS || OS == LINUX 
    alignas(64) char Handle[64]{};
    #endif
};

mutex create_mutex();
void free_mutex(mutex *m);

// Block the calling thread until a lock on the mutex can
// be obtained. The mutex remains locked until unlock() is called.
void lock(mutex *m);

// Try to lock the mutex.
//
// If the mutex isn't currently locked by any thread, the calling thread
// locks it and the function returns true.
//
// If the mutex is currently locked by another thread, the function returns
// false without blocking (the calling thread continues its execution).
bool try_lock(mutex *m);

// Unlock the mutex.
// If any threads are waiting for the lock on this mutex, one of them will be
// unblocked unless the calling thread has locked this mutex multiple times
// (recursive mutex).
void unlock(mutex *m);

//
// Mutex, but instead of using system level functions it is implemented
// as an atomic spin lock with very low CPU overhead.
//
// fast_mutex is NOT compatible with condition_variable.
// It should also be noted that fast_mutex typically does not provide as
// accurate thread scheduling as the standard mutex does.
//
// Because of the limitations of this object, it should only be used in
// situations where the mutex needs to be locked/unlocked very frequently.
//
struct fast_mutex {
  s32 Lock = 0;
};

// Try to lock the mutex. If it fails, the function will
// return immediately (non-blocking).
//
// Returns true if the lock was acquired
inline bool try_lock(fast_mutex *m) {
  s32 oldLock = atomic_swap(&m->Lock, 1);
  return oldLock == 0;
}

// Block the calling thread until a lock on the mutex can
// be obtained. The mutex remains locked until unlock() is called.
inline void lock(fast_mutex *m) {
  while (!try_lock(m)) thread_sleep(0);
}

// Unlock the mutex.
// If any threads are waiting for the lock on this mutex, one of them will be
// unblocked.
inline void unlock(fast_mutex *m) { atomic_swap(&m->Lock, 0); }

//
// Condition variable.
// @TODO: Example usage
//
struct condition_variable {
    #if OS == WINDOWS
    alignas(64) char Handle[64]{};
    #elif OS == MACOS || OS == LINUX 
    alignas(64) char Handle[48]{};
    #endif
};

// This condition variable won't work until init() is called.
condition_variable create_condition_variable();
void free_condition_variable(condition_variable *c);

namespace internal {
void pre_wait(condition_variable *c);
void do_wait(condition_variable *c, mutex *m);
}

// Wait for the condition.
// The function will block the calling thread until the condition variable
// is woken by notify_one(), notify_all() or a spurious wake up.
inline void wait(condition_variable *c, mutex *m) {
#if OS == WINDOWS
  internal::pre_wait(c);

  // Release the mutex while waiting for the condition (will decrease
  // the number of waiters when done)...
  unlock(m);
  internal::do_wait(c, m);
  lock(m);
#else
  internal::do_wait(c, m);
#endif
}

// Notify one thread that is waiting for the condition.
// If at least one thread is blocked waiting for this condition variable, one
// will be woken up.
//
// Only threads that started waiting prior to this call will be woken up.
void notify_one(condition_variable *c);

// Wake up all threads that are blocked waiting for this condition variable.
//
// Only threads that started waiting prior to this call will be woken up.
void notify_all(condition_variable *c);

struct thread {
  void *Handle = null;
  u32 ThreadID;
};

thread create_and_launch_thread(delegate<void(void *)> function, void *userData = null);

// Wait for the thread to finish
void wait(thread t);

// Terminate the thread without waiting
void terminate(thread t);

// Information to pass to the new thread (what to run).
struct thread_start_info {
  delegate<void(void *)> Function;
  void *UserData = null;

#if OS == WINDOWS
  // We have to make sure the module the thread is executing in
  // doesn't get unloaded while the thread is still doing work.
  void *Module = null;
#endif

  // Pointer to the implicit context in the "parent" thread.
  // We copy its members to the newly created thread.
  const context *ContextPtr = null;
  bool ParentWasUsingTemporaryAllocator;
};

// Call this to init lstd specific thread-local variables
inline void lstd_init_thread() {
  // We are allowed to do this because we are the parents
  *const_cast<allocator *>(&TemporaryAllocator) = {
      arena_allocator, (void *)&TemporaryAllocatorData};

#if defined DEBUG_MEMORY
  debug_memory_init();
#endif

  u64 os_get_current_thread_id(void);
  const_cast<context *>(&Context)->ThreadID = os_get_current_thread_id();
}

LSTD_END_NAMESPACE

#if OS == WINDOWS
#include "windows/thread.h"
#elif OS == MACOS || OS == LINUX 
#include "posix/thread.h"
#elif OS == NO_OS
// No OS (e.g. programming on baremetal).
// Let the user define interfacing with hardware.
#else
#error Implement.
#endif
