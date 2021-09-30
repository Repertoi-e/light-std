module;

#include "lstd/platform/windows.h"  // Declarations of Win32 functions

export module lstd.path.nt;

import lstd.fmt;
import lstd.path.general;

import lstd.os.win32.memory;

//
// This module provides facilities to work with paths and files, WindowsNT/95 version.
// Also provides facilities to query the OS and move/copy/read/write, etc.
// All functions are prefixed with "path_" so you can find them easily with autocomplete.
//
// We import this module when compiling for Windows.
// If you want to explicitly work with Windows paths, import this module directly.
//

LSTD_BEGIN_NAMESPACE

export {
    constexpr char OS_PATH_SEPARATOR = '\\';

    always_inline constexpr bool path_is_sep(code_point ch) { return ch == '\\' || ch == '/'; }

    struct path_split_drive_result {
        string DriveOrUNC, Path;
    };

    // POSIX paths don't have drives/UNC share points. This is only exported here.
    //
    // Split a pathname into drive/UNC sharepoint and relative path specifiers.
    // Returns a _DriveOrUNC_ and _Path_ strings, either part may be empty.
    // They are substrings so they shouldn't be freed.
    //
    // The acceptable slashes for the UNC sharepoint are \. The path after that might contain /, we allow that (as well as \ of course).
    // In any case, make sure the UNC part at least starts with \. I'm not sure if Windows even allows network paths with forward slashes.
    // I wish it wasn't such a mess..........
    //
    // e.g.
    //    c:/dir               -> { "c:", "/dir" }
    //    \\host\computer/dir  -> { "\\host\computer", "/dir" }
    //
    // Paths cannot contain both a drive letter and a UNC path.
    constexpr path_split_drive_result path_split_drive(string path);

    // Returns whether a path is absolute.
    // Trivial in POSIX (starts with '/'), harder on Windows.
    // For Windows it is absolute if it starts with a slash or backslash (current volume),
    // or if it begins with a volume letter or UNC-resource.
    //
    // e.g.
    //    /home/user/me       -> true
    //    C:/Users/User       -> true
    //    \\host\computer\dir -> true
    //    ./data/myData       -> false
    //    ../data/myData      -> false
    //    data/myData         -> false
    constexpr bool path_is_absolute(string path);

    // Joins two or more paths.
    // Ignore the previous parts if a part is absolute.
    // This is the de facto way to build paths. Takes care of slashes automatically.
    [[nodiscard("Leak")]] string path_join(array<string> paths);

    [[nodiscard("Leak")]] string path_join(string one, string other);

    // Normalize a pathname by collapsing redundant separators and up-level references so that A//B, A/B/, A/./B and A/foo/../B all become A/B.
    // This string manipulation may change the meaning of a path that contains symbolic links.
    //
    // On Windows, it converts forward slashes to backward slashes.
    //
    // There is an edge case in which the path ends with a slash, both /home/user/dir and /home/user/dir/ mean the same thing.
    // You can use other functions to check if they are really directories or files (by querying the OS).
    [[nodiscard("Leak")]] string
    path_normalize(string path);

    // Splits path into two components: head (everything up to the last '/') and tail (the rest).
    // The resulting head won't end in '/' unless it is the root.
    //
    // The Windows version handles \ and drive letters/UNC sharepoints of course.
    //
    // Note: The returned strings are substrings so they shouldn't be freed.
    constexpr path_split_result path_split(string path);

    // Returns the final component of the path
    // e.g.
    //    /home/user/me/     ->
    //    /home/user/me.txt  -> me.txt
    //    /home/user/dir     -> dir
    //
    // Note: The result is a substring and shouldn't be freed.

    constexpr string path_base_name(string path);

    // Returns everything before the final component of the path
    // e.g.
    //    /home/user/me/     -> /home/user/me
    //    /home/user/me.txt  -> /home/user
    //    /home/user/dir     -> /home/user
    //
    // Note: The result is a substring and shouldn't be freed.
    constexpr string path_directory(string path);

    // Split a path in root and extension.
    // The extension is everything starting at the last dot in the last pathname component; the root is everything before that.
    //
    //    /home/user/me.txt       -> { "/home/user/me,       ".txt" }
    //    /home/user/me.data.txt  -> { "/home/user/me.data", "/txt" }
    //    /home/user/me           -> { "/home/user/me",      "" }
    //
    // Note: The returned strings are substrings so they shouldn't be freed.
    constexpr path_split_extension_result path_split_extension(string path);

    //
    // The following routines query the OS:
    //
    bool path_exists(string path);  // == is_file() || is_directory()
    bool path_is_file(string path);
    bool path_is_directory(string path);

    bool path_is_symbolic_link(string path);

    s64 path_file_size(string path);

    time_t path_creation_time(string path);
    time_t path_last_access_time(string path);
    time_t path_last_modification_time(string path);

    bool path_create_directory(string path);
    bool path_delete_file(string path);
    bool path_delete_directory(string path);

    // @Robustness: We don't handle directories.
    //
    // Copies a file to destination.
    // Destination can point to another file - in which case it gets overwritten (if the parameter is true)
    // or a directory - in which case the file name is kept the same or determined by the OS (in the case of duplicate files).
    bool path_copy(string path, string dest, bool overwrite);

    // @Robustness: We don't handle directories.
    //
    // Moves a file to destination.
    // Destination can point to another file - in which case it gets overwritten (if the parameter is true)
    // or a directory - in which case the file name is kept the same or determined by the OS (in the case of duplicate files).
    bool path_move(string path, string dest, bool overwrite);

    // Renames a file/directory
    bool path_rename(string path, string newName);

    // A hard link is a way to represent a single file by more than one path.
    // Hard links continue to work fine if you delete the source file since they use reference counting.
    // Hard links can be created to files (not directories) only on the same volume.
    //
    // Destination must exist, otherwise this function fails.
    bool path_create_hard_link(string path, string dest);

    // Symbolic links are different from hard links. Hard links do not link paths on different
    // volumes or file systems, whereas symbolic links may point to any file or directory
    // irrespective of the volumes on which the link and target reside.
    //
    // Hard links always refer to an existing file, whereas symbolic links may contain an
    // arbitrary path that does not point to anything.
    //
    // Destination must exist, otherwise this function fails.
    bool path_create_symbolic_link(string path, string dest);

    // This is used for traversing every file in a directory.
    // This is not recursive but we define a method which does that further down.
    //
    // _Path_ needs to be a valid path before using it.
    //
    struct path_walker {
        string Path;  // Doesn't get cloned, valid as long as the string passed in the constructor is valid

        string CurrentFileName;  // Gets allocated by this object, call free after use to prevent leak

        void *Handle = null;  // null in the beginning, null after calling _path_read_next_entry_ and there were no more files.
                              // Check this for when to stop calling _path_read_next_entry_.

        s64 Index = 0;

        path_walker() {}
        path_walker(string path) : Path(path) {}

       private:
        wchar *Path16 = null;

        char PlatformFileInfo[sizeof(WIN32_FIND_DATAW)]{};

        // I don't usually use "private" but we seriously don't need to expose this garbage to users...
        friend void path_read_next_entry(path_walker &walker);
    };

    // @Cleanup
    void free_path_walker(path_walker & walker) {
        free(walker.CurrentFileName.Data);
    }

    void path_read_next_entry(path_walker & walker);

    // Return an array of all files in a directory.
    // _recursively_ determines if files in subdirectories are included.
    //
    // If you don't want the overhead of us building an array you can use
    // the path_walker API directly (take a look at the implementation of this function further down the file).
    [[nodiscard("Leak")]] array<string> path_walk(string path, bool recursively = false);
}

#define GET_READONLY_EXISTING_HANDLE(x, fail)                                                                                                                                  \
    CREATE_FILE_HANDLE_CHECKED(x, CreateFileW(utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL), fail); \
    defer(CloseHandle(x));

string get_path_from_here_to(string here, string there) {
    if (string_find(here, there) == -1) {
        return there;
    } else {
        if (here.Count == there.Count) {
            return here;
        } else {
            string difference = substring(there, string_length(here), string_length(there));
            return difference;
        }
    }
}

wchar *utf8_to_utf16(string str) { return platform_utf8_to_utf16(str); }

constexpr path_split_drive_result path_split_drive(string path) {
    if (string_length(path) >= 2) {
        if (substring(path, 0, 2) == string("\\\\") && path[2] != '\\') {
            // It is an UNC path

            //  vvvvvvvvvvvvvvvvvvvv drive letter or UNC path
            //  \\machine\mountpoint\directory\etc\...
            //             directory ^^^^^^^^^^^^^^^

            s64 index = string_find_any_of(path, "\\/", 2);
            if (index == -1) return {"", path};

            s64 index2 = string_find_any_of(path, "\\/", index + 1);

            // A UNC path can't have two slashes in a row
            // (after the initial two)
            if (index2 == index + 1) return {"", path};
            if (index2 == -1) {
                index2 = string_length(path);
            }
            return {substring(path, 0, index2), substring(path, index2, string_length(path))};
        }

        if (path[1] == ':') {
            return {substring(path, 0, 2), substring(path, 2, string_length(path))};
        }
    }

    return {"", path};
}

constexpr bool path_is_absolute(string path) {
    auto [_, rest] = path_split_drive(path);
    return rest && path_is_sep(rest[0]);
}

[[nodiscard("Leak")]] string path_join(array<string> paths) {
    assert(paths.Count >= 2);

    auto [result_drive, result_path] = path_split_drive(paths[0]);

    string result = clone(result_path);

    For(range(1, paths.Count)) {
        auto p                 = paths[it];
        auto [p_drive, p_path] = path_split_drive(p);
        if (p_path && path_is_sep(p_path[0])) {
            // Second path is absolute
            if (p_drive || !result_drive) {
                result_drive = p_drive;  // These are just substrings so it's fine
            }

            free(result.Data);
            result = clone(p_path);

            continue;
        } else if (p_drive && p_drive != result_drive) {
            if (string_compare_ignore_case(p_drive, result_drive) != -1) {
                // Different drives => ignore the first path entirely
                result_drive = p_drive;

                free(result.Data);
                result = clone(p_path);

                continue;
            }
            // Same drives, different case
            result_drive = p_drive;
        }

        // Second path is relative to the first
        if (result && !path_is_sep(result[-1])) {
            string_append(&result, '\\');
        }
        string_append(&result, p_path);
    }

    // Add separator between UNC and non-absolute path if needed
    if (result && !path_is_sep(result[0]) && result_drive && result_drive[-1] != ':') {
        string_insert_at_index(&result, 0, '\\');
    } else {
        string_insert_at_index(&result, 0, result_drive);
    }
    return result;
}

[[nodiscard("Leak")]] string path_join(string one, string other) {
    auto arr = make_stack_array(one, other);
    return path_join(arr);
}

[[nodiscard("Leak")]] string path_normalize(string path) {
    string result;
    resize(&result, path.Count);

    if (match_beginning(path, "\\\\.\\") || match_beginning(path, "\\\\?\\")) {
        // In the case of paths with these prefixes:
        // \\.\ -> device names
        // \\?\ -> literal paths
        // do not do any normalization, but return the path unchanged.
        free(result.Data);
        result = clone(path);
        return result;
    }

    auto [DriveOrUNC, rest] = path_split_drive(path);
    if (DriveOrUNC) {
        string_append(&result, DriveOrUNC);
    }

    // Collapse leading slashes
    if (path_is_sep(rest[0])) {
        string_append(&result, '\\');
        while (path_is_sep(rest[0])) advance_cp(&rest, 1);
    }

    auto components = path_split_into_components(rest);
    defer(free(components.Data));

    s64 i = 0;
    while (i < components.Count) {
        auto it = components[i];
        if (!it || it == string(".")) {
            remove_ordered_at_index(&components, i);
        } else if (it == string("..")) {
            if (i > 0 && components[i - 1] != string("..")) {
                remove_range(&components, i - 1, i + 1);
                --i;
            } else if (i == 0 && result && path_is_sep(result[-1])) {
                remove_ordered_at_index(&components, i);
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }

    // If the path is now empty, substitute "."
    if (!result && !components) {
        return ".";
    }

    For(components) {
        string_append(&result, it);
        string_append(&result, '\\');
    }
    // Remove the trailing slash we added in the final iteration of the loop
    string_remove_at_index(&result, -1);

    return result;
}

constexpr path_split_result path_split(string path) {
    auto [DriveOrUNC, rest] = path_split_drive(path);

    // Set i to index beyond path's last slash
    s64 i = string_find_any_of(rest, "/\\", string_length(rest), true) + 1;

    string head = substring(rest, 0, i);
    string tail = substring(rest, i, string_length(rest));

    string trimmed = substring(head, 0, string_find_not_any_of(head, "/\\", string_length(head), true) + 1);
    if (trimmed) head = trimmed;

    head = substring(path, 0, string_length(head) + string_length(DriveOrUNC));

    return {head, tail};
}

constexpr string path_base_name(string path) {
    auto [_, tail] = path_split(path);
    return tail;
}

constexpr string path_directory(string path) {
    auto [head, _] = path_split(path);
    return head;
}

constexpr path_split_extension_result path_split_extension(string path) {
    return path_split_extension_general(path, '/', '\\', '.');
}

// == is_file() || is_directory()
bool path_exists(string path) {
    HANDLE file = CreateFileW(utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    CloseHandle(file);
    return true;
}

bool path_is_file(string path) {
    HANDLE file = CreateFileW(utf8_to_utf16(path), 0, 0, null, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    defer(CloseHandle(file));

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) return false;
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool path_is_directory(string path) {
    HANDLE file = CreateFileW(utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
    if (file == INVALID_HANDLE_VALUE) return false;
    defer(CloseHandle(file));

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(file, &info)) return false;
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool path_is_symbolic_link(string path) {
    DWORD attribs = GetFileAttributesW(utf8_to_utf16(path));
    if (attribs != INVALID_FILE_ATTRIBUTES) {
        return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    }
    return false;
}

s64 path_file_size(string path) {
    if (path_is_directory(path)) return 0;

    CREATE_FILE_HANDLE_CHECKED(file, CreateFileW(utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ, null, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, null), 0);
    defer(CloseHandle(file));

    LARGE_INTEGER size = {0};
    GetFileSizeEx(file, &size);
    return size.QuadPart;
}

time_t path_creation_time(string path) {
    GET_READONLY_EXISTING_HANDLE(handle, 0);
    FILETIME time;
    if (!GetFileTime(handle, &time, null, null)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

time_t path_last_access_time(string path) {
    GET_READONLY_EXISTING_HANDLE(handle, 0);
    FILETIME time;
    if (!GetFileTime(handle, null, &time, null)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

time_t path_last_modification_time(string path) {
    GET_READONLY_EXISTING_HANDLE(handle, 0);
    FILETIME time;
    if (!GetFileTime(handle, null, null, &time)) return 0;
    return ((time_t) time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

bool path_create_directory(string path) {
    if (path_exists(path)) return false;
    return CreateDirectoryW(utf8_to_utf16(path), null);
}

bool path_delete_file(string path) {
    if (!path_is_file(path)) return false;
    return DeleteFileW(utf8_to_utf16(path));
}

bool path_delete_directory(string path) {
    if (!path_is_directory(path)) return false;
    return RemoveDirectoryW(utf8_to_utf16(path));
}

// @Robustness: Handle directories?
bool path_copy(string path, string dest, bool overwrite) {
    if (!path_is_file(path)) return false;

    auto *u16 = utf8_to_utf16(path);

    if (path_is_directory(dest)) {
        auto p = path_join(dest, path_base_name(path));
        defer(free(p.Data));

        return CopyFileW(u16, utf8_to_utf16(p), !overwrite);
    }
    return CopyFileW(u16, utf8_to_utf16(dest), !overwrite);
}

// @Robustness: Handle directories?
bool path_move(string path, string dest, bool overwrite) {
    if (!path_is_file(path)) return false;

    if (path_is_directory(dest)) {
        auto p = path_join(dest, path_base_name(path));
        defer(free(p.Data));

        return MoveFileExW(utf8_to_utf16(path), utf8_to_utf16(p), MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0));
    }
    return MoveFileExW(utf8_to_utf16(path), utf8_to_utf16(dest), MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0));
}

bool path_rename(string path, string newName) {
    if (!path_exists(path)) return false;

    auto p = path_join(path_directory(path), newName);
    defer(free(p.Data));

    return MoveFileW(utf8_to_utf16(path), utf8_to_utf16(p));
}

bool path_create_hard_link(string path, string dest) {
    if (!path_is_directory(path)) return false;
    if (!path_is_directory(dest)) return false;
    return CreateHardLinkW(utf8_to_utf16(dest), utf8_to_utf16(path), null);
}

bool path_create_symbolic_link(string path, string dest) {
    if (!path_exists(path)) return false;
    if (!path_exists(dest)) return false;

    u32 flag = path_is_directory(dest) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
    return CreateSymbolicLinkW(utf8_to_utf16(dest), utf8_to_utf16(path), flag);
}

void path_read_next_entry(path_walker &walker) {
    do {
        if (!walker.Handle) {
            if (!walker.Path16) {
                string queryPath = path_join(walker.Path, "*");
                defer(free(queryPath.Data));

                walker.Path16 = utf8_to_utf16(queryPath);
            }

            string path = walker.Path;
            CREATE_FILE_HANDLE_CHECKED(f, FindFirstFileW(walker.Path16, (WIN32_FIND_DATAW *) walker.PlatformFileInfo), ;);
            walker.Handle = (void *) f;
        } else {
            u32 ERROR_NO_MORE_FILES = 18;

#define CHECK_FIND_NEXT(call)                                                        \
    if (!call) {                                                                     \
        if (GetLastError() != ERROR_NO_MORE_FILES) {                                 \
            windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call); \
        }                                                                            \
        if (walker.Handle != INVALID_HANDLE_VALUE) {                                 \
            WIN32_CHECK_BOOL(r, FindClose((HANDLE) walker.Handle));                  \
        }                                                                            \
                                                                                     \
        walker.Handle = null; /* No more files.. terminate */                        \
        return;                                                                      \
    }
            CHECK_FIND_NEXT(FindNextFileW((HANDLE) walker.Handle, (WIN32_FIND_DATAW *) walker.PlatformFileInfo));
        }
        ++walker.Index;

        free(walker.CurrentFileName.Data);

        auto *fileName = ((WIN32_FIND_DATAW *) walker.PlatformFileInfo)->cFileName;
        resize(&walker.CurrentFileName, c_string_length(fileName) * 4);                                 // @Cleanup
        utf16_to_utf8(fileName, (char *) walker.CurrentFileName.Data, &walker.CurrentFileName.Count);  // @Constcast

    } while (walker.CurrentFileName == string("..") || walker.CurrentFileName == string("."));
    assert(walker.CurrentFileName != string("..") && walker.CurrentFileName != string("."));
}

// This version appends paths to the array _result_. Copy this and modify it to suit your use case.
void path_walk_recursively_impl(string path, string first, array<string> *result) {
    assert(path_is_directory(path));

    auto walker = path_walker(path);
    defer(free_path_walker(walker));

    while (true) {
        path_read_next_entry(walker);
        if (!walker.Handle) break;

        string p = path_join(get_path_from_here_to(first, path), walker.CurrentFileName);
        add(result, p);

        if (path_is_directory(p)) {
            path_walk_recursively_impl(p, first, result);
        }
    }
}

[[nodiscard("Leak")]] array<string> path_walk(string path, bool recursively) {
    assert(path_is_directory(path));

    array<string> result;

    if (!recursively) {
        auto walker = path_walker(path);
        defer(free_path_walker(walker));

        while (true) {
            path_read_next_entry(walker);
            if (!walker.Handle) break;

            string file = path_join(path, walker.CurrentFileName);
            add(&result, file);
        }
    } else {
        path_walk_recursively_impl(path, path, &result);
    }
    return result;
}

LSTD_END_NAMESPACE
