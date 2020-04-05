#pragma once

#include "intrin.h"
#include "storage/delegate.h"

struct _RTL_CRITICAL_SECTION;

LSTD_BEGIN_NAMESPACE

namespace thread {

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

// Mutex class.
// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. The mutex is non-recursive (i.e. a
// program may deadlock if the thread that owns a mutex object calls lock()
// on that object).
struct mutex : non_copyable, non_movable, non_assignable {
    union {
        struct alignas(64) {
            char Handle[40]{};
        } Win32;
    } PlatformData{};
    volatile bool AlreadyLocked;

    // pthread_mutex_init(&mHandle, NULL);
    mutex();

    // pthread_mutex_destroy(&mHandle);
    ~mutex();

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
struct recursive_mutex : non_copyable, non_movable, non_assignable {
    union {
        struct alignas(64) {
            char Handle[40]{};
        } Win32;
    } PlatformData{};

    // pthread_mutexattr_t attr;
    // pthread_mutexattr_init(&attr);
    // pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    // pthread_mutex_init(&mHandle, &attr);
    recursive_mutex();

    // pthread_mutex_destroy(&mHandle);
    ~recursive_mutex();

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
//      mutex m;
//      s32 counter;
//
//      void increment()
//      {
//        scoped_lock<mutex> guard(&m);
//        ++counter;
//      }
template <typename T>
struct scoped_lock {
    using mutex_t = T;

    mutex_t *Mutex = null;

    explicit scoped_lock(mutex_t *mutex) {
        if (mutex) {
            Mutex = mutex;
            Mutex->lock();
        }
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
//       mutex m;
//       condition_variable cond;
//
//       // Wait for the counter to reach a certain number
//       void wait_counter(s32 targetCount)
//       {
//         scoped_lock<mutex> guard(&m);
//         while(count < targetCount)
//           cond.wait(m);
//       }
//
//       // Increment the counter, and notify waiting threads
//       void increment()
//       {
//         scoped_lock<mutex> guard(&m);
//         ++count;
//         cond.notify_all();
//       }
struct condition_variable : non_copyable, non_movable, non_assignable {
   private:
#if OS == WINDOWS
    // Implement platform specific Windows code for wait() in these functions,
    // because wait() is templated
    void pre_wait();
    void do_wait();
#endif
   public:
    char Handle[64] = {0};  // pthread_cond_t

    // pthread_cond_init(&mHandle, NULL);
    condition_variable();

    // pthread_cond_destroy(&mHandle);
    ~condition_variable();

    // Wait for the condition.
    // The function will block the calling thread until the condition variable
    // is woken by notify_one(), notify_all() or a spurious wake up.

    // pthread_cond_wait(&mHandle, &aMutex.mHandle);
    template <typename MutexT>
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

struct thread : non_copyable, non_movable, non_assignable {
    mutable mutex DataMutex;

    // Unique thread ID. Used only on Windows
    u32 Win32ThreadId = 0;

    uptr_t Handle;

    // True if this object is not a thread of execution.
    bool NotAThread = true;

    // Construct without an associated thread of execution (i.e. non-joinable).
    thread() = default;

    // Thread starting constructor.
    // Construct a thread object with a new thread of execution.
    thread(delegate<void(void *)> function, void *userData) { start(function, userData); }

    // If the thread is joinable upon destruction, os_exit_process() will be called, which terminates the process.
    // It is always wise to call join() before deleting a thread object.
    ~thread();

    void start(delegate<void(void *)> function, void *userData);

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

// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. It is similar to the thread::mutex object,
// but instead of using system level functions, it is implemented as an atomic
// spin lock with very low CPU overhead.
//
// Fast_Mutex is NOT compatible with condition_variable (however,
// it IS compatible with scoped_lock). It should also be noted that
// Fast_Mutex typically does not provide as accurate thread scheduling
// as a the standard mutex does.
//
// Because of the limitations of this object, it should only be used in
// situations where the mutex needs to be locked/unlocked very frequently.
struct fast_mutex {
   public:
    fast_mutex() : _Lock(0) {}

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.
    // Defined in *platform*_thread.cpp although platform inspecific because we include this file in context.h
    void lock();

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

template <typename T>
struct future : non_copyable, non_movable, non_assignable {
    using data_t = T;

    thread Thread;

    future() = default;

    // Future starting constructor.
    // Construct a future object and begin execution.
    future(delegate<void(void *)> function, void *userData) { start(function, userData); }

    ~future() { close(); }

    void start(delegate<void(void *)> function, void *userData) { Thread.start(function, userData); }

    void close() {
        assert(Thread.joinable() && "Thread not started");
        Thread.join();
    }
};

template <typename T, typename MutexType = mutex>
struct promise : non_copyable, non_movable, non_assignable {
    using data_t = T;

    condition_variable Cond;
    MutexType Mutex;

    // Don't access this without locking the mutex (use copy/set_result())
    T Result;
    bool Done = false;

    void get_result(T *out) {
        scoped_lock<MutexType> guard(&Mutex);
        while (!Done) {
            Cond.wait(&Mutex);
        }
        clone(out, Result);
        return Result;
    }

    void set_result(T result) {
        scoped_lock<MutexType> guard(&Mutex);
        Done = true;
        clone(&Result, result);
        Cond.notify_one();
    }

    void move_and_set_result(T *result) {
        scoped_lock<MutexType> guard(&Mutex);
        Done = true;
        move(&Result, result);
        Cond.notify_one();
    }

    bool is_done() {
        scoped_lock<MutexType> guard(&Mutex);
        return Done;
    }
};

}  // namespace thread

// The number of threads which can possibly execute concurrently.
// This value is useful for determining the optimal number of threads to use for a task.
u32 os_get_hardware_concurrency();

LSTD_END_NAMESPACE
