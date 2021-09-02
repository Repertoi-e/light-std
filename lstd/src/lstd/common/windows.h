#pragma once

//
// This file includes declarations for the Win32/64 API we use, as well as helper macros for error reporting.
//

#include "../common/common.h"
#include "windows_api.h"

LSTD_BEGIN_NAMESPACE

// Logs a formatted error message.
void windows_report_hresult_error(u32 hresult, const char *apiFunction, source_location loc = source_location::current());

LSTD_END_NAMESPACE

// CHECKHR checks the return value of _call_ and if the returned HRESULT is less than zero, reports an error.
#define WIN_CHECKHR(call)                                                            \
    {                                                                                \
        u32 result = call;                                                           \
        if (result < 0) LSTD_NAMESPACE::windows_report_hresult_error(result, #call); \
    }

// CHECKHR_BOOL checks the return value of _call_ and if the returned is false, reports an error.
#define WIN_CHECKBOOL(call)                                                                                   \
    {                                                                                                         \
        bool result = call;                                                                                   \
        if (!result) LSTD_NAMESPACE::windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call); \
    }

// DX_CHECK is used for checking directx calls. The difference from WIN_CHECKHR is that
// in Release configuration, the macro expands to just the call (no error checking).
#if not defined NDEBUG
#define DX_CHECK(call) WIN_CHECKHR(call)
#else
#define DX_CHECK(call) call
#endif

#define COM_SAFE_RELEASE(x) \
    if (x) {                \
        x->Release();       \
        x = null;           \
    }
