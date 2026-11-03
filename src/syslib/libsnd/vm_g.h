#pragma once

#include "common.h"

/* SDK VM current state; the later layout includes tone_min and tone_max.
 * https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libsnd/vm_seq.c
 */
typedef struct SvmCurrent {
    /* 0x00 */ s8 prog_tones;
    /* 0x01 */ s8 vabId;
    /* 0x02 */ s8 note;
    /* 0x03 */ s8 fine;
    /* 0x04 */ s8 volume;
    /* 0x05 */ s8 pan;
    /* 0x06 */ s8 prog;
    /* 0x07 */ s8 fake_program;
    /* 0x08 */ s8 field08;
    /* 0x09 */ s8 field09;
    /* 0x0A */ s8 mvol;
    /* 0x0B */ s8 mpan;
    /* 0x0C */ s8 tone;
    /* 0x0D */ s8 tone_vol;
    /* 0x0E */ s8 tone_pan;
    /* 0x0F */ s8 tone_prior;
    /* 0x10 */ s8 tone_center;
    /* 0x11 */ u8 tone_shift;
    /* 0x12 */ s8 tone_min;
    /* 0x13 */ s8 tone_max;
    /* 0x14 */ u8 tone_mode;
    /* 0x15 */ u8 reserved15;
    /* 0x16 */ s16 seq_sep_no;
    /* 0x18 */ s16 tone_vag_idx;
    /* 0x1A */ s16 voice;
    /* 0x1C */ s16 voiceOffset;
    /* 0x1E */ s16 field1e;
} SvmCurrent;

extern SvmCurrent _svm_cur;
