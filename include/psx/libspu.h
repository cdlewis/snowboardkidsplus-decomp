#pragma once

#include "decomp/types.h"

typedef struct {
    s16 left, right;
} SpuVolume;

typedef struct {
    SpuVolume volume;
    s32 reverb, mix;
} SpuExtAttr;

typedef struct {
    u32 mask;
    SpuVolume mvol, mvolmode, mvolx;
    SpuExtAttr cd, ext;
} SpuCommonAttr;

void SpuSetCommonAttr(SpuCommonAttr *attr);
s32 SpuInitMalloc(s32 num, char *top);
void SpuGetVoiceVolume(s32 voice, s16 *left, s16 *right);

#define SPU_TRANSFER_BY_DMA 0
#define SPU_TRANSFER_BY_IO 1
u32 SpuSetNoiseVoice(s32 on_off, u32 voice_bit);
u32 SpuSetReverbVoice(s32 on_off, u32 voice_bit);
s32 SpuIsTransferCompleted(s32 immediate);
s32 SpuSetTransferMode(s32 mode);
u32 SpuWrite(u8 *address, u32 size);
