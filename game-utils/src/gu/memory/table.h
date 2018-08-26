#pragma once

#include "../context.h"
#include "memory.h"

#include "hash.h"

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
void reserve(Table<Key, Value> &table, size_t size) {
    table.Reserved = size;
    table.Allocator = table.Allocator;

    auto newContext = *__context;
    if (table.Allocator.Function) newContext.Allocator = table.Allocator;
    {
        PUSH_CONTEXT(newContext);

        table.Slots = Table_Slots<typename Table<Key, Value>::Key_Type, typename Table<Key, Value>::Value_Type>(size);
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

template <typename Key, typename Value>
std::tuple<typename Table<Key, Value>::Value_Type, bool> find(Table<Key, Value> table,
                                                              typename Table<Key, Value>::Key_Type const &key) {
    u32 hash = Hash<typename Table<Key, Value>::Key_Type>::get(key);
    ;
    s32 index = find_index(table, key, hash);
    if (index == -1) {
        return {table.UnfoundValue, false};
    }

    return {table.Slots.Values[index], true};
}

template <typename Key, typename Value>
void expand(Table<Key, Value> &table) {
    auto newContext = *__context;
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