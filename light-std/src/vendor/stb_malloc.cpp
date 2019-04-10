#include "../lstd/common.h"

#define STBM_ASSERT assert
#define STBM_MEMSET LSTD_NAMESPACE ::fill_memory

// @Thread
// #define STBM_MUTEX_HANDLE LSTD_NAMESPACE_NAME ::thread::mutex*
// #define STBM_MUTEX_ACQUIRE(x) x->lock()
// #define STBM_MUTEX_RELEASE(x) x->unlock()

#define STBM_ATOMIC_COMPARE_AND_SWAP32

#define STB_MALLOC_IMPLEMENTATION
#include "stb_malloc.hpp"
