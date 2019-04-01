#pragma once

#include "../string/string_view.hpp"
#include "memory_view.hpp"

#include "stack_dynamic_memory.hpp"

LSTD_BEGIN_NAMESPACE

// StackSize - the amount of bytes used on the stack before dynamically allocating memory
template <size_t StackSize>
struct memory_buffer {
    stack_dynamic_memory<byte, StackSize> Data;
    size_t ByteLength = 0;

    memory_buffer() = default;
    memory_buffer(const memory_view &view) {
        ByteLength = memory.ByteLength;
        Data = stack_dynamic_memory<byte, StackSize>(memory.Data, memory.ByteLength);
        Length = utf8_strlen(Data.get(), ByteLength);
    }

    // Allows negative reversed indexing which begins at the end
    byte &get(s64 index);
    byte get(s64 index) const;

    size_t get_capacity();
    bool has_space_for(size_t count);

    void append(char32_t cp);
    void append_unsafe(char32_t cp);

    void append(const memory_view &memory);
    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_unsafe(const memory_view &view);

    void append_byte(byte b);
    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_byte_unsafe(byte b);

    void append_pointer_and_size(const byte *data, size_t size);
    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_pointer_and_size_unsafe(const byte *data, size_t size);

    // Find the first occurence of _b_
    size_t find(byte b) const;

    // Find the last occurence of _b_
    size_t find_reverse(byte b) const;

    // Set ByteLength to 0
    void clear() { ByteLength = 0; }

    // Free any dynamic memory allocated by this object
    void release() {
        Data.release();
        clear();
    }

    memory_buffer &operator=(const memory_view &view);

    byte *begin() { return Data.get(); }
    byte *end() { return Data.get() + ByteLength; }
    const byte *begin() const { return Data.get(); }
    const byte *end() const { return Data.get() + ByteLength; }

    memory_view get_view() const { return memory_view(Data.get(), ByteLength); }

    // Read/write [] operator
    byte &operator[](s64 index) { return get(index); }
    // Read-only [] operator
    byte operator[](s64 index) const { return get(index); }

    operator bool() const { return ByteLength; }
    operator memory_view() const { return get_view(); }

    void swap(memory_buffer &other) {
        Data.swap(other.Data);
        std::swap(ByteLength, other.ByteLength);
    }
};

template <size_t StackSize>
byte &memory_buffer<StackSize>::get(s64 index) {
    size_t realIndex = translate_index(index, ByteLength);
    assert(realIndex < ByteLength);
    return Data[realIndex];
}

template <size_t StackSize>
byte memory_buffer<StackSize>::get(s64 index) const {
    return get(index);
}

template <size_t StackSize>
size_t memory_buffer<StackSize>::get_capacity() {
    if (Data.is_dynamic()) return Data.Reserved;
    return Data.STACK_ELEMENTS;
}

template <size_t StackSize>
bool memory_buffer<StackSize>::has_space_for(size_t count) {
    size_t needed = ByteLength + count;
    if (Data.is_dynamic()) return needed <= Data.Reserved;
    return needed <= Data.STACK_ELEMENTS;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_byte(byte b) {
    Data.grow(1);
    *(Data.get() + ByteLength++) = b;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append(char32_t cp) {
    size_t cpSize = get_size_of_code_point(cp);
    Data.grow(cpSize);
    encode_code_point(Data.get() + ByteLength, cp);
    ByteLength += cpSize;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_byte_unsafe(byte b) {
    *(Dataget() + ByteLength++) = b;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_unsafe(char32_t cp) {
    encode_code_point(Dataget() + ByteLength, cp);
    ByteLength += get_size_of_code_point(cp);
}

template <size_t StackSize>
void memory_buffer<StackSize>::append(const memory_view &memory) {
    append_pointer_and_size(memory.Data, memory.ByteLength);
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_unsafe(const memory_view &view) {
    append_pointer_and_size_unsafe(memory.Data, memory.ByteLength);
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_pointer_and_size(const byte *data, size_t size) {
    Data.grow(size);
    copy_memory(Data.get() + ByteLength, data, size);
    ByteLength += size;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_pointer_and_size_unsafe(const byte *data, size_t size) {
    copy_memory(Data.get() + ByteLength, data, size);
    ByteLength = ByteLength + size;
}

template <size_t StackSize>
size_t memory_buffer<StackSize>::find(byte b) const {
    for (size_t i = 0; i < ByteLength; ++i) {
        if (get(i) == b) return i;
    }
    return npos;
}

template <size_t StackSize>
size_t memory_buffer<StackSize>::find_reverse(byte b) const {
    for (size_t i = ByteLength - 1; i >= 0; --i) {
        if (get(i) == b) return i;
    }
    return npos;
}

template <size_t StackSize>
memory_buffer<StackSize> &memory_buffer<StackSize>::operator=(const memory_view &view) {
    release();

    memory_buffer(view).swap(*this);
    return *this;
}

LSTD_END_NAMESPACE
