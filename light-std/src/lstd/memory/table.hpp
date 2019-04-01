#pragma once

#include "../context.hpp"
#include "memory.hpp"

#include "hash.hpp"

#include <tuple>

LSTD_BEGIN_NAMESPACE

template <typename Key, typename Value>
struct table_iterator;

// Table means hash-map/unordered_map etc.
template <typename Key, typename Value>
struct table {
    using key_t = Key;
    using value_t = Value;

    static const size_t MINIMUM_SIZE = 32;
    size_t Count = 0;
    size_t Reserved = 0;

    // By default, the value that gets returned if a value is not found
    // is a default constructed value_t. This value can be changed if
    // special behaviour is desired.
    value_t UnfoundValue = value_t();

    // The allocator used for expanding the table.
    // This value is null until this object allocates memory or the user sets it manually.
    allocator_closure Allocator;

    table() = default;
    table(const table& other);
    table(table&& other);
    ~table();

    void release();

    // Copies the key and the value into the table.
    void put(const key_t& key, const value_t& value);

    // Returns a tuple of the value and a bool (true if found). Doesn't return the value by reference so
    // modifying it doesn't update it in the table, use pointers if you want that kind of behaviour.
    std::tuple<value_t&, bool> find(const key_t& key);

    bool has(const key_t& key);

    table_iterator<Key, Value> begin();
    table_iterator<Key, Value> end();
    table_iterator<Key, Value> begin() const;
    table_iterator<Key, Value> end() const;

    void swap(table& other);

    table& operator=(const table& other);
    table& operator=(table&& other);

   private:
    // This doesn't free the old memory before allocating new, so that's up to the caller to do.
    void reserve(size_t size);

    s32 find_index(const key_t& key, uptr_t hash);

    // Double's the size of the table and copies the elements to their new location.
    void grow();

    // We store slots as SOA to minimize cache misses.
    bool* _OccupancyMask = null;
    key_t* _Keys = null;
    value_t* _Values = null;
    uptr_t* _Hashes = null;

    friend struct table_iterator<key_t, value_t>;
};

template <typename Key, typename Value>
table<Key, Value>::table(const table& other) {
    Count = other.Count;
    Reserved = other.Reserved;
    UnfoundValue = other.UnfoundValue;

    _OccupancyMask = new (&Allocator, ensure_allocator) bool[Reserved];
    _Keys = new (Allocator) key_t[Reserved];
    _Values = new (Allocator) value_t[Reserved];
    _Hashes = new (Allocator) uptr_t[Reserved];

    copy_elements(_OccupancyMask, other._OccupancyMask, Reserved);
    copy_elements(_Keys, other._Keys, Reserved);
    copy_elements(_Values, other._Values, Reserved);
    copy_elements(_Hashes, other._Hashes, Reserved);
}

template <typename Key, typename Value>
table<Key, Value>::table(table&& other) {
    other.swap(*this);
}

template <typename Key, typename Value>
table<Key, Value>::~table() {
    release();
}

template <typename Key, typename Value>
void table<Key, Value>::release() {
    if (Reserved) {
        delete[] _OccupancyMask;
        delete[] _Keys;
        delete[] _Values;
        delete[] _Hashes;

        _OccupancyMask = null;
        _Keys = null;
        _Values = null;
        _Hashes = null;
        Reserved = 0;
        Count = 0;
    }
}

template <typename Key, typename Value>
void table<Key, Value>::put(const key_t& key, const value_t& value) {
    uptr_t h = hash<key_t>::get(key);
    s32 index = find_index(key, h);
    if (index == -1) {
        if (Count >= Reserved) {
            grow();
        }
        assert(Count <= Reserved);

        index = (s32)(h % Reserved);
        while (_OccupancyMask[index]) {
            // Resolve collision
            index++;
            if ((size_t) index >= Reserved) {
                index = 0;
            }
        }

        Count++;
    }

    _OccupancyMask[index] = true;
    new (&_Keys[index]) key_t(key);
    new (&_Values[index]) value_t(value);
    _Hashes[index] = h;
}

template <typename Key, typename Value>
std::tuple<typename table<Key, Value>::value_t&, bool> table<Key, Value>::find(const key_t& key) {
    uptr_t h = hash<key_t>::get(key);

    s32 index = find_index(key, h);
    if (index == -1) {
        return {UnfoundValue, false};
    }

    return std::forward_as_tuple(_Values[index], true);
}

template <typename Key, typename Value>
bool table<Key, Value>::has(const key_t& key) {
    auto [_, found] = find(key);
    return found;
}

template <typename Key, typename Value>
table_iterator<Key, Value> table<Key, Value>::begin() {
    return table_iterator<Key, Value>(*this);
}

template <typename Key, typename Value>
table_iterator<Key, Value> table<Key, Value>::end() {
    return table_iterator<Key, Value>(*this, Reserved);
}

template <typename Key, typename Value>
table_iterator<Key, Value> table<Key, Value>::begin() const {
    return table_iterator<Key, Value>(*this);
}

template <typename Key, typename Value>
table_iterator<Key, Value> table<Key, Value>::end() const {
    return table_iterator<Key, Value>(*this, Reserved);
}

template <typename Key, typename Value>
void table<Key, Value>::swap(table& other) {
    std::swap(Count, other.Count);
    std::swap(Allocator, other.Allocator);
    std::swap(Reserved, other.Reserved);
    std::swap(UnfoundValue, other.UnfoundValue);

    std::swap(_OccupancyMask, other._OccupancyMask);
    std::swap(_Keys, other._Keys);
    std::swap(_Values, other._Values);
    std::swap(_Hashes, other._Hashes);
}

template <typename Key, typename Value>
table<Key, Value>& table<Key, Value>::operator=(const table& other) {
    release();

    table(other).swap(*this);
    return *this;
}

template <typename Key, typename Value>
table<Key, Value>& table<Key, Value>::operator=(table&& other) {
    release();

    table(std::move(other)).swap(*this);
    return *this;
}

template <typename Key, typename Value>
void table<Key, Value>::reserve(size_t size) {
    Reserved = size;

    _OccupancyMask = new (&Allocator, ensure_allocator) bool[size];
    zero_memory(_OccupancyMask, size);

    _Keys = new (Allocator) Key[size];
    _Values = new (Allocator) Value[size];
    _Hashes = new (Allocator) uptr_t[size];
}

template <typename Key, typename Value>
s32 table<Key, Value>::find_index(const key_t& key, uptr_t hash) {
    if (!Reserved) {
        return -1;
    }

    size_t index = hash % Reserved;
    while (_OccupancyMask[index]) {
        if (_Hashes[index] == hash) {
            if (_Keys[index] == key) {
                return (s32) index;
            }
        }

        index++;
        if (index >= Reserved) {
            index = 0;
        }
    }

    return -1;
}

template <typename Key, typename Value>
void table<Key, Value>::grow() {
    size_t oldReserved = Reserved;
    auto oldOccupancyMask = _OccupancyMask;
    auto oldKeys = _Keys;
    auto oldValues = _Values;
    auto oldHashes = _Hashes;

    size_t newSize = Reserved * 2;
    if (newSize < MINIMUM_SIZE) {
        newSize = MINIMUM_SIZE;
    }

    reserve(newSize);

    For(range(oldReserved)) {
        if (oldOccupancyMask[it]) {
            put(oldKeys[it], oldValues[it]);
        }
    }

    if (oldReserved) {
        delete[] oldOccupancyMask;
        delete[] oldKeys;
        delete[] oldValues;
        delete[] oldHashes;
    }
}

template <typename Key, typename Value>
struct table_iterator {
    const table<Key, Value>& ParentTable;
    s64 SlotIndex = 0;

    explicit table_iterator(const table<Key, Value>& table, s64 index = -1) : ParentTable(table), SlotIndex(index) {
        // Find the first pair
        ++(*this);
    }

    table_iterator& operator++() {
        while (SlotIndex < (s64) ParentTable.Reserved) {
            SlotIndex++;
            if (SlotIndex == ParentTable.Reserved) break;
            if (ParentTable._OccupancyMask && ParentTable._OccupancyMask[SlotIndex]) {
                break;
            }
        }
        return *this;
    }

    table_iterator operator++(int) {
        table_iterator pre = *this;
        ++(*this);
        return pre;
    }

    bool operator==(table_iterator other) const { return SlotIndex == other.SlotIndex; }
    bool operator!=(table_iterator other) const { return !(*this == other); }

    std::tuple<Key&, Value&> operator*() const {
        return std::forward_as_tuple(ParentTable._Keys[SlotIndex], ParentTable._Values[SlotIndex]);
    }
};

LSTD_END_NAMESPACE