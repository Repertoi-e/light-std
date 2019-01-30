#pragma once

#include "../context.hpp"

#include "array.hpp"
#include "memory.hpp"

LSTD_BEGIN_NAMESPACE

template <typename T>
struct Dynamic_Array {
    using Data_Type = T;

    Data_Type *Data = 0;
    size_t Count = 0, Reserved = 0;

    // The allocator used for expanding the array.
    // This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

    Dynamic_Array() {}
    Dynamic_Array(const Dynamic_Array &other) {
        Reserved = other.Reserved;
        Count = other.Count;

        Data = New_and_ensure_allocator<Data_Type>(Reserved, Allocator);
        copy_elements(Data, other.Data, Reserved);
    }

    Dynamic_Array(Dynamic_Array &&other) { other.swap(*this); }
    ~Dynamic_Array() { release(); }

    Dynamic_Array &operator=(const Dynamic_Array &other) {
        release();

        Dynamic_Array(other).swap(*this);
        return *this;
    }

    Dynamic_Array &operator=(Dynamic_Array &&other) {
        release();

        Dynamic_Array(std::move(other)).swap(*this);
        return *this;
    }

    // Clears the array
    void clear() {
        auto it = end() - 1;
        while (Count != 0) {
            it->~Data_Type();
            --Count;
        };
    }

    // Clears the array and deallocates memory
    void release() {
        if (Data) Delete(Data, Reserved, Allocator);

        Data = null;
        Count = 0;
        Reserved = 0;
    }

    void insert(Data_Type *where, const Data_Type &item) {
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
        *where = item;
        Count++;
    }

    // Insert a range of items (begin, end].
    void insert(Data_Type *where, Data_Type *begin, Data_Type *end) {
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

    void insert_front(const Data_Type &item) {
        if (Count == 0) {
            add(item);
        } else {
            insert(begin(), item);
        }
    }

    void add(const Data_Type &item) {
        if (Count == 0) {
            reserve(8);
            Data[Count++] = item;
        } else {
            insert(end(), item);
        }
    }

    // Find the index of the first occuring _item_ in the array, npos if it's not found
    size_t find(const Data_Type &item) const {
        Data_Type *index = Data;
        For(range(Count)) {
            if (*index++ == item) {
                return it;
            }
        }
        return npos;
    }

    // Find the index of the last occuring _item_ in the array, npos if it's not found
    size_t find_last(const Data_Type &item) const {
        Data_Type *index = Data;
        For(range(Count)) {
            if (*index-- == item) {
                return Count - it - 1;
            }
        }
        return npos;
    }

    // Checks if there is enough reserved space for _count_ elements
    bool has_space_for(size_t count) {
        if (Count + count > Reserved) {
            return false;
        }
        return true;
    }

    bool has(const Data_Type &item) { return find(item) != npos; }

    void sort() { std::sort(begin(), end()); }

    template <typename Pred>
    void sort(Pred &&predicate) {
        std::sort(begin(), end(), predicate);
    }

    void remove(Data_Type *where) {
        assert(where >= begin() && where < end());

        where->~Data_Type();

        uptr_t offset = where - begin();
        if (offset < Count) {
            move_elements(where, where + 1, Count - offset - 1);
        }

        Count--;
    }

    void pop() {
        assert(Count > 0);
        Data[Count--].~Data_Type();
    }

    void swap(Dynamic_Array &other) {
        std::swap(Data, other.Data);
        std::swap(Count, other.Count);
        std::swap(Reserved, other.Reserved);
        std::swap(Allocator, other.Allocator);
    }

    T *begin() { return Data; }
    T *end() { return Data + Count; }
    const T *begin() const { return Data; }
    const T *end() const { return Data + Count; }

    Data_Type &operator[](size_t index) { return Data[index]; }
    const Data_Type &operator[](size_t index) const { return Data[index]; }

    bool operator==(const Dynamic_Array &other) {
        if (Count != other.Count) return false;
        For(range(Count)) {
            if (Data[it] != other.Data[it]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Dynamic_Array &other) { return !(*this == other); }

    void reserve(size_t reserve) {
        if (reserve <= Reserved) return;

        Data_Type *newMemory = New_and_ensure_allocator<Data_Type>(reserve, Allocator);

        move_elements(newMemory, Data, Count);
        Delete(Data, Reserved, Allocator);

        Data = newMemory;
        Reserved = reserve;
    }

    void grow(size_t n) { reserve(Reserved + n); }
};

//
//    == and != for static and dynamic arrays
//

template <typename T, typename U, size_t N>
bool operator==(const Dynamic_Array<T> &left, const Array<U, N> &right) {
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
bool operator==(const Array<U, N> &left, const Dynamic_Array<T> &right) {
    return right == left;
}

template <typename T, typename U, size_t N>
bool operator!=(const Dynamic_Array<T> &left, const Array<U, N> &right) {
    return !(left == right);
}

template <typename T, typename U, size_t N>
bool operator!=(const Array<U, N> &left, const Dynamic_Array<T> &right) {
    return right != left;
}

LSTD_END_NAMESPACE
