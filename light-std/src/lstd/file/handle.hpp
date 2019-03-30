#pragma once

#include "path.hpp"

LSTD_BEGIN_NAMESPACE

namespace file {

// This structure is immutable.
// To change the file/directory this is pointing to, simply create a new one.
struct Handle {
   private:
    struct Iterator : NonCopyable {
       private:
        uptr_t PlatformHandle = 0;
        byte PlatformFileInfo[592];
        file::Path Path;
        size_t Index = 0;

       public:
        Iterator() {}
        Iterator(const file::Path &path);

        void operator++() { (*this)++; }
        void operator++(s32);

        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const { return !(*this == other); }

        string operator*() const;

       private:
        void read_next_entry();

        friend struct Handle;
    };

   public:
    using iterator = Iterator;
    using visit_func_t = Delegate<void(const string &)>;

    const Path Path;

    Handle(const file::Path &path);

    // Get a handle relative to this Handle's path
    Handle open_relative(file::Path path) const;

    // is_file() doesn't always equal !is_directory()
    bool is_file() const;
    // is_file() doesn't always equal !is_directory()
    bool is_directory() const;
    // is_file() || is_directory()
    bool exists() const;

    bool is_symbolic_link() const;

    size_t file_size() const;

    // TODO: Replace return value when we have some kind of Time structure
    u64 creation_time() const;
    u64 last_access_time() const;
    u64 last_modification_time() const;

    // Creates a directory with this path if it doesn't exist yet
    bool create_directory() const;

    // Only works if this handle points to a valid file (not a directory)
    bool delete_file();

    // Removes a directory with this path if it's empty
    bool delete_directory() const;

    // Removes all directory's contents, then the directory itself
    void delete_directory_with_contents() const;

    // Copies all contents to a destination directory
    // Creates destination if it doesn't exist.
    // Destination points to the actual directory that is to be created, not its parent!
    //
    // e.g.
    //      Handle dir("/my_project/");
    //      dir.copy_directory_contents(dir.open("../backups/my_project/"))
    //
    void copy_directory_contents(const Handle &destination) const;

    // Copies a file to destination
    // Destination can point to another file (in which case it gets overwritten if it exists and the parameter is true)
    // or a directory (in which case the file name is kept the same or determined by the OS)
    bool copy(const Handle &destination, bool overwrite = true) const;

    // Moves a file to destination
    // Destination can point to another file (in which case it gets overwritten if it exists and the parameter is true)
    // or a directory (in which case the file name is kept the same or determined by the OS)
    bool move(const Handle &destination, bool overwrite = true) const;

    // Renames file/directory
    bool rename(const string_view &newName) const;

    // A hard link is a way to represent a single file by more than one path.
    // Hard links continue to work fine if you delete the source file since they use reference counting.
    // Hard links can be created to files (not directories) only on the same volume.
    //
    // Destination must exist, otherwise this function fails.
    bool create_hard_link(const Handle &destination) const;

    // Symbolic links are different from hard links. Hard links do not link paths on different
    // volumes or file systems, whereas symbolic links may point to any file or directory
    // irrespective of the volumes on which the link and target reside.
    //
    // Hard links always refer to an existing file, whereas symbolic links may contain an
    // arbitrary path that does not point to anything.
    //
    // Destination must exist, otherwise this function fails.
    bool create_symbolic_link(const Handle &destination) const;

    // If this Handle is pointing to a directory,
    // call _func_ on each file/subdirectory recursively.
    //
    // (To traverse non-recursively, just use for(auto it: handle) {...})
    void traverse_recursively(visit_func_t func) const;

    iterator begin() const { return Iterator(Path); }
    iterator end() const { return Iterator(); }

    string compute_base64() const;

   private:
    void traverse_recursively(const Handle &first, Handle::visit_func_t func) const;

    // Used on Windows only
    Shared_Memory<wchar_t> PathUtf16;
};

}  // namespace file

LSTD_END_NAMESPACE
