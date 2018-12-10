#pragma once

#include "../context.h"

#include "memory.h"

CPPU_BEGIN_NAMESPACE

template <typename T>
struct Dynamic_Array {
    using Data_Type = T;

    Data_Type *Data = 0;
    size_t Count = 0, _Reserved = 0;

    // The allocator used for expanding the array.
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

    Dynamic_Array() {}
    Dynamic_Array(const Dynamic_Array &other) {
        _Reserved = other._Reserved;
        Count = other.Count;

        Data = New_And_Ensure_Allocator<Data_Type>(_Reserved, Allocator);
        CopyElements(Data, other.Data, _Reserved);
    }

    Dynamic_Array(Dynamic_Array &&other) { other.swap(*this); }
    ~Dynamic_Array() { release(); }

    Dynamic_Array &operator=(const Dynamic_Array &other) {
        release();

        Function(other).swap(*this);
        return *this;
    }

    Dynamic_Array &operator=(Dynamic_Array &&other) {
        release();

        Function(std::move(other)).swap(*this);
        return *this;
    }

    // Clears the array and deallocates memory
    void release() {
        if (Data) Delete(Data, _Reserved, Allocator);

        Data = null;
        Count = 0;
        _Reserved = 0;
    }

    void insert(Data_Type *where, const Data_Type &item) {
        uptr_t offset = where - begin();
        if (Count >= _Reserved) {
            size_t required = 2 * _Reserved;
            if (required < 8) required = 8;

            _reserve(required);
        }

        // The reserve above might have invalidated the old pointer
        where = begin() + offset;
        assert(where >= begin() && where <= end());

        if (offset < Count) {
            MoveElements(where + 1, where, Count - offset);
        }
        *where = item;
        Count++;
    }

    // Insert a range of items (begin, end].
    void insert(Data_Type *where, Data_Type *begin, Data_Type *end) {
        size_t elementsCount = end - begin;
        uptr_t offset = where - this->begin();

        size_t required = _Reserved;
        while (Count + elementsCount >= required) {
            required = 2 * _Reserved;
            if (required < 8) required = 8;
        }
        _reserve(required);

        // The reserve above might have invalidated the old pointer
        where = this->begin() + offset;
        assert(where >= this->begin() && where <= this->end());

        if (offset < Count) {
            MoveElements(where + elementsCount, where, Count - offset);
        }
        CopyElements(where, begin, elementsCount);
        Count += elementsCount;
    }

    // Returns the index of item in the array, -1 if it's not found
    s64 find(const Data_Type &item) const {
        Data_Type *index = Data;
        for (size_t i = 0; i < Count; i++) {
            if (*index++ == item) {
                return (s64) i;
            }
        }
        return -1;
    }

    void remove(Data_Type *where) {
        assert(where >= begin() && where < end());

        where->~Data_Type();

        uptr_t offset = where - begin();
        if (offset < Count) {
            MoveElements(where, where + 1, Count - offset - 1);
        }

        Count--;
    }

    void add(const Data_Type &item) {
        if (Count == 0) {
            _reserve(8);
            Data[Count++] = item;
        } else {
            insert(end(), item);
        }
    }

    void add_front(const Data_Type &item) {
        if (Count == 0) {
            add(item);
        } else {
            insert(begin(), item);
        }
    }

    void pop() {
        assert(Count > 0);
        Data[Count--].~Data_Type();
    }

    void swap(Dynamic_Array &other) {
        std::swap(Data, other.Data);
        std::swap(Count, other.Count);
        std::swap(_Reserved, other._Reserved);
        std::swap(Allocator, other.Allocator);
    }

    T *begin() { return Data; }
    T *end() { return Data + Count; }
    const T *begin() const { return Data; }
    const T *end() const { return Data + Count; }

    Data_Type &operator[](size_t index) { return Data[index]; }
    const Data_Type &operator[](size_t index) const { return Data[index]; }

    b32 operator==(const Dynamic_Array &other) {
        if (Count != other.Count) return false;
        for (size_t i = 0; i < Count; i++) {
            if (Data[i] != other.Data[i]) {
                return false;
            }
        }
        return true;
    }

    b32 operator!=(const Dynamic_Array &other) { return !(*this == other); }

    void _reserve(size_t reserve) {
        if (reserve <= _Reserved) return;

        Data_Type *newMemory = New_And_Ensure_Allocator<Data_Type>(reserve, Allocator);

        MoveElements(newMemory, Data, Count);
        Delete(Data, _Reserved, Allocator);

        Data = newMemory;
        _Reserved = reserve;
    }
};

//
//    == and != for static and dynamic arrays
//
#include "array.h"

template <typename T, typename U, size_t N>
b32 operator==(const Dynamic_Array<T> &left, const Array<U, N> &right) {
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
b32 operator==(const Array<U, N> &left, const Dynamic_Array<T> &right) {
    return right == left;
}

template <typename T, typename U, size_t N>
b32 operator!=(const Dynamic_Array<T> &left, const Array<U, N> &right) {
    return !(left == right);
}

template <typename T, typename U, size_t N>
b32 operator!=(const Array<U, N> &left, const Dynamic_Array<T> &right) {
    return right != left;
}

CPPU_END_NAMESPACE
