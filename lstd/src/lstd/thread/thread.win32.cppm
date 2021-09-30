module;

#include "lstd/platform/windows.h"  // Declarations of Win32 functions

export module lstd.thread.win32;

export import lstd.delegate;
export import lstd.context;
export import lstd.memory;

LSTD_BEGIN_NAMESPACE

export {
    // Blocks the calling thread for at least a given period of time in ms.
    // sleep(0) supposedly tells the os to yield execution to another thread.
    void sleep(u32 ms);

    // For synchronizing access to shared memory between several threads.
    // The mutex can be recursive i.e. a program doesn't deadlock if the thread
    // that owns a mutex object calls lock() again.
    //
    // For recursive mutexes, see _recursive_mutex_.
    //
    // Scoped lock can be done with a defer statement.
    struct mutex {
        union {
            struct alignas(64) {
                char Handle[40]{};
            } Win32;
        } PlatformData{};
    };

    mutex create_mutex();
    void free_mutex(mutex * m);

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.
    void lock(mutex * m);

    // Try to lock the mutex.
    //
    // If the mutex isn't currently locked by any thread, the calling thread
    // locks it and the function returns true.
    //
    // If the mutex is currently locked by another thread, the function returns false
    // without blocking (the calling thread continues its execution).
    bool try_lock(mutex * m);

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked
    // unless the calling thread has locked this mutex multiple times (recursive mutex).
    void unlock(mutex * m);

    // Mutex, but instead of using system level functions it is implemented
    // as an atomic spin lock with very low CPU overhead.
    //
    // fast_mutex is NOT compatible with condition_variable.
    // It should also be noted that fast_mutex typically does not provide as
    // accurate thread scheduling as the standard mutex does.
    //
    // Because of the limitations of this object, it should only be used in
    // situations where the mutex needs to be locked/unlocked very frequently.
    struct fast_mutex {
        s32 Lock = 0;
    };

    // Try to lock the mutex. If it fails, the function will
    // return immediately (non-blocking).
    //
    // Returns true if the lock was acquired
    bool try_lock(fast_mutex * m) {
        s32 oldLock = atomic_swap(&m->Lock, 1);
        return oldLock == 0;
    }

    // Block the calling thread until a lock on the mutex can
    // be obtained. The mutex remains locked until unlock() is called.
    void lock(fast_mutex * m) {
        while (!try_lock(m)) sleep(0);
    }

    // Unlock the mutex.
    // If any threads are waiting for the lock on this mutex, one of them will be unblocked.
    void unlock(fast_mutex * m) {
        atomic_swap(&m->Lock, 0);
    }

    //
    // Condition variable.
    // @TODO: Example usage
    struct condition_variable {
#if OS == WINDOWS
        void pre_wait();
        void do_wait();
#endif

        char Handle[64] = {0};
    };

    // This condition variable won't work until init() is called.
    condition_variable create_condition_variable();
    void free_condition_variable(condition_variable * c);

    // Wait for the condition.
    // The function will block the calling thread until the condition variable
    // is woken by notify_one(), notify_all() or a spurious wake up.
    template <typename MutexT>
    void wait(condition_variable * c, MutexT * m) {
#if OS == WINDOWS
        c->pre_wait();
#endif

        // Release the mutex while waiting for the condition (will decrease
        // the number of waiters when done)...
        unlock(m);

#if OS == WINDOWS
        c->do_wait();
#endif
        lock(m);
    }

    // Notify one thread that is waiting for the condition.
    // If at least one thread is blocked waiting for this condition variable, one will be woken up.
    //
    // Only threads that started waiting prior to this call will be woken up.
    void notify_one(condition_variable * c);

    // Wake up all threads that are blocked waiting for this condition variable.
    //
    // Only threads that started waiting prior to this call will be woken up.
    void notify_all(condition_variable * c);

    struct thread {
        // Don't read this directly, use an atomic operation:   atomic_compare_exchange_pointer(&Handle, null, null).
        void *Handle = null;
        u32 ThreadID;
    };

    thread create_and_launch_thread(const delegate<void(void *)> &function, void *userData = null);

    // Wait for the thread to finish
    void wait(thread t);

    // Terminate the thread without waiting
    void terminate(thread t);
}

mutex create_mutex() {
    mutex m;
    InitializeCriticalSection((CRITICAL_SECTION *) m.PlatformData.Win32.Handle);
    return m;
}

void free_mutex(mutex *m) {
    auto *p = (CRITICAL_SECTION *) m->PlatformData.Win32.Handle;
    if (p) DeleteCriticalSection(p);
}

void lock(mutex *m) {
    EnterCriticalSection((CRITICAL_SECTION *) m->PlatformData.Win32.Handle);
}

bool try_lock(mutex *m) {
    return TryEnterCriticalSection((CRITICAL_SECTION *) m->PlatformData.Win32.Handle);
}

void unlock(mutex *m) {
    LeaveCriticalSection((CRITICAL_SECTION *) m->PlatformData.Win32.Handle);
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

condition_variable create_condition_variable() {
    condition_variable c;

    auto *data = (CV_Data *) c.Handle;

    data->Events[_CONDITION_EVENT_ONE] = CreateEventW(null, 0, 0, null);
    data->Events[_CONDITION_EVENT_ALL] = CreateEventW(null, 1, 0, null);
    InitializeCriticalSection(&data->WaitersCountLock);

    return c;
}

void free_condition_variable(condition_variable *c) {
    auto *data = (CV_Data *) c->Handle;
    if (data) {
        CloseHandle(data->Events[_CONDITION_EVENT_ONE]);
        CloseHandle(data->Events[_CONDITION_EVENT_ALL]);
        DeleteCriticalSection(&data->WaitersCountLock);
    }
}

void pre_wait(condition_variable *c) {
    auto *data = (CV_Data *) c->Handle;

    // Increment number of waiters
    EnterCriticalSection(&data->WaitersCountLock);
    ++data->WaitersCount;
    LeaveCriticalSection(&data->WaitersCountLock);
}

void do_wait(condition_variable *c) {
    auto *data = (CV_Data *) c->Handle;

    // Wait for either event to become signaled due to notify_one() or notify_all() being called
    s32 result = WaitForMultipleObjects(2, data->Events, 0, INFINITE);

    // Check if we are the last waiter
    EnterCriticalSection(&data->WaitersCountLock);
    --data->WaitersCount;
    bool lastWaiter = result == WAIT_OBJECT_0 + _CONDITION_EVENT_ALL && data->WaitersCount == 0;
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we are the last waiter to be notified to stop waiting, reset the event
    if (lastWaiter) ResetEvent(data->Events[_CONDITION_EVENT_ALL]);
}

void notify_one(condition_variable *c) {
    auto *data = (CV_Data *) c->Handle;

    // Are there any waiters?
    EnterCriticalSection(&data->WaitersCountLock);
    bool haveWaiters = data->WaitersCount > 0;
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we have any waiting threads, send them a signal
    if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ONE]);
}

void notify_all(condition_variable *c) {
    auto *data = (CV_Data *) c->Handle;

    // Are there any waiters?
    EnterCriticalSection(&data->WaitersCountLock);
    bool haveWaiters = data->WaitersCount > 0;
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we have any waiting threads, send them a signal
    if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ALL]);
}

// Information to pass to the new thread (what to run).
export struct thread_start_info {
    delegate<void(void *)> Function;
    void *UserData = null;

    // We have to make sure the module the thread is executing in
    // doesn't get unloaded while the thread is still doing work.
    HMODULE Module = null;

    // Pointer to the implicit context in the "parent" thread.
    // We copy its members to the newly created thread.
    const context *ContextPtr = null;
    bool ParentWasUsingTemporaryAllocator;
};

export u32 __stdcall thread_wrapper_function(void *data) {
    auto *ti = (thread_start_info *) data;

    // We are allowed to do this because we are the parents
    *const_cast<allocator *>(&TemporaryAllocator) = {default_temp_allocator, (void *) &TemporaryAllocatorData};

    // Copy the parent thread's context
    auto newContext     = *ti->ContextPtr;
    newContext.ThreadID = GetCurrentThreadId();
    // If the parent thread was using the temporary allocator,
    // set the new thread to also use the temporary allocator,
    // but it needs to point to its own temp data (otherwise we are not thread-safe).
    if (ti->ParentWasUsingTemporaryAllocator) {
        newContext.Alloc = TemporaryAllocator;
    }
    OVERRIDE_CONTEXT(newContext);

    ti->Function(ti->UserData);  // Call the thread function with the user data

    free(ti);

    ExitThread(0);
    if (ti->Module) FreeLibrary(ti->Module);

    return 0;
}

thread create_and_launch_thread(const delegate<void(void *)> &function, void *userData);

void wait(thread t) {
    assert(t.ThreadID != Context.ThreadID);  // A thread cannot wait for itself!
    WaitForSingleObject(t.Handle, INFINITE);
}

void terminate(thread t) {
    if (t.Handle) {
        TerminateThread(t.Handle, 0);
    }
}

void sleep(u32 ms) { Sleep(ms); }

LSTD_END_NAMESPACE
