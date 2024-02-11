#pragma once

#include "../../atomic.h"
#include "../../context.h"
#include "../../delegate.h"
#include "../../memory.h"

#include <pthread.h>
#include <unistd.h> // For usleep

LSTD_BEGIN_NAMESPACE

inline mutex create_mutex() {
  mutex m;

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init((pthread_mutex_t *)m.Handle, &attr);
  return m;
}

inline void free_mutex(mutex *m) {
  auto *p = (pthread_mutex_t *)m->Handle;
  if (p) pthread_mutex_destroy(p);
}

inline void lock(mutex *m) {
  pthread_mutex_lock((pthread_mutex_t *)m->Handle);
}

inline bool try_lock(mutex *m) {
  return pthread_mutex_trylock((pthread_mutex_t *)m->Handle);
}

inline void unlock(mutex *m) {
  pthread_mutex_unlock((pthread_mutex_t *)m->Handle);
}

struct CV_Data {
  pthread_cond_t Cond;
};

inline condition_variable create_condition_variable() {
  condition_variable c;

  auto *data = (CV_Data *)c.Handle;
  pthread_cond_init(&data->Cond, null);
  return c;
}

inline void free_condition_variable(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;
  if (data) {
    pthread_cond_destroy(&data->Cond);
  }
}

void report_warning_no_allocations(string message);

namespace internal {
inline void pre_wait(condition_variable *c) {
}

inline void do_wait(condition_variable *c, mutex *m) {
  auto *data = (CV_Data *)c->Handle;

  int result = pthread_cond_wait(&data->Cond, (pthread_mutex_t *)m->Handle);
  if (result != 0) {
    report_warning_no_allocations("Error in pthread_cond_wait");
  }
}
}

inline void notify_one(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;
  pthread_cond_signal(&data->Cond);
}

inline void notify_all(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;
  pthread_cond_broadcast(&data->Cond);
}

inline void *thread_wrapper_function(void *data) {
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

  return data;
}

inline void wait(thread t) {
  assert(t.ThreadID != Context.ThreadID);  // A thread cannot wait for itself!
  pthread_join((pthread_t) t.Handle, null);
}

inline void terminate(thread t) {
  if (t.Handle) {
    pthread_cancel((pthread_t) t.Handle);
  }
}

inline void thread_sleep(u32 ms) { usleep(ms * 1000); }

LSTD_END_NAMESPACE
