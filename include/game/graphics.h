#pragma once

#include "psx/sprite.h"
#include "psx/libgpu.h"

/* Unscaled 5-bit RGB channels and the PSX color's semi-transparency bit. */
typedef struct UnpackedPsxColor {
    u8 r, g, b, stp;
} UnpackedPsxColor;

/* A draw-mode command and sprite share a single DMA packet tag. */
typedef struct TexturedSpritePacket {
    u32 tag;
    u32 drawMode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} TexturedSpritePacket;

extern POLY_FT4 gTexturedQuadTemplate;
extern SPRT gSpriteTemplate;
extern TexturedSpritePacket gTexturedSpriteTemplate;

void initSprite(GsSPRITE *sprite);
void unpackPsxColor(u32 color, UnpackedPsxColor *out);
void initPrimitiveTemplates(void);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
void setSpriteColor(s32 r, s32 g, s32 b);

void releaseSavedImageEntry(u16 index);
void requestSavedImageRestore(u16 index);
