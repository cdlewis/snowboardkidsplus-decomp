#pragma once

#include "decomp/types.h"
#include "psx/libgpu.h"

typedef struct {
    u32 length;
    u32 *org;
    u32 offset, point;
    u32 *tag;
} GsOT;

extern DRAWENV GsDRAWENV;
extern DISPENV GsDISPENV;
extern s16 PSDGPU;
