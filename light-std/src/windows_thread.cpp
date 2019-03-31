#include "lstd/common.hpp"

#if OS == WINDOWS

#include "lstd/context.hpp"
#include "lstd/thread.hpp"

#include "lstd/fmt.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <process.h>

LSTD_BEGIN_NAMESPACE

namespace thread {

//
// Mutexes:
//

#define MHANDLE (CRITICAL_SECTION *) _Handle

mutex::mutex() : _AlreadyLocked(false) { InitializeCriticalSection(MHANDLE); }
mutex::~mutex() { DeleteCriticalSection(MHANDLE); }

void mutex::lock() {
    EnterCriticalSection(MHANDLE);

    // Simulate deadlock...
    while (_AlreadyLocked) Sleep(1000);
    _AlreadyLocked = true;
}

bool mutex::try_lock() {
    bool result = TryEnterCriticalSection(MHANDLE);
    if (result && _AlreadyLocked) {
        LeaveCriticalSection(MHANDLE);
        result = false;
    }
    return result;
}

void mutex::unlock() {
    _AlreadyLocked = false;
    LeaveCriticalSection(MHANDLE);
}

recursive_mutex::recursive_mutex() { InitializeCriticalSection(MHANDLE); }
recursive_mutex::~recursive_mutex() { DeleteCriticalSection(MHANDLE); }

void recursive_mutex::lock() { EnterCriticalSection(MHANDLE); }
bool recursive_mutex::try_lock() { return TryEnterCriticalSection(MHANDLE) ? true : false; }

void recursive_mutex::unlock() { LeaveCriticalSection(MHANDLE); }

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

condition_variable::condition_variable() {
    auto *data = (CV_Data *) _Handle;

    data->Events[_CONDITION_EVENT_ONE] = CreateEvent(null, FALSE, FALSE, null);
    data->Events[_CONDITION_EVENT_ALL] = CreateEvent(null, TRUE, FALSE, null);
    InitializeCriticalSection(&data->WaitersCountLock);
}

condition_variable::~condition_variable() {
    auto *data = (CV_Data *) _Handle;

    CloseHandle(data->Events[_CONDITION_EVENT_ONE]);
    CloseHandle(data->Events[_CONDITION_EVENT_ALL]);
    DeleteCriticalSection(&data->WaitersCountLock);
}

void condition_variable::pre_wait() {
    auto *data = (CV_Data *) _Handle;

    // Increment number of waiters
    EnterCriticalSection(&data->WaitersCountLock);
    ++data->WaitersCount;
    LeaveCriticalSection(&data->WaitersCountLock);
}

void condition_variable::do_wait() {
    auto *data = (CV_Data *) _Handle;

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
    auto *data = (CV_Data *) _Handle;

    // Are there any waiters?
    EnterCriticalSection(&data->WaitersCountLock);
    bool haveWaiters = (data->WaitersCount > 0);
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we have any waiting threads, send them a signal
    if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ONE]);
}

void condition_variable::notify_all() {
    auto *data = (CV_Data *) _Handle;

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
struct Thread_Start_Info {
    delegate<void(void *)> Function;
    void *UserData;
    thread *ThreadPtr;

    // This may be null. It's used only when we don't link with the CRT,
    // because we have to make sure the module the thread is executing in
    // doesn't get unloaded while the thread is still doing work.
    // (The CRT usually does that for us.)
    bool NoCrt = false;
    HMODULE Module;

    // Pointer to the implicit context in the "parent" thread.
    // We copy its members to the newly created thread.
    const Implicit_Context *ContextPtr;
};

u32 __stdcall thread::wrapper_function(void *data) {
    auto *ti = (Thread_Start_Info *) data;

    // Copy the context from the "parent" thread
    *const_cast<Implicit_Context *>(&Context) = *ti->ContextPtr;

    ti->Function(ti->UserData);

    // The thread is no longer executing
    scoped_lock<mutex> _(ti->ThreadPtr->_DataMutex);
    ti->ThreadPtr->_NotAThread = true;

    if (ti->NoCrt) {
        CloseHandle((HANDLE) ti->ThreadPtr->Handle);
        ti->ThreadPtr->Handle = 0;

        if (ti->Module) {
            FreeLibrary(ti->Module);
        }
    }

    // Seems like this line fixes a rare crash
    ti->Function.~delegate<void(void *)>();

    delete ti;

    return 0;
}

thread::thread(delegate<void(void *)> function, void *userData) {
    scoped_lock<mutex> _(_DataMutex);

    // Passed to the thread wrapper, which will eventually free it
    auto *ti = new Thread_Start_Info;
    {
        ti->Function = function;
        ti->UserData = userData;
        ti->ThreadPtr = this;
        ti->ContextPtr = &Context;
    }

    _NotAThread = false;

    // Create the thread
    // if (pthread_create(&mHandle, NULL, wrapper_function, (void *) ti) != 0) mHandle = 0;
#if !defined LSTD_NO_CRT
    Handle = _beginthreadex(null, 0, wrapper_function, (void *) ti, /*Creation Flags*/ 0, &_Win32ThreadId);
#else
    ti->NoCrt = true;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR) wrapper_function, &ti->Module);

    Handle = (uptr_t) CreateThread(null, 0, (LPTHREAD_START_ROUTINE) wrapper_function, ti, 0, (DWORD *) &Win32ThreadID);
#endif
    if (!Handle || (HANDLE) Handle == INVALID_HANDLE_VALUE) {
        _NotAThread = true;
        delete ti;
    }
}

thread::~thread() {
    if (joinable()) os_exit_program(-1);
}

void thread::join() {
    if (joinable()) {
        // pthread_join(mHandle, NULL);
        if (Handle && (HANDLE) Handle != INVALID_HANDLE_VALUE) {
            WaitForSingleObject((HANDLE) Handle, INFINITE);
            CloseHandle((HANDLE) Handle);

            scoped_lock<mutex> _(_DataMutex);
            Handle = 0;
            _NotAThread = true;
        }
    }
}

bool thread::joinable() const {
    scoped_lock<mutex> _(_DataMutex);
    bool result = !_NotAThread;
    return result;
}

void thread::detach() {
    scoped_lock<mutex> _(_DataMutex);
    if (!_NotAThread) {
        // pthread_detach(mHandle);
        CloseHandle((HANDLE) Handle);
        _NotAThread = true;
    }
}

// return _pthread_t_to_ID(mHandle);
thread::id thread::get_id() const {
    if (!joinable()) return id();
    return id((u64) _Win32ThreadId);
}

u32 get_hardware_concurrency() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (u32) si.dwNumberOfProcessors;
}

//
// this_thread:
//

thread::id this_thread::get_id() { return thread::id((u64) GetCurrentThreadId()); }
void this_thread::yield() { Sleep(0); }
void this_thread::sleep_for(u32 ms) { Sleep((DWORD) ms); }

}  // namespace thread

LSTD_END_NAMESPACE

#endif