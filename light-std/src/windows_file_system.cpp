#include "lstd/common.hpp"

#if OS == WINDOWS

#include "lstd/file/handle.hpp"

#include "lstd/memory/hash.hpp"

#undef MAC
#undef _MAC
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlwapi.h>

LSTD_BEGIN_NAMESPACE

namespace file {

Handle::Iterator::Iterator(const file::Path &path) : Path(path) { read_next_entry(); }

void Handle::Iterator::operator++(s32) { read_next_entry(); }

string Handle::Iterator::operator*() const {
    string fileName;
    fileName.from_utf16(((WIN32_FIND_DATA *) PlatformFileInfo)->cFileName);
    auto path = Path / fileName.get_view();
    path.resolve();
    return string(path.get());
}

bool Handle::Iterator::operator==(const Iterator &other) const {
    if (!PlatformHandle && !other.PlatformHandle) return true;
    if (PlatformHandle && other.PlatformHandle) {
        if (*(*this) == *other) return true;
    }
    return false;
}

void Handle::Iterator::read_next_entry() {
    string fileName;
    do {
        if (!PlatformHandle) {
            auto queryPath = Path / "*";
            auto *query = string(queryPath.get()).to_utf16();
            defer { delete query; };

            auto handle = FindFirstFileW(query, (WIN32_FIND_DATA *) PlatformFileInfo);
            if (handle == INVALID_HANDLE_VALUE) return;
            PlatformHandle = (uptr_t) handle;
        } else {
            if (!FindNextFileW((HANDLE) PlatformHandle, (WIN32_FIND_DATA *) PlatformFileInfo)) {
                // No more files
                FindClose((HANDLE) PlatformHandle);
                PlatformHandle = 0;
                return;
            }
        }
        ++Index;
        fileName.from_utf16(((WIN32_FIND_DATA *) PlatformFileInfo)->cFileName);
    } while (fileName == ".." || fileName == ".");
    assert(fileName != ".." && fileName != ".");
}

void Handle::traverse_recursively(const Handle &first, Handle::visit_func_t func) const {
    for (auto it = begin(); it != end(); ++it) {
        func(*it);
        if ((((WIN32_FIND_DATA *) it.PlatformFileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            first.open_relative(*it).traverse_recursively(first, func);
        }
    }
}

void Handle::traverse_recursively(visit_func_t func) const { traverse_recursively(*this, func); }

Handle::Handle(const file::Path &path) : Path(path) {
    PathUtf16 = Shared_Memory<wchar_t>(string(path.get()).to_utf16());
}

bool Handle::is_file() const {
    HANDLE file = CreateFileW(PathUtf16.get(), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    defer { CloseHandle(file); };

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) {
        return false;
    }
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool Handle::is_directory() const {
    HANDLE file = CreateFileW(PathUtf16.get(), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    defer { CloseHandle(file); };

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) {
        return false;
    }
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool Handle::exists() const {
    HANDLE file = CreateFileW(PathUtf16.get(), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    CloseHandle(file);
    return true;
}

bool Handle::is_symbolic_link() const {
    auto attribs = GetFileAttributesW(PathUtf16.get());
    if (attribs != INVALID_FILE_ATTRIBUTES) {
        return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }
    return false;
}

size_t Handle::file_size() const {
    HANDLE file =
        CreateFileW(PathUtf16.get(), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, null);
    if (file == INVALID_HANDLE_VALUE) {
        return 0;
    }

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);
    return (size_t) size.QuadPart;
}

#define GET_READONLY_EXISTING_HANDLE(x, fail)                                                                      \
    HANDLE x = CreateFileW(PathUtf16.get(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, null, OPEN_EXISTING, \
                           FILE_ATTRIBUTE_NORMAL, NULL);                                                           \
    if (x == INVALID_HANDLE_VALUE) fail;                                                                           \
    defer { CloseHandle(x); }

u64 Handle::creation_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, &time, null, null)) return 0;
    return ((u64) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

u64 Handle::last_access_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, null, &time, null)) return 0;
    return ((u64) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

u64 Handle::last_modification_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, null, null, &time)) return 0;
    return ((u64) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

bool Handle::create_directory() const {
    if (exists()) return false;
    return CreateDirectoryW(PathUtf16.get(), null);
}

bool Handle::delete_file() {
    if (!is_file()) return false;
    return DeleteFileW(PathUtf16.get());
}

bool Handle::delete_directory() const {
    if (!is_directory()) return false;
    return RemoveDirectoryW(PathUtf16.get());
}

void Handle::delete_directory_with_contents() const {}

void Handle::copy_directory_contents(const Handle &destination) const {}

bool Handle::copy(const Handle &destination, bool overwrite) const {
    if (!is_file()) return false;

    auto destPath = destination.Path;
    wchar_t *dest;

    if (destination.is_directory()) {
        destPath = destPath / Path.file_name();
        dest = string(destPath.get()).to_utf16();
    } else {
        dest = destination.PathUtf16.get();
    }

    return CopyFileW(PathUtf16.get(), dest, !overwrite);
}

bool Handle::move(const Handle &destination, bool overwrite) const {
    if (!is_file()) return false;

    auto destPath = destination.Path;
    wchar_t *dest;

    if (destination.is_directory()) {
        destPath = destPath / Path.file_name();
        dest = string(destPath.get()).to_utf16();
    } else {
        dest = destination.PathUtf16.get();
    }

    if (MoveFileExW(PathUtf16.get(), dest, MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
        *(const_cast<file::Path *>(&Path)) = destPath;
        return true;
    }
    return false;
}

bool Handle::rename(const string_view &newName) const {
    if (!exists()) return false;

    wchar_t *newPath = string((file::Path(Path.directory()) / newName).get()).to_utf16();
    defer { delete newPath; };

    if (MoveFileW(PathUtf16.get(), newPath)) {
        *(const_cast<file::Path *>(&Path)) = file::Path(Path.directory()) / newName;
        return true;
    }

    return false;
}

bool Handle::create_hard_link(const Handle &destination) const {
    if (!is_directory()) return false;
    if (!destination.is_directory()) return false;
    return CreateHardLinkW(destination.PathUtf16.get(), PathUtf16.get(), null);
}

bool Handle::create_symbolic_link(const Handle &destination) const {
    if (!exists()) return false;
    if (!destination.exists()) return false;
    return CreateSymbolicLinkW(destination.PathUtf16.get(), PathUtf16.get(),
                               destination.is_directory() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
}
}  // namespace file

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS