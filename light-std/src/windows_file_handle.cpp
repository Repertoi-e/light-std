#include "lstd/common.h"

#if OS == WINDOWS

#include "lstd/file.h"
#include "lstd/os.h"

#include "lstd/io/fmt.h"

#undef MAC
#undef _MAC
#include <Windows.h>

LSTD_BEGIN_NAMESPACE

namespace file {

void get_last_error_as_string(string *out) {
    DWORD errorMessageID = GetLastError();
    if (errorMessageID == 0) return;

    LPSTR messageBuffer = null;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null,
        errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, null);

    string message(messageBuffer, size);
    clone(out, message);

    // Free the buffer.
    LocalFree(messageBuffer);
}

void open_handle_fail(file::path path) {
    string error;
    get_last_error_as_string(&error);
    fmt::print("Couldn't open file handle: \"{}\", error: {}\n", path, error);
}

handle::handle(path path) : Path(path) {
    Utf16Path = (wchar_t *) encode_owner(new byte[(path.UnifiedPath.Length + 1) * 2 + POINTER_SIZE], this);
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

    HANDLE file =
        CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, null);
    if (file == INVALID_HANDLE_VALUE) {
        open_handle_fail(Path);
        return 0;
    }
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);
    return (size_t) size.QuadPart;
}

#define GET_READONLY_EXISTING_HANDLE(x, fail)                                                                \
    HANDLE x = CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, null, OPEN_EXISTING, \
                           FILE_ATTRIBUTE_NORMAL, NULL);                                                     \
    if (x == INVALID_HANDLE_VALUE) {                                                                         \
        open_handle_fail(Path);                                                                              \
        fail;                                                                                                \
    }                                                                                                        \
    defer(CloseHandle(x));

time_t handle::creation_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, &time, null, null)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

time_t handle::last_access_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, null, &time, null)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

time_t handle::last_modification_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
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

    auto p = dest.Path;
    if (dest.is_directory()) {
        p.combine_with(Path.file_name());

        auto *d = new wchar_t[p.UnifiedPath.Length + 1];
        defer(delete d);

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

        auto *d = new wchar_t[p.UnifiedPath.Length + 1];
        defer(delete d);

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

    auto *d = new wchar_t[p.UnifiedPath.Length + 1];
    defer(delete d);
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
    if (!exists()) return false;

    HANDLE handle =
        CreateFileW(Utf16Path, GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (handle == INVALID_HANDLE_VALUE) {
        open_handle_fail(Path);
        return null;
    }
    defer(CloseHandle(handle));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(handle, &size);

    out->reserve(out->ByteLength + size.QuadPart);
    byte *target = const_cast<byte *>(out->Data) + out->ByteLength;
    DWORD bytesRead;
    if (!ReadFile(handle, target, (u32) size.QuadPart, &bytesRead, null)) return false;
    assert(size.QuadPart == bytesRead);

    out->ByteLength += bytesRead;
    out->Length += utf8_strlen(target, bytesRead);
    return true;
}

bool handle::write_to_file(string contents, write_mode mode) const {
    HANDLE handle = CreateFileW(Utf16Path, GENERIC_WRITE, 0, null, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, null);
    if (handle == INVALID_HANDLE_VALUE) {
        open_handle_fail(Path);
        return false;
    }
    defer(CloseHandle(handle));

    LARGE_INTEGER pointer = {};
    pointer.QuadPart = 0;
    if (mode == write_mode::Append) SetFilePointerEx(handle, pointer, null, FILE_END);
    if (mode == write_mode::Overwrite_Entire) SetEndOfFile(handle);

    DWORD bytesWritten;
    if (!WriteFile(handle, contents.Data, (u32) contents.ByteLength, &bytesWritten, null)) return false;
    if (bytesWritten != contents.ByteLength) return false;
    return true;
}

void handle::iterator::read_next_entry() {
    CurrentFileName.release();

    string fileName;
    do {
        if (!Handle) {
            auto queryPath = Path;
            queryPath.combine_with("*");

            auto *query = new wchar_t[queryPath.UnifiedPath.Length + 1];
            defer(delete query);

            utf8_to_utf16(queryPath.UnifiedPath.Data, queryPath.UnifiedPath.Length, query);

            auto handle = FindFirstFileW(query, (WIN32_FIND_DATAW *) PlatformFileInfo);
            if (handle == INVALID_HANDLE_VALUE) {
                open_handle_fail(Path);
                return;
            }
            Handle = (void *) handle;
        } else {
            if (!FindNextFileW((HANDLE) Handle, (WIN32_FIND_DATAW *) PlatformFileInfo)) {
                FindClose((HANDLE) Handle);
                Handle = 0;
                return;
            }
        }
        ++Index;

        auto *fileName = ((WIN32_FIND_DATAW *) PlatformFileInfo)->cFileName;
        CurrentFileName.reserve(c_string_strlen(fileName));
        utf16_to_utf8(fileName, const_cast<byte *>(CurrentFileName.Data), &CurrentFileName.ByteLength);
        CurrentFileName.Length = utf8_strlen(CurrentFileName.Data, CurrentFileName.ByteLength);
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
