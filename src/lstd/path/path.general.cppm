module;

#include "../common.h"

export module lstd.path.general;

export import lstd.string;

//
// Here we define functions that are not specific to a platform
//

LSTD_BEGIN_NAMESPACE

export {
	mark_as_leak array<string> path_split_into_components(string path, string seps = "\\/") {
		array<string> result;
		reserve(result);

		auto matchSep = [=](code_point cp) { return has(seps, cp); };

		s64 start = 0, prev = 0;
		while ((start = search(path, &matchSep, search_options{ .Start = start + 1 })) != -1) {
			result += { slice(path, prev, start) };
			prev = start + 1;
		}

		// There is an edge case in which the path ends with a slash, in that case there is no "another" component.
		// The if is here so we don't crash with index out of bounds.
		//
		// Note that both /home/user/dir and /home/user/dir/ mean the same thing.
		// You can use other functions to check if the former is really a directory or a file (querying the OS).
		if (prev < length(path)) {
			// Add the last component - from prev to path.Length
			result += { slice(path, prev, length(path)) };
		}
		return result;
	}

	struct path_split_extension_result {
		string Root, Extension;
	};

	path_split_extension_result path_split_extension_general(string path, code_point sep, code_point altSep, code_point extensionSep) {
		s64 sepIndex = search(path, sep, search_options{ .Start = -1, .Reversed = true });
		if (altSep) {
			s64 altSepIndex = search(path, altSep, search_options{ .Start = -1, .Reversed = true });
			if (altSepIndex > sepIndex) sepIndex = altSepIndex;
		}

		// Most OSes use a dot to separate extensions but we support other characters as well
		s64 dotIndex = search(path, extensionSep, search_options{ .Start = -1, .Reversed = true });

		if (dotIndex > sepIndex) {
			// Skip leading dots
			s64 filenameIndex = sepIndex + 1;
			while (filenameIndex < dotIndex) {
				if (path[filenameIndex] != extensionSep) {
					return { slice(path, 0, dotIndex), slice(path, dotIndex, length(path)) };
				}
				++filenameIndex;
			}
		}
		return { path, "" };
	}

	// Used by _path_split_ implement in platform-specific modules.
	// Exported here for sanity.
	// @TODO: Forward declare API here as well!
	struct path_split_result {
		string Head, Tail;
	};
}

LSTD_END_NAMESPACE