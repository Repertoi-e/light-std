#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/file.h"
#include "lstd/io/fmt.h"
#include "lstd/os.h"

#undef MAC
#undef _MAC
#include <Windows.h>

LSTD_BEGIN_NAMESPACE

namespace file {

#define CREATE_FILE_HANDLE_CHECKED(handleName, call, returnOnFail)                                              \
    HANDLE handleName = call;                                                                                   \
    if (handleName == INVALID_HANDLE_VALUE) {                                                                   \
        string extendedCallSite;                                                                                \
        fmt::sprint(&extendedCallSite, "{}\n        (the path was: {!YELLOW}\"{}\"{!GRAY})\n", #call, Path);    \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), extendedCallSite, __FILE__, __LINE__); \
        return returnOnFail;                                                                                    \
    }

handle::handle(path path) {
    clone(const_cast<file::path *>(&Path), path);

    // @Bug path.UnifiedPath.Length is not enough (2 wide chars for one char)
    Utf16Path = (wchar_t *) Context.Alloc.allocate((path.UnifiedPath.Length + 1) * sizeof(wchar_t));
    encode_owner(Utf16Path, this);

    utf8_to_utf16(path.UnifiedPath.Data, path.UnifiedPath.Length, Utf16Path);
}

bool handle::is_file() const {
    HANDLE file = CreateFileW(Utf16Path, 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
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
    HANDLE file =
        CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
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
    HANDLE file = CreateFileW(Utf16Path, 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    CloseHandle(file);
    return true;
}

bool handle::is_symbolic_link() const {
    auto attribs = GetFileAttributesW(Utf16Path);
    if (attribs != INVALID_FILE_ATTRIBUTES) {
        return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }
    return false;
}

size_t handle::file_size() const {
    if (is_directory()) return 0;

    CREATE_FILE_HANDLE_CHECKED(
        file, CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, null),
        0);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);
    return (size_t) size.QuadPart;
}

#define GET_READONLY_EXISTING_HANDLE(x, fail)                                                                 \
    CREATE_FILE_HANDLE_CHECKED(x,                                                                             \
                               CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, null, \
                                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL),                       \
                               fail);                                                                         \
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
    return CreateDirectoryW(Utf16Path, null);
}

bool handle::delete_file() const {
    if (!is_file()) return false;
    return DeleteFileW(Utf16Path);
}

bool handle::delete_directory() const {
    if (!is_directory()) return false;
    return RemoveDirectoryW(Utf16Path);
}

bool handle::copy(handle dest, bool overwrite) const {
    if (!is_file()) return false;

    if (dest.is_directory()) {
        auto p = dest.Path;
        p.combine_with(Path.file_name());

        // @Bug p.UnifiedPath.Length is not enough (2 wide chars for one char)
        auto *d = new (Context.TemporaryAlloc) wchar_t[p.UnifiedPath.Length + 1];
        utf8_to_utf16(p.UnifiedPath.Data, p.UnifiedPath.Length, d);

        return CopyFileW(Utf16Path, d, !overwrite);
    }
    return CopyFileW(Utf16Path, dest.Utf16Path, !overwrite);
}

bool handle::move(handle dest, bool overwrite) const {
    if (!is_file()) return false;

    auto p = dest.Path;
    if (dest.is_directory()) {
        p.combine_with(Path.file_name());

        // @Bug p.UnifiedPath.Length is not enough (2 wide chars for one char)
        auto *d = new (Context.TemporaryAlloc) wchar_t[p.UnifiedPath.Length + 1];
        utf8_to_utf16(p.UnifiedPath.Data, p.UnifiedPath.Length, d);

        if (MoveFileExW(Utf16Path, d, MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
            ::move(const_cast<path *>(&Path), &p);
            return true;
        }
    } else {
        if (MoveFileExW(Utf16Path, dest.Utf16Path,
                        MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
            ::clone(const_cast<path *>(&Path), p);
            return true;
        }
    }
    return false;
}

bool handle::rename(string newName) const {
    if (!exists()) return false;

    auto p = path(Path.directory());
    p.combine_with(newName);

    // @Bug p.UnifiedPath.Length is not enough (2 wide chars for one char)
    auto *d = new (Context.TemporaryAlloc) wchar_t[p.UnifiedPath.Length + 1];
    utf8_to_utf16(p.UnifiedPath.Data, p.UnifiedPath.Length, d);

    if (MoveFileW(Utf16Path, d)) {
        ::move(const_cast<path *>(&Path), &p);
        return true;
    }

    return false;
}

bool handle::create_hard_link(handle dest) const {
    if (!is_directory()) return false;
    if (!dest.is_directory()) return false;
    return CreateHardLinkW(dest.Utf16Path, Utf16Path, null);
}

bool handle::create_symbolic_link(handle dest) const {
    if (!exists()) return false;
    if (!dest.exists()) return false;
    return CreateSymbolicLinkW(dest.Utf16Path, Utf16Path, dest.is_directory() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
}

bool handle::read_entire_file(string *out) const {
    CREATE_FILE_HANDLE_CHECKED(
        file, CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null),
        null);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);

    out->reserve(out->ByteLength + size.QuadPart);
    char *target = const_cast<char *>(out->Data) + out->ByteLength;
    DWORD bytesRead;
    if (!ReadFile(file, target, (u32) size.QuadPart, &bytesRead, null)) return false;
    assert(size.QuadPart == bytesRead);

    out->ByteLength += bytesRead;
    out->Length += utf8_length(target, bytesRead);
    return true;
}

bool handle::write_to_file(string contents, write_mode mode) const {
    CREATE_FILE_HANDLE_CHECKED(
        file, CreateFileW(Utf16Path, GENERIC_WRITE, 0, null, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, null), false);
    defer(CloseHandle(file));

    LARGE_INTEGER pointer = {};
    pointer.QuadPart = 0;
    if (mode == write_mode::Append) SetFilePointerEx(file, pointer, null, FILE_END);
    if (mode == write_mode::Overwrite_Entire) SetEndOfFile(file);

    DWORD bytesWritten;
    if (!WriteFile(file, contents.Data, (u32) contents.ByteLength, &bytesWritten, null)) return false;
    if (bytesWritten != contents.ByteLength) return false;
    return true;
}

void handle::iterator::read_next_entry() {
    string fileName;
    do {
        if (!Handle) {
            file::path queryPath;
            clone(&queryPath, Path);
            queryPath.combine_with("*");

            // @Bug queryPath.UnifiedPath.Length is not enough (2 wide chars for one char)
            auto *query = new (Context.TemporaryAlloc) wchar_t[queryPath.UnifiedPath.Length + 1];
            utf8_to_utf16(queryPath.UnifiedPath.Data, queryPath.UnifiedPath.Length, query);

            CREATE_FILE_HANDLE_CHECKED(file, FindFirstFileW(query, (WIN32_FIND_DATAW *) PlatformFileInfo), ;);
            Handle = (void *) file;
        } else {
            if (!FindNextFileW((HANDLE) Handle, (WIN32_FIND_DATAW *) PlatformFileInfo)) {
                if (GetLastError() != ERROR_NO_MORE_FILES) {
                    windows_report_hresult_error(
                        HRESULT_FROM_WIN32(GetLastError()),
                        "FindNextFileW((HANDLE) Handle, (WIN32_FIND_DATAW *) PlatformFileInfo)", __FILE__, __LINE__);
                }
                if (Handle != INVALID_HANDLE_VALUE) {
                    if (!FindClose((HANDLE) Handle)) {
                        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), "FindClose((HANDLE) Handle)",
                                                     __FILE__, __LINE__);
                    }
                }
                Handle = null;
                return;
            }
        }
        ++Index;

        CurrentFileName.release();

        auto *fileName = ((WIN32_FIND_DATAW *) PlatformFileInfo)->cFileName;
        CurrentFileName.reserve(c_string_length(fileName) * 2); // @Bug c_string_length * 2 is not enough
        utf16_to_utf8(fileName, const_cast<char *>(CurrentFileName.Data), &CurrentFileName.ByteLength);
        CurrentFileName.Length = utf8_length(CurrentFileName.Data, CurrentFileName.ByteLength);
    } while (CurrentFileName == ".." || CurrentFileName == ".");
    assert(CurrentFileName != ".." && CurrentFileName != ".");
}

void handle::traverse_recursively(path first, path currentDirectory, const delegate<void(path)> &func) const {
    for (auto it = begin(); it != end(); ++it) {
        file::path relativeFileName;
        clone(&relativeFileName, currentDirectory);
        relativeFileName.combine_with(*it);
        func(relativeFileName);

        if ((((WIN32_FIND_DATA *) it.PlatformFileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            file::path p;
            first.get_path_from_here_to(currentDirectory, &p);
            p.combine_with(*it);
            p.combine_with("./");

            handle(p).traverse_recursively(first, p, func);
        }
    }
}

}  // namespace file

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
