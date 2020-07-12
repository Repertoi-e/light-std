#include "lstd/internal/common.h"

#if OS == WINDOWS

#include "lstd/file.h"
#include "lstd/io/fmt.h"
#include "lstd/os.h"

LSTD_BEGIN_NAMESPACE

namespace file {

#define CREATE_FILE_HANDLE_CHECKED(handleName, call, returnOnFail)                                                  \
    HANDLE handleName = call;                                                                                       \
    if (handleName == INVALID_HANDLE_VALUE) {                                                                       \
        string extendedCallSite = fmt::sprint("{}\n        (the path was: {!YELLOW}\"{}\"{!GRAY})\n", #call, Path); \
        defer(extendedCallSite.release());                                                                          \
        windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), extendedCallSite, __FILE__, __LINE__);     \
        return returnOnFail;                                                                                        \
    }

static wchar_t *utf8_path_to_utf16(const file::path &path) {
    // @Bug path.Str.Length is not enough (2 wide chars for one char)
    auto *result = allocate_array(wchar_t, path.Str.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(path.Str.Data, path.Str.Length, result);
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

    auto *utf16 = utf8_path_to_utf16(Path);

    if (dest.is_directory()) {
        auto p = dest.Path;
        p.combine_with(Path.file_name());

        // @Bug p.Str.Length is not enough (2 wide chars for one char)
        auto *d = allocate_array(wchar_t, p.Str.Length + 1, Context.TemporaryAlloc);
        utf8_to_utf16(p.Str.Data, p.Str.Length, d);

        return CopyFileW(utf16, d, !overwrite);
    }
    return CopyFileW(utf16, utf8_path_to_utf16(dest.Path), !overwrite);
}

bool handle::move(handle dest, bool overwrite) const {
    if (!is_file()) return false;

    auto p = dest.Path;
    if (dest.is_directory()) {
        p.combine_with(Path.file_name());

        // @Bug p.Str.Length is not enough (2 wide chars for one char)
        auto *d = allocate_array(wchar_t, p.Str.Length + 1, Context.TemporaryAlloc);
        utf8_to_utf16(p.Str.Data, p.Str.Length, d);

        if (MoveFileExW(utf8_path_to_utf16(Path), d, MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
            ::move(const_cast<path *>(&Path), &p);
            return true;
        }
    } else {
        if (MoveFileExW(utf8_path_to_utf16(Path), utf8_path_to_utf16(dest.Path),
                        MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0))) {
            ::clone(const_cast<path *>(&Path), p);
            return true;
        }
    }
    return false;
}

bool handle::rename(const string &newName) const {
    if (!exists()) return false;

    auto p = path(Path.directory());
    p.combine_with(newName);

    // @Bug p.Str.Length is not enough (2 wide chars for one char)
    auto *d = allocate_array(wchar_t, p.Str.Length + 1, Context.TemporaryAlloc);
    utf8_to_utf16(p.Str.Data, p.Str.Length, d);

    if (MoveFileW(utf8_path_to_utf16(Path), d)) {
        ::move(const_cast<path *>(&Path), &p);
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

pair<bool, string> handle::read_entire_file() const {
    pair<bool, string> failReturn = {false, ""};  // Hack, because specifying directly in the macro below doesn't work

    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_path_to_utf16(Path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null), failReturn);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);

    string result;
    result.reserve(size.QuadPart);
    char *target = const_cast<char *>(result.Data);
    DWORD bytesRead;
    if (!ReadFile(file, target, (u32) size.QuadPart, &bytesRead, null)) return {false, ""};
    assert(size.QuadPart == bytesRead);

    result.ByteLength += bytesRead;
    result.Length += utf8_length(target, bytesRead);
    return {true, result};
}

bool handle::write_to_file(const string &contents, write_mode mode) const {
    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_path_to_utf16(Path), GENERIC_WRITE, 0, null, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, null), false);
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
    do {
        if (!Handle) {
            file::path queryPath;
            clone(&queryPath, Path);
            queryPath.combine_with("*");
            defer(queryPath.release());

            // @Bug queryPath.Str.Length is not enough (2 wide chars for one char)
            auto *query = allocate_array(wchar_t, queryPath.Str.Length + 1, Context.TemporaryAlloc);
            utf8_to_utf16(queryPath.Str.Data, queryPath.Str.Length, query);

            CREATE_FILE_HANDLE_CHECKED(file, FindFirstFileW(query, (WIN32_FIND_DATAW *) PlatformFileInfo), ;);
            Handle = (void *) file;
        } else {
#define CHECK_FIND_NEXT(call)                                                                            \
    if (!call) {                                                                                         \
        if (GetLastError() != ERROR_NO_MORE_FILES) {                                                     \
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call, __FILE__, __LINE__); \
        }                                                                                                \
        if (Handle != INVALID_HANDLE_VALUE) {                                                            \
            WINDOWS_CHECKBOOL(FindClose((HANDLE) Handle));                                               \
        }                                                                                                \
                                                                                                         \
        Handle = null;                                                                                   \
        return;                                                                                          \
    }
            CHECK_FIND_NEXT(FindNextFileW((HANDLE) Handle, (WIN32_FIND_DATAW *) PlatformFileInfo));
        }
        ++Index;

        CurrentFileName.release();

        auto *fileName = ((WIN32_FIND_DATAW *) PlatformFileInfo)->cFileName;
        CurrentFileName.reserve(c_string_length(fileName) * 2);  // @Bug c_string_length * 2 is not enough
        utf16_to_utf8(fileName, const_cast<char *>(CurrentFileName.Data), &CurrentFileName.ByteLength);
        CurrentFileName.Length = utf8_length(CurrentFileName.Data, CurrentFileName.ByteLength);
    } while (CurrentFileName == ".." || CurrentFileName == ".");
    assert(CurrentFileName != ".." && CurrentFileName != ".");
}

void handle::traverse_impl(const delegate<void(const path &)> &func) const {
    for (auto it = begin(); it != end(); ++it) {
        file::path relativeFileName;
        clone(&relativeFileName, Path);
        relativeFileName.combine_with(*it);

        func(relativeFileName);

        relativeFileName.release();
    }
}

void handle::traverse_recursively_impl(const path &first, const path &currentDirectory, const delegate<void(const path &)> &func) const {
    for (auto it = begin(); it != end(); ++it) {
        file::path relativeFileName;
        clone(&relativeFileName, currentDirectory);
        relativeFileName.combine_with(*it);

        func(relativeFileName);

        if ((((WIN32_FIND_DATA *) it.PlatformFileInfo)->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            file::path p;
            clone(&p, first.get_path_from_here_to(currentDirectory));

            p.combine_with(*it);
            p.combine_with("./");

            handle(p).traverse_recursively_impl(first, p, func);

            p.release();
        }
        relativeFileName.release();
    }
}

}  // namespace file

LSTD_END_NAMESPACE

#endif  // OS == WINDOWS
