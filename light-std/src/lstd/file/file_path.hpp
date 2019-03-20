#pragma once

#include "../common.hpp"
#include "../string/string.hpp"

LSTD_BEGIN_NAMESPACE

// This object stores a path to a file or directory and provides common
// operations like getting the file name or extension.
//
// File_Path uses a unified format for storing paths that can be used
// consistently on every platform, using only '/' as a separator.
// When a File_Path is constructed from a string, the path is translated into
// the unified format. All operations on File_Path also return paths in the
// unified format. To obtain a path in native platform format, use to_native().
// Note that this applies mainly for Windows systems, 
// on other platforms to_native() just returns the unified format.
struct File_Path {
    // Unified format (to get native format, call to_native())
    string Path;

    Allocator_Closure &Allocator = Path.Allocator;

    File_Path() {}
    File_Path(const string &path) : Path(path) { unify(); }
    File_Path(const byte *path) : Path(path) { unify(); }
    File_Path(const char *path) : Path(path) { unify(); }

private:
    void unify() {

    }
};

#if OS == WINDOWS
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
    return "";
}

LSTD_END_NAMESPACE
