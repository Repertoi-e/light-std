#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/os.h"
#include "lstd/types/windows.h"

import fmt;

LSTD_BEGIN_NAMESPACE

string utf16_to_utf8(const utf16 *str, allocator alloc = {});

string get_error_string(HRESULT hr) {
    if (!hr) return "No error"; 

    utf16 *message16 = null;

    size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &message16, 0, null);
    defer(LocalFree(message16));

    return utf16_to_utf8(message16);
}

void windows_report_hresult_error(u32 hresult, const string &call, source_location loc) {
    print("\n{!}>>> An error occured while calling a function returning an HRESULT.\n");
    print("    {!GRAY}{}{!}\n", call);
    print("        ... was called at {!YELLOW}{}:{} (in function: {}) {!} and returned {!GRAY}{:#x}\n", loc.File, loc.Line, loc.Function, hresult);
    print("        Error: {!RED}{}\n", get_error_string(hresult));
    print("               ");
    print("{!}\n\n");
}

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
