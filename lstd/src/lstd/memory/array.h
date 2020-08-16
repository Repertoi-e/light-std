#pragma once

#include "../internal/context.h"
#include "stack_array.h"

LSTD_BEGIN_NAMESPACE

// _array_ works with types that can be copied byte by byte correctly, we don't handle C++'s messy copy constructors.
// Take a look at the type policy in "internal/common.h".
//
// We also don't free this with a destructor anymore (we used to in a past review of our design).
// The user manually manages the memory with release(), which frees the allocated block (if the object has allocated) and resets the object.
// The defer macro helps a lot (e.g. defer(arr.release()) calls release at any scope exit).
//
// Python-style negative indexing is supported (just like string). -1 means the last element (at index Count - 1).
template <typename T>
struct array {
    using data_t = T;

    data_t *Data = null;
    s64 Count = 0;
    s64 Reserved = 0;

    constexpr array() = default;
    constexpr array(data_t *data, s64 count) : Data(data), Count(count), Reserved(0) {}
    constexpr array(initializer_list<data_t> items) : Data((data_t *) items.begin()), Count(items.size()), Reserved(0) {}

    // We no longer use destructors for deallocation. Call release() explicitly (take a look at the defer macro!).
    // ~array() { release(); }

    // Makes sure the array has reserved enough space for at least _target_ new elements.
    // Note that it may reserve way more than required.
    // Final reserve amount is equal to the next power of two bigger than (_target_ + Count), starting at 8.
    //
    // Allocates a buffer (using the Context's allocator) if the array hasn't already allocated.
    // If this object is just a view (Reserved == 0) i.e. this is the first time it is allocating, the old
    // elements are copied (again, we do a simple memcpy, we don't handle copy constructors).
    //
    // You can also specify a specific alignment for the elements directly (instead of using the Context variable).
    void reserve(s64 target, u32 alignment = 0) {
        if (Count + target < Reserved) return;

        target = max<s64>(ceil_pow_of_2(target + Count + 1), 8);

        if (Reserved) {
            auto oldAlignment = ((allocation_header *) Data - 1)->Alignment;
            if (alignment == 0) {
                alignment = oldAlignment;
            } else {
                assert(alignment == oldAlignment && "Reserving with an alignment but the object already has a buffer with a different alignment. Specify alignment 0 to automatically use the old one.");
            }

            Data = reallocate_array(Data, target);
        } else {
            auto *oldData = Data;
            Data = allocate_array_aligned(data_t, target, alignment);
            // We removed the ownership system.
            // encode_owner(Data, this);
            if (Count) copy_memory(Data, oldData, Count * sizeof(data_t));
        }
        Reserved = target;
    }

    // Call destructor on each element if the buffer is allocated, free any memory and reset count
    void release() {
        reset();
        if (Reserved) free(Data);
        Data = null;
        Count = Reserved = 0;
    }

    // Call destructor on each element if the buffer is allocated, don't free the buffer, just move Count to 0
    void reset() {
        if (Reserved) {
            while (Count) {
                Data[Count - 1].~data_t();
                --Count;
            }
        } else {
            Count = 0;
        }
    }

    // We don't have bounds checking (for speed)
    data_t &get(s64 index) { return Data[translate_index(index, Count)]; }
    constexpr const data_t &get(s64 index) const { return Data[translate_index(index, Count)]; }

    // Calls our quick_sort() on the elements.
    void sort() { quick_sort(Data, Data + Count); }

    // Sets the _index_'th element in the array (also calls the destructor on the old one)
    array *set(s64 index, const data_t &element) {
        auto i = translate_index(index, Count);
        Data[i].~data_t();
        Data[i] = element;
        return this;
    }

    // Inserts an empty element at a specified index and returns a pointer to it in the buffer.
    //
    // This is useful for the following way to clone insert an object (because the default insert just shallow-copies the object).
    //
    // data_t toBeCloned = ...;
    // clone(arr.insert(index), toBeCloned);
    //
    // Because _insert_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
    data_t *insert(s64 index) { return insert(index, data_t()); }

    // Inserts an element at a specified index and returns a pointer to it in the buffer
    data_t *insert(s64 index, const data_t &element) {
        reserve(1);

        s64 offset = translate_index(index, Count, true);
        auto *where = begin() + offset;
        if (offset < Count) {
            copy_memory(where + 1, where, (Count - offset) * sizeof(data_t));
        }
        copy_memory(where, &element, sizeof(data_t));
        ++Count;
        return where;
    }

    // Insert an array at a specified index and returns a pointer to the beginning of it in the buffer
    data_t *insert(s64 index, const array &arr) { return insert_pointer_and_size(index, arr.Data, arr.Count); }

    // Insert a buffer of elements at a specified index.
    data_t *insert_pointer_and_size(s64 index, const data_t *ptr, s64 size) {
        reserve(size);
        
        s64 offset = translate_index(index, Count, true);
        auto *where = begin() + offset;
        if (offset < Count) {
            copy_memory(where + size, where, (Count - offset) * sizeof(data_t));
        }
        copy_memory(where, ptr, size * sizeof(data_t));
        Count += size;
        return where;
    }

    // Removes element at specified index and moves following elements back
    array *remove(s64 index) {
        // If the array is a view, we don't want to modify the original!
        if (!Reserved) reserve(0);

        s64 offset = translate_index(index, Count);

        auto *where = begin() + offset;
        where->~data_t();
        copy_memory(where, where + 1, (Count - offset - 1) * sizeof(data_t));
        --Count;
        return this;
    }

    // Removes a range of elements and moves following elements back
    // [begin, end)
    array *remove(s64 begin, s64 end) {
        // If the array is a view, we don't want to modify the original!
        if (!Reserved) reserve(0);

        s64 targetBegin = translate_index(index, Count);
        s64 targetEnd = translate_index(index, Count, true);

        auto where = begin() + targetBegin;
        for (auto *destruct = where; destruct != (begin() + targetEnd); ++destruct) {
            destruct->~data_t();
        }

        s64 elementCount = targetEnd - targetBegin;
        copy_memory(where, where + elementCount, (Count - offset - elementCount) * sizeof(data_t));
        Count -= elementCount;
        return this;
    }

    // Inserts an empty element at the end of the array and returns a pointer to it in the buffer.
    //
    // This is useful for the following way to clone append an object (because the default append just shallow-copies the object).
    //
    // data_t toBeCloned = ...;
    // clone(arr.append(), toBeCloned);
    //
    // Because _append_ returns a pointer where the object is placed, clone() can place the deep copy there directly.
    data_t *append() { return append(data_t()); }

    // Appends an element to the end and returns a pointer to it in the buffer
    data_t *append(const data_t &element) { return insert(Count, element); }

    // Appends an array to the end and returns a pointer to it in the buffer
    data_t *append(const array &arr) { return insert(Count, arr); }

    // Appends a buffer of elements to the end and returns a pointer to it in the buffer
    data_t *append_pointer_and_size(const data_t *ptr, s64 size) { return insert_pointer_and_size(Count, ptr, size); }

    // Compares this array to _arr_ and returns the index of the first element that is different.
    // If the arrays are equal, the returned value is -1.
    template <typename U>
    constexpr s64 compare(const array<U> &arr) const {
        static_assert(is_equal_comparable_v<T, U>, "Arrays have types which cannot be compared with operator ==");

        if (Data == arr.Data && Count == arr.Count) return -1;

        if (!Count && !arr.Count) return -1;
        if (!Count || !arr.Count) return 0;

        const T *s1 = begin();
        const U *s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return -1;
            if (s1 == end()) return s1 - begin();
            if (s2 == arr.end()) return s2 - arr.begin();
        }
        return s1 - begin();
    }

    // Compares this array to to _arr_ lexicographically.
    // The result is less than 0 if this array sorts before the other, 0 if they are equal, and greater than 0 otherwise.
    template <typename U>
    constexpr s32 compare_lexicographically(const array<U> &arr) const {
        static_assert(is_equal_comparable_v<T, U>, "Arrays have types which cannot be compared with operator ==");
        static_assert(is_less_comparable_v<T, U>, "Arrays have types which cannot be compared with operator <");

        if (Data == arr.Data && Count == arr.Count) return 0;

        if (!Count && !arr.Count) return -1;
        if (!Count) return -1;
        if (!arr.Count) return 1;

        const T *s1 = begin();
        const U *s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return 0;
            if (s1 == end()) return -1;
            if (s2 == arr.end()) return 1;
        }
        return s1 < s2 ? -1 : 1;
    }

    // Find the first occurence of an element which matches the predicate and is after a specified index.
    // Predicate must take a single argument (the current element) and return if it matches.
    constexpr s64 find(const delegate<bool(const data_t &)> &predicate, s64 start = 0) const {
        if (!Data || Count == 0) return -1;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (predicate(*p++)) return it;
        return -1;
    }

    // Find the first occurence of an element that is after a specified index
    constexpr s64 find(const T &element, s64 start = 0) const {
        if (!Data || Count == 0) return -1;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (*p++ == element) return it;
        return -1;
    }

    // Find the first occurence of a subarray that is after a specified index
    constexpr s64 find(const array &arr, s64 start = 0) const {
        if (!Data || Count == 0) return -1;
        assert(arr.Data);
        assert(arr.Count);

        start = translate_index(start, Count);

        For(range(start, Count)) {
            auto progress = arr.begin();
            for (auto search = begin() + it; progress != arr.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == arr.end()) return it;
        }
        return -1;
    }

    // Find the last occurence of an element that is before a specified index
    constexpr s64 find_reverse(const T &element, s64 start = 0) const {
        if (!Data || Count == 0) return -1;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (*p-- == element) return it;
        return -1;
    }

    // Find the last occurence of a subarray that is before a specified index
    constexpr s64 find_reverse(const array &arr, s64 start = 0) const {
        if (!Data || Count == 0) return -1;
        assert(arr.Data);
        assert(arr.Count);

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        For(range(start - arr.Count + 1, -1, -1)) {
            auto progress = arr.begin();
            for (auto search = begin() + it; progress != arr.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == arr.end()) return it;
        }
        return -1;
    }

    // Find the first occurence of any element in the specified subarray that is after a specified index
    constexpr s64 find_any_of(const array &allowed, s64 start = 0) const {
        if (!Data || Count == 0) return -1;
        assert(allowed.Data);
        assert(allowed.Count);

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (allowed.has(*p++)) return it;
        return -1;
    }

    // Find the last occurence of any element in the specified subarray
    // that is before a specified index (0 means: start from the end)
    constexpr s64 find_reverse_any_of(const array &allowed, s64 start = 0) const {
        if (!Data || Count == 0) return -1;
        assert(allowed.Data);
        assert(allowed.Count);

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (allowed.has(*p--)) return it;
        return -1;
    }

    // Find the first absence of an element that is after a specified index
    constexpr s64 find_not(const data_t &element, s64 start = 0) const {
        if (!Data || Count == 0) return -1;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (*p++ != element) return it;
        return -1;
    }

    // Find the last absence of an element that is before the specified index
    constexpr s64 find_reverse_not(const data_t &element, s64 start = 0) const {
        if (!Data || Count == 0) return -1;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (*p-- != element) return it;
        return -1;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    constexpr s64 find_not_any_of(const array &banned, s64 start = 0) const {
        if (!Data || Count == 0) return -1;
        assert(banned.Data);
        assert(banned.Count);

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (!banned.has(*p++)) return it;
        return -1;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    constexpr s64 find_reverse_not_any_of(const array &banned, s64 start = 0) const {
        if (!Data || Count == 0) return -1;
        assert(banned.Data);
        assert(banned.Count);

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (!banned.has(*p--)) return it;
        return -1;
    }

    // Checks if _item_ is contained in the array
    constexpr bool has(const data_t &item) const { return find(item) != -1; }

    // Checks if there is enough reserved space for _n_ elements
    constexpr bool has_space_for(s64 n) { return (Count + size) <= Reserved; }

    //
    // Iterator:
    //
    using iterator = data_t *;
    using const_iterator = const data_t *;

    constexpr iterator begin() { return Data; }
    constexpr iterator end() { return Data + Count; }
    constexpr const_iterator begin() const { return Data; }
    constexpr const_iterator end() const { return Data + Count; }

    //
    // Operators:
    //

    // Returns a string which is a view into this buffer
    template <typename U = T, typename = typename enable_if<is_same_v<remove_cv_t<U>, char> || is_same_v<remove_cv_t<U>, u8>>::type>
    operator string() const { return string((const char *) Data, Count); }

    data_t &operator[](s64 index) { return get(index); }
    constexpr const data_t &operator[](s64 index) const { return get(index); }

    // Check two arrays for equality
    template <typename U>
    constexpr bool operator==(const array<U> &other) const {
        return compare(other) == -1;
    }

    template <typename U>
    constexpr bool operator!=(const array<U> &other) const {
        return !(*this == other);
    }

    template <typename U>
    constexpr bool operator<(const array<U> &other) const {
        return compare_lexicographically(other) < 0;
    }

    template <typename U>
    constexpr bool operator>(const array<U> &other) const {
        return compare_lexicographically(other) > 0;
    }

    template <typename U>
    constexpr bool operator<=(const array<U> &other) const {
        return !(*this > other);
    }

    template <typename U>
    constexpr bool operator>=(const array<U> &other) const {
        return !(*this < other);
    }
};

template <typename T, s64 N>
stack_array<T, N>::operator array<T>() const { return array<T>((T *) Data, Count); }

inline string::operator array<char>() const { return array<char>((char *) Data, ByteLength); }

// Be careful not to call this with _dest_ pointing to _src_!
// Returns just _dest_.
template <typename T>
array<T> *clone(array<T> *dest, const array<T> &src) {
    dest->reset();
    dest->append_pointer_and_size(src.Data, src.Count);
    return dest;
}

//
// == and != for stack_array and array
//
template <typename T, typename U, s64 N>
constexpr bool operator==(const array<T> &left, const stack_array<U, N> &right) {
    static_assert(is_equal_comparable_v<T, U>, "Types cannot be compared with operator ==");

    if (left.Count != right.Count) return false;

    For(range(left.Count)) {
        if (!(left.Data[it] == right.Data[it])) {
            return false;
        }
    }
    return true;
}

template <typename T, typename U, s64 N>
constexpr bool operator==(const stack_array<U, N> &left, const array<T> &right) {
    return right == left;
}

template <typename T, typename U, s64 N>
constexpr bool operator!=(const array<T> &left, const stack_array<U, N> &right) {
    return !(left == right);
}

template <typename T, typename U, s64 N>
constexpr bool operator!=(const stack_array<U, N> &left, const array<T> &right) {
    return right != left;
}

LSTD_END_NAMESPACE
