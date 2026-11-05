#pragma once

#include "psx/sprite.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

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

typedef struct Position3D {
    s32 x, y, z;
} Position3D;

/* UV bounds and GPU texture attributes for one animated quad frame. */
typedef struct TaskVisualFrame {
    u8 u, v, width, height;
    u16 tpage, clut;
} TaskVisualFrame;

typedef struct TaskVisualVertex {
    s16 x, y, z, pad;
} TaskVisualVertex;

/* The visual tasks use one 0x100-byte task resource. */
typedef struct TaskVisualResource {
    u8 unk00[0x50];
    Position3D position;
    u8 unk5C[4];
    s32 scaleX, scaleY, scaleZ;
    u8 unk6C[0x14];
    TaskVisualVertex vertices[4];
    TaskVisualVertex transformedVertices[4];
    TaskVisualFrame *frame;
    s16 unkC4, unkC6, unkC8;
    s16 framePosition; /* Signed 8.8 frame index. */
    u8 r0, g0, b0, code;
    u8 unkD0[0x30];
} TaskVisualResource;

extern GsOT D_800EBFF0[2];
extern u16 D_8013AF78 __attribute__((section(".data")));

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
struct GsOT;

u32 packTextureColor(TextureColor *color);
void addFlatLine(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 r, s32 g, s32 b);
void addFlatQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 x2, s32 y2, s32 x3, s32 y3, s32 r, s32 g, s32 b);
void addFlatRectangle(u32 *orderingTable, s32 x, s32 y, s32 w, s32 h,
                      s32 r, s32 g, s32 b);
void addFlatRectangleWithDrawMode(u32 *orderingTable, s32 tpage, s32 flags,
                                  s32 x, s32 y, s32 w, s32 h,
                                  s32 r, s32 g, s32 b);
void addTexturedRectangle(u32 *orderingTable, s32 clut, s32 x, s32 y,
                          s32 w, s32 h, s32 r, s32 g, s32 b);
/* Copy a tagged packet into the current buffer and link it into the table. */
void addCopiedPrimitive(u32 *orderingTable, u32 *packet);
void initSprite(GsSPRITE *sprite);
void setTaskVisualFrame(TaskVisualResource *resource, TaskVisualFrame *frames,
                        s32 scale);
void unpackPsxColor(u32 color, UnpackedPsxColor *out);
void setTextPalette(s32 palette);
void addTextCharacter(s32 x, s32 y, u32 character);
void initPrimitiveTemplates(void);
void initTexturedQuadTemplate(void);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
void setSpriteColor(s32 r, s32 g, s32 b);
void func_800A7D8C(s16 x, s16 y, s16 width, s16 height, s16 tpage,
                   s16 clut, GsOT *ot, u16 depth);
void drawFullScreenTexturedBackground(void);
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, struct GsOT *ot, u16 depth);
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, struct GsOT *ot, u16 depth);
void startPrimaryValueTransition(s32 duration, s32 start, s32 end);

void readVram16Pixels(s32 x, s32 y, u32 *pixels);
void releaseSavedImageEntry(u16 index);
void requestSavedImageRestore(u16 index);
void uploadImage64x256(u32 *pixels, s16 x, s16 y);
void resetPaletteRecords(void);
void setPaletteRecordColor(u16 index, u8 r, u8 g, u8 b);
void startPaletteCycle(u16 direction, u16 index, u8 first, u8 last, u8 interval);
void func_80071300(s16 mode, s16 unused, GameBackground *background,
                   GameBackgroundMap *map, u8 *cells, u8 *tiles);
void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map);

/* Upload a TIM image, optionally offsetting its pixel and CLUT positions. */
void func_80015EE0(u32 *tim, s16 *position);
void uploadBufferedTexturePair(void);

/* Add a full-screen semi-transparent tile to an ordering table. */
void func_80070C5C(s32 intensity, struct GsOT *ot, u16 depth);
void startThreeFrameScreenBlend(void);

void func_8006C998(s16 x, s16 y, s16 sprite, s32 palette, s32 blendMode,
                   GsOT *ot, u16 depth);
void drawMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, GsOT *ot, u16 depth);
