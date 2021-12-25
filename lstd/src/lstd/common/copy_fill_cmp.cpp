#include "../common.h"

import lstd.fmt;

LSTD_BEGIN_NAMESPACE

#if ARCH == X86
#include <emmintrin.h>  //Intel/AMD SSE intrinsics
#if COMPILER == MSVC
#include <intrin.h>  // __cpuid (Visual Studio)
#else
#include <cpuid.h>  // __get_cpuid (GCC / Clang)
#endif
#endif

extern void wordcopy_fwd_aligned(u64 dstp, u64 srcp, u64 len);
extern void wordcopy_fwd_dest_aligned(u64 dstp, u64 srcp, u64 len);
extern void wordcopy_bwd_aligned(u64 dstp, u64 srcp, u64 len);
extern void wordcopy_bwd_dest_aligned(u64 dstp, u64 srcp, u64 len);

// Implements optimized copy_memory and fill_memory
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

// Function equivalent of memmove, courtesy of glibc,
// https://github.com/lattera/glibc/blob/master/string/memmove.c
/* 
 * Copy memory to memory until the specified number of bytes
 * has been copied.  Overlap is handled correctly.
 * Copyright (C) 1991-2018 Free Software Foundation, Inc.
 * This file is part of the GNU C Library.
 * Contributed by Torbjorn Granlund (tege@sics.se).
 *
 * The GNU C Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * The GNU C Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the GNU C Library; if not, see
 * <http://www.gnu.org/licenses/>.
 */
#define OP_T_THRES 16
#define OPSIZ sizeof(u64)  // We copy 8 bytes at a time

/* Copy exactly NBYTES bytes from SRC_BP to DST_BP,
   without any assumptions about alignment of the pointers.  */
#define BYTE_COPY_FWD(dst_bp, src_bp, nbytes) \
    {                                         \
        size_t __nbytes = (nbytes);           \
        while (__nbytes > 0) {                \
            byte x = ((byte *) src_bp)[0];    \
            src_bp += 1;                      \
            __nbytes -= 1;                    \
            ((byte *) dst_bp)[0] = x;         \
            dst_bp += 1;                      \
        }                                     \
    }

/* Copy exactly NBYTES_TO_COPY bytes from SRC_END_PTR to DST_END_PTR,
   beginning at the bytes right before the pointers and continuing towards
   smaller addresses.  Don't assume anything about alignment of the
   pointers.  */
#define BYTE_COPY_BWD(dst_ep, src_ep, nbytes) \
    {                                         \
        size_t __nbytes = (nbytes);           \
        while (__nbytes > 0) {                \
            byte x;                           \
            src_ep -= 1;                      \
            x = ((byte *) src_ep)[0];         \
            dst_ep -= 1;                      \
            __nbytes -= 1;                    \
            ((byte *) dst_ep)[0] = x;         \
        }                                     \
    }

/* Copy *up to* NBYTES bytes from SRC_BP to DST_BP, with
   the assumption that DST_BP is aligned on an OPSIZ multiple.  If
   not all bytes could be easily copied, store remaining number of bytes
   in NBYTES_LEFT, otherwise store 0.  */
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)           \
    if (src_bp % OPSIZ == 0) {                                       \
        wordcopy_fwd_aligned(dst_bp, src_bp, (nbytes) / OPSIZ);      \
    } else {                                                         \
        wordcopy_fwd_dest_aligned(dst_bp, src_bp, (nbytes) / OPSIZ); \
    }                                                                \
    src_bp += (nbytes) & -OPSIZ;                                     \
    dst_bp += (nbytes) & -OPSIZ;                                     \
    (nbytes_left) = (nbytes) % OPSIZ;

/* Copy *up to* NBYTES_TO_COPY bytes from SRC_END_PTR to DST_END_PTR,
   beginning at the words (of type u64) right before the pointers and
   continuing towards smaller addresses.  May take advantage of that
   DST_END_PTR is aligned on an OPSIZ multiple.  If not all bytes could be
   easily copied, store remaining number of bytes in NBYTES_REMAINING,
   otherwise store 0.  */
#define WORD_COPY_BWD(dst_ep, src_ep, nbytes_left, nbytes)               \
    {                                                                    \
        if (src_ep % OPSIZ == 0) {                                       \
            wordcopy_bwd_aligned(dst_ep, src_ep, (nbytes) / OPSIZ);      \
        } else {                                                         \
            wordcopy_bwd_dest_aligned(dst_ep, src_ep, (nbytes) / OPSIZ); \
        }                                                                \
        src_ep -= (nbytes) & -OPSIZ;                                     \
        dst_ep -= (nbytes) & -OPSIZ;                                     \
        (nbytes_left) = (nbytes) % OPSIZ;                                \
    }

#pragma warning(disable : 4146)

void *optimized_copy_memory(void *dst, const void *src, u64 len) {
    u64 dstp = (u64) dst;
    u64 srcp = (u64) src;

    /* This test makes the forward copying code be used whenever possible.
     Reduces the working set.  */
    if (dstp - srcp >= len) /* *Unsigned* compare!  */
    {
        /* Copy from the beginning to the end.  */

        /* If there not too few bytes to copy, use word copy.  */
        if (len >= OP_T_THRES) {
            /* Copy just a few bytes to make DSTP aligned.  */
            len -= (-dstp) % OPSIZ;
            BYTE_COPY_FWD(dstp, srcp, (-dstp) % OPSIZ);

            /* Copy whole pages from SRCP to DSTP by virtual address
	     manipulation, as much as possible.  */
            // PAGE_COPY_FWD_MAYBE(dstp, srcp, len, len);

            /* Copy from SRCP to DSTP taking advantage of the known
	     alignment of DSTP.  Number of bytes remaining is put
	     in the third argument, i.e. in LEN.  This number may
	     vary from machine to machine.  */

            WORD_COPY_FWD(dstp, srcp, len, len);

            /* Fall out and copy the tail.  */
        }

        /* There are just a few bytes to copy.  Use byte memory operations.  */
        BYTE_COPY_FWD(dstp, srcp, len);
    } else {
        /* Copy from the end to the beginning.  */
        srcp += len;
        dstp += len;

        /* If there not too few bytes to copy, use word copy.  */
        if (len >= OP_T_THRES) {
            /* Copy just a few bytes to make DSTP aligned.  */
            len -= dstp % OPSIZ;
            BYTE_COPY_BWD(dstp, srcp, dstp % OPSIZ);

            /* Copy from SRCP to DSTP taking advantage of the known
	     alignment of DSTP.  Number of bytes remaining is put
	     in the third argument, i.e. in LEN.  This number may
	     vary from machine to machine.  */

            WORD_COPY_BWD(dstp, srcp, len, len);

            /* Fall out and copy the tail.  */
        }

        /* There are just a few bytes to copy.  Use byte memory operations.  */
        BYTE_COPY_BWD(dstp, srcp, len);
    }
    return dst;
}

// This sets up copy_memory the first time it's called
file_scope void *dispatcher_copy_memory(void *dst, const void *src, u64 size) {
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
        copy_memory_fast = &kryptonite;
    } else {
        copy_memory_fast = &tiberium;
    }
#else
    copy_memory_fast = &optimized_copy_memory;
#endif
    // Once we set it, actually run it
    return copy_memory_fast(dst, src, size);
}
}  // namespace apex

void *(*copy_memory_fast)(void *dst, const void *src, u64 size) = apex::dispatcher_copy_memory;

void *optimized_fill_memory_no_sse(void *dst, char c, u64 len) {
    u64 dstp = (u64) dst;
    if (len >= 8) {
        size_t xlen;
        u64 cccc;
        cccc = (u8) c;
        cccc |= cccc << 8;
        cccc |= cccc << 16;
        if (OPSIZ > 4)
            /* Do the shift in two steps to avoid warning if long has 32 bits.  */
            cccc |= (cccc << 16) << 16;
        /* There are at least some bytes to set.
         No need to test for LEN == 0 in this alignment loop.  */
        while (dstp % OPSIZ != 0) {
            ((byte *) dstp)[0] = c;
            dstp += 1;
            len -= 1;
        }
        /* Write 8 `u64' per iteration until less than 8 `u64' remain.  */
        xlen = len / (OPSIZ * 8);
        while (xlen > 0) {
            ((u64 *) dstp)[0]  = cccc;
            ((u64 *) dstp)[1] = cccc;
            ((u64 *) dstp)[2] = cccc;
            ((u64 *) dstp)[3] = cccc;
            ((u64 *) dstp)[4] = cccc;
            ((u64 *) dstp)[5] = cccc;
            ((u64 *) dstp)[6] = cccc;
            ((u64 *) dstp)[7] = cccc;
            dstp += 8 * OPSIZ;
            xlen -= 1;
        }
        len %= OPSIZ * 8;
        /* Write 1 `u64' per iteration until less than OPSIZ bytes remain.  */
        xlen = len / OPSIZ;
        while (xlen > 0) {
            ((u64 *) dstp)[0] = cccc;
            dstp += OPSIZ;
            xlen -= 1;
        }
        len %= OPSIZ;
    }
    /* Write the last few bytes.  */
    while (len > 0) {
        ((byte *) dstp)[0] = c;
        dstp += 1;
        len -= 1;
    }
    return dst;
}

file_scope void fill_single_byte(void *dst, char c, u64 size) {
    // This needs to be marked volatile to avoid compiler optimizing with memset intrinsic
    // and thus causing a stack overflow in optimized_fill_memory.
    auto *b = (volatile byte *) dst;
    while (size--) *b++ = c;
}

void *optimized_fill_memory(void *dst, char c, u64 size) {
    char *d = (char *) dst;

#if ARCH == X86
    // Try SSE2...
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

    return dst;
#else
    // If no SSE2 then still do an optimized by filling words at a time
    return optimized_fill_memory_no_sse((char *) dst, c, size);
#endif
}

extern int memcmp_common_alignment(u64 srcp1, u64 srcp2, size_t len);
extern int memcmp_not_common_alignment(u64 srcp1, u64 srcp2, size_t len);

s32 optimized_compare_memory(const void *s1, const void *s2, u64 len) {
    u64 a0, b0;
    s32 res;

    u64 srcp1 = (u64) s1;
    u64 srcp2 = (u64) s2;

    if (len >= OP_T_THRES) {
        /* There are at least some bytes to compare.  No need to test
	 for LEN == 0 in this alignment loop.  */
        while (srcp2 % OPSIZ != 0) {
            a0 = ((byte *) srcp1)[0];
            b0 = ((byte *) srcp2)[0];
            srcp1 += 1;
            srcp2 += 1;
            res = (s32) (a0 - b0);
            if (res != 0)
                return res;
            len -= 1;
        }

        /* SRCP2 is now aligned for memory operations on `u64'.
	 SRCP1 alignment determines if we can do a simple,
	 aligned compare or need to shuffle bits.  */

        if (srcp1 % OPSIZ == 0)
            res = memcmp_common_alignment(srcp1, srcp2, len / OPSIZ);
        else
            res = memcmp_not_common_alignment(srcp1, srcp2, len / OPSIZ);
        if (res != 0)
            return res;

        /* Number of bytes remaining in the interval [0..OPSIZ-1].  */
        srcp1 += len & -OPSIZ;
        srcp2 += len & -OPSIZ;
        len %= OPSIZ;
    }

    /* There are just a few bytes to compare.  Use byte memory operations.  */
    while (len != 0) {
        a0 = ((byte *) srcp1)[0];
        b0 = ((byte *) srcp2)[0];
        srcp1 += 1;
        srcp2 += 1;
        res = (s32) (a0 - b0);
        if (res != 0)
            return res;
        len -= 1;
    }

    return 0;
}

void *(*fill_memory_fast)(void *dst, char value, u64 size)           = optimized_fill_memory;
s32 (*compare_memory_fast)(const void *s1, const void *s2, u64 size) = optimized_compare_memory;

LSTD_END_NAMESPACE
