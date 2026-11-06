#pragma once

#include "psx/sprite.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

typedef struct PlayerTextureOrigin {
    s16 x, y;
} PlayerTextureOrigin;

extern PlayerTextureOrigin D_801204D4[4];

/* A DMA packet containing four GPU command words. */
typedef struct GpuPacket4 {
    u32 tag;
    u32 commands[4];
} GpuPacket4;

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

/* Texture resource and frame indices shared by quad and sprite drawing. */
typedef struct TexturedQuadFrame {
    s16 textureIndex;
    s16 frameIndex;
} TexturedQuadFrame;

/* Entries end when palette is at least 255. */
typedef struct TexturedQuadListEntry {
    s16 quadIndex;
    s16 palette;
    s16 x, y;
    s16 width, height;
} TexturedQuadListEntry;

extern TexturedQuadFrame D_8012DFF4[];

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
    GsCOORDINATE2 coordinate;
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
extern GsOT D_800EC018[2];

void sortLitObject(GsDOBJ2 *object);
extern u32 *D_8013AF6C __attribute__((section(".data")));
extern u16 D_8013AF7A __attribute__((section(".data")));
extern u16 D_8013AF7C __attribute__((section(".data")));

void drawFixedSpriteAndBottomBarTask(void);
extern u16 D_8013AF78 __attribute__((section(".data")));

extern POLY_FT4 gTexturedQuadTemplate;
typedef struct SpritePrimitiveTemplate {
    DR_TPAGE drawPage;
    SPRT sprite;
} SpritePrimitiveTemplate;

extern SpritePrimitiveTemplate gSpritePrimitiveTemplate;
#define gSpriteTemplate (gSpritePrimitiveTemplate.sprite)
extern TexturedSpritePacket gTexturedSpriteTemplate;

void drawNoProcessMessage(void);
void drawCyanBackgroundAndSetFog(void);
void createResourceLoadTask(void);
void initializeObjectVisualTask(void);
void initializeInteractionVisualTask(void);
void createPlayerSpriteAnimationTask(s32 x, s32 y, s32 player, s32 checkPlayer, u16 alternateLayer);
void createPlayerNumberDisplayTask(s32 value, s32 showExtraValue, s32 player);
void createActivePlayerSlidingNumberTask(s32 value, s16 player);
void createPositionedEffectTask(s32 x, s32 y, s32 z);
void createPlayerTrailTask(s32 playerIndex);
void createRotatingQuadTrailTask(void);
void createQuadShrinkTask(void);
void createRightQuadShrinkTask(void);
typedef struct TextureColor {
    u8 r, g, b;
    u8 stp; /* Low bit becomes the texture color's semi-transparency bit. */
} TextureColor;

typedef struct GameBackground GameBackground;
typedef struct GameBackgroundMap GameBackgroundMap;
struct GsOT;

void readVramPixelColor(s32 x, s32 y, TextureColor *out);
u32 packTextureColor(TextureColor *color);
void addFlatTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                     s32 x2, s32 y2, s32 r, s32 g, s32 b);
void addGouraudTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 r0, s32 g0, s32 b0,
                  s32 r1, s32 g1, s32 b1, s32 r2, s32 g2, s32 b2);
void addTexturedTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                         s32 x2, s32 y2, u8 tpage, s32 clut,
                         s32 u0, s32 v0, s32 u1, s32 v1, s32 u2, s32 v2,
                         s32 r, s32 g, s32 b);
void addSemiTransparentTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                                s32 x1, s32 y1, s32 x2, s32 y2,
                                s32 r, s32 g, s32 b);
void addSemiTransparentGouraudTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                  s32 x1, s32 y1, s32 x2, s32 y2,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2);
void addFlatLine(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 r, s32 g, s32 b);
void addFlatQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 x2, s32 y2, s32 x3, s32 y3, s32 r, s32 g, s32 b);
void addSemiTransparentFlatQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                                          s32 x0, s32 y0, s32 x1, s32 y1,
                                          s32 x2, s32 y2, s32 x3, s32 y3,
                                          s32 r, s32 g, s32 b);
void addFlatRectangle(u32 *orderingTable, s32 x, s32 y, s32 w, s32 h,
                      s32 r, s32 g, s32 b);
void addFlatRectangleWithDrawMode(u32 *orderingTable, s32 tpage, s32 flags,
                                  s32 x, s32 y, s32 w, s32 h,
                                  s32 r, s32 g, s32 b);
void addTexturedRectangle(u32 *orderingTable, s32 clut, s32 x, s32 y,
                          s32 w, s32 h, s32 r, s32 g, s32 b);
/* Copy a tagged packet into the current buffer and link it into the table. */
void addCopiedPrimitive(u32 *orderingTable, u32 *packet);
void addTexturedSprite(u32 *orderingTable, s32 tpage, s32 clut, s32 x,
                       s32 y, s32 w, s32 h, u16 u, u16 v,
                       s32 r, s32 g, s32 b);
void initSprite(GsSPRITE *sprite);
void prepareTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frame, u8 usePitch);
void setTaskVisualFrame(TaskVisualResource *resource, TaskVisualFrame *frames,
                        s32 scale);
void unpackPsxColor(u32 color, UnpackedPsxColor *out);
void setTextPalette(s32 palette);
void addTextCharacter(s32 x, s32 y, u32 character);
void addStretchedTextCharacter(s32 x, s32 y, s32 width, u32 character);
void initPrimitiveTemplates(void);
void initTexturedQuadTemplate(void);
void initSpritePrimitiveTemplate(void);
void setSpriteTextureBlend(s32 index, s32 palette, s32 blend);
void setTexturedQuadTexture(s32 index, s32 palette);
void setTexturedQuadTextureBlend(s32 index, s32 palette, s32 blend);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
void drawStretchedTexturedQuad(s16 x, s16 y, s16 width, s16 height, s32 index,
                                GsOT *ot, u16 depth);
void drawTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void drawInclusiveTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void setSpriteColor(s32 r, s32 g, s32 b);
void func_8006CF38(s32 x, s32 y, s32 frame, s32 palette, s32 blend,
                   s32 pivotX, s32 pivotY, s32 scaleX, s32 scaleY,
                   s32 rotation, GsOT *ot, s32 depth);
void func_800A7D8C(s16 x, s16 y, s16 width, s16 height, s16 tpage,
                   s16 clut, GsOT *ot, u16 depth);
void drawFullScreenTexturedBackground(void);
void drawBlackScreenTask(void);
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, struct GsOT *ot, u16 depth);
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, struct GsOT *ot, u16 depth);
void startPrimaryValueTransition(s32 duration, s32 start, s32 end);

s32 func_800164E8(s16 x, s16 y, u32 *pixels, u16 duration);
u16 saveVram16Pixels(s32 x, s32 y);
void readVram16Pixels(s32 x, s32 y, u32 *pixels);
void releaseSavedImageEntry(u16 index);
void requestSavedImageRestore(u16 index);
void uploadImage64x256(u32 *pixels, s16 x, s16 y);
void resetPaletteRecords(void);
void setPaletteRecordColor(u16 index, u8 r, u8 g, u8 b);
void startPaletteCycle(u16 direction, u16 index, u8 first, u8 last, u8 interval);
void func_80071300(s16 mode, s16 unused, GameBackground *background,
                   GameBackgroundMap *map, u8 *cells, u8 *tiles);
void uploadTextureArchiveEntry(s16 x, s16 y, s16 clutX, s16 clutY, s32 index);
void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map);

/* Upload a TIM image, optionally offsetting its pixel and CLUT positions. */
void uploadTimImageWithOffset(u32 *tim, s16 *position);
void uploadBufferedTexturePair(void);

/* Add a full-screen semi-transparent tile to an ordering table. */
void func_80070C5C(s32 intensity, struct GsOT *ot, u16 depth);
void startThreeFrameScreenBlend(void);

void func_8006C998(s16 x, s16 y, s16 sprite, s32 palette, s32 blendMode,
                   GsOT *ot, u16 depth);
void drawMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, GsOT *ot, u16 depth);
struct GsOT;
extern struct GsOT D_800EBFF0[2];
extern u16 D_8013AF78 __attribute__((section(".data")));

void func_8006D41C(s16 x, s16 y, s16 limit, const char *text,
                   s16 palette, s16 blend, struct GsOT *ot, u16 depth);
void addDrawPagePacket(DR_TPAGE *source, struct GsOT *ot, u16 depth);
void addSpritePacket(SPRT *source, struct GsOT *ot, u16 depth);
void addFourWordGpuPacket(GpuPacket4 *source, GsOT *ot, u16 depth);
void addTexturedSpritePacket(TexturedSpritePacket *source, GsOT *ot, u16 depth);

void drawTexturedFrameStyle9(s16 x, s16 y, s16 w, s16 h, s16 palette,
                             s16 blend, struct GsOT *ot, u16 depth);

void drawPaletteMappedIcon(s16 x, s16 y, s16 index, struct GsOT *ot, u16 depth);

extern u32 gSynchronizedFrameCount;
void waitForFrameHcount(u32 minimumHcount);

void func_8006C240(s16 x, s16 y, s16 width, s16 height, s16 index, s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawTexturedQuadList(s16 x, s16 y, TexturedQuadListEntry *entry, s16 blend, GsOT *ot, u16 depth);

void drawDemonstrationOverlay(void);
void func_8006F57C(s16 x, s16 y, s16 count, const char *text, s32 palette,
                   s16 blend, GsOT *ot, u16 depth);

void drawAnimatedSprite16(s16 frame, s16 x, s16 y, s16 unused, GsOT *ot, u16 depth);
void drawIndexedFlatTriangle(void);
