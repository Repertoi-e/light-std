#pragma once

#include "../context.h"
#include "memory.h"

#include "hash.h"

#include <iterator>
#include <tuple>

GU_BEGIN_NAMESPACE

template <typename Key, typename Value>
struct Table {
    using Key_Type = Key;
    using Value_Type = Value;

    static const size_t MINIMUM_SIZE = 32;
    size_t Count = 0, Reserved = 0;

    // By default, the value that gets returned is a default constructed Value_Type
    // This value can be changed if special behaviour is desired.
    Value_Type UnfoundValue = Value_Type();

    // The allocator used for expanding the table.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's
    // one automatically.
    Allocator_Closure Allocator;

    // Slots
    bool *OccupancyMask = null;
    Key_Type *Keys = null;
    Value_Type *Values = null;
    uptr_t *Hashes = null;

    Table() {}
    Table(Table const &other);
    Table(Table &&other);
    ~Table();

    Table &operator=(Table const &other);
    Table &operator=(Table &&other);
};

template <typename Key, typename Value>
Table<Key, Value>::Table(Table<Key, Value> const &other) {
    Reserved = other.Reserved;
    Allocator = other.Allocator;
    UnfoundValue = other.UnfoundValue;

    OccupancyMask = New<bool>(Reserved, Allocator);
    Keys = New<Key_Type>(Reserved, Allocator);
    Values = New<Value_Type>(Reserved, Allocator);
    Hashes = New<uptr_t>(Reserved, Allocator);

    CopyElements(OccupancyMask, other.OccupancyMask, Reserved);
    CopyElements(Keys, other.Keys, Reserved);
    CopyElements(Values, other.Values, Reserved);
    CopyElements(Hashes, other.Hashes, Reserved);
}

template <typename Key, typename Value>
Table<Key, Value>::Table(Table<Key, Value> &&other) {
    *this = std::move(other);
}

template <typename Key, typename Value>
void release(Table<Key, Value> &table) {
    if (table.Reserved) {
        table.Reserved = 0;
        Delete(table.OccupancyMask, table.Allocator);
        Delete(table.Keys, table.Allocator);
        Delete(table.Values, table.Allocator);
        Delete(table.Hashes, table.Allocator);
    }
}

template <typename Key, typename Value>
Table<Key, Value>::~Table() {
    release(*this);
}

template <typename Key, typename Value>
Table<Key, Value> &Table<Key, Value>::operator=(Table<Key, Value> const &other) {
    release(*this);

    Reserved = other.Reserved;
    Allocator = other.Allocator;
    UnfoundValue = other.UnfoundValue;

    OccupancyMask = New<bool>(Reserved, Allocator);
    Keys = New<Key_Type>(Reserved, Allocator);
    Values = New<Value_Type>(Reserved, Allocator);
    Hashes = New<uptr_t>(Reserved, Allocator);

    CopyElements(OccupancyMask, other.OccupancyMask, Reserved);
    CopyElements(Keys, other.Keys, Reserved);
    CopyElements(Values, other.Values, Reserved);
    CopyElements(Hashes, other.Hashes, Reserved);

    return *this;
}

template <typename Key, typename Value>
Table<Key, Value> &Table<Key, Value>::operator=(Table<Key, Value> &&other) {
    if (this != &other) {
        release(*this);

        Reserved = other.Reserved;
        Allocator = other.Allocator;
        UnfoundValue = other.UnfoundValue;

        OccupancyMask = other.OccupancyMask;
        Keys = other.Keys;
        Values = other.Values;
        Hashes = other.Hashes;

        other.Reserved = 0;
        other.OccupancyMask = null;
        other.Keys = null;
        other.Values = null;
        other.Hashes = null;
    }
}

template <typename Key, typename Value>
struct Table_Iterator : public std::iterator<std::forward_iterator_tag, std::tuple<Key const &, Value &>> {
    Table<Key, Value> const &Table;
    s64 SlotIndex = -1;

    explicit Table_Iterator(::Table<Key, Value> const &table, s64 index = -1) : Table(table), SlotIndex(index) {
        // Find the first pair
        ++(*this);
    }

    Table_Iterator &operator++() {
        while (SlotIndex < (s64) Table.Reserved) {
            SlotIndex++;
            if (Table.OccupancyMask && Table.OccupancyMask[SlotIndex]) {
                break;
            }
        }
        return *this;
    }

    Table_Iterator operator++(int) {
        Table_Iterator pre = *this;
        ++(*this);
        return pre;
    }

    bool operator==(Table_Iterator other) const { return SlotIndex == other.SlotIndex; }
    bool operator!=(Table_Iterator other) const { return !(*this == other); }
    std::tuple<Key, Value> operator*() const { return std::make_tuple(Table.Keys[SlotIndex], Table.Values[SlotIndex]); }
};

template <typename Key, typename Value>
inline Table_Iterator<Key, Value> begin(Table<Key, Value> const &table) {
    return Table_Iterator<Key, Value>(table);
}

template <typename Key, typename Value>
inline Table_Iterator<Key, Value> end(Table<Key, Value> const &table) {
    return Table_Iterator<Key, Value>(table, table.Reserved);
}

template <typename Key, typename Value>
void reserve(Table<Key, Value> &table, size_t size) {
    if (size < table.Reserved) return;

    table.Reserved = size;

    table.OccupancyMask = New<bool>(size, table.Allocator);
    table.Keys = New<Key>(size, table.Allocator);
    table.Values = New<Value>(size, table.Allocator);
    table.Hashes = New<uptr_t>(size, table.Allocator);
}

// Returns -1 if not found
template <typename Key, typename Value>
s32 find_index(Table<Key, Value> const &table, typename Table<Key, Value>::Key_Type const &key, uptr_t hash) {
    if (!table.Reserved) {
        return -1;
    }

	size_t index = hash % table.Reserved;
    while (table.OccupancyMask[index]) {
        if (table.Hashes[index] == hash) {
            if (table.Keys[index] == key) {
                return (s32) index;
            }
        }

        index++;
        if (index >= table.Reserved) {
            index = 0;
        }
    }

    return -1;
}

// Copies the key and the value into the table.
template <typename Key, typename Value>
void put(Table<Key, Value> &table, typename Table<Key, Value>::Key_Type const &key,
         typename Table<Key, Value>::Value_Type const &value) {
    uptr_t hash = Hash<typename Table<Key, Value>::Key_Type>::get(key);
    s32 index = find_index(table, key, hash);
    if (index == -1) {
        if (table.Count >= table.Reserved) {
            expand(table);
        }
        assert(table.Count <= table.Reserved);

		index = (s32) (hash % table.Reserved);
		while (table.OccupancyMask[index]) {
			// Resolve collision
			index++;
			if (index >= table.Reserved) {
				index = 0;
			}
		}

        table.Count++;
    }

    table.OccupancyMask[index] = true;
    table.Keys[index] = key;
    table.Values[index] = value;
    table.Hashes[index] = hash;
}

// Returns a tuple of the value and true if found (false if not found). Doesn't return the value by reference so
// modifying it doesn't update it in the table, use pointers to get around that.
template <typename Key, typename Value>
std::tuple<typename Table<Key, Value>::Value_Type, bool> find(Table<Key, Value> const &table,
                                                              typename Table<Key, Value>::Key_Type const &key) {
    uptr_t hash = Hash<typename Table<Key, Value>::Key_Type>::get(key);

    s32 index = find_index(table, key, hash);
    if (index == -1) {
        return {table.UnfoundValue, false};
    }

    return {table.Values[index], true};
}

template <typename Key, typename Value>
void expand(Table<Key, Value> &table) {
    // I love C++
    // I love C++
    // I love C++
    size_t oldReserved = table.Reserved;
    auto oldOccupancyMask = table.OccupancyMask;
    auto oldKeys = table.Keys;
    auto oldValues = table.Values;
    auto oldHashes = table.Hashes;

    size_t newSize = table.Reserved * 2;
    if (newSize < table.MINIMUM_SIZE) {
        newSize = table.MINIMUM_SIZE;
    }

    reserve(table, newSize);

    for (size_t i = 0; i < oldReserved; i++) {
        if (oldOccupancyMask[i]) {
            put(table, oldKeys[i], oldValues[i]);
        }
    }

    if (oldReserved) {
        Delete(oldOccupancyMask, table.Allocator);
        Delete(oldKeys, table.Allocator);
        Delete(oldValues, table.Allocator);
        Delete(oldHashes, table.Allocator);
    }
}

GU_END_NAMESPACE