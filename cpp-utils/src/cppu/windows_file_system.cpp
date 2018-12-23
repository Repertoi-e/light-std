#include "common.hpp"

// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented

#if 0  // defined OS_WINDOWS

// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented
// #TODO: Not implemented

#include "file/local_file_path.h"

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

CPPU_BEGIN_NAMESPACE

static void reset_info(Local_File_Path const &path) {
    if (path.FileInfo) Delete((struct stat *) path.FileInfo);
    if (path.LinkInfo) Delete((struct stat *) path.LinkInfo);

    path.FileInfo = null;
    path.LinkInfo = null;
}

Local_File_Path::~Local_File_Path() { reset_info(*this); }

static void read_file_info(Local_File_Path const &path) {
    if (path.FileInfo) return;

    path.FileInfo = (void *) New<struct stat>();

    if (stat(path.Path.Data, (struct stat *) path.FileInfo)) {
        Delete((struct stat *) path.FileInfo);
        path.FileInfo = null;
    }
}

static void read_link_info(Local_File_Path const &path) {
    if (path.LinkInfo) return;

    path.LinkInfo = (void *) New<struct stat>();

    if (lstat(path.Path.Data, (struct stat *) path.LinkInfo)) {
        Delete((struct stat *) path.LinkInfo);
        path.LinkInfo = null;
    }
}

b32 exists(Local_File_Path const &path) {
    read_file_info(path);
    return path.FileInfo;
}

b32 is_file(Local_File_Path const &path) {
    read_file_info(path);
    if (path.FileInfo) {
        return S_ISREG(((struct stat *) path.FileInfo)->st_mode);
    }
    return false;
}

b32 is_dir(Local_File_Path const &path) {
    read_file_info(path);
    if (path.FileInfo) {
        return S_ISDIR(((struct stat *) path.FileInfo)->st_mode);
    }
    return false;
}

b32 is_symbolic_link(Local_File_Path const &path) {
    read_link_info(path);
    if (path.FileInfo) {
        return S_ISLNK(((struct stat *) path.LinkInfo)->st_mode);
    }
    return false;
}

void visit_entries(Local_File_Path const &path, Visit_Func function) {
    DIR *dir = opendir(path.Path.Data);
    if (!dir) return;
    defer { closedir(dir); };

    while (true) {
        struct dirent *entry = readdir(dir);
        if (!entry) break;

        string name = entry->d_name;
        if (name == ".." || name == ".") continue;

        function(Local_File_Path(path));
    }
}

size_t file_size(Local_File_Path const &path) {
    if (!is_file(path)) return 0;
    return ((struct stat *) path.FileInfo)->st_size;
}

u32 last_access_time(Local_File_Path const &path) {
    read_file_info(path);
    if (path.FileInfo) {
        return ((struct stat *) path.FileInfo)->st_atime;
    }
    return 0;
}

u32 last_write_time(Local_File_Path const &path) {
    read_file_info(path);
    if (path.FileInfo) {
        return ((struct stat *) path.FileInfo)->st_mtime;
    }
    return 0;
}

b32 remove(Local_File_Path const &path) {
    if (!exists(path)) return false;

    if (::remove(path.Path.Data)) {
        return false;
    }

    reset_info(path);
    return true;
}

b32 rename(Local_File_Path const &path, string const &name) {
    if (!exists(path)) return false;

    // Not implemented
    return false;
}

CPPU_END_NAMESPACE

#endif  // defined OS_WINDOWS