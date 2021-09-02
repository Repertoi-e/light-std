module;

#include "../common/context.h"
#include "../memory/array.h"
#include "../memory/string.h"

export module path.general;

//
// Here we define functions that are not specific to a platform
//

LSTD_BEGIN_NAMESPACE

export {
    [[nodiscard("Leak")]] array<string> path_split_into_components(const string &path, const string &seps = "\\/") {
        array<string> result;
        s64 start = 0, prev = 0;
        while ((start = find_any_of(path, seps, start + 1)) != -1) {
            array_append(result, path[{prev, start}]);
            prev = start + 1;
        }

        // There is an edge case in which the path ends with a slash, in that case there is no "another" component.
        // The if is here so we don't crash with index out of bounds.
        //
        // Note that both /home/user/dir and /home/user/dir/ mean the same thing.
        // You can use other functions to check if the former is really a directory or a file (querying the OS).
        if (prev < path.Length) {
            // Add the last component - from prev to path.Length
            array_append(result, path[{prev, path.Length}]);
        }
        return result;
    }

    struct path_split_extension_result {
        string Root, Extension;
    };

    constexpr path_split_extension_result path_split_extension_general(const string &path, char32_t sep, char32_t altSep, char32_t extensionSep) {
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
                    return {path[{0, dotIndex}], path[{dotIndex, path.Length}]};
                ++filenameIndex;
            }
        }
        return {path, ""};
    }

    // Used by _path_split_ implement in platform-specific modules. Exported here for sanity to make sure they use the same struct.
    struct path_split_result {
        string Head, Tail;
    };

    // Used by _path_read_entire_file_.
    struct path_read_entire_file_result {
        bytes Content;
        bool Success;
    };

    // Used by _path_write_to_file_.
    enum path_write_mode {
        Append = 0,

        // If the file is 50 bytes and you write 20,
        // "Overwrite" keeps those 30 bytes at the end
        // while "Overwrite_Entire" deletes them.
        Overwrite,
        Overwrite_Entire,
    };
}

LSTD_END_NAMESPACE