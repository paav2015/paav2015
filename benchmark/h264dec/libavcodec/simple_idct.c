/*
 * Simple IDCT
 *
 * Copyright (c) 2001 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * simpleidct in C.
 */

/*
  based upon some outcommented c code from mpeg2dec (idct_mmx.c
  written by Aaron Holtzman <aholtzma@ess.engr.uvic.ca>)
 */
#include "avcodec.h"
#include "dsputil.h"
#include "mathops.h"
#include "simple_idct.h"

#if 0
#define W1 2841 /* 2048*sqrt (2)*cos (1*pi/16) */
#define W2 2676 /* 2048*sqrt (2)*cos (2*pi/16) */
#define W3 2408 /* 2048*sqrt (2)*cos (3*pi/16) */
#define W4 2048 /* 2048*sqrt (2)*cos (4*pi/16) */
#define W5 1609 /* 2048*sqrt (2)*cos (5*pi/16) */
#define W6 1108 /* 2048*sqrt (2)*cos (6*pi/16) */
#define W7 565  /* 2048*sqrt (2)*cos (7*pi/16) */
#define ROW_SHIFT 8
#define COL_SHIFT 17
#else
#define W1  22725  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W2  21407  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W3  19266  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W4  16383  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W5  12873  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W6  8867   //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define W7  4520   //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define ROW_SHIFT 11
#define COL_SHIFT 20 // 6
#endif

static inline void idctRowCondDC (DCTELEM * row)
{
        int a0, a1, a2, a3, b0, b1, b2, b3;
        uint64_t temp;

#if HAVE_BIGENDIAN
#define ROW0_MASK 0xffff000000000000LL
#else
#define ROW0_MASK 0xffffLL
#endif
        if(sizeof(DCTELEM)==2){
            if ( ((((uint64_t *)row)[0] & ~ROW0_MASK) |
                  ((uint64_t *)row)[1]) == 0) {
                temp = (row[0] << 3) & 0xffff;
                temp += temp << 16;
                temp += temp << 32;
                ((uint64_t *)row)[0] = temp;
                ((uint64_t *)row)[1] = temp;
                return;
            }
        }else{
            if (!(row[1]|row[2]|row[3]|row[4]|row[5]|row[6]|row[7])) {
                row[0]=row[1]=row[2]=row[3]=row[4]=row[5]=row[6]=row[7]= row[0] << 3;
                return;
            }
        }

        a0 = (W4 * row[0]) + (1 << (ROW_SHIFT - 1));
        a1 = a0;
        a2 = a0;
        a3 = a0;

        /* no need to optimize : gcc does it */
        a0 += W2 * row[2];
        a1 += W6 * row[2];
        a2 -= W6 * row[2];
        a3 -= W2 * row[2];

        b0 = MUL16(W1, row[1]);
        MAC16(b0, W3, row[3]);
        b1 = MUL16(W3, row[1]);
        MAC16(b1, -W7, row[3]);
        b2 = MUL16(W5, row[1]);
        MAC16(b2, -W1, row[3]);
        b3 = MUL16(W7, row[1]);
        MAC16(b3, -W5, row[3]);

        temp = ((uint64_t*)row)[1];

        if (temp != 0) {
            a0 += W4*row[4] + W6*row[6];
            a1 += - W4*row[4] - W2*row[6];
            a2 += - W4*row[4] + W2*row[6];
            a3 += W4*row[4] - W6*row[6];

            MAC16(b0, W5, row[5]);
            MAC16(b0, W7, row[7]);

            MAC16(b1, -W1, row[5]);
            MAC16(b1, -W5, row[7]);

            MAC16(b2, W7, row[5]);
            MAC16(b2, W3, row[7]);

            MAC16(b3, W3, row[5]);
            MAC16(b3, -W1, row[7]);
        }

        row[0] = (a0 + b0) >> ROW_SHIFT;
        row[7] = (a0 - b0) >> ROW_SHIFT;
        row[1] = (a1 + b1) >> ROW_SHIFT;
        row[6] = (a1 - b1) >> ROW_SHIFT;
        row[2] = (a2 + b2) >> ROW_SHIFT;
        row[5] = (a2 - b2) >> ROW_SHIFT;
        row[3] = (a3 + b3) >> ROW_SHIFT;
        row[4] = (a3 - b3) >> ROW_SHIFT;
}

static inline void idctSparseColPut (uint8_t *dest, int line_size,
                                     DCTELEM * col)
{
        int a0, a1, a2, a3, b0, b1, b2, b3;
        uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;

        /* XXX: I did that only to give same values as previous code */
        a0 = W4 * (col[8*0] + ((1<<(COL_SHIFT-1))/W4));
        a1 = a0;
        a2 = a0;
        a3 = a0;

        a0 +=  + W2*col[8*2];
        a1 +=  + W6*col[8*2];
        a2 +=  - W6*col[8*2];
        a3 +=  - W2*col[8*2];

        b0 = MUL16(W1, col[8*1]);
        b1 = MUL16(W3, col[8*1]);
        b2 = MUL16(W5, col[8*1]);
        b3 = MUL16(W7, col[8*1]);

        MAC16(b0, + W3, col[8*3]);
        MAC16(b1, - W7, col[8*3]);
        MAC16(b2, - W1, col[8*3]);
        MAC16(b3, - W5, col[8*3]);

        if(col[8*4]){
            a0 += + W4*col[8*4];
            a1 += - W4*col[8*4];
            a2 += - W4*col[8*4];
            a3 += + W4*col[8*4];
        }

        if (col[8*5]) {
            MAC16(b0, + W5, col[8*5]);
            MAC16(b1, - W1, col[8*5]);
            MAC16(b2, + W7, col[8*5]);
            MAC16(b3, + W3, col[8*5]);
        }

        if(col[8*6]){
            a0 += + W6*col[8*6];
            a1 += - W2*col[8*6];
            a2 += + W2*col[8*6];
            a3 += - W6*col[8*6];
        }

        if (col[8*7]) {
            MAC16(b0, + W7, col[8*7]);
            MAC16(b1, - W5, col[8*7]);
            MAC16(b2, + W3, col[8*7]);
            MAC16(b3, - W1, col[8*7]);
        }

        dest[0] = cm[(a0 + b0) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a1 + b1) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a2 + b2) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a3 + b3) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a3 - b3) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a2 - b2) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a1 - b1) >> COL_SHIFT];
        dest += line_size;
        dest[0] = cm[(a0 - b0) >> COL_SHIFT];
}

static inline void idctSparseColAdd (uint8_t *dest, int line_size,
                                     DCTELEM * col)
{
        int a0, a1, a2, a3, b0, b1, b2, b3;
        uint8_t *cm = ff_cropTbl + MAX_NEG_CROP;

        /* XXX: I did that only to give same values as previous code */
        a0 = W4 * (col[8*0] + ((1<<(COL_SHIFT-1))/W4));
        a1 = a0;
        a2 = a0;
        a3 = a0;

        a0 +=  + W2*col[8*2];
        a1 +=  + W6*col[8*2];
        a2 +=  - W6*col[8*2];
        a3 +=  - W2*col[8*2];

        b0 = MUL16(W1, col[8*1]);
        b1 = MUL16(W3, col[8*1]);
        b2 = MUL16(W5, col[8*1]);
        b3 = MUL16(W7, col[8*1]);

        MAC16(b0, + W3, col[8*3]);
        MAC16(b1, - W7, col[8*3]);
        MAC16(b2, - W1, col[8*3]);
        MAC16(b3, - W5, col[8*3]);

        if(col[8*4]){
            a0 += + W4*col[8*4];
            a1 += - W4*col[8*4];
            a2 += - W4*col[8*4];
            a3 += + W4*col[8*4];
        }

        if (col[8*5]) {
            MAC16(b0, + W5, col[8*5]);
            MAC16(b1, - W1, col[8*5]);
            MAC16(b2, + W7, col[8*5]);
            MAC16(b3, + W3, col[8*5]);
        }

        if(col[8*6]){
            a0 += + W6*col[8*6];
            a1 += - W2*col[8*6];
            a2 += + W2*col[8*6];
            a3 += - W6*col[8*6];
        }

        if (col[8*7]) {
            MAC16(b0, + W7, col[8*7]);
            MAC16(b1, - W5, col[8*7]);
            MAC16(b2, + W3, col[8*7]);
            MAC16(b3, - W1, col[8*7]);
        }

        dest[0] = cm[dest[0] + ((a0 + b0) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a1 + b1) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a2 + b2) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a3 + b3) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a3 - b3) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a2 - b2) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a1 - b1) >> COL_SHIFT)];
        dest += line_size;
        dest[0] = cm[dest[0] + ((a0 - b0) >> COL_SHIFT)];
}

static inline void idctSparseCol (DCTELEM * col)
{
        int a0, a1, a2, a3, b0, b1, b2, b3;

        /* XXX: I did that only to give same values as previous code */
        a0 = W4 * (col[8*0] + ((1<<(COL_SHIFT-1))/W4));
        a1 = a0;
        a2 = a0;
        a3 = a0;

        a0 +=  + W2*col[8*2];
        a1 +=  + W6*col[8*2];
        a2 +=  - W6*col[8*2];
        a3 +=  - W2*col[8*2];

        b0 = MUL16(W1, col[8*1]);
        b1 = MUL16(W3, col[8*1]);
        b2 = MUL16(W5, col[8*1]);
        b3 = MUL16(W7, col[8*1]);

        MAC16(b0, + W3, col[8*3]);
        MAC16(b1, - W7, col[8*3]);
        MAC16(b2, - W1, col[8*3]);
        MAC16(b3, - W5, col[8*3]);

        if(col[8*4]){
            a0 += + W4*col[8*4];
            a1 += - W4*col[8*4];
            a2 += - W4*col[8*4];
            a3 += + W4*col[8*4];
        }

        if (col[8*5]) {
            MAC16(b0, + W5, col[8*5]);
            MAC16(b1, - W1, col[8*5]);
            MAC16(b2, + W7, col[8*5]);
            MAC16(b3, + W3, col[8*5]);
        }

        if(col[8*6]){
            a0 += + W6*col[8*6];
            a1 += - W2*col[8*6];
            a2 += + W2*col[8*6];
            a3 += - W6*col[8*6];
        }

        if (col[8*7]) {
            MAC16(b0, + W7, col[8*7]);
            MAC16(b1, - W5, col[8*7]);
            MAC16(b2, + W3, col[8*7]);
            MAC16(b3, - W1, col[8*7]);
        }

        col[0 ] = ((a0 + b0) >> COL_SHIFT);
        col[8 ] = ((a1 + b1) >> COL_SHIFT);
        col[16] = ((a2 + b2) >> COL_SHIFT);
        col[24] = ((a3 + b3) >> COL_SHIFT);
        col[32] = ((a3 - b3) >> COL_SHIFT);
        col[40] = ((a2 - b2) >> COL_SHIFT);
        col[48] = ((a1 - b1) >> COL_SHIFT);
        col[56] = ((a0 - b0) >> COL_SHIFT);
}

void ff_simple_idct_put(uint8_t *dest, int line_size, DCTELEM *block)
{
    int i;
    for(i=0; i<8; i++)
        idctRowCondDC(block + i*8);

    for(i=0; i<8; i++)
        idctSparseColPut(dest + i, line_size, block + i);
}

void ff_simple_idct_add(uint8_t *dest, int line_size, DCTELEM *block)
{
    int i;
    for(i=0; i<8; i++)
        idctRowCondDC(block + i*8);

    for(i=0; i<8; i++)
        idctSparseColAdd(dest + i, line_size, block + i);
}

void ff_simple_idct(DCTELEM *block)
{
    int i;
    for(i=0; i<8; i++)
        idctRowCondDC(block + i*8);

    for(i=0; i<8; i++)
        idctSparseCol(block + i);
}
