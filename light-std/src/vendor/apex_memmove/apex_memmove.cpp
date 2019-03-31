#include "apex_memmove.hpp"

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)  // Test for Intel/AMD architecture
#include <emmintrin.h>                                                             //Intel/AMD SSE intrinsics
#if COMPILER == MSVC
#include <intrin.h>  // __cpuid (Visual Studio)
#else
#include <cpuid.h>  // __get_cpuid (GCC / Clang)
#endif
#endif

LSTD_BEGIN_NAMESPACE

// apex memmove (tiberium, kryptonite and mithril) memcpy/memmove functions written by Trevor Herselman in 2014
namespace apex {

#if COMPILER == MSVC
#pragma warning(push)
#pragma warning(disable : 4146)  // warning C4146: unary minus operator applied to unsigned type, result still unsigned
#pragma warning( \
    disable : 4244)  // warning C4244: '-=': conversion from '__int64' to 'std::size_t', possible loss of data
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)  // Test for Intel/AMD architecture

// based on memmove09 for "num <= 112" and memmove40 for "num > 112"
void *tiberium(void *dest, const void *src, size_t num) {
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
                                        return dest;
                                    }
                                    *(s64 *) ((byte *) dest + num - 16) = rax;
                                    *(s64 *) ((byte *) dest + num - 8) = rcx;
                                    _mm_storeu_si128((__m128i *) dest, xmm0);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 64), xmm4);
                                    return dest;
                                }
                                *(s64 *) ((byte *) dest + num - 16) = rax;
                                *(s64 *) ((byte *) dest + num - 8) = rcx;
                                _mm_storeu_si128((__m128i *) dest, xmm0);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                return dest;
                            }
                            *(s64 *) ((byte *) dest + num - 16) = rax;
                            *(s64 *) ((byte *) dest + num - 8) = rcx;
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                            return dest;
                        }
                        *(s64 *) ((byte *) dest + num - 16) = rax;
                        *(s64 *) ((byte *) dest + num - 8) = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                    return dest;
                }
                s64 rax = *(s64 *) ((byte *) src + num - 16);
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) ((byte *) dest + num - 16) = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            }
            _mm_storeu_si128((__m128i *) dest, xmm0);
            return dest;
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
        return dest;
    }

    void *ret = dest;
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
                            return ret;
                        }
                        _mm_storeu_si128((__m128i *) dest, xmm0);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                        _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm3);
                        return ret;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm3);
                    return ret;
                }
                _mm_storeu_si128((__m128i *) dest, xmm0);
                return ret;
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
                        return ret;
                    }
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                    _mm_stream_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                    return ret;
                }
                _mm_stream_si128((__m128i *) dest, xmm0);
                return ret;
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
        return ret;
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
                        return ret;
                    }
                    _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest - 32), xmm1);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm3);
                    return ret;
                }
                _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm3);
                return ret;
            }
            _mm_storeu_si128((__m128i *) ((byte *) dest - 16), xmm0);
            return ret;
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
                    return ret;
                }
                num = -num;
                auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                return ret;
            }
            _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
            return ret;
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
    return ret;
}

// based on memmove09 for "num <= 112" and memmove41 for "num > 112"; memmove09's
// "num <= 112" proved fastest overall (weighted), even on Core i5!
void *kryptonite(void *dest, const void *src, size_t num) {
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
                                        return dest;
                                    }
                                    *(s64 *) ((byte *) dest + num - 16) = rax;
                                    *(s64 *) ((byte *) dest + num - 8) = rcx;
                                    _mm_storeu_si128((__m128i *) dest, xmm0);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                    _mm_storeu_si128((__m128i *) ((byte *) dest + 64), xmm4);
                                    return dest;
                                }
                                *(s64 *) ((byte *) dest + num - 16) = rax;
                                *(s64 *) ((byte *) dest + num - 8) = rcx;
                                _mm_storeu_si128((__m128i *) dest, xmm0);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                                _mm_storeu_si128((__m128i *) ((byte *) dest + 48), xmm3);
                                return dest;
                            }
                            *(s64 *) ((byte *) dest + num - 16) = rax;
                            *(s64 *) ((byte *) dest + num - 8) = rcx;
                            _mm_storeu_si128((__m128i *) dest, xmm0);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                            _mm_storeu_si128((__m128i *) ((byte *) dest + 32), xmm2);
                            return dest;
                        }
                        *(s64 *) ((byte *) dest + num - 16) = rax;
                        *(s64 *) ((byte *) dest + num - 8) = rcx;
                    }
                    _mm_storeu_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + 16), xmm1);
                    return dest;
                }
                s64 rax = *(s64 *) ((byte *) src + num - 16);
                s64 rcx = *(s64 *) ((byte *) src + num - 8);
                *(s64 *) ((byte *) dest + num - 16) = rax;
                *(s64 *) ((byte *) dest + num - 8) = rcx;
            }
            _mm_storeu_si128((__m128i *) dest, xmm0);
            return dest;
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
        return dest;
    }
    void *ret = dest;
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
                    return ret;
                }
                _mm_storeu_si128((__m128i *) dest, _mm_loadu_si128((__m128i *) src));
                return ret;
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
                        return ret;
                    }
                    auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num - 16));
                    _mm_stream_si128((__m128i *) dest, xmm0);
                    _mm_storeu_si128((__m128i *) ((byte *) dest + num - 16), xmm7);
                    return ret;
                }
                _mm_stream_si128((__m128i *) dest, xmm0);
                return ret;
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
        return ret;
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
                return ret;
            }
            _mm_storeu_si128((__m128i *) ((byte *) dest - 16), _mm_loadu_si128((__m128i *) ((byte *) src - 16)));
            return ret;
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
                    return ret;
                }
                num = -num;
                auto xmm7 = _mm_loadu_si128((__m128i *) ((byte *) src + num));
                _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
                _mm_storeu_si128((__m128i *) ((byte *) dest + num), xmm7);
                return ret;
            }
            _mm_stream_si128((__m128i *) ((byte *) dest - 16), xmm0);
            return ret;
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
    return ret;
}
#endif  // end test for Intel/AMD

#if COMPILER == MSVC
#pragma warning(pop)
#endif

// This sets up memcpy and memmove the first time it's called
static void *dispatcher(void *dest, const void *src, size_t num) {
#if (defined _M_X64 || defined __x86_64__ || defined __i386 || defined _M_IX86)
#if COMPILER == MSVC
    s32 cpuid[4] = {-1};  // Visual Studio
    __cpuid(cpuid, 1);
#define bit_SSE2 (1 << 26)  // Taken from GCC <cpuid.h> ... just more visual & descriptive!
#define bit_SSSE3 (1 << 9)
#define bit_SSE4_2 (1 << 20)
#else
    u32 s32 cpuid[4];  // GCC / LLVM (Clang)
    __get_cpuid(1, &cpuid[0], &cpuid[1], &cpuid[2], &cpuid[3]);
#endif
    // detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access
    if (cpuid[2] & bit_SSE4_2) {
        copy_memory = move_memory = &kryptonite;
    } else {
        copy_memory = move_memory = &tiberium;
    }
#endif
    return move_memory(dest, src, num);  // safe to call memmove even for memcpy!
}
}  // namespace apex

void *(*copy_memory)(void *dest, const void *src, size_t num) = apex::dispatcher;
void *(*move_memory)(void *dest, const void *src, size_t num) = apex::dispatcher;

LSTD_END_NAMESPACE