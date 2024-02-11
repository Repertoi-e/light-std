#pragma once

#include "../array.h"
#include "../common.h"
#include "../string.h"

//
// This module provides facilities to work with paths and files.
// Also provides facilities to query the OS and move/copy/read/write, etc.
// All functions are prefixed with "path_" so you can find them easily with
// autocomplete.
//
// We treat both / and \ as valid separators, to be compatible with all OSes.
//

LSTD_BEGIN_NAMESPACE

// Returns whether a path is absolute.
// Trivial in POSIX (starts with '/'), harder on Windows.
// For Windows it is absolute if it starts with a slash or backslash (current
// volume), or if it begins with a volume letter or UNC-resource.
//
// e.g.
//    /home/user/me       -> true
//    C:/Users/User       -> true
//    \\host\computer\dir -> true
//    ./data/myData       -> false
//    ../data/myData      -> false
//    data/myData         -> false
bool path_is_absolute(string path);

inline const char OS_PATH_SEPARATOR = '\\';

always_inline bool path_is_sep(code_point ch) {
  return ch == '\\' || ch == '/';
}

struct path_split_drive_result {
  string DriveOrUNC, Path;
};

// This function is only relevant for NT. POSIX paths don't have drives/UNC
// share points.
//
// Split a pathname into drive/UNC sharepoint and relative path specifiers.
// Returns a _DriveOrUNC_ and _Path_ strings, either part may be empty.
// They are substrings so they shouldn't be freed.
//
// The acceptable slashes for the UNC sharepoint are \. The path after that
// might contain /, we allow that (as well as \ of course). In any case, make
// sure the UNC part at least starts with \. I'm not sure if Windows even allows
// network paths with forward slashes. I wish it wasn't such a mess..........
//
// e.g.
//    c:/dir               -> { "c:", "/dir" }
//    \\host\computer/dir  -> { "\\host\computer", "/dir" }
//
// Paths cannot contain both a drive letter and a UNC path.
path_split_drive_result path_split_drive(string path);

// Joins two or more paths.
// Ignore the previous parts if a part is absolute.
// This is the de facto way to build paths. Takes care of slashes automatically.
mark_as_leak string path_join(array<string> paths);

mark_as_leak string path_join(string one, string other);

// Normalize a pathname by collapsing redundant separators and up-level
// references so that A//B, A/B/, A/./B and A/foo/../B all become A/B. This
// string manipulation may change the meaning of a path that contains symbolic
// links.
//
// On Windows, it also converts forward slashes to backward slashes.
//
// There is an edge case in which the path ends with a slash,
// both /home/user/dir and /home/user/dir/ mean the same thing.
// You can use other functions to check if they are really
// directories or files (by querying the OS).
mark_as_leak string path_normalize(string path);

struct path_split_result {
  string Head, Tail;
};

// Splits path into two components: head (everything up to the last '/') and
// tail (the rest). The resulting head won't end in '/' unless it is the root.
//
// The Windows version handles \ and drive letters/UNC sharepoints of course.
//
// Note: The returned strings are substrings so they shouldn't be freed.
path_split_result path_split(string path);

// Returns an array with every element of a path, splits on slashes.
mark_as_leak inline array<string> path_split_into_components(string path, string seps = "\\/");

// Returns the final component of the path
// e.g.
//    /home/user/me/     ->
//    /home/user/me.txt  -> me.txt
//    /home/user/dir     -> dir
//
// Note: The result is a substring and shouldn't be freed.

string path_base_name(string path);

// Returns everything before the final component of the path
// e.g.
//    /home/user/me/     -> /home/user/me
//    /home/user/me.txt  -> /home/user
//    /home/user/dir     -> /home/user
//
// Note: The result is a substring and shouldn't be freed.
string path_directory(string path);

struct path_split_extension_result {
  string Root, Extension;
};

// Split a path in root and extension.
// The extension is everything starting at the last dot in the
// last pathname component; the root is everything before that.
//
//    /home/user/me.txt       -> { "/home/user/me,       ".txt" }
//    /home/user/me.data.txt  -> { "/home/user/me.data", "/txt" }
//    /home/user/me           -> { "/home/user/me",      "" }
//
// Note: The returned strings are substrings so they shouldn't be freed.
path_split_extension_result path_split_extension(string path);

//
// The following routines query the OS:
//

bool path_exists(string path);  // == is_file() || is_directory()
bool path_is_file(string path);
bool path_is_directory(string path);

bool path_is_symbolic_link(string path);

s64 path_file_size(string path);

time_t path_creation_time(string path);
time_t path_last_access_time(string path);
time_t path_last_modification_time(string path);

bool path_create_directory(string path);
bool path_delete_file(string path);
bool path_delete_directory(string path);

// Copies a file to destination.
// Destination can point to another file - in which case it gets overwritten
// (if the parameter is true) or a directory - in which case the file name is
// kept the same or determined by the OS (in the case of duplicate files).
// @Robustness: We don't handle directories.
bool path_copy(string path, string dest, bool overwrite);

// Moves a file to destination.
// Destination can point to another file - in which case it gets overwritten
// (if the parameter is true) or a directory - in which case the file name is
// kept the same or determined by the OS (in the case of duplicate files).
// @Robustness: We don't handle directories.
bool path_move(string path, string dest, bool overwrite);

// Renames a file/directory, newName shouldn't include the full path (it's just
// the new name).
bool path_rename(string path, string newName);

// A hard link is a way to represent a single file by more than one path.
// Hard links continue to work fine if you delete the source file since they use
// reference counting. Hard links can be created to files (not directories) only
// on the same volume.
//
// Destination must exist, otherwise this function fails.
bool path_create_hard_link(string path, string dest);

// Symbolic links are different from hard links. Hard links do not link paths on
// different volumes or file systems, whereas symbolic links may point to any
// file or directory irrespective of the volumes on which the link and target
// reside.
//
// Hard links always refer to an existing file, whereas symbolic links may
// contain an arbitrary path that does not point to anything.
//
// Destination must exist, otherwise this function fails.
bool path_create_symbolic_link(string path, string dest);

// This is used for traversing every file in a directory.
// This is not recursive but we define a method which does that further down.
//
// _Path_ needs to be a valid path before using it.
//
struct path_walker {
  // null in the beginning, null after calling
  // _path_read_next_entry_ and
  // there were no more files. Check this for when to stop calling
  // _path_read_next_entry_.
  void *Handle = null;  

  string Path;  // Doesn't get cloned, valid as long as the string passed in the
                // constructor is valid

  string CurrentFileName;  // Gets allocated by this object, call free after use
                           // to prevent leak

  s64 Index = 0;

#if OS == WINDOWS
  wchar *Path16 = null;
  char PlatformFileInfo[sizeof(WIN32_FIND_DATAW)]{};
#endif

  path_walker() {}
  path_walker(string path) : Path(path) {}
};

mark_as_leak array<string> path_walk(string path, bool recursively);
void path_read_next_entry(path_walker ref walker);

inline void free_path_walker(path_walker ref walker) { free(walker.CurrentFileName); }

inline string get_path_from_here_to(string here, string there) {
  if (search(here, there) == -1) {
    return there;
  } else {
    if (here.Count == there.Count) {
      return here;
    } else {
      string difference = slice(there, length(here), length(there));
      return difference;
    }
  }
}

inline path_split_drive_result path_split_drive(string path) {
  if (length(path) >= 2) {
    if (strings_match(slice(path, 0, 2), "\\\\") && path[2] != '\\') {
      // It is an UNC path

      //  vvvvvvvvvvvvvvvvvvvv drive letter or UNC path
      //  \\machine\mountpoint\directory\etc\...
      //             directory ^^^^^^^^^^^^^^^

      auto matchSeps = [](code_point cp) { return has("\\/", cp); };

      s64 index = search(path, &matchSeps, search_options{.Start = 2});
      if (index == -1) return {"", path};

      s64 index2 = search(path, &matchSeps, search_options{.Start = index + 1});

      // A UNC path can't have two slashes in a row
      // (after the initial two)
      if (index2 == index + 1) return {"", path};
      if (index2 == -1) {
        index2 = length(path);
      }
      return {slice(path, 0, index2), slice(path, index2, length(path))};
    }

    if (path[1] == ':') {
      return {slice(path, 0, 2), slice(path, 2, length(path))};
    }
  }

  return {"", path};
}

inline bool path_is_absolute(string path) {
  auto [_, rest] = path_split_drive(path);
  return rest.Count && path_is_sep(rest[0]);
}

mark_as_leak inline string path_join(array<string> paths) {
  assert(paths.Count >= 2);

  auto [result_drive, result_path] = path_split_drive(paths[0]);

  string result = clone(result_path);

  For(range(1, paths.Count)) {
    auto p = paths[it];
    auto [p_drive, p_path] = path_split_drive(p);
    if (p_path.Count && path_is_sep(p_path[0])) {
      // Second path is absolute
      if (p_drive.Count || !result_drive.Count) {
        result_drive = p_drive;  // These are just substrings so it's fine
      }

      free(result);
      result = clone(p_path);

      continue;
    } else if (p_drive.Count && !strings_match(p_drive, result_drive)) {
      if (!strings_match_ignore_case(p_drive, result_drive)) {
        // Different drives => ignore the first path entirely
        result_drive = p_drive;

        free(result);
        result = clone(p_path);

        continue;
      }
      // Same drives, different case
      result_drive = p_drive;
    }

    // Second path is relative to the first
    if (result.Count && !path_is_sep(result[-1])) {
      result += '/';
    }
    result += p_path;
  }

  // Add separator between UNC and non-absolute path if needed
  if (result.Count && !path_is_sep(result[0]) && result_drive.Count &&
      result_drive[-1] != ':') {
    insert_at_index(result, 0, '\\');
  } else {
    insert_at_index(result, 0, result_drive);
  }
  return result;
}

mark_as_leak inline string path_join(string one, string other) {
  auto arr = make_stack_array(one, other);
  return path_join(arr);
}

mark_as_leak inline string path_normalize(string path) {
  string result;
  reserve(result, path.Count);

  if (match_beginning(path, "\\\\.\\") || match_beginning(path, "\\\\?\\")) {
    // In the case of paths with these prefixes:
    // \\.\ -> device names
    // \\?\ -> literal paths
    // do not do any normalization, but return the path unchanged.
    free(result);
    result = clone(path);
    return result;
  }

  auto [DriveOrUNC, rest] = path_split_drive(path);
  if (DriveOrUNC.Count) {
    result += DriveOrUNC;
  }

  // Collapse leading slashes
  if (path_is_sep(rest[0])) {
    result += '\\';
    while (path_is_sep(rest[0])) advance_cp(&rest, 1);
  }

  auto components = path_split_into_components(rest);
  defer(free(components));

  s64 i = 0;
  while (i < components.Count) {
    auto it = components[i];
    if (!it.Count || strings_match(it, ".")) {
      remove_ordered_at_index(components, i);
    } else if (strings_match(it, "..")) {
      if (i > 0 && !strings_match(components[i - 1], "..")) {
        remove_range(components, i - 1, i + 1);
        --i;
      } else if (i == 0 && result.Count && path_is_sep(result[-1])) {
        remove_ordered_at_index(components, i);
      } else {
        ++i;
      }
    } else {
      ++i;
    }
  }

  // If the path is now empty, substitute "."
  if (!result.Count && !components.Count) {
    return ".";
  }

  For(components) {
    result += it;
    result += '/';
  }
  // Remove the trailing slash we added in the final iteration of the loop
  remove_at_index(result, -1);

  return result;
}

inline path_split_result path_split(string path) {
  auto [DriveOrUNC, rest] = path_split_drive(path);

  // Set i to index beyond path's last slash

  auto matchSeps =
      delegate<bool(code_point)>([](code_point cp) { return has("\\/", cp); });
  auto matchNotSeps =
      delegate<bool(code_point)>([](code_point cp) { return !has("\\/", cp); });

  s64 i =
      search(rest, matchSeps, search_options{.Start = -1, .Reversed = true}) +
      1;

  string head = slice(rest, 0, i);
  string tail = slice(rest, i, length(rest));

  string trimmed = slice(head, 0,
                         search(head, matchNotSeps,
                                search_options{.Start = -1, .Reversed = true}) +
                             1);
  if (trimmed.Count) head = trimmed;

  head = slice(path, 0, length(head) + length(DriveOrUNC));

  return {head, tail};
}

inline string path_base_name(string path) {
  auto [_, tail] = path_split(path);
  return tail;
}

inline string path_directory(string path) {
  auto [head, _] = path_split(path);
  return head;
}

mark_as_leak inline array<string> path_split_into_components(string path, string seps) {
  array<string> result;
  reserve(result);

  auto matchSep = [=](code_point cp) { return has(seps, cp); };

  s64 start = 0, prev = 0;
  while ((start = search(path, &matchSep,
                         search_options{.Start = start + 1})) != -1) {
    result += {slice(path, prev, start)};
    prev = start + 1;
  }

  // There is an edge case in which the path ends with a slash, in that case
  // there is no "another" component. The if is here so we don't crash with
  // index out of bounds.
  //
  // Note that both /home/user/dir and /home/user/dir/ mean the same thing.
  // You can use other functions to check if the former is really a directory or
  // a file (querying the OS).
  if (prev < length(path)) {
    // Add the last component - from prev to path.Length
    result += {slice(path, prev, length(path))};
  }
  return result;
}

inline path_split_extension_result path_split_extension_general(
    string path, code_point sep, code_point altSep, code_point extensionSep) {
  s64 sepIndex =
      search(path, sep, search_options{.Start = -1, .Reversed = true});
  if (altSep) {
    s64 altSepIndex =
        search(path, altSep, search_options{.Start = -1, .Reversed = true});
    if (altSepIndex > sepIndex) sepIndex = altSepIndex;
  }

  // Most OSes use a dot to separate extensions but we support other characters
  // as well
  s64 dotIndex =
      search(path, extensionSep, search_options{.Start = -1, .Reversed = true});

  if (dotIndex > sepIndex) {
    // Skip leading dots
    s64 filenameIndex = sepIndex + 1;
    while (filenameIndex < dotIndex) {
      if (path[filenameIndex] != extensionSep) {
        return {slice(path, 0, dotIndex), slice(path, dotIndex, length(path))};
      }
      ++filenameIndex;
    }
  }
  return {path, ""};
}

inline path_split_extension_result path_split_extension(string path) {
  return path_split_extension_general(path, '/', '\\', '.');
}

LSTD_END_NAMESPACE

#if OS == WINDOWS
#include "windows/path.h"
#elif OS == MACOS || OS == LINUX 
#include "posix/path.h"
#elif OS == NO_OS
// No OS (e.g. programming on baremetal).
// Let the user define interfacing with hardware.
#else
#error Implement.
#endif