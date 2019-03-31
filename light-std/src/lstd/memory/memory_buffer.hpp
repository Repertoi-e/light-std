#pragma once

#include "../string/string_view.hpp"
#include "memory.hpp"
#include "memory_view.hpp"

LSTD_BEGIN_NAMESPACE

// StackSize - the amount of bytes used on the stack before dynamically allocating memory
template <size_t StackSize>
struct memory_buffer {
    static const size_t STACK_BUFFER_SIZE = StackSize;

    byte StackData[StackSize] = {0};

    // A pointer to the data in the buffer
    // This could be either StackData or a dynamically allocated buffer
    byte* Data = StackData;

    // The amount of bytes currently stored in the buffer
    size_t ByteLength = 0;

    // The amount of memory dynamically allocated by the buffer
    // (Always 0 if the buffer uses the stack)
    size_t Reserved = 0;

    allocator_closure Allocator;

    memory_buffer() = default;
    memory_buffer(byte* data, size_t size);
    memory_buffer(const memory_view& view);
    memory_buffer(const memory_buffer& other);
    memory_buffer(memory_buffer&& other);

    // Allows negative reversed indexing which begins at the end
    byte& get(s64 index);
    byte get(s64 index) const;
    void grow(size_t soWeCanHaveAtleastThisMuchFree);
    void reserve(size_t size);
    size_t get_capacity();
    bool has_space_for(size_t count);
    void append_byte(byte b);
    void append(char32_t cp);

    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_byte_unsafe(byte b);
    void append_unsafe(char32_t cp);
    void append(const memory_view& memory);

    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_unsafe(const memory_view& view);
    void append_pointer_and_size(const byte* data, size_t size);

    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_pointer_and_size_unsafe(const byte* data, size_t size);

    // Find the first occurence of _b_
    size_t find(byte b) const;

    // Find the last occurence of _b_
    size_t find_reverse(byte b) const;

    // Set ByteLength to 0
    void clear();

    // Free any dynamic memory allocated by this object
    void release();

    memory_buffer& operator=(const memory_view& view);
    memory_buffer& operator=(const memory_buffer& other);
    memory_buffer& operator=(memory_buffer&& other);

    byte* begin();
    byte* end();
    const byte* begin() const;
    const byte* end() const;

    memory_view get_view() const;

    // Read/write [] operator
    byte& operator[](s64 index);
    // Read-only [] operator
    byte operator[](s64 index) const;

    operator bool() const;
    operator memory_view() const;

    void swap(memory_buffer& other);
};

template <size_t StackSize>
memory_buffer<StackSize>::memory_buffer(byte* data, size_t size) {}

template <size_t StackSize>
memory_buffer<StackSize>::memory_buffer(const memory_view& view) : memory_buffer(view.Data, view.ByteLength) {}

template <size_t StackSize>
memory_buffer<StackSize>::memory_buffer(const memory_buffer& other) {}

template <size_t StackSize>
memory_buffer<StackSize>::memory_buffer(memory_buffer&& other) {
    other.swap(*this);
}

template <size_t StackSize>
byte& memory_buffer<StackSize>::get(s64 index) {
    size_t realIndex = translate_index(index, ByteLength);
    assert(realIndex < ByteLength);
    return Data[realIndex];
}

template <size_t StackSize>
byte memory_buffer<StackSize>::get(s64 index) const {
    return get(index);
}

template <size_t StackSize>
void memory_buffer<StackSize>::grow(size_t soWeCanHaveAtleastThisMuchFree) {
    if (Data && Data != StackData && Reserved) {
        size_t toReserve = Reserved;
        while (toReserve < (ByteLength + soWeCanHaveAtleastThisMuchFree)) {
            toReserve *= 2;
        }
        reserve(toReserve);
    } else {
        reserve(ByteLength + soWeCanHaveAtleastThisMuchFree);
    }
}

template <size_t StackSize>
void memory_buffer<StackSize>::reserve(size_t size) {
    if (Data == StackData) {
        // Return if there is enough space
        if (size <= STACK_BUFFER_SIZE) return;

        // If we are small but we need more size, it's time to convert
        // to a dynamically allocated memory.
        Data = new (&Allocator, ensure_allocator) byte[size];
        copy_memory(Data, StackData, ByteLength);
        Reserved = size;
    } else {
        // Return if there is enough space
        if (size <= Reserved) return;

        Data = resize(Data, size);
        Reserved = size;
    }
}

template <size_t StackSize>
size_t memory_buffer<StackSize>::get_capacity() {
    if (Data != StackData) return Reserved;
    return StackSize;
}

template <size_t StackSize>
bool memory_buffer<StackSize>::has_space_for(size_t count) {
    size_t needed = ByteLength + count;
    if (Data == StackData) {
        return needed <= STACK_BUFFER_SIZE;
    }
    return needed <= Reserved;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_byte(byte b) {
    grow(1);
    *(Data + ByteLength++) = b;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append(char32_t cp) {
    size_t cpSize = get_size_of_code_point(cp);
    grow(cpSize);
    encode_code_point(Data + ByteLength, cp);
    ByteLength += cpSize;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_byte_unsafe(byte b) {
    *(Data + ByteLength++) = b;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_unsafe(char32_t cp) {
    encode_code_point(Data + ByteLength, cp);
    ByteLength += get_size_of_code_point(cp);
}

template <size_t StackSize>
void memory_buffer<StackSize>::append(const memory_view& memory) {
    append_pointer_and_size(memory.Data, memory.ByteLength);
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_unsafe(const memory_view& view) {
    append_pointer_and_size_unsafe(memory.Data, memory.ByteLength);
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_pointer_and_size(const byte* data, size_t size) {
    grow(size);
    copy_memory(Data + ByteLength, data, size);
    ByteLength += size;
}

template <size_t StackSize>
void memory_buffer<StackSize>::append_pointer_and_size_unsafe(const byte* data, size_t size) {
    copy_memory(Data + ByteLength, data, size);
    ByteLength = ByteLength + size;
}

template <size_t StackSize>
size_t memory_buffer<StackSize>::find(byte b) const {
    assert(Data);
    for (size_t i = 0; i < ByteLength; ++i) {
        if (get(i) == b) return i;
    }
    return npos;
}

template <size_t StackSize>
size_t memory_buffer<StackSize>::find_reverse(byte b) const {
    assert(Data);
    for (size_t i = ByteLength - 1; i >= 0; --i) {
        if (get(i) == b) return i;
    }
    return npos;
}

template <size_t StackSize>
void memory_buffer<StackSize>::clear() {
    ByteLength = 0;
}

template <size_t StackSize>
void memory_buffer<StackSize>::release() {
    if (Data && Data != StackData && Reserved) {
        Delete(Data, Reserved, Allocator);
        Data = StackData;

        Reserved = 0;
    }
    clear();
}

template <size_t StackSize>
memory_buffer<StackSize>& memory_buffer<StackSize>::operator=(const memory_view& view) {
    release();

    memory_buffer(view).swap(*this);
    return *this;
}

template <size_t StackSize>
memory_buffer<StackSize>& memory_buffer<StackSize>::operator=(const memory_buffer& other) {
    release();

    memory_buffer(other).swap(*this);
    return *this;
}

template <size_t StackSize>
memory_buffer<StackSize>& memory_buffer<StackSize>::operator=(memory_buffer&& other) {
    release();

    memory_buffer(std::move(other)).swap(*this);
    return *this;
}

template <size_t StackSize>
byte* memory_buffer<StackSize>::begin() {
    return Data;
}

template <size_t StackSize>
byte* memory_buffer<StackSize>::end() {
    return Data + ByteLength;
}

template <size_t StackSize>
const byte* memory_buffer<StackSize>::begin() const {
    return Data;
}

template <size_t StackSize>
const byte* memory_buffer<StackSize>::end() const {
    return Data + ByteLength;
}

template <size_t StackSize>
memory_view memory_buffer<StackSize>::get_view() const {
    return memory_view(Data, ByteLength);
}

template <size_t StackSize>
byte& memory_buffer<StackSize>::operator[](s64 index) {
    return get(index);
}

template <size_t StackSize>
byte memory_buffer<StackSize>::operator[](s64 index) const {
    return get(index);
}

template <size_t StackSize>
memory_buffer<StackSize>::operator bool() const {
    return ByteLength != 0;
}

template <size_t StackSize>
memory_buffer<StackSize>::operator memory_view() const {
    return get_view();
}

template <size_t StackSize>
void memory_buffer<StackSize>::swap(memory_buffer& other) {
    if (Data != StackData && other.Data != other.StackData) {
        std::swap(Data, other.Data);
    } else {
        For(range(STACK_BUFFER_SIZE)) {
            auto temp = StackData[it];
            StackData[it] = other.StackData[it];
            other.StackData[it] = temp;
        }

        bool isOtherSmall = other.Data == other.StackData;
        if (Data != StackData || !isOtherSmall) {
            if (Data == StackData) {
                auto temp = other.Data;
                other.Data = other.StackData;
                Data = temp;
            }
            if (isOtherSmall) {
                auto temp = Data;
                Data = StackData;
                other.Data = temp;
            }
        }
    }
    std::swap(Allocator, other.Allocator);
    std::swap(Reserved, other.Reserved);
    std::swap(ByteLength, other.ByteLength);
}

LSTD_END_NAMESPACE
