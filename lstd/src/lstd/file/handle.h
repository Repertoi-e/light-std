#pragma once

#include "../memory/delegate.h"
#include "path.h"

LSTD_BEGIN_NAMESPACE

namespace file {

// @BadAPI Honestly this whole struct shouldn't exist
struct handle {
    // The mode used when writing to a file
    enum write_mode {
        Append = 0,

        // If the file is 50 bytes and you write 20,
        // "Overwrite" keeps those 30 bytes at the end
        // while "Overwrite_Entire" deletes them.
        Overwrite,
        Overwrite_Entire,
    };

    mutable string Path;

    handle() {}
    handle(const string &str) : Path(str) {}

    // is_file() doesn't always equal !is_directory()
    bool is_file() const;
    // is_file() doesn't always equal !is_directory()
    bool is_directory() const;
    // is_file() || is_directory()
    bool exists() const;

    bool is_symbolic_link() const;

    s64 file_size() const;

    time_t creation_time() const;
    time_t last_access_time() const;
    time_t last_modification_time() const;

    // Creates a directory with this path if it doesn't exist yet
    bool create_directory() const;

    // Only works if this handle points to a valid file (not a directory)
    bool delete_file() const;

    // Removes a directory with this path if it's empty
    bool delete_directory() const;

    // Copies a file to destination.
    // Destination can point to another file (in which case it gets overwritten if it exists and the parameter is true)
    // or a directory (in which case the file name is kept the same or determined by the OS)
    bool copy(handle dest, bool overwrite = true) const;

    // Moves a file to destination.
    // Destination can point to another file (in which case it gets overwritten if it exists and the parameter is true)
    // or a directory (in which case the file name is kept the same or determined by the OS)
    //
    // @Leak Currently these functions replace the _Path_
    // @TODO Move these outside _handle_ since it's a strange API decision anyway. Honestly this whole struct shouldn't exist.
    bool move(handle dest, bool overwrite = true) const;

    // Renames file/directory
    //
    // @Leak Currently these functions replace the _Path_
    // @TODO Move these outside _handle_ since it's a strange API decision anyway. Honestly this whole struct shouldn't exist.
    bool rename(const string &newName) const;

    // A hard link is a way to represent a single file by more than one path.
    // Hard links continue to work fine if you delete the source file since they use reference counting.
    // Hard links can be created to files (not directories) only on the same volume.
    //
    // Destination must exist, otherwise this function fails.
    bool create_hard_link(handle dest) const;

    // Symbolic links are different from hard links. Hard links do not link paths on different
    // volumes or file systems, whereas symbolic links may point to any file or directory
    // irrespective of the volumes on which the link and target reside.
    //
    // Hard links always refer to an existing file, whereas symbolic links may contain an
    // arbitrary path that does not point to anything.
    //
    // Destination must exist, otherwise this function fails.
    bool create_symbolic_link(handle dest) const;

    // If this handle is pointing to a directory,
    // call _func_ on each file/directory inside of it.
    void traverse(const delegate<void(const string &)> &func) const {
        assert(is_directory());
        if (!path::is_sep(Path[-1])) {
            string newPath = path::join(Path, "");
            free(Path);
            Path = newPath;
        }
        traverse_impl(func);
    }

    // If this handle is pointing to a directory,
    // call _func_ on each file/subdirectory recursively.
    void traverse_recursively(const delegate<void(const string &)> &func) const {
        assert(is_directory());
        if (!path::is_sep(Path[-1])) {
            string newPath = path::join(Path, "");
            free(Path);
            Path = newPath;
        }
        traverse_recursively_impl(Path, Path, func);
    }

    struct read_entire_file_result {
        bytes Content;
        bool Success;
    };

    // Reads entire file (no async variant available at the moment). Caller is responsible for freeing the string.
    [[nodiscard("Leak")]] read_entire_file_result read_entire_file() const;

    // Write the data memory points to to a file.
    // Returns true on success.
    // (no async variant at the moment)
    bool write_to_file(const string &contents, write_mode policy = write_mode::Overwrite_Entire) const;

    //
    // Iterator:
    //
    struct iterator : non_copyable {
        void *Handle = null;
        char PlatformFileInfo[592]{};  // This is the size of the Windows structure. We don't want to include Windows.h here..

        string CurrentFileName;

        string Path;
        s64 Index = 0;

        iterator() {}
        iterator(const string &path) : Path(path) { read_next_entry(); }

        // I know we are against hidden freeing but having this destructor is actually really fine.
        // Things would be a whole lot more ugly and complicated without it.
        ~iterator() { free(CurrentFileName); }

        void operator++() { (*this)++; }
        void operator++(s32) { read_next_entry(); }

        bool operator==(const iterator &other) const {
            if (!Handle && !other.Handle) return true;
            if (Handle && other.Handle) {
                if (*(*this) == *other) return true;
            }
            return false;
        }

        bool operator!=(const iterator &other) const { return !(*this == other); }

        // The returned string is valid as long as this iterator is valid
        string operator*() const { return CurrentFileName; }

       private:
        void read_next_entry();
    };

    iterator begin() const { return iterator(Path); }
    iterator end() const { return iterator(); }

   private:
    void traverse_impl(const delegate<void(const string &)> &func) const;
    void traverse_recursively_impl(const string &first, const string &currentDirectory, const delegate<void(const string &)> &func) const;
};

}  // namespace file

LSTD_END_NAMESPACE
