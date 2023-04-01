#ifndef INCLUDED_tlsf
#define INCLUDED_tlsf

/*
** Two Level Segregated Fit memory allocator, version 3.1.
** Written by Matthew Conte
**	http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**	http://www.gii.upv.es/tlsf/main/docs
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
**
** Copyright (c) 2006-2016, Matthew Conte
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the copyright holder nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL MATTHEW CONTE BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// This source was downloaded from https://github.com/mattconte/tlsf.
// We provide a thin wrapper for this in memory/tlsf_allocator.h
// You can still use this (as all "vendor" libraries we use within this library)
// raw. Note: The realloc function has been changed (renamed to resize,
// different behaviour)!
//       See comments in tlsf.cpp
//

#include "lstd/common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* tlsf_t: a TLSF structure. Can contain 1 to N pools. */
/* pool_t: a block of memory that TLSF can manage. */
typedef void *tlsf_t;
typedef void *pool_t;

/* Create/destroy a memory pool. */
tlsf_t tlsf_create(void *mem);
tlsf_t tlsf_create_with_pool(void *mem, u64 bytes);
void tlsf_destroy(tlsf_t tlsf);
pool_t tlsf_get_pool(tlsf_t tlsf);

/* Add/remove memory pools. */
pool_t tlsf_add_pool(tlsf_t tlsf, void *mem, u64 bytes);
void tlsf_remove_pool(tlsf_t tlsf, pool_t pool);

/* malloc/memalign/realloc/free replacements. */
void *tlsf_malloc(tlsf_t tlsf, u64 bytes);
// void* tlsf_memalign(tlsf_t tlsf, u64 align, u64 bytes); // :WEMODIFIED: We
// handle alignment in our allocate/reallocate. Just a note to not use this if
// you didn't know that we handled alignment automatically. If you meant to use
// this function, use this declaration - the implementation is still there in
// tlsf.cpp
void *tlsf_resize(tlsf_t tlsf, void *ptr,
                  u64 size);  // :WEMODIFIED: Renamed this from realloc to
                              // resize. See comments in tlsf.cpp
void tlsf_free(tlsf_t tlsf, void *ptr);

/* Returns internal block size, not original request size */
u64 tlsf_block_size(void *ptr);

/* Overheads/limits of internal structures. */
u64 tlsf_size(void);
u64 tlsf_align_size(void);
u64 tlsf_block_size_min(void);
u64 tlsf_block_size_max(void);
u64 tlsf_pool_overhead(void);
u64 tlsf_alloc_overhead(void);

/* Debugging. */
typedef void (*tlsf_walker)(void *ptr, u64 size, int used, void *user);
void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void *user);
/* Returns nonzero if any internal consistency check fails. */
int tlsf_check(tlsf_t tlsf);
int tlsf_check_pool(pool_t pool);

#if defined(__cplusplus)
}
#endif

#endif
