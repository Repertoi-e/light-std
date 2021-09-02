#pragma once

#include "memory/delegate.h"

LSTD_BEGIN_NAMESPACE

namespace thread {

// The thread ID is a unique identifier for each thread.
struct id {
    u64 Value;

    id() {
    }

    id(u64 value)
        : Value(value) {
    };

    friend bool operator==(const id &id1, const id &id2) { return id1.Value == id2.Value; }
    friend bool operator!=(const id &id1, const id &id2) { return id1.Value != id2.Value; }
    friend bool operator<=(const id &id1, const id &id2) { return id1.Value <= id2.Value; }
    friend bool operator<(const id &id1, const id &id2) { return id1.Value < id2.Value; }
    friend bool operator>=(const id &id1, const id &id2) { return id1.Value >= id2.Value; }
    friend bool operator>(const id &id1, const id &id2) { return id1.Value > id2.Value; }
};

// Mutex class.
// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. The mutex can be recursive (i.e. a
// program doesn't deadlock if the thread that owns a mutex object calls lock()
// on that object).
struct mutex : non_assignable {
    union {
        struct alignas(64) {
            char Handle[40]{};
        } Win32;
    } PlatformData{};

    // This mutex won't work until init() is called.
    //
    // @POSIX pthread_mutex_init(&mHandle, NULL);
    void init();

    // @POSIX pthread_mutex_destroy(&mHandle);
    void release();

    // We no longer use destructors for releasing handles/memory etc.
    // ~mutex();

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.
    //
    // If the mutex is currently locked by the calling thread, the function asserts false and produces a deadlock.
    // See recursive_mutex for a mutex type that allows multiple locks from the same thread.
    //
    // @POSIX pthread_mutex_lock(&mHandle);
    void lock();

    // Try to lock the mutex.
    //
    // If the mutex isn't currently locked by any thread, the calling thread locks it and owns it until unlock() is called. The function returns true.
    // If the mutex is currently locked by another thread, the function fails and returns false, without blocking
    // (the calling thread continues its execution).
    //
    // If the mutex is currently locked by the calling thread, the function asserts false and produces a deadlock.
    // See recursive_mutex for a mutex type that allows multiple locks from the same thread.
    //
    // @POSIX return (pthread_mutex_trylock(&mHandle) == 0) ? true : false;
    bool try_lock();

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked.

    // @POSIX pthread_mutex_unlock(&mHandle);
    void unlock();

    friend struct condition_variable;
};

// @Pedantic We want to make sure the user doesn't clone things that don't make sense.
inline mutex *clone(mutex *dest, const mutex &src) {
    assert(false && "We don't deep copy mutexes");
    return null;
}

//
// @Cleanup: We don't need this with defer.
// 
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
struct scoped_lock : non_assignable {
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

// @Pedantic We want to make sure the user doesn't clone things that don't make sense.
template <typename T>
scoped_lock<T> *clone(scoped_lock<T> *dest, const scoped_lock<T> &src) {
    assert(false && "We don't deep copy scoped locks");
    return null;
}

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
struct condition_variable : non_assignable {
private:
#if OS == WINDOWS
    // Implement platform specific Windows code for wait() in these functions,
    // because wait() is templated
    void pre_wait();
    void do_wait();
#endif
public:
    char Handle[64] = {0}; // pthread_cond_t

    // This condition variable won't work until init() is called.
    //
    // @POSIX pthread_cond_init(&mHandle, NULL);
    void init();

    // We no longer use destructors for releasing handles/memory etc.
    // ~condition_variable();

    // @POSIX pthread_cond_destroy(&mHandle);
    void release();

    // Wait for the condition.
    // The function will block the calling thread until the condition variable
    // is woken by notify_one(), notify_all() or a spurious wake up.

    // @POSIX pthread_cond_wait(&mHandle, &aMutex.mHandle);
    template <typename MutexT>
    void wait(MutexT *mutex) {
        pre_wait();

        // Release the mutex while waiting for the condition (will decrease
        // the number of waiters when done)...
        mutex->unlock();
        do_wait();
        mutex->lock();
    }

    // Notify one thread that is waiting for the condition.
    // If at least one thread is blocked waiting for this condition variable, one will be woken up.
    //
    // Only threads that started waiting prior to this call will be woken up.

    // @POSIX pthread_cond_signal(&mHandle);
    void notify_one();

    // Wake up all threads that are blocked waiting for this condition variable.
    //
    // Only threads that started waiting prior to this call will be woken up.

    // @POSIX pthread_cond_broadcast(&mHandle);
    void notify_all();
};

// @Pedantic We want to make sure the user doesn't clone things that don't make sense.
inline condition_variable *clone(condition_variable *dest, const condition_variable &src) {
    assert(false && "We don't deep copy condition variables");
    return null;
}

struct thread : non_assignable {
    // Don't read this directly, use atomic operations - atomic_compare_exchange_pointer(&Handle, null, null).
    // You probably don't want this anyway?
    void *Handle = null;

    // Non-starting constructor.
    thread() {
    }

    void init_and_launch(const delegate<void(void *)> &function, void *userData = null);

    void wait();

    // Terminate the thread without waiting execution
    void terminate();

    id get_id() const;

private:
    // Unique thread ID. Used only on Windows.
    u32 Win32ThreadId = 0;

    // This is the internal thread wrapper function.
#if OS == WINDOWS
    static u32 __stdcall wrapper_function(void *data);
#else
    static void *wrapper_function(void *data);
#endif
};

// @Pedantic We want to make sure the user doesn't clone things that don't make sense.
inline thread *clone(thread *dest, const thread &src) {
    assert(false && "We don't deep copy threads");
    return null;
}

// This is a mutual exclusion object for synchronizing access to shared
// memory areas for several threads. It is similar to the thread::mutex object,
// but instead of using system level functions, it is implemented as an atomic
// spin lock with very low CPU overhead.
//
// fast_mutex is NOT compatible with condition_variable.
// It should also be noted that fast_mutex typically does not provide as
// accurate thread scheduling as the standard mutex does.
//
// Because of the limitations of this object, it should only be used in
// situations where the mutex needs to be locked/unlocked very frequently.
struct fast_mutex : non_assignable {
    s32 Lock = 0;

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.
    // Defined in *platform*_thread.cpp although platform inspecific because we include this file in context.h
    void lock();

    // Try to lock the mutex. If it fails, the function will
    // return immediately (non-blocking).
    //
    // Returns true if the lock was acquired
    bool try_lock() {
        s32 oldLock = atomic_swap(&Lock, 1);
        return oldLock == 0;
    }

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked.
    void unlock() {
        atomic_swap(&Lock, 0);
    }
};

// @Pedantic We want to make sure the user doesn't clone things that don't make sense.
inline fast_mutex *clone(fast_mutex *dest, const fast_mutex &src) {
    assert(false && "We don't deep copy mutexes");
    return null;
}

// Blocks the calling thread for at least a given period of time in ms.
// sleep(0) supposedly tells the os to yield execution to another thread.
void sleep(u32 ms);

} // namespace thread

// The number of threads which can execute concurrently on the current hardware (may be different from the number of cores because of hyperthreads).
u32 os_get_hardware_concurrency();

LSTD_END_NAMESPACE
