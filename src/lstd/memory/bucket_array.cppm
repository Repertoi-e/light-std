module;

#include "../common.h"

export module lstd.bucket_array;

export import lstd.context;
export import lstd.delegate;

LSTD_BEGIN_NAMESPACE

export {
    template <typename T_, s64 ElementsPerBucket = 128>
    struct bucket_array {
        using T                                  = T_;
        constexpr static s64 ELEMENTS_PER_BUCKET = ElementsPerBucket;

        struct bucket {
            T *Data      = null;
            s64 Count    = 0;
            bucket *Next = null;
        };

        bucket BaseBucket;
        bucket *BucketHead = null;  // null means BaseBucket

        allocator Alloc;
    };

    template <typename>
    constexpr bool is_bucket_array = false;

    template <typename T, s64 N>
    constexpr bool is_bucket_array<bucket_array<T, N>> = true;

    template <typename T>
    concept any_bucket_array = is_bucket_array<T>;

    void free_bucket_array(any_bucket_array auto * arr) {
        auto *b = arr->BucketHead->Next;  // The first bucket is on the stack
        while (b) {
            auto *toFree = b;

            b = b->Next;
            free(toFree);
        }
    }

    auto *get_bucket_head(any_bucket_array auto * arr) {
        if (!arr->BucketHead) return &arr->BaseBucket;
        return arr.BucketHead;
    }

    // Search based on predicate
    template <any_bucket_array Arr>
    auto *find(Arr * arr, delegate<bool(typename Arr::T *)> predicate) {
        auto *b = get_bucket_head(arr);
        while (b) {
            auto *p = b->Data;
            For(range(b->Count)) {
                if (predicate(p)) return p;
            }
            b = b->Next;
        }
        return null;
    }

    template <any_bucket_array Arr>
    auto *append(Arr * arr, const typename Arr::T &element) {
        auto *b    = get_bucket_head(arr);
        auto *last = b;

        while (b) {
            if (b->Count != Arr::ELEMENTS_PER_BUCKET) {
                *(b->Data + b->Count) = element;
                ++b->Count;
                return b->Data + b->Count - 1;
            }
            last = b;
            b    = b->Next;
        }

        if (!arr->Alloc) arr->Alloc = Context.Alloc;

        b = last->Next = malloc<typename Arr::bucket>({.Alloc = arr->Alloc});
        b->Elements    = malloc<typename Arr::T>({.Count = Arr::ELEMENTS_PER_BUCKET, .Alloc = arr->Alloc});
        *b->Data       = element;
        b->Count = 1;
        return b->Data;
    }

    template <any_bucket_array Arr, typename U>
    auto *find_or_create(Arr * arr, const U &toMatch) {
        auto *result = find(arr, [&](Arr::T *element) { return *element == toMatch; });
        if (result) return result;

        result = malloc<Arr::T>({.Alloc = arr->Alloc});
        append(arr, *result);
        return result;
    }
}

LSTD_END_NAMESPACE
