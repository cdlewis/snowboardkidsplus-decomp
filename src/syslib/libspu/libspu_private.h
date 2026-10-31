#ifndef LIBSPU_PRIVATE_H
#define LIBSPU_PRIVATE_H

#include "psx/libspu.h"

/* Shared state is owned by the SDK spu data object, still in split data. */
extern s32 _spu_inTransfer;
extern s32 _spu_trans_mode;
extern s32 _spu_transMode;
u32 _SpuSetAnyVoice(s32 on_off, u32 voice_bit, s32 first, s32 second);
void _spu_setInTransfer(s32 value);
s32 _spu_getInTransfer(void);

#endif
