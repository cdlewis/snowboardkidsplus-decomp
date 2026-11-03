#pragma once

#include "psx/libspu.h"

typedef struct {
    u32 mask;
    s32 mode;
    SpuVolume depth;
    s32 delay, feedback;
} SpuReverbAttr;

extern SpuReverbAttr _svm_rattr;
extern s32 SpuSetReverbModeParam(SpuReverbAttr *attr);
