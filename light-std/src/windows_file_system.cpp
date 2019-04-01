#include "lstd/common.hpp"

#if OS == WINDOWS

#include "lstd/file/handle.hpp"

#include "lstd/memory/hash.hpp"

#undef MAC
#undef _MAC
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LSTD_BEGIN_NAMESPACE

namespace file {

handle::iterator::iterator(const path &path) : _Path(path) { read_next_entry(); }

void handle::iterator::operator++(s32) { read_next_entry(); }

string handle::iterator::operator*() const {
    string fileName;
    fileName.from_utf16(((WIN32_FIND_DATA *) _PlatformFileInfo)->cFileName);
    auto path = _Path / fileName.get_view();
    path.resolve();
    return string(path.get());
}

bool handle::iterator::operator==(const iterator &other) const {
    if (!_PlatformHandle && !other._PlatformHandle) return true;
    if (_PlatformHandle && other._PlatformHandle) {
        if (*(*this) == *other) return true;
    }
    return false;
}

void handle::iterator::read_next_entry() {
    string fileName;
    do {
        if (!_PlatformHandle) {
            auto queryPath = _Path / "*";
            auto *query = string(queryPath.get()).to_utf16();
            defer { delete query; };

            auto handle = FindFirstFileW(query, (WIN32_FIND_DATA *) _PlatformFileInfo);
            if (handle == INVALID_HANDLE_VALUE) return;
            _PlatformHandle = (uptr_t) handle;
        } else {
            if (!FindNextFileW((HANDLE) _PlatformHandle, (WIN32_FIND_DATA *) _PlatformFileInfo)) {
                // No more files
                FindClose((HANDLE) _PlatformHandle);
                _PlatformHandle = 0;
                return;
            }
        }
        ++_Index;
        fileName.from_utf16(((WIN32_FIND_DATA *) _PlatformFileInfo)->cFileName);
    } while (fileName == ".." || fileName == ".");
    assert(fileName != ".." && fileName != ".");
}

void handle::traverse_recursively(const handle &first, visit_func_t func) const {
    for (auto it = begin(); it != end(); ++it) {
        func(*it);
        if ((((WIN32_FIND_DATA *) it._PlatformFileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            first.open_relative(*it).traverse_recursively(first, func);
        }
    }
}

void handle::traverse_recursively(visit_func_t func) const { traverse_recursively(*this, func); }

handle::handle(const path &path) : Path(path) { _PathUtf16 = shared_memory(string(path.get()).to_utf16()); }

bool handle::is_file() const {
    HANDLE file = CreateFileW(_PathUtf16.get(), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    defer { CloseHandle(file); };

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) {
        return false;
    }
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool handle::is_directory() const {
    HANDLE file = CreateFileW(_PathUtf16.get(), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    defer { CloseHandle(file); };

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) {
        return false;
    }
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool handle::exists() const {
    HANDLE file = CreateFileW(_PathUtf16.get(), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    CloseHandle(file);
    return true;
}

bool handle::is_symbolic_link() const {
    auto attribs = GetFileAttributesW(_PathUtf16.get());
    if (attribs != INVALID_FILE_ATTRIBUTES) {
        return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }
    return false;
}

size_t handle::file_size() const {
    HANDLE file = CreateFileW(_PathUtf16.get(), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING,
                              FILE_ATTRIBUTE_READONLY, null);
    if (file == INVALID_HANDLE_VALUE) {
        return 0;
    }

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);
    return (size_t) size.QuadPart;
}

#define GET_READONLY_EXISTING_HANDLE(x, fail)                                                                       \
    HANDLE x = CreateFileW(_PathUtf16.get(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, null, OPEN_EXISTING, \
                           FILE_ATTRIBUTE_NORMAL, NULL);                                                            \
    if (x == INVALID_HANDLE_VALUE) fail;                                                                            \
    defer { CloseHandle(x); }

u64 handle::creation_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, &time, null, null)) return 0;
    return ((u64) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

u64 handle::last_access_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, null, &time, null)) return 0;
    return ((u64) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

u64 handle::last_modification_time() const {
    GET_READONLY_EXISTING_HANDLE(handle, return 0);
    FILETIME time;
    if (!GetFileTime(handle, null, null, &time)) return 0;
    return ((u64) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

bool handle::create_directory() const {
    if (exists()) return false;
    return CreateDirectoryW(_PathUtf16.get(), null);
}

bool handle::delete_file() const {
    if (!is_file()) return false;
    return DeleteFileW(_PathUtf16.get());
}

bool handle::delete_directory() const {
    if (!is_directory()) return false;
    return RemoveDirectoryW(_PathUtf16.get());
}

void handle::delete_directory_with_contents() const {}

void handle::copy_directory_contents(const handle &destination) const {}

bool handle::copy(const handle &destination, bool overwrite) const {
    if (!is_file()) return false;

    auto destPath = destination.Path;
    wchar_t *dest;

    if (destination.is_directory()) {
        destPath = destPath / Path.file_name();
        dest = string(destPath.get()).to_utf16();
    } else {
        dest = destination._PathUtf16.get();
    }

    return CopyFileW(_PathUtf16.get(), dest, !overwrite);
}

bool handle::move(const handle &destination, bool overwrite) const {
    if (!is_file()) return false;

    auto destPath = destination.Path;
    wchar_t *dest;

    if (destination.is_directory()) {
        destPath = destPath / Path.file_name();
        dest = string(destPath.get()).to_utf16();
    } else {
        dest = destination._PathUtf16.get();
    }

    if (MoveFileExW(_PathUtf16.get(), dest, MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
        *(const_cast<path *>(&Path)) = destPath;
        return true;
    }
    return false;
}

bool handle::rename(const string_view &newName) const {
    if (!exists()) return false;

    wchar_t *newPath = string((path(Path.directory()) / newName).get()).to_utf16();
    defer { delete newPath; };

    if (MoveFileW(_PathUtf16.get(), newPath)) {
        *(const_cast<path *>(&Path)) = path(Path.directory()) / newName;
        return true;
    }

    return false;
}

bool handle::create_hard_link(const handle &destination) const {
    if (!is_directory()) return false;
    if (!destination.is_directory()) return false;
    return CreateHardLinkW(destination._PathUtf16.get(), _PathUtf16.get(), null);
}

bool handle::create_symbolic_link(const handle &destination) const {
    if (!exists()) return false;
    if (!destination.exists()) return false;
    return CreateSymbolicLinkW(destination._PathUtf16.get(), _PathUtf16.get(),
                               destination.is_directory() ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);
}
}  // namespace file

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS