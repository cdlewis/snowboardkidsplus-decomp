#ifndef PSX_LIBSPU_H
#define PSX_LIBSPU_H

#include "decomp/types.h"

#define SPU_TRANSFER_BY_DMA 0
#define SPU_TRANSFER_BY_IO 1
u32 SpuSetNoiseVoice(s32 on_off, u32 voice_bit);
u32 SpuSetReverbVoice(s32 on_off, u32 voice_bit);
s32 SpuSetTransferMode(s32 mode);

#endif
