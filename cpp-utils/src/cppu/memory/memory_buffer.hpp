#pragma once

#include "memory.hpp"
#include "memory_view.hpp"

CPPU_BEGIN_NAMESPACE

// StackSize - the amount of bytes used on the stack before dynamically allocating memory
template <size_t StackSize>
struct Memory_Buffer {
    static const size_t STACK_BUFFER_SIZE = StackSize;

    byte StackData[StackSize] = {0};

    // A pointer to the data in the buffer
    // This could be either StackData or a dynamically allocated buffer
    byte *Data = StackData;

    // The amount of bytes currently stored in the buffer
    size_t ByteLength = 0;

    // The amount of memory dynamically allocated by the buffer
    // (Always 0 if the buffer uses the stack)
    size_t Reserved = 0;

    Allocator_Closure Allocator;

    Memory_Buffer() {}
    Memory_Buffer(byte *data, size_t size) {}
    Memory_Buffer(const Memory_View &view) : Memory_Buffer(view.Data, view.ByteLength) {}
    Memory_Buffer(const Memory_Buffer &other) {}
    Memory_Buffer(Memory_Buffer &&other) { other.swap(*this); }

    // Allows negative reversed indexing which begins at the end
    byte &get(s64 index) {
        size_t realIndex = translate_index(index, ByteLength);
        assert(realIndex < ByteLength);
        return Data[realIndex];
    }

    byte get(s64 index) const { return get(index); }

    void grow(size_t soWeCanHaveAtleastThisMuchFree) {
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

    void reserve(size_t size) {
        if (Data == StackData) {
            // Return if there is enough space
            if (size <= STACK_BUFFER_SIZE) return;

            // If we are small but we need more size, it's time to convert
            // to a dynamically allocated memory.
            Data = New_and_ensure_allocator<byte>(size, Allocator);
            copy_memory(Data, StackData, ByteLength);
            Reserved = size;
        } else {
            // Return if there is enough space
            if (size <= Reserved) return;

            Data = Resize_and_ensure_allocator(Data, Reserved, size, Allocator);
            Reserved = size;
        }
    }

    size_t get_capacity() {
        if (Data != StackData) return Reserved;
        return StackSize;
    }

    bool has_space_for(size_t count) {
        size_t needed = ByteLength + count;
        if (Data == StackData) {
            return needed <= STACK_BUFFER_SIZE;
        }
        return needed <= Reserved;
    }

    void append(byte b) {
        grow(1);
        *(Data + ByteLength++) = b;
    }

    void append_codepoint(char32_t cp) {
        size_t cpSize = get_size_of_code_point(cp);
        grow(cpSize);
        encode_code_point((char *) (Data + ByteLength), cp);
        ByteLength += cpSize;
    }

    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_unsafe(byte b) { *(Data + ByteLength++) = b; }

    void append_cstring(const char *data) { append_pointer_and_size((const byte *) data, cstring_strlen(data)); }
    void append_cstring_unsafe(const Memory_Buffer &other) {
        append_pointer_and_size_unsafe(data, cstring_strlen(data));
    }

    template <size_t S>
    void append(const Memory_Buffer<S> &other) { append_pointer_and_size(other.Data, other.ByteLength); }
    void append(const Memory_View &view) { append_pointer_and_size(view.Data, view.ByteLength); }

    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    template <size_t S>
    void append_unsafe(const Memory_Buffer<S> &other) {
        append_pointer_and_size_unsafe(other.Data, other.ByteLength);
    }
    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_unsafe(const Memory_View &view) { append_pointer_and_size_unsafe(view.Data, view.ByteLength); }

    void append_pointer_and_size(const byte *data, size_t size) {
        grow(size);
        copy_memory(Data + ByteLength, data, size);
        ByteLength += size;
    }

    // Append without checking if there is enough space
    // Useful if you want every last bit of performance
    void append_pointer_and_size_unsafe(const byte *data, size_t size) {
        copy_memory(Data + ByteLength, data, size);
        ByteLength = ByteLength + size;
    }

    // Find the first occurence of _b_
    size_t find(byte b) const {
        assert(Data);
        for (size_t i = 0; i < ByteLength; ++i)
            if (get(i) == b) return i;
        return npos;
    }

    // Find the last occurence of _b_
    size_t find_last(byte b) const {
        assert(Data);
        for (size_t i = ByteLength - 1; i >= 0; --i)
            if (get(i) == b) return i;
        return npos;
    }

    // Set ByteLength to 0
    void clear() { ByteLength = 0; }

    // Free any dynamic memory allocated by this object
    void release() {
        if (Data && Data != StackData && Reserved) {
            Delete(Data, Reserved, Allocator);
            Data = StackData;

            Reserved = 0;
        }
        clear();
    }

    Memory_Buffer &operator=(const Memory_View &view) {
        release();

        Memory_Buffer(view).swap(*this);
        return *this;
    }

    Memory_Buffer &operator=(const Memory_Buffer &other) {
        release();

        Memory_Buffer(other).swap(*this);
        return *this;
    }

    Memory_Buffer &operator=(Memory_Buffer &&other) {
        release();

        Memory_Buffer(std::move(other)).swap(*this);
        return *this;
    }

    constexpr byte *begin() { return Data; }
    constexpr byte *end() { return Data + ByteLength; }
    constexpr const byte *begin() const { return Data; }
    constexpr const byte *end() const { return Data + ByteLength; }

    inline Memory_View get_view() { return Memory_View(Data, ByteLength); }

    // Read/write [] operator
    byte &operator[](s64 index) { return get(index); }
    // Read-only [] operator
    byte operator[](s64 index) const { return get(index); }

    operator bool() const { return ByteLength != 0; }

    void swap(Memory_Buffer &other) {
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
};

CPPU_END_NAMESPACE
