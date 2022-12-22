module;

#include "../common.h"

export module lstd.qsort;

export import lstd.delegate;

LSTD_BEGIN_NAMESPACE

/*******************************************************************************
*
*  Author:  Remi Dufour - remi.dufour@gmail.com
*  Date:    July 23rd, 2012
*
*  Name:        Quicksort
*
*  Description: This is a well-known sorting algorithm developed by C. A. R. 
*               Hoare. It is a comparison sort and in this implementation,
*               is not a stable sort.
*
*  Note:        This is public-domain C implementation written from
*               scratch.  Use it at your own risk.
*
*******************************************************************************/

export {
    template <typename T>
    using quick_sort_comparison_func = delegate<s32(const T *, const T *)>;

    //
    // This function performs a basic Quicksort. This implementation is the
    // in-place version of the algorithm and is done in he following way:
    //
    // 1. In the middle of the array, we determine a pivot that we temporarily swap to the end.
    // 2. From the beginning to the end of the array, we swap any elements smaller
    //    than this pivot to the start, adjacent to other elements that were
    //    already moved.
    // 3. We swap the pivot next to these smaller elements.
    // 4. For both sub-arrays on sides of the pivot, we repeat this process recursively.
    // 5. For a sub-array smaller than a certain threshold, the insertion sort algorithm takes over.
    //
    // As an optimization, rather than performing a real recursion, we keep a
    // global stack to track boundaries for each recursion level.
    //
    // To ensure that at most O(log2 N) space is used, we recurse into the smaller
    // partition first. The log2 of the highest unsigned value of an integer type
    // is the number of bits needed to store that integer.
    //
    void quick_sort(void *array, s64 length, s64 size, quick_sort_comparison_func<void> compare);

    template <typename T>
    s32 default_comparison(const T *lhs, const T *rhs) {
        return *lhs <=> *rhs;
    }

    template <typename T>
    void quick_sort(T *first, s64 count, quick_sort_comparison_func<T> compare = default_comparison<T>) {
        auto compareVoid = [&](const void *lhs, const void *rhs) { return compare((const T *) lhs, (const T *) rhs); };
        auto wrapper     = quick_sort_comparison_func<void>(&compareVoid);
        quick_sort(first, count, sizeof(T), wrapper);
    }

    template <typename T>
    void quick_sort(T *first, T *last, quick_sort_comparison_func<T> compare = default_comparison<T>) {
        quick_sort(first, last - first + 1, compare);
    }
}

// Swaps the elements of two arrays.
//
// The length of the swap is determined by the value of "SIZE".  While both
// arrays can't overlap, the case in which both pointers are the same works.
#define SWAP(A, B, SIZE)                      \
    {                                         \
        char *a_byte      = A;                \
        char *b_byte      = B;                \
        const char *a_end = a_byte + SIZE;    \
                                              \
        while (a_byte < a_end) {              \
            const char swap_byte = *b_byte;   \
            *b_byte++            = *a_byte;   \
            *a_byte++            = swap_byte; \
        }                                     \
    }

// Swaps the elements of an array with its next value.
//
// The length of the swap is determined by the value of "SIZE".  This macro
// must be used at the beginning of a scope and "A" shouldn't be an expression.
#define SWAP_NEXT(A, SIZE)                       \
    char *a_byte      = A;                       \
    const char *a_end = A + SIZE;                \
                                                 \
    while (a_byte < a_end) {                     \
        const char swap_byte = *(a_byte + SIZE); \
        *(a_byte + SIZE)     = *a_byte;          \
        *a_byte++            = swap_byte;        \
    }

void quick_sort(void *array, s64 length, s64 size, quick_sort_comparison_func<void> compare) {
    // Recursive stacks for array boundaries (both inclusive)
    struct stackframe {
        void *Left;
        void *Right;
    } stack[sizeof(void *) * 8];

    stackframe *recursion = stack;
    s64 threshold         = size << 2;

    // Assign the first recursion level of the sorting
    recursion->Left  = array;
    recursion->Right = (char *) array + size * (length - 1);

    do {
        // Partition the array
        char *index = (char *) recursion->Left;
        char *right = (char *) recursion->Right;
        char *left  = index;

        // Assigning store to the left
        char *store = index;

        --recursion;

        // Determine a pivot (in the middle) and move it to the end
        s64 middle = (right - left) >> 1;
        SWAP(left + middle - middle % size, right, size)

        while (index < right) {
            if (compare(right, index) > 0) {
                SWAP(index, store, size)
                store += size;
            }
            index += size;
        }

        // Move the pivot to its final place
        SWAP(right, store, size)

#define RECURSE_LEFT                        \
    if (left < store - size) {              \
        (++recursion)->Left = left;         \
        recursion->Right    = store - size; \
    }
#define RECURSE_RIGHT                       \
    if (store + size < right) {             \
        (++recursion)->Left = store + size; \
        recursion->Right    = right;        \
    }

#define INSERTION_SORT_LOOP(LEFT)                                   \
    {                                                               \
        char *trail = index - size;                                 \
        while (trail >= LEFT && compare(trail, trail + size) > 0) { \
            SWAP_NEXT(trail, size)                                  \
            trail -= size;                                          \
        }                                                           \
    }

#define INSERTION_SORT_LEFT                                 \
    for (index = left + size; index < store; index += size) \
    INSERTION_SORT_LOOP(left)

#define INSERTION_SORT_RIGHT                                         \
    for (index = store + (size << 1); index <= right; index += size) \
    INSERTION_SORT_LOOP(store + size)

#define SORT_LEFT                    \
    if (store - left <= threshold) { \
        INSERTION_SORT_LEFT          \
    } else {                         \
        RECURSE_LEFT                 \
    }

#define SORT_RIGHT                    \
    if (right - store <= threshold) { \
        INSERTION_SORT_RIGHT          \
    } else {                          \
        RECURSE_RIGHT                 \
    }

        // Recurse into the smaller partition first
        if (store - left < right - store) {
            // Left side is smaller
            SORT_RIGHT
            SORT_LEFT
        } else {
            // Right side is smaller
            SORT_LEFT
            SORT_RIGHT
        }
    } while (recursion >= stack);
}

LSTD_END_NAMESPACE
