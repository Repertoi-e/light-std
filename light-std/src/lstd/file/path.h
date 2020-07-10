#pragma once

#include "../internal/common.h"
#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace file {

#if OS == WINDOWS
constexpr char OS_PATH_SEPARATOR = '\\';
#else
constexpr char OS_PATH_SEPARATOR = '/';
#endif

// This object stores a path to a file or directory and provides common
// operations like getting the file name or extension.
//
// Path uses a unified format for storing paths that can be used
// consistently on every platform, using only '/' as a separator.
// When a Path is constructed from a string, the path is translated into
// the unified format. All operations on Path also return paths in the
// unified format. To obtain a path in native platform format, use to_native().
// Note that this applies mainly for Windows systems,
// on other platforms to_native() just returns the unified format.
//
// ! Passing an ill-formed path to parsing functions
//   results in undefined behaviour so please don't do that :D
struct path {
    string UnifiedPath;

    path() = default;
    path(const string &path) : UnifiedPath(path) { unify(); }

    // Returns the the relative path to get from this path to _other_.
    // e.g.
    //   /data/bin/ and /data/bin/debug/tests
    // returns: debug/tests
    // Returns _other_ if this path isn't contained within _other_
    // Stores result in _out_.
    void get_path_from_here_to(path other, path *out) const {
        assert(is_pointing_to_content() && other.is_pointing_to_content());

        if (UnifiedPath.find(other.UnifiedPath) == -1) {
            clone(out, other);
        } else {
            if (UnifiedPath.Length == other.UnifiedPath.Length) {
                clone(out, *this);
            } else {
                path difference = other.UnifiedPath.substring(UnifiedPath.Length, other.UnifiedPath.Length);
                clone(out, difference);
            }
        }
    }

    // True if the path has a trailing separator
    bool is_pointing_to_content() const { return UnifiedPath[-1] == '/'; }

    // Parses the file name
    // e.g. ../my_dir/my_file.txt
    // -> my_file.txt
    string file_name() const;

    // Parses the base name of the file
    // e.g. ../my_dir/my_file.txt
    // -> my_file
    string base_name() const;

    // Parses the extension of the file
    // e.g. ../my_dir/my_file.txt
    // -> .txt
    string extension() const;

    // Parses the directory of the file
    // e.g. ../my_dir/my_file.txt
    // -> ../my_dir/
    string directory() const;

    // If on Windows, returns the drive letter (if the path contains it,
    // otherwise and empty string)
    // On other platforms this always returns an empty string
    // e.g. C:/Data/Documents/
    // -> C:
    string drive_letter() const;

    // Returns whether the path is absolute (not relative - doesn't start with "." or "..")
    // e.g.
    // /home/user/me -> true
    // ./data/myData -> false
    // C:/Users/User -> true
    // C:/Users/User -> true
    bool is_absolute() const {
        if (UnifiedPath.Length == 0) return false;
        return UnifiedPath[0] == '/' || drive_letter().Length;
    }

    void combine_with(path other) { combine_with(other.UnifiedPath); }
    void combine_with(const string &str);

    bool operator==(path other) const { return UnifiedPath == other.UnifiedPath; }
    bool operator!=(path other) const { return UnifiedPath != other.UnifiedPath; }

    bool operator==(const string &other) const { return UnifiedPath == other; }
    bool operator!=(const string &other) const { return UnifiedPath != other; }

    // Remove any occurences of ".." (that aren't the first one)
    // e.g.  ../data/my_data/../my_other_data
    // -> ../data/my_other_data
    void resolve();

    void unify() {
        UnifiedPath.replace_all("\\", "/");
        resolve();
    }
};
}  // namespace file

inline bool operator==(const string &one, file::path other) { return one == other.UnifiedPath; }
inline bool operator!=(const string &one, file::path other) { return one != other.UnifiedPath; }

file::path *clone(file::path *dest, file::path src);
file::path *move(file::path *dest, file::path *src);

LSTD_END_NAMESPACE
