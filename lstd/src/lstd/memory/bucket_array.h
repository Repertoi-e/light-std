#pragma once

#include "../common/context.h"

LSTD_BEGIN_NAMESPACE

template <typename T_, s64 ElementsPerBucket = 128>
struct bucket_array {
    using T = T_;
    constexpr static s64 ELEMENTS_PER_BUCKET = ElementsPerBucket;

    struct bucket {
        T *Elements  = null;
        s64 Count    = 0, Allocated = 0;
        bucket *Next = null;
    };

    bucket BaseBucket;
    bucket *BucketHead = null; // null means BaseBucket

    bucket_array() {
    }
};

template <typename T>
struct is_bucket_array_helper : types::false_t {
};

template <typename T, s64 ElementsPerBucket>
struct is_bucket_array_helper<bucket_array<T, ElementsPerBucket>> : types::true_t {
};

template <typename T>
concept is_bucket_array = is_bucket_array_helper<T>::value;

template <is_bucket_array T>
void free(T &arr) {
    auto *b = arr.BucketHead->Next; // The first bucket is on the stack
    while (b) {
        auto *toFree = b;
        b            = b->Next;
        free(toFree);
    }
}

template <is_bucket_array T>
auto *get_bucket_head(T &arr) {
    if (!arr.BucketHead) return &arr.BaseBucket;
    return arr.BucketHead;
}

// Search based on predicate
template <is_bucket_array T>
auto *find(const T &arr, const delegate<bool(typename T::T *)> &predicate) {
    auto *b = get_bucket_head(arr);
    while (b) {
        auto index = b->Elements.find(predicate);
        if (index != -1) return (typename T::T *) b->Elements[index];
        b = b->Next;
    }
    return null;
}

template <is_bucket_array T>
auto *append(T &arr, const typename T::T &element, allocator alloc = {}) {
    if (!alloc) alloc = Context.Alloc;

    auto *b = get_bucket_head(arr), *last = b;
    while (b) {
        if (b->Allocated != b->Count) {
            clone(b->Elements + b->Count, element);
            ++b->Count;
            return b->Elements + b->Count - 1;
        }
        last = b;
        b    = b->Next;
    }

    b            = last->Next = allocate<typename T::bucket>({.Alloc = alloc});
    b->Elements  = allocate_array<typename T::T>(T::ELEMENTS_PER_BUCKET, {.Alloc = alloc});
    b->Allocated = T::ELEMENTS_PER_BUCKET;
    clone(b->Elements, element);
    b->Count = 1;
    return b->Elements;
}

template <is_bucket_array T, typename U>
auto *find_or_create(const T &arr, const U &toMatch, const delegate<U(typename T::T *)> &map, allocator alloc = {}) {
    if (!alloc) alloc = Context.Alloc;

    T *result = find(arr, [&](T *element) { return map(element) == toMatch; });
    if (result) return result;

    result = allocate<T>({.Alloc = alloc});
    append(arr, *result);
    return result;
}

LSTD_END_NAMESPACE
