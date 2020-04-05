#pragma once

#include "file.h"
#include "io/fmt.h"
#include "storage/table.h"

LSTD_BEGIN_NAMESPACE

// Use this to load assets from a folder on disk.
// You can set this to automatically watch for changes and reload.
struct catalog {
    struct entity {
        bool Loaded;

        array<file::path> FilesAssociated;
        delegate<void(array<file::path>)> Callback;

        bool Watched;
        array<time_t> LastWriteTimes;
    };

    struct bucket {
        array<entity> Entities;
        bucket *Next = null;
    };
    bucket BaseBucket;
    bucket *BucketList = &BaseBucket;

    s32 EntitiesPerBucket = 256;

    file::path Root;

    catalog() {}

    catalog(file::path root) { ensure_initted(root); }

    ~catalog() {
        auto *b = BucketList->Next;  // The first bucket is on the stack
        while (b) {
            auto *toDelete = b;
            b = b->Next;
            delete toDelete;
        }
    }

    void ensure_initted(file::path root) {
        if (Root.UnifiedPath.Length) return;
        assert(root.is_pointing_to_content() && "Create a catalog which points to a folder, not a file");
        clone(&Root, root);
    }

    void load(array<file::path> files, delegate<void(array<file::path>)> callback, bool watch) {
        auto *b = find_available_bucket();

        entity *e = b->Entities.append();
        e->FilesAssociated.reserve(files.Count);
        clone(&e->Callback, callback);
        e->Watched = watch;
        e->LastWriteTimes.reserve(files.Count);

        For(files) {
            file::path path = Root;
            path.combine_with(it);
            move(e->FilesAssociated.append(), &path);

            e->LastWriteTimes.append(file::handle(path).last_modification_time());
        }

        callback(e->FilesAssociated);
    }

   private:
    bucket *find_available_bucket() {
        auto *b = BucketList, *last = b;
        while (b) {
            if (b->Entities.Count != b->Entities.Reserved) return b;
            last = b;
            b = b->Next;
        }

        auto result = new bucket;
        last->Next = result;
        result->Entities.reserve(EntitiesPerBucket);
        return result;
    }
};

LSTD_END_NAMESPACE
