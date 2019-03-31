#include "../../lstd/context.hpp"

#include "../../lstd/memory/memory.hpp"
#include "../../lstd/thread.hpp"

#define STBM_POINTER_SIZE 64

#define STBM_ASSERT assert

#if defined LSTD_NAMESPACE_NAME
#define STBM_MEMSET LSTD_NAMESPACE_NAME ::fill_memory
#else
#define STBM_MEMSET fill_memory
#endif

#if defined LSTD_NAMESPACE_NAME
#define STBM_MUTEX_HANDLE LSTD_NAMESPACE_NAME ::thread::mutex*
#else
#define STBM_MUTEX_HANDLE thread::mutex*
#endif
#define STBM_MUTEX_ACQUIRE(x) x->lock()
#define STBM_MUTEX_RELEASE(x) x->unlock()

#define STBM_ATOMIC_COMPARE_AND_SWAP32

#define STB_MALLOC_IMPLEMENTATION
#include "stb_malloc.hpp"
