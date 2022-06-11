module;

#include "../common.h"

export module lstd.path.posix;

import lstd.path.general;

//
// This module provides facilities to work with paths and files, POSIX version.
// Also provides facilities to query the OS and move/copy/read/write, etc.
// All functions are prefixed with "path_" so you can find them easily with autocomplete.
//
// We import this module when compiling for POSIX platforms.
// If you want to explicitly work with POSIX paths, import this module directly.
//

LSTD_BEGIN_NAMESPACE

export {
    constexpr char OS_PATH_SEPARATOR = '/';

    always_inline constexpr bool path_is_sep(code_point ch) { return ch == '/'; }

    // Returns whether a path is absolute.
    // Trivial in POSIX (starts with '/'), harder on Windows.
    //
    // e.g.
    //    /home/user/me       -> true
    //    ./data/myData       -> false
    //    ../data/myData      -> false
    //    data/myData         -> false
    constexpr bool path_is_absolute(string path) { return path_is_sep(path[0]); }

    // Joins two or more paths.
    // Ignore the previous parts if a part is absolute.
    // This is the de facto way to build paths. Takes care of slashes automatically.
    [[nodiscard("Leak")]] string path_join(array<string> paths) {
        assert(false && "Not implemented");
        return "";
    }

    [[nodiscard("Leak")]] always_inline string path_join(string one, string other) {
        auto arr = make_stack_array(one, other);
        return path_join(arr);
    }

    // Normalize a pathname by collapsing redundant separators and up-level references so that A//B, A/B/, A/./B and A/foo/../B all become A/B.
    // This string manipulation may change the meaning of a path that contains symbolic links.
    string path_normalize(string path) {
        assert(false && "Not implemented");
        return "";
    }

    // Splits path into two components: head (everything up to the last '/') and tail (the rest).
    // The resulting head won't end in '/' unless it is the root.
    //
    // Note: The returned strings are substrings so they shouldn't be freed.
    path_split_result path_split(string path) {
        s64 i = string_find(path, '/', -1, true) + 1;

        string head = substring(path, 0, i);
        string tail = substring(path, i, string_length(path));

        // If head exists and doesn not consist only of slashes
        if (head && string_find_not(head, '/') != -1) {
            head = substring(head, 0, string_find_not(head, '/', -1, true) + 1);
        }

        return {head, tail};
    }

    // Returns the final component of the path
    // e.g.
    //    /home/user/me/     ->
    //    /home/user/me.txt  -> me.txt
    //    /home/user/dir     -> dir
    //
    // Note: The result is a substring and shouldn't be freed.
    always_inline string path_base_name(string path) {
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
    always_inline string path_directory(string path) {
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
    always_inline constexpr path_split_extension_result path_split_extension(string path) {
        return path_split_extension_general(path, '/', 0, '.');
    }
}

LSTD_END_NAMESPACE
