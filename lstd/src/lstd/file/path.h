#pragma once

#include "../internal/common.h"
#include "../memory/string.h"
#include "../parse.h"

LSTD_BEGIN_NAMESPACE

namespace path {

//
// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
//    e.g. _is_absolute_ uses the Windows routine when the program is compiled for Windows, otherwise calls the posix version.
//         to work explicitly with one type of path, use the suffixed version (_is_absolute_windows_).
//

#if OS == WINDOWS
constexpr char OS_PATH_SEPARATOR = '\\';
#else
constexpr char OS_PATH_SEPARATOR = '/';
#endif

always_inline constexpr bool is_sep_windows(char32_t ch) { return ch == '\\' || ch == '/'; }
always_inline constexpr bool is_sep_posix(char32_t ch) { return ch == '/'; }

// Helper function to determine if a code point is a path separator depending on the OS we are compiling on
always_inline constexpr bool is_sep(char32_t ch) {
#if OS == WINDOWS
    return is_sep_windows(ch);
#else
    return is_sep_posix(ch);
#endif
}

struct split_drive_result {
    string DriveOrUNC, Path;
};

split_drive_result split_drive_windows(const string &path);

// Posix paths don't have drives/UNC share points.
inline split_drive_result split_drive_posix(const string &path) { return {"", path}; }

// Posix paths don't have drives/UNC share points.
//
// On Windows: split a pathname into drive/UNC sharepoint and relative path specifiers.
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
//
// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
inline split_drive_result split_drive(const string &path) {
#if OS == WINDOWS
    return split_drive_windows(path);
#else
    return split_drive_posix(path);
#endif
}

// @TODO: Constexpr
inline bool is_absolute_windows(const string &path) {
    auto [_, rest] = split_drive_windows(path);
    return rest && is_sep_windows(rest[0]);
}

constexpr bool is_absolute_posix(const string &path) { return is_sep_posix(path[0]); }

// Returns whether a path is absolute.
// Trivial in Posix (starts with '/'), harder on Windows.
// For Windows it is absolute if it starts with a slash or backslash (current volume),
// or if it begins with a volume letter or UNC-resource.
//
// e.g.
//    /home/user/me       -> true
//    C:/Users/User       -> true
//    ./data/myData       -> false
//    ../data/myData      -> false
//    data/myData         -> false
//    \\host\computer\dir -> true
inline bool is_absolute(const string &path) {
#if OS == WINDOWS
    return is_absolute_windows(path);
#else
    return is_absolute_posix(path);
#endif
}

[[nodiscard("Leak")]] string join_windows(const array_view<string> &paths);

[[nodiscard("Leak")]] string join_posix(const array_view<string> &paths);

// Joins two or more paths
// Ignore the previous parts if a part is absolute.
//
// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
[[nodiscard("Leak")]] inline string join(const array_view<string> &paths) {
#if OS == WINDOWS
    return join_windows(paths);
#else
    return join_posix(paths);
#endif
}

[[nodiscard("Leak")]] inline string join(const string &one, const string &other) {
    auto arr = to_stack_array(one, other);
    return join(arr);
}

struct split_result {
    string Head, Tail;
};

split_result split_windows(const string &path);
split_result split_posix(const string &path);

// Splits path into two components: head (everything up to the last '/') and tail (the rest).
// They are substrings so they shouldn't be freed.
// The resulting head won't end in '/' unless it is the root.
//
// The Windows version handles \ and drives/UNC.
//
// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
inline split_result split(const string &path) {
#if OS == WINDOWS
    return split_windows(path);
#else
    return split_posix(path);
#endif
}

inline string base_name_windows(const string &path) {
    auto [_, tail] = split_windows(path);
    return tail;
}

inline string base_name_posix(const string &path) {
    auto [_, tail] = split_posix(path);
    return tail;
}

// Returns the final component of the path
// e.g.
//    /home/user/me/     ->
//    /home/user/me.txt  -> me.txt
//    /home/user/dir     -> dir
//
// Note: The result is a substring and shouldn't be freed.
inline string base_name(const string &path) {
    auto [_, tail] = split(path);
    return tail;
}

inline string directory_windows(const string &path) {
    auto [head, _] = split_windows(path);
    return head;
}

inline string directory_posix(const string &path) {
    auto [head, _] = split_posix(path);
    return head;
}

// Returns everything before the final component of the path
// e.g.
//    /home/user/me/     -> /home/user/me
//    /home/user/me.txt  -> /home/user
//    /home/user/dir     -> /home/user
//
// Note: The result is a substring and shouldn't be freed.
inline string directory(const string &path) {
    auto [head, _] = split(path);
    return head;
}

struct split_extension_result {
    string Root, Extension;
};

constexpr split_extension_result split_extension_generic(const string &path, char32_t sep, char32_t altSep, char32_t extensionSep) {
    s64 sepIndex = find_cp_reverse(path, sep);
    if (altSep) {
        s64 altSepIndex = find_cp_reverse(path, altSep);
        if (altSepIndex > sepIndex) sepIndex = altSepIndex;
    }

    // Most OSes use a dot to separate extensions but we support other characters as well
    s64 dotIndex = find_cp_reverse(path, extensionSep);

    if (dotIndex > sepIndex) {
        // Skip leading dots
        s64 filenameIndex = sepIndex + 1;
        while (filenameIndex < dotIndex) {
            if (path[filenameIndex] != extensionSep)
                return {path(0, dotIndex), path(dotIndex, path.Length)};
            ++filenameIndex;
        }
    }
    return {path, ""};
}

constexpr split_extension_result split_extension_windows(const string &path) { return split_extension_generic(path, '/', '\\', '.'); }
constexpr split_extension_result split_extension_posix(const string &path) { return split_extension_generic(path, '/', 0, '.'); }

// Split a path in root and extension.
// The extension is everything starting at the last dot in the last pathname component; the root is everything before that.
// The returned strings are substrings so they shouldn't be freed.
//
//    /home/user/me.txt       -> { "/home/user/me,       ".txt" }
//    /home/user/me.data.txt  -> { "/home/user/me.data", "/txt" }
//    /home/user/me           -> { "/home/user/me",      "" }
//
// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
constexpr split_extension_result split_extension(const string &path) {
#if OS == WINDOWS
    return split_extension_windows(path);
#else
    return split_extension_posix(path);
#endif
}

[[nodiscard("Leak")]] string normalize_windows(const string &path);
[[nodiscard("Leak")]] string normalize_posix(const string &path);

// Normalize a pathname by collapsing redundant separators and up-level references so that A//B, A/B/, A/./B and A/foo/../B all become A/B.
// This string manipulation may change the meaning of a path that contains symbolic links.
//
// On Windows, it converts forward slashes to backward slashes.
//
// There is an edge case in which the path ends with a slash, both /home/user/dir and /home/user/dir/ mean the same thing.
// You can use other functions to check if they are really directories or files (by querying the OS).
//
// By default this module handles path by what is best for the OS the program is compiled for. We do that here using macros.
// If you want to work explicitly with a format, you can use the suffixed functions directly.
[[nodiscard("Leak")]] inline string normalize(const string &path) {
#if OS == WINDOWS
    return normalize_windows(path);
#else
    return normalize_posix(path);
#endif
}

}  // namespace path

LSTD_END_NAMESPACE
