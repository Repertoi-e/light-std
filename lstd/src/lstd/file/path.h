#pragma once

#include "../internal/common.h"
#include "../memory/string.h"
#include "../parse.h"

LSTD_BEGIN_NAMESPACE

namespace file {

//
// All functions working with paths assume the directory separator is '/'.
// The caller is responsible for Windows path hygienization.
//

#if OS == WINDOWS
constexpr const char *OS_PATH_SEPARATOR = '\\';
#else
constexpr const char *OS_PATH_SEPARATOR = '/';
#endif

// Returns the drive letter if the path contains it - an empty string otherwise.
// e.g.
//    C:/Data/Documents/   -> C:
//    C:                   -> C:
//
// Note: The result is a substring and shouldn't be freed.
constexpr string drive_letter(const string &path) {
    if (path.Length < 2) return "";
    if (path[1] == ':') return path(0, 2);
    return "";
}

// Returns whether a path is absolute - starts with '/' or a drive letter
// e.g.
//    /home/user/me  -> true
//    C:/Users/User  -> true
//    ./data/myData  -> false
//    ../data/myData -> false
//    data/myData    -> false
constexpr bool is_absolute(const string &path) { return path && (path[0] == '/' || drive_letter(path)); }

// True if the path has a trailing separator
constexpr bool is_pointing_to_content(const string &path) { return path && path[-1] == '/'; }

// Returns the name of the file or directory:
// e.g.
//    /home/user/me/     -> me
//    /home/user/me.txt  -> me.txt
//
// Note: The result is a substring and shouldn't be freed.
constexpr string file_name(const string &path) {
    if (!path) return "";

    s64 last = find_cp_reverse(path, '/', is_pointing_to_content(path) ? -2 : 0);
    if (last == -1) return "";
    return path(last + 1, is_pointing_to_content() ? -1 : path.Length);
}

// Returns the name of the file or directory and removes the extension:
// e.g.
//    /home/user/me/          -> me
//    /home/user/me.txt       -> me
//    /home/user/me.data.txt  -> me.data
constexpr string base_name(const string &path) {
    auto fileName = file_name(path);
    s64 last = find_cp_reverse(fileName, '.');
    if (last == -1) return fileName;
    return fileName(0, last);
}

// Returns the extension of the file
// e.g.
//    /home/user/me/          ->
//    /home/user/me.txt       ->
//    /home/user/me.data.txt  -> .txt
constexpr string extension(const string &path) {
    auto fileName = file_name(path);
    s64 last = find_cp_reverse(fileName, '.');
    if (last == -1) return "";
    return fileName(last, fileName.Length);
}

// Parses the directory
// e.g.
//    /home/user/me/          -> home/user/
//    /home/user/me/data.txt  -> home/user/me/
constexpr string directory(const string &path) {
    if (!path) return "";

    s64 last = find_cp_reverse(path, '/', is_pointing_to_content() ? -2 : 0);
    if (last == -1) return "";
    return path(0, last + 1);
}

// Splits path into components.
[[nodiscard("Leak")]] array<string> split_path(const string &path) {
    array<string> result;
    s64 start = 0, prev = 0;
    while ((start = find_cp(path, '/', start)) != -1) {
        append(result, path(prev, start));
        prev = start;
    }
    return result;
}

// Normalize a pathname by collapsing redundant separators and up-level references so that A//B, A/B/, A/./B and A/foo/../B all become A/B.
// This string manipulation may change the meaning of a path that contains symbolic links.
// On Windows, it converts forward slashes to backward slashes.
[[nodiscard("Leak")]] string normalize_path_windows(const string &path) {
    if (match_beginning(path, "\\\\.\\") || match_beginning(path, "\\\\?\\")) {
        // In the case of paths with these prefixes:
        // \\.\ -> device names
        // \\?\ -> literal paths
        // do not do any normalization, but return the path unchanged.
        return path;
    }

    string p;
    clone(&p, path);
    replace_all(p, '\\', '/');  // Unify path before working with it

    string toFree = p;  // Since we use _p_ as a pointer we need to save the original string and free it
    defer(free(toFree));

    string result;
    reserve(result, path.Length);

    auto drive = drive_letter(p);
    if (drive) {
        append_string(result, drive);
    }

    // Collapse leading slashes
    if (p[0] == '/') {
        append_cp(result, '/');
        while (p[0] == '/') advance_cp(&p, 1);
    }

    auto components = split_path(p);
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
            } else if (i == 0 && match_end(result, "/")) {
                remove_at_index(components, i);
            } else {
                ++i;
            }
        } else {
            ++i;
        }

        // If the path is now empty, substitute "."
        if (!result && !components) {
            return ".";
        }

        For(components) {
            append_string(result, it);
        }
        return result;
    }

    // i = 0
    // while i < len(comps):
    //     if not comps[i] or comps[i] == curdir:
    //         del comps[i]
    //     elif comps[i] == pardir:
    //         if i > 0 and comps[i-1] != pardir:
    //             del comps[i-1:i+1]
    //             i -= 1
    //         elif i == 0 and prefix.endswith(sep):
    //             del comps[i]
    //         else:
    //             i += 1
    //     else:
    //         i += 1

    replace_all(result, '/', '\\');  // Replace slashes with Windows native '\'
    return result;
}

// Normalize a pathname by collapsing redundant separators and up-level references so that A//B, A/B/, A/./B and A/foo/../B all become A/B.
// This string manipulation may change the meaning of a path that contains symbolic links.
[[nodiscard("Leak")]] string normalize_path_posix(const string &path) {
}

// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
#if OS == WINDOWS
#define normalize_path normalize_path_windows
#else
#define normalize_path normalize_path_posix
#endif

LSTD_END_NAMESPACE
