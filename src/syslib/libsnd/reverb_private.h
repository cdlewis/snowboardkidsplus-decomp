#ifndef LIBSND_REVERB_PRIVATE_H
#define LIBSND_REVERB_PRIVATE_H

#include "psx/libspu.h"

typedef struct {
    u32 mask;
    s32 mode;
    SpuVolume depth;
    s32 delay, feedback;
} SpuReverbAttr;

extern SpuReverbAttr D_8013CEC8;
extern s32 SpuSetReverbModeParam(SpuReverbAttr *attr);

#endif
