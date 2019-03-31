#pragma once

#include "../common.hpp"

#include "range.hpp"

LSTD_BEGIN_NAMESPACE

enum class allocator_mode { ALLOCATE = 0, RESIZE, FREE, FREE_ALL };

// This specifies what the signature of each allocation function should look like.
//
// _mode_ is what we are doing currently, allocating, resizing, freeing a
// block or freeing everything
//
// _allocatorData_ is a pointer to the allocator
// that is being used
//
// _size_ is the size of the allocation
//
// _oldMemory_ is used only when resizing
// _oldSize_ is the old size of memory block, used only when resizing
//
// and the last uptr_t is reserved for user data.
typedef void *(*allocator_func)(allocator_mode mode, void *data, size_t size, void *oldMemory, size_t oldSize, uptr_t);

struct allocator_closure {
    allocator_func Function = null;
    void *Data = null;

    operator bool() const { return Function != null; }
};

// The default allocator (malloc), requests memory from the OS
extern allocator_func g_DefaultAllocator;

#define MALLOC \
    allocator_closure { g_DefaultAllocator, null }

LSTD_END_NAMESPACE
