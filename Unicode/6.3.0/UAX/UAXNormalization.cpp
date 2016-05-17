#include <stdlib.h>
#include <stdint.h>
#include "UAX.h"

using namespace UAX;
using namespace Normalization;

struct NormalizationAlgorithm {
};

// bool Normalization::

// from ucdn.c
/*
 * Copyright (C) 2012 Grigori Goronzy <greg@kinoho.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* constants required for Hangul (de)composition */
#define SBASE 0xAC00
#define LBASE 0x1100
#define VBASE 0x1161
#define TBASE 0x11A7
#define SCOUNT 11172
#define LCOUNT 19
#define VCOUNT 21
#define TCOUNT 28
#define NCOUNT (VCOUNT * TCOUNT)

int hangul_pair_decompose(uint32_t code, uint32_t *a, uint32_t *b)
{
    int si = code - SBASE;

    if (si < 0 || si >= SCOUNT)
        return 0;

    if (si % TCOUNT) {
        /* LV,T */
        *a = SBASE + (si / TCOUNT) * TCOUNT;
        *b = TBASE + (si % TCOUNT);
        return 3;
    } else {
        /* L,V */
        *a = LBASE + (si / NCOUNT);
        *b = VBASE + (si % NCOUNT) / TCOUNT;
        return 2;
    }
}

int hangul_pair_compose(uint32_t *code, uint32_t a, uint32_t b)
{
    if (b < VBASE || b >= (TBASE + TCOUNT))
        return 0;

    if ((a < LBASE || a >= (LBASE + LCOUNT))
            && (a < SBASE || a >= (SBASE + SCOUNT)))
        return 0;

    if (a >= SBASE) {
        /* LV,T */
        *code = a + (b - TBASE);
        return 3;
    } else {
        /* L,V */
        int li = a - LBASE;
        int vi = b - VBASE;
        *code = SBASE + li * NCOUNT + vi * TCOUNT;
        return 2;
    }
}
