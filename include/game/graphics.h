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

void drawNoProcessMessage(void);
void createResourceLoadTask(void);
typedef struct TextureColor {
    u8 r, g, b;
    u8 stp; /* Low bit becomes the texture color's semi-transparency bit. */
} TextureColor;

typedef struct GameBackground GameBackground;
typedef struct GameBackgroundMap GameBackgroundMap;

u32 packTextureColor(TextureColor *color);
void initSprite(GsSPRITE *sprite);
void unpackPsxColor(u32 color, UnpackedPsxColor *out);
void setTextPalette(s32 palette);
void initPrimitiveTemplates(void);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
void setSpriteColor(s32 r, s32 g, s32 b);

void readVram16Pixels(s32 x, s32 y, u32 *pixels);
void releaseSavedImageEntry(u16 index);
void requestSavedImageRestore(u16 index);
void resetPaletteRecords(void);
void setPaletteRecordColor(u16 index, u8 r, u8 g, u8 b);
void func_80071300(s16 mode, s16 unused, GameBackground *background,
                   GameBackgroundMap *map, u8 *cells, u8 *tiles);
void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map);
