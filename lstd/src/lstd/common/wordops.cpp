#include "../common.h"

// https://code.woboq.org/userspace/glibc/string/wordcopy.c.html#30

/* wordcopy.c - suborutines for memory copy functions.
 * Copyright (C) 1991-2019 Free Software Foundation, Inc.
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
#define OPSIZ sizeof(u64)

#if ENDIAN == LITTLE_ENDIAN
#define MERGE(w0, sh_1, w1, sh_2) (((w0) >> (sh_1)) | ((w1) << (sh_2)))
#define CMP_LT_OR_GT(a, b) memcmp_bytes((a), (b))

file_scope int memcmp_bytes(u64 a, u64 b) {
    u64 srcp1 = (u64) &a;
    u64 srcp2 = (u64) &b;
    u64 a0, b0;

    do {
        a0 = ((byte *) srcp1)[0];
        b0 = ((byte *) srcp2)[0];
        srcp1 += 1;
        srcp2 += 1;
    } while (a0 == b0);
    return (s32) (a0 - b0);
}
#endif
#if ENDIAN == BIG_ENDIAN
#define MERGE(w0, sh_1, w1, sh_2) (((w0) << (sh_1)) | ((w1) >> (sh_2)))
#define CMP_LT_OR_GT(a, b) ((a) > (b) ? 1 : -1)
#endif

void wordcopy_fwd_aligned(u64 dstp, u64 srcp, u64 len) {
    u64 a0, a1;
    switch (len % 8) {
        case 2:
            a0 = ((u64 *) srcp)[0];
            srcp -= 6 * OPSIZ;
            dstp -= 7 * OPSIZ;
            len += 6;
            goto do1;
        case 3:
            a1 = ((u64 *) srcp)[0];
            srcp -= 5 * OPSIZ;
            dstp -= 6 * OPSIZ;
            len += 5;
            goto do2;
        case 4:
            a0 = ((u64 *) srcp)[0];
            srcp -= 4 * OPSIZ;
            dstp -= 5 * OPSIZ;
            len += 4;
            goto do3;
        case 5:
            a1 = ((u64 *) srcp)[0];
            srcp -= 3 * OPSIZ;
            dstp -= 4 * OPSIZ;
            len += 3;
            goto do4;
        case 6:
            a0 = ((u64 *) srcp)[0];
            srcp -= 2 * OPSIZ;
            dstp -= 3 * OPSIZ;
            len += 2;
            goto do5;
        case 7:
            a1 = ((u64 *) srcp)[0];
            srcp -= 1 * OPSIZ;
            dstp -= 2 * OPSIZ;
            len += 1;
            goto do6;
        case 0:
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                return;
            a0 = ((u64 *) srcp)[0];
            srcp -= 0 * OPSIZ;
            dstp -= 1 * OPSIZ;
            goto do7;
        case 1:
            a1 = ((u64 *) srcp)[0];
            srcp -= -1 * OPSIZ;
            dstp -= 0 * OPSIZ;
            len -= 1;
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                goto do0;
            goto do8; /* No-op.  */
    }
    do {
    do8:
        a0                = ((u64 *) srcp)[0];
        ((u64 *) dstp)[0] = a1;
    do7:
        a1                = ((u64 *) srcp)[1];
        ((u64 *) dstp)[1] = a0;
    do6:
        a0                = ((u64 *) srcp)[2];
        ((u64 *) dstp)[2] = a1;
    do5:
        a1                = ((u64 *) srcp)[3];
        ((u64 *) dstp)[3] = a0;
    do4:
        a0                = ((u64 *) srcp)[4];
        ((u64 *) dstp)[4] = a1;
    do3:
        a1                = ((u64 *) srcp)[5];
        ((u64 *) dstp)[5] = a0;
    do2:
        a0                = ((u64 *) srcp)[6];
        ((u64 *) dstp)[6] = a1;
    do1:
        a1                = ((u64 *) srcp)[7];
        ((u64 *) dstp)[7] = a0;
        srcp += 8 * OPSIZ;
        dstp += 8 * OPSIZ;
        len -= 8;
    } while (len != 0);
    /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
    ((u64 *) dstp)[0] = a1;
}

#pragma warning(disable : 4146)

void wordcopy_fwd_dest_aligned(u64 dstp, u64 srcp, u64 len) {
    u64 a0, a1, a2, a3;
    int sh_1, sh_2;
    /* Calculate how to shift a word read at the memory operation
     aligned srcp to make it aligned for copy.  */
    sh_1 = 8 * (srcp % OPSIZ);
    sh_2 = 8 * OPSIZ - sh_1;
    /* Make SRCP aligned by rounding it down to the beginning of the `u64'
     it points in the middle of.  */
    srcp &= -OPSIZ;
    switch (len % 4) {
        case 2:
            a1 = ((u64 *) srcp)[0];
            a2 = ((u64 *) srcp)[1];
            srcp -= 1 * OPSIZ;
            dstp -= 3 * OPSIZ;
            len += 2;
            goto do1;
        case 3:
            a0 = ((u64 *) srcp)[0];
            a1 = ((u64 *) srcp)[1];
            srcp -= 0 * OPSIZ;
            dstp -= 2 * OPSIZ;
            len += 1;
            goto do2;
        case 0:
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                return;
            a3 = ((u64 *) srcp)[0];
            a0 = ((u64 *) srcp)[1];
            srcp -= -1 * OPSIZ;
            dstp -= 1 * OPSIZ;
            len += 0;
            goto do3;
        case 1:
            a2 = ((u64 *) srcp)[0];
            a3 = ((u64 *) srcp)[1];
            srcp -= -2 * OPSIZ;
            dstp -= 0 * OPSIZ;
            len -= 1;
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                goto do0;
            goto do4; /* No-op.  */
    }
    do {
    do4:
        a0                = ((u64 *) srcp)[0];
        ((u64 *) dstp)[0] = MERGE(a2, sh_1, a3, sh_2);
    do3:
        a1                = ((u64 *) srcp)[1];
        ((u64 *) dstp)[1] = MERGE(a3, sh_1, a0, sh_2);
    do2:
        a2                = ((u64 *) srcp)[2];
        ((u64 *) dstp)[2] = MERGE(a0, sh_1, a1, sh_2);
    do1:
        a3                = ((u64 *) srcp)[3];
        ((u64 *) dstp)[3] = MERGE(a1, sh_1, a2, sh_2);
        srcp += 4 * OPSIZ;
        dstp += 4 * OPSIZ;
        len -= 4;
    } while (len != 0);
    /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
    ((u64 *) dstp)[0] = MERGE(a2, sh_1, a3, sh_2);
}

/* _wordcopy_bwd_aligned -- Copy block finishing right before
   SRCP to block finishing right before DSTP with LEN `u64' words
   (not LEN bytes!).  Both SRCP and DSTP should be aligned for memory
   operations on `u64's.  */
void wordcopy_bwd_aligned(u64 dstp, u64 srcp, u64 len) {
    u64 a0, a1;
    switch (len % 8) {
        case 2:
            srcp -= 2 * OPSIZ;
            dstp -= 1 * OPSIZ;
            a0 = ((u64 *) srcp)[1];
            len += 6;
            goto do1;
        case 3:
            srcp -= 3 * OPSIZ;
            dstp -= 2 * OPSIZ;
            a1 = ((u64 *) srcp)[2];
            len += 5;
            goto do2;
        case 4:
            srcp -= 4 * OPSIZ;
            dstp -= 3 * OPSIZ;
            a0 = ((u64 *) srcp)[3];
            len += 4;
            goto do3;
        case 5:
            srcp -= 5 * OPSIZ;
            dstp -= 4 * OPSIZ;
            a1 = ((u64 *) srcp)[4];
            len += 3;
            goto do4;
        case 6:
            srcp -= 6 * OPSIZ;
            dstp -= 5 * OPSIZ;
            a0 = ((u64 *) srcp)[5];
            len += 2;
            goto do5;
        case 7:
            srcp -= 7 * OPSIZ;
            dstp -= 6 * OPSIZ;
            a1 = ((u64 *) srcp)[6];
            len += 1;
            goto do6;
        case 0:
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                return;
            srcp -= 8 * OPSIZ;
            dstp -= 7 * OPSIZ;
            a0 = ((u64 *) srcp)[7];
            goto do7;
        case 1:
            srcp -= 9 * OPSIZ;
            dstp -= 8 * OPSIZ;
            a1 = ((u64 *) srcp)[8];
            len -= 1;
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                goto do0;
            goto do8; /* No-op.  */
    }
    do {
    do8:
        a0                = ((u64 *) srcp)[7];
        ((u64 *) dstp)[7] = a1;
    do7:
        a1                = ((u64 *) srcp)[6];
        ((u64 *) dstp)[6] = a0;
    do6:
        a0                = ((u64 *) srcp)[5];
        ((u64 *) dstp)[5] = a1;
    do5:
        a1                = ((u64 *) srcp)[4];
        ((u64 *) dstp)[4] = a0;
    do4:
        a0                = ((u64 *) srcp)[3];
        ((u64 *) dstp)[3] = a1;
    do3:
        a1                = ((u64 *) srcp)[2];
        ((u64 *) dstp)[2] = a0;
    do2:
        a0                = ((u64 *) srcp)[1];
        ((u64 *) dstp)[1] = a1;
    do1:
        a1                = ((u64 *) srcp)[0];
        ((u64 *) dstp)[0] = a0;
        srcp -= 8 * OPSIZ;
        dstp -= 8 * OPSIZ;
        len -= 8;
    } while (len != 0);
    /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
    ((u64 *) dstp)[7] = a1;
}
/* _wordcopy_bwd_dest_aligned -- Copy block finishing right
   before SRCP to block finishing right before DSTP with LEN `u64'
   words (not LEN bytes!).  DSTP should be aligned for memory
   operations on `u64', but SRCP must *not* be aligned.  */
void wordcopy_bwd_dest_aligned(u64 dstp, u64 srcp, u64 len) {
    u64 a0, a1, a2, a3;
    int sh_1, sh_2;
    /* Calculate how to shift a word read at the memory operation
     aligned srcp to make it aligned for copy.  */
    sh_1 = 8 * (srcp % OPSIZ);
    sh_2 = 8 * OPSIZ - sh_1;
    /* Make srcp aligned by rounding it down to the beginning of the u64
     it points in the middle of.  */
    srcp &= -OPSIZ;
    srcp += OPSIZ;
    switch (len % 4) {
        case 2:
            srcp -= 3 * OPSIZ;
            dstp -= 1 * OPSIZ;
            a2 = ((u64 *) srcp)[2];
            a1 = ((u64 *) srcp)[1];
            len += 2;
            goto do1;
        case 3:
            srcp -= 4 * OPSIZ;
            dstp -= 2 * OPSIZ;
            a3 = ((u64 *) srcp)[3];
            a2 = ((u64 *) srcp)[2];
            len += 1;
            goto do2;
        case 0:
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                return;
            srcp -= 5 * OPSIZ;
            dstp -= 3 * OPSIZ;
            a0 = ((u64 *) srcp)[4];
            a3 = ((u64 *) srcp)[3];
            goto do3;
        case 1:
            srcp -= 6 * OPSIZ;
            dstp -= 4 * OPSIZ;
            a1 = ((u64 *) srcp)[5];
            a0 = ((u64 *) srcp)[4];
            len -= 1;
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                goto do0;
            goto do4; /* No-op.  */
    }
    do {
    do4:
        a3                = ((u64 *) srcp)[3];
        ((u64 *) dstp)[3] = MERGE(a0, sh_1, a1, sh_2);
    do3:
        a2                = ((u64 *) srcp)[2];
        ((u64 *) dstp)[2] = MERGE(a3, sh_1, a0, sh_2);
    do2:
        a1                = ((u64 *) srcp)[1];
        ((u64 *) dstp)[1] = MERGE(a2, sh_1, a3, sh_2);
    do1:
        a0                = ((u64 *) srcp)[0];
        ((u64 *) dstp)[0] = MERGE(a1, sh_1, a2, sh_2);
        srcp -= 4 * OPSIZ;
        dstp -= 4 * OPSIZ;
        len -= 4;
    } while (len != 0);
    /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
    ((u64 *) dstp)[3] = MERGE(a0, sh_1, a1, sh_2);
}

//
// Routines below taken from the same library.
// https://github.com/lattera/glibc/blob/master/string/memcmp.c
//

/* memcmp_common_alignment -- Compare blocks at SRCP1 and SRCP2 with LEN `u64'
   objects (not LEN bytes!).  Both SRCP1 and SRCP2 should be aligned for
   memory operations on `u64's.  */
int memcmp_common_alignment(u64 srcp1, u64 srcp2, size_t len) {
    u64 a0, a1;
    u64 b0, b1;

    switch (len % 4) {
        default: /* Avoid warning about uninitialized local variables.  */
        case 2:
            a0 = ((u64 *) srcp1)[0];
            b0 = ((u64 *) srcp2)[0];
            srcp1 -= 2 * OPSIZ;
            srcp2 -= 2 * OPSIZ;
            len += 2;
            goto do1;
        case 3:
            a1 = ((u64 *) srcp1)[0];
            b1 = ((u64 *) srcp2)[0];
            srcp1 -= OPSIZ;
            srcp2 -= OPSIZ;
            len += 1;
            goto do2;
        case 0:
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                return 0;
            a0 = ((u64 *) srcp1)[0];
            b0 = ((u64 *) srcp2)[0];
            goto do3;
        case 1:
            a1 = ((u64 *) srcp1)[0];
            b1 = ((u64 *) srcp2)[0];
            srcp1 += OPSIZ;
            srcp2 += OPSIZ;
            len -= 1;
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                goto do0;
            /* Fall through.  */
    }

    do {
        a0 = ((u64 *) srcp1)[0];
        b0 = ((u64 *) srcp2)[0];
        if (a1 != b1)
            return CMP_LT_OR_GT(a1, b1);

    do3:
        a1 = ((u64 *) srcp1)[1];
        b1 = ((u64 *) srcp2)[1];
        if (a0 != b0)
            return CMP_LT_OR_GT(a0, b0);

    do2:
        a0 = ((u64 *) srcp1)[2];
        b0 = ((u64 *) srcp2)[2];
        if (a1 != b1)
            return CMP_LT_OR_GT(a1, b1);

    do1:
        a1 = ((u64 *) srcp1)[3];
        b1 = ((u64 *) srcp2)[3];
        if (a0 != b0)
            return CMP_LT_OR_GT(a0, b0);

        srcp1 += 4 * OPSIZ;
        srcp2 += 4 * OPSIZ;
        len -= 4;
    } while (len != 0);

    /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
    if (a1 != b1)
        return CMP_LT_OR_GT(a1, b1);
    return 0;
}

/* memcmp_not_common_alignment -- Compare blocks at SRCP1 and SRCP2 with LEN
   `u64' objects (not LEN bytes!).  SRCP2 should be aligned for memory
   operations on `u64', but SRCP1 *should be unaligned*.  */
int memcmp_not_common_alignment(u64 srcp1, u64 srcp2, size_t len) {
    u64 a0, a1, a2, a3;
    u64 b0, b1, b2, b3;
    u64 x;
    int shl, shr;

    /* Calculate how to shift a word read at the memory operation
     aligned srcp1 to make it aligned for comparison.  */

    shl = 8 * (srcp1 % OPSIZ);
    shr = 8 * OPSIZ - shl;

    /* Make SRCP1 aligned by rounding it down to the beginning of the `u64'
     it points in the middle of.  */
    srcp1 &= -OPSIZ;

    switch (len % 4) {
        default: /* Avoid warning about uninitialized local variables.  */
        case 2:
            a1 = ((u64 *) srcp1)[0];
            a2 = ((u64 *) srcp1)[1];
            b2 = ((u64 *) srcp2)[0];
            srcp1 -= 1 * OPSIZ;
            srcp2 -= 2 * OPSIZ;
            len += 2;
            goto do1;
        case 3:
            a0 = ((u64 *) srcp1)[0];
            a1 = ((u64 *) srcp1)[1];
            b1 = ((u64 *) srcp2)[0];
            srcp2 -= 1 * OPSIZ;
            len += 1;
            goto do2;
        case 0:
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                return 0;
            a3 = ((u64 *) srcp1)[0];
            a0 = ((u64 *) srcp1)[1];
            b0 = ((u64 *) srcp2)[0];
            srcp1 += 1 * OPSIZ;
            goto do3;
        case 1:
            a2 = ((u64 *) srcp1)[0];
            a3 = ((u64 *) srcp1)[1];
            b3 = ((u64 *) srcp2)[0];
            srcp1 += 2 * OPSIZ;
            srcp2 += 1 * OPSIZ;
            len -= 1;
            if (OP_T_THRES <= 3 * OPSIZ && len == 0)
                goto do0;
            /* Fall through.  */
    }

    do {
        a0 = ((u64 *) srcp1)[0];
        b0 = ((u64 *) srcp2)[0];
        x  = MERGE(a2, shl, a3, shr);
        if (x != b3)
            return CMP_LT_OR_GT(x, b3);

    do3:
        a1 = ((u64 *) srcp1)[1];
        b1 = ((u64 *) srcp2)[1];
        x  = MERGE(a3, shl, a0, shr);
        if (x != b0)
            return CMP_LT_OR_GT(x, b0);

    do2:
        a2 = ((u64 *) srcp1)[2];
        b2 = ((u64 *) srcp2)[2];
        x  = MERGE(a0, shl, a1, shr);
        if (x != b1)
            return CMP_LT_OR_GT(x, b1);

    do1:
        a3 = ((u64 *) srcp1)[3];
        b3 = ((u64 *) srcp2)[3];
        x  = MERGE(a1, shl, a2, shr);
        if (x != b2)
            return CMP_LT_OR_GT(x, b2);

        srcp1 += 4 * OPSIZ;
        srcp2 += 4 * OPSIZ;
        len -= 4;
    } while (len != 0);

    /* This is the right position for do0.  Please don't move
     it into the loop.  */
do0:
    x = MERGE(a2, shl, a3, shr);
    if (x != b3)
        return CMP_LT_OR_GT(x, b3);
    return 0;
}
