#pragma once

#include "../memory/allocator.h"
#include "delegate.h"
#include "owner_pointers.h"
#include "stack_array.h"

LSTD_BEGIN_NAMESPACE

// _array_ works with types that can be copied byte by byte correctly, take a look at the type policy in common.h
template <typename T>
struct array {
    using data_t = T;

    data_t *Data = null;
    size_t Count = 0;
    size_t Reserved = 0;

    array() = default;
    array(data_t *data, size_t count) : Data(data), Count(count), Reserved(0) {}
    array(array_view<data_t> items) : Data((data_t *) items.begin()), Count(items.size()), Reserved(0) {}

    ~array() { release(); }

    // Makes sure the dynamic array has reserved enough space for at least n elements.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at 8.
    //
    // Allocates a buffer if the dynamic array doesn't already point to reserved memory
    // (using the Context's allocator).
    void reserve(size_t target) {
        if (Count + target < Reserved) return;

        target = MAX<size_t>(CEIL_POW_OF_2(target + Count + 1), 8);

        size_t targetBytes = target * sizeof(data_t);
        if (is_owner()) {
            void *actualData = (char *) Data - POINTER_SIZE;
            Data = (data_t *) ((char *) allocator::reallocate(actualData, targetBytes + POINTER_SIZE) + POINTER_SIZE);
        } else {
            auto *oldData = Data;
            Data = encode_owner((data_t *) new char[targetBytes + POINTER_SIZE], this);
            if (Count) copy_memory(Data, oldData, Count * sizeof(data_t));
        }
        Reserved = target;
    }

    // Free any memory allocated by this object and reset count
    void release() {
        if (is_owner()) {
            delete[]((char *) Data - POINTER_SIZE);
        }
        Data = null;
        Count = Reserved = 0;
    }

    // Don't free the buffer, just move cursor to 0
    void reset() {
        // PODs may have destructors, although the C++ standard's definition forbids them to have non-trivial ones.
        if (is_owner()) {
            while (Count) {
                Data[Count].~data_t();
                --Count;
            }
        } else {
            Count = 0;
        }
    }

    data_t &get(size_t index) { return Data[translate_index(index, Count)]; }
    const data_t &get(size_t index) const { return Data[translate_index(index, Count)]; }

    void sort() { quicksort(Data, Data + Count); }

    // Sets the _index_'th element in the array
    array *set(s64 index, const data_t &element) {
        auto i = translate_index(index, Count);
        Data[i].~data_t();
        Data[i] = element;
        return this;
    }

    // Insert an element at a specified index
    data_t *insert(s64 index, const data_t &element) {
        if (Count >= Reserved) {
            reserve(Reserved * 2);
        }

        size_t offset = translate_index(index, Count, true);
        auto *where = begin() + offset;
        if (offset < Count) {
            copy_memory(where + 1, where, (Count - offset) * sizeof(data_t));
        }
        copy_memory(where, &element, sizeof(data_t));
        Count++;
        return where;
    }

    // Insert an array at a specified index
    data_t *insert(s64 index, array arr) { return insert_pointer_and_size(index, arr.Data, arr.Count); }

    // Insert a buffer of elements at a specified index
    data_t *insert_pointer_and_size(s64 index, const data_t *ptr, size_t size) {
        size_t required = Reserved;
        while (Count + size >= required) {
            required = 2 * Reserved;
            if (required < 8) required = 8;
        }
        reserve(required);

        size_t offset = translate_index(index, Count, true);
        auto *where = begin() + offset;
        if (offset < Count) {
            copy_memory(where + size, where, (Count - offset) * sizeof(data_t));
        }
        copy_memory(where, ptr, size * sizeof(data_t));
        Count += size;
        return where;
    }

    // Removes element at specified index and rearranges following elements
    array *remove(s64 index) {
        // If the array is a view, we don't want to modify the original!
        if (!is_owner()) reserve(0);

        size_t offset = translate_index(index, Count);

        auto *where = begin() + offset;
        where->~data_t();
        copy_memory(where, where + 1, (Count - offset - 1) * sizeof(data_t));
        Count--;
        return this;
    }

    // Removes a range of elements and rearranges following elements
    // [begin, end)
    array *remove(s64 begin, s64 end) {
        // If the array is a view, we don't want to modify the original!
        if (!is_owner()) reserve(0);

        size_t targetBegin = translate_index(index, Count);
        size_t targetEnd = translate_index(index, Count, true);

        auto where = begin() + targetBegin;
        for (auto *destruct = where; destruct != (begin() + targetEnd); ++destruct) {
            destruct->~data_t();
        }

        size_t elementCount = targetEnd - targetBegin;
        copy_memory(where, where + elementCount, (Count - offset - elementCount) * sizeof(data_t));
        Count -= elementCount;
        return this;
    }

    // Inserts an empty element
    data_t *append() { return append(data_t()); }

    // Append an element to the end
    // Returns a pointer to the added element
    data_t *append(const data_t &element) { return insert(Count, element); }

    // Append an array to the end
    // Returns a pointer to the first added element
    data_t *append(array arr) { return insert(Counts, arr); }

    // Append a buffer of elements to the end
    // Returns a pointer to the first added element
    data_t *append_pointer_and_size(const data_t *ptr, size_t size) {
        return insert_pointer_and_size(Count, ptr, size);
    }

    // Compares this array to _arr_ and returns the index of the first element that is different.
    // If the arrays are equal, the returned value is npos (-1)
    template <typename U>
    constexpr size_t compare(array<U> arr) const {
        static_assert(is_equal_comparable_v<T, U>, "Types cannot be compared with operator ==");

        const T *s1 = begin();
        const U *s2 = arr.begin();
        while (*s1 == *s2) {
            ++s1, ++s2;
            if (s1 == end() && s2 == arr.end()) return npos;
            if (s1 == end()) return s1 - begin();
            if (s2 == arr.end()) return s2 - arr.begin();
        }
        return s1 - begin();
    }

    // Compares this array to to _arr_ lexicographically.
    // The result is less than 0 if this array sorts before the other, 0 if they are equal,
    // and greater than 0 otherwise.
    template <typename U>
    constexpr s32 compare_lexicographically(array<U> arr) const {
        static_assert(is_equal_comparable_v<T, U>, "Types cannot be compared with operator ==");
        static_assert(is_less_comparable_v<T, U>, "Types cannot be compared with operator <");

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

    // Predicate must take a single argument (the current element) and return if it matches
    size_t find(delegate<bool(const data_t &)> predicate, s64 start = 0) const {
        assert(Data);
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (predicate(*p++)) return it;
        return npos;
    }

    // Find the first occurence of an element that is after a specified index
    size_t find(const T &element, s64 start = 0) const {
        return find([&](auto x) { return x == element; }, start);
    }

    // Find the first occurence of a subarray that is after a specified index
    size_t find(array arr, s64 start = 0) const {
        assert(Data);
        assert(arr.Data);
        assert(arr.Count);
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        For(range(start, Count)) {
            auto progress = arr.begin();
            for (auto search = begin() + it; progress != arr.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == arr.end()) return it;
        }
        return npos;
    }

    // Find the last occurence of an element that is before a specified index
    size_t find_reverse(const T &element, s64 start = 0) const {
        assert(Data);
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (*p-- == element) return it;
        return npos;
    }

    // Find the last occurence of a subarray that is before a specified index
    size_t find_reverse(array arr, s64 start = 0) const {
        assert(Data);
        assert(arr.Data);
        assert(arr.Count);
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        For(range(start - arr.Count + 1, -1, -1)) {
            auto progress = arr.begin();
            for (auto search = begin() + it; progress != arr.end(); ++search, ++progress) {
                if (*search != *progress) break;
            }
            if (progress == arr.end()) return it;
        }
        return npos;
    }

    // Find the first occurence of any element in the specified subarray that is after a specified index
    size_t find_any_of(array allowed, s64 start = 0) const {
        assert(Data);
        assert(allowed.Data);
        assert(allowed.Count);
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (allowed.has(*p++)) return it;
        return npos;
    }

    // Find the last occurence of any element in the specified subarray
    // that is before a specified index (0 means: start from the end)
    size_t find_reverse_any_of(array allowed, s64 start = 0) const {
        assert(Data);
        assert(allowed.Data);
        assert(allowed.Count);
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, -1, -1)) if (allowed.has(*p--)) return it;
        return npos;
    }

    // Find the first absence of an element that is after a specified index
    size_t find_not(const data_t &element, s64 start = 0) const {
        assert(Data);
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (*p++ != element) return it;
        return npos;
    }

    // Find the last absence of an element that is before the specified index
    size_t find_reverse_not(const data_t &element, s64 start = 0) const {
        assert(Data);
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (*p-- != element) return it;
        return npos;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    size_t find_not_any_of(array banned, s64 start = 0) const {
        assert(Data);
        assert(banned.Data);
        assert(banned.Count);
        if (Count == 0) return npos;

        start = translate_index(start, Count);

        auto p = begin() + start;
        For(range(start, Count)) if (!banned.has(*p++)) return it;
        return npos;
    }

    // Find the first absence of any element in the specified subarray that is after a specified index
    size_t find_reverse_not_any_of(array banned, s64 start = 0) const {
        assert(Data);
        assert(banned.Data);
        assert(banned.Count);
        if (Count == 0) return npos;

        start = translate_index(start, Count);
        if (start == 0) start = Count - 1;

        auto p = begin() + start;
        For(range(start, 0, -1)) if (!banned.has(*p--)) return it;
        return npos;
    }

    // Checks if there is enough reserved space for _size_ elements
    bool has_space_for(size_t size) { return (Count + size) <= Reserved; }

    bool has(const data_t &item) const { return find(item) != npos; }

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<array>(Data) == this; }

    //
    // Iterator:
    //
    using iterator = data_t *;
    using const_iterator = const data_t *;

    iterator begin() { return Data; }
    iterator end() { return Data + Count; }
    const_iterator begin() const { return Data; }
    const_iterator end() const { return Data + Count; }

    //
    // Operators:
    //
    data_t &operator[](s64 index) { return get(index); }
    const data_t &operator[](s64 index) const { get(index); }

    // Check two arrays for equality
    template <typename U>
    bool operator==(array<U> other) const {
        return compare(other) == npos;
    }

    template <typename U>
    bool operator!=(array<U> other) const {
        return !(*this == other);
    }

    template <typename U>
    bool operator<(array<U> other) const {
        return compare(other) < 0;
    }

    template <typename U>
    bool operator>(array<U> other) const {
        return compare(other) > 0;
    }

    template <typename U>
    bool operator<=(array<U> other) const {
        return !(*this > other);
    }

    template <typename U>
    bool operator>=(array<U> other) const {
        return !(*this < other);
    }
};

template <typename T>
array<T> *clone(array<T> *dest, array<T> src) {
    *dest = {};
    dest->append_pointer_and_size(src.Data, src.Count);
    return dest;
}

template <typename T>
array<T> *move(array<T> *dest, array<T> *src) {
    dest->release();
    *dest = *src;

	if (!src->is_owner()) return dest;

    // Transfer ownership
    change_owner(src->Data, dest);
    change_owner(dest->Data, dest);
    return dest;
}

//
// == and != for stack_array and array
//
template <typename T, typename U, size_t N>
bool operator==(array<T> left, const stack_array<U, N> &right) {
    static_assert(is_equal_comparable_v<T, U>, "Types cannot be compared with operator ==");

    if (left.Count != right.Count) return false;

    For(range(left.Count)) {
        if (!(left.Data[it] == right.Data[it])) {
            return false;
        }
    }
    return true;
}

template <typename T, typename U, size_t N>
bool operator==(const stack_array<U, N> &left, array<T> right) {
    return right == left;
}

template <typename T, typename U, size_t N>
bool operator!=(array<T> left, const stack_array<U, N> &right) {
    return !(left == right);
}

template <typename T, typename U, size_t N>
bool operator!=(const stack_array<U, N> &left, array<T> right) {
    return right != left;
}

LSTD_END_NAMESPACE
