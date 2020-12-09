#include "lstd/internal/common.h"

#if OS == WINDOWS && defined BUILD_NO_CRT

#include "lstd/internal/context.h"

extern "C" int _fltused = 0;

#include <Windows.h>
#include <limits.h>
#include <shellapi.h>

// The following code is taken from the deep dark parts of MSVC's crt...

// The DECLARE_ALTERNATE_NAME macro provides an architecture-neutral way
// of specifying /alternatename comments to the linker.  It prepends the leading
// decoration character for x86 and hybrid and leaves names unmodified for other
// architectures.

#define DECLARE_ALTERNATE_NAME_DATA(name, alternate_name) \
    __pragma(comment(linker, "/alternatename:" #name "=" #alternate_name))

#pragma section(".CRT$XCA", long, read)
#pragma section(".CRT$XCZ", long, read)
#pragma section(".CRT$XIA", long, read)
#pragma section(".CRT$XIZ", long, read)
#pragma section(".CRT$XPA", long, read)
#pragma section(".CRT$XPZ", long, read)
#pragma section(".CRT$XTA", long, read)
#pragma section(".CRT$XTZ", long, read)
#pragma section(".CRT$XIC", long, read)
#pragma section(".CRT$XLA", long, read)
#pragma section(".CRT$XLC", long, read)
#pragma section(".CRT$XLD", long, read)
#pragma section(".CRT$XLZ", long, read)
#pragma section(".CRT$XDA", long, read)
#pragma section(".CRT$XDZ", long, read)
#pragma section(".rdata$T", long, read)

typedef void(__cdecl *_PVFV)(void);
typedef int(__cdecl *_PIFV)(void);

#if COMPILER == MSVC || COMPILER == GCC
#pragma push_macro("allocate")
#undef allocate
#endif

extern __declspec(allocate(".CRT$XIA")) _PIFV __xi_a[] = {null};  // First C Initializer
extern __declspec(allocate(".CRT$XIZ")) _PIFV __xi_z[] = {null};  // Last C Initializer
extern __declspec(allocate(".CRT$XCA")) _PVFV __xc_a[] = {null};  // First C++ Initializer
extern __declspec(allocate(".CRT$XCZ")) _PVFV __xc_z[] = {null};  // Last C++ Initializer
extern __declspec(allocate(".CRT$XPA")) _PVFV __xp_a[] = {null};  // First Pre-Terminator
extern __declspec(allocate(".CRT$XPZ")) _PVFV __xp_z[] = {null};  // Last Pre-Terminator
extern __declspec(allocate(".CRT$XTA")) _PVFV __xt_a[] = {null};  // First Terminator
extern __declspec(allocate(".CRT$XTZ")) _PVFV __xt_z[] = {null};  // Last Terminator
#pragma comment(linker, "/merge:.CRT=.rdata")

void __cdecl walk_table_of_functions(_PVFV *pfbegin, _PVFV *pfend) {
    for (; pfbegin < pfend; ++pfbegin) {
        if (*pfbegin) (**pfbegin)();
    }
}

int __cdecl walk_table_of_functions_and_return_result(_PIFV *pfbegin, _PIFV *pfend) {
    int ret = 0;
    for (; pfbegin < pfend && ret == 0; ++pfbegin) {
        if (*pfbegin) ret = (**pfbegin)();
    }
    return ret;
}

#if defined LSTD_NAMESPACE_NAME
using namespace LSTD_NAMESPACE_NAME;
#endif

extern "C" {
struct onexit_table_t {
    _PVFV *_first, *_last, *_end;
};
file_scope onexit_table_t ExitTable{};

file_scope bool ExitMutexInitted = false;
file_scope CRITICAL_SECTION AtExitMutex;

int __cdecl atexit(_PVFV function) {
    if (!ExitMutexInitted) {
        InitializeCriticalSectionEx(&AtExitMutex, 4000, 0);
        ExitMutexInitted = true;
    }
    EnterCriticalSection(&AtExitMutex);
    defer(LeaveCriticalSection(&AtExitMutex));

    _PVFV *first = ExitTable._first;
    _PVFV *last = ExitTable._last;
    _PVFV *end = ExitTable._end;

    // If there is no room for the new entry, reallocate a larger table:
    if (last == end) {
        size_t oldCount = end - first;
        size_t increment = oldCount > 512 ? 512 : oldCount;

        size_t newCount = oldCount + increment;
        if (newCount == 0) newCount = 32;

        _PVFV *newFirst = null;
        if (newCount >= oldCount) {
            if (ExitTable._first) {
                newFirst = reallocate_array(first, newCount);
            } else {
                newFirst = allocate_array(_PVFV, newCount);
            }
        }
        if (newFirst == null) return -1;

        first = newFirst;
        last = newFirst + oldCount;
        end = newFirst + newCount;
    }

    *last++ = (_PVFV)(function);

    ExitTable._first = first;
    ExitTable._last = last;
    ExitTable._end = end;

    return 0;
}

// Thread-local storage:
ULONG _tls_index = 0;

#pragma data_seg(".tls")
#if defined(_M_X64)
__declspec(allocate(".tls"))
#endif /* defined (_M_X64) */
    char _tls_start = 0;

#pragma data_seg(".tls$ZZZ")
#if defined(_M_X64)
__declspec(allocate(".tls$ZZZ"))
#endif /* defined (_M_X64) */
    char _tls_end = 0;

#pragma data_seg()

__declspec(allocate(".CRT$XLA")) PIMAGE_TLS_CALLBACK __xl_a = 0;
__declspec(allocate(".CRT$XLZ")) PIMAGE_TLS_CALLBACK __xl_z = 0;

__declspec(allocate(".rdata$T")) extern const IMAGE_TLS_DIRECTORY _tls_used = {
    (ULONGLONG) &_tls_start,   // start of tls data
    (ULONGLONG) &_tls_end,     // end of tls data
    (ULONGLONG) &_tls_index,   // address of tls_index
    (ULONGLONG)(&__xl_a + 1),  // pointer to call back array
    (ULONG) 0,                 // size of tls zero fill
    (ULONG) 0                  // characteristics
};

static __declspec(allocate(".CRT$XDA")) _PVFV __xd_a = null;
static __declspec(allocate(".CRT$XDZ")) _PVFV __xd_z = null;

// When any thread starts up, walk the array of function pointers found
// in sections.CRT$XD*, calling each non - null entry to dynamically
// initialize that thread's copy of a __declspec(thread) variable.
void __stdcall __dyn_tls_init(PVOID, DWORD dwReason, LPVOID) {
    if (dwReason != DLL_THREAD_ATTACH) return;

#pragma warning(push)
#pragma warning(disable : 26000)
    for (_PVFV *pfunc = &__xd_a + 1; pfunc != &__xd_z; ++pfunc) {
        if (*pfunc) (*pfunc)();
    }
#pragma warning(pop)
}

typedef void(__stdcall *_tls_callback_type)(void *, unsigned long, void *);

#define FUNCS_PER_NODE 30

typedef struct TlsDtorNode {
    int count;
    struct TlsDtorNode *next;
    _PVFV funcs[FUNCS_PER_NODE];
} TlsDtorNode;

static __declspec(thread) TlsDtorNode *dtor_list;
static __declspec(thread) TlsDtorNode dtor_list_head;

int __cdecl __tlregdtor(_PVFV func) {
    if (dtor_list == null) {
        dtor_list = &dtor_list_head;
        dtor_list_head.count = 0;
    } else if (dtor_list->count == FUNCS_PER_NODE) {
        auto *pnode = (TlsDtorNode *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TlsDtorNode) * 1);
        if (pnode == null) {
            return -1;
        }
        pnode->count = 0;
        pnode->next = dtor_list;
        dtor_list = pnode;
        /* this helps prefast make sure dtor_list->count is 0 */
        dtor_list->count = 0;
    }
    dtor_list->funcs[dtor_list->count++] = func;
    return 0;
}

file_scope void __stdcall tls_uninit(HANDLE, DWORD const dwReason, LPVOID) {
    if (dwReason != DLL_THREAD_DETACH && dwReason != DLL_PROCESS_DETACH) {
        return;
    }

    TlsDtorNode *pnext = null;
    for (TlsDtorNode *pnode = dtor_list; pnode != null; pnode = pnext) {
        for (int i = pnode->count - 1; i >= 0; --i) {
            if (pnode->funcs[i]) pnode->funcs[i]();
        }

        // Free every TlsDtorNode except the original one, which is statically allocated.
        pnext = pnode->next;
        if (pnext) HeapFree(GetProcessHeap(), 0, pnode);

        dtor_list = pnext;
    }
}

extern const PIMAGE_TLS_CALLBACK __dyn_tls_init_callback = __dyn_tls_init;
extern const _tls_callback_type __dyn_tls_dtor_callback = tls_uninit;
static __declspec(allocate(".CRT$XLC")) PIMAGE_TLS_CALLBACK __xl_c = __dyn_tls_init;
static __declspec(allocate(".CRT$XLD")) _tls_callback_type __xl_d = tls_uninit;

// Access to these variables is guarded in the below functions.  They may only
// be modified while the lock is held.  _Tss_epoch is readable from user
// code and is read without taking the lock.
file_scope s32 InitEpoch = INT_MIN;
__declspec(thread) int _Init_thread_epoch = INT_MIN;

file_scope CRITICAL_SECTION g_TssMutex;
file_scope CONDITION_VARIABLE g_TssCv;

void __cdecl _Init_thread_header(int *const pOnce) {
    EnterCriticalSection(&g_TssMutex);
    defer(LeaveCriticalSection(&g_TssMutex));

    if (!*pOnce) {
        *pOnce = -1;
    } else {
        while (*pOnce == -1) {
            SleepConditionVariableCS(&g_TssCv, &g_TssMutex, 100);
            if (!*pOnce) {
                *pOnce = -1;
                return;
            }
        }
        _Init_thread_epoch = InitEpoch;
    }
}

// Called by the thread that completes initialization of a variable.
// Increment the global and per thread counters, mark the variable as
// initialized, and release waiting threads.
void __cdecl _Init_thread_footer(int *const pOnce) noexcept {
    EnterCriticalSection(&g_TssMutex);
    ++InitEpoch;
    *pOnce = InitEpoch;
    _Init_thread_epoch = InitEpoch;
    LeaveCriticalSection(&g_TssMutex);

    WakeAllConditionVariable(&g_TssCv);
}
}

int _cdecl _purecall(void) {
    // TODO: Tell the user, and terminate process!
    return 0;
}

// Terminator for synchronization data structures.
file_scope void __cdecl thread_uninit() { DeleteCriticalSection(&g_TssMutex); }

// Initializer for synchronization data structures.
file_scope int __cdecl thread_init() {
    InitializeCriticalSectionEx(&g_TssMutex, 4000, 0);
    InitializeConditionVariable(&g_TssCv);
    atexit(thread_uninit);
    return 0;
}

__declspec(allocate(".CRT$XIC")) static _PIFV __scrt_initialize_tss_var = thread_init;

#if COMPILER == MSVC || COMPILER == GCC
#pragma pop_macro("allocate")
#endif

extern "C" LPSTR *CommandLineToArgvA(LPWSTR lpCmdLine, INT *pNumArgs) {
    int numArgs;
    LPWSTR *args;
    int retval;

    args = CommandLineToArgvW(lpCmdLine, &numArgs);
    if (args == null) return null;

    int storage = numArgs * sizeof(LPSTR);
    for (int i = 0; i < numArgs; ++i) {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, null, 0, null, &lpUsedDefaultChar);
        if (!SUCCEEDED(retval)) {
            LocalFree(args);
            return null;
        }

        storage += retval;
    }

    LPSTR *result = (LPSTR *) LocalAlloc(LMEM_FIXED, storage);
    if (result == null) {
        LocalFree(args);
        return null;
    }

    int bufLen = storage - numArgs * sizeof(LPSTR);
    LPSTR buffer = ((LPSTR) result) + numArgs * sizeof(LPSTR);
    for (int i = 0; i < numArgs; ++i) {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, buffer, bufLen, null, &lpUsedDefaultChar);
        if (!SUCCEEDED(retval)) {
            LocalFree(result);
            LocalFree(args);
            return null;
        }

        result[i] = buffer;
        buffer += retval;
        bufLen -= retval;
    }

    LocalFree(args);

    *pNumArgs = numArgs;
    return result;
}

extern "C" void execute_on_exit_table() {
    // Execute atexit callbacks
    _PVFV *first = ExitTable._first, *last = ExitTable._last;
    if (first) {
        _PVFV *savedFirst = first, *savedLast = last;
        while (true) {
            while (--last >= first && *last == null)
                ;

            if (last < first) {
                // There are no more valid entries in the list. We are done.
                break;
            }

            // Call the function pointer and mark it as visited
            (*last)();
            *last = null;

            // Reset iteration if either the begin or end pointer has changed:
            _PVFV *newFirst = ExitTable._first, *newLast = ExitTable._last;
            if (newFirst != savedFirst || newLast != savedLast) {
                first = savedFirst = newFirst;
                last = savedLast = newLast;
            }
        }
    }

    if (ExitMutexInitted) {
        DeleteCriticalSection(&AtExitMutex);
    }
}

// This flag is incremented each time DLL_PROCESS_ATTACH is processed successfully
// and is decremented each time DLL_PROCESS_DETACH is processed (the detach is
// always assumed to complete successfully).
file_scope int ProcAttached = 0;

// The client may define a _pRawDllMain.  This function gets called for attach
// notifications before any other function is called, and gets called for detach
// notifications after any other function is called.  If no _pRawDllMain is
// defined, it is aliased to the no-op _pDefaultRawDllMain.
typedef BOOL(WINAPI *__scrt_dllmain_type)(HINSTANCE, DWORD, LPVOID);
extern "C" extern __scrt_dllmain_type const _pRawDllMain;
extern "C" extern __scrt_dllmain_type const _pDefaultRawDllMain = null;
DECLARE_ALTERNATE_NAME_DATA(_pRawDllMain, _pDefaultRawDllMain)

file_scope BOOL WINAPI dllmain_raw(HINSTANCE const instance, DWORD const reason, LPVOID const reserved) {
    if (!_pRawDllMain) return TRUE;

    return _pRawDllMain(instance, reason, reserved);
}
file_scope bool __cdecl is_potentially_valid_image_base(void *const image_base) noexcept {
    if (!image_base) return false;

    auto header = (PIMAGE_DOS_HEADER) image_base;
    if (header->e_magic != IMAGE_DOS_SIGNATURE) return false;

    auto ntHeader = (PIMAGE_NT_HEADERS)((PBYTE) header + header->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE) return false;
    if (ntHeader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC) return false;

    return true;
}

// Given an RVA, finds the PE section in the pointed-to image that includes the
// RVA.  Returns null if no such section exists or the section is not found.
file_scope PIMAGE_SECTION_HEADER __cdecl find_pe_section(unsigned char *const imageBase, uintptr_t const rva) noexcept {
    auto const header = (PIMAGE_DOS_HEADER) imageBase;
    auto const ntHeader = (PIMAGE_NT_HEADERS)((PBYTE) header + header->e_lfanew);

    // Find the section holding the RVA.  We make no assumptions here about the
    // sort order of the section descriptors, though they always appear to be
    // sorted by ascending section RVA.
    PIMAGE_SECTION_HEADER first = IMAGE_FIRST_SECTION(ntHeader);
    PIMAGE_SECTION_HEADER last = first + ntHeader->FileHeader.NumberOfSections;
    for (auto it = first; it != last; ++it) {
        if (rva >= it->VirtualAddress && rva < it->VirtualAddress + it->Misc.VirtualSize) {
            return it;
        }
    }
    return null;
}

extern "C" IMAGE_DOS_HEADER __ImageBase;

// Tests whether a target address is located within the current PE image (the
// image located at __ImageBase), that the target address is located in a proper
// section of the image, and that the section in which it is located is not
// writable.
extern "C" bool __cdecl is_nonwritable_in_current_image(void const *const target) {
    auto targetAddress = (byte const *) target;
    auto imageBase = (byte *) &__ImageBase;

    if (!is_potentially_valid_image_base(imageBase)) return false;

    // Convert the target address to an RVA within the image and find the
    // corresponding PE section.  Return failure if the target address is
    // not found within the current image:
    uintptr_t const rvaTarget = targetAddress - imageBase;
    PIMAGE_SECTION_HEADER const section_header = find_pe_section(imageBase, rvaTarget);
    if (!section_header) return false;

    // Check the section characteristics to see if the target address is
    // located within a writable section, returning a failure if it is:
    if (section_header->Characteristics & IMAGE_SCN_MEM_WRITE) return false;

    return true;
}

file_scope BOOL WINAPI dllmain_crt_dispatch(HINSTANCE const instance, DWORD const reason, LPVOID const reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            // If we have any dynamically initialized __declspec(thread) variables, we
            // invoke their initialization for the thread on which the DLL is being loaded.
            if (is_nonwritable_in_current_image(__dyn_tls_init)) {
                __dyn_tls_init(instance, DLL_THREAD_ATTACH, reserved);
            }

            if (walk_table_of_functions_and_return_result(__xi_a, __xi_z) != 0) return FALSE;
            walk_table_of_functions(__xc_a, __xc_z);
            ++ProcAttached;
            return TRUE;
        }
        case DLL_PROCESS_DETACH: {
            // If the attach did not complete successfully, or if the detach was already
            // executed, do not execute the detach:
            if (ProcAttached <= 0) return FALSE;

            --ProcAttached;

            execute_on_exit_table();
            walk_table_of_functions(__xp_a, __xp_z);
            walk_table_of_functions(__xt_a, __xt_z);

            return TRUE;
        }
    }

    return TRUE;
}

extern "C" int main(int argc, char *argv[]);

// If we are building a dll that links to this library we don't need a valid "main" since it will never get called
// anyways
extern "C" int main_stub(int argc, char *argv[]) {
    assert(false && "Did you forget to add an entry point to your .exe?");
    return -1;
}
DECLARE_ALTERNATE_NAME_DATA(main, main_stub)

void os_exit_program(int code) {
    if (is_nonwritable_in_current_image(tls_uninit)) {
        tls_uninit(null, DLL_PROCESS_DETACH, null);
    }
    execute_on_exit_table();
    walk_table_of_functions(__xp_a, __xp_z);
    walk_table_of_functions(__xt_a, __xt_z);

    ExitProcess(code);
}

// Entry point for executables
extern "C" void main_no_crt() {
    // If this module has any dynamically initialized __declspec(thread)
    // variables, then we invoke their initialization for the primary thread
    // used to start the process:
    if (is_nonwritable_in_current_image(__dyn_tls_init)) {
        __dyn_tls_init(null, DLL_THREAD_ATTACH, null);
    }

    if (walk_table_of_functions_and_return_result(__xi_a, __xi_z) != 0) return;
    walk_table_of_functions(__xc_a, __xc_z);

    LPSTR *szArglist;
    int nArgs = 0;

    szArglist = CommandLineToArgvA(GetCommandLineW(), &nArgs);
    if (!szArglist) {
        return;
    }

    os_exit_program(main(nArgs, szArglist));
}

extern "C" BOOL WINAPI DllMain(HINSTANCE const instance, DWORD const reason, LPVOID const reserved);

extern "C" BOOL WINAPI DllMain_stub(HINSTANCE const instance, DWORD const reason, LPVOID const reserved) {
    return TRUE;
}
DECLARE_ALTERNATE_NAME_DATA(DllMain, DllMain_stub)

// Entry point for DLLs
extern "C" BOOL WINAPI main_no_crt_dll(HINSTANCE const instance, DWORD const reason, LPVOID const reserved) {
    // If this is a process detach notification, check that there was a prior
    // process attach notification that was processed successfully.  This is
    // to ensure that we don't detach more times than we attach.
    if (reason == DLL_PROCESS_DETACH && ProcAttached <= 0) {
        return FALSE;
    }

    BOOL result = TRUE;
    if (reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH) {
        result = dllmain_raw(instance, reason, reserved);
        if (!result) {
            return result;
        }

        result = dllmain_crt_dispatch(instance, reason, reserved);
        if (!result) {
            return result;
        }
    }

    result = DllMain(instance, reason, reserved);

    // If the client DllMain routine failed, unwind the initialization:
    if (reason == DLL_PROCESS_ATTACH && !result) {
        DllMain(instance, DLL_PROCESS_DETACH, reserved);
        dllmain_crt_dispatch(instance, DLL_PROCESS_DETACH, reserved);
        dllmain_raw(instance, DLL_PROCESS_DETACH, reserved);
    }

    if (reason == DLL_PROCESS_DETACH || reason == DLL_THREAD_DETACH) {
        result = dllmain_crt_dispatch(instance, reason, reserved);
        if (!result) {
            return result;
        }

        result = dllmain_raw(instance, reason, reserved);
        if (!result) {
            return result;
        }
    }

    return result;
}

extern "C" {
#pragma function(memset)
void *memset(void *dest, int c, size_t count) {
    fill_memory(dest, c, count);
    return dest;
}

#pragma function(memcpy)
void *memcpy(void *dest, const void *src, size_t count) {
    copy_memory(dest, src, count);
    return dest;
}
}

#endif
