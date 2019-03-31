#pragma once

#include "../context.hpp"

#include "array.hpp"
#include "memory.hpp"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct dynamic_array {
    using data_t = T;

    data_t* Data = null;
    size_t Count = 0, Reserved = 0;

    // The allocator used for expanding the array.
    // This value is null until this object allocates memory or the user sets it manually.
    allocator_closure Allocator;

    dynamic_array() = default;
    dynamic_array(const dynamic_array& other);
    dynamic_array(dynamic_array&& other);
    ~dynamic_array();

    dynamic_array& operator=(const dynamic_array& other);
    dynamic_array& operator=(dynamic_array&& other);

    // Clears the array
    void clear();

    // Clears the array and deallocates memory
    void release();

    void reserve(size_t reserve);
    void grow(size_t n);

    // Insert a single item
    void insert(data_t* where, const data_t& item);

    // Insert a range of items (begin, end].
    void insert(data_t* where, data_t* begin, data_t* end);

    void insert_front(const data_t& item);

    // Inserts at the back
    void add(const data_t& item);

    // Find the index of the first occuring _item_ in the array, npos if it's not found
    size_t find(const data_t& item) const;

    // Find the index of the last occuring _item_ in the array, npos if it's not found
    size_t find_reverse(const data_t& item) const;

    // Checks if there is enough reserved space for _count_ elements
    bool has_space_for(size_t count) const;

    bool has(const data_t& item);

#if !defined LSTD_NO_CRT
    void sort();

    template <typename Pred>
    void sort(Pred&& predicate);
#endif

    void remove(data_t* where);
    void pop();

    void swap(dynamic_array& other);

    T* begin();
    T* end();
    const T* begin() const;
    const T* end() const;

    data_t& operator[](size_t index);
    const data_t& operator[](size_t index) const;

    bool operator==(const dynamic_array& other);
    bool operator!=(const dynamic_array& other);
};

template <typename T>
dynamic_array<T>::dynamic_array(const dynamic_array& other) {
    Reserved = other.Reserved;
    Count = other.Count;

    Data = new (&Allocator, ensure_allocator) data_t[Reserved];
    copy_elements(Data, other.Data, Reserved);
}

template <typename T>
dynamic_array<T>::dynamic_array(dynamic_array&& other) {
    other.swap(*this);
}

template <typename T>
dynamic_array<T>::~dynamic_array() {
    release();
}

template <typename T>
dynamic_array<T>& dynamic_array<T>::operator=(const dynamic_array& other) {
    release();

    dynamic_array(other).swap(*this);
    return *this;
}

template <typename T>
dynamic_array<T>& dynamic_array<T>::operator=(dynamic_array&& other) {
    release();

    dynamic_array(std::move(other)).swap(*this);
    return *this;
}

template <typename T>
void dynamic_array<T>::clear() {
    auto it = end() - 1;
    while (Count != 0) {
        it->~data_t();
        --Count;
    };
}

template <typename T>
void dynamic_array<T>::release() {
    if (Data) delete[] Data;

    Data = null;
    Count = 0;
    Reserved = 0;
}

template <typename T>
void dynamic_array<T>::insert(data_t* where, const data_t& item) {
    uptr_t offset = where - begin();
    if (Count >= Reserved) {
        size_t required = 2 * Reserved;
        if (required < 8) required = 8;

        reserve(required);
    }

    // The reserve above might have invalidated the old pointer
    where = begin() + offset;
    assert(where >= begin() && where <= end());

    if (offset < Count) {
        move_elements(where + 1, where, Count - offset);
    }
    new (where) data_t(item);
    Count++;
}

template <typename T>
void dynamic_array<T>::insert(data_t* where, data_t* begin, data_t* end) {
    size_t elementsCount = end - begin;
    uptr_t offset = where - this->begin();

    size_t required = Reserved;
    while (Count + elementsCount >= required) {
        required = 2 * Reserved;
        if (required < 8) required = 8;
    }
    reserve(required);

    // The reserve above might have invalidated the old pointer
    where = this->begin() + offset;
    assert(where >= this->begin() && where <= this->end());

    if (offset < Count) {
        move_elements(where + elementsCount, where, Count - offset);
    }
    copy_elements(where, begin, elementsCount);
    Count += elementsCount;
}

template <typename T>
void dynamic_array<T>::insert_front(const data_t& item) {
    if (Count == 0) {
        add(item);
    } else {
        insert(begin(), item);
    }
}

template <typename T>
void dynamic_array<T>::add(const data_t& item) {
    if (Count == 0) {
        reserve(8);
        Data[Count++] = item;
    } else {
        insert(end(), item);
    }
}

template <typename T>
size_t dynamic_array<T>::find(const data_t& item) const {
    data_t* index = Data;
    For(range(Count)) {
        if (*index++ == item) {
            return it;
        }
    }
    return npos;
}

template <typename T>
size_t dynamic_array<T>::find_reverse(const data_t& item) const {
    data_t* index = Data;
    For(range(Count)) {
        if (*index-- == item) {
            return Count - it - 1;
        }
    }
    return npos;
}

template <typename T>
bool dynamic_array<T>::has_space_for(size_t count) const {
    if (Count + count > Reserved) {
        return false;
    }
    return true;
}

template <typename T>
bool dynamic_array<T>::has(const data_t& item) {
    return find(item) != npos;
}

template <typename T>
void dynamic_array<T>::sort() {
    std::sort(begin(), end());
}

template <typename T>
template <typename Pred>
void dynamic_array<T>::sort(Pred&& predicate) {
    std::sort(begin(), end(), predicate);
}

template <typename T>
void dynamic_array<T>::remove(data_t* where) {
    assert(where >= begin() && where < end());

    where->~data_t();

    uptr_t offset = where - begin();
    if (offset < Count) {
        move_elements(where, where + 1, Count - offset - 1);
    }

    Count--;
}

template <typename T>
void dynamic_array<T>::pop() {
    assert(Count > 0);
    Data[Count--].~data_t();
}

template <typename T>
void dynamic_array<T>::swap(dynamic_array& other) {
    std::swap(Data, other.Data);
    std::swap(Count, other.Count);
    std::swap(Reserved, other.Reserved);
    std::swap(Allocator, other.Allocator);
}

template <typename T>
T* dynamic_array<T>::begin() {
    return Data;
}

template <typename T>
T* dynamic_array<T>::end() {
    return Data + Count;
}

template <typename T>
const T* dynamic_array<T>::begin() const {
    return Data;
}

template <typename T>
const T* dynamic_array<T>::end() const {
    return Data + Count;
}

template <typename T>
typename dynamic_array<T>::data_t& dynamic_array<T>::operator[](size_t index) {
    return Data[index];
}

template <typename T>
const typename dynamic_array<T>::data_t& dynamic_array<T>::operator[](size_t index) const {
    return Data[index];
}

template <typename T>
bool dynamic_array<T>::operator==(const dynamic_array& other) {
    if (Count != other.Count) return false;
    For(range(Count)) {
        if (Data[it] != other.Data[it]) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool dynamic_array<T>::operator!=(const dynamic_array& other) {
    return !(*this == other);
}

template <typename T>
void dynamic_array<T>::reserve(size_t reserve) {
    if (reserve <= Reserved) return;

    data_t* newMemory = new (&Allocator, ensure_allocator) data_t[reserve];

    if (Data) {
        move_elements(newMemory, Data, Count);
        delete[] Data;
    }

    Data = newMemory;
    Reserved = reserve;
}

template <typename T>
void dynamic_array<T>::grow(size_t n) {
    reserve(Reserved + n);
}

//
//    == and != for static and dynamic arrays
//

template <typename T, typename U, size_t N>
bool operator==(const dynamic_array<T>& left, const array<U, N>& right) {
    if constexpr (!std::is_same_v<T, U>) {
        return false;
    } else {
        if (left.Count != right.Count) return false;

        For(range(left.Count)) {
            if (left.Data[it] != right.Data[it]) {
                return false;
            }
        }
        return true;
    }
}

template <typename T, typename U, size_t N>
bool operator==(const array<U, N>& left, const dynamic_array<T>& right) {
    return right == left;
}

template <typename T, typename U, size_t N>
bool operator!=(const dynamic_array<T>& left, const array<U, N>& right) {
    return !(left == right);
}

template <typename T, typename U, size_t N>
bool operator!=(const array<U, N>& left, const dynamic_array<T>& right) {
    return right != left;
}

LSTD_END_NAMESPACE
