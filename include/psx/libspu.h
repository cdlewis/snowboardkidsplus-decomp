#ifndef PSX_LIBSPU_H
#define PSX_LIBSPU_H

#include "decomp/types.h"

typedef struct {
    s16 left, right;
} SpuVolume;

s32 SpuInitMalloc(s32 num, char *top);
void SpuGetVoiceVolume(s32 voice, s16 *left, s16 *right);

#define SPU_TRANSFER_BY_DMA 0
#define SPU_TRANSFER_BY_IO 1
u32 SpuSetNoiseVoice(s32 on_off, u32 voice_bit);
u32 SpuSetReverbVoice(s32 on_off, u32 voice_bit);
s32 SpuIsTransferCompleted(s32 immediate);
s32 SpuSetTransferMode(s32 mode);

#endif
