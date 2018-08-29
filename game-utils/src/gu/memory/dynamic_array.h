#pragma once

#include "../context.h"
#include "memory.h"

GU_BEGIN_NAMESPACE

template <typename T>
struct Dynamic_Array {
    using Type = T;

    T *Data = 0;
    size_t Count = 0, Reserved = 0;

    // The allocator used for expanding the array.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's
    // one automatically.
    Allocator_Closure Allocator;

    Dynamic_Array() { Allocator = CONTEXT_ALLOC; }
    Dynamic_Array(Dynamic_Array const &other);
    Dynamic_Array(Dynamic_Array &&other);
    ~Dynamic_Array();

    T &operator[](size_t index) { return Data[index]; }
    Dynamic_Array &operator=(Dynamic_Array &&other);
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
    if (reserve <= array.Reserved) {
        return;
    }

    T *newMemory = New<T>(reserve, array.Allocator);

    CopyMemory(newMemory, array.Data, array.Count * sizeof(T));
    Delete(array.Data, array.Reserved, array.Allocator);

    array.Data = newMemory;
    array.Reserved = reserve;
}

template <typename T>
void insert(Dynamic_Array<T> &array, T *where, typename Dynamic_Array<T>::Type const &item) {
    assert(where >= begin(array) && where <= end(array));

    ptr_t offset = where - begin(array);
    if (array.Count >= array.Reserved) {
        size_t required = 2 * array.Reserved;
        if (required < 8) {
            required = 8;
        }

        reserve(array, required);
    }

    // The reserve above might have invalidated the old pointer
    where = begin(array) + offset;

    if (offset < array.Count) {
        MoveMemory(where + 1, where, ((size_t) array.Count - (size_t) offset) * sizeof(T));
    }
    // Just copying the memory doesn't work here because if T (or one of its members) has a copy constructor it won't
    // get called and may lead to unwanted behaviour.
    // CopyMemory(where, &item, sizeof(T));
    *where = item;
    array.Count++;
}

template <typename T>
void remove(Dynamic_Array<T> &array, T *where) {
    assert(where >= begin(array) && where < end(array));

    where->~T();

    ptr_t offset = where - begin(array);
    if (offset < array.Count) {
        MoveMemory(where, where + 1, ((size_t) array.Count - (size_t) offset - 1) * sizeof(T));
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

template <typename T>
T *first(Dynamic_Array<T> &array) {
    return begin(array);
}

template <typename T>
T *last(Dynamic_Array<T> &array) {
    return end(array) - 1;
}

// Clears the array and deallocates memory
template <typename T>
void release(Dynamic_Array<T> &array) {
    if (array.Data) {
        Delete(array.Data, array.Reserved, array.Allocator);
    }
    array.Data = 0;

    array.Count = 0;
    array.Reserved = 0;
}

template <typename T>
Dynamic_Array<T>::Dynamic_Array(Dynamic_Array<T> const &other) {
    Allocator = other.Allocator;
    Reserved = other.Reserved;
    Count = other.Count;

    Data = New<T>(Reserved, Allocator);
    CopyMemory(Data, other.Data, Reserved * sizeof(T));
}

template <typename T>
inline Dynamic_Array<T>::Dynamic_Array(Dynamic_Array<T> &&other) {
    Allocator = other.Allocator;
    Data = other.Data;
    Reserved = other.Reserved;
    Count = other.Count;

    other.Data = 0;
    other.Reserved = 0;
    other.Count = 0;
}

template <typename T>
inline Dynamic_Array<T> &Dynamic_Array<T>::operator=(Dynamic_Array<T> &&other) {
    if (this != &other) {
        if (Data) {
            Delete(Data, Allocator);
        }

        Allocator = other.Allocator;
        Data = other.Data;
        Reserved = other.Reserved;
        Count = other.Count;

        other.Data = 0;
        other.Reserved = 0;
        other.Count = 0;
    }
    return *this;
}

template <typename T>
Dynamic_Array<T>::~Dynamic_Array() {
    release(*this);
}

GU_END_NAMESPACE
