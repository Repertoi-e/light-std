#pragma once

#include "../../fmt.h"

#include "api.h"  // Declarations of Win32 functions
#include "../memory.h"

LSTD_BEGIN_NAMESPACE

// Return an array of all files in a directory.
// _recursively_ determines if files in subdirectories are included.
//
// If you don't want the overhead of us building an array you can use
// the path_walker API directly (take a look at the implementation of this
// function further down the file).
mark_as_leak array<string> path_walk(string path, bool recursively = false);

#define GET_READONLY_EXISTING_HANDLE(x, fail)                              \
  CREATE_FILE_HANDLE_CHECKED(                                              \
      x,                                                                   \
      CreateFileW(platform_utf8_to_utf16(path), GENERIC_READ,              \
                  FILE_SHARE_READ | FILE_SHARE_WRITE, null, OPEN_EXISTING, \
                  FILE_ATTRIBUTE_NORMAL, NULL),                            \
      fail);                                                               \
  defer(CloseHandle(x));

// == is_file() || is_directory()
inline bool path_exists(string path) {
  HANDLE file =
      CreateFileW(platform_utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ,
                  null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
  if (file == INVALID_HANDLE_VALUE) return false;
  CloseHandle(file);
  return true;
}

inline bool path_is_file(string path) {
  HANDLE file = CreateFileW(platform_utf8_to_utf16(path), 0, 0, null,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, null);
  if (file == INVALID_HANDLE_VALUE) return false;
  defer(CloseHandle(file));

  BY_HANDLE_FILE_INFORMATION info;
  if (!GetFileInformationByHandle(file, &info)) return false;
  return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

inline bool path_is_directory(string path) {
  HANDLE file =
      CreateFileW(platform_utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ,
                  null, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, null);
  if (file == INVALID_HANDLE_VALUE) return false;
  defer(CloseHandle(file));

  BY_HANDLE_FILE_INFORMATION info;
  if (!GetFileInformationByHandle(file, &info)) return false;
  return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

inline bool path_is_symbolic_link(string path) {
  DWORD attribs = GetFileAttributesW(platform_utf8_to_utf16(path));
  if (attribs != INVALID_FILE_ATTRIBUTES) {
    return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
  }
  return false;
}

inline s64 path_file_size(string path) {
  if (path_is_directory(path)) return 0;

  CREATE_FILE_HANDLE_CHECKED(
      file,
      CreateFileW(platform_utf8_to_utf16(path), GENERIC_READ, FILE_SHARE_READ,
                  null, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, null),
      0);
  defer(CloseHandle(file));

  LARGE_INTEGER size = {0};
  GetFileSizeEx(file, &size);
  return size.QuadPart;
}

inline time_t path_creation_time(string path) {
  GET_READONLY_EXISTING_HANDLE(handle, 0);
  FILETIME time;
  if (!GetFileTime(handle, &time, null, null)) return 0;
  return ((time_t)time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

inline time_t path_last_access_time(string path) {
  GET_READONLY_EXISTING_HANDLE(handle, 0);
  FILETIME time;
  if (!GetFileTime(handle, null, &time, null)) return 0;
  return ((time_t)time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

inline time_t path_last_modification_time(string path) {
  GET_READONLY_EXISTING_HANDLE(handle, 0);
  FILETIME time;
  if (!GetFileTime(handle, null, null, &time)) return 0;
  return ((time_t)time.dwHighDateTime) << 32 | time.dwLowDateTime;
}

inline bool path_create_directory(string path) {
  if (path_exists(path)) return false;
  return CreateDirectoryW(platform_utf8_to_utf16(path), null);
}

inline bool path_delete_file(string path) {
  if (!path_is_file(path)) return false;
  return DeleteFileW(platform_utf8_to_utf16(path));
}

inline bool path_delete_directory(string path) {
  if (!path_is_directory(path)) return false;
  return RemoveDirectoryW(platform_utf8_to_utf16(path));
}

// @Robustness: Handle directories?
inline bool path_copy(string path, string dest, bool overwrite) {
  if (!path_is_file(path)) return false;

  auto *u16 = platform_utf8_to_utf16(path);

  if (path_is_directory(dest)) {
    auto p = path_join(dest, path_base_name(path));
    defer(free(p));

    return CopyFileW(u16, platform_utf8_to_utf16(p), !overwrite);
  }
  return CopyFileW(u16, platform_utf8_to_utf16(dest), !overwrite);
}

// @Robustness: Handle directories?
inline bool path_move(string path, string dest, bool overwrite) {
  if (!path_is_file(path)) return false;

  if (path_is_directory(dest)) {
    auto p = path_join(dest, path_base_name(path));
    defer(free(p));

    return MoveFileExW(platform_utf8_to_utf16(path), platform_utf8_to_utf16(p),
                       MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED |
                           (overwrite ? MOVEFILE_REPLACE_EXISTING : 0));
  }
  return MoveFileExW(platform_utf8_to_utf16(path), platform_utf8_to_utf16(dest),
                     MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED |
                         (overwrite ? MOVEFILE_REPLACE_EXISTING : 0));
}

inline bool path_rename(string path, string newName) {
  if (!path_exists(path)) return false;

  auto p = path_join(path_directory(path), newName);
  defer(free(p));

  return MoveFileW(platform_utf8_to_utf16(path), platform_utf8_to_utf16(p));
}

inline bool path_create_hard_link(string path, string dest) {
  if (!path_is_directory(path)) return false;
  if (!path_is_directory(dest)) return false;
  return CreateHardLinkW(platform_utf8_to_utf16(dest),
                         platform_utf8_to_utf16(path), null);
}

inline bool path_create_symbolic_link(string path, string dest) {
  if (!path_exists(path)) return false;
  if (!path_exists(dest)) return false;

  u32 flag = path_is_directory(dest) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
  return CreateSymbolicLinkW(platform_utf8_to_utf16(dest),
                             platform_utf8_to_utf16(path), flag);
}

inline void path_read_next_entry(path_walker ref walker) {
  do {
    if (!walker.Handle) {
      if (!walker.Path16) {
        string queryPath = path_join(walker.Path, "*");
        defer(free(queryPath));

        walker.Path16 = platform_utf8_to_utf16(queryPath);
      }

      string path = walker.Path;
      CREATE_FILE_HANDLE_CHECKED(
          f,
          FindFirstFileW(walker.Path16,
                         (WIN32_FIND_DATAW *)walker.PlatformFileInfo),
          ;);
      walker.Handle = (void *)f;
    } else {
      u32 ERROR_NO_MORE_FILES = 18;

#define CHECK_FIND_NEXT(call)                                                  \
  if (!call) {                                                                 \
    if (GetLastError() != ERROR_NO_MORE_FILES) {                               \
      windows_report_hresult_error(HRESULT_FROM_WIN32(GetLastError()), #call); \
    }                                                                          \
    if (walker.Handle != INVALID_HANDLE_VALUE) {                               \
      WIN32_CHECK_BOOL(r, FindClose((HANDLE)walker.Handle));                   \
    }                                                                          \
                                                                               \
    walker.Handle = null; /* No more files.. terminate */                      \
    return;                                                                    \
  }
      CHECK_FIND_NEXT(FindNextFileW(
          (HANDLE)walker.Handle, (WIN32_FIND_DATAW *)walker.PlatformFileInfo));
    }
    ++walker.Index;

    free(walker.CurrentFileName);

    auto *fileName = ((WIN32_FIND_DATAW *)walker.PlatformFileInfo)->cFileName;
    reserve(walker.CurrentFileName, c_string_length(fileName) * 4);  // @Cleanup
    utf16_to_utf8(fileName, (char *)walker.CurrentFileName.Data,
                  &walker.CurrentFileName.Count);  // @Constcast

  } while (strings_match(walker.CurrentFileName, "..") ||
           strings_match(walker.CurrentFileName, "."));
  assert(!strings_match(walker.CurrentFileName, "..") &&
         !strings_match(walker.CurrentFileName, "."));
}

// This version appends paths to the array _result_. Copy this and modify it to
// suit your use case.
inline void path_walk_recursively_impl(string path, string first,
                                       array<string> ref result) {
  assert(path_is_directory(path));

  auto walker = path_walker(path);
  defer(free_path_walker(walker));

  while (true) {
    path_read_next_entry(walker);
    if (!walker.Handle) break;

    string p =
        path_join(get_path_from_here_to(first, path), walker.CurrentFileName);
    result += {p};

    if (path_is_directory(p)) {
      path_walk_recursively_impl(p, first, result);
    }
  }
}

inline mark_as_leak array<string> path_walk(string path, bool recursively) {
  assert(path_is_directory(path));

  array<string> result;

  if (!recursively) {
    auto walker = path_walker(path);
    defer(free_path_walker(walker));

    while (true) {
      path_read_next_entry(walker);
      if (!walker.Handle) break;

      string file = path_join(path, walker.CurrentFileName);
      result += {file};
    }
  } else {
    path_walk_recursively_impl(path, path, result);
  }
  return result;
}

LSTD_END_NAMESPACE
