#pragma once

// Maximum size of an allocation we will attemp to request
#define MAX_ALLOCATION_REQUEST 0xFFFFFFFFFFFFFFE0  // Around 16384 PiB

// In debug by default we do some extra checks to catch memory-related bugs.
// See the lstd.memory module for details (or search for DEBUG_MEMORY) and see what extra stuff we do.

#if defined DEBUG || defined DEBUG_OPTIMIZED
#if !defined DEBUG_MEMORY && !defined FORCE_NO_DEBUG_MEMORY
#define DEBUG_MEMORY
#endif
#else
// Don't enable extra info when in Release configuration unless predefined
#endif

#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4273)  // Different linkage
#endif

using size_t = u64;  // We don't support 32 bits, do we?

//
// The date is 4th of September 2021 and for the first time we were able to launch an almost
// non-trivial application that does rendering, UI, graphing math functions, hot-loading dlls...
// without linking with the C/C++ runtime library. That means that it's entirely free of dependencies
// that may change with the compiler version.
//
// In order to get FreeType and imgui to work, I needed to provide definitions for some standard library
// functions (sscanf, strtod, strlen, ... memcmp, ... strncpy, ..., etc..
// They are provided in a file called "platform/windows_no_crt/common_functions.h".
//
// Reading files with fread, fopen is out of the question.
// I modified the code to use the lstd.path module.
//
// Memory functions (malloc, calloc, realloc, free) are provided by default.
// We do this in order to not add YET another way to allocate a block (currently you can
// do that with malloc/free or new/delete, imagine if we added our own allocation function).
// Keeping it malloc is less confusing and error-prone and ... also it's nostalgic.
//
#if defined LSTD_DONT_DEFINE_STD
#include <new>
#else
#if COMPILER == MSVC
// Note: If you get many compile errors (but you have defined LSTD_DONT_DEFINE_STD).
// You probably need to define it globally, because not all headers from this library might see the macro.
inline void *__cdecl operator new(size_t, void *p) noexcept { return p; }
inline void *__cdecl operator new[](size_t, void *p) noexcept { return p; }
#else
inline void *operator new(size_t, void *p) noexcept { return p; }
inline void *operator new[](size_t, void *p) noexcept { return p; }
#endif
#endif

//
// Here we define the memory functions usually provided by the standard library.
// The templated ones are in the lstd namespace in order to not conflict too much.
// They are named the same way for simplicity sake (imagine if we added our own allocation function).
// Keeping it malloc is less confusing and ... also it's nostalgic.
//

extern "C" {
#if COMPILER == MSVC
// Allocates a block of a given size
restrict void *malloc(size_t size);

// Calls malloc with _num_ * _size_ and fills the block with 0s
restrict void *calloc(size_t num, size_t size);

// Attemps to expand _ptr_ to _new_size_.
// If it's not possible, calls malloc with _new_size_ and copies the old contents.
restrict void *realloc(void *ptr, size_t newSize);

// Frees a block allocated by malloc.
void free(void *ptr);
#else
#error Implement.
#endif
}

#if COMPILER == MSVC
#pragma warning(pop)
#endif

using align_val_t = size_t;

#if !defined LSTD_DONT_DEFINE_STD
[[nodiscard]] void *operator new(size_t size);
[[nodiscard]] void *operator new[](size_t size);

[[nodiscard]] void *operator new(size_t size, align_val_t alignment);
[[nodiscard]] void *operator new[](size_t size, align_val_t alignment);
#endif

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;

void operator delete(void *ptr, align_val_t alignment) noexcept;
void operator delete[](void *ptr, align_val_t alignment) noexcept;
