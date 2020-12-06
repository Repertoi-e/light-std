module;

#include "../internal/common.h"
#include "../memory/string.h"

export module path.nt;

//
// Common pathname manipulations, WindowsNT/95 version.
//
// We import this module when compiling for Windows.
// If you want to explicitly work with Windows paths, import this module directly.
//

export constexpr char OS_PATH_SEPARATOR = '\\';

export always_inline constexpr bool is_path_sep(char32_t ch) { return ch == '\\' || ch == '/'; }

export struct split_drive_result {
    string DriveOrUnc, Path;
};

// Posix paths don't have drives/UNC share points. This is only exported here.
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
inline split_drive_result path_split_drive(const string &path) {
    if (path.Length >= 2) {
        if (path(0, 2) == "\\\\" && path[2] != '\\') {
            // It is an UNC path

            //  vvvvvvvvvvvvvvvvvvvv drive letter or UNC path
            //  \\machine\mountpoint\directory\etc\...
            //             directory ^^^^^^^^^^^^^^^

            s64 index = find_any_of(path, "\\/", 2);
            if (index == -1) return {"", path};

            s64 index2 = find_any_of(path, "\\/", index + 1);

            // A UNC path can't have two slashes in a row
            // (after the initial two)
            if (index2 == index + 1) return {"", path};
            if (index2 == -1) {
                index2 = path.Length;
            }
            return {path(0, index2), path(index2, path.Length)};
        }

        if (path[1] == ':') {
            return {path(0, 2), path(2, path.Length)};
        }
    }

    return {"", path};
}

// Returns whether a path is absolute.
// Trivial in Posix (starts with '/'), harder on Windows.
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
inline bool path_is_absolute(const string &path) {
    auto [_, rest] = path_split_drive(path);
    return rest && is_path_sep(rest[0]);
}

// Joins two or more paths.
// Ignore the previous parts if a part is absolute.
// This is the de facto way to build paths. Takes care of slashes automatically.
[[nodiscard("Leak")]] string path_join(const array_view<string> &paths) {
    assert(paths.Count >= 2);

    auto [result_drive, result_path] = split_drive(paths[0]);

    string result;
    clone(&result, result_path);

    For(range(1, paths.Count)) {
        auto p = paths[it];
        auto [p_drive, p_path] = split_drive(p);
        if (p_path && is_sep_windows(p_path[0])) {
            // Second path is absolute
            if (p_drive || !result_drive) {
                result_drive = p_drive;  // These are just substrings so it's fine
            }

            free(result);
            clone(&result, p_path);

            continue;
        } else if (p_drive && p_drive != result_drive) {
            if (compare_ignore_case(p_drive, result_drive) != -1) {
                // Different drives => ignore the first path entirely
                result_drive = p_drive;

                free(result);
                clone(&result, p_path);

                continue;
            }
            // Same drives, different case
            result_drive = p_drive;
        }

        // Second path is relative to the first
        if (result && !is_sep_windows(result[-1])) {
            append_cp(result, '\\');
        }
        append_string(result, p_path);
    }

    // Add separator between UNC and non-absolute path if needed
    if (result && !is_sep_windows(result[0]) && result_drive && result_drive[-1] != ':') {
        insert(result, 0, '\\');
    } else {
        insert_string(result, 0, result_drive);
    }
    return result;
}
