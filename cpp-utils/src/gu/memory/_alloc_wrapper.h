#pragma once

#include "memory.h"

// This procedure allocates memory with specified allocator.
// If that allocator is null, uses the CONTEXT_ALLOCATOR
// and **sets** the passed Allocator reference to the one used.

// This file is meant to be used only inside the library (hense the _ in the name)
// But maybe I'll think about putting these in memory.h in the future if they
// prove to be good design.

template <typename T>
inline T *New_And_Set_Allocator(Allocator_Closure &allocator) {
	if (!allocator) allocator = CONTEXT_ALLOC;
	return New<T>(allocator);
}

template <typename T>
inline T *New_And_Set_Allocator(size_t count, Allocator_Closure &allocator) {
	if (!allocator) allocator = CONTEXT_ALLOC;
	return New<T>(count, allocator);
}

template <typename T>
inline T *Resize_And_Set_Allocator(T *memory, size_t oldSize, size_t newSize, Allocator_Closure &allocator) {
	if (!allocator) allocator = CONTEXT_ALLOC;
	return Resize(memory, oldSize, newSize, allocator);
}