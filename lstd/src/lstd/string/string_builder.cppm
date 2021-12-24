module;

#include "../common.h"

export module lstd.string_builder;

export import lstd.memory;
export import lstd.string;
export import lstd.context;

LSTD_BEGIN_NAMESPACE

// Good for building large strings because it doesn't have to constantly reallocate.
// Starts with a 1_KiB buffer on the stack, if that fills up, allocates on the heap using _Alloc_.
// We provide an explicit allocator so you can set it in the beginning, before it ever allocates.
// If it's still null when we require a new buffer we use the Context's one.
export {
    struct string_builder {
        static constexpr s64 BUFFER_SIZE = 1_KiB;

        struct buffer {
            char Data[BUFFER_SIZE]{};
            s64 Occupied = 0;
            buffer *Next = null;
        };

        buffer BaseBuffer;
        buffer *CurrentBuffer = null;  // null means BaseBuffer. We don't point to BaseBuffer because pointers to other members are dangerous when copying.

        // Counts how many buffers have been dynamically allocated.
        s64 IndirectionCount = 0;

        // The allocator used for allocating new buffers past the first one (which is stack allocated).
        // This value is null until this object allocates memory (in which case it sets
        // it to the Context's allocator) or the user sets it manually.
        allocator Alloc;
    };

    // Don't free the buffers, just reset cursor
    void reset(string_builder * builder);

    // Free any memory allocated by this object and reset cursor
    void free_buffers(string_builder * builder);

    string_builder::buffer *get_current_buffer(string_builder * builder);

    // Append _size_ bytes from _data_ to the builder
    void append(string_builder * builder, const char *data, s64 size);

    // Append a code point to the builder
    void append(string_builder * builder, code_point cp) {
        char encodedCp[4];
        utf8_encode_cp(encodedCp, cp);
        append(builder, encodedCp, utf8_get_size_of_cp(encodedCp));
    }

    // Append a string to the builder
    void append(string_builder * builder, string str) { append(builder, str.Data, str.Count); }

    // Merges all buffers in one string.
    // Maybe release the buffers as well?
    // The most common use case is builder_to_string() and then free_buffers() -- the builder is not needed anymore.
    [[nodiscard("Leak")]] string builder_to_string(string_builder * builder);
}

void reset(string_builder *builder) {
    builder->CurrentBuffer = null;  // null means BaseBuffer

    auto *b = &builder->BaseBuffer;
    while (b) {
        b->Occupied = 0;

        b = b->Next;
    }
}

void append(string_builder *builder, const char *data, s64 size) {
    auto *currentBuffer = get_current_buffer(builder);

    s64 availableSpace = builder->BUFFER_SIZE - currentBuffer->Occupied;
    if (availableSpace >= size) {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, size);
        currentBuffer->Occupied += size;
    } else {
        copy_memory(currentBuffer->Data + currentBuffer->Occupied, data, availableSpace);
        currentBuffer->Occupied += availableSpace;

        // If the entire string doesn't fit inside the available space,
        // allocate the next buffer and continue appending.
        if (!builder->Alloc) builder->Alloc = Context.Alloc;
        auto *b = malloc<string_builder::buffer>({.Alloc = builder->Alloc});

        currentBuffer->Next    = b;
        builder->CurrentBuffer = b;

        builder->IndirectionCount++;

        append(builder, data + availableSpace, size - availableSpace);
    }
}

[[nodiscard("Leak")]] string builder_to_string(string_builder *builder) {
    string result;
    make_dynamic(&result, (builder->IndirectionCount + 1) * builder->BUFFER_SIZE);

    auto *b = &builder->BaseBuffer;
    while (b) {
        string_append(&result, b->Data, b->Occupied);
        b = b->Next;
    }
    return result;
}

string_builder::buffer *get_current_buffer(string_builder *builder) {
    if (builder->CurrentBuffer == null) return &builder->BaseBuffer;
    return builder->CurrentBuffer;
}

void free_buffers(string_builder *builder) {
    // We don't need to free the base buffer, it is allocated on the stack
    auto *b = builder->BaseBuffer.Next;
    while (b) {
        auto *old = b;

        b = b->Next;
        free(old);
    }

    builder->CurrentBuffer       = null;  // null means BaseBuffer
    builder->BaseBuffer.Occupied = 0;
}

LSTD_END_NAMESPACE
