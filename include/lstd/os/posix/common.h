#pragma once

#include "../../fmt.h"
#include "../../variant.h"
#include "../../memory.h"
#include "../../writer.h"

#include "../path.h"
#include "../thread.h"

#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <limits.h>
#include <sys/time.h>
#include <stdio.h>
#include <libproc.h>
#include <string.h>

LSTD_BEGIN_NAMESPACE

void platform_report_error(string message, source_location loc = source_location::current());

inline void report_warning_no_allocations(string message)
{
    string preMessage = ">>> Warning (in windows_common.cpp): ";
    write(STDERR_FILENO, preMessage.Data, preMessage.Count);

    write(STDERR_FILENO, message.Data, message.Count);

    string postMessage = ".\n";
    write(STDERR_FILENO, postMessage.Data, postMessage.Count);
}

inline void setup_console()
{
    // Set terminal to use UTF-8 encoding
    if (setenv("LANG", "en_US.UTF-8", 1) == -1)
    {
        // Handle error setting locale, if necessary
        report_warning_no_allocations("Couldn't set console locale to UTF-8 - some characters might be messed up");
    }

    // Enable ANSI escape sequences for the terminal
    struct termios term;
    if (tcgetattr(STDOUT_FILENO, &term) != -1)
    {
        term.c_lflag |= ECHOCTL | ECHOKE;
        if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &term) == -1)
        {
            report_warning_no_allocations("Couldn't set ANSI escape chars console attributes - some characters might be messed up");
        }
    }
    else
    {
        report_warning_no_allocations("Couldn't set ANSI escape chars console attributes - some characters might be messed up");
    }
}

inline const u32 ERROR_INSUFFICIENT_BUFFER = 122;

inline time_t os_get_time()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

inline f64 os_time_to_seconds(time_t time)
{
    return (double)time / 1000000.0;
}

inline string os_get_working_dir()
{
    char dir[PATH_MAX];
    if (getcwd(dir, sizeof(dir)) != nullptr)
    {
        lock(&S->WorkingDirMutex);
        defer(unlock(&S->WorkingDirMutex));

        PUSH_ALLOC(PERSISTENT) { 
            free(S->WorkingDir);
            S->WorkingDir = path_normalize(dir); 
        }
        return S->WorkingDir;
    }
    else
    {
        report_warning_no_allocations("Couldn't get working directory");
        return "";
    }
}

inline void os_set_working_dir(string dir)
{
    assert(path_is_absolute(dir));

    if (chdir(to_c_string_temp(dir)) == -1)
    {
        report_warning_no_allocations("Couldn't set working directory");
    }
    else
    {
        lock(&S->WorkingDirMutex);
        defer(unlock(&S->WorkingDirMutex));

        PUSH_ALLOC(PERSISTENT) { S->WorkingDir = clone(dir); }
    }
}

inline const u32 ERROR_ENVVAR_NOT_FOUND = 203;

//
// @TODO: Cache environment variables when running the program in order to avoid
// allocating. Store them null-terminated in the cache, to avoid callers which
// expect C style strings having to convert.
//
mark_as_leak inline os_get_env_result os_get_env(string name, bool silent)
{
    const char *value = getenv(to_c_string_temp(name));
    if (value == nullptr)
    {
        if (!silent)
        {
            // Handle error: environment variable not found
            platform_report_error(tprint("Couldn't find environment variable with value \"{}\"", name));
        }
        return {"", false};
    }
    else
    {
        return {value, true};
    }
}

inline void os_set_env(string name, string value)
{
    // Can't use two to_c_string_temp calls because the second one might invalidate the first
    PUSH_ALLOC(PERSISTENT)
    {
        const char *name = to_c_string(name);
        defer(free(name));

        int ret = setenv(name, to_c_string_temp(value), 1);
        if (ret != 0)
        {
            platform_report_error("Failed to set env variable");
        }
    }
}

inline void os_remove_env(string name)
{
    int ret = unsetenv(to_c_string_temp(name));
    if (ret != 0)
    {
        platform_report_error("Failed to unset env variable");
    }
}

mark_as_leak inline string os_get_clipboard_content()
{
    FILE *pipe = popen("pbpaste", "r");
    if (!pipe)
    {
        platform_report_error("Failed to get clipboard");
        return "";
    }
    defer(pclose(pipe));

    string content;

    char buffer[128];
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            add(content, buffer);
        }
    }

    return content;
}

inline void os_set_clipboard_content(string content)
{
    FILE *pipe = popen("pbcopy", "w");
    if (!pipe)
    {
        platform_report_error("Failed to set clipboard");
    }
    defer(pclose(pipe));

    fputs(to_c_string_temp(content), pipe);
}

inline u32 os_get_hardware_concurrency() { return sysconf(_SC_NPROCESSORS_ONLN); }

inline u32 os_get_pid() { return getpid(); }

inline u64 os_get_current_thread_id() { return (u64)pthread_self(); }

inline string os_read_from_console_overwrite_previous_call()
{
    auto bytes = read(STDIN_FILENO, S->CinBuffer, S->CONSOLE_BUFFER_SIZE - 1);
    if (bytes == -1)
    {
        platform_report_error("Error reading from console");
        return "";
    }

    S->CinBuffer[bytes] = '\0';
    return S->CinBuffer;
}

mark_as_leak inline optional<string> os_read_entire_file(string path)
{
    FILE *file = fopen(to_c_string_temp(path), "rb");
    if (!file)
    {
        platform_report_error(tprint("Failed to open file \"{}\" for reading", path));
        return {};
    }
    defer(fclose(file));

    // Get the file size
    fseek(file, 0, SEEK_END);
    u64 size = ftell(file);
    rewind(file);

    string result;
    reserve(result, size);

    // Read the file contents into the buffer
    size_t bytesRead = fread(result.Data, 1, size, file);
    if (bytesRead != size)
    {
        platform_report_error(tprint("Failed to read entire file \"{}\"", path));
        free(result);
        return {};
    }
    result.Count = bytesRead;

    return result;
}

inline bool os_write_to_file(string path, string contents,
                             file_write_mode mode)
{
    FILE *file;
    if (mode == file_write_mode::Append)
        file = fopen(to_c_string_temp(path), "ab");
    else if (mode == file_write_mode::Overwrite)
        file = fopen(to_c_string_temp(path), "wb");
    else if (mode == file_write_mode::Overwrite_Entire)
        file = fopen(to_c_string_temp(path), "wb+");
    else
    {
        platform_report_error(tprint("Invalid file write mode {}", mode));
        return false;
    }

    if (!file)
    {
        platform_report_error(tprint("Failed to open file \"{}\" for writing", path));
        return false;
    }
    defer(fclose(file));

    size_t bytesWritten = fwrite(to_c_string_temp(contents), 1, length(contents), file);
    if (bytesWritten != length(contents))
    {
        platform_report_error(tprint("Failed to write to file \"{}\"", path));
        return false;
    }

    return true;
}

inline void console::write(const char *data, s64 size)
{
    if (LockMutex)
        lock(&S->CoutMutex);

    if (size > Available)
    {
        if (LockMutex) unlock(&S->CoutMutex);
        flush();
        if (LockMutex) lock(&S->CoutMutex);
    }

    memcpy(Current, data, size);

    Current += size;
    Available -= size;

    if (LockMutex)
        unlock(&S->CoutMutex);
}

inline void console::flush()
{
    if (LockMutex)
        lock(&S->CoutMutex);

    if (!Buffer)
    {
        if (OutputType == console::COUT)
        {
            Buffer = Current = S->CoutBuffer;
        }
        else
        {
            Buffer = Current = S->CerrBuffer;
        }

        BufferSize = Available = S->CONSOLE_BUFFER_SIZE;
    }

    auto target = OutputType == console::COUT ? stdout : stderr;
    fwrite(Buffer, sizeof(char), (size_t)(BufferSize - Available), target);

    Current = Buffer;
    Available = S->CONSOLE_BUFFER_SIZE;

    if (LockMutex)
        unlock(&S->CoutMutex);
}

inline void get_module_name()
{
    char buffer[PROC_PIDPATHINFO_MAXSIZE];

    if (proc_pidpath(getpid(), buffer, sizeof(buffer)) <= 0)
    {
        platform_report_error("Error in proc_pidpath");
        buffer[0] = 0;
    }

    free(S->ModuleName);
    PUSH_ALLOC(PERSISTENT) { S->ModuleName = path_normalize(string(buffer)); }
}

inline void parse_arguments()
{
    return;

    char *command_line = strdup(getenv("CMDLINE"));
    if (command_line == NULL)
    {
        report_warning_no_allocations("Couldn't parse command line arguments");
        return;
    }

    char *token;
    char *saveptr;
    s32 argc = 0;

    char **argv = (char **)malloc(sizeof(char *)); // Allocate memory for an array of pointers
    defer(free(argv));

    for (token = strtok_r(command_line, " ", &saveptr); token != NULL; token = strtok_r(NULL, " ", &saveptr))
    {
        argv[argc] = strdup(token);
        argc++;
        argv = (char **)realloc(argv, (argc + 1) * sizeof(char *)); // Resize the array of pointers
    }

    argv[argc] = NULL; // Null-terminate the array

    // Store the arguments
    PUSH_ALLOC(PERSISTENT)
    {
        reserve(S->Argv, argc);
        // Loop over all arguments and add them, skip the .exe name
        for (int i = 1; i < argc; ++i)
        {
            add(S->Argv, argv[i]);
        }
    }
}

inline void platform_specific_init_common_state()
{
    setup_console();

    get_module_name();

    parse_arguments();
}

LSTD_END_NAMESPACE
