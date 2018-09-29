#pragma once

#include "../memory/memory.h"

#include "string.h"

GU_BEGIN_NAMESPACE

inline constexpr size_t STRING_BUILDER_BUFFER_SIZE = 4_KiB;

struct String_Builder {
    struct Buffer {
        char Data[STRING_BUILDER_BUFFER_SIZE];
        size_t Occupied = 0;
        Buffer *Next = null;
    };

    Buffer _BaseBuffer;
    Buffer *CurrentBuffer = &_BaseBuffer;

    // The allocator used for allocating new buffers past the first one (which is stack allocated).
	// This value is null until this object allocates memory or the user sets it manually.
    Allocator_Closure Allocator;

	// Append a string to the builder
	void append(const string &str);

	// Append a non encoded character to a string
	void append(char32_t codePoint);

	// Append a null terminated utf-8 cstyle string.
	void append_cstring(const char *str);

	// Append _size_ bytes of string contained in _data_
	void append_pointer_and_size(const char *data, size_t size);

	// Don't deallocate, just move cursor to 0
	void reset();

	// Free the entire builder
	void release();

    ~String_Builder();
};

// This needs to be in global scope because all other to_string functions are global.
string to_string(String_Builder &builder);

GU_END_NAMESPACE
