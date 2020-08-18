#pragma once

#include "../internal/context.h"

template <typename T, s64 ElementsPerBucket = 128>
struct bucket_array : non_copyable, non_movable, non_assignable {
    struct bucket {
        T *Elements = null;
        s64 Count = 0, Reserved = 0;
        bucket *Next = null;
    };
    bucket BaseBucket;
    bucket *BucketHead = null;  // null means BaseBucket

    bucket_array() {}

    void release() {
        auto *b = BucketHead->Next;  // The first bucket is on the stack
        while (b) {
            auto *toFree = b;
            b = b->Next;
            free(toFree);
        }
    }

    bucket *get_bucket_head() {
        if (!BucketHead) return &BaseBucket;
        return BucketHead;
    }

    // Search based on predicate
    T *find(const delegate<bool(T *)> &predicate) {
        auto *b = get_bucket_head();
        while (b) {
            auto index = b->Assets.find(predicate);
            if (index != -1) return (T *) b->Assets[index];
            b = b->Next;
        }
        return null;
    }

    template <typename U>
    T *find_or_create(const U &toMatch, const delegate<U(T *)> &map, allocator alloc = {}) {
        if (!alloc) alloc = Context.Alloc;

        T *result = find([&](T *element) { return map(element) == toMatch; });
        if (result) return result;

        result = allocate(T, alloc);
        add(*result);
        return result;
    }

    T *add(const T &element, allocator alloc = {}) {
        if (!alloc) alloc = Context.Alloc;

        auto *b = get_bucket_head(), *last = b;
        while (b) {
            if (b->Reserved != b->Count) {
                clone(b->Elements + b->Count, element);
                ++b->Count;
                return b->Elements + b->Count - 1;
            }
            last = b;
            b = b->Next;
        }

        b = last->Next = allocate(bucket, alloc);
        b->Elements = allocate_array(T, ElementsPerBucket, alloc);
        b->Reserved = ElementsPerBucket;
        clone(b->Elements, element);
        b->Count = 1;
        return b->Elements;
    }
};
