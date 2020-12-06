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

        array<string> FilesAssociated;  // @Leak
        delegate<void(const array_view<string> &)> Callback;

        bool Watched;
        array<time_t> LastWriteTimes;  // @Leak
    };

    string Root;
    bucket_array<entity, 256> Entities;

    catalog() {}
    catalog(const string &root);

    void release() {
        free(Root);
        free(Entities);
    }

    void ensure_initted(const string &root);

    void load(const array_view<string> &files, const delegate<void(const array_view<string> &)> &callback, bool watch, allocator alloc = {});
};

LSTD_END_NAMESPACE
