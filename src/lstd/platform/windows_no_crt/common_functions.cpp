#include "common_functions.h"

#include "setjmp.h"

import lstd.string;

LSTD_USING_NAMESPACE;

//
// Some implementations of these are taken from:
// https://github.com/beloff-ZA/LIBFT/blob/master/libft.h
//
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   libft.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: beloff <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/06/13 02:38:01 by beloff            #+#    #+#             */
/*   Updated: 2018/06/13 05:31:09 by beloff           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

extern "C" {

// If we are building with MSVC in Release, the compiler optimizes the following functions as instrinsics.
#if COMPILER == MSVC and (defined DEBUG_OPTIMIZED or defined RELEASE)
#pragma function(memset)
#pragma function(memcpy)
#pragma function(memmove)

#pragma function(strlen)
#pragma function(strcmp)
#pragma function(memcmp)
#pragma function(strcpy)
#pragma function(memchr)
#pragma function(strcat)
#endif

size_t strlen(const char *s) {
    int i;

    i = 0;
    while (s[i] != '\0')
        i++;
    return (i);
}

int strcmp(const char *s1, const char *s2) {
    int i;

    i = 0;
    while (s1[i] != '\0' && s2[i] != '\0' && s1[i] == s2[i])
        i++;
    return ((unsigned char) s1[i] - (unsigned char) s2[i]);
}

char *strcpy(char *dst, const char *src) {
    int i;

    i = 0;
    while (src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return (dst);
}

const void *memchr(const void *s, int c, size_t n) {
    size_t i;

    i = 0;
    while (n--) {
        if (((unsigned char *) s)[i] == (unsigned char) c)
            return ((void *) &((unsigned char *) s)[i]);
        i++;
    }
    return (NULL);
}

char *strcat(char *s1, const char *s2) {
    int i;
    int j;

    i = 0;
    j = 0;
    while (s1[i] != '\0') {
        i++;
    }
    while (s2[j] != '\0') {
        s1[i + j] = s2[j];
        j++;
    }
    s1[i + j] = '\0';
    return (s1);
}

double fmod(double x, double y) {
    union {
        double f;
        u64 i;
    } ux = {x}, uy = {y};
    int ex = ux.i >> 52 & 0x7ff;
    int ey = uy.i >> 52 & 0x7ff;
    int sx = ux.i >> 63;
    u64 i;

    /* in the followings uxi should be ux.i, but then gcc wrongly adds */
    /* float load/store to inner loops ruining performance and code size */
    u64 uxi = ux.i;

    if (uy.i << 1 == 0 || is_nan(y) || ex == 0x7ff)
        return (x * y) / (x * y);
    if (uxi << 1 <= uy.i << 1) {
        if (uxi << 1 == uy.i << 1)
            return 0 * x;
        return x;
    }

    /* normalize x and y */
    if (!ex) {
        for (i = uxi << 12; i >> 63 == 0; ex--, i <<= 1)
            ;
        uxi <<= -ex + 1;
    } else {
        uxi &= (u64) (-1) >> 12;
        uxi |= 1ULL << 52;
    }
    if (!ey) {
        for (i = uy.i << 12; i >> 63 == 0; ey--, i <<= 1)
            ;
        uy.i <<= -ey + 1;
    } else {
        uy.i &= (u64) (-1) >> 12;
        uy.i |= 1ULL << 52;
    }

    /* x mod y */
    for (; ex > ey; ex--) {
        i = uxi - uy.i;
        if (i >> 63 == 0) {
            if (i == 0)
                return 0 * x;
            uxi = i;
        }
        uxi <<= 1;
    }
    i = uxi - uy.i;
    if (i >> 63 == 0) {
        if (i == 0)
            return 0 * x;
        uxi = i;
    }
    for (; uxi >> 52 == 0; uxi <<= 1, ex--)
        ;

    /* scale result */
    if (ex > 0) {
        uxi -= 1ULL << 52;
        uxi |= (u64) ex << 52;
    } else {
        uxi >>= -ex + 1;
    }
    uxi |= (u64) sx << 63;
    ux.i = uxi;
    return ux.f;
}

const char *strstr(const char *haystack, const char *needle) {
    int i;
    int j;

    if (needle[0] == '\0')
        return ((char *) haystack);
    i = 0;
    while (haystack[i] != '\0') {
        j = 0;
        while (needle[j] != '\0') {
            if (haystack[i + j] != needle[j])
                break;
            j++;
        }
        if (needle[j] == '\0')
            return ((char *) haystack + i);
        i++;
    }
    return (0);
}

const char *strchr(const char *s, int c) {
    while (*s != (char) c && *s != '\0')
        s++;
    if (*s == (char) c)
        return ((char *) s);
    return (NULL);
}

int strncmp(const char *s1, const char *s2, size_t n) {
    size_t i;

    if (!n)
        return (0);
    i = 1;
    while (*s1 == *s2) {
        if (!*s1++ || i++ == n)
            return (0);
        s2++;
    }
    return ((unsigned char) (*s1) - (unsigned char) (*s2));
}

char *strncpy(char *dst, const char *src, size_t len) {
    size_t i;

    i = 0;
    while (src[i] && i < len) {
        dst[i] = src[i];
        i++;
    }
    while (i < len)
        dst[i++] = '\0';
    return (dst);
}

const char *strrchr(const char *s, int c) {
    size_t len;

    len = strlen((char *) s);
    while (0 != len && s[len] != (char) c)
        len--;
    if (s[len] == (char) c)
        return ((char *) &s[len]);
    return (NULL);
}

static unsigned char charmap(char c) {
    char chr;

    chr = (char) to_upper(c);
    if (chr >= '0' && chr <= '9')
        return (chr - '0');
    if (chr >= 'A' && chr <= 'Z')
        return (chr - 'A' + 10);
    return (127);
}

static int getbase(const char **nptr, int base) {
    const char *ptr;

    ptr = *nptr;
    if ((base == 0 || base == 16) && *ptr == '0') {
        if (*(++ptr) == 'x' && (++(ptr)))
            base = 16;
        else if (*ptr == '0')
            base = 8;
        else
            base = 10;
        *nptr = ptr;
    }
    return (base);
}

long strtol(const char *nptr, char **endptr, int base) {
    int neg;
    long result;
    char digit;

    if (base < 0 || base > 36)
        return (0);
    neg    = 0;
    result = 0;
    while (is_space(*nptr))
        nptr++;
    if (*nptr == '-' || *nptr == '+')
        if (*nptr++ == '-')
            neg = 1;
    base = getbase(&nptr, base);
    while ((digit = charmap(*nptr++)) < base)
        if ((result = (result * base) + digit) < 0) {
            if (endptr)
                *endptr = (char *) nptr;
            return (numeric<s32>::max() + neg);
        }
    if (endptr)
        *endptr = (char *) nptr;
    return (result + (result * -2 * neg));
}

/*
 * strtod and atof implementation from minlibc. https://github.com/GaloisInc/minlibc
 * Here is a copy of the license: 
 * 
 * Copyright (c) 2014 Galois Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 * 
 *   * Redistributions in binary form must reproduce the above copyright 
 *     notice, this list of conditions and the following disclaimer in 
 *     the documentation and/or other materials provided with the 
 *     distribution.
 * 
 *   * Neither the name of Galois, Inc. nor the names of its contributors 
 *     may be used to endorse or promote products derived from this 
 *     software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
double strtod(const char *s, char **endptr) {
    // This function stolen from either Rolf Neugebauer or Andrew Tolmach.
    // Probably Rolf.
    const char *begin = s;

    double a = 0.0;
    int e    = 0;
    int c;
    while ((c = *s++) != '\0' && is_digit(c)) {
        a = a * 10.0 + (c - '0');
    }
    if (c == '.') {
        while ((c = *s++) != '\0' && is_digit(c)) {
            a = a * 10.0 + (c - '0');
            e = e - 1;
        }
    }
    if (c == 'e' || c == 'E') {
        int sign = 1;
        int i    = 0;
        c        = *s++;
        if (c == '+')
            c = *s++;
        else if (c == '-') {
            c    = *s++;
            sign = -1;
        }
        while (is_digit(c)) {
            i = i * 10 + (c - '0');
            c = *s++;
        }
        e += i * sign;
    }
    while (e > 0) {
        a *= 10.0;
        e--;
    }
    while (e < 0) {
        a *= 0.1;
        e++;
    }

    if (endptr) {
        if (s != begin) {
            *endptr = (char *) (s - 1);
        } else {
            // If we couldn't parse anything...
            *endptr = (char *) s;
        }
    }

    return a;
}

double atof(const char *s) {
    return strtod(s, null);
}

//
// The following is an implementation of sscanf by Rolf Neugebauer.
// Here is the license:

/*
 ****************************************************************************
 * (C) 2003 - Rolf Neugebauer - Intel Research Cambridge
 ****************************************************************************
 *
 *        File: printf.c
 *      Author: Rolf Neugebauer (neugebar@dcs.gla.ac.uk)
 *     Changes: Grzegorz Milos (gm281@cam.ac.uk) 
 *
 *        Date: Aug 2003, Aug 2005
 *
 * Environment: Xen Minimal OS
 * Description: Library functions for printing
 *              (Linux port, mainly lib/vsprintf.c)
 *
 ****************************************************************************
 */

/*
 * Copyright (C) 1991, 1992  Linus Torvalds
 */

/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */

/*
 * Fri Jul 13 2001 Crutcher Dunnavant <crutcher+kernel@datastacks.com>
 * - changed to provide snprintf and vsnprintf functions
 * So Feb  1 16:51:32 CET 2004 Juergen Quade <quade@hsnr.de>
 * - scnprintf and vscnprintf
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
//

/**
 * simple_strtoul - convert a string to an unsigned long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {
    unsigned long result = 0, value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((*cp == 'x') && is_hex_digit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    }
    while (is_hex_digit(*cp) &&
           (value = is_digit(*cp) ? *cp - '0' : to_upper(*cp) - 'A' + 10) < base) {
        result = result * base + value;
        cp++;
    }
    if (endp)
        *endp = (char *) cp;
    return result;
}

/**
 * simple_strtol - convert a string to a signed long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long simple_strtol(const char *cp, char **endp, unsigned int base) {
    if (*cp == '-')
        return -((long) simple_strtoul(cp + 1, endp, base));
    return (long) simple_strtoul(cp, endp, base);
}

/**
 * simple_strtoull - convert a string to an unsigned long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base) {
    unsigned long long result = 0, value;

    if (!base) {
        base = 10;
        if (*cp == '0') {
            base = 8;
            cp++;
            if ((*cp == 'x') && is_hex_digit(cp[1])) {
                cp++;
                base = 16;
            }
        }
    }
    while (is_hex_digit(*cp) && (value = is_digit(*cp) ? *cp - '0' : (is_lower(*cp) ? to_upper(*cp) : *cp) - 'A' + 10) < base) {
        result = result * base + value;
        cp++;
    }
    if (endp)
        *endp = (char *) cp;
    return result;
}

/**
 * simple_strtoll - convert a string to a signed long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 */
long long simple_strtoll(const char *cp, char **endp, unsigned int base) {
    if (*cp == '-')
        return -((long long) simple_strtoull(cp + 1, endp, base));
    return (long long) simple_strtoull(cp, endp, base);
}

static int skip_atoi(const char **s) {
    int i = 0;

    while (is_digit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2     /* unsigned/signed long */
#define PLUS 4     /* show plus */
#define SPACE 8    /* space if plus */
#define LEFT 16    /* left justified */
#define SPECIAL 32 /* 0x */
#define LARGE 64   /* use 'ABCDEF' instead of 'abcdef' */

/**
 * vsscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	format of buffer
 * @args:	arguments
 */
int vsscanf(const char *buf, const char *fmt, va_list args) {
    const char *str = buf;
    char *next;
    char digit;
    int num = 0;
    int qualifier;
    int base;
    int field_width;
    int is_sign = 0;

    while (*fmt && *str) {
        /* skip any white space in format */
        /* white space in format matchs any amount of
		 * white space, including none, in the input.
		 */
        if (is_space(*fmt)) {
            while (is_space(*fmt))
                ++fmt;
            while (is_space(*str))
                ++str;
        }

        /* anything that is not a conversion must match exactly */
        if (*fmt != '%' && *fmt) {
            if (*fmt++ != *str++)
                break;
            continue;
        }

        if (!*fmt)
            break;
        ++fmt;

        /* skip this conversion.
		 * advance both strings to next white space
		 */
        if (*fmt == '*') {
            while (!is_space(*fmt) && *fmt)
                fmt++;
            while (!is_space(*str) && *str)
                str++;
            continue;
        }

        /* get field width */
        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);

        /* get conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
            *fmt == 'Z' || *fmt == 'z') {
            qualifier = *fmt++;
            if (qualifier == *fmt) [[unlikely]] {
                if (qualifier == 'h') {
                    qualifier = 'H';
                    fmt++;
                } else if (qualifier == 'l') {
                    qualifier = 'L';
                    fmt++;
                }
            }
        }
        base    = 10;
        is_sign = 0;

        if (!*fmt || !*str)
            break;

        switch (*fmt++) {
            case 'c': {
                char *s = (char *) va_arg(args, char *);
                if (field_width == -1)
                    field_width = 1;
                do {
                    *s++ = *str++;
                } while (--field_width > 0 && *str);
                num++;
            }
                continue;
            case 's': {
                char *s = (char *) va_arg(args, char *);
                if (field_width == -1)
                    field_width = numeric<s32>::max();
                /* first, skip leading white space in buffer */
                while (is_space(*str))
                    str++;

                /* now copy until next white space */
                while (*str && !is_space(*str) && field_width--) {
                    *s++ = *str++;
                }
                *s = '\0';
                num++;
            }
                continue;
            case 'n':
                /* return number of characters read so far */
                {
                    int *i = (int *) va_arg(args, int *);
                    *i     = (int) (str - buf);
                }
                continue;
            case 'o':
                base = 8;
                break;
            case 'x':
            case 'X':
                base = 16;
                break;
            case 'i':
                base = 0;
            case 'd':
                is_sign = 1;
            case 'u':
                break;
            case '%':
                /* looking for '%' in str */
                if (*str++ != '%')
                    return num;
                continue;
            default:
                /* invalid format; stop here */
                return num;
        }

        /* have some sort of integer conversion.
		 * first, skip white space in buffer.
		 */
        while (is_space(*str))
            str++;

        digit = *str;
        if (is_sign && digit == '-')
            digit = *(str + 1);

        if (!digit || (base == 16 && !is_hex_digit(digit)) || (base == 10 && !is_digit(digit)) || (base == 8 && (!is_digit(digit) || digit > '7')) || (base == 0 && !is_digit(digit)))
            break;

        switch (qualifier) {
            case 'H': /* that's 'hh' in format */
                if (is_sign) {
                    signed char *s = (signed char *) va_arg(args, signed char *);
                    *s             = (signed char) simple_strtol(str, &next, base);
                } else {
                    unsigned char *s = (unsigned char *) va_arg(args, unsigned char *);
                    *s               = (unsigned char) simple_strtoul(str, &next, base);
                }
                break;
            case 'h':
                if (is_sign) {
                    short *s = (short *) va_arg(args, short *);
                    *s       = (short) simple_strtol(str, &next, base);
                } else {
                    unsigned short *s = (unsigned short *) va_arg(args, unsigned short *);
                    *s                = (unsigned short) simple_strtoul(str, &next, base);
                }
                break;
            case 'l':
                if (is_sign) {
                    long *l = (long *) va_arg(args, long *);
                    *l      = simple_strtol(str, &next, base);
                } else {
                    unsigned long *l = (unsigned long *) va_arg(args, unsigned long *);
                    *l               = simple_strtoul(str, &next, base);
                }
                break;
            case 'L':
                if (is_sign) {
                    long long *l = (long long *) va_arg(args, long long *);
                    *l           = simple_strtoll(str, &next, base);
                } else {
                    unsigned long long *l = (unsigned long long *) va_arg(args, unsigned long long *);
                    *l                    = simple_strtoull(str, &next, base);
                }
                break;
            case 'Z':
            case 'z': {
                size_t *s = (size_t *) va_arg(args, size_t *);
                *s        = (size_t) simple_strtoul(str, &next, base);
            } break;
            default:
                if (is_sign) {
                    int *i = (int *) va_arg(args, int *);
                    *i     = (int) simple_strtol(str, &next, base);
                } else {
                    unsigned int *i = (unsigned int *) va_arg(args, unsigned int *);
                    *i              = (unsigned int) simple_strtoul(str, &next, base);
                }
                break;
        }
        num++;

        if (!next)
            break;
        str = next;
    }
    return num;
}

int sscanf(const char *str, const char *fmt, ...) {
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsscanf(str, fmt, args);
    va_end(args);
    return i;
}

void qsort(void *data, size_t items, size_t size, int (*compare)(const void *, const void *)) {
    return quick_sort(data, (s64) items, (s64) size, compare);
}

int toupper(int c) { return to_upper(c); }

float fmodf(float x, float y) { return (float) fmod((double) x, (double) y); }
float powf(float x, float y) { return (float) pow((double) x, (double) y); }
float logf(float x) { return (float) log((double) x); }
float fabsf(float x) { return (float) abs((double) x); }
float sqrtf(float x) { return (float) sqrt((double) x); }
float cosf(float x) { return (float) cos((double) x); }
float sinf(float x) { return (float) sin((double) x); }
float acosf(float x) { return (float) acos((double) x); }
float atan2f(float x, float y) { return (float) atan2((double) x, (double) y); }
float ceilf(float x) { return (float) ceil((double) x); }
}
