#pragma once

#include "string.h"

LSTD_BEGIN_NAMESPACE

// A buffer that uses a stack allocated buffer before dynamically allocating.
// StackSize - the amount of bytes used on the stack
template <size_t StackSize>
struct stack_dynamic_buffer {
    byte StackData[StackSize]{};
    byte *Data = StackData;

    size_t Reserved = 0;
    size_t ByteLength = 0;

    stack_dynamic_buffer() = default;

    stack_dynamic_buffer(array_view<byte> view) {
        if (sizeof(StackData) > view.size()) {
            reserve();
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
    // (using the Context's allocator by default).
    //
    // If you want to prepare an allocator, call reserve() with _size_ == _StackSize_ + 1
    //
    // For robustness, this function asserts if you pass an allocator, but the buffer has already
    // reserved a buffer with a *different* allocator.
    //
    // If the buffer points to reserved memory but doesn't own it, this function asserts.
    void reserve(size_t size, allocator alloc = {null, null}) {
        if (size < sizeof(StackData)) return;
        if (size < Reserved) return;

        if (!Reserved && size < ByteLength) {
            size += ByteLength;
        }

        size_t reserveTarget = 8;
        while (reserveTarget < size) {
            reserveTarget *= 2;
        }

        if (Reserved) {
            assert(is_owner() && "Cannot resize a buffer that isn't owned by this object.");

            auto *actualData = const_cast<byte *>(Data) - POINTER_SIZE;

            if (alloc) {
                auto *header = (allocation_header *) actualData - 1;
                assert(
                    alloc.Function == header->AllocatorFunction && alloc.Context == header->AllocatorContext &&
                    "Calling reserve() on an object that already has reserved a buffer but with a different allocator. "
                    "Call with null allocator to avoid that.");
            }

            Data = (byte *) allocator::reallocate(actualData, reserveTarget + POINTER_SIZE) + POINTER_SIZE;
        } else {
            auto *oldData = Data;
            Data = encode_owner(new (alloc) byte[reserveTarget + POINTER_SIZE], this);
            if (ByteLength) copy_memory(const_cast<byte *>(Data), oldData, ByteLength);
        }
        Reserved = reserveTarget;
    }

    // Releases the memory allocated by this buffer.
    // If this buffer doesn't own the memory it points to, this function does nothing.
    void release() {
        if (is_owner()) {
            delete[] (Data - POINTER_SIZE);
            Data = null;
            ByteLength = Reserved = 0;
        }
    }

    // Allows negative reversed indexing which begins at the end
    byte &get(s64 index) { return Data[translate_index(index, ByteLength)]; }
    byte get(s64 index) const { Data[translate_index(index, ByteLength)]; }

    // Sets the _index_'th byte in the string
    void set(s64 index, byte b) { Data[translate_index(index, ByteLength)] = b; }

    // Insert a byte at a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert(s64 index, byte b, bool unsafe = false) {
        if (!unsafe) reserve(ByteLength + 1);

        auto *target = Data + translate_index(index, ByteLength, true);
        uptr_t offset = (uptr_t)(target - Data);
        copy_memory((byte *) Data + offset + 1, target, ByteLength - (target - Data));
        *target = b;

        ++ByteLength;
    }

    // Insert data after a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert(s64 index, array_view<byte> view, bool unsafe = false) {
        insert_pointer_and_size(index, view.begin(), view.size(), unsafe);
    }

    // Insert a buffer of bytes at a specified index
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void insert_pointer_and_size(s64 index, const byte *data, size_t count, bool unsafe = false) {
        if (!unsafe) reserve(ByteLength + 1);

        auto *target = Data + translate_index(index, ByteLength, true);
        uptr_t offset = (uptr_t)(target - Data);
        copy_memory((byte *) Data + offset + count, target, ByteLength - (target - Data));

        copy_memory(target, data, count);

        ByteLength += count;
    }

    // Remove byte at specified index
    void remove(s64 index) {
        auto *targetBegin = Data + translate_index(begin, ByteLength);

        uptr_t offset = (uptr_t)(targetBegin - Data);
        copy_memory((byte *) Data + offset, targetBegin + 1, ByteLength - offset - 1);

        --ByteLength;
    }

    // Remove a range of bytes.
    // [begin, end)
    void remove(s64 begin, s64 end) {
        auto *targetBegin = Data + translate_index(begin, ByteLength);
        auto *targetEnd = Data + translate_index(begin, ByteLength, true);

        assert(targetEnd > targetBegin);

        size_t bytes = targetEnd - targetBegin;
        uptr_t offset = (uptr_t)(targetBegin - Data);
        copy_memory((byte *) Data + offset, targetEnd, ByteLength - offset - bytes);

        ByteLength -= bytes;
    }

    // Append a byte
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append(byte b, bool unsafe = false) { insert(ByteLength, b, unsafe); }

    // Append one view to another
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append(array_view<byte> view, bool unsafe = false) {
        append_pointer_and_size(view.begin(), view.size(), unsafe);
    }

    // Append _count_ bytes of string contained in _data_
    // _unsafe_ - avoid reserving (may attempt to write past buffer if there is not enough space!)
    void append_pointer_and_size(const byte *data, size_t count, bool unsafe = false) {
        insert_pointer_and_size(ByteLength, data, count, unsafe);
    }

    // Returns true if this object has any memory allocated by itself
    bool is_owner() const { return Reserved && decode_owner<stack_dynamic_buffer>(Data) == this; }

    //
    // Iterator:
    //
    using iterator = byte *;
    using const_iterator = const byte *;

    iterator begin() { return Data; }
    iterator end() { return Data + ByteLength; }

    const_iterator begin() const { return Data; }
    const_iterator end() const { return Data + ByteLength; }

    //
    // Operators:
    //
    operator array_view<byte>() { return array_view<byte>(Data, Data + ByteLength); }
    explicit operator bool() const { return ByteLength; }

    // Read/write [] operator
    byte &operator[](s64 index) { return get(index); }
    // Read-only [] operator
    byte operator[](s64 index) const { return get(index); }
};

LSTD_END_NAMESPACE