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
struct path {
    path() {}
    path(const memory_view &str) : _Path(str) { unify(); }

    path(const path &other) = default;
    path(path &&other) = default;

    path &operator=(const memory_view &other);
    path &operator=(const path &other) = default;
    path &operator=(path &&other) = default;

    // True if the path has a trailing separator
    bool is_pointing_to_content() const { return _Path[-1] == '/'; }

    // Returns the native representation of the path.
    // ('\\' instead of "/" for Windows..)
    //
    // This doesn't return a string_view because we don't
    // want different behaviour on different platforms.
    string get_native() const;

    // Returns the stored string
    string_view get() const { return _Path; }

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
    bool is_absolute() const {
        if (_Path.Length == 0) return false;
        return _Path[0] == '/' || drive_letter().Length;
    }

    // Return this + "/" + other
    path combined_with(const memory_view &memory) const;

    // Return this + "/" + other
    path combined_with(const path &other) const { return combined_with(other.get()); }

    path operator/(const path &other) const { return combined_with(other); }
    path operator/(const memory_view &memory) const { return combined_with(memory); }

    bool operator==(const path &other) const { return _Path == other._Path; }
    bool operator!=(const path &other) const { return _Path != other._Path; }

    bool operator==(const memory_view &other) const { return _Path == other; }
    bool operator!=(const memory_view &other) const { return _Path != other; }

    // Remove any occurences of ".." (that aren't the first one)
    // e.g.  ../data/my_data/../my_other_data
    // -> ../data/my_other_data
    void resolve();

   private:
    string _Path;

    void unify() {
        _Path = _Path.replaced_all("\\", "/");
        resolve();
    }
};
}  // namespace file

inline file::path operator/(const memory_view &one, const file::path &other) {
    return file::path(one).combined_with(other);
}

inline bool operator==(const memory_view &one, const file::path &other) { return file::path(one) == other; }
inline bool operator!=(const memory_view &one, const file::path &other) { return file::path(one) != other; }

LSTD_END_NAMESPACE
