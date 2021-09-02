#pragma once

#include "../common/context.h"
#include "stack_array.h"

LSTD_BEGIN_NAMESPACE

//
// This object contains a typed pointer and a size. Used as a basic wrapper around arrays.
// :CodeReusability: This is considered array_like (take a look at array_like.h).
//
// Functions on this object allow negative reversed indexing which begins at
// the end of the string, so -1 is the last character -2 the one before that, etc. (Python-style)
//
// It also contains an integer "Allocated" that says how many bytes has this object allocated.
// Note: We have a very fluid philosophy of containers and ownership. We don't implement
// copy constructors or destructors, which means that the programmer is totally in control
// of how the memory gets managed.
//
// This means that this type can be just an array wrapper (a view) or it can also be used as a dynamic array type.
// It dependes on the use case if this should be interpretted just view (or a subview) or an object owns a block.
//
// This object may represent a non-owning pointer to a buffer (that may have come from another allocated array which is still alive)
// or a "owning" pointer to an allocated block. In the latter case the memory must be released by the user with free().
//
// The user manually manages the memory with free(), which frees the allocated block (if Allocated != 0).
// You can also call free on a copy, which invalidates the original/all other copies.
// In order to get a deep copy use clone().
// !! See type policy in "internal/common.h"
//
// Note: The defer macro helps with calling free (i.e. defer(free(arr)) releases the array at any scope exit).
//
template <typename T_>
struct array {
    using T = T_;

    T *Data       = null;
    s64 Count     = 0;
    s64 Allocated = 0;

    constexpr array() {
    }

    constexpr array(T *data, s64 count)
        : Data(data),
          Count(count) {
    }

    constexpr array(const initializer_list<T> &items) {
        static_assert(false, "This bug bit me hard... Don't create arrays which are views into initializer lists (they get optimized in Release).");
        static_assert(false, "You may want to reserve a dynamic array with those values. In that case make an empty array and use append_list().");
        static_assert(false, "Or you can store them in an array on the stack - e.g. use to_stack_array(1, 2, 3...)");
    }

    //
    // Iterators:
    //
    using iterator = T *;
    using const_iterator = const T *;

    constexpr iterator begin() { return Data; }
    constexpr iterator end() { return Data + Count; }
    constexpr const_iterator begin() const { return Data; }
    constexpr const_iterator end() const { return Data + Count; }

    //
    // Operators:
    //
    constexpr T &operator[](s64 index) { return Data[translate_index(index, Count)]; }
    constexpr const T &operator[](s64 index) const { return Data[translate_index(index, Count)]; }

    explicit operator bool() const { return Count; }

    // Implicit conversion operators between utf8 (a utf8 byte) and byte (which is also a byte but has different semantics).
    // They are identical (both are unsigned bytes) but we use them to differentiate when we are working with just arrays of bytes or
    // arrays that contain encoded utf8. It's more of an "intent of use" kind of thing and being explicit with it is, I think, good.
    template <typename U = T>
        requires(types::is_same<types::remove_const_t<U>, utf8>)
    operator array<byte>() const { return array((byte *) Data, Count); }

    template <typename U = T>
        requires(types::is_same<types::remove_const_t<U>, byte>)
    operator array<utf8>() const { return array((utf8 *) Data, Count); }
};

// Short-hand name for just saying "a bunch of bytes" and it doesn't matter where they came from.
// Array views can point to literally anything in memory (in contrast to strings, where we assume they point to a valid utf8 encoded string).
using bytes = array<byte>;

// Operator to convert a stack array into an array view.
template <typename T, s64 N>
stack_array<T, N>::operator array<T>() const { return array<T>((T *) Data, Count); }

// We need this because we can't check templated types with types::is_same...
template <typename T>
struct is_array_helper : types::false_t {
};

template <typename T>
struct is_array_helper<array<T>> : types::true_t {
};

template <typename T>
concept is_array = is_array_helper<T>::value;

//
// The following two functions are useful for abbreviated function templates, because we don't have to declare
// the underlying type of the array, which helps with brevity.
//

// Wrapper for copy_memory with typed pointers.
template <typename T>
T *copy_elements(T *dst, const T *src, s64 n) { return (T *) copy_memory(dst, src, n * sizeof(T)); }

// Calls destructor for an element.
template <typename T>
void destroy_at(T *at) {
    if constexpr (!types::is_scalar<T>) {
        at->~T();
    }
}

// Makes sure the array has reserved enough space for at least _n_ new elements.
// This may reserve way more than required. The reserve amount is equal to the next power of two bigger than (_n_ + Count), starting at 8.
//
// Allocates a buffer (using the Context's allocator) if the array hasn't already allocated.
// If this object is just a view (Allocated == 0) i.e. this is the first time it is allocating, the old
// elements are copied (again, we do a simple bytes copy, we don't handle copy constructors).
void array_reserve(is_array auto &arr, s64 n) {
    if (arr.Count + n < arr.Allocated) return;

    s64 target = max(ceil_pow_of_2(n + arr.Count + 1), 8);

    if (arr.Allocated) {
        arr.Data = reallocate_array(arr.Data, target);
    } else {
        auto *oldView = arr.Data;
        arr.Data      = allocate_array<types::remove_pointer_t<decltype(arr.Data)>>(target);
        if (arr.Count) copy_elements(arr.Data, oldView, arr.Count);
    }
    arr.Allocated = target;
}

// Call destructor on each element if the buffer is allocated.
// Don't free the buffer, just move Count to 0
void array_reset(is_array auto &arr) {
    if (arr.Allocated) {
        while (arr.Count) {
            destroy_at(arr.Data + arr.Count - 1);
            --arr.Count;
        }
    }
    arr.Count = 0;
}

// Call destructor on each element if the buffer is allocated.
// Free any memory and reset Count.
void free(is_array auto &arr) {
    array_reset(arr);
    if (arr.Allocated) free(arr.Data);
    arr.Data      = null;
    arr.Allocated = 0;
}

// Checks if there is enough reserved space for _n_ elements
constexpr bool array_has_space_for(const is_array auto &arr, s64 n) { return arr.Count + n <= arr.Allocated; }

// Sets the _index_'th element in the array (also calls the destructor on the old one)
template <is_array T>
void array_set(T &arr, s64 index, const array_data_t<T> &element) {
    auto i = translate_index(index, arr.Count);
    destroy_at(arr.Data + i);

    arr.Data[i] = element;
}

// Inserts an element at a specified index and returns a pointer to it in the buffer
template <is_array T>
auto *array_insert_at(T &arr, s64 index, const array_data_t<T> &element) {
    array_reserve(arr, 1);

    s64 offset  = translate_index(index, arr.Count, true);
    auto *where = arr.Data + offset;
    if (offset < arr.Count) {
        copy_elements(where + 1, where, arr.Count - offset);
    }
    copy_elements(where, &element, 1);
    ++arr.Count;
    return where;
}

// Inserts an empty element at a specified index and returns a pointer to it in the buffer.
//
// Here is a way to clone insert an object (because the default insert just shallow-copies the object).
//
// T toBeCloned = ...;
// clone(arr.insert(index), toBeCloned);
//
// Because _insert_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
auto *array_insert_at(is_array auto &arr, s64 index) { return insert(arr, index, {}); }

// Insert a buffer of elements at a specified index.
template <is_array T>
auto *array_insert_at(T &arr, s64 index, const array_data_t<T> *ptr, s64 size) {
    array_reserve(arr, size);

    s64 offset  = translate_index(index, arr.Count, true);
    auto *where = arr.Data + offset;
    if (offset < arr.Count) {
        copy_elements(where + size, where, arr.Count - offset);
    }
    copy_elements(where, ptr, size);
    arr.Count += size;
    return where;
}

// Insert an array at a specified index and returns a pointer to the beginning of it in the buffer
template <typename T>
auto *array_insert_at(T &arr, s64 index, T &arr2) { return array_insert_at(arr, index, arr2.Data, arr2.Count); }

// Removes element at specified index and moves following elements back
template <typename T>
void array_remove_at(array<T> &arr, s64 index) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) array_reserve(arr, 0);

    s64 offset = translate_index(index, arr.Count);

    auto *where = arr.Data + offset;
    destroy_at(where);
    copy_elements(where, where + 1, arr.Count - offset - 1);
    --arr.Count;
}

// Removes the first matching element and moves following elements back
template <is_array T>
void array_remove(T &arr, const array_data_t<T> &element) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) array_reserve(arr, 0);

    s64 index = -1;

    auto *p = arr.Data, *end = arr.Data + arr.Count;
    while (p != end) {
        if (*p == element) {
            index = p - arr.Data;
            break;
        }
        ++p;
    }

    assert(index != -1 && "Element not in list");

    string_remove_at(arr, index);
}

// Removes element at specified index and moves the last element to the empty slot.
// This is faster than remove because it doesn't move everything back (doesn't keep the order of the elements).
void array_remove_unordered(is_array auto &arr, s64 index) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) array_reserve(arr, 0);

    s64 offset = translate_index(index, arr.Count);

    auto *where = arr.Data + offset;
    destroy_at(where);
    copy_elements(where, arr.Data + arr.Count - 1, 1);
    --arr.Count;
}

// Removes a range of elements and moves following elements back
// [begin, end)
void array_remove_range(is_array auto &arr, s64 begin, s64 end) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) array_reserve(arr, 0);

    s64 targetBegin = translate_index(begin, arr.Count);
    s64 targetEnd   = translate_index(end, arr.Count, true);

    auto where    = arr.Data + targetBegin;
    auto whereEnd = arr.Data + targetEnd;

    for (auto *d = where; d != whereEnd; ++d) destroy_at(d);

    s64 elementCount = whereEnd - where;
    copy_elements(where, whereEnd, arr.Count - targetBegin - elementCount);
    arr.Count -= elementCount;
}

// Appends an element to the end and returns a pointer to it in the buffer
template <is_array T>
auto *array_append(T &arr, const array_data_t<T> &element) { return array_insert_at(arr, arr.Count, element); }

// Inserts an empty element at the end of the array and returns a pointer to it in the buffer.
//
// This is useful for the following way to clone append an object (because the default append just shallow-copies the object).
//
// data_t toBeCloned = ...;
// clone(arr.append(), toBeCloned);
//
// Because _append_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
auto *array_append(is_array auto &arr) { return array_append(arr, {}); }

// Appends a buffer of elements to the end and returns a pointer to it in the buffer
template <is_array T>
auto *array_append(T &arr, const array_data_t<T> *ptr, s64 size) { return array_insert_at(arr, arr.Count, ptr, size); }

// Appends an array to the end and returns a pointer to the beginning of it in the buffer
template <is_array T>
auto *array_append(T &arr, T &arr2) { return array_insert_at(arr, arr.Count, arr2); }

// Replace all occurences of a subarray from an array with another array
template <is_array T>
void array_replace_all(T &arr, const T &arr2, const T &arr3) {
    if (!arr.Data || arr.Count == 0) return;
    if (!arr2.Data || arr2.Count == 0) return;

    if (arr3.Count)
        assert(arr3.Data);

    s64 diff = arr3.Count - arr2.Count;

    s64 i = 0;
    while (i < arr.Count && (i = find(arr, arr2, i)) != -1) {
        // We need to reserve in any case (even if the diff is 0 or negative),
        // because we need to make sure we can modify the array (it's not a view).
        array_reserve(arr, abs(diff));

        auto *data = arr.Data + i;

        // Call the destructor on the old elements
        For(range(arr2.Count)) {
            destroy_at(data + it);
        }

        // Make space for the new elements
        if (diff) {
            copy_elements(data + arr3.Count, data + arr2.Count, arr.Count - i - arr2.Count);
        }

        // Copy replacement elements
        copy_elements(data, arr3.Data, arr3.Count);

        arr.Count += diff;

        i += arr3.Count;
    }
}

// Replace all occurences of an element from an array with another element.
// Wrapper.
template <is_array T>
void array_replace_all(T &arr, const array_data_t<T> &target, const array_data_t<T> &replace) {
    auto targetArr  = to_stack_array(target);
    auto replaceArr = to_stack_array(replace);
    array_replace_all(arr, targetArr, replaceArr);
}

// Replace all occurences of an element from an array with an array.
// Wrapper.
template <is_array T>
void array_replace_all(T &arr, const array_data_t<T> &target, const T &replace) {
    auto targetArr = to_stack_array(target);
    array_replace_all(arr, targetArr, replace);
}

// Replace all occurences of a subarray from an array with an element.
// Wrapper.
template <is_array T>
void array_replace_all(T &arr, const T &target, const array_data_t<T> &replace) {
    auto replaceArr = to_stack_array(replace);
    array_replace_all(arr, target, replaceArr);
}

// Removes all occurences of a subarray from an array
// Wrapper.
template <is_array T>
void array_remove_all(T &arr, const T &target) {
    array_replace_all(arr, target, {}); // Replace with an empty array
}

// Removes all occurences of an element from an array.
// Wrapper.
template <is_array T>
void array_remove_all(T &arr, const array_data_t<T> &element) {
    auto tarr = to_stack_array(element);
    array_remove_all(arr, tarr);
}

// Be careful not to call this with _dest_ pointing to _src_!
// Returns just _dest_.
template <typename T>
array<T> *clone(array<T> *dest, const array<T> &src) {
    array_reset(*dest);
    array_append(*dest, src.Data, src.Count);
    return dest;
}

LSTD_END_NAMESPACE
