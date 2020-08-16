#include "common.h"

#include <math.h>  // @DependencyCleanup

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
#endif

#if ARCH == X86
void tiberium(void *dest, const void *src, s64 num) {
    if (num <= 112) {
        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) src);
            if (num > 16) {
                if (num >= 32) {
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                    if (num > 32) {
                        s64 rax = *(s64 *) ((char *) src + num - 16);
                        s64 rcx = *(s64 *) ((char *) src + num - 8);
                        if (num > 48) {
                            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + 32));
                            if (num > 64) {
                                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + 48));
                                if (num > 80) {
                                    auto xmm4 = _mm_loadu_si128((__m128i *) ((char *) src + 64));
                                    if (num > 96) {
                                        auto xmm5 = _mm_loadu_si128((__m128i *) ((char *) src + 80));
                                        *(s64 *) ((char *) dest + num - 16) = rax;
                                        *(s64 *) ((char *) dest + num - 8) = rcx;
                                        _mm_storeu_si128((__m128i *) dest, xmm0);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 48), xmm3);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 64), xmm4);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 80), xmm5);
                                        return;
                                    }
                                    *(s64 *) ((char *) dest + num - 16) = rax;
                                    *(s64 *) ((char *) dest + num - 8) = rcx;
                                    _mm_storeu_si128((__m128i *) dest, xmm0);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 64), xmm4);
                                    return;
                                }
                                *(s64 *) ((char *) dest + num - 16) = rax;
                                *(s64 *) ((char *) dest + num - 8) = rcx;
                                _mm_storeu_si128((__m128i *) dest, xmm0);
                                _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((char *) dest + 48), xmm3);
                                return;
                            }
                            *(s64 *) ((char *) dest + num - 16) = rax;
                            *(s64 *) ((char *) dest + num - 8) = rcx;
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                            return;
                        }
                        *(s64 *) ((char *) dest + num - 16) = rax;
                        *(s64 *) ((char *) dest + num - 8) = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                    return;
                }
                s64 rax = *(s64 *) ((char *) src + num - 16);
                s64 rcx = *(s64 *) ((char *) src + num - 8);
                *(s64 *) ((char *) dest + num - 16) = rax;
                *(s64 *) ((char *) dest + num - 8) = rcx;
            }
            _mm_storeu_si128((__m128i *) dest, xmm0);
            return;
        }
        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((char *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((char *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((char *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((char *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            char al = *(char *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((char *) src + num - 2);
                *(char *) dest = al;
                *(s16 *) ((char *) dest + num - 2) = cx;
            } else
                *(char *) dest = al;
        }
        return;
    }

    if (((s64) dest - (s64) src) >= num) {
        if (num < (1024 * 256)) {
            s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
            dest = (char *) dest + offset;    // "Point to the end"
            src = (char *) src + offset;      // "Point to the end"
            num -= offset;                    // "Remaining data after loop"
            offset = -offset;                 // "Negative index from the end"

            do {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_storeu_si128((__m128i *) ((char *) dest + offset), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dest + offset + 16), xmm1);
                _mm_storeu_si128((__m128i *) ((char *) dest + offset + 32), xmm2);
                _mm_storeu_si128((__m128i *) ((char *) dest + offset + 48), xmm3);
            } while (offset += 64);

            if (num >= 16) {
                auto xmm0 = _mm_loadu_si128((__m128i *) src);
                if (num > 16) {
                    auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + num - 16));
                    if (num > 32) {
                        auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                        if (num > 48) {
                            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + 32));
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                            _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm3);
                            return;
                        }
                        _mm_storeu_si128((__m128i *) dest, xmm0);
                        _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm3);
                        return;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm3);
                    return;
                }
                _mm_storeu_si128((__m128i *) dest, xmm0);
                return;
            }
        } else  // do forward streaming copy/move
        {
            // We MUST do prealignment on streaming copies!
            auto prealign = -(s64) dest & 0xf;
            if (prealign) {
                if (prealign >= 8) {
                    s64 rax = *(s64 *) src;
                    if (prealign > 8) {
                        s64 rcx = *(s64 *) ((char *) src + prealign - 8);
                        *(s64 *) dest = rax;
                        *(s64 *) ((char *) dest + prealign - 8) = rcx;
                    } else
                        *(s64 *) dest = rax;
                } else if (prealign >= 4) {
                    s32 eax = *(s32 *) src;
                    if (prealign > 4) {
                        s32 ecx = *(s32 *) ((char *) src + prealign - 4);
                        *(s32 *) dest = eax;
                        *(s32 *) ((char *) dest + prealign - 4) = ecx;
                    } else
                        *(s32 *) dest = eax;
                } else {
                    char al = *(char *) src;
                    if (prealign > 1) {
                        s16 cx = *(s16 *) ((char *) src + prealign - 2);
                        *(char *) dest = al;
                        *(s16 *) ((char *) dest + prealign - 2) = cx;
                    } else
                        *(char *) dest = al;
                }
                src = (char *) src + prealign;
                dest = (char *) dest + prealign;
                num -= prealign;
            }

            // Begin prefetching upto 4KB
            for (s64 offset = 0; offset < 4096; offset += 256) {
                _mm_prefetch(((char *) src + offset), _MM_HINT_NTA);
                _mm_prefetch(((char *) src + offset + 64), _MM_HINT_NTA);
                _mm_prefetch(((char *) src + offset + 128), _MM_HINT_NTA);
                _mm_prefetch(((char *) src + offset + 192), _MM_HINT_NTA);
            }

            s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
            num -= offset;                    // "Remaining data after loop"
            offset -= 4096;                   // stage 1 INCLUDES prefetches
            dest = (char *) dest + offset;    // "Point to the end"
            src = (char *) src + offset;      // "Point to the end"
            offset = -offset;                 // "Negative index from the end"

            do  // stage 1 ~~ WITH prefetching
            {
                _mm_prefetch((char *) src + offset + 4096, _MM_HINT_NTA);
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm3);
            } while (offset += 64);

            offset = -4096;
            dest = (char *) dest + 4096;
            src = (char *) src + 4096;

            _mm_prefetch(((char *) src + num - 64), _MM_HINT_NTA);  // prefetch the final tail section

            do  // stage 2 ~~ WITHOUT further prefetching
            {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm3);
            } while (offset += 64);

            if (num >= 16) {
                auto xmm0 = _mm_loadu_si128((__m128i *) src);
                if (num > 16) {
                    if (num > 32) {
                        auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                        auto xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + num - 32));
                        auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num - 16));
                        _mm_stream_si128((__m128i *) dest, xmm0);
                        _mm_stream_si128((__m128i *) ((char *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dest + num - 32), xmm6);
                        _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm7);
                        return;
                    }
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num - 16));
                    _mm_stream_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm7);
                    return;
                }
                _mm_stream_si128((__m128i *) dest, xmm0);
                return;
            }
        }

        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((char *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((char *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((char *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((char *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            char al = *(char *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((char *) src + num - 2);
                *(char *) dest = al;
                *(s16 *) ((char *) dest + num - 2) = cx;
            } else
                *(char *) dest = al;
        }
        return;
    }  // src < dest ... do reverse copy
    src = (char *) src + num;
    dest = (char *) dest + num;

    if (num < (1024 * 256)) {
        s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
        dest = (char *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (char *) src - offset;      // "Point to the end" ... actually, we point to the start!
        num -= offset;                    // "Remaining data after loop"
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 64;
        do {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_storeu_si128((__m128i *) ((char *) dest + offset + 48), xmm0);
            _mm_storeu_si128((__m128i *) ((char *) dest + offset + 32), xmm1);
            _mm_storeu_si128((__m128i *) ((char *) dest + offset + 16), xmm2);
            _mm_storeu_si128((__m128i *) ((char *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
            if (num > 16) {
                num = -num;
                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + num));
                if (num > 32) {
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src - 32));
                    if (num > 48) {
                        auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src - 48));
                        _mm_storeu_si128((__m128i *) ((char *) dest - 16), xmm0);
                        _mm_storeu_si128((__m128i *) ((char *) dest - 32), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dest - 48), xmm2);
                        _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm3);
                        return;
                    }
                    _mm_storeu_si128((__m128i *) ((char *) dest - 16), xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm3);
                    return;
                }
                _mm_storeu_si128((__m128i *) ((char *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm3);
                return;
            }
            _mm_storeu_si128((__m128i *) ((char *) dest - 16), xmm0);
            return;
        }
    } else  // do reversed streaming copy/move
    {
        // We MUST do prealignment on streaming copies!
        auto prealign = (s64) dest & 0xf;
        if (prealign) {
            src = (char *) src - prealign;
            dest = (char *) dest - prealign;
            num -= prealign;
            if (prealign >= 8) {
                s64 rax = *(s64 *) ((char *) src + prealign - 8);
                if (prealign > 8) {
                    s64 rcx = *(s64 *) src;
                    *(s64 *) ((char *) dest + prealign - 8) = rax;
                    *(s64 *) dest = rcx;
                } else
                    *(s64 *) dest = rax;  // different on purpose, because we know the exact num now,
                // which is 8, and "dest" has already been aligned!
            } else if (prealign >= 4) {
                s32 eax = *(s32 *) ((char *) src + prealign - 4);
                if (prealign > 4) {
                    s32 ecx = *(s32 *) src;
                    *(s32 *) ((char *) dest + prealign - 4) = eax;
                    *(s32 *) dest = ecx;
                } else
                    *(s32 *) dest = eax;  // different on purpose!
            } else {
                char al = *(char *) ((char *) src + prealign - 1);
                if (prealign > 1) {
                    s16 cx = *(s16 *) src;
                    *(char *) ((char *) dest + prealign - 1) = al;
                    *(s16 *) dest = cx;
                } else
                    *(char *) dest = al;  // different on purpose!
            }
        }

        // Begin prefetching upto 4KB
        for (s64 offset = 0; offset > -4096; offset -= 256) {
            _mm_prefetch(((char *) src + offset - 64), _MM_HINT_NTA);
            _mm_prefetch(((char *) src + offset - 128), _MM_HINT_NTA);
            _mm_prefetch(((char *) src + offset - 192), _MM_HINT_NTA);
            _mm_prefetch(((char *) src + offset - 256), _MM_HINT_NTA);
        }

        s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
        num -= offset;                    // "Remaining data after loop"
        offset -= 4096;                   // stage 1 INCLUDES prefetches
        dest = (char *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (char *) src - offset;      // "Point to the end" ... actually, we point to the start!
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 64;
        do  // stage 1 ~~ WITH prefetching
        {
            _mm_prefetch((char *) src + offset - 4096, _MM_HINT_NTA);
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        offset = 4096;
        dest = (char *) dest - 4096;
        src = (char *) src - 4096;

        _mm_prefetch(((char *) src - 64), _MM_HINT_NTA);  // prefetch the final tail section

        offset -= 64;
        do  // stage 2 ~~ WITHOUT further prefetching
        {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
            if (num > 16) {
                if (num > 32) {
                    num = -num;
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src - 32));
                    auto xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + num + 16));
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num));
                    _mm_stream_si128((__m128i *) ((char *) dest - 16), xmm0);
                    _mm_stream_si128((__m128i *) ((char *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num + 16), xmm6);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm7);
                    return;
                }
                num = -num;
                auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num));
                _mm_stream_si128((__m128i *) ((char *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm7);
                return;
            }
            _mm_stream_si128((__m128i *) ((char *) dest - 16), xmm0);
            return;
        }
    }

    if (num >= 8) {
        s64 rax = *(s64 *) ((char *) src - 8);
        if (num > 8) {
            num = -num;  // that's right, we're converting an unsigned value to a negative, saves 2 clock cycles!
            s64 rcx = *(s64 *) ((char *) src + num);
            *(s64 *) ((char *) dest - 8) = rax;
            *(s64 *) ((char *) dest + num) = rcx;
        } else
            *(s64 *) ((char *) dest - 8) = rax;
    } else if (num >= 4) {
        s32 eax = *(s32 *) ((char *) src - 4);
        if (num > 4) {
            num = -num;
            s32 ecx = *(s32 *) ((char *) src + num);
            *(s32 *) ((char *) dest - 4) = eax;
            *(s32 *) ((char *) dest + num) = ecx;
        } else
            *(s32 *) ((char *) dest - 4) = eax;
    } else if (num >= 1) {
        char al = *((char *) src - 1);
        if (num > 1) {
            num = -num;
            s16 cx = *(s16 *) ((char *) src + num);
            *((char *) dest - 1) = al;
            *(s16 *) ((char *) dest + num) = cx;
        } else
            *((char *) dest - 1) = al;
    }
}

void kryptonite(void *dest, const void *src, s64 num) {
    if (num <= 112) {
        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) src);
            if (num > 16) {
                if (num >= 32) {
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                    if (num > 32) {
                        s64 rax = *(s64 *) ((char *) src + num - 16);
                        s64 rcx = *(s64 *) ((char *) src + num - 8);
                        if (num > 48) {
                            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + 32));
                            if (num > 64) {
                                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + 48));
                                if (num > 80) {
                                    auto xmm4 = _mm_loadu_si128((__m128i *) ((char *) src + 64));
                                    if (num > 96) {
                                        auto xmm5 = _mm_loadu_si128((__m128i *) ((char *) src + 80));
                                        *(s64 *) ((char *) dest + num - 16) = rax;
                                        *(s64 *) ((char *) dest + num - 8) = rcx;
                                        _mm_storeu_si128((__m128i *) dest, xmm0);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 48), xmm3);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 64), xmm4);
                                        _mm_storeu_si128((__m128i *) ((char *) dest + 80), xmm5);
                                        return;
                                    }
                                    *(s64 *) ((char *) dest + num - 16) = rax;
                                    *(s64 *) ((char *) dest + num - 8) = rcx;
                                    _mm_storeu_si128((__m128i *) dest, xmm0);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((char *) dest + 64), xmm4);
                                    return;
                                }
                                *(s64 *) ((char *) dest + num - 16) = rax;
                                *(s64 *) ((char *) dest + num - 8) = rcx;
                                _mm_storeu_si128((__m128i *) dest, xmm0);
                                _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((char *) dest + 48), xmm3);
                                return;
                            }
                            *(s64 *) ((char *) dest + num - 16) = rax;
                            *(s64 *) ((char *) dest + num - 8) = rcx;
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((char *) dest + 32), xmm2);
                            return;
                        }
                        *(s64 *) ((char *) dest + num - 16) = rax;
                        *(s64 *) ((char *) dest + num - 8) = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dest + 16), xmm1);
                    return;
                }
                s64 rax = *(s64 *) ((char *) src + num - 16);
                s64 rcx = *(s64 *) ((char *) src + num - 8);
                *(s64 *) ((char *) dest + num - 16) = rax;
                *(s64 *) ((char *) dest + num - 8) = rcx;
            }
            _mm_storeu_si128((__m128i *) dest, xmm0);
            return;
        }
        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((char *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((char *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((char *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((char *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            char al = *(char *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((char *) src + num - 2);
                *(char *) dest = al;
                *(s16 *) ((char *) dest + num - 2) = cx;
            } else
                *(char *) dest = al;
        }
        return;
    }

    if (((s64) dest - (s64) src) >= num) {
        if (num < (1024 * 256)) {
            s64 offset = (s64)(num & -0x20);  // "Round down to nearest multiple of 64"
            dest = (char *) dest + offset;    // "Point to the end"
            src = (char *) src + offset;      // "Point to the end"
            num -= offset;                    // "Remaining data after loop"
            offset = -offset;                 // "Negative index from the end"

            do {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                _mm_storeu_si128((__m128i *) ((char *) dest + offset), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dest + offset + 16), xmm1);
            } while (offset += 32);

            if (num >= 16) {
                if (num > 16) {
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num - 16));
                    auto xmm0 = _mm_loadu_si128((__m128i *) src);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm7);
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    return;
                }
                _mm_storeu_si128((__m128i *) dest, _mm_loadu_si128((__m128i *) src));
                return;
            }
        } else  // do forward streaming copy/move
        {
            // We MUST do prealignment on streaming copies!
            auto prealign = -(s64) dest & 0xf;
            if (prealign) {
                if (prealign >= 8) {
                    s64 rax = *(s64 *) src;
                    if (prealign > 8) {
                        s64 rcx = *(s64 *) ((char *) src + prealign - 8);
                        *(s64 *) dest = rax;
                        *(s64 *) ((char *) dest + prealign - 8) = rcx;
                    } else
                        *(s64 *) dest = rax;
                } else if (prealign >= 4) {
                    s32 eax = *(s32 *) src;
                    if (prealign > 4) {
                        s32 ecx = *(s32 *) ((char *) src + prealign - 4);
                        *(s32 *) dest = eax;
                        *(s32 *) ((char *) dest + prealign - 4) = ecx;
                    } else
                        *(s32 *) dest = eax;
                } else {
                    char al = *(char *) src;
                    if (prealign > 1) {
                        s16 cx = *(s16 *) ((char *) src + prealign - 2);
                        *(char *) dest = al;
                        *(s16 *) ((char *) dest + prealign - 2) = cx;
                    } else
                        *(char *) dest = al;
                }
                src = (char *) src + prealign;
                dest = (char *) dest + prealign;
                num -= prealign;
            }

            // Begin prefetching upto 4KB
            for (s64 offset = 0; offset < 4096; offset += 256) {
                _mm_prefetch(((char *) src + offset), _MM_HINT_NTA);
                _mm_prefetch(((char *) src + offset + 64), _MM_HINT_NTA);
                _mm_prefetch(((char *) src + offset + 128), _MM_HINT_NTA);
                _mm_prefetch(((char *) src + offset + 192), _MM_HINT_NTA);
            }

            s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
            num -= offset;                    // "Remaining data after loop"
            offset -= 4096;                   // stage 1 INCLUDES prefetches
            dest = (char *) dest + offset;    // "Point to the end"
            src = (char *) src + offset;      // "Point to the end"
            offset = -offset;                 // "Negative index from the end"

            do  // stage 1 ~~ WITH prefetching
            {
                _mm_prefetch((char *) src + offset + 4096, _MM_HINT_NTA);
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm3);
            } while (offset += 64);

            offset = -4096;
            dest = (char *) dest + 4096;
            src = (char *) src + 4096;

            _mm_prefetch(((char *) src + num - 64), _MM_HINT_NTA);  // prefetch the final tail section

            do  // stage 2 ~~ WITHOUT further prefetching
            {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm3);
            } while (offset += 64);

            if (num >= 16) {
                auto xmm0 = _mm_loadu_si128((__m128i *) src);
                if (num > 16) {
                    if (num > 32) {
                        auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + 16));
                        auto xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + num - 32));
                        auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num - 16));
                        _mm_stream_si128((__m128i *) dest, xmm0);
                        _mm_stream_si128((__m128i *) ((char *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((char *) dest + num - 32), xmm6);
                        _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm7);
                        return;
                    }
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num - 16));
                    _mm_stream_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num - 16), xmm7);
                    return;
                }
                _mm_stream_si128((__m128i *) dest, xmm0);
                return;
            }
        }

        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((char *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((char *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((char *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((char *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            char al = *(char *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((char *) src + num - 2);
                *(char *) dest = al;
                *(s16 *) ((char *) dest + num - 2) = cx;
            } else
                *(char *) dest = al;
        }
        return;
    }  // src < dest ... do reverse copy
    src = (char *) src + num;
    dest = (char *) dest + num;

    if (num < (1024 * 256)) {
        s64 offset = (s64)(num & -0x20);  // "Round down to nearest multiple of 64"
        dest = (char *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (char *) src - offset;      // "Point to the end" ... actually, we point to the start!
        num -= offset;                    // "Remaining data after loop"
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 32;
        do {
            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_storeu_si128((__m128i *) ((char *) dest + offset + 16), xmm2);
            _mm_storeu_si128((__m128i *) ((char *) dest + offset), xmm3);
        } while ((offset -= 32) >= 0);

        if (num >= 16) {
            if (num > 16) {
                num = -num;
                // The order has been mixed so the compiler will not re-order the statements!
                auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num));
                auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
                _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm7);
                _mm_storeu_si128((__m128i *) ((char *) dest - 16), xmm0);
                return;
            }
            _mm_storeu_si128((__m128i *) ((char *) dest - 16), _mm_loadu_si128((__m128i *) ((char *) src - 16)));
            return;
        }
    } else  // do reversed streaming copy/move
    {
        // We MUST do prealignment on streaming copies!
        auto prealign = (s64) dest & 0xf;
        if (prealign) {
            src = (char *) src - prealign;
            dest = (char *) dest - prealign;
            num -= prealign;
            if (prealign >= 8) {
                s64 rax = *(s64 *) ((char *) src + prealign - 8);
                if (prealign > 8) {
                    s64 rcx = *(s64 *) src;
                    *(s64 *) ((char *) dest + prealign - 8) = rax;
                    *(s64 *) dest = rcx;
                } else
                    *(s64 *) dest = rax;  // different on purpose, because we know the exact num now,
                // which is 8, and "dest" has already been aligned!
            } else if (prealign >= 4) {
                s32 eax = *(s32 *) ((char *) src + prealign - 4);
                if (prealign > 4) {
                    s32 ecx = *(s32 *) src;
                    *(s32 *) ((char *) dest + prealign - 4) = eax;
                    *(s32 *) dest = ecx;
                } else
                    *(s32 *) dest = eax;  // different on purpose!
            } else {
                char al = *(char *) ((char *) src + prealign - 1);
                if (prealign > 1) {
                    s16 cx = *(s16 *) src;
                    *(char *) ((char *) dest + prealign - 1) = al;
                    *(s16 *) dest = cx;
                } else
                    *(char *) dest = al;  // different on purpose!
            }
        }

        // Begin prefetching upto 4KB
        for (s64 offset = 0; offset > -4096; offset -= 256) {
            _mm_prefetch(((char *) src + offset - 64), _MM_HINT_NTA);
            _mm_prefetch(((char *) src + offset - 128), _MM_HINT_NTA);
            _mm_prefetch(((char *) src + offset - 192), _MM_HINT_NTA);
            _mm_prefetch(((char *) src + offset - 256), _MM_HINT_NTA);
        }

        s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
        num -= offset;                    // "Remaining data after loop"
        offset -= 4096;                   // stage 1 INCLUDES prefetches
        dest = (char *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (char *) src - offset;      // "Point to the end" ... actually, we point to the start!
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 64;
        do  // stage 1 ~~ WITH prefetching
        {
            _mm_prefetch((char *) src + offset - 4096, _MM_HINT_NTA);
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        offset = 4096;
        dest = (char *) dest - 4096;
        src = (char *) src - 4096;

        _mm_prefetch(((char *) src - 64), _MM_HINT_NTA);  // prefetch the final tail section

        offset -= 64;
        do  // stage 2 ~~ WITHOUT further prefetching
        {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((char *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((char *) src + offset));
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((char *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((char *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((char *) src - 16));
            if (num > 16) {
                if (num > 32) {
                    num = -num;
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((char *) src - 32));
                    auto xmm6 = _mm_loadu_si128((__m128i *) ((char *) src + num + 16));
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num));
                    _mm_stream_si128((__m128i *) ((char *) dest - 16), xmm0);
                    _mm_stream_si128((__m128i *) ((char *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num + 16), xmm6);
                    _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm7);
                    return;
                }
                num = -num;
                auto xmm7 = _mm_loadu_si128((__m128i *) ((char *) src + num));
                _mm_stream_si128((__m128i *) ((char *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((char *) dest + num), xmm7);
                return;
            }
            _mm_stream_si128((__m128i *) ((char *) dest - 16), xmm0);
            return;
        }
    }

    if (num >= 8) {
        s64 rax = *(s64 *) ((char *) src - 8);
        if (num > 8) {
            num = -num;  // that's right, we're converting an unsigned value to a negative, saves 2 clock cycles!
            auto rcx = *(s64 *) ((char *) src + num);
            *(s64 *) ((char *) dest - 8) = rax;
            *(s64 *) ((char *) dest + num) = rcx;
        } else
            *(s64 *) ((char *) dest - 8) = rax;
    } else if (num >= 4) {
        s32 eax = *(s32 *) ((char *) src - 4);
        if (num > 4) {
            num = -num;
            s32 ecx = *(s32 *) ((char *) src + num);
            *(s32 *) ((char *) dest - 4) = eax;
            *(s32 *) ((char *) dest + num) = ecx;
        } else
            *(s32 *) ((char *) dest - 4) = eax;
    } else if (num >= 1) {
        char al = *((char *) src - 1);
        if (num > 1) {
            num = -num;
            s16 cx = *(s16 *) ((char *) src + num);
            *((char *) dest - 1) = al;
            *(s16 *) ((char *) dest + num) = cx;
        } else
            *((char *) dest - 1) = al;
    }
    return;
}
#endif  // end test for x86 arch

#if COMPILER == MSVC
#pragma warning(pop)
#endif

// This sets up copy_memory and move_memory the first time it's called
file_scope void dispatcher(void *dest, const void *src, s64 num) {
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
        copy_memory = &apex::kryptonite;
    } else {
        copy_memory = &apex::tiberium;
    }
#else
    copy_memory = &const_copy_memory
#endif
    // Once we set it, actually run it
    copy_memory(dest, src, num);
}
}  // namespace apex

void (*copy_memory)(void *dest, const void *src, s64 num) = apex::dispatcher;

//
// SSE optimized fill_memory
// If the platform doesn't support SSE, it still writes 4 bytes at a time (instead of 16)
//

void optimized_fill_memory(void *dest, char c, s64 num) {
    char *d = (char *) dest;

#if ARCH == X86
    s64 offset = ((s64) dest) % 16;
    s64 num16bytes = (num - offset) / 16;
    s64 remaining = num - num16bytes * 16 - offset;
    if (num < offset) {
        offset = 0;
        num16bytes = 0;
        remaining = num;
    }

    const_fill_memory(d, c, offset);
    d += offset;

    __m128i c16 = _mm_set_epi8(c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c);
    For(range(num16bytes)) {
        _mm_store_si128((__m128i *) d, c16);
        d += 16;
    }
    const_fill_memory(d, c, remaining);
#else
    s64 offset = ((s64) dest) % 4;
    s64 num4bytes = (num - offset) / 4;
    s64 remaining = num - num16bytes * 4 - offset;

    const_fill_memory(d, c, offset);
    d += offset;

    union {
        u32 Word;
        char Bytes[4] = {c, c, c, c};
    } c4;

    For(range(num4bytes)) {
        *((u32 *) d) = c4.Word;
        d += 4;
    }
    const_fill_memory(d, c, remaining);
#endif
}

void (*fill_memory)(void *dest, char value, s64 num) = optimized_fill_memory;

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

file_scope s64 compare_memory_common_alignment(const char *s1, const char *s2, s64 num) {
    s64 progress = 0;
    u32 a0, a1;
    u32 b0, b1;

    switch (num % 4) {
        default:
        case 2:
            a0 = *((u32 *) s1);
            b0 = *((u32 *) s2);
            s1 -= 8;
            s2 -= 8;
            num += 2;
            goto do1;
        case 3:
            a1 = *((u32 *) s1);
            b1 = *((u32 *) s2);
            s1 -= 4;
            s2 -= 4;
            num += 1;
            goto do2;
        case 0:
            a0 = *((u32 *) s1);
            b0 = *((u32 *) s2);
            goto do3;
        case 1:
            a1 = *((u32 *) s1);
            b1 = *((u32 *) s2);
            s1 += 4;
            s2 += 4;
            num -= 1;
    }
    do {
        a0 = *((u32 *) s1);
        b0 = *((u32 *) s2);
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
        num -= 4;
    } while (num != 0);

    if (a1 != b1) return num + compare_bytes_of_two_u32s(a1, b1);
    return -1;
}

#if ENDIAN == LITTLE_ENDIAN
#define MERGE(w0, sh_1, w1, sh_2) (((w0) >> (sh_1)) | ((w1) << (sh_2)))
#else
#define MERGE(w0, sh_1, w1, sh_2) (((w0) << (sh_1)) | ((w1) >> (sh_2)))
#endif

file_scope s64 compare_memory_not_common_alignment(const char *s1, const char *s2, s64 num) {
    s64 progress = 0;
    u32 a0, a1, a2, a3;
    u32 b0, b1, b2, b3;
    u32 x;
    s32 shl, shr;

    // Calculate how to shift a word read at the memory operation aligned srcp1 to make it aligned for comparison.
    shl = 8 * ((s64) s1 % 4);
    shr = 8 * 4 - shl;

    // Make SRCP1 aligned by rounding it down to the beginning of the u32 it points in the middle of.
    s1 = (const char *) ((u64) s1 & -4);
    switch (num % 4) {
        default:
        case 2:
            a1 = *((u32 *) s1);
            a2 = *((u32 *) s1 + 1);
            b2 = *((u32 *) s2);
            s1 -= 4;
            s2 -= 8;
            num += 2;
            goto do1;
        case 3:
            a0 = *((u32 *) s1);
            a1 = *((u32 *) s1 + 1);
            b1 = *((u32 *) s2);
            s2 -= 4;
            num += 1;
            goto do2;
        case 0:
            a3 = *((u32 *) s1);
            a0 = *((u32 *) s1 + 1);
            b0 = *((u32 *) s2);
            s1 += 4;
            goto do3;
        case 1:
            a2 = *((u32 *) s1);
            a3 = *((u32 *) s1 + 1);
            b3 = *((u32 *) s2);
            s1 += 8;
            s2 += 4;
            num -= 1;
    }
    do {
        a0 = *((u32 *) s1);
        b0 = *((u32 *) s2);
        x = MERGE(a2, shl, a3, shr);
        if (x != b3) return progress + compare_bytes_of_two_u32s(x, b3);
        progress += 4;
    do3:
        a1 = *((u32 *) s1 + 1);
        b1 = *((u32 *) s2 + 1);
        x = MERGE(a3, shl, a0, shr);
        if (x != b0) return progress + compare_bytes_of_two_u32s(x, b0);
        progress += 4;
    do2:
        a2 = *((u32 *) s1 + 2);
        b2 = *((u32 *) s2 + 2);
        x = MERGE(a0, shl, a1, shr);
        if (x != b1) return progress + compare_bytes_of_two_u32s(x, b1);
        progress += 4;
    do1:
        a3 = *((u32 *) s1 + 3);
        b3 = *((u32 *) s2 + 3);
        x = MERGE(a1, shl, a2, shr);
        if (x != b2) return progress + compare_bytes_of_two_u32s(x, b2);
        progress += 4;
        s1 += 16;
        s2 += 16;
        num -= 4;
    } while (num != 0);

    x = MERGE(a2, shl, a3, shr);
    if (x != b3) return num + compare_bytes_of_two_u32s(x, b3);
    return -1;
}

s64 optimized_compare_memory(const void *ptr1, const void *ptr2, s64 num) {
    s64 progress = 0;

    auto s1 = (const char *) ptr1;
    auto s2 = (const char *) ptr2;

    if (num >= 16) {
        for (s64 index = 0; (s64) s2 % 4 != 0; ++index) {
            if (*s1++ != *s2++) return index;
            ++progress;
            --num;
        }

        s64 res;
        if ((u64) s1 % 4 == 0) {
            res = compare_memory_common_alignment(s1, s2, num / 4);
        } else {
            res = compare_memory_not_common_alignment(s1, s2, num / 4);
        }
        if (res != -1) return progress + res;

        // Number of bytes remaining in the interval [0..3]
        s1 += num & -4;
        s2 += num & -4;
        num %= 4;
    }

    // There are just a few bytes to compare. Use byte memory operations.
    for (s64 index = 0; num--; ++index) {
        if (*s1++ != *s2++) return progress + index;
    }
    return -1;
}

s64 (*compare_memory)(const void *ptr1, const void *ptr2, s64 num) = optimized_compare_memory;

f32 min(f32 x, f32 y) {
#if BUILD_NO_CRT
    if (is_nan(x) || is_nan(y)) return x + y;
    return x < y ? x : y;
#else
    return fminf(x, y);
#endif
}

f32 max(f32 x, f32 y) {
#if BUILD_NO_CRT
    if (is_nan(x) || is_nan(y)) return x + y;
    return x > y ? x : y;
#else
    return fmaxf(x, y);
#endif
}

f64 min(f64 x, f64 y) {
#if BUILD_NO_CRT
    if (is_nan(x) || is_nan(y)) return x + y;
    return x < y ? x : y;
#else
    return fmin(x, y);
#endif
}

f64 max(f64 x, f64 y) {
#if BUILD_NO_CRT
    if (is_nan(x) || is_nan(y)) return x + y;
    return x > y ? x : y;
#else
    return fmax(x, y);
#endif
}

LSTD_END_NAMESPACE
