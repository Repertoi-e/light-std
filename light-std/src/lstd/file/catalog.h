#pragma once

#include "../memory/bucket_array.h"
#include "path.h"

LSTD_BEGIN_NAMESPACE

// Use this to load assets from a folder on disk.
// You can set this to automatically watch for changes and reload.

// @TODO: This is WIP, no file watching yet.
struct catalog : non_copyable, non_movable, non_assignable {
    struct entity {
        bool Loaded;

        array<file::path> FilesAssociated;
        delegate<void(array<file::path>)> Callback;

        bool Watched;
        array<time_t> LastWriteTimes;
    };

    file::path Root;
    bucket_array<entity, 256> Entities;

    catalog() = default;
    catalog(file::path root);

    void ensure_initted(file::path root);

    void load(array<file::path> files, delegate<void(array<file::path>)> callback, bool watch, allocator alloc = {});
};

LSTD_END_NAMESPACE
