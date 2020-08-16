#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/internal/context.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

namespace thread {

// Block the calling thread until a lock on the mutex can
// be obtained. The mutex remains locked until unlock() is called.
void fast_mutex::lock() {
    while (!try_lock()) yield();
}

//
// Mutexes:
//
void mutex::init() { InitializeCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle); }

void mutex::release() {
    auto *p = (CRITICAL_SECTION *) PlatformData.Win32.Handle;
    if (p) DeleteCriticalSection(p);
}

void mutex::lock() {
    EnterCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle);

    // Simulate deadlock...
    while (AlreadyLocked) Sleep(1000);
    AlreadyLocked = true;
}

bool mutex::try_lock() {
    bool result = TryEnterCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle);
    if (result) {
        if (AlreadyLocked) {
            LeaveCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle);
            result = false;
        } else {
            AlreadyLocked = true;
        }
    }
    return result;
}

void mutex::unlock() {
    assert(AlreadyLocked);
    AlreadyLocked = false;
    LeaveCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle);
}

void recursive_mutex::init() { InitializeCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle); }

void recursive_mutex::release() {
    auto *p = (CRITICAL_SECTION *) PlatformData.Win32.Handle;
    if (p) DeleteCriticalSection(p);
}

void recursive_mutex::lock() { EnterCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle); }

bool recursive_mutex::try_lock() { return TryEnterCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle); }

void recursive_mutex::unlock() { LeaveCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle); }

//
// Condition variable:
//

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

void condition_variable::init() {
    auto *data = (CV_Data *) Handle;

    data->Events[_CONDITION_EVENT_ONE] = CreateEvent(null, FALSE, FALSE, null);
    data->Events[_CONDITION_EVENT_ALL] = CreateEvent(null, TRUE, FALSE, null);
    InitializeCriticalSection(&data->WaitersCountLock);
}

void condition_variable::release() {
    auto *data = (CV_Data *) Handle;
    if (data) {
        CloseHandle(data->Events[_CONDITION_EVENT_ONE]);
        CloseHandle(data->Events[_CONDITION_EVENT_ALL]);
        DeleteCriticalSection(&data->WaitersCountLock);
    }
}

void condition_variable::pre_wait() {
    auto *data = (CV_Data *) Handle;

    // Increment number of waiters
    EnterCriticalSection(&data->WaitersCountLock);
    ++data->WaitersCount;
    LeaveCriticalSection(&data->WaitersCountLock);
}

void condition_variable::do_wait() {
    auto *data = (CV_Data *) Handle;

    // Wait for either event to become signaled due to notify_one() or notify_all() being called
    s32 result = WaitForMultipleObjects(2, data->Events, FALSE, INFINITE);

    // Check if we are the last waiter
    EnterCriticalSection(&data->WaitersCountLock);
    --data->WaitersCount;
    bool lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) && (data->WaitersCount == 0);
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we are the last waiter to be notified to stop waiting, reset the event
    if (lastWaiter) ResetEvent(data->Events[_CONDITION_EVENT_ALL]);
}

void condition_variable::notify_one() {
    auto *data = (CV_Data *) Handle;

    // Are there any waiters?
    EnterCriticalSection(&data->WaitersCountLock);
    bool haveWaiters = (data->WaitersCount > 0);
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we have any waiting threads, send them a signal
    if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ONE]);
}

void condition_variable::notify_all() {
    auto *data = (CV_Data *) Handle;

    // Are there any waiters?
    EnterCriticalSection(&data->WaitersCountLock);
    bool haveWaiters = (data->WaitersCount > 0);
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we have any waiting threads, send them a signal
    if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ALL]);
}

//
// Thread:
//

// Information to pass to the new thread (what to run).
struct thread_start_info {
    delegate<void(void *)> Function;
    void *UserData = null;

    thread *ThreadPtr = null;

    // We have to make sure the module the thread is executing in
    // doesn't get unloaded while the thread is still doing work.
    // The CRT usually does that for us but we avoid using the CRT.
    HMODULE Module = null;

    // Pointer to the implicit context in the "parent" thread.
    // We copy its members to the newly created thread.
    implicit_context *ContextPtr = null;
};

u32 __stdcall thread::wrapper_function(void *data) {
    auto *ti = (thread_start_info *) data;
    defer(free(ti));

    // Copy the context from the parent thread
    Context = *ti->ContextPtr;
    Context.TemporaryAllocData = {};
    Context.TemporaryAlloc.Context = &Context.TemporaryAllocData;
    Context.ThreadID = ::thread::id((u64) GetCurrentThreadId());

    ti->Function(ti->UserData);

    void *handle = atomic_compare_exchange_pointer(&ti->ThreadPtr->Handle, null, null);
    if (handle && handle != INVALID_HANDLE_VALUE) {
        atomic_exchange_64((long long *) &ti->ThreadPtr->Handle, 0);
        atomic_exchange(&ti->ThreadPtr->Finished, 1);
        CloseHandle(handle);
    }

    if (ti->Module) FreeLibrary(ti->Module);

    return 0;
}

void thread::init(const delegate<void(void *)> &function, void *userData) {
    // Passed to the thread wrapper, which will eventually free it
    auto *ti = allocate(thread_start_info);
    ti->Function = function;
    ti->UserData = userData;
    ti->ThreadPtr = this;
    ti->ContextPtr = &Context;

    Finished = false;

    // Create the thread
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR) wrapper_function, &ti->Module);
    auto handle = CreateThread(null, 0, (LPTHREAD_START_ROUTINE) wrapper_function, ti, 0, (DWORD *) &Win32ThreadId);
    Handle = handle;

    if (!handle || handle == INVALID_HANDLE_VALUE) {
        // We can access this diretly since thread wrapper never even ran
        Finished = true;
        free(ti);
    }
}

void thread::join() {
    u32 finished = atomic_compare_exchange(&Finished, 0, 0);

    if (!finished) {
        // pthread_join(mHandle, NULL);

        void *handle = atomic_compare_exchange_pointer(&Handle, null, null);
        if (handle && handle != INVALID_HANDLE_VALUE) {
            WaitForSingleObject(handle, INFINITE);
        }
    }
}

void thread::detach() {
    u32 finished = atomic_compare_exchange((u32 *) &Finished, 0, 0);

    if (!finished) {
        // pthread_join(mHandle, NULL);

        void *handle = atomic_compare_exchange_pointer(&Handle, null, null);
        if (handle && handle != INVALID_HANDLE_VALUE) {
            atomic_exchange_pointer(&Handle, null);
            atomic_exchange(&Finished, 1);
            CloseHandle(handle);
        }
    }
}

// return _pthread_t_to_ID(mHandle);
::thread::id thread::get_id() const {
    u32 finished = atomic_compare_exchange((u32 *) &Finished, 0, 0);
    if (!finished) return id();
    return id((u64) Win32ThreadId);
}

void yield() { Sleep(0); }
void sleep(u32 ms) { Sleep((DWORD) ms); }

}  // namespace thread

u32 os_get_hardware_concurrency() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (u32) si.dwNumberOfProcessors;
}

LSTD_END_NAMESPACE

#endif