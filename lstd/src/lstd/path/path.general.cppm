module;

#include "../common.h"

export module lstd.path.general;

export import lstd.string;

//
// Here we define functions that are not specific to a platform
//

LSTD_BEGIN_NAMESPACE

export {
    [[nodiscard("Leak")]] array<string> path_split_into_components(string path, string seps = "\\/") {
        array<string> result;
        s64 start = 0, prev = 0;
        while ((start = string_find_any_of(path, seps, start + 1)) != -1) {
            add(&result, substring(path, prev, start));
            prev = start + 1;
        }

        // There is an edge case in which the path ends with a slash, in that case there is no "another" component.
        // The if is here so we don't crash with index out of bounds.
        //
        // Note that both /home/user/dir and /home/user/dir/ mean the same thing.
        // You can use other functions to check if the former is really a directory or a file (querying the OS).
        if (prev < string_length(path)) {
            // Add the last component - from prev to path.Length
            add(&result, substring(path, prev, string_length(path)));
        }
        return result;
    }

    struct path_split_extension_result {
        string Root, Extension;
    };

    constexpr path_split_extension_result path_split_extension_general(string path, code_point sep, code_point altSep, code_point extensionSep) {
        s64 sepIndex = string_find(path, sep, string_length(path), true);
        if (altSep) {
            s64 altSepIndex = string_find(path, altSep, string_length(path), true);
            if (altSepIndex > sepIndex) sepIndex = altSepIndex;
        }

        // Most OSes use a dot to separate extensions but we support other characters as well
        s64 dotIndex = string_find(path, extensionSep, string_length(path), true);

        if (dotIndex > sepIndex) {
            // Skip leading dots
            s64 filenameIndex = sepIndex + 1;
            while (filenameIndex < dotIndex) {
                if (path[filenameIndex] != extensionSep) {
                    return {substring(path, 0, dotIndex), substring(path, dotIndex, string_length(path))};
                }
                ++filenameIndex;
            }
        }
        return {path, ""};
    }

    // Used by _path_split_ implement in platform-specific modules.
    // Exported here for sanity.
    // @TODO: Forward declare API here as well!
    struct path_split_result {
        string Head, Tail;
    };
}

LSTD_END_NAMESPACE