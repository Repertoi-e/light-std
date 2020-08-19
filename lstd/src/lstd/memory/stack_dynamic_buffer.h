#pragma once

#include "../internal/context.h"
#include "array.h"
#include "string.h"

LSTD_BEGIN_NAMESPACE

// A buffer that uses a stack allocated buffer before dynamically allocating.
// StackSize - the amount of bytes used on the stack
template <s64 StackSize>
struct stack_dynamic_buffer : non_copyable, non_movable, non_assignable {
    char StackData[StackSize]{};
    char *Data = StackData;

    s64 Allocated = 0;
    s64 ByteLength = 0;

    stack_dynamic_buffer() {}

    stack_dynamic_buffer(const array<char> &arr) {
        if (sizeof(StackData) > arr.Count) {
            reserve(arr.Count);
        }

        ByteLength = arr.Count;
        copy_memory(StackData, arr.Data, ByteLength);
    }

    // We no longer use destructors for deallocation.
    // ~stack_dynamic_buffer() { release(); }

    // Makes sure buffer has reserved enough space for at least n bytes.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at 8.
    //
    // ! Reserves only if there is not enough space on the stack
    void reserve(s64 target) {
        if (target < sizeof(StackData)) return;
        if (ByteLength + target < Allocated) return;

        target = max<s64>(ceil_pow_of_2(target + ByteLength + 1), 8);

        if (Allocated) {
            Data = reallocate_array(Data, target);
        } else {
            auto *oldData = Data;
            Data = allocate_array(char, target);
            // We removed the ownership system.
            // encode_owner(Data, this);
            if (ByteLength) copy_memory(const_cast<char *>(Data), oldData, ByteLength);
        }
        Allocated = target;
    }

    // Releases the memory allocated by this buffer.
    // If this buffer doesn't own the memory it points to, this function does nothing.
    void release() {
        if (Allocated) free(Data);
        Data = null;
        ByteLength = Allocated = 0;
    }

    // Don't free the buffer, just move cursor to 0
    void reset() { ByteLength = 0; }

    // Allows negative reversed indexing which begins at the end
    char &get(s64 index) { return Data[translate_index(index, ByteLength)]; }
    char get(s64 index) const { return Data[translate_index(index, ByteLength)]; }

    // Sets the _index_'th byte in the string
    void set(s64 index, char b) { Data[translate_index(index, ByteLength)] = b; }

    // Insert a byte at a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert(s64 index, char b, bool unsafe = false) {
        if (!unsafe) reserve(ByteLength + 1);

        auto *target = Data + translate_index(index, ByteLength, true);
        u64 offset = (u64)(target - Data);
        copy_memory((char *) Data + offset + 1, target, ByteLength - (target - Data));
        *target = b;

        ++ByteLength;
    }

    // Insert data after a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert_array(s64 index, const array<char> &arr, bool unsafe = false) {
        insert_pointer_and_size(index, view.Data, view.Count, unsafe);
    }

    // Insert data after a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert_array(s64 index, const initializer_list<char> &list, bool unsafe = false) {
        insert_pointer_and_size(index, list.begin(), list.size(), unsafe);
    }

    // Insert a buffer of bytes at a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert_pointer_and_size(s64 index, const char *data, s64 count, bool unsafe = false) {
        if (!unsafe) reserve(ByteLength + 1);

        auto *target = Data + translate_index(index, ByteLength, true);
        u64 offset = (u64)(target - Data);
        copy_memory((char *) Data + offset + count, target, ByteLength - (target - Data));

        copy_memory(target, data, count);

        ByteLength += count;
    }

    // Remove byte at specified index
    void remove(s64 index) {
        auto *targetBegin = Data + translate_index(begin, ByteLength);

        u64 offset = (u64)(targetBegin - Data);
        copy_memory((char *) Data + offset, targetBegin + 1, ByteLength - offset - 1);

        --ByteLength;
    }

    // Remove a range of bytes.
    // [begin, end)
    void remove_range(s64 begin, s64 end) {
        auto *targetBegin = Data + translate_index(begin, ByteLength);
        auto *targetEnd = Data + translate_index(begin, ByteLength, true);

        assert(targetEnd > targetBegin);

        s64 bytes = targetEnd - targetBegin;
        u64 offset = (u64)(targetBegin - Data);
        copy_memory((char *) Data + offset, targetEnd, ByteLength - offset - bytes);

        ByteLength -= bytes;
    }

    // Append a byte
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append(char b, bool unsafe = false) { insert(ByteLength, b, unsafe); }

    // Append _count_ bytes of string contained in _data_
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append_pointer_and_size(const char *data, s64 count, bool unsafe = false) {
        insert_pointer_and_size(ByteLength, data, count, unsafe);
    }

    // Append one view to another
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append_array(const array<char> &view, bool unsafe = false) {
        append_pointer_and_size(view.Data, view.Count, unsafe);
    }

    // Append a list
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append_list(const initializer_list<char> &list, bool unsafe = false) {
        append_pointer_and_size(list.begin(), list.size(), unsafe);
    }

    //
    // Iterator:
    //
    using iterator = char *;
    using const_iterator = const char *;

    iterator begin() { return Data; }
    iterator end() { return Data + ByteLength; }

    const_iterator begin() const { return Data; }
    const_iterator end() const { return Data + ByteLength; }

    //
    // Operators:
    //
    operator array<char>() { return array<char>(Data, ByteLength); }
    explicit operator bool() const { return ByteLength; }

    // Read/write [] operator
    char &operator[](s64 index) { return get(index); }
    // Read-only [] operator
    char operator[](s64 index) const { return get(index); }
};

LSTD_END_NAMESPACE