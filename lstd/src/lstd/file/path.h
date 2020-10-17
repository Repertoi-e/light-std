#pragma once

#include "../internal/common.h"
#include "../memory/string.h"

LSTD_BEGIN_NAMESPACE

namespace file {

#if OS == WINDOWS
constexpr const char *OS_PATH_SEPARATORS = "\\/";
#else
constexpr const char *OS_PATH_SEPARATORS = "/";
#endif

// This object stores a path to a file or directory and provides common
// operations like getting the file name or extension.
//
// Path parses both \ and / for Windows and / for every other OS.
// The path may contain both characters. To get a unified string
// representing the path, call unified().
//
// ! Passing an ill-formed path to parsing functions
//   results in undefined behaviour so please don't do that :D
struct path {
    string Str;
    bool Reserved = false;

    path() {}
    path(const string &path) : Str(path) {}

    void release() {
        if (Reserved) free(Str);
    }

    // Returns the the relative path to get from this path to _there_.
    // e.g.
    //   /data/bin/ and /data/bin/debug/tests
    // -> debug/tests
    //
    // Returns _there_ if this path isn't contained within _there_
    // @TODO: This just bails if it can't get from here to there without using ../
    path get_path_from_here_to(const path &there) const {
        assert(is_pointing_to_content() && there.is_pointing_to_content());

        if (find_substring(Str, there.Str) == -1) {
            return path(there.Str);
        } else {
            if (Str.Length == there.Str.Length) {
                return path(Str);
            } else {
                path difference = substring(there.Str, Str.Length, there.Str.Length);
                return difference;
            }
        }
    }

    // True if the path has a trailing separator
    bool is_pointing_to_content() const {
        auto last = Str[-1];
        return last == '/' || last == '\\';
    }

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

    // Returns the drive letter (if the path contains it, otherwise and empty string).
    // e.g. C:/Data/Documents/
    // -> C:
    string drive_letter() const;

    // Returns whether the path is absolute (not relative - doesn't start with "." or "..")
    // e.g.
    // /home/user/me -> true
    // C:/Users/User -> true
    // ./data/myData -> false
    // data/myData   -> false
    bool is_absolute() const {
        if (Str.Length == 0) return false;
        auto first = Str[0];
        return first == '/' || first == '\\' || drive_letter().Length;
    }

    void combine_with(const path &other) { combine_with(other.Str); }
    void combine_with(const string &str);

    bool operator==(const path &other) const { return Str == other.Str; }
    bool operator!=(const path &other) const { return Str != other.Str; }

    bool operator==(const string &other) const { return Str == other; }
    bool operator!=(const string &other) const { return Str != other; }

    // Remove any occurences of ".." (that aren't the first one)
    // e.g.  ../data/my_data/../my_other_data
    // -> ../data/my_other_data
    string resolved() const;

    // Returns a path good for display.
    // Resolves any ./ or ../ and replaces all \ with /.
    string unified() const {
        string result = resolved();
        replace_all(result, "\\", "/");
        return result;
    }
};
}  // namespace file

inline bool operator==(const string &one, const file::path &other) { return one == other.Str; }
inline bool operator!=(const string &one, const file::path &other) { return one != other.Str; }

file::path *clone(file::path *dest, const file::path &src);
// file::path *move(file::path *dest, file::path *src);

LSTD_END_NAMESPACE
