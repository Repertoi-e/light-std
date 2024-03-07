#pragma once

#include "../../atomic.h"
#include "../../context.h"
#include "../../delegate.h"
#include "../../memory.h"

LSTD_BEGIN_NAMESPACE

inline mutex create_mutex() {
  mutex m;
  InitializeCriticalSection((CRITICAL_SECTION *)m.PlatformData.Win32.Handle);
  return m;
}

inline void free_mutex(mutex *m) {
  auto *p = (CRITICAL_SECTION *)m->PlatformData.Win32.Handle;
  if (p) DeleteCriticalSection(p);
}

inline void lock(mutex *m) {
  EnterCriticalSection((CRITICAL_SECTION *)m->PlatformData.Win32.Handle);
}

inline bool try_lock(mutex *m) {
  return TryEnterCriticalSection(
      (CRITICAL_SECTION *)m->PlatformData.Win32.Handle);
}

inline void unlock(mutex *m) {
  LeaveCriticalSection((CRITICAL_SECTION *)m->PlatformData.Win32.Handle);
}

struct CV_Data {
  // Signal and broadcast event HANDLEs.
  HANDLE Events[2];

  // Count of the number of waiters.
  u32 WaitersCount;

  // Serialize access to mWaitersCount.
  CRITICAL_SECTION WaitersCountLock;
};

#define _CONDITION_EVENT_ONE 0
#define _CONDITION_EVENT_ALL 1

inline condition_variable create_condition_variable() {
  condition_variable c;

  auto *data = (CV_Data *)c.Handle;

  data->Events[_CONDITION_EVENT_ONE] = CreateEventW(null, 0, 0, null);
  data->Events[_CONDITION_EVENT_ALL] = CreateEventW(null, 1, 0, null);
  InitializeCriticalSection(&data->WaitersCountLock);

  return c;
}

inline void free_condition_variable(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;
  if (data) {
    CloseHandle(data->Events[_CONDITION_EVENT_ONE]);
    CloseHandle(data->Events[_CONDITION_EVENT_ALL]);
    DeleteCriticalSection(&data->WaitersCountLock);
  }
}

namespace internal {
inline void pre_wait(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

  // Increment number of waiters
  EnterCriticalSection(&data->WaitersCountLock);
  ++data->WaitersCount;
  LeaveCriticalSection(&data->WaitersCountLock);
}

inline void do_wait(condition_variable *c, mutex *m) {
  auto *data = (CV_Data *)c->Handle;

  // Wait for either event to become signaled due to notify_one() or
  // notify_all() being called
  s32 result = WaitForMultipleObjects(2, data->Events, 0, INFINITE);

  // Check if we are the last waiter
  EnterCriticalSection(&data->WaitersCountLock);
  --data->WaitersCount;
  bool lastWaiter =
      result == WAIT_OBJECT_0 + _CONDITION_EVENT_ALL && data->WaitersCount == 0;
  LeaveCriticalSection(&data->WaitersCountLock);

  // If we are the last waiter to be notified to stop waiting, reset the event
  if (lastWaiter) ResetEvent(data->Events[_CONDITION_EVENT_ALL]);
}
}

inline void notify_one(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

  // Are there any waiters?
  EnterCriticalSection(&data->WaitersCountLock);
  bool haveWaiters = data->WaitersCount > 0;
  LeaveCriticalSection(&data->WaitersCountLock);

  // If we have any waiting threads, send them a signal
  if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ONE]);
}

inline void notify_all(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

  // Are there any waiters?
  EnterCriticalSection(&data->WaitersCountLock);
  bool haveWaiters = data->WaitersCount > 0;
  LeaveCriticalSection(&data->WaitersCountLock);

  // If we have any waiting threads, send them a signal
  if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ALL]);
}

inline u32 __stdcall thread_wrapper_function(void *data) {
  auto *ti = (thread_start_info *)data;

  // Copy the parent thread's context
  auto newContext = *ti->ContextPtr;
  // If the parent thread was using the temporary allocator,
  // set the new thread to also use the temporary allocator,
  // but it needs to point to its own temp data (otherwise we are not
  // thread-safe).
  if (ti->ParentWasUsingTemporaryAllocator) {
    newContext.Alloc = TemporaryAllocator;
  }
  OVERRIDE_CONTEXT(newContext);

  lstd_init_thread();

  ti->Function(ti->UserData);  // <--- Call the user function with the user data

#if defined DEBUG_MEMORY
  debug_memory_uninit();
#endif

  // free(ti); // Cross-thread free! @Leak

  ExitThread(0);
  if (ti->Module) FreeLibrary(ti->Module);

  return 0;
}

inline void wait(thread t) {
  assert(t.ThreadID != Context.ThreadID);  // A thread cannot wait for itself!
  WaitForSingleObject(t.Handle, INFINITE);
}

inline void terminate(thread t) {
  if (t.Handle) {
    TerminateThread(t.Handle, 0);
  }
}

inline void thread_sleep(u32 ms) { Sleep(ms); }

LSTD_END_NAMESPACE
