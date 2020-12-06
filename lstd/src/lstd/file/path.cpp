#include "path.h"

LSTD_BEGIN_NAMESPACE

namespace path {

split_result split_windows(const string &path) {
    auto [driveOrUNC, rest] = split_drive_windows(path);

    // Set i to index beyond path's last slash
    s64 i = find_reverse_any_of(rest, "/\\") + 1;

    string head = rest(0, i);
    string tail = rest(i, rest.Length);

    string trimmed = head(0, find_reverse_not_any_of(head, "/\\") + 1);
    if (trimmed) head = trimmed;

    head = path(0, head.Length + driveOrUNC.Length);

    return {head, tail};
}

split_result split_posix(const string &path) {
    s64 i = find_reverse_any_of(path, "/") + 1;

    string head = path(0, i);
    string tail = path(i, path.Length);

    // If head exists and doesn not consist only of slashes
    if (head && find_cp_not(head, '/') != -1) {
        head = head(0, find_cp_not(head, '/', true) + 1);
    }

    return {head, tail};
}

static array<string> split_path_into_components(const string &path) {
    array<string> result;
    s64 start = 0, prev = 0;
    while ((start = find_any_of(path, "/\\", start + 1)) != -1) {
        append(result, path(prev, start));
        prev = start + 1;
    }

    // There is an edge case in which the path ends with a slash, in that case there is no "another" component.
    // both /home/user/dir and /home/user/dir/ mean the same thing. You can use other functions to check if they are really directories (querying the OS).
    if (prev < path.Length) {
        append(result, path(prev, path.Length));
    }
    return result;
}

// Ported from the python library:
//
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

string normalize_windows(const string &path) {
    if (match_beginning(path, "\\\\.\\") || match_beginning(path, "\\\\?\\")) {
        // In the case of paths with these prefixes:
        // \\.\ -> device names
        // \\?\ -> literal paths
        // do not do any normalization, but return the path unchanged.
        return path;
    }

    string result;
    reserve(result, path.Length);

    auto [driveOrUnc, rest] = split_drive_windows(path);
    if (driveOrUnc) {
        append_string(result, driveOrUnc);
    }

    // Collapse leading slashes
    if (is_sep(rest[0])) {
        append_cp(result, '\\');
        while (is_sep(rest[0])) advance_cp(&rest, 1);
    }

    auto components = split_path_into_components(rest);
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
            } else if (i == 0 && result && is_sep(result[-1])) {
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

string normalize_path_posix(const string &path) {
    assert(false && "Not implemented");
    return "";
}

}  // namespace path

LSTD_END_NAMESPACE