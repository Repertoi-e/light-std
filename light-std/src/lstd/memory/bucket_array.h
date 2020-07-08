#pragma once

#include "array.h"
#include "delegate.h"

template <typename T, s64 ElementsPerBucket = 128>
struct bucket_array : non_copyable, non_movable, non_assignable {
    struct bucket {
        T *Elements = null;
        s64 Count = 0, Reserved = 0;
        bucket *Next = null;
    };
    bucket BaseBucket;
    bucket *BucketList = &BaseBucket;

    bucket_array() = default;
    ~bucket_array() {
        auto *b = BucketList->Next;  // The first bucket is on the stack
        while (b) {
            auto *toDelete = b;
            b = b->Next;
            delete toDelete;
        }
    }

    // Search based on predicate
    T *find(delegate<bool(T *)> predicate) {
        auto *b = BucketList;
        while (b) {
            auto index = b->Assets.find(predicate);
            if (index != -1) return (T *) b->Assets[index];
            b = b->Next;
        }
        return null;
    }

    template <typename U>
    T *find_or_create(const U &toMatch, delegate<U(T *)> map, allocator alloc = {}) {
        if (!alloc) alloc = Context.Alloc;

        T *result = find([&](T *element) { return map(element) == toMatch; });
        if (result) return result;

        result = new (alloc) T;
        add(*result);
        return result;
    }

    T *add(const T &element, allocator alloc = {}) {
        if (!alloc) alloc = Context.Alloc;

        auto *b = BucketList, *last = b;
        while (b) {
            if (b->Reserved != b->Count) {
                clone(b->Elements + b->Count, element);
                ++b->Count;
                return b->Elements + b->Count - 1;
            }
            last = b;
            b = b->Next;
        }

        b = last->Next = new (alloc) bucket;
        b->Elements = new (alloc) T[ElementsPerBucket];
        b->Reserved = ElementsPerBucket;
        clone(b->Elements, element);
        b->Count = 1;
        return b->Elements;
    }
};
