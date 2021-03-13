#pragma once

typedef void(__cdecl *_PVFV)(void);
typedef int(__cdecl *_PIFV)(void);
typedef void(__cdecl *_PVFI)(int);

// These tables are filled by the linker and are usually called before the main function by the CRT.
// Since we don't have the CRT we have to call them manually.

//
// Section attributes
// Taken from: "vcruntime/internal_shared.h", commented out stuff we don't need. I still don't know what pogo is...
//
#pragma section(".CRT$XCA", long, read)  // First C++ Initializer
#pragma section(".CRT$XCAA", long, read)  // Startup C++ Initializer
#pragma section(".CRT$XCZ", long, read)  // Last C++ Initializer

#pragma section(".CRT$XDA", long, read)  // First Dynamic TLS Initializer
#pragma section(".CRT$XDZ", long, read)  // Last Dynamic TLS Initializer

#pragma section(".CRT$XIA", long, read)  // First C Initializer
// #pragma section(".CRT$XIAA", long, read)  // Startup C Initializer
// #pragma section(".CRT$XIAB", long, read)  // PGO C Initializer
// #pragma section(".CRT$XIAC", long, read)  // Post-PGO C Initializer
#pragma section(".CRT$XIC", long, read)  // CRT C Initializers
// #pragma section(".CRT$XIYA", long, read)  // VCCorLib Threading Model Initializer
// #pragma section(".CRT$XIYAA", long, read)  // XAML Designer Threading Model Override Initializer
// #pragma section(".CRT$XIYB", long, read)  // VCCorLib Main Initializer
#pragma section(".CRT$XIZ", long, read)  // Last C Initializer

#pragma section(".CRT$XLA", long, read)  // First Loader TLS Callback
#pragma section(".CRT$XLC", long, read)  // CRT TLS Constructor
#pragma section(".CRT$XLD", long, read)  // CRT TLS Terminator
#pragma section(".CRT$XLZ", long, read)  // Last Loader TLS Callback

#pragma section(".CRT$XPA", long, read)  // First Pre-Terminator
// #pragma section(".CRT$XPB", long, read)  // CRT ConcRT Pre-Terminator
// #pragma section(".CRT$XPX", long, read)  // CRT Pre-Terminators
// #pragma section(".CRT$XPXA", long, read)  // CRT stdio Pre-Terminator
#pragma section(".CRT$XPZ", long, read)  // Last Pre-Terminator

#pragma section(".CRT$XTA", long, read)  // First Terminator
#pragma section(".CRT$XTZ", long, read)  // Last Terminator

// #pragma section(".CRTMA$XCA", long, read)  // First Managed C++ Initializer
// #pragma section(".CRTMA$XCZ", long, read)  // Last Managed C++ Initializer

// #pragma section(".CRTVT$XCA", long, read)  // First Managed VTable Initializer
// #pragma section(".CRTVT$XCZ", long, read)  // Last Managed VTable Initializer

#pragma section(".rdata$T", long, read)

// #pragma section(".rtc$IAA", long, read)  // First RTC Initializer
// #pragma section(".rtc$IZZ", long, read)  // Last RTC Initializer
//
// #pragma section(".rtc$TAA", long, read)  // First RTC Terminator
// #pragma section(".rtc$TZZ", long, read)  // Last RTC Terminator
#define _CRTALLOC(x) __declspec(allocate(x))

#pragma comment(linker, "/merge:.CRT=.rdata")

// We prefix these with "lstd_" because Windows.h includes corecrt_startup.h with declares _initterm_ and _initterm_e_ as dllimport. Sigh. 
extern "C" void __cdecl lstd_initterm(_PVFV *const first, _PVFV *const last);
extern "C" int __cdecl lstd_initterm_e(_PIFV *const first, _PIFV *const last);

// Taken from "ucrt/process.h":
typedef void(__stdcall *_tls_callback_type)(void *, unsigned long, void *);
