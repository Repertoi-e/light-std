#pragma once

#include "api.h"  // Declarations of Win32 functions

#include "../../fmt.h"
#include "../../variant.h"
#include "../../memory.h"
#include "../../path.h"
#include "../../writer.h"
#include "thread.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;
#define MODULE_HANDLE ((HMODULE)&__ImageBase)

#if defined LSTD_NO_CRT
extern "C" {
// Exits the application with the given exit code.
// Also runs all callbacks registered with exit_schedule().
void exit(s32 exitCode = 0);

// Produces an abnormal program termination.
// Doesn't call any callbacks registered with exit_schedule().
void abort();

// Schedule a function to run when this module exists - before any global C++
// destructors are called and before the CRT (if we are linking against it)
// uninitializes.
//
// Also runs when calling exit(exitCode).
//
// Note: We don't try to be as general as possible. exit_schedule() is merely
// a utility that might be useful to ensure e.g. files are flushed or handles
// closed. (Don't use this for freeing memory, the OS claims it back
// anyway!!).
void atexit(void (*func)(void));
}
#else
#include <stdlib.h>
#endif

LSTD_BEGIN_NAMESPACE

inline LARGE_INTEGER Win32PerformanceFrequency;  // Used to time stuff

// Windows uses wchar.. Sigh...
//
// This function uses the platform temporary allocator if no explicit allocator
// was specified.
inline wchar *platform_utf8_to_utf16(string str, allocator alloc = {}) {
  if (!str.Count) return null;

  if (!alloc) alloc = S->TempAlloc;

  wchar *result;
  PUSH_ALLOC(alloc) {
    // src.Length * 2 because one unicode character might take 2 wide chars.
    // This is just an approximation, not all space will be used!
    result = malloc<wchar>({.Count = length(str) * 2 + 1});
  }

  utf8_to_utf16(str.Data, length(str), result);
  return result;
}

// This function uses the platform temporary allocator if no explicit allocator
// was specified.
inline string platform_utf16_to_utf8(const wchar *str, allocator alloc = {}) {
  string result;

  if (!alloc) alloc = S->TempAlloc;

  PUSH_ALLOC(alloc) {
    // String length * 4 because one unicode character might take 4 bytes in
    // utf8. This is just an approximation, not all space will be used!
    reserve(result, c_string_length(str) * 4);
  }

  utf16_to_utf8(str, (char *)result.Data, &result.Count);

  return result;
}

inline void report_warning_no_allocations(string message) {
  DWORD ignored;

  string preMessage = ">>> Warning (in windows_common.cpp): ";
  WriteFile(S->CerrHandle, preMessage.Data, (DWORD)preMessage.Count, &ignored, null);

  WriteFile(S->CerrHandle, message.Data, (DWORD)message.Count, &ignored, null);

  string postMessage = ".\n";
  WriteFile(S->CerrHandle, postMessage.Data, (DWORD)postMessage.Count, &ignored, null);
}

inline void setup_console() {
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text
    CONSOLE_SCREEN_BUFFER_INFO cInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cInfo);
    cInfo.dwSize.Y = 500;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), cInfo.dwSize);
  }

  S->CinHandle = GetStdHandle(STD_INPUT_HANDLE);
  S->CoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  S->CerrHandle = GetStdHandle(STD_ERROR_HANDLE);

  if (!SetConsoleOutputCP(CP_UTF8)) {
    report_warning_no_allocations(
        "Couldn't set console code page to UTF8 - "
        "some characters might be messed up");
  }

  // Enable ANSI escape sequences for the console
  DWORD dw = 0;
  GetConsoleMode(S->CoutHandle, &dw);
  SetConsoleMode(S->CoutHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

  GetConsoleMode(S->CerrHandle, &dw);
  SetConsoleMode(S->CerrHandle, dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

inline const u32 ERROR_INSUFFICIENT_BUFFER = 122;

inline time_t os_get_time() {
  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return count.QuadPart;
}

inline f64 os_time_to_seconds(time_t time) {
  return (f64)time / Win32PerformanceFrequency.QuadPart;
}

inline string os_get_working_dir() {
  DWORD required = GetCurrentDirectoryW(0, null);

  auto *dir16 = malloc<wchar>({.Count = required + 1, .Alloc = TEMP});
  defer(free(dir16));
  if (!GetCurrentDirectoryW(required + 1, dir16)) {
    windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()),
                                 "GetCurrentDirectory");
    return "";
  }

  string workingDir = utf16_to_utf8(dir16);
  defer(free(workingDir));

  lock(&S->WorkingDirMutex);
  defer(unlock(&S->WorkingDirMutex));

  PUSH_ALLOC(PERSISTENT) { 
    free(S->WorkingDir);
    S->WorkingDir = path_normalize(workingDir); 
  }
  return S->WorkingDir;
}

inline void os_set_working_dir(string dir) {
  assert(path_is_absolute(dir));

  WIN32_CHECK_BOOL(r, SetCurrentDirectoryW(utf8_to_utf16(dir)));

  lock(&S->WorkingDirMutex);
  defer(unlock(&S->WorkingDirMutex));

  PUSH_ALLOC(PERSISTENT) { S->WorkingDir = clone(dir); }
}

inline const u32 ERROR_ENVVAR_NOT_FOUND = 203;

//
// @TODO: Cache environment variables when running the program in order to avoid
// allocating. Store them null-terminated in the cache, to avoid callers which
// expect C style strings having to convert.
//
inline mark_as_leak os_get_env_result os_get_env(string name, bool silent) {
  auto *name16 = utf8_to_utf16(name, PERSISTENT);
  defer(free(name16));

  DWORD bufferSize =
      65535;  // Limit according to
              // http://msdn.microsoft.com/en-us/library/ms683188.aspx

  auto *buffer = malloc<wchar>({.Count = bufferSize, .Alloc = TEMP});
  auto r = GetEnvironmentVariableW(name16, buffer, bufferSize);

  if (r == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
    if (!silent) {
      platform_report_error(
          tprint("Couldn't find environment variable with value \"{}\"", name));
    }
    return {"", false};
  }

  // 65535 may be the limit but let's not take risks
  if (r > bufferSize) {
    buffer = malloc<wchar>({.Count = r, .Alloc = TEMP});
    GetEnvironmentVariableW(name16, buffer, r);
    bufferSize = r;

    // Possible to fail a second time ?
  }

  return {utf16_to_utf8(buffer, PERSISTENT), true};
}

inline void os_set_env(string name, string value) {
  if (length(value) > 32767) {
    // @Cleanup: The docs say windows doesn't allow that but we should test it.
    assert(false);
  }

  WIN32_CHECK_BOOL(
      r, SetEnvironmentVariableW(utf8_to_utf16(name), utf8_to_utf16(value)));
}

inline void os_remove_env(string name) {
  WIN32_CHECK_BOOL(r, SetEnvironmentVariableW(utf8_to_utf16(name), null));
}

inline mark_as_leak string os_get_clipboard_content() {
  if (!OpenClipboard(null)) {
    platform_report_error("Failed to open clipboard");
    return "";
  }
  defer(CloseClipboard());

  HANDLE object = GetClipboardData(CF_UNICODETEXT);
  if (!object) {
    platform_report_error("Failed to convert clipboard to string");
    return "";
  }

  auto *clipboard16 = (wchar *)GlobalLock(object);
  if (!clipboard16) {
    platform_report_error("Failed to lock global handle");
    return "";
  }
  defer(GlobalUnlock(object));

  return utf16_to_utf8(clipboard16, PERSISTENT);
}

inline void os_set_clipboard_content(string content) {
  HANDLE object =
      GlobalAlloc(GMEM_MOVEABLE, length(content) * 2 * sizeof(wchar));
  if (!object) {
    platform_report_error("Failed to open clipboard");
    return;
  }
  defer(GlobalFree(object));

  auto *clipboard16 = (wchar *)GlobalLock(object);
  if (!clipboard16) {
    platform_report_error("Failed to lock global handle");
    return;
  }

  utf8_to_utf16(content.Data, length(content), clipboard16);
  GlobalUnlock(object);

  if (!OpenClipboard(null)) {
    platform_report_error("Failed to open clipboard.");
    return;
  }
  defer(CloseClipboard());

  EmptyClipboard();
  SetClipboardData(CF_UNICODETEXT, object);
  CloseClipboard();
}

inline u32 os_get_hardware_concurrency() {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwNumberOfProcessors;
}

inline u32 os_get_pid() { return (u32)GetCurrentProcessId(); }

inline u64 os_get_current_thread_id() { return GetCurrentThreadId(); }

inline string os_read_from_console_overwrite_previous_call() {
  DWORD read;
  ReadFile(S->CinHandle, S->CinBuffer, (DWORD)S->CONSOLE_BUFFER_SIZE, &read,
           null);
  return string(S->CinBuffer, (s64)read);
}

// @TODO @Clarity: Print more useful message about the path
// :CopyAndPaste from path.nt
#define CREATE_MAPPING_CHECKED(handleName, call)                              \
  HANDLE handleName = call;                                                   \
  if (!handleName) {                                                          \
    string extendedCallSite = sprint(                                         \
        "{}\n        (the name was: {!YELLOW}\"{}\"{!GRAY})\n", #call, name); \
    char *cStr = to_c_string(extendedCallSite);                               \
    windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), cStr);   \
    free(cStr);                                                               \
    free(extendedCallSite);                                                   \
    return;                                                                   \
  }

inline mark_as_leak optional<string> os_read_entire_file(string path) {
  CREATE_FILE_HANDLE_CHECKED(
      file,
      CreateFileW(utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ, null,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null),
      {});
  defer(CloseHandle(file));

  LARGE_INTEGER size = {0};
  GetFileSizeEx(file, &size);

  string result;
  reserve(result, size.QuadPart);
  DWORD bytesRead;
  if (!ReadFile(file, result.Data, (u32)size.QuadPart, &bytesRead, null))
  {
    free(result);
    return {};
  }
  assert(size.QuadPart == bytesRead);

  result.Count = bytesRead;
  return result;
}

inline bool os_write_to_file(string path, string contents,
                             file_write_mode mode) {
  CREATE_FILE_HANDLE_CHECKED(
      file,
      CreateFileW(utf8_to_utf16(path), GENERIC_WRITE, 0, null, OPEN_ALWAYS,
                  FILE_ATTRIBUTE_NORMAL, null),
      false);
  defer(CloseHandle(file));

  LARGE_INTEGER pointer = {};
  pointer.QuadPart = 0;
  if (mode == file_write_mode::Append)
    SetFilePointerEx(file, pointer, null, FILE_END);
  if (mode == file_write_mode::Overwrite_Entire) SetEndOfFile(file);

  DWORD bytesWritten;
  if (!WriteFile(file, contents.Data, (u32)contents.Count, &bytesWritten, null))
    return false;
  if (bytesWritten != contents.Count) return false;
  return true;
}

inline void console::write(const char *data, s64 size) {
  if (LockMutex) lock(&S->CoutMutex);

  if (size > Available) {
    flush();
  }

  memcpy(Current, data, size);

  Current += size;
  Available -= size;

  if (LockMutex) unlock(&S->CoutMutex);
}

inline void console::flush() {
  if (LockMutex) lock(&S->CoutMutex);

  if (!Buffer) {
    if (OutputType == console::COUT) {
      Buffer = Current = S->CoutBuffer;
    } else {
      Buffer = Current = S->CerrBuffer;
    }

    BufferSize = Available = S->CONSOLE_BUFFER_SIZE;
  }

  HANDLE target = OutputType == console::COUT ? S->CoutHandle : S->CerrHandle;

  DWORD ignored;
  WriteFile(target, Buffer, (DWORD)(BufferSize - Available), &ignored, null);

  Current = Buffer;
  Available = S->CONSOLE_BUFFER_SIZE;

  if (LockMutex) unlock(&S->CoutMutex);
}

inline void get_module_name() {
  // Get the module name
  wchar *buffer = malloc<wchar>({.Count = MAX_PATH, .Alloc = PERSISTENT});
  defer(free(buffer));

  s64 reserved = MAX_PATH;

  while (true) {
    s64 written = GetModuleFileNameW(MODULE_HANDLE, buffer, (DWORD)reserved);
    if (written == reserved) {
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        reserved *= 2;
        free(buffer);
        buffer = malloc<wchar>({.Count = reserved, .Alloc = PERSISTENT});
        continue;
      }
    }
    break;
  }

  string moduleName = utf16_to_utf8(buffer);
  PUSH_ALLOC(PERSISTENT) { S->ModuleName = path_normalize(moduleName); }
}

inline void parse_arguments() {
  wchar **argv;
  s32 argc;

  // @Cleanup @DependencyCleanup: Parse arguments ourselves? We depend on this
  // function which is in a library we reference ONLY because of this one
  // function.
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv == null) {
    report_warning_no_allocations(
        "Couldn't parse command line arguments, "
        "os_get_command_line_arguments() will return "
        "an empty array in all cases");
    return;
  }

  defer(LocalFree(argv));

  PUSH_ALLOC(PERSISTENT) {
    s32 n = argc - 1;
    if (n > 0) {
      reserve(S->Argv, n);
    }
  }

  // Loop over all arguments and add them, skip the .exe name
  For(range(1, argc)) add(S->Argv, utf16_to_utf8(argv[it], PERSISTENT));
}

// This needs to be called when our program runs, but also when a new thread
// starts! See windows_common.cpp for implementation details. Note: You
// shouldn't ever call this.
inline void platform_init_context() {
  auto newContext = context(context::dont_init_t{});
  newContext.ThreadID = GetCurrentThreadId();
  newContext.Alloc = {};
  newContext.AllocAlignment = POINTER_SIZE;
  newContext.AllocOptions = 0;
  newContext.LogAllAllocations = false;
  newContext.PanicHandler = default_panic_handler;
  newContext.Log = &cout;
  newContext.FmtDisableAnsiCodes = false;
#if defined DEBUG_MEMORY
  newContext.DebugMemoryHeapVerifyFrequency = 255;
  newContext
      .DebugMemoryPrintListOfUnfreedAllocationsAtThreadExitOrProgramTermination =
      false;
#endif
  newContext.FmtParseErrorHandler = fmt_default_parse_error_handler;
  newContext._HandlingPanic = false;
  newContext._LoggingAnAllocation = false;
  OVERRIDE_CONTEXT(newContext);

  *const_cast<allocator *>(&TemporaryAllocator) = {
      arena_allocator, (void *)&TemporaryAllocatorData};
}

inline void platform_specific_init_common_state()
{
  QueryPerformanceFrequency(&Win32PerformanceFrequency);

  setup_console()

  get_module_name();

  parse_arguments();
}

LSTD_END_NAMESPACE

#if defined LSTD_NO_CRT
extern "C" {
inline void exit(s32 exitCode) {
  // :PlatformExitTermination

  // We can't call this from a DLL because of ExitProcess.
  // Search for :PlatformExitTermination to see the other place we call this set
  // of functions.
  lock(&S->ExitScheduleMutex);
  For(S->ExitFunctions) it();
  unlock(&S->ExitScheduleMutex);

  ExitProcess(exitCode);
}

inline void abort() {
  // Don't do any cleanup, just exit
  ExitProcess(3);
}

inline void atexit(void (*function)(void)) {
  lock(&S->ExitScheduleMutex);

  // @Cleanup Lock-free list
  PUSH_ALLOC(PERSISTENT) {
    reserve(S->ExitFunctions);
    add(S->ExitFunctions, function);
  }

  unlock(&S->ExitScheduleMutex);
}
}

#endif

#undef MODULE_HANDLE
#undef CREATE_MAPPING_CHECKED
