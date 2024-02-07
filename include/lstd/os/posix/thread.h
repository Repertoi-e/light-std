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
  pthread_mutex_init((pthread_mutex_t *)m.Handle, null);
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
  pthread_mutex_t WaitersCountLock;
  u32 WaitersCount;
};

inline condition_variable create_condition_variable() {
  condition_variable c;

  auto *data = (CV_Data *)c.Handle;

    pthread_cond_init(&data->Cond, NULL);
    pthread_mutex_init(&data->WaitersCountLock, NULL);
    data->WaitersCount = 0;

  return c;
}

inline void free_condition_variable(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;
  if (data) {
    pthread_cond_destroy(&data->Cond);
    pthread_mutex_destroy(&data->WaitersCountLock);
  }
}

void report_warning_no_allocations(string message);

namespace internal {
inline void pre_wait(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

  // Increment number of waiters
  pthread_mutex_lock(&data->WaitersCountLock);
  data->WaitersCount++;
  pthread_mutex_unlock(&data->WaitersCountLock);
}

inline void do_wait(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

  // Wait for either event to become signaled due to notify_one() or
  // notify_all() being called
  pthread_mutex_lock(&data->WaitersCountLock);
  while (data->WaitersCount > 0) {
        int result = pthread_cond_wait(&data->Cond, &data->WaitersCountLock);
        if (result != 0) {
            report_warning_no_allocations("Error in pthread_cond_wait");
        }
    }
  --data->WaitersCount;
  pthread_mutex_unlock(&data->WaitersCountLock);
}
}

inline void notify_one(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

    pthread_mutex_lock(&data->WaitersCountLock);
    if (data->WaitersCount > 0) {
        pthread_cond_signal(&data->Cond);
    }
    pthread_mutex_unlock(&data->WaitersCountLock);
}

inline void notify_all(condition_variable *c) {
  auto *data = (CV_Data *)c->Handle;

    pthread_mutex_lock(&data->WaitersCountLock);
    if (data->WaitersCount > 0) {
        pthread_cond_broadcast(&data->Cond);
    }
    pthread_mutex_unlock(&data->WaitersCountLock);
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

  pthread_exit(0);

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
