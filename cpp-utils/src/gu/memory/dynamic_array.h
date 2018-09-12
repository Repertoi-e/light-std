#pragma once

#include "../context.h"
#include "memory.h"

GU_BEGIN_NAMESPACE

template <typename T>
struct Dynamic_Array {
    using Type = T;

    T *Data = 0;
    size_t Count = 0, _Reserved = 0;

    // The allocator used for expanding the array.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's one automatically.
    Allocator_Closure Allocator;

    Dynamic_Array() { Allocator = CONTEXT_ALLOC; }
    Dynamic_Array(Dynamic_Array const &other);
    Dynamic_Array(Dynamic_Array &&other);
    ~Dynamic_Array();

    T &operator[](size_t index) { return Data[index]; }
    Dynamic_Array &operator=(Dynamic_Array const &other);
    Dynamic_Array &operator=(Dynamic_Array &&other);

    bool operator==(Dynamic_Array const &other) {
        if (Count != other.Count) return false;
        for (size_t i = 0; i < Count; i++) {
            if (Data[i] != other.Data[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(Dynamic_Array const &other) { return !(*this == other); }
};

template <typename T>
inline T *begin(Dynamic_Array<T> &array) {
    return array.Data;
}

template <typename T>
inline T *end(Dynamic_Array<T> &array) {
    return array.Data + array.Count;
}

template <typename T>
void reserve(Dynamic_Array<T> &array, size_t reserve) {
    if (reserve <= array._Reserved) return;

    T *newMemory = New<T>(reserve, array.Allocator);

    CopyElements(newMemory, array.Data, array.Count);
    Delete(array.Data, array._Reserved, array.Allocator);

    array.Data = newMemory;
    array._Reserved = reserve;
}

template <typename T>
void insert(Dynamic_Array<T> &array, T *where, typename Dynamic_Array<T>::Type const &item) {
    assert(where >= begin(array) && where <= end(array));

    uptr_t offset = where - begin(array);
    if (array.Count >= array._Reserved) {
        size_t required = 2 * array._Reserved;
        if (required < 8) required = 8;

        reserve(array, required);
    }

    // The reserve above might have invalidated the old pointer
    where = begin(array) + offset;

    if (offset < array.Count) {
        MoveMemory(where + 1, where, (array.Count - offset) * sizeof(T));
    }
    // Just copying the memory doesn't work here because if T (or one of its members) has a copy constructor it won't
    // get called and may lead to unwanted behaviour.
    // CopyMemory(where, &item, sizeof(T));
    *where = item;
    array.Count++;
}

// Returns the index of item in the array, -1 if it's not found
template <typename T>
s32 find(Dynamic_Array<T> &array, typename Dynamic_Array<T>::Type const &item) {
    for (size_t i = 0; i < array.Count; i++) {
        if (array[i] == item) {
            return (s32) i;
        }
    }
    return -1;
}

template <typename T>
void remove(Dynamic_Array<T> &array, typename Dynamic_Array<T>::Type *where) {
    assert(where >= begin(array) && where < end(array));

    where->~T();

    uptr_t offset = where - begin(array);
    if (offset < array.Count) {
        MoveMemory(where, where + 1, (array.Count - offset - 1) * sizeof(T));
    }

    array.Count--;
}

template <typename T>
void add(Dynamic_Array<T> &array, typename Dynamic_Array<T>::Type const &item) {
    if (array.Count == 0) {
        reserve(array, 8);
        array.Data[array.Count++] = item;
    } else {
        insert(array, end(array), item);
    }
}

template <typename T>
void add_front(Dynamic_Array<T> &array, typename Dynamic_Array<T>::Type const &item) {
    if (array.Count == 0) {
        add(array, item);
    } else {
        insert(array, begin(array), item);
    }
}

template <typename T>
void pop(Dynamic_Array<T> &array) {
    assert(array.Count > 0);
    array.Data[array.Count--].~T();
}

// Clears the array and deallocates memory
template <typename T>
void release(Dynamic_Array<T> &array) {
    if (array.Data) Delete(array.Data, array._Reserved, array.Allocator);

    array.Data = null;

    array.Count = 0;
    array._Reserved = 0;
}

template <typename T>
Dynamic_Array<T>::Dynamic_Array(Dynamic_Array<T> const &other) {
    _Reserved = other._Reserved;
    Count = other.Count;

    Data = New<T>(_Reserved, Allocator);
    CopyElements(Data, other.Data, _Reserved);
}

template <typename T>
inline Dynamic_Array<T>::Dynamic_Array(Dynamic_Array<T> &&other) {
    *this = std::move(other);
}

template <typename T>
inline Dynamic_Array<T> &Dynamic_Array<T>::operator=(Dynamic_Array<T> const &other) {
    if (Data) Delete(Data, _Reserved, Allocator);

    _Reserved = other._Reserved;
    Count = other.Count;

    Data = New<T>(_Reserved, Allocator);
    CopyElements(Data, other.Data, _Reserved);

    return *this;
}

template <typename T>
inline Dynamic_Array<T> &Dynamic_Array<T>::operator=(Dynamic_Array<T> &&other) {
    if (this != &other) {
        if (Data) Delete(Data, _Reserved, Allocator);

        Data = other.Data;
        _Reserved = other._Reserved;
        Count = other.Count;

        other.Data = 0;
        other._Reserved = 0;
        other.Count = 0;
    }
    return *this;
}

template <typename T>
Dynamic_Array<T>::~Dynamic_Array() {
    release(*this);
}

//
//	== and != for static and dynamic arrays
//
#include "array.h"

template <typename T, typename U, size_t N>
bool operator==(Dynamic_Array<T> const &left, Array<U, N> const &right) {
    if constexpr (!std::is_same_v<T, U>) {
        return false;
    } else {
        if (left.Count != right.Count) return false;
        for (size_t i = 0; i < left.Count; i++) {
            if (left.Data[i] != right.Data[i]) {
                return false;
            }
        }
        return true;
    }
}

template <typename T, typename U, size_t N>
bool operator==(Array<U, N> const &left, Dynamic_Array<T> const &right) {
    return right == left;
}

template <typename T, typename U, size_t N>
bool operator!=(Dynamic_Array<T> const &left, Array<U, N> const &right) {
    return !(left == right);
}

template <typename T, typename U, size_t N>
bool operator!=(Array<U, N> const &left, Dynamic_Array<T> const &right) {
    return right != left;
}

GU_END_NAMESPACE
