#pragma once

#include "psx/sprite.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

/* Packed texture adjustments for a TMD object's CLUT, UVs, and texture page. */
typedef struct ModelTextureOffset {
    u16 clut;
    u16 uv;
    u16 tpage;
} ModelTextureOffset;

extern ModelTextureOffset gPlayerRootTextureOffsets[4];
extern ModelTextureOffset gPlayerPartTextureOffsets[4];
void func_8001604C(u16 part, u32 *model, ModelTextureOffset *offset);

#ifdef GRAPHICS_GP_ADDRESSING
#define GRAPHICS_DATA
#else
#define GRAPHICS_DATA __attribute__((section(".data")))
#endif

typedef struct PlayerTextureOrigin {
    s16 x, y;
} PlayerTextureOrigin;

extern PlayerTextureOrigin D_801204D4[4];
extern POLY_F3 gNumberAdjustmentArrow;

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
    s32 groundY, velocityY, unkD8;
    s32 unkDC;
    s32 unkE0, bounceCount;
    u8 unkE8[0x18];
} TaskVisualResource;

/* Sprite palettes indexed by the character nibble in saved records. */
extern u8 D_80124284[];

extern GsOT D_800EBFF0[2];
extern GsOT D_800EC018[2];
extern GsOT D_800EC0B8[2];
extern GsOT D_800EC090[2];
extern GsOT D_800EC130[2];

u16 func_80059254(void);
void drawTaskObject(u16 index);
void drawTaskObjectAlternateLayer(u16 index);

extern GsDOBJ2 D_80129A80[30];
void sortLitObject(GsDOBJ2 *object);
void sortLitObjectWithDrawArea(GsDOBJ2 *object, u16 index, RECT *bounds);
extern u32 *D_8013AF6C GRAPHICS_DATA;
extern u16 D_8013AF7A GRAPHICS_DATA;
extern u16 D_8013AF7C GRAPHICS_DATA;

void drawFixedSpriteAndBottomBarTask(void);
extern u16 D_8013AF78 GRAPHICS_DATA;

extern POLY_FT4 gTexturedQuadTemplate;
typedef struct SpritePrimitiveTemplate {
    DR_TPAGE drawPage;
    SPRT sprite;
} SpritePrimitiveTemplate;

extern SpritePrimitiveTemplate gSpritePrimitiveTemplate;
extern GsSPRITE gRotatedSpriteTemplate;
#define gSpriteTemplate (gSpritePrimitiveTemplate.sprite)
extern TexturedSpritePacket gTexturedSpriteTemplate;

void drawNoProcessMessage(void);
void drawCyanBackgroundAndSetFog(void);
void createResourceLoadTask(void);
void initializeObjectVisualTask(void);
void initializeInteractionVisualTask(void);
void initializeCourseVisualTask(void);
extern TaskVisualFrame D_8013B29C[1];
extern TaskVisualFrame D_801294AC[2];
extern TaskVisualFrame *gCourseVisualFrames;
void createPlayerSpriteAnimationTask(s32 x, s32 y, s32 player, s32 checkPlayer, u16 alternateLayer);
void createPlayerNumberDisplayTask(s32 value, s32 showExtraValue, s32 player);
void createActivePlayerSlidingNumberTask(s32 value, s16 player);
void createFallingParticleTask(void);
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
void addGouraudQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3);
void addFlatQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 x2, s32 y2, s32 x3, s32 y3, s32 r, s32 g, s32 b);
void addSemiTransparentFlatQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                                          s32 x0, s32 y0, s32 x1, s32 y1,
                                          s32 x2, s32 y2, s32 x3, s32 y3,
                                          s32 r, s32 g, s32 b);
void addSemiTransparentGouraudQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                  s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3);
void addFlatRectangle(u32 *orderingTable, s32 x, s32 y, s32 w, s32 h,
                      s32 r, s32 g, s32 b);
void addFlatRectangleWithDrawMode(u32 *orderingTable, s32 tpage, s32 flags,
                                  s32 x, s32 y, s32 w, s32 h,
                                  s32 r, s32 g, s32 b);
void addTexturedQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3, s32 flags, u8 tpage, s32 clut,
                  s32 u0, s32 v0, s32 u1, s32 v1, s32 u2, s32 v2,
                  s32 u3, s32 v3, s32 r, s32 g, s32 b);
void addGouraudTexturedQuad(u32 *orderingTable, s16 x0, s16 y0, s16 x1, s16 y1,
                  s16 x2, s16 y2, s16 x3, s16 y3, u8 tpage, s16 clut,
                  u8 u0, u8 v0, u8 u1, u8 v1, u8 u2, u8 v2,
                  u8 u3, u8 v3, u8 r0, u8 g0, u8 b0,
                  u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2,
                  u8 r3, u8 g3, u8 b3);
void addTexturedRectangle(u32 *orderingTable, s32 clut, s32 x, s32 y,
                          s32 w, s32 h, s32 r, s32 g, s32 b);
/* Copy a tagged packet into the current buffer and link it into the table. */
void addCopiedPrimitive(u32 *orderingTable, u32 *packet);
void addTexturedSprite(u32 *orderingTable, s32 tpage, s32 clut, s32 x,
                       s32 y, s32 w, s32 h, u16 u, u16 v,
                       s32 r, s32 g, s32 b);
void initSprite(GsSPRITE *sprite);
void drawTaskVisualPositionHistory(TaskVisualResource *resource, VECTOR *history,
                                   TaskVisualFrame *frames);
void func_8005A788(TaskVisualResource *resource, TaskVisualFrame *frames, u8 mode);
void prepareTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frame, u8 usePitch);
extern TaskVisualFrame D_80124304[];
extern TaskVisualFrame D_80124354[];
void func_8005AC4C(TaskVisualResource *resource, u8 mode);
u16 func_80059254(void);
s16 func_80059290(TaskVisualResource *resource);
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
void drawSizedTextGlyph(s16 x, s16 y, s16 width, s16 height, const char *text,
                        s16 palette, s16 blend, GsOT *ot, u16 depth);
void setTexturedQuadTexture(s32 index, s32 palette);
void setTexturedQuadTextureBlend(s32 index, s32 palette, s32 blend);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
void drawTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void drawStretchedTexturedQuad(s16 x, s16 y, s16 width, s16 height, s32 index,
                                GsOT *ot, u16 depth);
void drawTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
/* A negative blend mode selects opaque drawing. */
void drawTexturedQuadByIndexWithBlend(s16 x, s16 y, s32 index, s16 palette,
                                      s16 blendMode, GsOT *ot, u16 depth);
void drawInclusiveTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void drawInclusiveTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void setSpriteColor(s32 r, s32 g, s32 b);
void func_8007240C(s16 x, s16 y, s16 width, s16 height, u8 r, u8 g, u8 b,
                   s16 blendMode, GsOT *ot, u16 depth);
/* A pivot of 0x7FFF selects the center of that dimension. */
void drawTransformedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex,
                                  s16 blendMode, s16 pivotX, s16 pivotY,
                                  s16 scaleX, s16 scaleY, s32 rotation,
                                  GsOT *ot, u16 depth);
void drawPagedTexturedRectangle(s16 x, s16 y, s16 width, s16 height, s16 tpage,
                   s16 clut, GsOT *ot, u16 depth);
void drawFullScreenTexturedBackground(void);
void drawBlackScreenTask(void);
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, struct GsOT *ot, u16 depth);
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, struct GsOT *ot, u16 depth);
void startPrimaryValueTransition(s32 duration, s32 start, s32 end);

s32 startPaletteTransition(s16 x, s16 y, u32 *pixels, u16 duration);
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
/* The count occupies the low halfword; subsequent words are tagged byte offsets. */
typedef union TimArchive {
    u16 count;
    s32 words[1];
} TimArchive;

void uploadTimArchive(TimArchive *archive, s16 *position);
void uploadTimImageWithOffset(u32 *tim, s16 *position);
void uploadBufferedTexturePair(void);
/* Queue the buffered screen sprites shifted one pixel in each direction. */
void queueSelectedCameraOverlayPackets(void);
void queueOffsetScreenCopySprites(void);

/* Add a full-screen semi-transparent tile to an ordering table. */
void func_80070C5C(s32 intensity, struct GsOT *ot, u16 depth);
void startThreeFrameScreenBlend(void);

void drawSpriteTextureRegion(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void func_8006C998(s16 x, s16 y, s32 sprite, s32 palette, s32 blendMode,
                   GsOT *ot, u16 depth);
void func_8006D984(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                   s16 palette, s16 blendMode, GsOT *ot, u16 depth);
void func_8006D66C(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                   s16 palette, s16 blendMode, GsOT *ot, u16 depth);
void func_8006DFB4(s16 x, s16 y, s16 digits, s16 zeroFill, s32 value,
                   s16 palette, s16 blendMode, GsOT *ot, u16 depth);
/* Draw the upper three bytes as two-digit fields with time separators. */
void drawPackedTime(s16 x, s16 y, u32 value, s16 palette, s16 blend,
                    GsOT *ot, u16 depth);
void drawMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, GsOT *ot, u16 depth);
void drawScaledMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame,
                              s16 scaleX, s16 scaleY, GsOT *ot, u16 depth);
struct GsOT;
extern struct GsOT D_800EBFF0[2];
extern u16 D_8013AF78 GRAPHICS_DATA;

void drawLimitedText8x8(s16 x, s16 y, s16 limit, const char *text,
                   s16 palette, s16 blend, struct GsOT *ot, u16 depth);
void addDrawPagePacket(DR_TPAGE *source, struct GsOT *ot, u16 depth);
void addSpritePacket(SPRT *source, struct GsOT *ot, u16 depth);
void addDrawAreaPacket(s32 x, s32 y, s32 width, s32 height, GsOT *ot, u16 depth);
void addFourWordGpuPacket(GpuPacket4 *source, GsOT *ot, u16 depth);
void addTexturedSpritePacket(TexturedSpritePacket *source, GsOT *ot, u16 depth);

void drawTexturedFrameStyle9(s16 x, s16 y, s16 w, s16 h, s16 palette,
                             s16 blend, struct GsOT *ot, u16 depth);

void drawPaletteMappedIcon(s16 x, s16 y, s16 index, struct GsOT *ot, u16 depth);
void drawScaledPaletteMappedIcon(s16 x, s16 y, s16 index, s16 scaleX,
                                 s16 scaleY, GsOT *ot, u16 depth);

extern u32 gSynchronizedFrameCount;
void waitForFrameHcount(u32 minimumHcount);

void drawSkewedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 unused,
                                               s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth);
void drawStretchedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 index, s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawTexturedQuadList(s16 x, s16 y, TexturedQuadListEntry *entry, s16 blend, GsOT *ot, u16 depth);

void drawDemonstrationOverlay(void);
void drawUppercaseProportionalText(s16 x, s16 y, s16 count, const char *text, s16 palette,
                   s16 blend, GsOT *ot, u16 depth);

void drawAnimatedSprite16(s16 frame, s16 x, s16 y, s16 unused, GsOT *ot, u16 depth);
void drawIndexedFlatTriangle(void);
void drawIndexedFlatQuad(void);

void drawWrappedDoubleByteText(s16 x, s16 y, s16 columns, s8 *text,
                               s16 palette, s16 blend, GsOT *ot, u16 depth);
void func_8006EB00(s16 x, s16 y, s16 w, s16 h, s8 *text, s16 palette,
                   s16 blend, GsOT *ot, u16 depth);

void func_8006EF30(s32 x, s32 y, s16 columns, const u16 *text,
                   s32 palette, s16 blendMode, GsOT *ot, u16 depth);

void func_8006E2C4(s16 x, s16 y, s16 count, s32 mode, s32 value,
                   s32 palette, s16 blend, GsOT *ot, u16 depth);

void func_8006EC84(s16 x, s16 y, s16 columns, const u16 *text, s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawTaskShadowedText(void);
void drawPlayerModeMessagePanelTask(void);

void drawWrappedText8x8(s16 x, s16 y, s16 columns, const char *text,
                        s16 palette, s16 blend, GsOT *ot, u16 depth);
void func_8006D29C(s16 x, s16 y, s16 width, s16 height, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);

void drawWrappedText16x16(s16 x, s16 y, s16 columns, const char *text,
                          s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawLimitedWrappedText16x16(s16 x, s16 y, s16 limit, s16 columns,
                                  const char *text, s16 palette, s16 blend,
                                  GsOT *ot, u16 depth);
void func_8006F8A4(s16 x, s16 y, s16 w, s16 h, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);

void drawRotatedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex,
                              s16 blendMode, s16 angle, GsOT *ot, u16 depth);

void drawSemiTransparentTextureRect(s16 x, s16 y, s16 width, s16 height,
                                    s16 textureDepth, s16 blendMode, s16 pageX, s16 pageY,
                                    s16 clutX, s16 clutY, GsOT *ot, u16 depth);

void drawSemiTransparentTextureRectUV(s16 x, s16 y, s16 width, s16 height,
                                      s16 textureDepth, s16 blendMode, s16 pageX, s16 pageY,
                                      u16 u, u16 v, s16 clutX, s16 clutY, GsOT *ot, u16 depth);

/* Full-screen rectangle followed by the two animated player view rectangles. */
extern RECT D_800EC158[3];
extern s16 D_8013AF7E GRAPHICS_DATA;
extern s16 D_8013AF80 GRAPHICS_DATA;

void updateSplitScreenTransitionTask(void);

void drawFirstPlayerStatusHud(void);
#undef GRAPHICS_DATA

void func_8006E2C4(s16 x, s16 y, s16 digits, s16 leadingZeroes, s32 value,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawCurrentTaskPlayerValuePanel(void);
