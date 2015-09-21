/*
 * H.26L/H.264/AVC/JVT/14496-10/... sei decoding
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 * H.264 / AVC / MPEG4 part10 sei decoding.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "avcodec.h"
#include "h264_types.h"
#include "golomb.h"

//#undef NDEBUG
#include <assert.h>

static const uint8_t sei_num_clock_ts_table[9]={
    1,  1,  1,  2,  2,  3,  3,  2,  3
};

void ff_h264_reset_sei(NalContext *n) {
    n->sei_recovery_frame_cnt       = -1;
    n->sei_dpb_output_delay         =  0;
    n->sei_cpb_removal_delay        = -1;
    n->sei_buffering_period_present =  0;
}

static int decode_picture_timing(NalContext *n, GetBitContext *gb){
    if(n->sps.nal_hrd_parameters_present_flag || n->sps.vcl_hrd_parameters_present_flag){
        n->sei_cpb_removal_delay = get_bits(gb, n->sps.cpb_removal_delay_length);
        n->sei_dpb_output_delay = get_bits(gb, n->sps.dpb_output_delay_length);
    }
    if(n->sps.pic_struct_present_flag){
        unsigned int i, num_clock_ts;
        n->sei_pic_struct = get_bits(gb, 4);
        n->sei_ct_type    = 0;

        if (n->sei_pic_struct > SEI_PIC_STRUCT_FRAME_TRIPLING)
            return -1;

        num_clock_ts = sei_num_clock_ts_table[n->sei_pic_struct];

        for (i = 0 ; i < num_clock_ts ; i++){
            if(get_bits(gb, 1)){                  /* clock_timestamp_flag */
                unsigned int full_timestamp_flag;
                n->sei_ct_type |= 1<<get_bits(gb, 2);
                skip_bits(gb, 1);                 /* nuit_field_based_flag */
                skip_bits(gb, 5);                 /* counting_type */
                full_timestamp_flag = get_bits(gb, 1);
                skip_bits(gb, 1);                 /* discontinuity_flag */
                skip_bits(gb, 1);                 /* cnt_dropped_flag */
                skip_bits(gb, 8);                 /* n_frames */
                if(full_timestamp_flag){
                    skip_bits(gb, 6);             /* seconds_value 0..59 */
                    skip_bits(gb, 6);             /* minutes_value 0..59 */
                    skip_bits(gb, 5);             /* hours_value 0..23 */
                }else{
                    if(get_bits(gb, 1)){          /* seconds_flag */
                        skip_bits(gb, 6);         /* seconds_value range 0..59 */
                        if(get_bits(gb, 1)){      /* minutes_flag */
                            skip_bits(gb, 6);     /* minutes_value 0..59 */
                            if(get_bits(gb, 1))   /* hours_flag */
                                skip_bits(gb, 5); /* hours_value 0..23 */
                        }
                    }
                }
                if(n->sps.time_offset_length > 0)
                    skip_bits(gb, n->sps.time_offset_length); /* time_offset */
            }
        }
    }
    return 0;
}

static int decode_unregistered_user_data(GetBitContext *gb, int size){
    char user_data[16+256];
    int e, build, i;

    if(size<16)
        return -1;

    for(i=0; i<(int) sizeof(user_data)-1 && i<size; i++){
        user_data[i]= get_bits(gb, 8);
    }

    user_data[i]= 0;
    e= sscanf(user_data+16, "x264 - core %d"/*%s - H.264/MPEG-4 AVC codec - Copyleft 2005 - http://www.videolan.org/x264.html*/, &build);
    (void) e;
    for(; i<size; i++)
        skip_bits(gb, 8);

    return 0;
}

static int decode_recovery_point(NalContext *n, GetBitContext *gb){

    n->sei_recovery_frame_cnt = get_ue_golomb(gb);
    skip_bits(gb, 4);       /* 1b exact_match_flag, 1b broken_link_flag, 2b changing_slice_group_idc */

    return 0;
}

static int decode_buffering_period(NalContext *n, GetBitContext *gb){
    unsigned int sps_id;
    int sched_sel_idx;
    SPS *sps;

    sps_id = get_ue_golomb_31(gb);
    if(sps_id > 31 || !n->sps_buffers[sps_id]) {
        av_log(AV_LOG_ERROR, "non-existing SPS %d referenced in buffering period\n", sps_id);
        return -1;
    }
    sps = n->sps_buffers[sps_id];

    // NOTE: This is really so duplicated in the standard... See H.264, D.1.1
    if (sps->nal_hrd_parameters_present_flag) {
        for (sched_sel_idx = 0; sched_sel_idx < sps->cpb_cnt; sched_sel_idx++) {
            n->initial_cpb_removal_delay[sched_sel_idx] = get_bits(gb, sps->initial_cpb_removal_delay_length);
            skip_bits(gb, sps->initial_cpb_removal_delay_length); // initial_cpb_removal_delay_offset
        }
    }
    if (sps->vcl_hrd_parameters_present_flag) {
        for (sched_sel_idx = 0; sched_sel_idx < sps->cpb_cnt; sched_sel_idx++) {
            n->initial_cpb_removal_delay[sched_sel_idx] = get_bits(gb, sps->initial_cpb_removal_delay_length);
            skip_bits(gb, sps->initial_cpb_removal_delay_length); // initial_cpb_removal_delay_offset
        }
    }

    n->sei_buffering_period_present = 1;
    return 0;
}

int ff_h264_decode_sei(NalContext *n, GetBitContext *gb){
    while(get_bits_count(gb) + 16 < gb->size_in_bits){
        int size, type;

        type=0;
        do{
            type+= show_bits(gb, 8);
        }while(get_bits(gb, 8) == 255);

        size=0;
        do{
            size+= show_bits(gb, 8);
        }while(get_bits(gb, 8) == 255);

        switch(type){
        case SEI_TYPE_PIC_TIMING: // Picture timing SEI
            if(decode_picture_timing(n, gb) < 0)
                return -1;
            break;
        case SEI_TYPE_USER_DATA_UNREGISTERED:
            if(decode_unregistered_user_data(gb, size) < 0)
                return -1;
            break;
        case SEI_TYPE_RECOVERY_POINT:
            if(decode_recovery_point(n, gb) < 0)
                return -1;
            break;
        case SEI_BUFFERING_PERIOD:
            if(decode_buffering_period(n, gb) < 0)
                return -1;
            break;
        default:
            skip_bits(gb, 8*size);
        }

        //FIXME check bits here
        align_get_bits(gb);
    }

    return 0;
}
