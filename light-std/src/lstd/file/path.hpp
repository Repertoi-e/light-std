#pragma once

#include "../common.hpp"
#include "../string/string.hpp"

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
// ! Passing an ill-formed path results in undefined behaviour
//   in parsing functions so please don't do that :D
//
// * In order to specify an explicit allocator,
// construct from a string which has it specified
struct Path {
    Path() {}
    Path(const string &path) : PathStr(path) { unify(); }
    Path(const string_view &path) : PathStr(path) { unify(); }
    Path(const byte *path) : PathStr(path) { unify(); }
    Path(const char *path) : PathStr(path) { unify(); }

    Path(const Path &other) = default;
    Path(Path &&other) = default;

    Path &operator=(const Path &other) = default;
    Path &operator=(Path &&other) = default;

    // True if the path has a trailing separator
    bool is_pointing_to_content() const { return PathStr[-1] == '/'; }

    // Returns the native representation of the path.
    // ('\\' instead of "/" for Windows..)
    //
    // This doesn't return a string_view because we don't
    // want different behaviour on different platforms.
    string get_native() const;

    // Returns the stored string
    inline string_view get() const { return PathStr; }

    // Parses the file name
    // e.g. ../my_dir/my_file.txt
    // -> my_file.txt
    string_view file_name() const;

    // Parses the base name of the file
    // e.g. ../my_dir/my_file.txt
    // -> my_file
    string_view base_name() const;

    // Parses the extension of the file
    // e.g. ../my_dir/my_file.txt
    // -> .txt
    string_view extension() const;

    // Parses the directory of the file
    // e.g. ../my_dir/my_file.txt
    // -> ../my_dir/
    string_view directory() const;

    // If on Windows, returns the drive letter (if the path contains it,
    // otherwise and empty string)
    // On other platforms this always returns an empty string
    // e.g. C:/Data/Documents/
    // -> C:
    string_view drive_letter() const;

    // Returns whether the path is absolute (not relative - doesn't start with "." or "..")
    // e.g.
    // /home/user/me -> true
    // ./data/myData -> false
    // C:/Users/User -> true
    // C:/Users/User -> true
    inline bool is_absolute() const {
        if (PathStr.Length == 0) return false;
        return PathStr[0] == '/' || drive_letter().Length;
    }

    // Return this + "/" + other
    Path combined_with(const string_view &other) const;

    // Return this + "/" + other
    inline Path combined_with(const Path &other) const { return combined_with(other.get()); }

    inline Path operator/(const Path &other) const { return combined_with(other); }
    inline Path operator/(const string_view &other) const { return combined_with(other); }
    inline Path operator/(const byte *other) const { return combined_with(string_view(other)); }
    inline Path operator/(const char *other) const { return combined_with(string_view(other)); }

    inline bool operator==(const Path &other) const { return PathStr == other.PathStr; }
    inline bool operator!=(const Path &other) const { return PathStr != other.PathStr; }

    inline bool operator==(const string_view &other) const { return PathStr == other; }
    inline bool operator!=(const string_view &other) const { return PathStr != other; }

    inline bool operator==(const byte *other) const { return PathStr == other; }
    inline bool operator!=(const byte *other) const { return PathStr != other; }

    inline bool operator==(const char *other) const { return PathStr == other; }
    inline bool operator!=(const char *other) const { return PathStr != other; }

    // Remove any occurences of ".." (that aren't the first one)
    // e.g.  ../data/my_data/../my_other_data
    // -> ../data/my_other_data
    void resolve();

   private:
    string PathStr;

    void unify() { PathStr = PathStr.replaced_all("\\", "/"); }
};

// Returns the drive letter on Windows, if the path is not
// absolute or the path doesn't have a drive letter returns
// an empty string.
inline string get_drive_letter(const Path &path) {
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

}  // namespace file

inline file::Path operator/(const string_view &one, const file::Path &other) {
    return file::Path(one).combined_with(other);
}

inline file::Path operator/(const byte *one, const file::Path &other) { return file::Path(one).combined_with(other); }
inline file::Path operator/(const char *one, const file::Path &other) { return file::Path(one).combined_with(other); }

inline bool operator==(const byte *one, const file::Path &other) { return file::Path(one) == other; }
inline bool operator==(const char *one, const file::Path &other) { return file::Path(one) == other; }

inline bool operator!=(const byte *one, const file::Path &other) { return file::Path(one) != other; }
inline bool operator!=(const char *one, const file::Path &other) { return file::Path(one) != other; }


LSTD_END_NAMESPACE
