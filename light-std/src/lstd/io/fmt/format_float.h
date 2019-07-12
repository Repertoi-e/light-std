#pragma once

#include "specs.h"
#include "value.h"

#include "float_const.h"

LSTD_BEGIN_NAMESPACE

// Modified implementation, source: https://salsa.debian.org/yangfl-guest/stb/blob/master/stb_sprintf.h
// Trimmed down version of stb_sprintf in order to support formatting floats only.

namespace fmt::internal {

using stb_callback = byte *(*) (void *user, byte *buf, size_t length);

#define MIN_BYTES 512

inline void get_float_info(s64 *bits, s32 *exp, f64 value) {
    union {
        f64 v;
        s64 b;
    } u;
    u.v = value;

    *bits = u.b & ((1ull << 52) - 1);
    *exp = (s32)(((u.b >> 52) & 2047) - 1023);
}

// _power_ can be -323 to +350
inline void raise_to_power_10(f64 *ohi, f64 *olo, f64 d, s32 power) {
    f64 ph, pl;
    if ((power >= 0) && (power <= 22)) {
        ddmulthi(ph, pl, d, BOT[power]);
    } else {
        s32 e, et, eb;
        f64 p2h, p2l;

        e = power;
        if (power < 0) e = -e;
        et = (e * 0x2c9) >> 14; /* %23 */
        if (et > 13) et = 13;
        eb = e - (et * 23);

        ph = d;
        pl = 0.0;
        if (power < 0) {
            if (eb) {
                --eb;
                ddmulthi(ph, pl, d, NEGBOT[eb]);
                ddmultlos(ph, pl, d, NEGBOTERR[eb]);
            }
            if (et) {
                ddrenorm(ph, pl);
                --et;
                ddmulthi(p2h, p2l, ph, NEGTOP[et]);
                ddmultlo(p2h, p2l, ph, pl, NEGTOP[et], NEGTOPERR[et]);
                ph = p2h;
                pl = p2l;
            }
        } else {
            if (eb) {
                e = eb;
                if (eb > 22) eb = 22;
                e -= eb;
                ddmulthi(ph, pl, d, BOT[eb]);
                if (e) {
                    ddrenorm(ph, pl);
                    ddmulthi(p2h, p2l, ph, BOT[e]);
                    ddmultlos(p2h, p2l, BOT[e], pl);
                    ph = p2h;
                    pl = p2l;
                }
            }
            if (et) {
                ddrenorm(ph, pl);
                --et;
                ddmulthi(p2h, p2l, ph, TOP[et]);
                ddmultlo(p2h, p2l, ph, pl, TOP[et], TOPERR[et]);
                ph = p2h;
                pl = p2l;
            }
        }
    }
    ddrenorm(ph, pl);
    *ohi = ph;
    *olo = pl;
}

// Given a float value, returns the significant bits in bits, and the position of the decimal point in _decimal_pos_
// NAN/INF are ignored and assumed to have been already handled
// _frac_digits_ is absolute normally, but if you want from first significant digits (got %g and %e), or in 0x80000000
inline void get_float_string_internal(byte **start, u32 *length, byte *out, s32 *decimal_pos, f64 value,
                                      u32 frac_digits) {
    s32 e, tens;

    f64 d = value;
    s64 bits = bit_cast<s64>(d);
    s32 expo = (s32)((bits >> 52) & 2047);

    if (expo == 0)  // is zero or denormal
    {
        if ((bits << 1) == 0)  // do zero
        {
            *decimal_pos = 1;
            *start = out;
            out[0] = '0';
            *length = 1;
            return;
        }
        // find the right expo for denormals
        {
            s64 v = 1ull << 51;
            while ((bits & v) == 0) {
                --expo;
                v >>= 1;
            }
        }
    }

    // find the decimal exponent as well as the decimal bits of the value
    {
        f64 ph, pl;

        // log10 estimate - very specifically tweaked to hit or undershoot by no more than 1 of log10 of all
        // expos 1..2046
        tens = expo - 1023;
        tens = (tens < 0) ? ((tens * 617) / 2048) : (((tens * 1233) / 4096) + 1);

        // move the significant bits into position and stick them into an int
        raise_to_power_10(&ph, &pl, d, 18 - tens);

        // get full as much precision from double-double as possible
        ddtoS64(bits, ph, pl);

        // check if we undershot
        if (((u64) bits) >= 1000000000000000000ull) ++tens;
    }

    // now do the rounding in integer land
    frac_digits = (frac_digits & 0x80000000) ? ((frac_digits & 0x7ffffff) + 1) : (tens + frac_digits);
    if ((frac_digits < 24)) {
        u32 dg = 1;
        if ((u64) bits >= POWTEN[9]) dg = 10;
        while ((u64) bits >= POWTEN[dg]) {
            ++dg;
            if (dg == 20) goto noround;
        }
        if (frac_digits < dg) {
            // add 0.5 at the right position and round
            e = dg - frac_digits;
            if ((u32) e >= 24) goto noround;
            u64 r = POWTEN[e];
            bits = bits + (r / 2);
            if ((u64) bits >= POWTEN[dg]) ++tens;
            bits /= r;
        }
    noround:;
    }

    // kill long trailing runs of zeros
    if (bits) {
        while (true) {
            if (bits <= 0xffffffff) break;
            if (bits % 1000) goto donez;
            bits /= 1000;
        }
        u32 n = (u32) bits;
        while ((n % 1000) == 0) n /= 1000;
        bits = n;
    donez:;
    }

    // convert to string
    out += 64;
    e = 0;
    while (true) {
        u32 n;
        char *o = out - 8;
        // do the conversion in chunks of U32s (avoid most 64-bit divides, worth it, constant denomiators be damned)
        if (bits >= 100000000) {
            n = (u32)(bits % 100000000);
            bits /= 100000000;
        } else {
            n = (u32) bits;
            bits = 0;
        }
        while (n) {
            out -= 2;
            *(u16 *) out = *(u16 *) &DIGITPAIR.Pair[(n % 100) * 2];
            n /= 100;
            e += 2;
        }
        if (bits == 0) {
            if ((e) && (out[0] == '0')) {
                ++out;
                --e;
            }
            break;
        }
        while (out != o) {
            *--out = '0';
            ++e;
        }
    }

    *decimal_pos = tens;
    *start = out;
    *length = e;
}

#define check_buffer_length(bytes)                                        \
    {                                                                     \
        size_t length = (size_t)(bf - buf);                               \
        if ((length + (bytes)) >= MIN_BYTES) {                            \
            if (0 == (bf = buf = callback(user, buf, length))) goto done; \
        }                                                                 \
    }
#define check_buffer(bytes) \
    { check_buffer_length(bytes); }

// Flush if there is even one byte in the buffer
#define flush_buffer() \
    { check_buffer_length(MIN_BYTES - 1); }

#define buffer_clamp(cl, v)               \
    cl = v;                               \
    s32 lg = MIN_BYTES - (s32)(bf - buf); \
    if (cl > lg) cl = lg;

// Pass -1 for precision for default value
inline void format_float(stb_callback callback, void *user, byte *buf, byte specType, f64 fv, s32 pr,
                         bool commas = false) {
    assert(callback);

    byte *bf = buf;

    s32 tz = 0;
    u32 fl = 0;

    byte num[512];
    byte lead[8]{};
    byte tail[8]{};

    switch (specType) {
        byte *s, *sn;
        const byte *h;
        u32 l, n, cs;

        u64 n64;
        s32 dp;

        case 'A':
        case 'a':
            h = (specType == 'A') ? "0123456789ABCDEFXP" : "0123456789abcdefxp";
            if (pr == -1) pr = 6;  // Default is 6
            get_float_info((s64 *) &n64, &dp, fv);

            s = num + 64;

            if (dp == -1023) {
                dp = (n64) ? -1022 : 0;
            } else {
                n64 |= (1ull << 52);
            }
            n64 <<= (64 - 56);
            if (pr < 15) n64 += ((8ull << 56) >> (pr * 4));

            // Add leading chars
            lead[1] = '0';
            lead[2] = 'x';
            lead[0] = 2;

            *s++ = h[(n64 >> 60) & 15];
            n64 <<= 4;
            if (pr) *s++ = '.';  // @Locale
            sn = s;

            // Print the bits
            n = pr;
            if (n > 13) n = 13;
            if (pr > (s32) n) tz = pr - n;
            pr = 0;
            while (n--) {
                *s++ = h[(n64 >> 60) & 15];
                n64 <<= 4;
            }

            // Print the expo
            tail[1] = h[17];
            if (dp < 0) {
                tail[2] = '-';
                dp = -dp;
            } else
                tail[2] = '+';
            n = (dp >= 1000) ? 6 : ((dp >= 100) ? 5 : ((dp >= 10) ? 4 : 3));
            tail[0] = (byte) n;
            while (true) {
                tail[n] = '0' + dp % 10;
                if (n <= 3) break;
                --n;
                dp /= 10;
            }

            dp = (s32)(s - sn);
            l = (s32)(s - (num + 64));
            s = num + 64;
            cs = 1 + (3 << 24);
            goto scopy;

        case 'G':
        case 'g':
            h = (specType == 'G') ? "0123456789ABCDEFXP" : "0123456789abcdefxp";
            if (pr == -1)
                pr = 6;  // Default is 6
            else if (pr == 0)
                pr = 1;  // Minimum is 1

            // Read the f64 into a string
            get_float_string_internal(&sn, &l, num, &dp, fv, (pr - 1) | 0x80000000);

            // Clamp the precision and delete extra zeros after clamp
            n = pr;
            if (l > (u32) pr) l = pr;
            while ((l > 1) && (pr) && (sn[l - 1] == '0')) {
                --pr;
                --l;
            }

            // Should we use %e
            if ((dp <= -4) || (dp > (s32) n)) {
                if (pr > (s32) l)
                    pr = l - 1;
                else if (pr)
                    --pr;  // when using %e, there is one digit before the decimal
                goto doexpfromg;
            }

            // this is the insane action to get the pr to match %g semantics for %f
            if (dp > 0) {
                pr = (dp < (s32) l) ? l - dp : 0;
            } else {
                pr = -dp + ((pr > (s32) l) ? (s32) l : pr);
            }
            goto dofloatfromg;

        case 'E':
        case 'e':
            h = (specType == 'E') ? "0123456789ABCDEFXP" : "0123456789abcdefxp";
            if (pr == -1) pr = 6;  // Default is 6

            // Read the f64 into a string
            get_float_string_internal(&sn, &l, num, &dp, fv, pr | 0x80000000);
        doexpfromg:
            tail[0] = 0;
            s = num + 64;
            *s++ = sn[0];

            if (pr) *s++ = '.';  // @Locale

            // Handle after decimal
            if ((l - 1) > (u32) pr) l = pr + 1;
            for (n = 1; n < l; n++) *s++ = sn[n];

            // Trailing zeros
            tz = pr - (l - 1);
            pr = 0;

            // Dump expo
            tail[1] = h[0xe];
            dp -= 1;
            if (dp < 0) {
                tail[2] = '-';
                dp = -dp;
            } else {
                tail[2] = '+';
            }

            n = (dp >= 100) ? 5 : 4;

            tail[0] = (byte) n;
            while (true) {
                tail[n] = '0' + dp % 10;
                if (n <= 3) break;
                --n;
                dp /= 10;
            }
            cs = 1 + (3 << 24);  // How many tens
            goto flt_lead;

        case 'f':
            // Default is 6
            if (pr == -1) pr = 6;

            // Read the f64 into a string
            get_float_string_internal(&sn, &l, num, &dp, fv, pr);
        dofloatfromg:
            tail[0] = 0;
            s = num + 64;

            // Handle the three decimal varieties
            if (dp <= 0) {
                s32 i;
                // Handle 0.000*000xxxx
                *s++ = '0';
                if (pr) *s++ = '.';  // @Locale
                n = -dp;
                if ((s32) n > pr) n = pr;
                i = n;
                while (i) {
                    if ((((uptr_t) s) & 3) == 0) break;
                    *s++ = '0';
                    --i;
                }
                while (i >= 4) {
                    *(u32 *) s = 0x30303030;
                    s += 4;
                    i -= 4;
                }
                while (i) {
                    *s++ = '0';
                    --i;
                }
                if ((s32)(l + n) > pr) l = pr - n;
                i = l;
                while (i) {
                    *s++ = *sn++;
                    --i;
                }
                tz = pr - (n + l);
                cs = 1 + (3 << 24);  // How many tens did we write (for commas below)
            } else {
                cs = commas ? ((600 - (u32) dp) % 3) : 0;
                if ((u32) dp >= l) {
                    // Handle xxxx000*000.0
                    n = 0;
                    while (true) {
                        if (commas && (++cs == 4)) {
                            cs = 0;
                            *s++ = ',';  // @Locale
                        } else {
                            *s++ = sn[n];
                            ++n;
                            if (n >= l) break;
                        }
                    }
                    if (n < (u32) dp) {
                        n = dp - n;
                        if (!commas) {
                            while (n) {
                                if ((((uptr_t) s) & 3) == 0) break;
                                *s++ = '0';
                                --n;
                            }
                            while (n >= 4) {
                                *(u32 *) s = 0x30303030;
                                s += 4;
                                n -= 4;
                            }
                        }
                        while (n) {
                            if (commas && (++cs == 4)) {
                                cs = 0;
                                *s++ = ',';  // @Locale
                            } else {
                                *s++ = '0';
                                --n;
                            }
                        }
                    }
                    cs = (s32)(s - (num + 64)) + (3 << 24);  // _cs_ is how many tens
                    if (pr) {
                        *s++ = '.';  // @Locale
                        tz = pr;
                    }
                } else {
                    // Handle xxxxx.xxxx000*000
                    n = 0;
                    while (true) {
                        if (commas && (++cs == 4)) {
                            cs = 0;
                            *s++ = ',';  // @Locale
                        } else {
                            *s++ = sn[n];
                            ++n;
                            if (n >= (u32) dp) break;
                        }
                    }
                    cs = (s32)(s - (num + 64)) + (3 << 24);  // _cs_ is how many tens
                    if (pr) *s++ = '.';                      // @Locale
                    if ((l - dp) > (u32) pr) l = pr + dp;
                    while (n < l) {
                        *s++ = sn[n];
                        ++n;
                    }
                    tz = pr - (l - dp);
                }
            }
            pr = 0;

        flt_lead:
            l = (u32)(s - (num + 64));
            s = num + 64;
            goto scopy;

        scopy:
            // _pr_ = leading zeros
            if (pr < (s32) l) pr = l;
            n = pr + lead[0] + tail[0] + tz;
            pr -= l;

            // Copy the spaces and/or zeros
            if (pr) {
                s32 i;
                u32 c;

                // Copy leader
                sn = lead + 1;
                while (lead[0]) {
                    buffer_clamp(i, lead[0]);
                    lead[0] -= (byte) i;
                    while (i) {
                        *bf++ = *sn++;
                        --i;
                    }
                    check_buffer(1);
                }

                // copy leading zeros
                c = cs >> 24;
                cs &= 0xffffff;
                cs = commas ? ((u32)(c - ((pr + cs) % (c + 1)))) : 0;
                while (pr > 0) {
                    buffer_clamp(i, pr);
                    pr -= i;
                    if (!commas) {
                        while (i) {
                            if ((((uptr_t) bf) & 3) == 0) break;
                            *bf++ = '0';
                            --i;
                        }
                        while (i >= 4) {
                            *(u32 *) bf = 0x30303030;
                            bf += 4;
                            i -= 4;
                        }
                    }
                    while (i) {
                        if (commas && (cs++ == c)) {
                            cs = 0;
                            *bf++ = ',';  // @Locale
                        } else
                            *bf++ = '0';
                        --i;
                    }
                    check_buffer(1);
                }
            }

            // Copy leader if there is still one
            sn = lead + 1;
            while (lead[0]) {
                s32 i;
                buffer_clamp(i, lead[0]);
                lead[0] -= (byte) i;
                while (i) {
                    *bf++ = *sn++;
                    --i;
                }
                check_buffer(1);
            }

            // Copy the string
            n = l;
            while (n) {
                s32 i;
                buffer_clamp(i, n);
                n -= i;
                while (i >= 4) {
                    *(u32 *) bf = *(u32 *) s;
                    bf += 4;
                    s += 4;
                    i -= 4;
                }
                while (i) {
                    *bf++ = *s++;
                    --i;
                }
                check_buffer(1);
            }

            // Copy trailing zeros
            while (tz) {
                s32 i;
                buffer_clamp(i, tz);
                tz -= i;
                while (i) {
                    if ((((uptr_t) bf) & 3) == 0) break;
                    *bf++ = '0';
                    --i;
                }
                while (i >= 4) {
                    *(u32 *) bf = 0x30303030;
                    bf += 4;
                    i -= 4;
                }
                while (i) {
                    *bf++ = '0';
                    --i;
                }
                check_buffer(1);
            }

            // Copy tail if there is one
            sn = tail + 1;
            while (tail[0]) {
                s32 i;
                buffer_clamp(i, tail[0]);
                tail[0] -= (char) i;
                while (i) {
                    *bf++ = *sn++;
                    --i;
                }
                check_buffer(1);
            }

            break;

        default:
            assert(false && "Formatting float with unknown spec type.");
    }
    flush_buffer();
done:
    return;
}

}  // namespace fmt::internal

LSTD_END_NAMESPACE

#undef check_buffer_length
#undef check_buffer
#undef flush_buffer
#undef buffer_clamp
#undef MIN_BYTES

// Cleanup from float_const.h
#undef ddmulthi
#undef ddtoS64
#undef ddrenorm
#undef ddmultlo
#undef ddmultlos