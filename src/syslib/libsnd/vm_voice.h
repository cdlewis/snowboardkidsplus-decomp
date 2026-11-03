#pragma once

#include "common.h"

/* Later SDK voice layout: 24 records, each 0x36 bytes.
 * https://github.com/hansbonini/psx_tomba/blob/b0aab7ef43785b257f66fb007df36b222e1a3665/src/psyq/libsnd/vm_aloc1.c
 */
typedef struct SvmVoice {
    /* 0x00 */ s16 field0;
    /* 0x02 */ s16 field02;
    /* 0x04 */ s16 field4;
    /* 0x06 */ s16 field06;
    /* 0x08 */ s16 field08;
    /* 0x0A */ u8 field0a;
    /* 0x0B */ u8 field0b;
    /* 0x0C */ s16 field0c;
    /* 0x0E */ s16 note;
    /* 0x10 */ s16 seq_id;
    /* 0x12 */ s16 fake_program;
    /* 0x14 */ s16 prog;
    /* 0x16 */ s16 tone;
    /* 0x18 */ s16 vabId;
    /* 0x1A */ s16 priority;
    /* 0x1C */ u8 field1c;
    /* 0x1D */ u8 field1d;
    /* 0x1E */ s16 auto_vol;
    /* 0x20 */ s16 field20;
    /* 0x22 */ s16 field22;
    /* 0x24 */ s16 start_vol;
    /* 0x26 */ s16 end_vol;
    /* 0x28 */ s16 auto_pan;
    /* 0x2A */ s16 field2a;
    /* 0x2C */ s16 field2c;
    /* 0x2E */ s16 field2e;
    /* 0x30 */ s16 start_pan;
    /* 0x32 */ s16 end_pan;
    /* 0x34 */ s16 field34;
} SvmVoice;

extern SvmVoice _svm_voice[24];
