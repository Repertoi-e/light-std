module;

#include "../internal/common.h"
#include "../memory/string.h"
#include "../parse.h"

export module path.nt;

import path.general;

//
// This module provides facilities to work with paths and files, WindowsNT/95 version.
// Also provides facilities to query the OS and move/copy/read/write, etc.
// All functions are prefixed with "path_" so you can find them easily with autocomplete.
//
// We import this module when compiling for Windows.
// If you want to explicitly work with Windows paths, import this module directly.
//

export {
    constexpr char OS_PATH_SEPARATOR = '\\';

    always_inline constexpr bool path_is_sep(char32_t ch) { return ch == '\\' || ch == '/'; }

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
    inline path_split_drive_result path_split_drive(const string &path) {
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
    inline bool path_is_absolute(const string &path) {
        auto [_, rest] = path_split_drive(path);
        return rest && path_is_sep(rest[0]);
    }

    // Joins two or more paths.
    // Ignore the previous parts if a part is absolute.
    // This is the de facto way to build paths. Takes care of slashes automatically.
    [[nodiscard("Leak")]] string path_join(const array_view<string> &paths) {
        assert(paths.Count >= 2);

        auto [result_drive, result_path] = path_split_drive(paths[0]);

        string result;
        clone(&result, result_path);

        For(range(1, paths.Count)) {
            auto p = paths[it];
            auto [p_drive, p_path] = path_split_drive(p);
            if (p_path && path_is_sep(p_path[0])) {
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
            if (result && !path_is_sep(result[-1])) {
                append_cp(result, '\\');
            }
            append_string(result, p_path);
        }

        // Add separator between UNC and non-absolute path if needed
        if (result && !path_is_sep(result[0]) && result_drive && result_drive[-1] != ':') {
            insert(result, 0, '\\');
        } else {
            insert_string(result, 0, result_drive);
        }
        return result;
    }

    [[nodiscard("Leak")]] always_inline string path_join(const string &one, const string &other) {
        auto arr = to_stack_array(one, other);
        return path_join(arr);
    }

    // Normalize a pathname by collapsing redundant separators and up-level references so that A//B, A/B/, A/./B and A/foo/../B all become A/B.
    // This string manipulation may change the meaning of a path that contains symbolic links.
    //
    // On Windows, it converts forward slashes to backward slashes.
    //
    // There is an edge case in which the path ends with a slash, both /home/user/dir and /home/user/dir/ mean the same thing.
    // You can use other functions to check if they are really directories or files (by querying the OS).
    [[nodiscard("Leak")]] string path_normalize(const string &path) {
        if (match_beginning(path, "\\\\.\\") || match_beginning(path, "\\\\?\\")) {
            // In the case of paths with these prefixes:
            // \\.\ -> device names
            // \\?\ -> literal paths
            // do not do any normalization, but return the path unchanged.
            return path;
        }

        string result;
        reserve(result, path.Length);

        auto [DriveOrUNC, rest] = path_split_drive(path);
        if (DriveOrUNC) {
            append_string(result, DriveOrUNC);
        }

        // Collapse leading slashes
        if (path_is_sep(rest[0])) {
            append_cp(result, '\\');
            while (path_is_sep(rest[0])) advance_cp(&rest, 1);
        }

        auto components = path_split_into_components(rest);
        defer(free(components));

        s64 i = 0;
        while (i < components.Count) {
            auto it = components[i];
            if (!it || it == ".") {
                remove_at_index(components, i);
            } else if (it == "..") {
                if (i > 0 && components[i - 1] != "..") {
                    remove_range(components, i - 1, i + 1);
                    --i;
                } else if (i == 0 && result && path_is_sep(result[-1])) {
                    remove_at_index(components, i);
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
            append_string(result, it);
            append_cp(result, '\\');
        }
        // Remove the trailing slash we added in the final iteration of the loop
        remove_at_index(result, -1);

        return result;
    }

    // Splits path into two components: head (everything up to the last '/') and tail (the rest).
    // The resulting head won't end in '/' unless it is the root.
    //
    // The Windows version handles \ and drive letters/UNC sharepoints of course.
    //
    // Note: The returned strings are substrings so they shouldn't be freed.
    path_split_result path_split(const string &path) {
        auto [DriveOrUNC, rest] = path_split_drive(path);

        // Set i to index beyond path's last slash
        s64 i = find_reverse_any_of(rest, "/\\") + 1;

        string head = rest(0, i);
        string tail = rest(i, rest.Length);

        string trimmed = head(0, find_reverse_not_any_of(head, "/\\") + 1);
        if (trimmed) head = trimmed;

        head = path(0, head.Length + DriveOrUNC.Length);

        return {head, tail};
    }

    // Returns the final component of the path
    // e.g.
    //    /home/user/me/     ->
    //    /home/user/me.txt  -> me.txt
    //    /home/user/dir     -> dir
    //
    // Note: The result is a substring and shouldn't be freed.
    always_inline string path_base_name(const string &path) {
        auto [_, tail] = path_split(path);
        return tail;
    }

    // Returns everything before the final component of the path
    // e.g.
    //    /home/user/me/     -> /home/user/me
    //    /home/user/me.txt  -> /home/user
    //    /home/user/dir     -> /home/user
    //
    // Note: The result is a substring and shouldn't be freed.
    always_inline string path_directory(const string &path) {
        auto [head, _] = path_split(path);
        return head;
    }

    // Split a path in root and extension.
    // The extension is everything starting at the last dot in the last pathname component; the root is everything before that.
    //
    //    /home/user/me.txt       -> { "/home/user/me,       ".txt" }
    //    /home/user/me.data.txt  -> { "/home/user/me.data", "/txt" }
    //    /home/user/me           -> { "/home/user/me",      "" }
    //
    // Note: The returned strings are substrings so they shouldn't be freed.
    always_inline constexpr path_split_extension_result path_split_extension(const string &path) {
        return path_split_extension_general(path, '/', '\\', '.');
    }
}