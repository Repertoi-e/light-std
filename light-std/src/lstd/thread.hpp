#pragma once

#include "memory/delegate.hpp"

#include "intrinsics/intrin.hpp"

LSTD_BEGIN_NAMESPACE

namespace thread {

// Mutex class.
// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. The mutex is non-recursive (i.e. a
// program may deadlock if the thread that owns a mutex object calls lock()
// on that object).
struct mutex {
   private:
    byte _Handle[64] = {0};  // pthread_mutex_t
    bool _AlreadyLocked;

   public:
    // pthread_mutex_init(&mHandle, NULL);
    mutex();

    // pthread_mutex_destroy(&mHandle);
    ~mutex();

    mutex(const mutex &) = delete;
    mutex &operator=(const mutex &) = delete;

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.

    // pthread_mutex_lock(&mHandle);
    void lock();

    // Try to lock the mutex. If it fails, the function will
    // return immediately (non-blocking).
    //
    // Returns true if the lock was acquired

    // return (pthread_mutex_trylock(&mHandle) == 0) ? true : false;
    bool try_lock();

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked.

    // pthread_mutex_unlock(&mHandle);
    void unlock();

    friend struct condition_variable;
};

// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. The mutex is recursive (i.e. a thread
// may lock the mutex several times, as long as it unlocks the mutex the same
// number of times).
struct recursive_mutex {
   private:
    byte _Handle[64] = {0};  // pthread_mutex_t
   public:
    // pthread_mutexattr_t attr;
    // pthread_mutexattr_init(&attr);
    // pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    // pthread_mutex_init(&mHandle, &attr);
    recursive_mutex();

    // pthread_mutex_destroy(&mHandle);
    ~recursive_mutex();

    recursive_mutex(const recursive_mutex &) = delete;
    recursive_mutex &operator=(const recursive_mutex &) = delete;

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.

    // pthread_mutex_lock(&mHandle);
    void lock();

    // Try to lock the mutex. If it fails, the function will
    // return immediately (non-blocking).
    //
    // Returns true if the lock was acquired

    // return (pthread_mutex_trylock(&mHandle) == 0) ? true : false;
    bool try_lock();

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked.

    // pthread_mutex_unlock(&mHandle);
    void unlock();

    friend struct condition_variable;
};

// Scoped lock.
// The constructor locks the mutex, and the destructor unlocks the mutex, so
// the mutex will automatically be unlocked when the lock guard goes out of
// scope. Example usage:
//
//      Mutex m;
//      s32 counter;
//
//      void increment()
//      {
//        Scoped_Lock<Mutex> guard(m);
//        ++counter;
//      }
template <class T>
struct scoped_lock {
    using mutex_t = T;

    mutex_t *Mutex = null;

    explicit scoped_lock(mutex_t &mutex) {
        Mutex = &mutex;
        Mutex->lock();
    }

    ~scoped_lock() {
        if (Mutex) Mutex->unlock();
    }
};

// Condition variable.
// This is a signalling object for synchronizing the execution flow for
// several threads. Example usage:
//
//       // Shared data and associated mutex and condition variable objects
//       s32 count;
//       Mutex m;
//       Condition_Variable cond;
//
//       // Wait for the counter to reach a certain number
//       void wait_counter(s32 targetCount)
//       {
//         Scoped_Lock<Mutex> guard(m);
//         while(count < targetCount)
//           cond.wait(m);
//       }
//
//       // Increment the counter, and notify waiting threads
//       void increment()
//       {
//         Scoped_Lock<Mutex> guard(m);
//         ++count;
//         cond.notify_all();
//       }
struct condition_variable {
   private:
#if OS == WINDOWS
    // Implement platform specific Windows code for wait() in these functions,
    // because wait() is templated
    void pre_wait();
    void do_wait();
#endif
    byte _Handle[64] = {0};  // pthread_cond_t
   public:
    // pthread_cond_init(&mHandle, NULL);
    condition_variable();

    // pthread_cond_destroy(&mHandle);
    ~condition_variable();

    condition_variable(const condition_variable &) = delete;
    condition_variable &operator=(const condition_variable &) = delete;

    // Wait for the condition.
    // The function will block the calling thread until the condition variable
    // is woken by notify_one(), notify_all() or a spurious wake up.

    // pthread_cond_wait(&mHandle, &aMutex.mHandle);
    template <class MutexT>
    void wait(MutexT &mutex) {
        pre_wait();

        // Release the mutex while waiting for the condition (will decrease
        // the number of waiters when done)...
        mutex.unlock();
        do_wait();
        mutex.lock();
    }

    // Notify one thread that is waiting for the condition.
    // If at least one thread is blocked waiting for this condition variable, one will be woken up.
    //
    // Only threads that started waiting prior to this call will be woken up.

    // pthread_cond_signal(&mHandle);
    void notify_one();

    // Wake up all threads that are blocked waiting for this condition variable.
    //
    // Only threads that started waiting prior to this call will be woken up.

    //  pthread_cond_broadcast(&mHandle);
    void notify_all();
};

struct thread {
   private:
    mutable mutex _DataMutex;

    // Unique thread ID. Used only on Windows
    u32 _Win32ThreadId = 0;

    // True if this object is not a thread of execution.
    bool _NotAThread = true;

   public:
    // The thread ID is a unique identifier for each thread.
    struct id {
        u64 Value;

        // The default constructed ID is that of thread without a thread of execution.
        id() : Value(0){};
        id(u64 value) : Value(value){};

        id(const id &) = default;
        id &operator=(const id &) = default;

        friend bool operator==(const id &id1, const id &id2) { return (id1.Value == id2.Value); }
        friend bool operator!=(const id &id1, const id &id2) { return (id1.Value != id2.Value); }
        friend bool operator<=(const id &id1, const id &id2) { return (id1.Value <= id2.Value); }
        friend bool operator<(const id &id1, const id &id2) { return (id1.Value < id2.Value); }
        friend bool operator>=(const id &id1, const id &id2) { return (id1.Value >= id2.Value); }
        friend bool operator>(const id &id1, const id &id2) { return (id1.Value > id2.Value); }
    };

    uptr_t Handle;

    // Construct without an associated thread of execution (i.e. non-joinable).
    thread() = default;

    // Thread starting constructor.
    // Construct a thread object with a new thread of execution.
    thread(delegate<void(void *)> function, void *userData);

    thread(const thread &) = delete;
    thread &operator=(const thread &) = delete;

    // If the thread is joinable upon destruction, os_exit_process() will be called, which terminates the process.
    // It is always wise to call join() before deleting a thread object.
    ~thread();

    // Wait for the thread to finish (join execution flows).
    // After calling join(), the thread object is no longer associated with
    // a thread of execution (i.e. it is not joinable, and you may not join
    // with it nor detach from it).
    void join();

    // Check if the thread is joinable.
    // A thread object is joinable if it has an associated thread of execution.
    bool joinable() const;

    // After calling detach(), the thread object is no longer assicated with
    // a thread of execution (i.e. it is not joinable). The thread continues
    // execution without the calling thread blocking, and when the thread
    // ends execution, any owned resources are released.
    void detach();

    // Return the thread ID of a thread object.
    id get_id() const;

   private:
    // This is the internal thread wrapper function.
#if OS == WINDOWS
    static u32 __stdcall wrapper_function(void *data);
#else
    static void *wrapper_function(void *data);
#endif
};

// The number of threads which can possibly execute concurrently.
// This value is useful for determining the optimal number of threads to use for a task.
u32 get_hardware_concurrency();

// The namespace this_thread provides methods for dealing with the calling thread.
namespace this_thread {

// Return the thread ID of the calling thread.

// _pthread_t_to_ID(pthread_self());
/*
static thread::id _pthread_t_to_ID(const pthread_t &aHandle)
{
  static mutex idMapLock;
  static std::map<pthread_t, unsigned long int> idMap;
  static unsigned long int idCount(1);

  lock_guard<mutex> guard(idMapLock);
  if(idMap.find(aHandle) == idMap.end())
    idMap[aHandle] = idCount ++;
  return thread::id(idMap[aHandle]);
}
*/
thread::id get_id();

// Yield execution to another thread.
// Offers the operating system the opportunity to schedule another thread
// that is ready to run on the current processor.

// sched_yield();
void yield();

// Blocks the calling thread for at least a given period of time in ms.
// usleep()
void sleep_for(u32 ms);
}  // namespace this_thread

// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. It is similar to the thread::Mutex object,
// but instead of using system level functions, it is implemented as an atomic
// spin lock with very low CPU overhead.
//
// Fast_Mutex is NOT compatible with Condition_Variable (however,
// it IS compatible with Scoped_Lock). It should also be noted that
// Fast_Mutex typically does not provide as accurate thread scheduling
// as a the standard mutex does.
//
// Because of the limitations of this object, it should only be used in
// situations where the mutex needs to be locked/unlocked very frequently.
class fast_mutex {
   public:
    fast_mutex() : _Lock(0) {}

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.
    void lock() {
        while (!try_lock()) {
            this_thread::yield();
        }
    }

    // Try to lock the mutex. If it fails, the function will
    // return immediately (non-blocking).
    //
    // Returns true if the lock was acquired
    bool try_lock() {
        s32 oldLock;
#if COMPILER == MSVC
        oldLock = _InterlockedExchange(&_Lock, 1);
#else
        asm volatile(
            "movl $1,%%eax\n\t"
            "xchg %%eax,%0\n\t"
            "movl %%eax,%1\n\t"
            : "=m"(Lock), "=m"(oldLock)
            :
            : "%eax", "memory");
#endif
        return oldLock == 0;
    }

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked.
    void unlock() {
#if COMPILER == MSVC
        _InterlockedExchange(&_Lock, 0);
#else
        asm volatile(
            "movl $0,%%eax\n\t"
            "xchg %%eax,%0\n\t"
            : "=m"(Lock)
            :
            : "%eax", "memory");
#endif
    }

   private:
    volatile long _Lock;
};

}  // namespace thread

LSTD_END_NAMESPACE
