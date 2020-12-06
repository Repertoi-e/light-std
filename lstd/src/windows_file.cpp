#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/file.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

namespace file {

#define CREATE_FILE_HANDLE_CHECKED(handleName, call, returnOnFail)                                                  \
    HANDLE handleName = call;                                                                                       \
    if (handleName == INVALID_HANDLE_VALUE) {                                                                       \
        string extendedCallSite = fmt::sprint("{}\n        (the path was: {!YELLOW}\"{}\"{!GRAY})\n", #call, Path); \
        defer(free(extendedCallSite));                                                                              \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), extendedCallSite, __FILE__, __LINE__);     \
        return returnOnFail;                                                                                        \
    }

file_scope utf16 *utf8_path_to_utf16(const string &path) {
    // @Bug path.Length is not enough (2 wide chars for one char)
    auto *result = allocate_array(utf16, path.Length + 1, Context.Temp);
    utf8_to_utf16(path.Data, path.Length, result);
    return result;
}

bool handle::is_file() const {
    HANDLE file = CreateFileW(utf8_path_to_utf16(Path), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    defer(CloseHandle(file));

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) {
        return false;
    }
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool handle::is_directory() const {
    HANDLE file = CreateFileW(utf8_path_to_utf16(Path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    defer(CloseHandle(file));

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) {
        return false;
    }
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool handle::exists() const {
    HANDLE file = CreateFileW(utf8_path_to_utf16(Path), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    CloseHandle(file);
    return true;
}

bool handle::is_symbolic_link() const {
    auto attribs = GetFileAttributesW(utf8_path_to_utf16(Path));
    if (attribs != INVALID_FILE_ATTRIBUTES) {
        return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }
    return false;
}

s64 handle::file_size() const {
    if (is_directory()) return 0;

    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_path_to_utf16(Path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, null), 0);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);
    return size.QuadPart;
}

#define GET_READONLY_EXISTING_HANDLE(x, fail)                                                                                                                                       \
    CREATE_FILE_HANDLE_CHECKED(x, CreateFileW(utf8_path_to_utf16(Path), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL), fail); \
    defer(CloseHandle(x));

time_t handle::creation_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, 0);
    FILETIME time;
    if (!GetFileTime(handle, &time, null, null)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

time_t handle::last_access_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, 0);
    FILETIME time;
    if (!GetFileTime(handle, null, &time, null)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

time_t handle::last_modification_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, 0);
    FILETIME time;
    if (!GetFileTime(handle, null, null, &time)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

bool handle::create_directory() const {
    if (exists()) return false;
    return CreateDirectoryW(utf8_path_to_utf16(Path), null);
}

bool handle::delete_file() const {
    if (!is_file()) return false;
    return DeleteFileW(utf8_path_to_utf16(Path));
}

bool handle::delete_directory() const {
    if (!is_directory()) return false;
    return RemoveDirectoryW(utf8_path_to_utf16(Path));
}

bool handle::copy(handle dest, bool overwrite) const {
    if (!is_file()) return false;

    auto *u16 = utf8_path_to_utf16(Path);

    if (dest.is_directory()) {
        auto p = path::join(dest.Path, path::base_name(Path));
        defer(free(p));

        // @Bug p.Length is not enough (2 wide chars for one char)
        auto *d = allocate_array(utf16, p.Length + 1, Context.Temp);
        utf8_to_utf16(p.Data, p.Length, d);

        return CopyFileW(u16, d, !overwrite);
    }
    return CopyFileW(u16, utf8_path_to_utf16(dest.Path), !overwrite);
}

bool handle::move(handle dest, bool overwrite) const {
    if (!is_file()) return false;

    auto p = dest.Path;
    if (dest.is_directory()) {
        auto p = path::join(dest.Path, path::base_name(Path));
        defer(free(p));

        // @Bug p.Length is not enough (2 wide chars for one char)
        auto *d = allocate_array(utf16, p.Length + 1, Context.Temp);
        utf8_to_utf16(p.Data, p.Length, d);

        if (MoveFileExW(utf8_path_to_utf16(Path), d, MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
            clone(&Path, p);
            return true;
        }
    } else {
        if (MoveFileExW(utf8_path_to_utf16(Path), utf8_path_to_utf16(dest.Path), MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
            clone(&Path, p);
            return true;
        }
    }
    return false;
}

bool handle::rename(const string &newName) const {
    if (!exists()) return false;

    auto p = path::join(path::directory(Path), newName);
    defer(free(p));

    // @Bug p.Length is not enough (2 wide chars for one char)
    auto *d = allocate_array(utf16, p.Length + 1, Context.Temp);
    utf8_to_utf16(p.Data, p.Length, d);

    if (MoveFileW(utf8_path_to_utf16(Path), d)) {
        Path = p;
        return true;
    }

    return false;
}

bool handle::create_hard_link(handle dest) const {
    if (!is_directory()) return false;
    if (!dest.is_directory()) return false;
    return CreateHardLinkW(utf8_path_to_utf16(dest.Path), utf8_path_to_utf16(Path), null);
}

bool handle::create_symbolic_link(handle dest) const {
    if (!exists()) return false;
    if (!dest.exists()) return false;
    return CreateSymbolicLinkW(utf8_path_to_utf16(dest.Path), utf8_path_to_utf16(Path), dest.is_directory() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
}

handle::read_entire_file_result handle::read_entire_file() const {
    read_entire_file_result fail = {array<byte>{}, false};
    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_path_to_utf16(Path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null), fail);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);

    array<byte> result;
    reserve(result, size.QuadPart);
    DWORD bytesRead;
    if (!ReadFile(file, result.Data, (u32) size.QuadPart, &bytesRead, null)) return {{}, false};
    assert(size.QuadPart == bytesRead);

    result.Count += bytesRead;
    return {result, true};
}

bool handle::write_to_file(const string &contents, write_mode mode) const {
    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_path_to_utf16(Path), GENERIC_WRITE, 0, null, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, null), false);
    defer(CloseHandle(file));

    LARGE_INTEGER pointer = {};
    pointer.QuadPart = 0;
    if (mode == write_mode::Append) SetFilePointerEx(file, pointer, null, FILE_END);
    if (mode == write_mode::Overwrite_Entire) SetEndOfFile(file);

    DWORD bytesWritten;
    if (!WriteFile(file, contents.Data, (u32) contents.Count, &bytesWritten, null)) return false;
    if (bytesWritten != contents.Count) return false;
    return true;
}

void handle::iterator::read_next_entry() {
    do {
        if (!Handle) {
            string queryPath = path::join(Path, "*");
            defer(free(queryPath));

            // @Bug queryPath.Length is not enough (2 wide chars for one char)
            auto *query = allocate_array(utf16, queryPath.Length + 1, Context.Temp);
            utf8_to_utf16(queryPath.Data, queryPath.Length, query);

            CREATE_FILE_HANDLE_CHECKED(file, FindFirstFileW(query, (WIN32_FIND_DATAW *) PlatformFileInfo), ;);
            Handle = (void *) file;
        } else {
#define CHECK_FIND_NEXT(call)                                                                            \
    if (!call) {                                                                                         \
        if (GetLastError() != ERROR_NO_MORE_FILES) {                                                     \
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call, __FILE__, __LINE__); \
        }                                                                                                \
        if (Handle != INVALID_HANDLE_VALUE) {                                                            \
            WIN32_CHECKBOOL(FindClose((HANDLE) Handle));                                                 \
        }                                                                                                \
                                                                                                         \
        Handle = null;                                                                                   \
        return;                                                                                          \
    }
            CHECK_FIND_NEXT(FindNextFileW((HANDLE) Handle, (WIN32_FIND_DATAW *) PlatformFileInfo));
        }
        ++Index;

        free(CurrentFileName);

        auto *fileName = ((WIN32_FIND_DATAW *) PlatformFileInfo)->cFileName;
        reserve(CurrentFileName, c_string_length(fileName) * 2);  // @Bug c_string_length * 2 is not enough
        utf16_to_utf8(fileName, const_cast<utf8 *>(CurrentFileName.Data), &CurrentFileName.Count);
        CurrentFileName.Length = utf8_length(CurrentFileName.Data, CurrentFileName.Count);
    } while (CurrentFileName == ".." || CurrentFileName == ".");
    assert(CurrentFileName != ".." && CurrentFileName != ".");
}

void handle::traverse_impl(const delegate<void(const string &)> &func) const {
    for (auto it = begin(); it != end(); ++it) {
        string relativeFileName = path::join(Path, *it);
        defer(free(relativeFileName));
        func(relativeFileName);
    }
}

static string get_path_from_here_to(const string &here, const string &there) {
    assert(path::is_sep(here[-1]) && path::is_sep(there[-1]));

    if (find_substring(here, there) == -1) {
        return there;
    } else {
        if (here.Length == there.Length) {
            return here;
        } else {
            string difference = substring(there, here.Length, there.Length);
            return difference;
        }
    }
}

void handle::traverse_recursively_impl(const string &first, const string &currentDirectory, const delegate<void(const string &)> &func) const {
    for (auto it = begin(); it != end(); ++it) {
        string relativeFileName = path::join(currentDirectory, *it);
        defer(free(relativeFileName));

        func(relativeFileName);

        if ((((WIN32_FIND_DATA *) it.PlatformFileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            auto pComponents = to_stack_array(get_path_from_here_to(first, currentDirectory), *it, "./");

            string p = path::join(pComponents);
            defer(free(p));

            handle(p).traverse_recursively_impl(first, p, func);
        }
    }
}

}  // namespace file

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
