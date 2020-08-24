#pragma once

#include "../memory/array.h"
#include "../memory/bucket_array.h"
#include "path.h"

LSTD_BEGIN_NAMESPACE

// Use this to load assets from a folder on disk.
// You can set this to automatically watch for changes and reload.

// @TODO: This is WIP, no file watching yet.
struct catalog : non_copyable, non_movable, non_assignable {
    struct entity {
        bool Loaded;

        array<file::path> FilesAssociated;  // @Leak
        delegate<void(const array_view<file::path> &)> Callback;

        bool Watched;
        array<time_t> LastWriteTimes;  // @Leak
    };

    file::path Root;
    bucket_array<entity, 256> Entities;

    catalog() {}
    catalog(file::path root);

    void release() {
        Root.release();
        free(Entities);
    }

    void ensure_initted(const file::path &root);

    void load(const array_view<file::path> &files, const delegate<void(const array_view<file::path> &)> &callback, bool watch, allocator alloc = {});
};

LSTD_END_NAMESPACE
