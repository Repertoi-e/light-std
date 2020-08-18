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
};

template <typename T, s64 ElementsPerBucket = 128>
void free(bucket_array<T, ElementsPerBucket> &arr) {
    auto *b = arr.BucketHead->Next;  // The first bucket is on the stack
    while (b) {
        auto *toFree = b;
        b = b->Next;
        free(toFree);
    }
}

template <typename T, s64 ElementsPerBucket = 128>
auto *get_bucket_head(bucket_array<T, ElementsPerBucket> &arr) {
    if (!arr.BucketHead) return &arr.BaseBucket;
    return arr.BucketHead;
}

// Search based on predicate
template <typename T, s64 ElementsPerBucket = 128>
T *find(const bucket_array<T, ElementsPerBucket> &arr, const delegate<bool(T *)> &predicate) {
    auto *b = get_bucket_head(arr);
    while (b) {
        auto index = b->Assets.find(predicate);
        if (index != -1) return (T *) b->Assets[index];
        b = b->Next;
    }
    return null;
}

template <typename T, s64 ElementsPerBucket = 128>
T *append(bucket_array<T, ElementsPerBucket> &arr, const T &element, allocator alloc = {}) {
    if (!alloc) alloc = Context.Alloc;

    auto *b = get_bucket_head(arr), *last = b;
    while (b) {
        if (b->Reserved != b->Count) {
            clone(b->Elements + b->Count, element);
            ++b->Count;
            return b->Elements + b->Count - 1;
        }
        last = b;
        b = b->Next;
    }

    b = last->Next = allocate(remove_pointer_t<decltype(b)>, alloc);
    b->Elements = allocate_array(T, ElementsPerBucket, alloc);
    b->Reserved = ElementsPerBucket;
    clone(b->Elements, element);
    b->Count = 1;
    return b->Elements;
}

template <typename T, typename U, s64 ElementsPerBucket = 128>
T *find_or_create(const bucket_array<T, ElementsPerBucket> &arr, const U &toMatch, const delegate<U(T *)> &map, allocator alloc = {}) {
    if (!alloc) alloc = Context.Alloc;

    T *result = find(arr, [&](T *element) { return map(element) == toMatch; });
    if (result) return result;

    result = allocate(T, alloc);
    append(arr, *result);
    return result;
}
