#pragma once

//
// This file includes declarations for the Win32/64 API we use, as well as helper macros for error reporting.
//

#include "../common.h"

// @Hack... dumb windows headers
#if !defined LSTD_JUST_DX
#include "windows_api.h"
#endif

LSTD_BEGIN_NAMESPACE

// Logs a formatted error message.
void windows_report_hresult_error(u32 hresult, const char *apiFunction, source_location loc = source_location::current());

LSTD_END_NAMESPACE

// If the returned HRESULT is less than zero, reports an error.
#define WIN32_CHECK_HR(result, call) \
    u32 result = call;               \
    if (result < 0) LSTD_NAMESPACE::windows_report_hresult_error(result, #call);

// If the returned is false, reports an error.
#define WIN32_CHECK_BOOL(result, call) \
    auto result = call;                \
    if (!result) LSTD_NAMESPACE::windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call);

#define CREATE_FILE_HANDLE_CHECKED(handleName, call, returnOnFail)                                             \
    HANDLE handleName = call;                                                                                  \
    if (handleName == INVALID_HANDLE_VALUE) {                                                                  \
        string extendedCallSite = sprint("{}\n        (the path was: {!YELLOW}\"{}\"{!GRAY})\n", #call, path); \
        char *cStr              = to_c_string(extendedCallSite);                                        \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), cStr);                                \
        free(extendedCallSite);                                                                           \
        free(cStr);                                                                                            \
        return returnOnFail;                                                                                   \
    }

// DX_CHECK is used for checking directx calls. The difference from WIN32_CHECK_HR is that
// in Release configuration, the macro expands to just the call (no error checking).
#if !defined NDEBUG
#define DX_CHECK(call) WIN32_CHECK_HR(call)
#else
#define DX_CHECK(call) call
#endif

#define COM_SAFE_RELEASE(x) \
    if (x) {                \
        x->Release();       \
        x = null;           \
    }
