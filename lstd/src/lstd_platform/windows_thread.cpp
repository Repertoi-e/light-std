#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/internal/context.h"
#include "lstd/os.h"
#include "lstd/types/windows.h"  // For API calls and definitions

LSTD_BEGIN_NAMESPACE

allocator win64_get_persistent_allocator();

namespace thread {

// Block the calling thread until a lock on the mutex can
// be obtained. The mutex remains locked until unlock() is called.
void fast_mutex::lock() {
    while (!try_lock()) sleep(0);
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
}

bool mutex::try_lock() {
    return TryEnterCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle);
}

void mutex::unlock() {
    LeaveCriticalSection((CRITICAL_SECTION *) PlatformData.Win32.Handle);
}

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

    data->Events[_CONDITION_EVENT_ONE] = CreateEventW(null, 0, 0, null);
    data->Events[_CONDITION_EVENT_ALL] = CreateEventW(null, 1, 0, null);
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
    s32 result = WaitForMultipleObjects(2, data->Events, 0, INFINITE);

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
    const context *ContextPtr = null;
};

u32 __stdcall thread::wrapper_function(void *data) {
    auto *ti = (thread_start_info *) data;

    // Initialize the default context for a new thread
    // win64_common_init_context();
    // The context has been initialized already (see tls_init in windows_common.cpp).

    // @TODO: Make this optional:
    // Now we copy the context variables from the parent thread
    s64 firstByte = offset_of(context, TempAlloc) + sizeof(context::TempAlloc);
    copy_memory((byte *) &Context + firstByte, (byte *) ti->ContextPtr + firstByte, sizeof(context) - firstByte);

    // If the parent thread was using the temporary allocator, set the new thread to also use the temporary allocator,
    // but it needs to point to its own temp data (otherwise we are not thread-safe).
    if (ti->ContextPtr->Alloc == ti->ContextPtr->TempAlloc) {
        auto newContext = Context;
        newContext.Alloc = Context.TempAlloc;
        OVERRIDE_CONTEXT(newContext);
    }

    ti->Function(ti->UserData);  // Call the thread function with the user data

    // Do we need this?
    // CloseHandle(ti->ThreadPtr->Handle);

    free(ti);

#if defined LSTD_NO_CRT
    ExitThread(0);
    if (ti->Module) FreeLibrary(ti->Module);
#else
    _endthreadex(0);
#endif

    return 0;
}

void thread::init_and_launch(const delegate<void(void *)> &function, void *userData) {
    // Passed to the thread wrapper, which will eventually free it
    auto *ti = allocate<thread_start_info>({.Alloc = win64_get_persistent_allocator()});
    ti->Function = function;
    ti->UserData = userData;
    ti->ThreadPtr = this;
    ti->ContextPtr = &Context;

#if defined LSTD_NO_CRT
    // Create the thread
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR) wrapper_function, &ti->Module);
    auto handle = CreateThread(null, 0, (LPTHREAD_START_ROUTINE) wrapper_function, ti, 0, (DWORD *) &Win32ThreadId);
#else
    auto handle = _beginthreadex(null, 0, wrapper_function, ti, 0, &Win32ThreadId);
#endif
    Handle = (void *) handle;

    if (!handle || (void *) handle == INVALID_HANDLE_VALUE) {
        // We free this directly since thread wrapper never even ran
        free(ti);
    }
}

void thread::wait() {
    assert(get_id() != Context.ThreadID);  // A thread cannot wait for itself!
    WaitForSingleObject(Handle, INFINITE);
}

void thread::terminate() {
    if (Handle) {
        TerminateThread(Handle, 0);
    }
}

// return _pthread_t_to_ID(mHandle);
::LSTD_NAMESPACE::thread::id thread::get_id() const {
    return id((u64) Win32ThreadId);
}

void sleep(u32 ms) { Sleep((DWORD) ms); }

}  // namespace thread

u32 os_get_hardware_concurrency() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return (u32) si.dwNumberOfProcessors;
}

LSTD_END_NAMESPACE

#endif