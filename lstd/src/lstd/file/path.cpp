#include "path.h"

LSTD_BEGIN_NAMESPACE

namespace path {

// Ported from python library:
//
//    p = os.fspath(p)
//    if len(p) >= 2:
//        if isinstance(p, bytes):
//            sep = b'\\'
//            altsep = b'/'
//            colon = b':'
//        else:
//            sep = '\\'
//            altsep = '/'
//            colon = ':'
//        normp = p.replace(altsep, sep)
//        if (normp[0:2] == sep*2) and (normp[2:3] != sep):
//            # is a UNC path:
//            # vvvvvvvvvvvvvvvvvvvv drive letter or UNC path
//            # \\machine\mountpoint\directory\etc\...
//            #           directory ^^^^^^^^^^^^^^^
//            index = normp.find(sep, 2)
//            if index == -1:
//                return p[:0], p
//            index2 = normp.find(sep, index + 1)
//            # a UNC path can't have two slashes in a row
//            # (after the initial two)
//            if index2 == index + 1:
//                return p[:0], p
//            if index2 == -1:
//                index2 = len(p)
//            return p[:index2], p[index2:]
//        if normp[1:2] == colon:
//            return p[:2], p[2:]
//    return p[:0], p
split_drive_result split_drive_windows(const string &path) {
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

// Ported from the python library:
//
//    path = os.fspath(path)
//    if isinstance(path, bytes):
//        sep = b'\\'
//        seps = b'\\/'
//        colon = b':'
//    else:
//        sep = '\\'
//        seps = '\\/'
//        colon = ':'
//    try:
//        if not paths:
//            path[:0] + sep  #23780: Ensure compatible data type even if p is null.
//        result_drive, result_path = splitdrive(path)
//        for p in map(os.fspath, paths):
//            p_drive, p_path = splitdrive(p)
//            if p_path and p_path[0] in seps:
//                # Second path is absolute
//                if p_drive or not result_drive:
//                    result_drive = p_drive
//                result_path = p_path
//                continue
//            elif p_drive and p_drive != result_drive:
//                if p_drive.lower() != result_drive.lower():
//                    # Different drives => ignore the first path entirely
//                    result_drive = p_drive
//                    result_path = p_path
//                    continue
//                # Same drive in different case
//                result_drive = p_drive
//            # Second path is relative to the first
//            if result_path and result_path[-1] not in seps:
//                result_path = result_path + sep
//            result_path = result_path + p_path
//        ## add separator between UNC and non-absolute path
//        if (result_path and result_path[0] not in seps and
//            result_drive and result_drive[-1:] != colon):
//            return result_drive + sep + result_path
//        return result_drive + result_path
//    except (TypeError, AttributeError, BytesWarning):
//        genericpath._check_arg_types('join', path, *paths)
//        raise
//
string join_windows(const array_view<string> &paths) {
    assert(paths.Count >= 2);

    //        sep = '\\'
    //        seps = '\\/'
    //        colon = ':'

    auto [result_drive, result_path] = split_drive(paths[0]);

    // We need to allocate a string to append all paths to
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

string join_posix(const array_view<string> &paths) {
    assert(false && "Not implemented");
    return "";
}

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