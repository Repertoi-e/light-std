#pragma once

#include "scalar_types.h"

// Maximum size of an allocation we will attemp to request
#define MAX_ALLOCATION_REQUEST 0xFFFFFFFFFFFFFFE0  // Around 16384 PiB

// @TODO @Critical Currently hot loading DLLs causes all sorts of failures when using DEBUG_MEMORY
// To fix them we need to restructure how we save the list of global allocations...
// For now don't do DEBUG_MEMORY.
#define FORCE_NO_DEBUG_MEMORY

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
// :STANDARDLIBRARYISBANNED:
//
// The date is 4th of September 2021 and for the first time we were able to launch an almost
// non-trivial application that does rendering, UI, graphing math functions, hot-loading dlls...
// without linking with the C/C++ runtime library. That means that it's entirely free of dependencies
// that may change with the compiler version.
//
// Moving forward, the policy will be that any piece of code that does something other than interfacing
// with the OS should be set in stone in a file in the project you are working on. That means that
// including any C++ standard library header (also C standard library headers which provides functionality
// is banned (stuff like stdint.h is fine though). In general, expect errors if you include such a header.
//
// We used to support using this library alongside the standard library, that's way there is the
// LSTD_DONT_DEFINE_STD macro, which fixes the errors you might get from including both.
// Now we don't try to guarantee stuff will work flawlessly, however I will leave it here
// incase it helps and you can't avoid not including the standard library.
//
// In order to get FreeType and imgui to work, I needed to provide definitions for some standard library
// functions (sscanf, strtod, strlen, ... memcmp, ... strncpy, ..., etc..
// They are provided in a file called "common_standard_library_functions.h".
// To make life easier I won't change their cryptic names so they can be ready for
// use if you link with a library which uses them.
//
// Reading files with fread, fopen is out of the question.
// I modified the code to use the lstd.path module.
//
// Memory functions (malloc, calloc, realloc, free) are provided by default.
// We do this in order to not add YET another way to allocate a block (currently you can
// do that with malloc/free or new/delete, imagine if we added our own allocation function).
// Keeping it malloc is less confusing and error-prone and ... also it's nostalgic.
//
//                          - Dimitar Sotirov, 4th September 2021
//
// Old info, but still valid:
//
// :AvoidSTDs:
// Normally <new> defines the placement new operator but if we avoid using headers from the C++ STD
// we define our own implementation here.
//
// By default we avoid STDs (like in real life) but if e.g. a library relies on it we would get definition errors.
// In general this library can work WITH or WITHOUT the normal standard library.
// Note: But you must tell us with a macro: LSTD_DONT_DEFINE_STD.
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
void *malloc(size_t size);

// Calls malloc with _num_ * _size_ and fills the block with 0s
void *calloc(size_t num, size_t size);

// Attemps to expand _ptr_ to _new_size_.
// If it's not possible, calls malloc with _new_size_ and copies the old contents.
void *realloc(void *ptr, size_t newSize);

// Frees a block allocated by malloc.
void free(void *ptr);
#else
#error
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
