#pragma once

#include "../common.h"

GU_BEGIN_NAMESPACE

// #TODO Permissions, copy, move, create links

template <typename T>
struct File_Path {
    // Unified format (to get native format, call to_native)
    string Path;

    File_Path() = default;
    File_Path(string const &str) : Path(str) {}
};

#if defined OS_WINDOWS
inline const char PLATFORM_PATH_SEPARATOR = '\\';
#else
inline const char PLATFORM_PATH_SEPARATOR = '/';
#endif

// This function converts any \\ characters in the path to /.
// Although this is not needed for proper function since this library deals with
// slashes through PLATFORM_PATH_SEPARATOR defined above.
template <typename T>
void convert_to_forward_slashes(File_Path<T> const &path) {
    for (size_t i = 0; i < path.Path.Size; i++) {
        if (path.Path[i] == '\\') {
            path.Path[i] = '/';
        }
    }
}

// True if the path has a trailing separator
template <typename T>
bool is_pointing_to_content(File_Path<T> const &path) {
    return path.Path[path.Path.Size - 1] == PLATFORM_PATH_SEPARATOR;
}

// Returns the drive letter on Windows (if the path is absolute, otherwise an empty string).
// Calling this function on Posix also returns an empty string.
template <typename T>
string get_drive_letter(File_Path<T> const &path) {
    const char *pathStr = path.Path.Data;
    const char *pos = find_cstring(pathStr, ":");

    if (pathStr - pos == 1) {
        string result;
        append_cstring_and_size(result, pathStr, pathStr - pos + 1);
        return result;
    }
    return "";
}

// If the starts with a drive letter (Windows) or a slash (Posix)

GU_END_NAMESPACE
