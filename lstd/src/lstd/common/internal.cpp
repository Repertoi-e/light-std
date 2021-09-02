#include "../memory/array.h"
#include "common.h"
#include "context.h"
#include "os_function_call.h"

import fmt;

LSTD_BEGIN_NAMESPACE

#if ARCH == X86
#include <emmintrin.h>  //Intel/AMD SSE intrinsics
#if COMPILER == MSVC
#include <intrin.h>  // __cpuid (Visual Studio)
#else
#include <cpuid.h>  // __get_cpuid (GCC / Clang)
#endif
#endif

// Implements optimized copy_memory, fill_memory and compare_memory
namespace apex {

// apex memmove (tiberium, kryptonite and mithril) memcpy/memmove functions written by Trevor Herselman in 2014
#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)  // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#pragma warning(disable : 4244)  // warning C4244: '-=': conversion from '__int64' to 'std::size_t', possible loss of data
#endif

#if ARCH == X86
void *tiberium(void *dst, const void *src, u64 size) {
    /* based on memmove09 for "size <= 112" and memmove40 for "size > 112" */
    if (size <= 112) {
        if (size >= 16) {
            const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
            if (size > 16) {
                long long rax, rcx;
                if (size >= 32) {
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                    if (size > 32) {
                        rax = *(long long *) ((char *) src + size - 16);
                        rcx = *(long long *) ((char *) src + size - 8);
                        if (size > 48) {
                            const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + 32));
                            if (size > 64) {
                                const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + 48));
                                if (size > 80) {
                                    const __m128i xmm4 = _mm_loadu_si128((__m128i *) ((char *) src + 64));
                                    if (size > 96) {
                                        const __m128i xmm5                        = _mm_loadu_si128((__m128i *) ((char *) src + 80));
                                        *(long long *) ((char *) dst + size - 16) = rax;
                                        *(long long *) ((char *) dst + size - 8)  = rcx;
                                        _mm_storeu_si128((__m128i *) dst, xmm0);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 48), xmm3);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 64), xmm4);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 80), xmm5);
                                        return dst;
                                    }
                                    *(long long *) ((char *) dst + size - 16) = rax;
                                    *(long long *) ((char *) dst + size - 8)  = rcx;
                                    _mm_storeu_si128((__m128i *) dst, xmm0);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 64), xmm4);
                                    return dst;
                                }
                                *(long long *) ((char *) dst + size - 16) = rax;
                                *(long long *) ((char *) dst + size - 8)  = rcx;
                                _mm_storeu_si128((__m128i *) dst, xmm0);
                                _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((char *) dst + 48), xmm3);
                                return dst;
                            }
                            *(long long *) ((char *) dst + size - 16) = rax;
                            *(long long *) ((char *) dst + size - 8)  = rcx;
                            _mm_storeu_si128((__m128i *) dst, xmm0);
                            _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                            return dst;
                        }
                        *(long long *) ((char *) dst + size - 16) = rax;
                        *(long long *) ((char *) dst + size - 8)  = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dst, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                    return dst;
                }
                rax                                       = *(long long *) ((char *) src + size - 16);
                rcx                                       = *(long long *) ((char *) src + size - 8);
                *(long long *) ((char *) dst + size - 16) = rax;
                *(long long *) ((char *) dst + size - 8)  = rcx;
            }
            _mm_storeu_si128((__m128i *) dst, xmm0);
            return dst;
        }
        if (size >= 8) {
            long long rax = *(long long *) src;
            if (size > 8) {
                long long rcx                            = *(long long *) ((char *) src + size - 8);
                *(long long *) dst                       = rax;
                *(long long *) ((char *) dst + size - 8) = rcx;
            } else
                *(long long *) dst = rax;
        } else if (size >= 4) {
            int eax = *(int *) src;
            if (size > 4) {
                int ecx                            = *(int *) ((char *) src + size - 4);
                *(int *) dst                       = eax;
                *(int *) ((char *) dst + size - 4) = ecx;
            } else
                *(int *) dst = eax;
        } else if (size >= 1) {
            char al = *(char *) src;
            if (size > 1) {
                short cx                             = *(short *) ((char *) src + size - 2);
                *(char *) dst                        = al;
                *(short *) ((char *) dst + size - 2) = cx;
            } else
                *(char *) dst = al;
        }
        return dst;
    }
    void *const ret = dst;
    if ((size_t) dst - (size_t) src >= size) {
        if (size < 1024 * 256) {
            long long offset = (long long) (size & -0x40); /* "Round down to nearest multiple of 64" */
            dst              = (char *) dst + offset;      /* "Point to the end" */
            src              = (char *) src + offset;      /* "Point to the end" */
            size -= offset;                                /* "Remaining data after loop" */
            offset = -offset;                              /* "Negative index from the end" */

            do {
                const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_storeu_si128((__m128i *) ((char *) dst + offset), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dst + offset + 16), xmm1);
                _mm_storeu_si128((__m128i *) ((char *) dst + offset + 32), xmm2);
                _mm_storeu_si128((__m128i *) ((char *) dst + offset + 48), xmm3);
            } while (offset += 64);

            if (size >= 16) {
                const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
                if (size > 16) {
                    const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + size - 16));
                    if (size > 32) {
                        const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                        if (size > 48) {
                            const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + 32));
                            _mm_storeu_si128((__m128i *) dst, xmm0);
                            _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                            _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm3);
                            return ret;
                        }
                        _mm_storeu_si128((__m128i *) dst, xmm0);
                        _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm3);
                        return ret;
                    }
                    _mm_storeu_si128((__m128i *) dst, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm3);
                    return ret;
                }
                _mm_storeu_si128((__m128i *) dst, xmm0);
                return ret;
            }
        } else /* do forward streaming copy/move */
        {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            long long offset;
            /* We MUST do prealignment on streaming copies! */
            const size_t prealign = -(size_t) dst & 0xf;
            if (prealign) {
                if (prealign >= 8) {
                    long long rax = *(long long *) src;
                    if (prealign > 8) {
                        long long rcx                                = *(long long *) ((char *) src + prealign - 8);
                        *(long long *) dst                           = rax;
                        *(long long *) ((char *) dst + prealign - 8) = rcx;
                    } else
                        *(long long *) dst = rax;
                } else if (prealign >= 4) {
                    int eax = *(int *) src;
                    if (prealign > 4) {
                        int ecx                                = *(int *) ((char *) src + prealign - 4);
                        *(int *) dst                           = eax;
                        *(int *) ((char *) dst + prealign - 4) = ecx;
                    } else
                        *(int *) dst = eax;
                } else {
                    char al = *(char *) src;
                    if (prealign > 1) {
                        short cx                                 = *(short *) ((char *) src + prealign - 2);
                        *(char *) dst                            = al;
                        *(short *) ((char *) dst + prealign - 2) = cx;
                    } else
                        *(char *) dst = al;
                }
                src = (char *) src + prealign;
                dst = (char *) dst + prealign;
                size -= prealign;
            }

            /* Begin prefetching upto 4KB */
            for (offset = 0; offset < 4096; offset += 256) {
                _mm_prefetch((char *) src + offset, _MM_HINT_NTA);
                _mm_prefetch((char *) src + offset + 64, _MM_HINT_NTA);
                _mm_prefetch((char *) src + offset + 128, _MM_HINT_NTA);
                _mm_prefetch((char *) src + offset + 192, _MM_HINT_NTA);
            }

            offset = (long long) (size & -0x40); /* "Round down to nearest multiple of 64" */
            size -= offset;                      /* "Remaining data after loop" */
            offset -= 4096;                      /* stage 1 INCLUDES prefetches */
            dst    = (char *) dst + offset;      /* "Point to the end" */
            src    = (char *) src + offset;      /* "Point to the end" */
            offset = -offset;                    /* "Negative index from the end" */

            do /* stage 1 ~~ WITH prefetching */
            {
                _mm_prefetch((char *) src + offset + 4096, _MM_HINT_NTA);
                xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm3);
            } while (offset += 64);

            offset = -4096;
            dst    = (char *) dst + 4096;
            src    = (char *) src + 4096;

            _mm_prefetch((char *) src + size - 64, _MM_HINT_NTA); /* prefetch the final tail section */

            do /* stage 2 ~~ WITHOUT further prefetching */
            {
                xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm3);
            } while (offset += 64);

            if (size >= 16) {
                xmm0 = _mm_loadu_si128((__m128i *) src);
                if (size > 16) {
                    __m128i xmm6;
                    __m128i xmm7;
                    if (size > 32) {
                        xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                        xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + size - 32));
                        xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size - 16));
                        _mm_stream_si128((__m128i *) dst, xmm0);
                        _mm_stream_si128((__m128i *) ((char *) dst + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dst + size - 32), xmm6);
                        _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm7);
                        return ret;
                    }
                    xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size - 16));
                    _mm_stream_si128((__m128i *) dst, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm7);
                    return ret;
                }
                _mm_stream_si128((__m128i *) dst, xmm0);
                return ret;
            }
        }

        if (size >= 8) {
            long long rax = *(long long *) src;
            if (size > 8) {
                long long rcx                            = *(long long *) ((char *) src + size - 8);
                *(long long *) dst                       = rax;
                *(long long *) ((char *) dst + size - 8) = rcx;
            } else
                *(long long *) dst = rax;
        } else if (size >= 4) {
            int eax = *(int *) src;
            if (size > 4) {
                int ecx                            = *(int *) ((char *) src + size - 4);
                *(int *) dst                       = eax;
                *(int *) ((char *) dst + size - 4) = ecx;
            } else
                *(int *) dst = eax;
        } else if (size >= 1) {
            char al = *(char *) src;
            if (size > 1) {
                short cx                             = *(short *) ((char *) src + size - 2);
                *(char *) dst                        = al;
                *(short *) ((char *) dst + size - 2) = cx;
            } else
                *(char *) dst = al;
        }
        return ret;
    } /* src < dst ... do reverse copy */
    int eax;
    int ecx;
    short cx;

    src = (char *) src + size;
    dst = (char *) dst + size;

    if (size < 1024 * 256) {
        __m128i xmm0;
        __m128i xmm1;
        __m128i xmm2;
        __m128i xmm3;
        long long offset = (long long) (size & -0x40); /* "Round down to nearest multiple of 64" */
        dst              = (char *) dst - offset;      /* "Point to the end" ... actually, we point to the start! */
        src              = (char *) src - offset;      /* "Point to the end" ... actually, we point to the start! */
        size -= offset;                                /* "Remaining data after loop" */
        /*offset = -offset;                          // "Negative index from the end" ... not when doing reverse copy/move! */

        offset -= 64;
        do {
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_storeu_si128((__m128i *) ((char *) dst + offset + 48), xmm0);
            _mm_storeu_si128((__m128i *) ((char *) dst + offset + 32), xmm1);
            _mm_storeu_si128((__m128i *) ((char *) dst + offset + 16), xmm2);
            _mm_storeu_si128((__m128i *) ((char *) dst + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (size >= 16) {
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
            if (size > 16) {
                size = -size;
                xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + size));
                if (size > 32) {
                    xmm1 = _mm_loadu_si128((__m128i *) ((char *) src - 32));
                    if (size > 48) {
                        xmm2 = _mm_loadu_si128((__m128i *) ((char *) src - 48));
                        _mm_storeu_si128((__m128i *) ((char *) dst - 16), xmm0);
                        _mm_storeu_si128((__m128i *) ((char *) dst - 32), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dst - 48), xmm2);
                        _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm3);
                        return ret;
                    }
                    _mm_storeu_si128((__m128i *) ((char *) dst - 16), xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dst - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm3);
                    return ret;
                }
                _mm_storeu_si128((__m128i *) ((char *) dst - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm3);
                return ret;
            }
            _mm_storeu_si128((__m128i *) ((char *) dst - 16), xmm0);
            return ret;
        }
    } else /* do reversed streaming copy/move */
    {
        __m128i xmm0;
        __m128i xmm1;
        __m128i xmm2;
        __m128i xmm3;
        __m128i xmm6;
        __m128i xmm7;
        long long offset;
        long long rax, rcx;
        /* We MUST do prealignment on streaming copies! */
        const size_t prealign = (size_t) dst & 0xf;
        if (prealign) {
            src = (char *) src - prealign;
            dst = (char *) dst - prealign;
            size -= prealign;
            if (prealign >= 8) {
                rax = *(long long *) ((char *) src + prealign - 8);
                if (prealign > 8) {
                    rcx                                          = *(long long *) src;
                    *(long long *) ((char *) dst + prealign - 8) = rax;
                    *(long long *) dst                           = rcx;
                } else
                    *(long long *) dst = rax; /* different on purpose, because we know the exact size now, which is 8, and "dst" has already been aligned! */
            } else if (prealign >= 4) {
                eax = *(int *) ((char *) src + prealign - 4);
                if (prealign > 4) {
                    ecx                                    = *(int *) src;
                    *(int *) ((char *) dst + prealign - 4) = eax;
                    *(int *) dst                           = ecx;
                } else
                    *(int *) dst = eax; /* different on purpose! */
            } else {
                char al = *((char *) src + prealign - 1);
                if (prealign > 1) {
                    cx                             = *(short *) src;
                    *((char *) dst + prealign - 1) = al;
                    *(short *) dst                 = cx;
                } else
                    *(char *) dst = al; /* different on purpose! */
            }
        }

        /* Begin prefetching upto 4KB */
        for (offset = 0; offset > -4096; offset -= 256) {
            _mm_prefetch((char *) src + offset - 64, _MM_HINT_NTA);
            _mm_prefetch((char *) src + offset - 128, _MM_HINT_NTA);
            _mm_prefetch((char *) src + offset - 192, _MM_HINT_NTA);
            _mm_prefetch((char *) src + offset - 256, _MM_HINT_NTA);
        }

        offset = (long long) (size & -0x40); /* "Round down to nearest multiple of 64" */
        size -= offset;                      /* "Remaining data after loop" */
        offset -= 4096;                      /* stage 1 INCLUDES prefetches */
        dst = (char *) dst - offset;         /* "Point to the end" ... actually, we point to the start! */
        src = (char *) src - offset;         /* "Point to the end" ... actually, we point to the start! */
        /*offset = -offset;                    // "Negative index from the end" ... not when doing reverse copy/move! */

        offset -= 64;
        do /* stage 1 ~~ WITH prefetching */
        {
            _mm_prefetch((char *) src + offset - 4096, _MM_HINT_NTA);
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm3);
        } while ((offset -= 64) >= 0);

        offset = 4096;
        dst    = (char *) dst - 4096;
        src    = (char *) src - 4096;

        _mm_prefetch((char *) src - 64, _MM_HINT_NTA); /* prefetch the final tail section */

        offset -= 64;
        do /* stage 2 ~~ WITHOUT further prefetching */
        {
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (size >= 16) {
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
            if (size > 16) {
                if (size > 32) {
                    size = -size;
                    xmm1 = _mm_loadu_si128((__m128i *) ((char *) src - 32));
                    xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + size + 16));
                    xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size));
                    _mm_stream_si128((__m128i *) ((char *) dst - 16), xmm0);
                    _mm_stream_si128((__m128i *) ((char *) dst - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size + 16), xmm6);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm7);
                    return ret;
                }
                size = -size;
                xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size));
                _mm_stream_si128((__m128i *) ((char *) dst - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm7);
                return ret;
            }
            _mm_stream_si128((__m128i *) ((char *) dst - 16), xmm0);
            return ret;
        }
    }

    if (size >= 8) {
        long long rcx;
        long long rax = *(long long *) ((char *) src - 8);
        if (size > 8) {
            size                                 = -size; /* that's right, we're converting an unsigned value to a negative, saves 2 clock cycles! */
            rcx                                  = *(long long *) ((char *) src + size);
            *(long long *) ((char *) dst - 8)    = rax;
            *(long long *) ((char *) dst + size) = rcx;
        } else
            *(long long *) ((char *) dst - 8) = rax;
    } else if (size >= 4) {
        eax = *(int *) ((char *) src - 4);
        if (size > 4) {
            size                           = -size;
            ecx                            = *(int *) ((char *) src + size);
            *(int *) ((char *) dst - 4)    = eax;
            *(int *) ((char *) dst + size) = ecx;
        } else
            *(int *) ((char *) dst - 4) = eax;
    } else if (size >= 1) {
        char al = *((char *) src - 1);
        if (size > 1) {
            size                             = -size;
            cx                               = *(short *) ((char *) src + size);
            *((char *) dst - 1)              = al;
            *(short *) ((char *) dst + size) = cx;
        } else
            *((char *) dst - 1) = al;
    }
    return ret;
}

void *kryptonite(void *dst, const void *src, u64 size) {
    /* based on memmove09 for "size <= 112" and memmove41 for "size > 112"; memmove09's "size <= 112" proved fastest overall (weighted), even on Core i5! */
    if (size <= 112) {
        if (size >= 16) {
            const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
            if (size > 16) {
                long long rax;
                long long rcx;
                if (size >= 32) {
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                    if (size > 32) {
                        rax = *(long long *) ((char *) src + size - 16);
                        rcx = *(long long *) ((char *) src + size - 8);
                        if (size > 48) {
                            const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + 32));
                            if (size > 64) {
                                const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + 48));
                                if (size > 80) {
                                    const __m128i xmm4 = _mm_loadu_si128((__m128i *) ((char *) src + 64));
                                    if (size > 96) {
                                        const __m128i xmm5                        = _mm_loadu_si128((__m128i *) ((char *) src + 80));
                                        *(long long *) ((char *) dst + size - 16) = rax;
                                        *(long long *) ((char *) dst + size - 8)  = rcx;
                                        _mm_storeu_si128((__m128i *) dst, xmm0);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 48), xmm3);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 64), xmm4);
                                        _mm_storeu_si128((__m128i *) ((char *) dst + 80), xmm5);
                                        return dst;
                                    }
                                    *(long long *) ((char *) dst + size - 16) = rax;
                                    *(long long *) ((char *) dst + size - 8)  = rcx;
                                    _mm_storeu_si128((__m128i *) dst, xmm0);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((char *) dst + 64), xmm4);
                                    return dst;
                                }
                                *(long long *) ((char *) dst + size - 16) = rax;
                                *(long long *) ((char *) dst + size - 8)  = rcx;
                                _mm_storeu_si128((__m128i *) dst, xmm0);
                                _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((char *) dst + 48), xmm3);
                                return dst;
                            }
                            *(long long *) ((char *) dst + size - 16) = rax;
                            *(long long *) ((char *) dst + size - 8)  = rcx;
                            _mm_storeu_si128((__m128i *) dst, xmm0);
                            _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((char *) dst + 32), xmm2);
                            return dst;
                        }
                        *(long long *) ((char *) dst + size - 16) = rax;
                        *(long long *) ((char *) dst + size - 8)  = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dst, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dst + 16), xmm1);
                    return dst;
                }
                rax                                       = *(long long *) ((char *) src + size - 16);
                rcx                                       = *(long long *) ((char *) src + size - 8);
                *(long long *) ((char *) dst + size - 16) = rax;
                *(long long *) ((char *) dst + size - 8)  = rcx;
            }
            _mm_storeu_si128((__m128i *) dst, xmm0);
            return dst;
        }
        if (size >= 8) {
            long long rax = *(long long *) src;
            if (size > 8) {
                long long rcx                            = *(long long *) ((char *) src + size - 8);
                *(long long *) dst                       = rax;
                *(long long *) ((char *) dst + size - 8) = rcx;
            } else
                *(long long *) dst = rax;
        } else if (size >= 4) {
            int eax = *(int *) src;
            if (size > 4) {
                int ecx                            = *(int *) ((char *) src + size - 4);
                *(int *) dst                       = eax;
                *(int *) ((char *) dst + size - 4) = ecx;
            } else
                *(int *) dst = eax;
        } else if (size >= 1) {
            char al = *(char *) src;
            if (size > 1) {
                short cx                             = *(short *) ((char *) src + size - 2);
                *(char *) dst                        = al;
                *(short *) ((char *) dst + size - 2) = cx;
            } else
                *(char *) dst = al;
        }
        return dst;
    }
    void *const ret = dst;
    if ((size_t) dst - (size_t) src >= size) {
        if (size < 1024 * 256) {
            long long offset = (long long) (size & -0x20); /* "Round down to nearest multiple of 64" */
            dst              = (char *) dst + offset;      /* "Point to the end" */
            src              = (char *) src + offset;      /* "Point to the end" */
            size -= offset;                                /* "Remaining data after loop" */
            offset = -offset;                              /* "Negative index from the end" */

            do {
                const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                _mm_storeu_si128((__m128i *) ((char *) dst + offset), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dst + offset + 16), xmm1);
            } while (offset += 32);

            if (size >= 16) {
                if (size > 16) {
                    const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size - 16));
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm7);
                    _mm_storeu_si128((__m128i *) dst, xmm0);
                    return ret;
                }
                _mm_storeu_si128((__m128i *) dst, _mm_loadu_si128((__m128i *) src));
                return ret;
            }
        } else /* do forward streaming copy/move */
        {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            __m128i xmm6;
            __m128i xmm7;
            long long offset;
            /* We MUST do prealignment on streaming copies! */
            const size_t prealign = -(size_t) dst & 0xf;
            if (prealign) {
                if (prealign >= 8) {
                    long long rax = *(long long *) src;
                    if (prealign > 8) {
                        long long rcx                                = *(long long *) ((char *) src + prealign - 8);
                        *(long long *) dst                           = rax;
                        *(long long *) ((char *) dst + prealign - 8) = rcx;
                    } else
                        *(long long *) dst = rax;
                } else if (prealign >= 4) {
                    int eax = *(int *) src;
                    if (prealign > 4) {
                        int ecx                                = *(int *) ((char *) src + prealign - 4);
                        *(int *) dst                           = eax;
                        *(int *) ((char *) dst + prealign - 4) = ecx;
                    } else
                        *(int *) dst = eax;
                } else {
                    char al = *(char *) src;
                    if (prealign > 1) {
                        short cx                                 = *(short *) ((char *) src + prealign - 2);
                        *(char *) dst                            = al;
                        *(short *) ((char *) dst + prealign - 2) = cx;
                    } else
                        *(char *) dst = al;
                }
                src = (char *) src + prealign;
                dst = (char *) dst + prealign;
                size -= prealign;
            }

            /* Begin prefetching upto 4KB */
            for (offset = 0; offset < 4096; offset += 256) {
                _mm_prefetch((char *) src + offset, _MM_HINT_NTA);
                _mm_prefetch((char *) src + offset + 64, _MM_HINT_NTA);
                _mm_prefetch((char *) src + offset + 128, _MM_HINT_NTA);
                _mm_prefetch((char *) src + offset + 192, _MM_HINT_NTA);
            }

            offset = (long long) (size & -0x40); /* "Round down to nearest multiple of 64" */
            size -= offset;                      /* "Remaining data after loop" */
            offset -= 4096;                      /* stage 1 INCLUDES prefetches */
            dst    = (char *) dst + offset;      /* "Point to the end" */
            src    = (char *) src + offset;      /* "Point to the end" */
            offset = -offset;                    /* "Negative index from the end" */

            do /* stage 1 ~~ WITH prefetching */
            {
                _mm_prefetch((char *) src + offset + 4096, _MM_HINT_NTA);
                xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm3);
            } while (offset += 64);

            offset = -4096;
            dst    = (char *) dst + 4096;
            src    = (char *) src + 4096;

            _mm_prefetch((char *) src + size - 64, _MM_HINT_NTA); /* prefetch the final tail section */

            do /* stage 2 ~~ WITHOUT further prefetching */
            {
                xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm3);
            } while (offset += 64);

            if (size >= 16) {
                xmm0 = _mm_loadu_si128((__m128i *) src);
                if (size > 16) {
                    if (size > 32) {
                        xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                        xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + size - 32));
                        xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size - 16));
                        _mm_stream_si128((__m128i *) dst, xmm0);
                        _mm_stream_si128((__m128i *) ((char *) dst + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dst + size - 32), xmm6);
                        _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm7);
                        return ret;
                    }
                    xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size - 16));
                    _mm_stream_si128((__m128i *) dst, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size - 16), xmm7);
                    return ret;
                }
                _mm_stream_si128((__m128i *) dst, xmm0);
                return ret;
            }
        }

        if (size >= 8) {
            long long rax = *(long long *) src;
            if (size > 8) {
                long long rcx                            = *(long long *) ((char *) src + size - 8);
                *(long long *) dst                       = rax;
                *(long long *) ((char *) dst + size - 8) = rcx;
            } else
                *(long long *) dst = rax;
        } else if (size >= 4) {
            int eax = *(int *) src;
            if (size > 4) {
                int ecx                            = *(int *) ((char *) src + size - 4);
                *(int *) dst                       = eax;
                *(int *) ((char *) dst + size - 4) = ecx;
            } else
                *(int *) dst = eax;
        } else if (size >= 1) {
            char al = *(char *) src;
            if (size > 1) {
                short cx                             = *(short *) ((char *) src + size - 2);
                *(char *) dst                        = al;
                *(short *) ((char *) dst + size - 2) = cx;
            } else
                *(char *) dst = al;
        }
        return ret;
    } /* src < dst ... do reverse copy */
    src = (char *) src + size;
    dst = (char *) dst + size;

    if (size < 1024 * 256) {
        long long offset = (long long) (size & -0x20); /* "Round down to nearest multiple of 64" */
        dst              = (char *) dst - offset;      /* "Point to the end" ... actually, we point to the start! */
        src              = (char *) src - offset;      /* "Point to the end" ... actually, we point to the start! */
        size -= offset;                                /* "Remaining data after loop" */
        /*offset = -offset;                          // "Negative index from the end" ... not when doing reverse copy/move! */

        offset -= 32;
        do {
            const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_storeu_si128((__m128i *) ((char *) dst + offset + 16), xmm2);
            _mm_storeu_si128((__m128i *) ((char *) dst + offset), xmm3);
        } while ((offset -= 32) >= 0);

        if (size >= 16) {
            if (size > 16) {
                size = -size;
                /* The order has been mixed so the compiler will not re-order the statements! */
                const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size));
                const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
                _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm7);
                _mm_storeu_si128((__m128i *) ((char *) dst - 16), xmm0);
                return ret;
            }
            _mm_storeu_si128((__m128i *) ((char *) dst - 16), _mm_loadu_si128((__m128i *) ((char *) src - 16)));
            return ret;
        }
    } else /* do reversed streaming copy/move */
    {
        __m128i xmm0;
        __m128i xmm1;
        __m128i xmm2;
        __m128i xmm3;
        __m128i xmm6;
        __m128i xmm7;
        long long offset;
        /* We MUST do prealignment on streaming copies! */
        const size_t prealign = (size_t) dst & 0xf;
        if (prealign) {
            src = (char *) src - prealign;
            dst = (char *) dst - prealign;
            size -= prealign;
            if (prealign >= 8) {
                long long rax = *(long long *) ((char *) src + prealign - 8);
                if (prealign > 8) {
                    long long rcx                                = *(long long *) src;
                    *(long long *) ((char *) dst + prealign - 8) = rax;
                    *(long long *) dst                           = rcx;
                } else
                    *(long long *) dst = rax; /* different on purpose, because we know the exact size now, which is 8, and "dst" has already been aligned! */
            } else if (prealign >= 4) {
                int eax = *(int *) ((char *) src + prealign - 4);
                if (prealign > 4) {
                    int ecx                                = *(int *) src;
                    *(int *) ((char *) dst + prealign - 4) = eax;
                    *(int *) dst                           = ecx;
                } else
                    *(int *) dst = eax; /* different on purpose! */
            } else {
                char al = *((char *) src + prealign - 1);
                if (prealign > 1) {
                    short cx                       = *(short *) src;
                    *((char *) dst + prealign - 1) = al;
                    *(short *) dst                 = cx;
                } else
                    *(char *) dst = al; /* different on purpose! */
            }
        }

        /* Begin prefetching upto 4KB */
        for (offset = 0; offset > -4096; offset -= 256) {
            _mm_prefetch((char *) src + offset - 64, _MM_HINT_NTA);
            _mm_prefetch((char *) src + offset - 128, _MM_HINT_NTA);
            _mm_prefetch((char *) src + offset - 192, _MM_HINT_NTA);
            _mm_prefetch((char *) src + offset - 256, _MM_HINT_NTA);
        }

        offset = (long long) (size & -0x40); /* "Round down to nearest multiple of 64" */
        size -= offset;                      /* "Remaining data after loop" */
        offset -= 4096;                      /* stage 1 INCLUDES prefetches */
        dst = (char *) dst - offset;         /* "Point to the end" ... actually, we point to the start! */
        src = (char *) src - offset;         /* "Point to the end" ... actually, we point to the start! */
        /*offset = -offset;                    // "Negative index from the end" ... not when doing reverse copy/move! */

        offset -= 64;
        do /* stage 1 ~~ WITH prefetching */
        {
            _mm_prefetch((char *) src + offset - 4096, _MM_HINT_NTA);
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm3);
        } while ((offset -= 64) >= 0);

        offset = 4096;
        dst    = (char *) dst - 4096;
        src    = (char *) src - 4096;

        _mm_prefetch((char *) src - 64, _MM_HINT_NTA); /* prefetch the final tail section */

        offset -= 64;
        do /* stage 2 ~~ WITHOUT further prefetching */
        {
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dst + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dst + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (size >= 16) {
            xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
            if (size > 16) {
                if (size > 32) {
                    size = -size;
                    xmm1 = _mm_loadu_si128((__m128i *) ((char *) src - 32));
                    xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + size + 16));
                    xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size));
                    _mm_stream_si128((__m128i *) ((char *) dst - 16), xmm0);
                    _mm_stream_si128((__m128i *) ((char *) dst - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size + 16), xmm6);
                    _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm7);
                    return ret;
                }
                size = -size;
                xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + size));
                _mm_stream_si128((__m128i *) ((char *) dst - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dst + size), xmm7);
                return ret;
            }
            _mm_stream_si128((__m128i *) ((char *) dst - 16), xmm0);
            return ret;
        }
    }

    if (size >= 8) {
        long long rcx;
        long long rax = *(long long *) ((char *) src - 8);
        if (size > 8) {
            size                                 = -size; /* that's right, we're converting an unsigned value to a negative, saves 2 clock cycles! */
            rcx                                  = *(long long *) ((char *) src + size);
            *(long long *) ((char *) dst - 8)    = rax;
            *(long long *) ((char *) dst + size) = rcx;
        } else
            *(long long *) ((char *) dst - 8) = rax;
    } else if (size >= 4) {
        int ecx;
        int eax = *(int *) ((char *) src - 4);
        if (size > 4) {
            size                           = -size;
            ecx                            = *(int *) ((char *) src + size);
            *(int *) ((char *) dst - 4)    = eax;
            *(int *) ((char *) dst + size) = ecx;
        } else
            *(int *) ((char *) dst - 4) = eax;
    } else if (size >= 1) {
        short cx;
        char al = *((char *) src - 1);
        if (size > 1) {
            size                             = -size;
            cx                               = *(short *) ((char *) src + size);
            *((char *) dst - 1)              = al;
            *(short *) ((char *) dst + size) = cx;
        } else
            *((char *) dst - 1) = al;
    }
    return ret;
}
#endif  // end test for x86 arch

#if COMPILER == MSVC
#pragma warning(pop)
#endif

// This sets up copy_memory the first time it's called
file_scope void *dispatcher(void *dst, const void *src, u64 size) {
#if ARCH == X86
#if COMPILER == MSVC
    s32 cpuid[4] = {-1};
    __cpuid(cpuid, 1);
#define bit_SSE2 (1 << 26)
#define bit_SSSE3 (1 << 9)
#define bit_SSE4_2 (1 << 20)
#else
    u32 cpuid[4];  //	GCC / LLVM (Clang)
    __get_cpuid(1, &cpuid[0], &cpuid[1], &cpuid[2], &cpuid[3]);
#endif
    // Detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access
    if (cpuid[2] & bit_SSE4_2) {
        copy_memory = &kryptonite;
    } else {
        copy_memory = &tiberium;
    }
#else
    copy_memory = &const_copy_memory;
#endif
    // Once we set it, actually run it
    return copy_memory(dst, src, size);
}
} // namespace apex

void *(*copy_memory)(void *dst, const void *src, u64 size) = apex::dispatcher;

//
// SSE optimized fill_memory
// If the platform doesn't support SSE, it still writes 4 bytes at a time (instead of 16)
//

file_scope void fill_single_byte(void *dst, char c, u64 size) {
    // This needs to be marked volatile to avoid compiler optimizing with memset intrinsic
    // and thus causing a stack overflow in optimized_fill_memory.
    auto *b = (volatile byte *) dst;
    while (size--) *b++ = c;
}

void *optimized_fill_memory(void *dst, char c, u64 size) {
    char *d = (char *) dst;

#if ARCH == X86
    u32 offset     = (u64) dst % 16;
    s64 num16bytes = (size - offset) / 16;
    s64 remaining  = size - num16bytes * 16 - offset;
    if (size < offset) {
        offset     = 0;
        num16bytes = 0;
        remaining  = size;
    }

    fill_single_byte(d, c, offset);
    d += offset;

    __m128i c16 = _mm_set_epi8(c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c);
    For(range(num16bytes)) {
        _mm_store_si128((__m128i *) d, c16);
        d += 16;
    }
    fill_single_byte(d, c, remaining);
#else
    const_fill_memory(dst, c, size);
#endif
    return dst;
}

void *(*fill_memory)(void *dst, char value, u64 size) = optimized_fill_memory;

//
// Compare memory (optimized code partly taken from https://code.woboq.org/userspace/glibc/string/memcmp.c.html)
//

file_scope s64 compare_bytes_of_two_u32s(u32 a, u32 b) {
    auto *s1 = (const char *) &a;
    auto *s2 = (const char *) &b;

    for (s64 index = 0; index < 4; ++index) {
        if (*s1++ != *s2++) return index;
    }
    return -1;
}

file_scope s64 compare_memory_common_alignment(const char *s1, const char *s2, u64 size) {
    s64 progress = 0;
    u32 a0, a1;
    u32 b0, b1;

    switch (size % 4) {
        default:
        case 2:
            a0 = *(u32 *) s1;
            b0 = *(u32 *) s2;
            s1 -= 8;
            s2 -= 8;
            size += 2;
            goto do1;
        case 3:
            a1 = *(u32 *) s1;
            b1 = *(u32 *) s2;
            s1 -= 4;
            s2 -= 4;
            size += 1;
            goto do2;
        case 0:
            a0 = *(u32 *) s1;
            b0 = *(u32 *) s2;
            goto do3;
        case 1:
            a1 = *(u32 *) s1;
            b1 = *(u32 *) s2;
            s1 += 4;
            s2 += 4;
            size -= 1;
    }
    do {
        a0 = *(u32 *) s1;
        b0 = *(u32 *) s2;
        if (a1 != b1) return progress + compare_bytes_of_two_u32s(a1, b1);
        progress += 4;
    do3:
        a1 = *((u32 *) s1 + 1);
        b1 = *((u32 *) s2 + 1);
        if (a0 != b0) return progress + compare_bytes_of_two_u32s(a0, b0);
        progress += 4;
    do2:
        a0 = *((u32 *) s1 + 2);
        b0 = *((u32 *) s2 + 2);
        if (a1 != b1) return progress + compare_bytes_of_two_u32s(a1, b1);
        progress += 4;
    do1:
        a1 = *((u32 *) s1 + 3);
        b1 = *((u32 *) s2 + 3);
        if (a0 != b0) return progress + compare_bytes_of_two_u32s(a0, b0);
        progress += 4;
        s1 += 16;
        s2 += 16;
        size -= 4;
    } while (size != 0);

    if (a1 != b1) return size + compare_bytes_of_two_u32s(a1, b1);
    return -1;
}

#if ENDIAN == LITTLE_ENDIAN
#define MERGE(w0, sh_1, w1, sh_2) (((w0) >> (sh_1)) | ((w1) << (sh_2)))
#else
#define MERGE(w0, sh_1, w1, sh_2) (((w0) << (sh_1)) | ((w1) >> (sh_2)))
#endif

file_scope s64 compare_memory_not_common_alignment(const char *s1, const char *s2, u64 size) {
    s64 progress = 0;
    u32 a0, a1, a2, a3;
    u32 b0, b1, b2, b3;
    u32 x;

    // Calculate how to shift a word read at the memory operation aligned srcp1 to make it aligned for comparison.
    s32 shl = 8 * ((s64) s1 % 4);
    s32 shr = 8 * 4 - shl;

    // Make SRCP1 aligned by rounding it down to the beginning of the u32 it points in the middle of.
    s1 = (const char *) ((u64) s1 & -4);
    switch (size % 4) {
        default:
        case 2:
            a1 = *(u32 *) s1;
            a2 = *((u32 *) s1 + 1);
            b2 = *(u32 *) s2;
            s1 -= 4;
            s2 -= 8;
            size += 2;
            goto do1;
        case 3:
            a0 = *(u32 *) s1;
            a1 = *((u32 *) s1 + 1);
            b1 = *(u32 *) s2;
            s2 -= 4;
            size += 1;
            goto do2;
        case 0:
            a3 = *(u32 *) s1;
            a0 = *((u32 *) s1 + 1);
            b0 = *(u32 *) s2;
            s1 += 4;
            goto do3;
        case 1:
            a2 = *(u32 *) s1;
            a3 = *((u32 *) s1 + 1);
            b3 = *(u32 *) s2;
            s1 += 8;
            s2 += 4;
            size -= 1;
    }
    do {
        a0 = *(u32 *) s1;
        b0 = *(u32 *) s2;
        x  = MERGE(a2, shl, a3, shr);
        if (x != b3) return progress + compare_bytes_of_two_u32s(x, b3);
        progress += 4;
    do3:
        a1 = *((u32 *) s1 + 1);
        b1 = *((u32 *) s2 + 1);
        x  = MERGE(a3, shl, a0, shr);
        if (x != b0) return progress + compare_bytes_of_two_u32s(x, b0);
        progress += 4;
    do2:
        a2 = *((u32 *) s1 + 2);
        b2 = *((u32 *) s2 + 2);
        x  = MERGE(a0, shl, a1, shr);
        if (x != b1) return progress + compare_bytes_of_two_u32s(x, b1);
        progress += 4;
    do1:
        a3 = *((u32 *) s1 + 3);
        b3 = *((u32 *) s2 + 3);
        x  = MERGE(a1, shl, a2, shr);
        if (x != b2) return progress + compare_bytes_of_two_u32s(x, b2);
        progress += 4;
        s1 += 16;
        s2 += 16;
        size -= 4;
    } while (size != 0);

    x = MERGE(a2, shl, a3, shr);
    if (x != b3) return size + compare_bytes_of_two_u32s(x, b3);
    return -1;
}

s64 optimized_compare_memory(const void *ptr1, const void *ptr2, u64 size) {
    s64 progress = 0;

    auto s1 = (const char *) ptr1;
    auto s2 = (const char *) ptr2;

    if (size >= 16) {
        for (s64 index = 0; (s64) s2 % 4 != 0; ++index) {
            if (*s1++ != *s2++) return index;
            ++progress;
            --size;
        }

        s64 res;
        if ((u64) s1 % 4 == 0) {
            res = compare_memory_common_alignment(s1, s2, size / 4);
        } else {
            res = compare_memory_not_common_alignment(s1, s2, size / 4);
        }
        if (res != -1) return progress + res;

        // Number of bytes remaining in the interval [0..3]
        s1 += size & -4;
        s2 += size & -4;
        size %= 4;
    }

    // There are just a few bytes to compare. Use byte memory operations.
    for (s64 index = 0; size--; ++index) {
        if (*s1++ != *s2++) return progress + index;
    }
    return -1;
}

s64 (*compare_memory)(const void *ptr1, const void *ptr2, u64 size) = optimized_compare_memory;

void default_panic_handler(const string &message, const array<os_function_call> &callStack) {
    if (Context._HandlingPanic) return;

    auto newContext           = Context;
    newContext._HandlingPanic = true;

    PUSH_CONTEXT(newContext) {
        print("\n\n{!}(context.cpp / default_crash_handler): A panic occurred and the program must terminate.\n");
        print("{!GRAY}        Error: {!RED}{}{!}\n\n", message);
        print("        ... and here is the call stack:\n");
        if (callStack.Count) {
            print("\n");
        }
        For(callStack) {
            print("        {!YELLOW}{}{!}\n", it.Name);
            print("          in file: {}:{}\n", it.File, it.LineNumber);
        }
        if (!callStack.Count) {
            print("          [No call stack available]\n");
        }
        print("\n\n");
    }
}

LSTD_END_NAMESPACE
