#pragma once

#include "file.h"
#include "io/fmt.h"
#include "storage/table.h"

LSTD_BEGIN_NAMESPACE

// @TODO
// inline size_t g_NextAssetID = 0;
// uptr_t get_hash(const asset &value) { return value.ID; }
// using create_asset_t = T *(*) (const asset_info &info);
// create_asset_t CreateAssetFunc = null;
// using destroy_asset_t = void (*)(T *asset);
// destroy_asset_t DestroyAssetFunc = null;

struct asset {
    string Name;
    file::path FilePath;
};

// @TODO: Hot reloading assets

template <typename T>
struct catalog {
    struct bucket {
        array<asset *> Assets;
        bucket *Next = null;
    };
    bucket BaseBucket;
    bucket *BucketList = &BaseBucket;

    catalog() { BaseBucket.Assets.reserve(128); }
    ~catalog() {
        auto *b = BucketList->Next;  // The first bucket is on the stack
        while (b) {
            auto *toDelete = b;
            b = b->Next;
            delete toDelete;
        }
    }

    void add(T *asst) {
        auto *b = BucketList, *last = b;
        while (b) {
            if (b->Assets.Reserved != b->Assets.Count) {
                b->Assets.append(asst);
                return;
            }
            last = b;
            b = b->Next;
        }

        last->Next = new bucket;
        last->Next->Assets.append(asst);
    }

    T *get(string name) {
        auto *b = BucketList;
        while (b) {
            auto index = b->Assets.find([&](auto x) { return x->Name == name; });
            if (index != npos) return (T *) b->Assets[index];
            b = b->Next;
        }
        return null;
    }
};

LSTD_END_NAMESPACE
