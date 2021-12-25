module;

#include "../common.h"

export module lstd.array;

export import lstd.memory;
export import lstd.array_like;
export import lstd.string_utils;

LSTD_BEGIN_NAMESPACE

// @TODO: Make fully constexpr

//
// This object contains a typed pointer and a size. Used as a basic wrapper around arrays.
// :CodeReusability: This is considered array_like (take a look at array_like.h).
//
// Functions on this object allow negative reversed indexing which begins at
// the end of the string, so -1 is the last character -2 the one before that, etc. (Python-style)
//
// Note: We have a very fluid philosophy of containers and ownership. We don't implement
// copy constructors or destructors, which means that the programmer is totally in control
// of how the memory gets managed.
// See :TypePolicy in "common.h"
//
// This means that this type can be just an array wrapper (a view) or it can also be used as a dynamic array type.
// It's up to the programmer. It also may point to a buffer that came from a totally different place.
// If this type has allocated (by explictly calling reserve() or any modifying functions which call reserve())
// then the programmer must call free(arr.Data).
//
// free(arr.Data) will crash if the pointer is not a heap allocated block (which should contain an allocation header).
//
// This object being just two 8 bit integers can be cheaply and safely passed to functions without performance
// concerns. In order to get a deep copy use clone().
//
// Note: The defer macro helps with calling free (i.e. defer(free(arr.Data))
// releases an allocated array's data at scope exit.
//

export {
    template <typename T>
    struct array;

    // The [] operator in array returns a reference to the object
    // in the buffer so it can be modified without being ugly and returning a pointer.
    // :ExplainYourReferences:
    //
    // String is just array<char> we expect to be in UTF-8.
    // This function is overloaded in string.cppm to work with code points.
    template <typename T>
    constexpr T &get_operator_square_brackets(array<T> * arr, s64 index);

    template <typename T>
    struct array {
        T *Data   = null;
        s64 Count = 0;

        // s64 Allocated = 0; // We now check the allocation header of _Data_ to save space on this structure.

        constexpr array() {}
        constexpr array(T *data, s64 count) : Data(data), Count(count) {}

        // We allow converting from c-style strings (char* or char8_t*)
        constexpr array(any_c_string_one_byte auto data) : Data((char *) data), Count(c_string_length(data)) {
            static_assert(types::is_same<T, char>, "Converting c-style string to an array of type that isn't a string");
        }

        // Take data + size
        constexpr array(any_c_string_one_byte auto data, s64 n) : Data((char *) data), Count(n) {
            static_assert(types::is_same<T, char>, "Converting c-style string to an array of type that isn't a string");
        }

        constexpr array(const initializer_list<T> &items) {
            // A bug caused by this bit me hard...

            static_assert(false, "Don't create arrays which are views into initializer lists (they get optimized in Release).");
            static_assert(false, "Use dynamic arrays or store the values on the stack - e.g. make_stack_array(1, 4, 9...)");
        }

        constexpr auto operator[](s64 index) { return get_operator_square_brackets(this, index); }
        constexpr explicit operator bool() { return Count; }  // To check if empty
    };

    template <typename T>
    constexpr T &get_operator_square_brackets(array<T> * arr, s64 index) { return arr->Data[index]; }

    template <typename T>
    concept any_array = types::is_same_template_decayed<T, array<s32>>;

    // To make range based for loops work.
    auto begin(any_array auto &arr) { return arr.Data; }
    auto end(any_array auto &arr) { return arr.Data + arr.Count; }

    // Allocates a buffer (using the Context's allocator by default).
    // If _arr->Count_ is not zero we copy the elements _arr_ is pointing to.
    //
    // When adding elements to a dynamic array it grows automatically.
    // Coming up with a good value for the initial _n_ can improve performance.
    void make_dynamic(any_array auto *arr, s64 n, allocator alloc = {});

    // Returns how many elements fit in _arr->Data_, i.e. the size of the buffer.
    // This is not always == arr->Count. When the buffer fills we resize it to fit more.
    s64 get_allocated(any_array auto arr);

    // When DEBUG_MEMORY is defined we keep a global list of allocations,
    // so we can check if the array is dynamically allocated.
    //
    // This can catch bugs. Calling routines that
    // insert/remove/modify elements to a non-dynamic array is dangerous.
    bool is_dynamically_allocated(any_array auto arr);

    // Allocates a buffer (using the Context's allocator) and copies the old elements.
    // We try to call realloc which may actually save us an allocation.
    void resize(any_array auto *arr, s64 n);

    // Checks _arr_ if there is space for at least _fit_ new elements.
    // Resizes the array if there is not enough space. The new size is equal to the next
    // power of two bigger than (arr->Count + fit), minimum 8.
    void maybe_grow(any_array auto *arr, s64 fit);

    // Don't free the buffer, just move Count to 0
    void reset(any_array auto *arr) { arr->Count = 0; }

    // Checks if there is enough reserved space for _fit_ elements
    bool has_space_for(any_array auto arr, s64 fit) { return arr.Count + fit <= get_allocated(arr); }

    // Overrides the _index_'th element in the array
    void set(any_array auto *arr, s64 index, auto element);

    // Inserts an element at a specified index and returns a pointer to it in the buffer
    auto *insert_at_index(any_array auto *arr, s64 index, auto element);

    // Insert a buffer of elements at a specified index
    auto *insert_pointer_and_size_at_index(any_array auto *arr, s64 index, auto *ptr, s64 size);

    // Insert an array at a specified index and returns a pointer to the beginning of it in the buffer
    auto *insert_array_at_index(any_array auto *arr, s64 index, any_array auto arr2) { return insert_pointer_and_size_at_index(arr, index, arr2.Data, arr2.Count); }

    // Removes first found element and moves following elements back.
    // Returns true on success (false if _element_ was not found in the array).
    bool remove_ordered(any_array auto *arr, auto element);

    // Removes first found element and moves the last element to the empty slot.
    // This is faster than remove because it doesn't move everything back
    // but this doesn't keep the order of the elements.
    // Returns true on success (false if _element_ was not found in the array).
    bool remove_unordered(any_array auto *arr, auto element);

    // Removes element at specified index and moves following elements back
    void remove_ordered_at_index(any_array auto *arr, s64 index);

    // Removes element at specified index and moves the last element to the empty slot.
    // This is faster than remove because it doesn't move everything back
    // but this doesn't keep the order of the elements.
    void remove_unordered_at_index(any_array auto *arr, s64 index);

    // Removes a range [begin, end) and moves following elements back
    void remove_range(any_array auto *arr, s64 begin, s64 end);

    // Removes a range [begin, end) and inserts _replace_.
    // May allocate and change the count:
    // moves following elements forward/backward if (end - begin) != replace.Count.
    void replace_range(any_array auto *arr, s64 begin, s64 end, any_array auto replace);

    // Appends an element to the end and returns a pointer to it in the buffer
    auto *add(any_array auto *arr, auto element) { return insert_at_index(arr, arr->Count, element); }

    // Appends a buffer of elements to the end and returns a pointer to it in the buffer
    auto *add_pointer_and_size(any_array auto *arr, auto *ptr, s64 size) { return insert_pointer_and_size_at_index(arr, arr->Count, ptr, size); }

    // Appends an array to the end and returns a pointer to the beginning of it in the buffer
    auto *add_array(any_array auto *arr, any_array auto arr2) { return insert_array_at_index(arr, arr->Count, arr2); }

    // Replace all occurences of a subarray with another array.
    // @Speed Fine if search.Count == replace.Count, slow and dumb otherwise (but we could improve). See comments in implementation.
    void replace_all(any_array auto *arr, any_array auto search, any_array auto replace);

    // Removes all occurences of a subarray from an array.
    void remove_all(any_array auto *arr, any_array auto search) { replace_all(arr, search, {}); }

    constexpr bool operator==(any_array auto a, any_array auto b) { return compare(a, b) == -1; }

    // Returns a deep copy of _src_
    auto clone(any_array auto src, allocator alloc = {}) {
        decltype(src) result;
        make_dynamic(&result, src.Count, alloc);
        add_array(&result, src);
        return result;
    }
}

void make_dynamic(any_array auto *arr, s64 n, allocator alloc) {
    using T = types::remove_pointer_t<decltype(arr->Data)>;

    auto *oldData = arr->Data;

    // If alloc is null we use the Context's allocator
    arr->Data = malloc<T>({.Count = n, .Alloc = alloc});
    if (arr->Count) copy_memory(arr->Data, oldData, arr->Count * sizeof(T));
}

// @Cleanup Decide if we want to store an _Allocated_ in the array object itself.
// This would use more memory (the object becomes 24 bytes) but wouldn't rely on the allocation header?
// Right now can't disable encoding an allocation header when using the temporary allocator
// because we can't guarantee temporary dynamic arrays would work.
s64 get_allocated(any_array auto arr) {
    using T = types::remove_pointer_t<decltype(arr.Data)>;
    return ((allocation_header *) arr.Data - 1)->Size / sizeof(T);
}

bool is_dynamically_allocated(any_array auto *arr) {
#if defined DEBUG_MEMORY
    //
    // Attempting to modify an array view...
    // Data wasn't dynamically allocated.
    //
    // Make sure you call make_dynamic(arr) beforehand.
    //
    // Attempting to modify an array from another thread...
    // Caution! This container is not thread-safe!
    //
    assert(debug_memory_list_contains((allocation_header *) arr->Data - 1));
#endif
    return true;
}

void resize(any_array auto *arr, s64 n) {
    assert(n >= arr->Count && "New space not enough to fit the old elements");
    arr->Data = realloc(arr->Data, {.NewCount = n});
}

void maybe_grow(any_array auto *arr, s64 fit) {
    assert(is_dynamically_allocated(arr));

    s64 space = get_allocated(*arr);

    if (arr->Count + fit <= space) return;

    s64 target = max(ceil_pow_of_2(arr->Count + fit + 1), 8);
    resize(arr, target);
}

void set(any_array auto *arr, s64 index, auto element) {
    assert(is_dynamically_allocated(arr));

    auto i        = translate_index(index, arr->Count);
    *arr->Data[i] = element;
}

auto *insert_at_index(any_array auto *arr, s64 index, auto element) {
    maybe_grow(arr, 1);

    s64 offset  = translate_index(index, arr->Count, true);
    auto *where = arr->Data + offset;
    if (offset < arr->Count) {
        copy_memory(where + 1, where, (arr->Count - offset) * sizeof(*where));
    }
    *where = element;
    ++arr->Count;
    return where;
}

auto *insert_pointer_and_size_at_index(any_array auto *arr, s64 index, auto *ptr, s64 size) {
    maybe_grow(arr, size);

    s64 offset  = translate_index(index, arr->Count, true);
    auto *where = arr->Data + offset;
    if (offset < arr->Count) {
        copy_memory(where + size, where, (arr->Count - offset) * sizeof(*where));
    }
    copy_memory(where, ptr, size * sizeof(*where));
    arr->Count += size;
    return where;
}

bool remove_ordered(any_array auto *arr, auto element) {
    s64 index = find(arr, element);
    if (index == -1) return false;

    remove_ordered_at_index(arr, index);

    return true;
}

void remove_unordered(any_array auto *arr, auto element) {
    s64 index = find(arr, element);
    if (index == -1) return false;

    remove_unordered_at_index(arr, index);

    return true;
}

void remove_ordered_at_index(any_array auto *arr, s64 index) {
    assert(is_dynamically_allocated(arr));

    s64 offset = translate_index(index, arr->Count);

    auto *where = arr->Data + offset;
    copy_memory(where, where + 1, (arr->Count - offset - 1) * sizeof(*where));
    --arr->Count;
}

void remove_unordered_at_index(any_array auto *arr, s64 index) {
    assert(is_dynamically_allocated(arr));

    s64 offset = translate_index(index, arr->Count);

    auto *where = arr->Data + offset;

    // No need when removing the last element
    if (offset != arr->Count - 1) {
        *where = arr->Data + arr->Count - 1;
    }
    --arr->Count;
}

void remove_range(any_array auto *arr, s64 begin, s64 end) {
    assert(is_dynamically_allocated(arr));

    s64 targetBegin = translate_index(begin, arr->Count);
    s64 targetEnd   = translate_index(end, arr->Count, true);

    auto where    = arr->Data + targetBegin;
    auto whereEnd = arr->Data + targetEnd;

    s64 elementCount = whereEnd - where;
    copy_memory(where, whereEnd, (arr->Count - targetBegin - elementCount) * sizeof(*where));
    arr->Count -= elementCount;
}

void replace_range(any_array auto *arr, s64 begin, s64 end, any_array auto replace) {
    assert(is_dynamically_allocated(arr));

    s64 targetBegin = translate_index(begin, arr->Count);
    s64 targetEnd   = translate_index(end, arr->Count, true);

    s64 whereSize = targetEnd - targetBegin;

    s64 diff = replace.Count - whereSize;

    if (diff > 0) {
        maybe_grow(arr, diff);
    }

    auto where = arr->Data + targetBegin;

    // Make space for the new elements
    copy_memory(where + replace.Count, where + whereSize, (arr->Count - targetBegin - whereSize) * sizeof(*where));

    // Copy replace elements
    copy_memory(where, replace.Data, replace.Count * sizeof(*where));

    arr->Count += diff;
}

void replace_all(any_array auto *arr, any_array auto search, any_array auto replace) {
    assert(is_dynamically_allocated(arr));

    if (!arr->Data || !arr->Count) return;

    assert(search.Data && search.Count);
    if (replace.Count) assert(replace.Data);

    if (search.Count == replace.Count) {
        // This case we can handle relatively fast.
        // @Speed Improve by using bit hacks for the case when the elements are less than a pointer size?
        auto *p = arr->Data;
        auto *e = arr->Data + arr->Count;
        while (p != e) {
            // @Speed We can do simply memcmp for scalar types and types that don't have overloaded ==.
            if (*p == search[0]) {
                auto *n  = p;
                auto *sp = search.Data;
                auto *se = search.Data + search.Count;
                while (n != e && sp != se) {
                    // Require only operator == to be defined (and not !=).
                    if (!(*n == *sp)) break;
                    ++n, ++sp;
                }

                if (sp == se) {
                    // Match found
                    copy_memory(p, replace.Data, replace.Count * sizeof(*p));
                    p += replace.Count;
                } else {
                    ++p;
                }
            } else {
                ++p;
            }
        }
    } else {
        //
        // @Speed This is the slow and dumb version for now.
        // We can improve performance by either:
        // * Allocating a buffer first which holds the result (space cost increases)
        // * Doing two passes, first one counting the number of occurences
        //   so we know the offsets for the second pass.
        // Though the second option would only work if search.Count > replace.Count.
        //
        // I think going with the former makes the most sense,
        // however at that point letting the caller write their own routine
        // will probably be better, since we can't for sure know the context
        // and if allocating another (possibly big) array is fine.
        //
        s64 diff = replace.Count - search.Count;

        s64 i = 0;
        while (i < arr->Count && (i = find(*arr, search, i)) != -1) {
            replace_range(arr, i, i + search.Count, replace);  // @Speed Slow and dumb version for now
            i += replace.Count;
        }
    }
}

LSTD_END_NAMESPACE
