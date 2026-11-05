#pragma once

#include "psx/sprite.h"
#include "psx/libgpu.h"

extern POLY_FT4 gTexturedQuadTemplate;

void initSprite(GsSPRITE *sprite);
void initPrimitiveTemplates(void);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
