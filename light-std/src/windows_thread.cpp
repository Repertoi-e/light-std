#include "lstd/common.hpp"

#if OS == WINDOWS

#include "lstd/context.hpp"
#include "lstd/thread.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <process.h>

LSTD_BEGIN_NAMESPACE

namespace thread {

//
// Mutexes:
//

#define MHANDLE (CRITICAL_SECTION *) Handle

Mutex::Mutex() : AlreadyLocked(false) { InitializeCriticalSection(MHANDLE); }
Mutex::~Mutex() { DeleteCriticalSection(MHANDLE); }

void Mutex::lock() {
    EnterCriticalSection(MHANDLE);

    // Simulate deadlock...
    while (AlreadyLocked) Sleep(1000);
    AlreadyLocked = true;
}

bool Mutex::try_lock() {
    bool result = (TryEnterCriticalSection(MHANDLE) ? true : false);
    if (result && AlreadyLocked) {
        LeaveCriticalSection(MHANDLE);
        result = false;
    }
    return result;
}

void Mutex::unlock() {
    AlreadyLocked = false;
    LeaveCriticalSection(MHANDLE);
}

Recursive_Mutex::Recursive_Mutex() { InitializeCriticalSection(MHANDLE); }
Recursive_Mutex::~Recursive_Mutex() { DeleteCriticalSection(MHANDLE); }

void Recursive_Mutex::lock() { EnterCriticalSection(MHANDLE); }
bool Recursive_Mutex::try_lock() { return TryEnterCriticalSection(MHANDLE) ? true : false; }

void Recursive_Mutex::unlock() { LeaveCriticalSection(MHANDLE); }

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

Condition_Variable::Condition_Variable() {
    auto *data = (CV_Data *) Handle;

    data->Events[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
    data->Events[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&data->WaitersCountLock);
}

Condition_Variable::~Condition_Variable() {
    auto *data = (CV_Data *) Handle;

    CloseHandle(data->Events[_CONDITION_EVENT_ONE]);
    CloseHandle(data->Events[_CONDITION_EVENT_ALL]);
    DeleteCriticalSection(&data->WaitersCountLock);
}

void Condition_Variable::pre_wait() {
    auto *data = (CV_Data *) Handle;

    // Increment number of waiters
    EnterCriticalSection(&data->WaitersCountLock);
    ++data->WaitersCount;
    LeaveCriticalSection(&data->WaitersCountLock);
}

void Condition_Variable::do_wait() {
    auto *data = (CV_Data *) Handle;

    // Wait for either event to become signaled due to notify_one() or
    // notify_all() being called
    s32 result = WaitForMultipleObjects(2, data->Events, FALSE, INFINITE);

    // Check if we are the last waiter
    EnterCriticalSection(&data->WaitersCountLock);
    --data->WaitersCount;
    bool lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) && (data->WaitersCount == 0);
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we are the last waiter to be notified to stop waiting, reset the event
    if (lastWaiter) ResetEvent(data->Events[_CONDITION_EVENT_ALL]);
}

void Condition_Variable::notify_one() {
    auto *data = (CV_Data *) Handle;

    // Are there any waiters?
    EnterCriticalSection(&data->WaitersCountLock);
    bool haveWaiters = (data->WaitersCount > 0);
    LeaveCriticalSection(&data->WaitersCountLock);

    // If we have any waiting threads, send them a signal
    if (haveWaiters) SetEvent(data->Events[_CONDITION_EVENT_ONE]);
}
void Condition_Variable::notify_all() {
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
struct Thread_Start_Info {
    Delegate<void(void *)> Function;
    void *UserData;
    Thread *ThreadPtr;

    // Pointer to the implicit context in the "parent" thread.
    // We copy its members to the newly created thread.
    const Implicit_Context *ContextPtr;
};

u32 __stdcall Thread::wrapper_function(void *data) {
    auto *ti = (Thread_Start_Info *) data;

    // Copy the context from the "parent" thread
    *const_cast<Implicit_Context *>(&Context) = *ti->ContextPtr;

    ti->Function(ti->UserData);

    // The thread is no longer executing
    Scoped_Lock<Mutex> guard(ti->ThreadPtr->DataMutex);
    ti->ThreadPtr->NotAThread = true;

    delete ti;

    return 0;
}

Thread::Thread(Delegate<void(void *)> function, void *userData) {
    Scoped_Lock<Mutex> guard(DataMutex);

    // Passed to the thread wrapper, which will eventually free it
    auto *ti = new Thread_Start_Info;
    ti->Function = function;
    ti->UserData = userData;
    ti->ThreadPtr = this;
    ti->ContextPtr = &Context;

    NotAThread = false;

    // Create the thread
    // if (pthread_create(&mHandle, NULL, wrapper_function, (void *) ti) != 0) mHandle = 0;
#if !defined LSTD_NO_CRT
    Handle = _beginthreadex(0, 0, wrapper_function, (void *) ti, 0, &Win32ThreadID);
#else
#error Use CreateThread and init thread local storage
#endif

    if (!Handle) {
        NotAThread = true;
        delete ti;
    }
}

Thread::~Thread() {
    if (joinable()) os_exit_program(-1);
}

void Thread::join() {
    if (joinable()) {
        // pthread_join(mHandle, NULL);
        WaitForSingleObject((HANDLE) Handle, INFINITE);
        CloseHandle((HANDLE) Handle);
    }
}

bool Thread::joinable() const {
    Scoped_Lock<Mutex> guard(DataMutex);
    bool result = !NotAThread;
    return result;
}

void Thread::detach() {
    Scoped_Lock<Mutex> guard(DataMutex);
    if (!NotAThread) {
        // pthread_detach(mHandle);
        CloseHandle((HANDLE) Handle);
        NotAThread = true;
    }
}

// return _pthread_t_to_ID(mHandle);
Thread::id Thread::get_id() const {
    if (!joinable()) return id();
    return Id((u64) Win32ThreadID);
}

u32 get_hardware_concurrency() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (u32) si.dwNumberOfProcessors;
}

//
// this_thread:
//

Thread::id this_thread::get_id() { return Thread::id((u64) GetCurrentThreadId()); }
void this_thread::yield() { Sleep(0); }
void this_thread::sleep_for(u32 ms) { Sleep((DWORD) ms); }

}  // namespace thread

LSTD_END_NAMESPACE

#endif