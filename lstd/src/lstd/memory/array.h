#pragma once

#include "../internal/context.h"
#include "stack_array.h"
#include "string_utils.h"

LSTD_BEGIN_NAMESPACE

// Functions on this object allow negative reversed indexing which begins at
// the end of the string, so -1 is the last character -2 the one before that, etc. (Python-style)
//
// This type is entirely constexpr. Everything except methods that modify the contents of the string can be used compile-time.
// e.g. sub_view, searching, trimming etc.
template <typename T_>
struct array_view {
    using T = T_;

    // :CodeReusability: Automatically generates ==, !=, <, <=, >, >=, compare_*, find_*, has functions etc.. take a look at "array_like.h"
    static constexpr bool IS_ARRAY_LIKE = true;

    T *Data = null;
    s64 Count = 0;

    constexpr array_view() {}
    constexpr array_view(T *data, s64 count) : Data(data), Count(count) {}
    constexpr array_view(const initializer_list<T> &items) {
        static_assert(false, "This bug bit me hard... You cannot create arrays which are views into initializer lists.");
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
    // arrays that contain encoded utf8. It's more of an "intent" thing and being explicit with it is, I think, good.
    template <typename U = T>
    requires(types::is_same<types::remove_const_t<U>, utf8>) operator array_view<byte>() const { return array_view<byte>((byte *) Data, Count); }

    template <typename U = T>
    requires(types::is_same<types::remove_const_t<U>, byte>) operator array_view<utf8>() const { return array_view<utf8>((utf8 *) Data, Count); }
};

// We use array_view<byte> when, e.g. parsing, and it's kinda a long name..
using bytes = array_view<byte>;

// This object may represent a non-owning pointer to to a byte buffer or a pointer to an allocated memory block.
// In the latter case the memory must be released by the user with free().
// Copying it does a shallow copy (creates a view). In order to get a deep copy use clone().
//
// Functions in this object allow negative reversed indexing which begins at
// the end of the array, so -1 is the last element -2 the one before that, etc. (Python-style)
//
// _array_ works with types that can be copied byte by byte correctly, we don't handle C++'s messy copy constructors.
// Take a look at the type policy in "internal/common.h".
//
// The user manually manages the memory with free(), which frees the allocated block (if there was anny allocated) and resets the object.
// The defer macro helps a lot (e.g. defer(free(arr)) calls release at any scope exit).
// !! More info about how we handle views/shallow copies and ownership in the type policy in "internal/common.h"
template <typename T>
struct array : public array_view<T> {
    s64 Allocated = 0;
    array() {}
};

template <typename T, s64 N>
stack_array<T, N>::operator array_view<T>() const { return array_view<T>((T *) Data, Count); }

#define data_t array_like_data_t

// Makes sure the array has reserved enough space for at least _target_ new elements.
// Note that it may reserve way more than required.
// Final reserve amount is equal to the next power of two bigger than (_target_ + Count), starting at 8.
//
// Allocates a buffer (using the Context's allocator) if the array hasn't already allocated.
// If this object is just a view (Allocated == 0) i.e. this is the first time it is allocating, the old
// elements are copied (again, we do a simple memcpy, we don't handle copy constructors).
//
// You can also specify a specific alignment for the elements directly (instead of using the Context variable).
template <typename T>
void reserve(array<T> &arr, s64 target, u32 alignment = 0) {
    if (arr.Count + target < arr.Allocated) return;

    target = max<s64>(ceil_pow_of_2(target + arr.Count + 1), 8);

    if (arr.Allocated) {
        auto oldAlignment = ((allocation_header *) arr.Data - 1)->Alignment;
        if (alignment == 0) {
            alignment = oldAlignment;
        } else {
            assert(alignment == oldAlignment && "Reserving with an alignment but the object already has a buffer with a different alignment. Specify alignment 0 to automatically use the old one.");
        }

        arr.Data = reallocate_array(arr.Data, target);
    } else {
        auto *oldData = arr.Data;
        arr.Data = allocate_array_aligned(T, target, alignment);
        // We removed the ownership system.
        // encode_owner(Data, this);
        if (arr.Count) copy_memory(arr.Data, oldData, arr.Count * sizeof(T));
    }
    arr.Allocated = target;
}

// Call destructor on each element if the buffer is allocated, don't free the buffer, just move Count to 0
template <typename T>
void reset(array<T> &arr) {
    if (arr.Allocated) {
        while (arr.Count) {
            arr.Data[arr.Count - 1].~T();
            --arr.Count;
        }
    } else {
        arr.Count = 0;
    }
}

// Call destructor on each element if the buffer is allocated, free any memory and reset count
template <typename T>
void free(array<T> &arr) {
    reset(arr);
    if (arr.Allocated) free(arr.Data);
    arr.Data = null;
    arr.Allocated = 0;
}

// Checks if there is enough reserved space for _n_ elements
template <typename T>
constexpr bool has_space_for(const array<T> &arr, s64 n) { return (arr.Count + n) <= arr.Allocated; }

// Sets the _index_'th element in the array (also calls the destructor on the old one)
template <typename T>
void *set(array<T> &arr, s64 index, const data_t<array<T>> &element) {
    auto i = translate_index(index, arr.Count);
    arr.Data[i].~T();
    arr.Data[i] = element;
}

// Inserts an element at a specified index and returns a pointer to it in the buffer
template <typename T>
T *insert(array<T> &arr, s64 index, const data_t<array<T>> &element) {
    reserve(arr, 1);

    s64 offset = translate_index(index, arr.Count, true);
    auto *where = arr.Data + offset;
    if (offset < arr.Count) {
        copy_memory(where + 1, where, (arr.Count - offset) * sizeof(T));
    }
    copy_memory(where, &element, sizeof(T));
    ++arr.Count;
    return where;
}

#undef data_t

// Inserts an empty element at a specified index and returns a pointer to it in the buffer.
//
// Here is a way to clone insert an object (because the default insert just shallow-copies the object).
//
// T toBeCloned = ...;
// clone(arr.insert(index), toBeCloned);
//
// Because _insert_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
template <typename T>
T *insert(array<T> &arr, s64 index) { return insert(arr, index, T()); }

// Insert a buffer of elements at a specified index.
template <typename T>
T *insert_pointer_and_size(array<T> &arr, s64 index, const T *ptr, s64 size) {
    reserve(arr, size);

    s64 offset = translate_index(index, arr.Count, true);
    auto *where = arr.Data + offset;
    if (offset < arr.Count) {
        copy_memory(where + size, where, (arr.Count - offset) * sizeof(T));
    }
    copy_memory(where, ptr, size * sizeof(T));
    arr.Count += size;
    return where;
}

// Insert an array at a specified index and returns a pointer to the beginning of it in the buffer
template <typename T>
T *insert_array(array<T> &arr, s64 index, const array_view<T> &arr2) { return insert_pointer_and_size(arr, index, arr2.Data, arr2.Count); }

// Insert a list of elements at a specified index and returns a pointer to the beginning of it in the buffer
template <typename T>
T *insert_list(array<T> &arr, s64 index, const initializer_list<T> &list) { return insert_pointer_and_size(arr, index, list.begin(), list.size()); }

// Removes element at specified index and moves following elements back
template <typename T>
void remove(array<T> &arr, s64 index) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) reserve(arr, 0);

    s64 offset = translate_index(index, arr.Count);

    auto *where = arr.Data + offset;
    where->~T();
    copy_memory(where, where + 1, (arr.Count - offset - 1) * sizeof(T));
    --arr.Count;
}

// Removes element at specified index and moves the last element to the empty slot.
// This is faster than remove because it doesn't keep the order of the elements.
template <typename T>
void remove_unordered(array<T> &arr, s64 index) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) reserve(arr, 0);

    s64 offset = translate_index(index, arr.Count);

    auto *where = arr.Data + offset;
    where->~T();
    copy_memory(where, arr.Data + arr.Count - 1, sizeof(T));
    --arr.Count;
}

// Removes a range of elements and moves following elements back
// [begin, end)
template <typename T>
void remove_range(array<T> &arr, s64 begin, s64 end) {
    // If the array is a view, we don't want to modify the original!
    if (!arr.Allocated) reserve(arr, 0);

    s64 targetBegin = translate_index(begin, arr.Count);
    s64 targetEnd = translate_index(end, arr.Count, true);

    auto where = arr.Data + targetBegin;
    for (auto *destruct = where; destruct != (arr.Data + targetEnd); ++destruct) {
        destruct->~T();
    }

    s64 elementCount = targetEnd - targetBegin;
    copy_memory(where, where + elementCount, (arr.Count - targetBegin - elementCount) * sizeof(T));
    arr.Count -= elementCount;
}

// Appends an element to the end and returns a pointer to it in the buffer
template <typename T>
T *append(array<T> &arr, const T &element) { return insert(arr, arr.Count, element); }

// Inserts an empty element at the end of the array and returns a pointer to it in the buffer.
//
// This is useful for the following way to clone append an object (because the default append just shallow-copies the object).
//
// data_t toBeCloned = ...;
// clone(arr.append(), toBeCloned);
//
// Because _append_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
template <typename T>
T *append(array<T> &arr) { return append(arr, T()); }

// Appends a buffer of elements to the end and returns a pointer to it in the buffer
template <typename T>
T *append_pointer_and_size(array<T> &arr, const T *ptr, s64 size) { return insert_pointer_and_size(arr, arr.Count, ptr, size); }

// Appends an array to the end and returns a pointer to the beginning of it in the buffer
template <typename T>
T *append_array(array<T> &arr, const array_view<T> &arr2) { return insert_array(arr, arr.Count, arr2); }

// Appends an array to the end and returns a pointer to the beginning of it in the buffer.
// This is different from append_array in that if the array is empty/uninitialized then it doesn't allocate memory
// but just sets the fields to the Data and Count in _arr2_. This is useful for specific use cases
// (e.g. when reading bytes and we didn't succeed so we might need to read more and append them - when using reader, take a look at reader.cpp in tests,
// we don't want to allocate for the first set of bytes because that might be the whole set so there is no reason for concatenation).
template <typename T>
T *append_array_or_set_fields(array<T> &arr, const array_view<T> &arr2) {
    if (!arr.Data && !arr.Count) {
        arr.Data = arr2.Data;
        arr.Count = arr2.Count;
        return arr.Data;
    }
    return append_array(arr, arr2);
}

// Appends an array to the end and returns a pointer to the beginning of it in the buffer
template <typename T>
T *append_list(array<T> &arr, const initializer_list<T> &list) { return insert_list(arr, arr.Count, list); }

// Be careful not to call this with _dest_ pointing to _src_!
// Returns just _dest_.
template <typename T>
array<T> *clone(array<T> *dest, const array<T> &src) {
    dest->reset();
    dest->append_pointer_and_size(src.Data, src.Count);
    return dest;
}

LSTD_END_NAMESPACE
