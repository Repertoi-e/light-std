#pragma once

#include "../common.h"
#include "../string/string.h"

GU_BEGIN_NAMESPACE

struct File_Path {
    // Unified format (to get native format, call to_native)
    string Path;

    File_Path() = default;
    File_Path(string const &str) : Path(str) {}
};

#if defined OS_WINDOWS
constexpr char OS_PATH_SEPARATOR = '\\';
#else
constexpr char OS_PATH_SEPARATOR = '/';
#endif

// This function converts any \ characters in the path to /.
inline void convert_to_forward_slashes(File_Path &path) {
    for (size_t i = 0; i < path.Path.Length; i++) {
        if (path.Path[i] == '\\') {
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // path.Path[i] = '/';
        }
    }
}

// True if the path has a trailing separator
inline bool is_pointing_to_content(File_Path const &path) {
    return path.Path[path.Path.Length - 1] == OS_PATH_SEPARATOR;
}

// Returns the drive letter on Windows, if the path is not
// absolute or the path doesn't have a drive letter returns
// an empty string.
inline string get_drive_letter(const File_Path &path) {
    // const char *pathStr = path.Path.Data;
    // const char *pos = find_cstring(pathStr, ":");
    //
    // if (pathStr - pos == 1) {
    //     string result;
    //     result.append_pointer_and_size(pathStr, pathStr - pos + 1);
    //     return result;
    // }
    // return "";
}

GU_END_NAMESPACE
