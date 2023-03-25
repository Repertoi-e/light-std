#include "lstd/common.h"

#if OS == WINDOWS

#include "windows.h"  // Declarations of Win32 functions

import lstd.fmt;
import lstd.os;

LSTD_BEGIN_NAMESPACE

string get_error_string(HRESULT hr) {
    if (!hr) return "No error";

    wchar *message16 = null;

    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message16, 0, null);
    defer(LocalFree(message16));

    return platform_utf16_to_utf8(message16);
}

void windows_report_hresult_error(u32 hresult, const char *apiFunction, source_location loc) {
    print("\n{!}>>> An error occured while calling a Windows function.\n");
    print("    {!GRAY}{}{!}\n", apiFunction);
    print("        ... was called at {!YELLOW}{}:{}{!} (in function: {!YELLOW}{}{!}) and returned error code {!GRAY}{:#x}\n", loc.File, loc.Line, loc.Function, hresult);
    print("        Error: {!RED}{}\n", get_error_string(hresult));
    print("               ");
    print("{!}\n\n");
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
