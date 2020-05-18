#include "../../lstd/internal/context.h"
#include "../../lstd/memory/string_utils.h"

#define STBTT_malloc(x, u) ((void) (u), operator new(x))
#define STBTT_free(x, u) ((void) (u), delete x)
#define STBTT_assert(x) assert(x)
#define STBTT_strlen(x) c_string_length(x)

#define STBTT_memcpy LSTD_NAMESPACE ::copy_memory
#define STBTT_memset LSTD_NAMESPACE ::fill_memory

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

