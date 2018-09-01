#pragma once

#include "../context.h"
#include "memory.h"

#include "hash.h"

#include <iterator>
#include <tuple>

GU_BEGIN_NAMESPACE

template <typename Key_Type, typename Value_Type>
struct Table_Slots {
    size_t Size = 0;
    bool *OccupancyMask = 0;
    Key_Type *Keys = 0;
    Value_Type *Values = 0;
    u32 *Hashes = 0;

    Table_Slots() {}

    Table_Slots(size_t size) {
        Size = size;
        OccupancyMask = New<bool>(size);
        Keys = New<Key_Type>(size);
        Values = New<Value_Type>(size);
        Hashes = New<u32>(size);
    }

    ~Table_Slots() {
        if (Size) {
            Delete(OccupancyMask);
            Delete(Keys);
            Delete(Values);
            Delete(Hashes);
        }
    }
};

template <typename Key, typename Value>
struct Table {
    using Key_Type = Key;
    using Value_Type = Value;

    static const size_t MINIMUM_SIZE = 32;
    size_t Count = 0, Reserved = 0;

    // The allocator used for expanding the table.
    // If we pass a null allocator to a New/Delete wrapper it uses the context's
    // one automatically.
    Allocator_Closure Allocator;

    Table_Slots<Key_Type, Value_Type> Slots;

    Value_Type UnfoundValue = Value_Type();
};

template <typename Key, typename Value>
struct Table_Iterator : public std::iterator<std::forward_iterator_tag, std::tuple<Key const &, Value &>> {
    Table<Key, Value> const &Table;
    s64 SlotIndex = -1;

    explicit Table_Iterator(::Table<Key, Value> const &table, s64 index = -1) : Table(table), SlotIndex(index) {
        // Find the first pair
        ++(*this);
    }

    Table_Iterator &operator++() {
        while (SlotIndex < (s64) Table.Slots.Size) {
            SlotIndex++;
            bool *occupancyMask = Table.Slots.OccupancyMask;
            if (occupancyMask && occupancyMask[SlotIndex]) {
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
    std::tuple<Key, Value> operator*() const {
        auto &slots = Table.Slots;
        return std::make_tuple(slots.Keys[SlotIndex], slots.Values[SlotIndex]);
    }
};

template <typename Key, typename Value>
inline Table_Iterator<Key, Value> begin(Table<Key, Value> const &table) {
    return Table_Iterator<Key, Value>(table);
}

template <typename Key, typename Value>
inline Table_Iterator<Key, Value> end(Table<Key, Value> const &table) {
    return Table_Iterator<Key, Value>(table, table.Slots.Size);
}

template <typename Key, typename Value>
void reserve(Table<Key, Value> &table, size_t size) {
    table.Reserved = size;

    auto newContext = __context;
    if (table.Allocator.Function) newContext.Allocator = table.Allocator;
    {
        PUSH_CONTEXT(newContext);

        // In order to avoid calling Table_Slots destructor on copy which would free the pointers and shallow copy them
        // to table.Slots and in order to avoid having to write copy move assign rvalue gvalue constructors which would
        // take 100000000 lines of code, we use placement new to initialize our slots struct.
        using SlotsType = Table_Slots<typename Table<Key, Value>::Key_Type, typename Table<Key, Value>::Value_Type>;

        new (&table.Slots) SlotsType(size);
    }
}

// Returns -1 if not found
template <typename Key, typename Value>
s32 find_index(Table<Key, Value> const &table, typename Table<Key, Value>::Key_Type const &key, u32 hash) {
    if (!table.Reserved) {
        return -1;
    }

    u32 slot = hash % table.Reserved;
    s32 index = slot;

    auto &slots = table.Slots;
    while (slots.OccupancyMask[index]) {
        if (slots.Hashes[index] == hash) {
            if (slots.Keys[index] == key) {
                return index;
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
    u32 hash = Hash<typename Table<Key, Value>::Key_Type>::get(key);
    s32 index = find_index(table, key, hash);
    if (index == -1) {
        if (table.Count * 2 >= table.Reserved) {
            expand(table);
        }
        assert(table.Count <= table.Reserved);

        index = hash % table.Reserved;
        while (table.Slots.OccupancyMask[index]) {
            index++;
            if (index >= table.Reserved) {
                index = 0;
            }
        }

        table.Count++;
    }

    auto &slots = table.Slots;
    slots.OccupancyMask[index] = true;
    slots.Keys[index] = key;
    slots.Values[index] = value;
    slots.Hashes[index] = hash;
}

// Returns a tuple of the value and true if found (false if not found). Doesn't return the value by reference so
// modifying it doesn't update it in the table, use pointers to get around that.
template <typename Key, typename Value>
std::tuple<typename Table<Key, Value>::Value_Type, bool> find(Table<Key, Value> const &table,
                                                              typename Table<Key, Value>::Key_Type const &key) {
    u32 hash = Hash<typename Table<Key, Value>::Key_Type>::get(key);

    s32 index = find_index(table, key, hash);
    if (index == -1) {
        return {table.UnfoundValue, false};
    }

    return {table.Slots.Values[index], true};
}

template <typename Key, typename Value>
void expand(Table<Key, Value> &table) {
    auto newContext = __context;
    if (table.Allocator.Function) newContext.Allocator = table.Allocator;
    {
        PUSH_CONTEXT(newContext);

        // Will get freed at the end of the scope by the magic power of the
        // destructor
        auto oldSlots = table.Slots;

        size_t newSize = table.Reserved * 2;
        if (newSize < table.MINIMUM_SIZE) {
            newSize = table.MINIMUM_SIZE;
        }

        reserve(table, newSize);

        for (size_t i = 0; i < oldSlots.Size; i++) {
            if (oldSlots.OccupancyMask[i]) {
                put(table, oldSlots.Keys[i], oldSlots.Values[i]);
            }
        }
    }
}

GU_END_NAMESPACE