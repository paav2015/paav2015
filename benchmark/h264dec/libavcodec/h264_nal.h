/*
 * Copyright 2013 TU Berlin
 *
 * This file is part of Starbench.
 *
 * This Starbench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Starbench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @author Chi Ching Chi <chi.c.chi@tu-berlin.de>
 */


#ifndef H264_NAL_H
#define H264_NAL_H

#include "avcodec.h"
#include "h264_types.h"

int decode_nal_units(NalContext *n, H264Slice *s, GetBitContext *gb);
NalContext *get_nal_context(int width, int height);
void free_nal_context(NalContext *nc);

#endif
