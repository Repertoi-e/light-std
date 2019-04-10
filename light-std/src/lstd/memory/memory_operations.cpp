#include "../common.h"

// Implements optimized copy_memory, fill_memory and compare_memory

#if ARCH == X86
#include <emmintrin.h>  //Intel/AMD SSE intrinsics
#if COMPILER == MSVC
#include <intrin.h>  // __cpuid (Visual Studio)
#else
#include <cpuid.h>  // __get_cpuid (GCC / Clang)
#endif
#endif

LSTD_BEGIN_NAMESPACE

namespace apex {

// apex memmove (tiberium, kryptonite and mithril) memcpy/memmove functions written by Trevor Herselman in 2014
#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)  // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#pragma warning( \
    disable : 4244)  // warning C4244: '-=': conversion from '__int64' to 'std::size_t', possible loss of data
#endif

#if ARCH == X86
void tiberium(void *dest, const void *src, size_t num) {
    if (num <= 112) {
        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) src);
            if (num > 16) {
                if (num >= 32) {
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + 16));
                    if (num > 32) {
                        s64 rax = *(s64 *) ((byte *) src + num - 16);
                        s64 rcx = *(s64 *) ((byte *) src + num - 8);
                        if (num > 48) {
                            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + 32));
                            if (num > 64) {
                                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + 48));
                                if (num > 80) {
                                    auto xmm4 = _mm_loadu_si128((__m128i *) ((byte *) src + 64));
                                    if (num > 96) {
                                        auto xmm5 = _mm_loadu_si128((__m128i *) ((byte *) src + 80));
                                        *(s64 *) ((byte *) dest + num - 16) = rax;
                                        *(s64 *) ((byte *) dest + num - 8) = rcx;
                                        _mm_storeu_si128((__m128i *) dest, xmm0);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 64), xmm4);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 80), xmm5);
                                        return;
                                    }
                                    *(s64 *) ((byte *) dest + num - 16) = rax;
                                    *(s64 *) ((byte *) dest + num - 8) = rcx;
                                    _mm_storeu_si128((__m128i *) dest, xmm0);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 64), xmm4);
                                    return;
                                }
                                *(s64 *) ((byte *) dest + num - 16) = rax;
                                *(s64 *) ((byte *) dest + num - 8) = rcx;
                                _mm_storeu_si128((__m128i *) dest, xmm0);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                return;
                            }
                            *(s64 *) ((byte *) dest + num - 16) = rax;
                            *(s64 *) ((byte *) dest + num - 8) = rcx;
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                            return;
                        }
                        *(s64 *) ((byte *) dest + num - 16) = rax;
                        *(s64 *) ((byte *) dest + num - 8) = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                    return;
                }
                s64 rax = *(s64 *) ((byte *) src + num - 16);
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) ((byte *) dest + num - 16) = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            }
            _mm_storeu_si128((__m128i *) dest, xmm0);
            return;
        }
        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((byte *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((byte *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            byte al = *(byte *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((byte *) src + num - 2);
                *(byte *) dest = al;
                *(s16 *) ((byte *) dest + num - 2) = cx;
            } else
                *(byte *) dest = al;
        }
        return;
    }

    if (((size_t) dest - (size_t) src) >= num) {
        if (num < (1024 * 256)) {
            s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
            dest = (byte *) dest + offset;    // "Point to the end"
            src = (byte *) src + offset;      // "Point to the end"
            num -= offset;                    // "Remaining data after loop"
            offset = -offset;                 // "Negative index from the end"

            do {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                _mm_storeu_si128((__m128i *) ((byte *) dest + offset), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 16), xmm1);
                _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 32), xmm2);
                _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 48), xmm3);
            } while (offset += 64);

            if (num >= 16) {
                auto xmm0 = _mm_loadu_si128((__m128i *) src);
                if (num > 16) {
                    auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                    if (num > 32) {
                        auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + 16));
                        if (num > 48) {
                            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + 32));
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm3);
                            return;
                        }
                        _mm_storeu_si128((__m128i *) dest, xmm0);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm3);
                        return;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm3);
                    return;
                }
                _mm_storeu_si128((__m128i *) dest, xmm0);
                return;
            }
        } else  // do forward streaming copy/move
        {
            // We MUST do prealignment on streaming copies!
            auto prealign = -(size_t) dest & 0xf;
            if (prealign) {
                if (prealign >= 8) {
                    s64 rax = *(s64 *) src;
                    if (prealign > 8) {
                        s64 rcx = *(s64 *) ((byte *) src + prealign - 8);
                        *(s64 *) dest = rax;
                        *(s64 *) ((byte *) dest + prealign - 8) = rcx;
                    } else
                        *(s64 *) dest = rax;
                } else if (prealign >= 4) {
                    s32 eax = *(s32 *) src;
                    if (prealign > 4) {
                        s32 ecx = *(s32 *) ((byte *) src + prealign - 4);
                        *(s32 *) dest = eax;
                        *(s32 *) ((byte *) dest + prealign - 4) = ecx;
                    } else
                        *(s32 *) dest = eax;
                } else {
                    byte al = *(byte *) src;
                    if (prealign > 1) {
                        s16 cx = *(s16 *) ((byte *) src + prealign - 2);
                        *(byte *) dest = al;
                        *(s16 *) ((byte *) dest + prealign - 2) = cx;
                    } else
                        *(byte *) dest = al;
                }
                src = (byte *) src + prealign;
                dest = (byte *) dest + prealign;
                num -= prealign;
            }

            // Begin prefetching upto 4KB
            for (s64 offset = 0; offset < 4096; offset += 256) {
                _mm_prefetch(((byte *) src + offset), _MM_HINT_NTA);
                _mm_prefetch(((byte *) src + offset + 64), _MM_HINT_NTA);
                _mm_prefetch(((byte *) src + offset + 128), _MM_HINT_NTA);
                _mm_prefetch(((byte *) src + offset + 192), _MM_HINT_NTA);
            }

            s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
            num -= offset;                    // "Remaining data after loop"
            offset -= 4096;                   // stage 1 INCLUDES prefetches
            dest = (byte *) dest + offset;    // "Point to the end"
            src = (byte *) src + offset;      // "Point to the end"
            offset = -offset;                 // "Negative index from the end"

            do  // stage 1 ~~ WITH prefetching
            {
                _mm_prefetch((byte *) src + offset + 4096, _MM_HINT_NTA);
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm3);
            } while (offset += 64);

            offset = -4096;
            dest = (byte *) dest + 4096;
            src = (byte *) src + 4096;

            _mm_prefetch(((byte *) src + num - 64), _MM_HINT_NTA);  // prefetch the final tail section

            do  // stage 2 ~~ WITHOUT further prefetching
            {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm3);
            } while (offset += 64);

            if (num >= 16) {
                auto xmm0 = _mm_loadu_si128((__m128i *) src);
                if (num > 16) {
                    if (num > 32) {
                        auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + 16));
                        auto xmm6 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 32));
                        auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                        _mm_stream_si128((__m128i *) dest, xmm0);
                        _mm_stream_si128((__m128i *) ((byte *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num - 32), xmm6);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                        return;
                    }
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                    _mm_stream_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                    return;
                }
                _mm_stream_si128((__m128i *) dest, xmm0);
                return;
            }
        }

        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((byte *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((byte *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            byte al = *(byte *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((byte *) src + num - 2);
                *(byte *) dest = al;
                *(s16 *) ((byte *) dest + num - 2) = cx;
            } else
                *(byte *) dest = al;
        }
        return;
    }  // src < dest ... do reverse copy
    src = (byte *) src + num;
    dest = (byte *) dest + num;

    if (num < (1024 * 256)) {
        s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
        dest = (byte *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (byte *) src - offset;      // "Point to the end" ... actually, we point to the start!
        num -= offset;                    // "Remaining data after loop"
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 64;
        do {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
            _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 48), xmm0);
            _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 32), xmm1);
            _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 16), xmm2);
            _mm_storeu_si128((__m128i *) ((byte *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));
            if (num > 16) {
                num = -num;
                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                if (num > 32) {
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src - 32));
                    if (num > 48) {
                        auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src - 48));
                        _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
                        _mm_storeu_si128((__m128i *) ((byte *) dest - 32), xmm1);
                        _mm_storeu_si128((__m128i *) ((byte *) dest - 48), xmm2);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm3);
                        return;
                    }
                    _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm3);
                    return;
                }
                _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm3);
                return;
            }
            _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
            return;
        }
    } else  // do reversed streaming copy/move
    {
        // We MUST do prealignment on streaming copies!
        auto prealign = (size_t) dest & 0xf;
        if (prealign) {
            src = (byte *) src - prealign;
            dest = (byte *) dest - prealign;
            num -= prealign;
            if (prealign >= 8) {
                s64 rax = *(s64 *) ((byte *) src + prealign - 8);
                if (prealign > 8) {
                    s64 rcx = *(s64 *) src;
                    *(s64 *) ((byte *) dest + prealign - 8) = rax;
                    *(s64 *) dest = rcx;
                } else
                    *(s64 *) dest = rax;  // different on purpose, because we know the exact num now,
                // which is 8, and "dest" has already been aligned!
            } else if (prealign >= 4) {
                s32 eax = *(s32 *) ((byte *) src + prealign - 4);
                if (prealign > 4) {
                    s32 ecx = *(s32 *) src;
                    *(s32 *) ((byte *) dest + prealign - 4) = eax;
                    *(s32 *) dest = ecx;
                } else
                    *(s32 *) dest = eax;  // different on purpose!
            } else {
                byte al = *(byte *) ((byte *) src + prealign - 1);
                if (prealign > 1) {
                    s16 cx = *(s16 *) src;
                    *(byte *) ((byte *) dest + prealign - 1) = al;
                    *(s16 *) dest = cx;
                } else
                    *(byte *) dest = al;  // different on purpose!
            }
        }

        // Begin prefetching upto 4KB
        for (s64 offset = 0; offset > -4096; offset -= 256) {
            _mm_prefetch(((byte *) src + offset - 64), _MM_HINT_NTA);
            _mm_prefetch(((byte *) src + offset - 128), _MM_HINT_NTA);
            _mm_prefetch(((byte *) src + offset - 192), _MM_HINT_NTA);
            _mm_prefetch(((byte *) src + offset - 256), _MM_HINT_NTA);
        }

        s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
        num -= offset;                    // "Remaining data after loop"
        offset -= 4096;                   // stage 1 INCLUDES prefetches
        dest = (byte *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (byte *) src - offset;      // "Point to the end" ... actually, we point to the start!
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 64;
        do  // stage 1 ~~ WITH prefetching
        {
            _mm_prefetch((byte *) src + offset - 4096, _MM_HINT_NTA);
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        offset = 4096;
        dest = (byte *) dest - 4096;
        src = (byte *) src - 4096;

        _mm_prefetch(((byte *) src - 64), _MM_HINT_NTA);  // prefetch the final tail section

        offset -= 64;
        do  // stage 2 ~~ WITHOUT further prefetching
        {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));
            if (num > 16) {
                if (num > 32) {
                    num = -num;
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src - 32));
                    auto xmm6 = _mm_loadu_si128((__m128i *) ((byte *) src + num + 16));
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                    _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
                    _mm_stream_si128((__m128i *) ((byte *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num + 16), xmm6);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                    return;
                }
                num = -num;
                auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                return;
            }
            _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
            return;
        }
    }

    if (num >= 8) {
        s64 rax = *(s64 *) ((byte *) src - 8);
        if (num > 8) {
            num = -num;  // that's right, we're converting an unsigned value to a negative, saves 2 clock cycles!
            s64 rcx = *(s64 *) ((byte *) src + num);
            *(s64 *) ((byte *) dest - 8) = rax;
            *(s64 *) ((byte *) dest + num) = rcx;
        } else
            *(s64 *) ((byte *) dest - 8) = rax;
    } else if (num >= 4) {
        s32 eax = *(s32 *) ((byte *) src - 4);
        if (num > 4) {
            num = -num;
            s32 ecx = *(s32 *) ((byte *) src + num);
            *(s32 *) ((byte *) dest - 4) = eax;
            *(s32 *) ((byte *) dest + num) = ecx;
        } else
            *(s32 *) ((byte *) dest - 4) = eax;
    } else if (num >= 1) {
        byte al = *((byte *) src - 1);
        if (num > 1) {
            num = -num;
            s16 cx = *(s16 *) ((byte *) src + num);
            *((byte *) dest - 1) = al;
            *(s16 *) ((byte *) dest + num) = cx;
        } else
            *((byte *) dest - 1) = al;
    }
}

void kryptonite(void *dest, const void *src, size_t num) {
    if (num <= 112) {
        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) src);
            if (num > 16) {
                if (num >= 32) {
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + 16));
                    if (num > 32) {
                        s64 rax = *(s64 *) ((byte *) src + num - 16);
                        s64 rcx = *(s64 *) ((byte *) src + num - 8);
                        if (num > 48) {
                            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + 32));
                            if (num > 64) {
                                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + 48));
                                if (num > 80) {
                                    auto xmm4 = _mm_loadu_si128((__m128i *) ((byte *) src + 64));
                                    if (num > 96) {
                                        auto xmm5 = _mm_loadu_si128((__m128i *) ((byte *) src + 80));
                                        *(s64 *) ((byte *) dest + num - 16) = rax;
                                        *(s64 *) ((byte *) dest + num - 8) = rcx;
                                        _mm_storeu_si128((__m128i *) dest, xmm0);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 64), xmm4);
                                        _mm_storeu_si128((__m128i *) ((byte *) dest + 80), xmm5);
                                        return;
                                    }
                                    *(s64 *) ((byte *) dest + num - 16) = rax;
                                    *(s64 *) ((byte *) dest + num - 8) = rcx;
                                    _mm_storeu_si128((__m128i *) dest, xmm0);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 64), xmm4);
                                    return;
                                }
                                *(s64 *) ((byte *) dest + num - 16) = rax;
                                *(s64 *) ((byte *) dest + num - 8) = rcx;
                                _mm_storeu_si128((__m128i *) dest, xmm0);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                return;
                            }
                            *(s64 *) ((byte *) dest + num - 16) = rax;
                            *(s64 *) ((byte *) dest + num - 8) = rcx;
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                            return;
                        }
                        *(s64 *) ((byte *) dest + num - 16) = rax;
                        *(s64 *) ((byte *) dest + num - 8) = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                    return;
                }
                s64 rax = *(s64 *) ((byte *) src + num - 16);
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) ((byte *) dest + num - 16) = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            }
            _mm_storeu_si128((__m128i *) dest, xmm0);
            return;
        }
        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((byte *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((byte *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            byte al = *(byte *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((byte *) src + num - 2);
                *(byte *) dest = al;
                *(s16 *) ((byte *) dest + num - 2) = cx;
            } else
                *(byte *) dest = al;
        }
        return;
    }

    if (((size_t) dest - (size_t) src) >= num) {
        if (num < (1024 * 256)) {
            s64 offset = (s64)(num & -0x20);  // "Round down to nearest multiple of 64"
            dest = (byte *) dest + offset;    // "Point to the end"
            src = (byte *) src + offset;      // "Point to the end"
            num -= offset;                    // "Remaining data after loop"
            offset = -offset;                 // "Negative index from the end"

            do {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                _mm_storeu_si128((__m128i *) ((byte *) dest + offset), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 16), xmm1);
            } while (offset += 32);

            if (num >= 16) {
                if (num > 16) {
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                    auto xmm0 = _mm_loadu_si128((__m128i *) src);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    return;
                }
                _mm_storeu_si128((__m128i *) dest, _mm_loadu_si128((__m128i *) src));
                return;
            }
        } else  // do forward streaming copy/move
        {
            // We MUST do prealignment on streaming copies!
            auto prealign = -(size_t) dest & 0xf;
            if (prealign) {
                if (prealign >= 8) {
                    s64 rax = *(s64 *) src;
                    if (prealign > 8) {
                        s64 rcx = *(s64 *) ((byte *) src + prealign - 8);
                        *(s64 *) dest = rax;
                        *(s64 *) ((byte *) dest + prealign - 8) = rcx;
                    } else
                        *(s64 *) dest = rax;
                } else if (prealign >= 4) {
                    s32 eax = *(s32 *) src;
                    if (prealign > 4) {
                        s32 ecx = *(s32 *) ((byte *) src + prealign - 4);
                        *(s32 *) dest = eax;
                        *(s32 *) ((byte *) dest + prealign - 4) = ecx;
                    } else
                        *(s32 *) dest = eax;
                } else {
                    byte al = *(byte *) src;
                    if (prealign > 1) {
                        s16 cx = *(s16 *) ((byte *) src + prealign - 2);
                        *(byte *) dest = al;
                        *(s16 *) ((byte *) dest + prealign - 2) = cx;
                    } else
                        *(byte *) dest = al;
                }
                src = (byte *) src + prealign;
                dest = (byte *) dest + prealign;
                num -= prealign;
            }

            // Begin prefetching upto 4KB
            for (s64 offset = 0; offset < 4096; offset += 256) {
                _mm_prefetch(((byte *) src + offset), _MM_HINT_NTA);
                _mm_prefetch(((byte *) src + offset + 64), _MM_HINT_NTA);
                _mm_prefetch(((byte *) src + offset + 128), _MM_HINT_NTA);
                _mm_prefetch(((byte *) src + offset + 192), _MM_HINT_NTA);
            }

            s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
            num -= offset;                    // "Remaining data after loop"
            offset -= 4096;                   // stage 1 INCLUDES prefetches
            dest = (byte *) dest + offset;    // "Point to the end"
            src = (byte *) src + offset;      // "Point to the end"
            offset = -offset;                 // "Negative index from the end"

            do  // stage 1 ~~ WITH prefetching
            {
                _mm_prefetch((byte *) src + offset + 4096, _MM_HINT_NTA);
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm3);
            } while (offset += 64);

            offset = -4096;
            dest = (byte *) dest + 4096;
            src = (byte *) src + 4096;

            _mm_prefetch(((byte *) src + num - 64), _MM_HINT_NTA);  // prefetch the final tail section

            do  // stage 2 ~~ WITHOUT further prefetching
            {
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm0);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm1);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm2);
                _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm3);
            } while (offset += 64);

            if (num >= 16) {
                auto xmm0 = _mm_loadu_si128((__m128i *) src);
                if (num > 16) {
                    if (num > 32) {
                        auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + 16));
                        auto xmm6 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 32));
                        auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                        _mm_stream_si128((__m128i *) dest, xmm0);
                        _mm_stream_si128((__m128i *) ((byte *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num - 32), xmm6);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                        return;
                    }
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                    _mm_stream_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                    return;
                }
                _mm_stream_si128((__m128i *) dest, xmm0);
                return;
            }
        }

        if (num >= 8) {
            s64 rax = *(s64 *) src;
            if (num > 8) {
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) dest = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            } else
                *(s64 *) dest = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((byte *) src + num - 4);
                *(s32 *) dest = eax;
                *(s32 *) ((byte *) dest + num - 4) = ecx;
            } else
                *(s32 *) dest = eax;
        } else if (num >= 1) {
            byte al = *(byte *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((byte *) src + num - 2);
                *(byte *) dest = al;
                *(s16 *) ((byte *) dest + num - 2) = cx;
            } else
                *(byte *) dest = al;
        }
        return;
    }  // src < dest ... do reverse copy
    src = (byte *) src + num;
    dest = (byte *) dest + num;

    if (num < (1024 * 256)) {
        s64 offset = (s64)(num & -0x20);  // "Round down to nearest multiple of 64"
        dest = (byte *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (byte *) src - offset;      // "Point to the end" ... actually, we point to the start!
        num -= offset;                    // "Remaining data after loop"
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 32;
        do {
            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
            _mm_storeu_si128((__m128i *) ((byte *) dest + offset + 16), xmm2);
            _mm_storeu_si128((__m128i *) ((byte *) dest + offset), xmm3);
        } while ((offset -= 32) >= 0);

        if (num >= 16) {
            if (num > 16) {
                num = -num;
                // The order has been mixed so the compiler will not re-order the statements!
                auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
                return;
            }
            _mm_storeu_si128((__m128i *) ((byte *) dest - 16), _mm_loadu_si128((__m128i *) ((byte *) src - 16)));
            return;
        }
    } else  // do reversed streaming copy/move
    {
        // We MUST do prealignment on streaming copies!
        auto prealign = (size_t) dest & 0xf;
        if (prealign) {
            src = (byte *) src - prealign;
            dest = (byte *) dest - prealign;
            num -= prealign;
            if (prealign >= 8) {
                s64 rax = *(s64 *) ((byte *) src + prealign - 8);
                if (prealign > 8) {
                    s64 rcx = *(s64 *) src;
                    *(s64 *) ((byte *) dest + prealign - 8) = rax;
                    *(s64 *) dest = rcx;
                } else
                    *(s64 *) dest = rax;  // different on purpose, because we know the exact num now,
                // which is 8, and "dest" has already been aligned!
            } else if (prealign >= 4) {
                s32 eax = *(s32 *) ((byte *) src + prealign - 4);
                if (prealign > 4) {
                    s32 ecx = *(s32 *) src;
                    *(s32 *) ((byte *) dest + prealign - 4) = eax;
                    *(s32 *) dest = ecx;
                } else
                    *(s32 *) dest = eax;  // different on purpose!
            } else {
                byte al = *(byte *) ((byte *) src + prealign - 1);
                if (prealign > 1) {
                    s16 cx = *(s16 *) src;
                    *(byte *) ((byte *) dest + prealign - 1) = al;
                    *(s16 *) dest = cx;
                } else
                    *(byte *) dest = al;  // different on purpose!
            }
        }

        // Begin prefetching upto 4KB
        for (s64 offset = 0; offset > -4096; offset -= 256) {
            _mm_prefetch(((byte *) src + offset - 64), _MM_HINT_NTA);
            _mm_prefetch(((byte *) src + offset - 128), _MM_HINT_NTA);
            _mm_prefetch(((byte *) src + offset - 192), _MM_HINT_NTA);
            _mm_prefetch(((byte *) src + offset - 256), _MM_HINT_NTA);
        }

        s64 offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
        num -= offset;                    // "Remaining data after loop"
        offset -= 4096;                   // stage 1 INCLUDES prefetches
        dest = (byte *) dest - offset;    // "Point to the end" ... actually, we point to the start!
        src = (byte *) src - offset;      // "Point to the end" ... actually, we point to the start!
        // offset = -offset;// "Negative index from the end" ... not when
        // doing reverse copy/move!

        offset -= 64;
        do  // stage 1 ~~ WITH prefetching
        {
            _mm_prefetch((byte *) src + offset - 4096, _MM_HINT_NTA);
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        offset = 4096;
        dest = (byte *) dest - 4096;
        src = (byte *) src - 4096;

        _mm_prefetch(((byte *) src - 64), _MM_HINT_NTA);  // prefetch the final tail section

        offset -= 64;
        do  // stage 2 ~~ WITHOUT further prefetching
        {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
            auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
            auto xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
            auto xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 48), xmm0);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 32), xmm1);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset + 16), xmm2);
            _mm_stream_si128((__m128i *) ((byte *) dest + offset), xmm3);
        } while ((offset -= 64) >= 0);

        if (num >= 16) {
            auto xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));
            if (num > 16) {
                if (num > 32) {
                    num = -num;
                    auto xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src - 32));
                    auto xmm6 = _mm_loadu_si128((__m128i *) ((byte *) src + num + 16));
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                    _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
                    _mm_stream_si128((__m128i *) ((byte *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num + 16), xmm6);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                    return;
                }
                num = -num;
                auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                return;
            }
            _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
            return;
        }
    }

    if (num >= 8) {
        s64 rax = *(s64 *) ((byte *) src - 8);
        if (num > 8) {
            num = -num;  // that's right, we're converting an unsigned value to a negative, saves 2 clock cycles!
            auto rcx = *(s64 *) ((byte *) src + num);
            *(s64 *) ((byte *) dest - 8) = rax;
            *(s64 *) ((byte *) dest + num) = rcx;
        } else
            *(s64 *) ((byte *) dest - 8) = rax;
    } else if (num >= 4) {
        s32 eax = *(s32 *) ((byte *) src - 4);
        if (num > 4) {
            num = -num;
            s32 ecx = *(s32 *) ((byte *) src + num);
            *(s32 *) ((byte *) dest - 4) = eax;
            *(s32 *) ((byte *) dest + num) = ecx;
        } else
            *(s32 *) ((byte *) dest - 4) = eax;
    } else if (num >= 1) {
        byte al = *((byte *) src - 1);
        if (num > 1) {
            num = -num;
            s16 cx = *(s16 *) ((byte *) src + num);
            *((byte *) dest - 1) = al;
            *(s16 *) ((byte *) dest + num) = cx;
        } else
            *((byte *) dest - 1) = al;
    }
    return;
}

#if BITS == 32
void *mithril(void *dst, const void *src, size_t num) {
    if (num <= 32) {
        if (num >= 16) {
            s64r1 = *(s64 *) src;
            s64r2 = *(s64 *) ((byte *) src + 8);
            if (num > 16) {
                s64r3 = *(s64 *) ((byte *) src + num - 16);
                s64r4 = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) dst = r1;
                *(s64 *) ((byte *) dst + 8) = r2;
                *(s64 *) ((byte *) dst + num - 16) = r3;
                *(s64 *) ((byte *) dst + num - 8) = r4;
                return;
            }
            *(s64 *) dst = r1;
            *(s64 *) ((byte *) dst + 8) = r2;
            return;
        }
        if (num >= 8) {
            s64rax = *(s64 *) src;
            if (num > 8) {
                s64rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) dst = rax;
                *(s64 *) ((byte *) dst + num - 8) = rcx;
            } else
                *(s64 *) dst = rax;
        } else if (num >= 4) {
            s32 eax = *(s32 *) src;
            if (num > 4) {
                s32 ecx = *(s32 *) ((byte *) src + num - 4);
                *(s32 *) dst = eax;
                *(s32 *) ((byte *) dst + num - 4) = ecx;
            } else
                *(s32 *) dst = eax;
        } else if (num >= 1) {
            byte al = *(byte *) src;
            if (num > 1) {
                s16 cx = *(s16 *) ((byte *) src + num - 2);
                *(byte *) dst = al;
                *(s16 *) ((byte *) dst + num - 2) = cx;
            } else
                *(byte *) dst = al;
        }
        return;
    } else {
        if (((size_t) dst - (size_t) src) >= num) {
            if ((size_t) dst & 0xf) {
                const size_t prealign = -(size_t) dst & 0xf;
                if (num <= 48) {
                    // prealignment might drop the remaining num below 32-bytes,
                    // we could also check "num - prealign <= 32", but since we're already here,
                    // and these statements can load upto 48-bytes, we might as well just do it!
                    // We `could` copy up to 64-bytes without any additional checks by using another SSE "loadu" &
                    // "storeu"
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
                    s64r1 = *(s64 *) ((byte *) src + 16);
                    s64r2 = *(s64 *) ((byte *) src + 24);
                    s64r3 = *(s64 *) ((byte *) src + num - 16);
                    s64r4 = *(s64 *) ((byte *) src + num - 8);
                    _mm_storeu_si128((__m128i *) dst, xmm0);
                    *(s64 *) ((byte *) dst + 16) = r1;
                    *(s64 *) ((byte *) dst + 24) = r2;
                    *(s64 *) ((byte *) dst + num - 16) = r3;
                    *(s64 *) ((byte *) dst + num - 8) = r4;
                    return;
                }

                if (prealign >= 8) {
                    s64rax = *(s64 *) src;
                    if (prealign > 8) {
                        s64rcx = *(s64 *) ((byte *) src + prealign - 8);
                        *(s64 *) dst = rax;
                        *(s64 *) ((byte *) dst + prealign - 8) = rcx;
                    } else
                        *(s64 *) dst = rax;
                } else if (prealign >= 4) {
                    s32 eax = *(s32 *) src;
                    if (prealign > 4) {
                        s32 ecx = *(s32 *) ((byte *) src + prealign - 4);
                        *(s32 *) dst = eax;
                        *(s32 *) ((byte *) dst + prealign - 4) = ecx;
                    } else
                        *(s32 *) dst = eax;
                } else {
                    byte al = *(byte *) src;
                    if (prealign > 1) {
                        s16 cx = *(s16 *) ((byte *) src + prealign - 2);
                        *(byte *) dst = al;
                        *(s16 *) ((byte *) dst + prealign - 2) = cx;
                    } else
                        *(byte *) dst = al;
                }
                src = (byte *) src + prealign;
                dst = (byte *) dst + prealign;
                num -= prealign;
            }

            if (num < (1024 * 256)) {
                s64offset = (s64)(num & -0x20);  // "Round down to nearest multiple of 32"
                dst = (byte *) dst + offset;     // "Point to the end"
                src = (byte *) src + offset;     // "Point to the end"
                num -= offset;                   // "Remaining data after loop"
                offset = -offset;                // "Negative index from the end"

                if (((size_t) src & 0xf) == 0) {
                    do {
                        const __m128i xmm0 = _mm_load_si128((__m128i *) ((byte *) src + offset));
                        const __m128i xmm1 = _mm_load_si128((__m128i *) ((byte *) src + offset + 16));
                        _mm_store_si128((__m128i *) ((byte *) dst + offset), xmm0);
                        _mm_store_si128((__m128i *) ((byte *) dst + offset + 16), xmm1);
                    } while (offset += 32);

                    if (num >= 16) {
                        const __m128i xmm0 = _mm_load_si128((__m128i *) src);
                        if (num > 16) {
                            s64rax = *(s64 *) ((byte *) src + num - 16);
                            s64rcx = *(s64 *) ((byte *) src + num - 8);
                            *(s64 *) ((byte *) dst + num - 16) = rax;
                            *(s64 *) ((byte *) dst + num - 8) = rcx;
                        }
                        _mm_store_si128((__m128i *) dst, xmm0);
                        return;
                    }
                } else {
                    do {
                        const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                        const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                        _mm_store_si128((__m128i *) ((byte *) dst + offset), xmm0);
                        _mm_store_si128((__m128i *) ((byte *) dst + offset + 16), xmm1);
                    } while (offset += 32);

                    if (num >= 16) {
                        const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
                        if (num > 16) {
                            s64rax = *(s64 *) ((byte *) src + num - 16);
                            s64rcx = *(s64 *) ((byte *) src + num - 8);
                            *(s64 *) ((byte *) dst + num - 16) = rax;
                            *(s64 *) ((byte *) dst + num - 8) = rcx;
                        }
                        _mm_store_si128((__m128i *) dst, xmm0);
                        return;
                    }
                }
            } else  // do forward streaming copy/move
            {
                // Begin prefetching upto 4KB
                for (s64i = 0; i < 4096; i += 256) {
                    _mm_prefetch(((byte *) src + i), _MM_HINT_NTA);
                    _mm_prefetch(((byte *) src + i + 64), _MM_HINT_NTA);
                    _mm_prefetch(((byte *) src + i + 128), _MM_HINT_NTA);
                    _mm_prefetch(((byte *) src + i + 192), _MM_HINT_NTA);
                }

                s64offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 64"
                num -= offset;                   // "Remaining data after loop"
                offset -= 4096;                  // stage 1 INCLUDES prefetches
                dst = (byte *) dst + offset;     // "Point to the end"
                src = (byte *) src + offset;     // "Point to the end"
                offset = -offset;                // "Negative index from the end"

                do  // stage 1 ~~ WITH prefetching
                {
                    _mm_prefetch((byte *) src + offset + 4096, _MM_HINT_NTA);
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                    const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                    const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset), xmm0);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 16), xmm1);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 32), xmm2);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 48), xmm3);
                } while (offset += 64);

                offset = -4096;
                dst = (byte *) dst + 4096;
                src = (byte *) src + 4096;

                _mm_prefetch(((byte *) src + num - 64), _MM_HINT_NTA);  // prefetch the final tail section

                do  // stage 2 ~~ WITHOUT further prefetching
                {
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                    const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                    const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset), xmm0);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 16), xmm1);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 32), xmm2);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 48), xmm3);
                } while (offset += 64);

                if (num >= 16) {
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) src);
                    if (num > 16) {
                        if (num > 32) {
                            const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + 16));
                            const __m128i xmm6 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 32));
                            const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                            _mm_stream_si128((__m128i *) dst, xmm0);
                            _mm_stream_si128((__m128i *) ((byte *) dst + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dst + num - 32), xmm6);
                            _mm_storeu_si128((__m128i *) ((byte *) dst + num - 16), xmm7);
                            return;
                        }
                        const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                        _mm_stream_si128((__m128i *) dst, xmm0);
                        _mm_storeu_si128((__m128i *) ((byte *) dst + num - 16), xmm7);
                        return;
                    }
                    _mm_stream_si128((__m128i *) dst, xmm0);
                    return;
                }
            }

            if (num >= 8) {
                s64rax = *(s64 *) src;
                if (num > 8) {
                    s64rcx = *(s64 *) ((byte *) src + num - 8);
                    *(s64 *) dst = rax;
                    *(s64 *) ((byte *) dst + num - 8) = rcx;
                } else
                    *(s64 *) dst = rax;
            } else if (num >= 4) {
                s32 eax = *(s32 *) src;
                if (num > 4) {
                    s32 ecx = *(s32 *) ((byte *) src + num - 4);
                    *(s32 *) dst = eax;
                    *(s32 *) ((byte *) dst + num - 4) = ecx;
                } else
                    *(s32 *) dst = eax;
            } else if (num >= 1) {
                byte al = *(byte *) src;
                if (num > 1) {
                    s16 cx = *(s16 *) ((byte *) src + num - 2);
                    *(byte *) dst = al;
                    *(s16 *) ((byte *) dst + num - 2) = cx;
                } else
                    *(byte *) dst = al;
            }
            return;
        } else  // src < dst ... do reverse copy
        {
            src = (byte *) src + num;
            dst = (byte *) dst + num;

            const size_t prealign = (size_t) dst & 0xf;
            if (prealign) {
                if (num <= 48) {
                    // prealignment might drop the remaining num below 32-bytes,
                    // we could also check "num - prealign <= 32", but since we're already here,
                    // and these statements can load upto 48-bytes, we might as well just do it!
                    // Actually, we can copy upto 64-bytes without any additional checks!
                    num = -num;
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));  // first 32-bytes in reverse
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src - 32));
                    const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                    _mm_storeu_si128((__m128i *) ((byte *) dst - 16), xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dst - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((byte *) dst + num),
                                     xmm7);  // the "back" bytes are actually the base address!
                    return;
                }

                src = (byte *) src - prealign;
                dst = (byte *) dst - prealign;
                num -= prealign;
                if (prealign >= 8) {
                    s64rax = *(s64 *) ((byte *) src + prealign - 8);
                    if (prealign > 8) {
                        s64rcx = *(s64 *) src;
                        *(s64 *) ((byte *) dst + prealign - 8) = rax;
                        *(s64 *) dst = rcx;
                    } else
                        *(s64 *) dst = rax;  // different on purpose, because we know the exact num now!
                } else if (prealign >= 4) {
                    s32 eax = *(s32 *) ((byte *) src + prealign - 4);
                    if (prealign > 4) {
                        s32 ecx = *(s32 *) src;
                        *(s32 *) ((byte *) dst + prealign - 4) = eax;
                        *(s32 *) dst = ecx;
                    } else
                        *(s32 *) dst = eax;  // different on purpose!
                } else {
                    byte al = *(byte *) ((byte *) src + prealign - 1);
                    if (prealign > 1) {
                        s16 cx = *(s16 *) src;
                        *(byte *) ((byte *) dst + prealign - 1) = al;
                        *(s16 *) dst = cx;
                    } else
                        *(byte *) dst = al;  // different on purpose!
                }
            }

            if (num < (1024 * 256)) {
                s64offset = (s64)(num & -0x20);  // "Round down to nearest multiple of 32"
                dst = (byte *) dst - offset;     // "Point to the end" ... actually, we point to the start!
                src = (byte *) src - offset;     // "Point to the end" ... actually, we point to the start!
                num -= offset;                   // "Remaining data after loop"
                // offset = -offset;									// "Negative index from the end" ... not when
                // doing reverse copy/move!

                offset -= 32;  // MSVC completely re-engineers this!

                if (((size_t) src & 0xf) == 0) {
                    do {
                        const __m128i xmm0 = _mm_load_si128((__m128i *) ((byte *) src + offset + 16));
                        const __m128i xmm1 = _mm_load_si128((__m128i *) ((byte *) src + offset));
                        _mm_store_si128((__m128i *) ((byte *) dst + offset + 16), xmm0);
                        _mm_store_si128((__m128i *) ((byte *) dst + offset), xmm1);
                    } while ((offset -= 32) >= 0);

                    if (num >= 16) {
                        const __m128i xmm0 = _mm_load_si128((__m128i *) ((byte *) src - 16));
                        if (num > 16) {
                            num = -num;
                            // const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((byte*)src + num) ); // SSE2
                            // alternative
                            s64rax = *(s64 *) ((byte *) src + num + 8);
                            s64rcx = *(s64 *) ((byte *) src + num);
                            //_mm_storeu_si128( (__m128i*)((byte*)dst + num), xmm7 );
                            *(s64 *) ((byte *) dst + num + 8) = rax;
                            *(s64 *) ((byte *) dst + num) = rcx;
                        }
                        _mm_store_si128((__m128i *) ((byte *) dst - 16), xmm0);
                        return;
                    }
                } else {
                    do {
                        const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                        const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                        _mm_store_si128((__m128i *) ((byte *) dst + offset + 16), xmm0);
                        _mm_store_si128((__m128i *) ((byte *) dst + offset), xmm1);
                    } while ((offset -= 32) >= 0);

                    if (num >= 16) {
                        const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));
                        if (num > 16) {
                            num = -num;
                            s64rax = *(s64 *) ((byte *) src + num + 8);
                            s64rcx = *(s64 *) ((byte *) src + num);
                            *(s64 *) ((byte *) dst + num + 8) = rax;
                            *(s64 *) ((byte *) dst + num) = rcx;
                        }
                        _mm_store_si128((__m128i *) ((byte *) dst - 16), xmm0);
                        return;
                    }
                }
            } else  // do reversed streaming copy/move
            {
                // Begin prefetching upto 4KB
                for (s64offset = 0; offset > -4096; offset -= 256) {
                    _mm_prefetch(((byte *) src + offset - 64), _MM_HINT_NTA);
                    _mm_prefetch(((byte *) src + offset - 128), _MM_HINT_NTA);
                    _mm_prefetch(((byte *) src + offset - 192), _MM_HINT_NTA);
                    _mm_prefetch(((byte *) src + offset - 256), _MM_HINT_NTA);
                }

                s64offset = (s64)(num & -0x40);  // "Round down to nearest multiple of 32"
                num -= offset;                   // "Remaining data after loop"
                offset -= 4096;                  // stage 1 INCLUDES prefetches
                dst = (byte *) dst - offset;     // "Point to the end" ... actually, we point to the start!
                src = (byte *) src - offset;     // "Point to the end" ... actually, we point to the start!
                // offset = -offset;									// "Negative index from the end" ... not when
                // doing reverse copy/move!

                offset -= 64;
                do  // stage 1 ~~ WITH prefetching
                {
                    _mm_prefetch((byte *) src + offset - 4096, _MM_HINT_NTA);
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                    const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                    const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 48), xmm0);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 32), xmm1);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 16), xmm2);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset), xmm3);
                } while ((offset -= 64) >= 0);

                offset = 4096;
                dst = (byte *) dst - 4096;
                src = (byte *) src - 4096;

                _mm_prefetch(((byte *) src - 64), _MM_HINT_NTA);  // prefetch the final tail section

                offset -= 64;
                do  // stage 2 ~~ WITHOUT further prefetching
                {
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 48));
                    const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 32));
                    const __m128i xmm2 = _mm_loadu_si128((__m128i *) ((byte *) src + offset + 16));
                    const __m128i xmm3 = _mm_loadu_si128((__m128i *) ((byte *) src + offset));
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 48), xmm0);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 32), xmm1);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset + 16), xmm2);
                    _mm_stream_si128((__m128i *) ((byte *) dst + offset), xmm3);
                } while ((offset -= 64) >= 0);

                if (num >= 16) {
                    const __m128i xmm0 = _mm_loadu_si128((__m128i *) ((byte *) src - 16));
                    if (num > 16) {
                        if (num > 32) {
                            num = -num;
                            const __m128i xmm1 = _mm_loadu_si128((__m128i *) ((byte *) src - 32));
                            const __m128i xmm6 = _mm_loadu_si128((__m128i *) ((byte *) src + num + 16));
                            const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                            _mm_stream_si128((__m128i *) ((byte *) dst - 16), xmm0);
                            _mm_stream_si128((__m128i *) ((byte *) dst - 32), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dst + num + 16), xmm6);
                            _mm_storeu_si128((__m128i *) ((byte *) dst + num), xmm7);
                            return;
                        }
                        num = -num;
                        const __m128i xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                        _mm_stream_si128((__m128i *) ((byte *) dst - 16), xmm0);
                        _mm_storeu_si128((__m128i *) ((byte *) dst + num), xmm7);
                        return;
                    }
                    _mm_stream_si128((__m128i *) ((byte *) dst - 16), xmm0);
                    return;
                }
            }

            if (num >= 8) {
                s64rax = *(s64 *) ((byte *) src - 8);
                if (num > 8) {
                    num =
                        -num;  // that's right, we're converting an unsigned value to a negative, saves 2 clock cycles!
                    s64rcx = *(s64 *) ((byte *) src + num);
                    *(s64 *) ((byte *) dst - 8) = rax;
                    *(s64 *) ((byte *) dst + num) = rcx;
                } else
                    *(s64 *) ((byte *) dst - 8) = rax;
            } else if (num >= 4) {
                s32 eax = *(s32 *) ((byte *) src - 4);
                if (num > 4) {
                    num = -num;
                    s32 ecx = *(s32 *) ((byte *) src + num);
                    *(s32 *) ((byte *) dst - 4) = eax;
                    *(s32 *) ((byte *) dst + num) = ecx;
                } else
                    *(s32 *) ((byte *) dst - 4) = eax;
            } else if (num >= 1) {
                byte al = *((byte *) src - 1);
                if (num > 1) {
                    num = -num;
                    s16 cx = *(s16 *) ((byte *) src + num);
                    *((byte *) dst - 1) = al;
                    *(s16 *) ((byte *) dst + num) = cx;
                } else
                    *((byte *) dst - 1) = al;
            }
        }
    }
}
#endif  // end test for 32 bit
#endif  // end test for x86 arch

#if COMPILER == MSVC
#pragma warning(pop)
#endif

// This sets up copy_memory and move_memory the first time it's called
static void dispatcher(void *dest, const void *src, size_t num) {
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
#if BITS == 64
    // Detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access
    if (cpuid[2] & bit_SSE4_2) {
        copy_memory = &apex::kryptonite;
    } else {
        copy_memory = &apex::tiberium;
    }
#else
    // 32 bit
    // Detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access
    if (cpuid[2] & bit_SSE4_2) {
        copy_memory = &kryptonite;
    } else if (cpuid[2] & bit_SSSE3) {
        //	`tiberium` CAN run with the SSE2 instructions alone, however there was something
        // about very old SSE2 (only) computers, some penalty when using `tiberiums` algorithm, so
        // I've restricted `tiberium` to Core/Core 2 based machines on 32-bit by doing this!
        copy_memory = &tiberium;
    } else if (cpuid[3] & bit_SSE2) {
        // This is for very, very old computers with SSE2 only! eg. old Pentium 4! There was
        // something about computers WITHOUT SSSE3 (Core microarchitecture) that made `tiberium` a
        // bit slower. So this is for super old (P4) PC's!
        copy_memory = &mithril;
    } else {
        copy_memory = &copy_memory_constexpr
    }
#endif
#else
    copy_memory = &copy_memory_constexpr
#endif
    // Once we set it, actually run it
    copy_memory(dest, src, num);
}
}  // namespace apex

void (*copy_memory)(void *dest, const void *src, size_t num) = apex::dispatcher;

//
// SSE optimized fill_memory
// If the platform doesn't support SSE, it still writes 4 bytes at a time (instead of 16)
//

void optimized_fill_memory(void *dest, byte c, size_t num) {
    byte *d = (byte *) dest;

#if ARCH == X86
    size_t offset = ((ptr_t) dest) % 16;
    size_t num16bytes = (num - offset) / 16;
    size_t remaining = num - num16bytes * 16 - offset;

    fill_memory_constexpr(d, c, offset);
    d += offset;

    __m128i c16 = _mm_set_epi8(c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c);
    For(range(num16bytes)) {
        _mm_store_si128((__m128i *) d, c16);
        d += 16;
    }
    fill_memory_constexpr(d, c, remaining);
#else
    size_t offset = ((ptr_t) dest) % 4;
    size_t num4bytes = (num - offset) / 4;
    size_t remaining = num - num16bytes * 4 - offset;

    fill_memory_constexpr(d, c, offset);
    d += offset;

    union {
        u32 Word;
        byte Bytes[4] = {c, c, c, c};
    } c4;

    For(range(num4bytes)) {
        *((u32 *) d) = c4.Word;
        d += 4;
    }
    fill_memory_constexpr(d, c, remaining);
#endif
}

void (*fill_memory)(void *dest, byte value, size_t num) = optimized_fill_memory;

//
// Compare memory (optimized code partly taken from https://code.woboq.org/userspace/glibc/string/memcmp.c.html)
//

static size_t compare_bytes_of_two_u32s(u32 a, u32 b) {
    auto *s1 = (const byte *) &a;
    auto *s2 = (const byte *) &b;

    for (size_t index = 0; index < 4; ++index) {
        if (*s1++ != *s2++) return index;
    }
    return npos;
}

static size_t compare_memory_common_alignment(const byte *s1, const byte *s2, size_t num) {
    size_t progress = 0;
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
    return npos;
}

#if ENDIAN == LITTLE_ENDIAN
#define MERGE(w0, sh_1, w1, sh_2) (((w0) >> (sh_1)) | ((w1) << (sh_2)))
#else
#define MERGE(w0, sh_1, w1, sh_2) (((w0) << (sh_1)) | ((w1) >> (sh_2)))
#endif

static size_t compare_memory_not_common_alignment(const byte *s1, const byte *s2, size_t num) {
    size_t progress = 0;
    u32 a0, a1, a2, a3;
    u32 b0, b1, b2, b3;
    u32 x;
    s32 shl, shr;

    // Calculate how to shift a word read at the memory operation aligned srcp1 to make it aligned for comparison.
    shl = 8 * ((uptr_t) s1 % 4);
    shr = 8 * 4 - shl;

    // Make SRCP1 aligned by rounding it down to the beginning of the u32 it points in the middle of.
    s1 = (const byte *) ((uptr_t) s1 & -4);
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
    return npos;
}

size_t optimized_compare_memory(const void *ptr1, const void *ptr2, size_t num) {
    size_t progress = 0;

    auto s1 = (const byte *) ptr1;
    auto s2 = (const byte *) ptr2;

    if (num >= 16) {
        for (size_t index = 0; (uptr_t) s2 % 4 != 0; ++index) {
            if (*s1++ != *s2++) return index;
            ++progress;
            --num;
        }

        size_t res;
        if ((uptr_t) s1 % 4 == 0) {
            res = compare_memory_common_alignment(s1, s2, num / 4);
        } else {
            res = compare_memory_not_common_alignment(s1, s2, num / 4);
        }
        if (res != npos) return progress + res;

        // Number of bytes remaining in the interval [0..3]
        s1 += num & -4;
        s2 += num & -4;
        num %= 4;
    }

    // There are just a few bytes to compare. Use byte memory operations.
    for (size_t index = 0; num--; ++index) {
        if (*s1++ != *s2++) return progress + index;
    }
    return npos;
}

size_t (*compare_memory)(const void *ptr1, const void *ptr2, size_t num) = optimized_compare_memory;

LSTD_END_NAMESPACE