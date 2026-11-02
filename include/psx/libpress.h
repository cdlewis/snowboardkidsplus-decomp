#pragma once

#include "psx/libetc.h"

typedef DmaCallback DecDCCb;
typedef struct DECDCTENV {
    u8 iq_y[64];
    u8 iq_c[64];
    s16 dct[64];
} DECDCTENV;

void DecDCTReset(s32 mode);
DECDCTENV *DecDCTGetEnv(DECDCTENV *environment);
DECDCTENV *DecDCTPutEnv(DECDCTENV *environment);
void DecDCTin(u32 *buffer, s32 mode);
s32 DecDCTinSync(s32 mode);
s32 DecDCToutSync(s32 mode);
s32 DecDCTBufSize(u32 *bitstream);
void DecDCTout(u32 *buffer, s32 size);
DecDCCb DecDCTinCallback(DecDCCb callback);
DecDCCb DecDCToutCallback(DecDCCb callback);
