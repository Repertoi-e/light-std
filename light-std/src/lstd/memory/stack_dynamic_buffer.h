#pragma once

#include "../internal/context.h"
#include "string.h"

LSTD_BEGIN_NAMESPACE

// A buffer that uses a stack allocated buffer before dynamically allocating.
// StackSize - the amount of bytes used on the stack
template <s64 StackSize>
struct stack_dynamic_buffer : non_copyable, non_movable, non_assignable {
    char StackData[StackSize]{};
    char *Data = StackData;

    s64 Reserved = 0;
    s64 ByteLength = 0;

    stack_dynamic_buffer() = default;

    stack_dynamic_buffer(array_view<char> view) {
        if (sizeof(StackData) > view.size()) {
            reserve(view.size());
        }

        ByteLength = view.size();
        copy_memory(StackData, view.begin(), ByteLength);
    }

    ~stack_dynamic_buffer() { release(); }

    // Makes sure string has reserved enough space for at least n bytes.
    // Note that it may reserve way more than required.
    // Reserves space equal to the next power of two bigger than _size_, starting at 8.
    //
    // ! Reserves only if there is not enough space on the stack
    //
    // Allocates a buffer if the buffer doesn't already point to reserved memory
    // (using the Context's allocator).
    void reserve(s64 target) {
        if (target < sizeof(StackData)) return;
        if (ByteLength + target < Reserved) return;

        target = max<s64>(ceil_pow_of_2(target + ByteLength + 1), 8);

        if (is_owner()) {
            Data = reallocate_array(Data, target);
        } else {
            auto *oldData = Data;
            Data = allocate_array(char, target);
            encode_owner(Data, this);
            if (ByteLength) copy_memory(const_cast<char *>(Data), oldData, ByteLength);
        }
        Reserved = target;
    }

    // Releases the memory allocated by this buffer.
    // If this buffer doesn't own the memory it points to, this function does nothing.
    void release() {
        if (is_owner()) {
            free(Data);
        }
        Data = null;
        ByteLength = Reserved = 0;
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
    void insert(s64 index, array_view<char> view, bool unsafe = false) {
        insert_pointer_and_size(index, view.begin(), view.size(), unsafe);
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
    void remove(s64 begin, s64 end) {
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

    // Append one view to another
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append(array_view<char> view, bool unsafe = false) {
        append_pointer_and_size(view.begin(), view.size(), unsafe);
    }

    // Append _count_ bytes of string contained in _data_
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append_pointer_and_size(const char *data, s64 count, bool unsafe = false) {
        insert_pointer_and_size(ByteLength, data, count, unsafe);
    }

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<stack_dynamic_buffer>(Data) == this; }

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
    operator array_view<char>() { return array_view<char>(Data, Data + ByteLength); }
    explicit operator bool() const { return ByteLength; }

    // Read/write [] operator
    char &operator[](s64 index) { return get(index); }
    // Read-only [] operator
    char operator[](s64 index) const { return get(index); }
};

LSTD_END_NAMESPACE