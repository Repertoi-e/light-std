#include "../lstd/context.h"
#include "../lstd/storage/string_utils.h"

#define STBI_ASSERT assert

#define STBI_MALLOC(size) operator new(size)
#define STBI_REALLOC(ptr, newSize) LSTD_NAMESPACE::allocator::reallocate(ptr, newSize)
#define STBI_FREE(ptr) delete ptr

#define STBI_WINDOWS_UTF8
#define STBI_NO_STDIO
#define STBI_FAILURE_USERMSG

// The next 4 defines were added by us (we modified stb_image.h)
#define STBI_MEMSET LSTD_NAMESPACE ::fill_memory
#define STBI_MEMCPY LSTD_NAMESPACE ::copy_memory

#define STBI_STRCMP LSTD_NAMESPACE ::compare_c_string_lexicographically
#define STBI_STRNCMP(s1, s2, n) LSTD_NAMESPACE::compare_utf8_lexicographically(s1, n, s2, n)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
