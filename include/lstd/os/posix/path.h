#pragma once

#include "../../string.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

LSTD_BEGIN_NAMESPACE

// == is_file() || is_directory()
inline bool path_exists(string path) {
    struct stat buffer;
    return stat(to_c_string_temp(path), &buffer) == 0;
}

inline bool path_is_file(string path) {
    struct stat buffer;
    if (stat(to_c_string_temp(path), &buffer) != 0)
        return false;
    return S_ISREG(buffer.st_mode);
}

inline bool path_is_directory(string path) {
    struct stat buffer;
    if (stat(to_c_string_temp(path), &buffer) != 0)
        return false;
    return S_ISDIR(buffer.st_mode);
}

inline bool path_is_symbolic_link(string path) {
    struct stat buffer;
    if (lstat(to_c_string_temp(path), &buffer) != 0)
        return false;
    return S_ISLNK(buffer.st_mode);
}

inline s64 path_file_size(string path) {
    struct stat buffer;
    if (stat(to_c_string_temp(path), &buffer) != 0 || S_ISDIR(buffer.st_mode))
        return 0;
    return buffer.st_size;
}

inline time_t path_creation_time(string path) {
    struct stat buffer;
    if (stat(to_c_string_temp(path), &buffer) != 0)
        return 0;
    return buffer.st_ctime;
}

inline time_t path_last_access_time(string path) {
    struct stat buffer;
    if (stat(to_c_string_temp(path), &buffer) != 0)
        return 0;
    return buffer.st_atime;
}

inline time_t path_last_modification_time(string path) {
    struct stat buffer;
    if (stat(to_c_string_temp(path), &buffer) != 0)
        return 0;
    return buffer.st_mtime;
}

inline bool path_create_directory(string path) {
    return mkdir(to_c_string_temp(path), 0777) == 0;
}

inline bool path_delete_file(string path) {
    return unlink(to_c_string_temp(path)) == 0;
}

inline bool path_delete_directory(string path) {
    return rmdir(to_c_string_temp(path)) == 0;
}

inline bool copy_file(const char *source, const char *destination, bool overwrite) {
    int source_fd, dest_fd;
    ssize_t bytes_read;
    char buffer[4096];

    // Open source file for reading
    source_fd = open(source, O_RDONLY);
    if (source_fd == -1) {
        perror("Error opening source file");
        return false;
    }

    // Open destination file for writing
    dest_fd = open(destination, O_WRONLY | O_CREAT | (overwrite ? O_TRUNC : O_EXCL), 0666);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(source_fd);
        return false;
    }

    // Copy data from source to destination
    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("Error writing to destination file");
            close(source_fd);
            close(dest_fd);
            return false;
        }
    }

    // Check for read error
    if (bytes_read == -1) {
        perror("Error reading from source file");
        close(source_fd);
        close(dest_fd);
        return false;
    }

    // Close file descriptors
    close(source_fd);
    close(dest_fd);

    return true;
}

// Implementing path_copy function
inline bool path_copy(string path, string dest, bool overwrite) {
    // Check if the source file exists
    if (!path_exists(path)) {
        fprintf(stderr, "Source file does not exist\n");
        return false;
    }

    // Check if destination file already exists and overwrite is not allowed
    if (path_exists(dest) && !overwrite) {
        fprintf(stderr, "Destination file already exists and overwrite is disabled\n");
        return false;
    }

    const char *path_c_string = to_c_string(path);
    defer(free(path_c_string));

    return copy_file(path_c_string, to_c_string_temp(dest), overwrite);
}

// Implementing path_move function
inline bool path_move(string path, string dest, bool overwrite) {
    // Check if the source file exists
    if (!path_exists(path)) {
        fprintf(stderr, "Source file does not exist\n");
        return false;
    }

    // Check if destination file already exists and overwrite is not allowed
    if (path_exists(dest) && !overwrite) {
        fprintf(stderr, "Destination file already exists and overwrite is disabled\n");
        return false;
    }

    const char *path_c_string = to_c_string(path);
    defer(free(path_c_string));
    
    if (path_copy(path_c_string, to_c_string_temp(dest), overwrite)) {
        if (unlink(to_c_string_temp(path)) == -1) {
            perror("Error deleting source file after move");
            return false;
        }
        return true;
    } else {
        fprintf(stderr, "Error moving file\n");
        return false;
    }
}

inline bool path_rename(string path, string newName) {
    const char *path_c_string = to_c_string(path);
    defer(free(path_c_string));

    return rename(path_c_string, to_c_string_temp(newName)) == 0;
}

inline bool path_create_hard_link(string path, string dest) {
   const char *path_c_string = to_c_string(path);
    defer(free(path_c_string));

    return link(path_c_string, to_c_string_temp(dest)) == 0;
}

inline bool path_create_symbolic_link(string path, string dest) {
   const char *path_c_string = to_c_string(path);
    defer(free(path_c_string));

    return symlink(path_c_string, to_c_string_temp(dest)) == 0;
}

LSTD_END_NAMESPACE
