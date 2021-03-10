/* ----------------------------------------------------------------------------
Copyright (c) 2018, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

#if !defined(MI_IN_ALLOC_C)
#error "this file should be included from 'alloc.c' (so aliases can work)"
#endif

#if defined(MI_MALLOC_OVERRIDE) && defined(_WIN32) && !(defined(MI_SHARED_LIB) && defined(_DLL))
#error "It is only possible to override "malloc" on Windows when building as a DLL (and linking the C runtime as a DLL)"
#endif

#if defined(MI_MALLOC_OVERRIDE) && !(defined(_WIN32)) // || (defined(__MACH__) && !defined(MI_INTERPOSE)))

// ------------------------------------------------------
// Override system malloc
// ------------------------------------------------------

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACH__)
  // use aliasing to alias the exported function to one of our `mi_` functions
  #if (defined(__GNUC__) && __GNUC__ >= 9)
    #define MI_FORWARD(fun)      __attribute__((alias(#fun), used, visibility("default"), copy(fun)))
  #else
    #define MI_FORWARD(fun)      __attribute__((alias(#fun), used, visibility("default")))
  #endif
  #define MI_FORWARD1(fun,x)      MI_FORWARD(fun)
  #define MI_FORWARD2(fun,x,y)    MI_FORWARD(fun)
  #define MI_FORWARD3(fun,x,y,z)  MI_FORWARD(fun)
  #define MI_FORWARD0(fun,x)      MI_FORWARD(fun)
  #define MI_FORWARD02(fun,x,y)   MI_FORWARD(fun)
#else
  // use forwarding by calling our `mi_` function
  #define MI_FORWARD1(fun,x)      { return fun(x); }
  #define MI_FORWARD2(fun,x,y)    { return fun(x,y); }
  #define MI_FORWARD3(fun,x,y,z)  { return fun(x,y,z); }
  #define MI_FORWARD0(fun,x)      { fun(x); }
  #define MI_FORWARD02(fun,x,y)   { fun(x,y); }
#endif

#if defined(__APPLE__) && defined(MI_SHARED_LIB_EXPORT) && defined(MI_INTERPOSE)
  // use interposing so `DYLD_INSERT_LIBRARIES` works without `DYLD_FORCE_FLAT_NAMESPACE=1`
  // See: <https://books.google.com/books?id=K8vUkpOXhN4C&pg=PA73>
  struct mi_interpose_s {
    const void* replacement;
    const void* target;
  };
  #define MI_INTERPOSE_FUN(oldfun,newfun) { (const void*)&newfun, (const void*)&oldfun }
  #define MI_INTERPOSE_MI(fun)            MI_INTERPOSE_FUN(fun,mi_##fun)
  __attribute__((used)) static struct mi_interpose_s _mi_interposes[]  __attribute__((section("__DATA, __interpose"))) =
  {
    MI_INTERPOSE_MI(malloc),
    MI_INTERPOSE_MI(calloc),
    MI_INTERPOSE_MI(realloc),
    MI_INTERPOSE_MI(strdup),
    MI_INTERPOSE_MI(strndup),
    MI_INTERPOSE_MI(realpath),
    MI_INTERPOSE_MI(posix_memalign),
    MI_INTERPOSE_MI(reallocf),
    MI_INTERPOSE_MI(valloc),
    #ifndef MI_OSX_ZONE
    // some code allocates from default zone but deallocates using plain free :-( (like NxHashResizeToCapacity <https://github.com/nneonneo/osx-10.9-opensource/blob/master/objc4-551.1/runtime/hashtable2.mm>)
    MI_INTERPOSE_FUN(free,mi_cfree), // use safe free that checks if pointers are from us
    #else
    // We interpose malloc_default_zone in alloc-override-osx.c
    MI_INTERPOSE_MI(free),
    #endif
    // some code allocates from a zone but deallocates using plain free :-( (like NxHashResizeToCapacity <https://github.com/nneonneo/osx-10.9-opensource/blob/master/objc4-551.1/runtime/hashtable2.mm>)
    MI_INTERPOSE_FUN(free,mi_cfree), // use safe free that checks if pointers are from us
  };
#elif defined(_MSC_VER)
  // cannot override malloc unless using a dll.
  // we just override new/delete which does work in a static library.
#else
  // On all other systems forward to our API
  void* malloc(size_t size)              MI_FORWARD1(mi_malloc, size);
  void* calloc(size_t size, size_t n)    MI_FORWARD2(mi_calloc, size, n);
  void* realloc(void* p, size_t newsize) MI_FORWARD2(mi_realloc, p, newsize);
  void  free(void* p)                    MI_FORWARD0(mi_free, p);
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACH__)
#pragma GCC visibility push(default)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------
// Posix & Unix functions definitions
// ------------------------------------------------------

void   cfree(void* p)                    MI_FORWARD0(mi_free, p);
void*  reallocf(void* p, size_t newsize) MI_FORWARD2(mi_reallocf,p,newsize);
size_t malloc_size(const void* p)        MI_FORWARD1(mi_usable_size,p);
#if !defined(__ANDROID__)
size_t malloc_usable_size(void *p)       MI_FORWARD1(mi_usable_size,p);
#else
size_t malloc_usable_size(const void *p) MI_FORWARD1(mi_usable_size,p);
#endif

// no forwarding here due to aliasing/name mangling issues
void* valloc(size_t size)                                     { return mi_valloc(size); }
void* pvalloc(size_t size)                                    { return mi_pvalloc(size); }
void* reallocarray(void* p, size_t count, size_t size)        { return mi_reallocarray(p, count, size); }
void* memalign(size_t alignment, size_t size)                 { return mi_memalign(alignment, size); }
int   posix_memalign(void** p, size_t alignment, size_t size) { return mi_posix_memalign(p, alignment, size); }
void* _aligned_malloc(size_t alignment, size_t size)          { return mi_aligned_alloc(alignment, size); }

// on some glibc `aligned_alloc` is declared `static inline` so we cannot override it (e.g. Conda). This happens
// when _GLIBCXX_HAVE_ALIGNED_ALLOC is not defined. However, in those cases it will use `memalign`, `posix_memalign`, 
// or `_aligned_malloc` and we can avoid overriding it ourselves.
// We should always override if using C compilation. (issue #276)
#if _GLIBCXX_HAVE_ALIGNED_ALLOC || !defined(__cplusplus)
void* aligned_alloc(size_t alignment, size_t size)   { return mi_aligned_alloc(alignment, size); }
#endif


#if defined(__GLIBC__) && defined(__linux__)
  // forward __libc interface (needed for glibc-based Linux distributions)
  void* __libc_malloc(size_t size)                  MI_FORWARD1(mi_malloc,size);
  void* __libc_calloc(size_t count, size_t size)    MI_FORWARD2(mi_calloc,count,size);
  void* __libc_realloc(void* p, size_t size)        MI_FORWARD2(mi_realloc,p,size);
  void  __libc_free(void* p)                        MI_FORWARD0(mi_free,p);
  void  __libc_cfree(void* p)                       MI_FORWARD0(mi_free,p);

  void* __libc_valloc(size_t size)                                { return mi_valloc(size); }
  void* __libc_pvalloc(size_t size)                               { return mi_pvalloc(size); }
  void* __libc_memalign(size_t alignment, size_t size)            { return mi_memalign(alignment,size); }
  int   __posix_memalign(void** p, size_t alignment, size_t size) { return mi_posix_memalign(p,alignment,size); }
#endif

#ifdef __cplusplus
}
#endif

#if (defined(__GNUC__) || defined(__clang__)) && !defined(__MACH__)
#pragma GCC visibility pop
#endif

#endif // MI_MALLOC_OVERRIDE && !_WIN32
