#pragma once

/* Original unit addressing policy; include before public headers. */
#include "common.h"
#include "game/engine/angles.h"
#define CONTROLLER_INPUT_ABSOLUTE_ADDRESSING
#include "game/io/controller.h"
#include "game/io/cd_queue.h"
#define CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
#include "game/io/controller_effects.h"
#include "game/io/controller.h"
#define GS_PACKET_ABSOLUTE_ADDRESSING
#include "game/graphics/graphics.h"
#include "game/engine/linear_random.h"
#include "game/engine/random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#define TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/race/player/race_player.h"
#include "game/race/camera/race_camera.h"
#include "game/menu/selected_records.h"
#define SELECTED_RECORD_GP_ADDRESSING
#include "game/menu/selected_record.h"
#include "psx/libsnd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "game/graphics/quad_animation.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_offset.h"
#include "psx/libapi.h"
#include "game/graphics/draw_mode_sprite.h"

typedef union PaletteColor {
    u16 value;
    u8 bytes[2];
} PaletteColor;

typedef struct CourseTriggerRecord {
    s16 geometryIndex;
    u8 value;
    u8 pad;
} CourseTriggerRecord;

typedef struct CourseSectorRange { s16 sector, count; } CourseSectorRange;

typedef struct TriangleColor { u8 r, g, b, code; } TriangleColor;
typedef struct GouraudTrianglePrimitive {
    u32 command;
    TriangleColor colors[3];
    s16 vertexIndices[3];
} GouraudTrianglePrimitive;
/* The GTE load at unk10 spans the three following vertex indices. */
typedef struct ModelSingleNormalGouraudTriangle {
    u32 command;
    TriangleColor colors[3];
    s16 unk10;
    s16 vertexIndices[3];
} ModelSingleNormalGouraudTriangle;
typedef struct GouraudQuadPrimitive {
    u32 command;
    TriangleColor colors[4];
    u16 vertexIndices[4];
} GouraudQuadPrimitive;
typedef struct TriangleScratchVertex {
    SVECTOR position;
    union {
        s32 unk08;
        struct { u8 u, v; u16 attribute; } uv;
    } texture;
    TriangleColor color;
} TriangleScratchVertex;

typedef struct UVPair { s8 u, v; } UVPair;
typedef struct TextureUV { UVPair coords; u16 attribute; } TextureUV;
typedef struct TexturedTrianglePrimitive {
    u32 command;
    TextureUV uv[3];
    u8 r, g, b, code;
    u16 vertexIndices[3];
} TexturedTrianglePrimitive;
/* The eight-byte GTE lighting load at unk14 also reads the vertex indices. */
typedef struct ModelLitTexturedTriangle {
    u32 command;
    TextureUV uv[3];
    u8 r, g, b, code;
    s16 unk14;
    s16 vertexIndices[3];
} ModelLitTexturedTriangle;

typedef struct TexturedQuadPrimitive {
    u32 command;
    TextureUV uv[4];
    u8 r, g, b, code;
    s16 vertexIndices[4];
} TexturedQuadPrimitive;
typedef struct TexturedGouraudTrianglePrimitive {
    u32 command;
    TextureUV uv[3];
    TriangleColor colors[3];
    u16 vertexIndices[3];
} TexturedGouraudTrianglePrimitive;
/* The eight-byte GTE lighting load at unk1C also reads the vertex indices. */
typedef struct ModelLitGouraudTexturedTriangle {
    u32 command;
    TextureUV uv[3];
    TriangleColor colors[3];
    s16 unk1C;
    u16 vertexIndices[3];
} ModelLitGouraudTexturedTriangle;
typedef struct TexturedGouraudQuadPrimitive {
    u32 command;
    TextureUV uv[4];
    TriangleColor colors[4];
    s16 vertexIndices[4];
} TexturedGouraudQuadPrimitive;
typedef struct TriangleTextureState {
    u32 reserved0;
    void (*callback)(TriangleScratchVertex *);
    u8 reserved[0x58];
    u16 tpage, clut;
    u8 r, g, b;
} TriangleTextureState;

typedef struct TexturedTriangleScratchpad {
    u32 reserved0;
    void (*callback)(TriangleScratchVertex *);
    s32 depth, depthCue;
    s32 flags, unk14, clipping;
    u8 reserved1C[0x34];
    struct { u16 x, y; } screen[3];
    u8 reserved5C[4];
    u16 tpage, clut;
    u8 r, g, b, code;
    u8 reserved68[3];
    u8 fogEnabled;
} TexturedTriangleScratchpad;

/* Sixteen world positions followed by per-position state words. */
typedef struct TrailPosition {
    s32 x, y, z;
} TrailPosition;
typedef struct PlayerTrailResource {
    TrailPosition positions[16];
    s32 state[16];
} PlayerTrailResource;

/* Two endpoints, height bounds, and rotation; each record occupies 0x28 bytes. */
typedef struct PlayerCollisionRegion {
    VECTOR endpoints[2];
    s16 minHeight, maxHeight;
    s16 pitch, yaw;
} PlayerCollisionRegion;
#define TRIANGLE_SCRATCHPAD ((TriangleScratchpad *)0x1F800000)
#define QUAD_SCRATCHPAD ((QuadScratchpad *)0x1F800000)

/* Indexed flat triangle in the current model primitive stream. */
typedef struct ModelFlatTriangle {
    u32 unk00;
    u8 r, g, b, code;
    s16 vertexIndices[3];
} ModelFlatTriangle;
/* The GTE lighting load reads the eight bytes beginning at unk08,
 * including the three vertex indices, as in the original primitive stream. */
typedef struct ModelLitFlatTriangle {
    u32 unk00;
    u8 r, g, b, code;
    s16 unk08;
    s16 vertexIndices[3];
} ModelLitFlatTriangle;
/* Each GTE lighting load reads eight bytes starting at unk00, including
 * the adjacent vertex index and following stream fields. */
typedef struct ModelLitGouraudTriangle {
    u32 unk00;
    u8 r, g, b, code;
    struct { s16 unk00, vertexIndex; } vertices[3];
} ModelLitGouraudTriangle;
/* Per-vertex colors and lighting inputs in the indexed primitive stream.
 * Each lighting load spans eight bytes starting at the corresponding unk00. */
typedef struct ModelColoredLitGouraudTriangle {
    u32 unk00;
    TriangleColor colors[3];
    struct { s16 unk00, vertexIndex; } vertices[3];
} ModelColoredLitGouraudTriangle;
/* Indexed flat quad in the current model primitive stream. */
typedef struct ModelFlatQuad {
    u32 unk00;
    u8 r, g, b, code;
    u16 vertexIndices[4];
} ModelFlatQuad;
typedef struct TriangleScratchpad {
    u32 unk00;
    void (*emitTriangle)(TriangleScratchVertex *);
    s32 depth, depthCue;
    s32 flags, unk14, clipping;
    u8 unk1C[0x34];
    struct { u16 x, y; } screen[3];
    u8 unk5C[8];
    u8 r, g, b, code;
    u8 unk68[2];
    u8 fogEnabled;
    u8 unk6B[5];
    TriangleScratchVertex vertices[3];
} TriangleScratchpad;

typedef struct LitTexturedTriangleScratchpad {
    u32 unk00;
    void (*emitTriangle)(TriangleScratchVertex *);
    s32 depth, depthCue;
    s32 flags, unk14, clipping;
    u8 unk1C[0x34];
    struct { u16 x, y; } screen[3];
    u8 unk5C[8];
    u8 r, g, b, code;
    u8 unk68[3];
    u8 fogEnabled;
    u8 unk6C[4];
    TriangleScratchVertex vertices[3];
} LitTexturedTriangleScratchpad;

/* Lighting loads span eight bytes from each unk00, including the adjacent
 * vertex index and following primitive-stream fields. */
typedef struct ModelLitTexturedGouraudTriangle {
    u32 command;
    TextureUV uv[3];
    u8 r, g, b, code;
    struct { s16 unk00, vertexIndex; } vertices[3];
} ModelLitTexturedGouraudTriangle;
typedef struct GouraudQuadScratchpad {
    u32 unk00;
    void (*emitQuad)(TriangleScratchVertex *);
    s32 depth, depthCue;
    u8 unk10[0x40];
    struct { u16 x, y; } screen[4];
    u8 unk60[4];
    u8 r, g, b, code;
    u8 unk68[2];
    u8 fogEnabled;
    u8 unk6B[5];
} GouraudQuadScratchpad;

typedef struct QuadScratchpad {
    u32 unk00;
    void (*emitQuad)(TriangleScratchVertex *);
    s32 depth, depthCue;
    u8 unk10[0x40];
    struct { u16 x, y; } screen[4];
    u8 unk60[4];
    u8 r, g, b, code;
    u8 unk68[2];
    u8 fogEnabled;
    u8 unk6B[5];
    TriangleScratchVertex vertices[4];
} QuadScratchpad;

typedef struct TexturedQuadScratchpad {
    u32 unk00;
    void (*emitQuad)(TriangleScratchVertex *);
    s32 depth, depthCue;
    u8 unk10[0x40];
    struct { u16 x, y; } screen[4];
    u16 tpage, clut;
    u8 r, g, b, code;
    u8 unk68[3];
    u8 fogEnabled;
    u8 unk6C[4];
} TexturedQuadScratchpad;

typedef struct IndexedTexturedQuadScratchpad {
    u32 unk00;
    void (*emitQuad)(TriangleScratchVertex *);
    s32 depth, depthCue;
    s32 flags, unk14, clipping;
    u8 unk1C[0x4B];
    u8 code;
    u8 unk68[3];
    u8 fogEnabled;
} IndexedTexturedQuadScratchpad;

typedef struct MovingLineResource {
    s16 x, y;
    u32 elapsed;
    u32 duration;
    u8 pad[0xF4];
} MovingLineResource;

typedef struct MovingLinePacket {
    u32 tag;
    u16 tpage, drawModeCommand;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad;
    s16 x1, y1;
} MovingLinePacket;

/* Interleaved screen positions and animation phase for 25 flickering dots. */
typedef struct FlickeringDotPosition { s16 x, y; } FlickeringDotPosition;
typedef struct FlickeringDotsResource {
    FlickeringDotPosition positions[25];
    s32 frame;
} FlickeringDotsResource;
/* Packed positions for the player-relative particle render task. */
typedef struct PlayerParticlePosition {
    s16 x, y, z;
} PlayerParticlePosition;

/* Positions and animation phase used by the falling-particle render task. */
typedef struct FallingParticleResource {
    s16 x[40];
    s16 y[40];
    s32 frame;
} FallingParticleResource;

typedef struct EightEntryEffectResource {
    VECTOR positions[8];
    SVECTOR rotations[8];
    s32 values[8];
    s32 elapsed;
} EightEntryEffectResource;
typedef struct EightEntryEffectSecondaryResource {
    SVECTOR vectors[8];
    s16 values40[8];
    s16 values50[8];
    u8 values60[8];
} EightEntryEffectSecondaryResource;

typedef struct FadingTextResource {
    s32 duration;
    s32 elapsed;
    s16 x, y;
    const char *text;
    u16 clut;
    u16 brightness;
    u8 pad[0xEC];
} FadingTextResource;

typedef struct FadingTextGlyphBanks {
    const s8 *entries[4];
} FadingTextGlyphBanks;

typedef struct QuadAnimationFrame {
    s32 duration;
    s32 scale;
    u8 r, g, b, blend;
    u16 tpage, clut;
    u8 u, v, w, h;
} QuadAnimationFrame;

typedef struct PrimaryQuadAnimationResource {
    s32 x, y, z;
    s32 unk0C;
    s32 elapsed;
    s32 frame;
    QuadAnimationFrame *frames;
    u8 pad[0xE4];
} PrimaryQuadAnimationResource;

typedef struct SemiTransparentGouraudTrianglePacket {
    u32 tag;
    u16 tpage, drawModeCommand;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} SemiTransparentGouraudTrianglePacket;

#define GOURAUD_LINE_CODE 0x50
#define FLAT_RECTANGLE_CODE 0x60
#define TEXTURED_QUAD_CODE 0x2C
#define TEXTURED_SPRITE_CODE 0x64
#define TEXTURED_QUAD_PACKET_WORDS 9
#define DRAW_MODE_COMMAND_HIGH 0xE100
#define SEMI_TRANSPARENT_FLAG 2

/* Draw mode and a flat rectangle share a single DMA packet tag. */
typedef struct FlatRectanglePacket {
    u32 tag;
    u16 tpage, drawModeCommand;
    u8 r0, g0, b0, code;
    s16 x0, y0, w, h;
} FlatRectanglePacket;

/* Draw mode and a flat quad share a single DMA packet tag. */
typedef struct FlatQuadPacket {
    u32 tag;
    u16 tpage, drawModeCommand;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2, x3, y3;
} FlatQuadPacket;

/* Draw mode and a Gouraud quad share a single DMA packet tag. */
typedef struct GouraudQuadPacket {
    u32 tag;
    u16 tpage, drawModeCommand;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} GouraudQuadPacket;

typedef struct VisualPlacement {
    u8 flags, unk01;
    s16 geometryIndex;
    s32 x, y, z;
} VisualPlacement;

struct VisualPlacementHeightData {
    s16 prefix;
    s16 offsets[16];
    s16 suffix;
};

/* Collectible placements use 0x10-byte records. */
typedef struct PickupPlacement {
    u16 unk00;
    s16 geometryIndex;
    s32 x, y, z;
} PickupPlacement;
typedef struct MovingVisualPlacement {
    s16 geometryIndex, pad;
    s32 startX, startZ, endX, endZ;
} MovingVisualPlacement;

typedef struct PickupResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    Position3D position;
    u8 unkAC[0x14];
    SVECTOR rotation;
    u8 unkC8[8];
    s16 geometryIndex;
    u8 unkD2[6];
    s32 collisionRange;
} PickupResource;

typedef union HeadingValue {
    s32 value;
    struct { s16 low, high; } halves;
} HeadingValue;
typedef struct BobbingTrailEmitterResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    Position3D position;
    u8 unkAC[0x14];
    SVECTOR rotation;
    u8 unkC8[8];
    s16 geometryIndex;
    u8 unkD2[6];
    s32 countdown;
    HeadingValue heading;
    s32 phase;
    HeadingValue baseHeading;
    s32 headingStep;
} BobbingTrailEmitterResource;
typedef struct EmitterTrailResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x74];
    s16 geometryIndex;
    u8 unkC6[0x12];
    s32 angle;
} EmitterTrailResource;
typedef struct PlacedObjectResource {
    GsCOORDINATE2 coordinate, groundCoordinate;
    Position3D position;
    u8 unkAC[0x14];
    SVECTOR rotation, groundRotation;
    s16 geometryIndex;
} PlacedObjectResource;

typedef struct OrbitingTaskResource {
    GsCOORDINATE2 coordinate, groundCoordinate;
    Position3D position;
    u8 unkAC[0x14];
    SVECTOR rotation, groundRotation;
    s16 geometryIndex, phase;
    u8 unkD4[4];
    s32 playerCooldowns[4];
} OrbitingTaskResource;
typedef struct OrbitingTaskPlacement {
    s16 geometryIndex, radius;
    s32 x, z;
} OrbitingTaskPlacement;

typedef struct FallingTaskResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    Position3D position;
    u8 unkAC[0x14];
    SVECTOR rotation;
    u8 unkC8[8];
    s16 geometryIndex, groundPitch, groundRoll, unkD6;
    s32 verticalVelocity, groundHeight;
} FallingTaskResource;
/* Four edge neighbors and corner indices in a 0x1C-byte region record. */
typedef struct GeometryRegion {
    s16 neighbors[4];
    s16 vertices[4];
    s16 unk10[6];
} GeometryRegion;

typedef struct TextureRegion {
    u8 u, v, w, h;
} TextureRegion;

typedef struct TextureRegionSet {
    s16 x, y;
    TextureRegion *regions;
    s16 depth;
    s16 clutX, clutY;
    s16 unk0E;
} TextureRegionSet;

typedef struct SemiTransparentTrianglePacket {
    u32 tag;
    u16 tpage, drawModeCode;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2;
} SemiTransparentTrianglePacket;

/* Two sprites cover the 320 by 240 screen; brightness is read from its low byte. */
typedef struct ScreenSpriteResource {
    DrawModeSpritePacket sprites[2];
    s32 elapsed;
    s32 duration;
    const s32 *brightness;
    u8 pad[0xC4];
} ScreenSpriteResource;

/* Callback and task data restored alongside a saved player state. */
typedef struct TaskSnapshot {
    TaskCallback callback;
    u16 data18, data1A, data1C, data1E;
} TaskSnapshot;

typedef struct SelectedStateRecord {
    s16 state;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    s32 unk10;
} SelectedStateRecord;
typedef struct PlayerPositionTrailResource {
    s32 positions[3][3];
    s32 unk24[3][3];
    u8 unk48[6];
    s16 unk4E;
    u8 pad50[0xB0];
} PlayerPositionTrailResource;

typedef struct PlayerRingEffectResource {
    s32 position[3];
    s32 unk0C;
    s32 elapsed;
    SVECTOR rotations[6];
} PlayerRingEffectResource;

typedef struct PlayerTripleQuadEffectResource {
    TrailPosition positions[3];
    SVECTOR rotation;
    s32 elapsed, scale;
    u16 state;
} PlayerTripleQuadEffectResource;

typedef struct RotatingQuadTrailResource {
    s32 countdown;
    s16 angles[8];
    s16 scales[8];
    u8 pad[0xDC];
} RotatingQuadTrailResource;

typedef struct QuadScaleScratchpad {
    MATRIX matrix;
    SVECTOR rotation;
    VECTOR scale;
    SVECTOR vertices[4];
} QuadScaleScratchpad;

typedef struct QuadTransitionResource {
    s32 frames;
    SVECTOR *vertices;
    POLY_FT4 *packet;
    u8 pad[0xF4];
} QuadTransitionResource;

/* Coordinate, position, and color for a quad with an oscillating Z rotation. */
typedef struct OscillatingQuadResource {
    GsCOORDINATE2 coordinate;
    SVECTOR rotation;
    s32 x, y, z;
    s32 elapsed;
    s16 unk68;
    s16 pad6A;
    s32 amplitude;
    u8 r, g, b;
    u8 pad73[0x8D];
} OscillatingQuadResource;

/* World position and geometry placement index for each proximity emitter. */
typedef struct EmitterPlacement {
    s32 x, y, z;
    s16 geometryIndex, pad;
} EmitterPlacement;

/* A coordinate and emission countdown in one 256-byte task resource. */
typedef struct ProximityEmitterResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x70];
    s16 unkC0, angle, unkC4;
    u8 unkC6[0xA];
    s16 geometryIndex;
    u8 unkD2[6];
    s32 countdown;
    u8 unkDC[0x24];
} ProximityEmitterResource;

/* Fixed-point motion for a particle spawned by a proximity emitter. */
typedef struct EmittedParticleResource {
    GsCOORDINATE2 coordinate;
    u8 unk50[0x50];
    s32 positionX, positionY, positionZ;
    s32 unkAC;
    s32 velocityX, velocityY, velocityZ, gravity;
    s16 rotation, angle, unkC4;
    u8 unkC6[0xA];
    s16 geometryIndex;
    u8 unkD2[0x2E];
} EmittedParticleResource;

/* One 256-byte task resource; the remaining fields belong to the effect callbacks. */
typedef struct PositionedEffectResource {
    s32 unk00;
    s32 x, y, z;
    u8 pad[0xF0];
} PositionedEffectResource;

typedef struct RotatingEffectResource {
    s32 elapsed;
    s32 x, y, z;
    s32 unk10;
    SVECTOR rotation;
    SVECTOR parentRotation;
    s16 clut;
    s16 textureU;
} RotatingEffectResource;

typedef struct QuadVertex {
    s16 x, y, z, pad;
} QuadVertex;

typedef struct ShrinkingQuadResource {
    s32 countdown;
    const QuadVertex *vertices;
    const POLY_FT4 *packet;
    u8 pad0C[0xF4];
} ShrinkingQuadResource;

typedef struct PaletteStripResource {
    s32 frame;
    s32 unk04;
    const POLY_FT4 *packet;
    s32 textureV;
    u8 pad10[0xF0];
} PaletteStripResource;

typedef struct SlidingQuadResource {
    s32 countdown;
    s32 unk04;
    const POLY_FT4 *packet;
    s32 textureV;
    u8 pad10[0xF0];
} SlidingQuadResource;

typedef struct RadialEffectResource {
    GsCOORDINATE2 coordinate;
    SVECTOR positions[6];
    SVECTOR innerPositions[6];
    SVECTOR velocities[6];
    s32 elapsed;
} RadialEffectResource;

typedef struct QuadAnimationStep {
    s32 duration, scale;
    u8 r, g, b, flags;
    u16 tpage, clut;
    u8 u, v, w, h;
} QuadAnimationStep;

typedef struct QuadAnimationResource {
    PositionEffectOrigin position;
    s32 elapsed, frame;
    const QuadAnimationStep *steps;
} QuadAnimationResource;

typedef struct TimedSpriteStep {
    s32 frame;
    s32 duration;
    s16 x, y;
    const char *text;
    s16 palette;
    s16 pad12;
} TimedSpriteStep;

typedef struct TimedSpriteSequenceResource {
    s32 frame;
    s32 duration;
    const TimedSpriteStep *step;
    u8 pad0C[0xF4];
} TimedSpriteSequenceResource;

extern const s8 D_8012EC04[], D_8012ECC8[], D_8012EECC[], D_8012F0D0[];

extern const FadingTextGlyphBanks gFadingTextGlyphBanks
    __attribute__((section(".rodata.fading_text_glyph_banks")));

/* Interleaved texture U coordinate and advance width for A through Z. */
extern u8 gUppercaseGlyphMetrics[52] __attribute__((section(".data.uppercase_glyph_metrics")));

/* Sector ranges for the selected course and its one-sector metadata block. */
extern CourseSectorRange gCourseResourceSectors[10][3] __attribute__((section(".data.course_sector_resources")));

extern CourseSectorRange gCourseMetadataSectors[10] __attribute__((section(".data.course_sector_resources")));

extern CourseSectorRange gCourseOptionalResourceSectors[10] __attribute__((section(".data.course_sector_resources")));

extern u8 gCourseMetadataBuffer[0x800] __attribute__((section(".data.course_sector_resources"), aligned(4)));

/* Screen copies selected by the saved camera state. */
extern TexturedSpritePacket gCameraOverlaySpritesBuffer0[2] __attribute__((section(".data.camera_overlay_packets")));
extern TexturedSpritePacket gCameraOverlaySpritesBuffer1[2] __attribute__((section(".data.camera_overlay_packets")));
extern POLY_FT4 gCameraOverlayQuadsBuffer0[2] __attribute__((section(".data.camera_overlay_packets")));
extern POLY_FT4 gCameraOverlayQuadsBuffer1[2] __attribute__((section(".data.camera_overlay_packets")));

/* Semi-transparent screen copies displaced one pixel horizontally. */
extern TexturedSpritePacket gScreenCopyPlusBuffer1[2] __attribute__((section(".data.offset_screen_copy_sprites")));
extern TexturedSpritePacket gScreenCopyPlusBuffer0[2] __attribute__((section(".data.offset_screen_copy_sprites")));
extern TexturedSpritePacket gScreenCopyMinusBuffer1[2] __attribute__((section(".data.offset_screen_copy_sprites")));
extern TexturedSpritePacket gScreenCopyMinusBuffer0[2] __attribute__((section(".data.offset_screen_copy_sprites")));

extern struct VisualPlacementHeightData gVisualPlacementHeightData __attribute__((section(".data.visual_placement_heights")));

extern SVECTOR *D_8013B688;
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern MATRIX D_8012A6B0, D_8012A6D0;
extern MATRIX *D_8013AF9C __attribute__((section(".data")));
extern POLY_FT4 D_8012967C;
extern s32 D_8013B300 __attribute__((section(".data")));
typedef struct PairedEffectResource {
    u8 unk00[0x20];
    VECTOR positions[2];
    s32 elapsed;
    u16 clut;
} PairedEffectResource;

typedef struct PlayerSnapshotTrailResource {
    GsCOORDINATE2 current, previous;
    u8 unkA0[0x30];
    VECTOR currentScale, previousScale;
    s16 remainingFrames;
} PlayerSnapshotTrailResource;

/* Current and previous state used by player rendering effects. */
typedef struct PlayerRenderSnapshot {
    VECTOR position;
    u16 unk10, unk12, unk14, unk16, unk18, unk1A;
} PlayerRenderSnapshot;

extern EmitterPlacement gProximityEmitterPlacements[2]
    __attribute__((section(".data.proximity_emitter_placements")));

extern MovingVisualPlacement gMovingVisualPlacements[9]
    __attribute__((section(".data.moving_visual_placements")));

extern OrbitingTaskPlacement gOrbitingTaskPlacements[2]
    __attribute__((section(".data.orbiting_task_placements")));

extern u32 gObjectSortSavedStackPointer
    __attribute__((section(".sdata.object_sort_saved_stack")));
extern u32 gAlternateObjectSortSavedStackPointer
    __attribute__((section(".sdata.alternate_object_sort_saved_stack")));

extern ControllerEffectState gControllerEffectStates[2];

extern PlayerRenderSnapshot gPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots")));
extern PlayerRenderSnapshot gPreviousPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots")));
extern MATRIX gPlayerRenderIdentityMatrix
    __attribute__((section(".data.player_render_snapshots")));

extern VECTOR gPlayerPairedQuadOffsets[2]
    __attribute__((section(".data.player_paired_quad_offsets")));

extern SVECTOR gPlayerTripleQuadOffsets[3]
    __attribute__((section(".data.player_triple_quad_offsets")));

extern POLY_FT4 gQuadShrinkPacket __attribute__((section(".data.quad_shrink")));
extern SVECTOR gQuadShrinkVertices[4] __attribute__((section(".data.quad_shrink")));

extern SVECTOR gRightQuadShrinkVertices[4] __attribute__((section(".data.right_quad_vertices")));
extern POLY_FT4 gRightQuadShrinkPacket __attribute__((section(".data.right_quad_packet")));

extern QuadAnimationFrame gQuadAnimationFrames[5] __attribute__((section(".data.quad_animation")));

extern u16 gTextClut;

extern QuadAnimationStep gAlternateQuadAnimationSteps[5]
    __attribute__((section(".data.alternate_quad_steps")));

extern QuadVertex gMiddleShrinkQuadVertices[4]
    __attribute__((section(".data.middle_shrink_vertices")));

extern POLY_FT4 gMiddleShrinkQuadPacket
    __attribute__((section(".data.middle_shrink_packet")));

extern POLY_FT4 gSlidingQuadPacket
    __attribute__((section(".data.sliding_quad_packet")));

extern char gDebugPositionXFormat[8] __attribute__((section(".data.debug_position_formats")));
extern char gDebugPositionYFormat[8] __attribute__((section(".data.debug_position_formats")));
extern char gDebugPositionZFormat[8] __attribute__((section(".data.debug_position_formats")));

/* Incomplete declarations retain the original absolute address loads. */
extern char debugPositionXFormatAbsolute[];
extern char debugPositionYFormatAbsolute[];
extern char debugPositionZFormatAbsolute[];

extern GameTask *D_8013B304 __attribute__((section(".data")));
extern RECT gReadPixelRect __attribute__((section(".data.read_pixel_rect")));
extern RECT gScanlineCopyRect __attribute__((section(".data.scanline_copy_rects")));
extern RECT gScanlineSeedRect __attribute__((section(".data.scanline_copy_rects")));

/* Coordinate updates use GP-relative accesses. */
extern RECT scanlineCopyRectGp;

/* The original stores coordinates through GP but passes an absolute address. */
extern RECT readPixelRectGp;

/* Shared collision regions remain in player_camera_state_tail. */
extern PlayerCollisionRegion D_80129A18[2];
extern u8 D_8013B2B5, D_8013B2B6;
extern VisualPlacement *D_8013B2BC;
extern u8 D_8013B2B9, D_8013B2BA;
/* The course lookup base precedes its valid pointer entries in shared frame data. */
extern TaskVisualFrame *D_80129540[];
void func_8005BF00(void);
void func_8005E728(void);
void func_8005B038(void);
extern PickupPlacement *D_8013B2C0;

/* Shared texture metadata remains in its original data owner. */
extern TextureRegionSet D_8012DDA4[];
extern TextureRegion *D_8013B6AC;

extern TaskSnapshot *D_8013B2E4;
extern RacePlayer *D_8013B2E8;
extern SelectedStateRecord *D_8012C6F4[];
extern TaskVisualFrame gPlacedObjectVisualFrame
    __attribute__((section(".data.placed_object_visual_frame")));

extern TaskVisualFrame *gCourseVisualFrames
    __attribute__((section(".sdata.course_visual_frames")));

extern CourseTriggerRecord gCourse0Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse1Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse2Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse3Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse4Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse5Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse6Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse8Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gCourse9Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4)));
extern CourseTriggerRecord gEmptyCourseTriggers[]
    __attribute__((section(".data.empty_course_triggers"), aligned(4)));
extern CourseTriggerRecord *gCourseTriggers[]
    __attribute__((section(".data.course_triggers")));

extern s8 *D_8012BBB4[][4];
extern PaletteColor gCapturedColorCyclePalette[16]
    __attribute__((section(".data.color_cycle_palette"), aligned(4)));
extern u16 gWhitePulsePalette[16]
    __attribute__((section(".bss.white_pulse_palette"), aligned(4)));
extern RECT gWhitePulsePaletteRect
    __attribute__((section(".data.color_cycle_rects")));
extern RECT gColorCycleCaptureRect
    __attribute__((section(".data.color_cycle_rects")));
extern s32 D_8013B338;
extern s32 D_8013B35C;
extern s32 D_8013B360;
extern s32 D_8013B364;
extern s32 D_8013B368;
extern s32 D_8013B36C;
extern ModelFlatTriangle *D_8013B680 __attribute__((section(".sdata")));
extern SVECTOR *D_8013B688 __attribute__((section(".sdata")));

extern GsOT *D_8013B670;
extern s32 D_8013B69C;
extern s32 D_8013B660;

extern u16 D_8013B376 __attribute__((section(".sdata")));

void updateProximityEmitterTask(void);
void func_8005D394(void);
void updateFallingTaskObject(void);

void emitProjectedGouraudTriangle(TriangleScratchVertex *);
void emitProjectedTexturedTriangle(TriangleScratchVertex *);
void emitProjectedGouraudQuad(TriangleScratchVertex *);
void emitProjectedTexturedQuad(TriangleScratchVertex *);
void emitProjectedGouraudTexturedQuad(TriangleScratchVertex *);
void emitProjectedGouraudTexturedTriangle(TriangleScratchVertex *);
void func_80053A34(TriangleScratchVertex *, s32);
void func_8006479C(void);
void func_800654B4(void);
void func_80066474(void);
void func_800689EC(void);
void func_8005D828(RacePlayer *player, PlayerCollisionRegion *region);
void func_8005B460(void);
void func_8005B6C0(void);
void updateCourseVisualTask(void);
void emitProjectedFlatTriangle(TriangleScratchVertex *vertices);
void emitProjectedFlatQuad(TriangleScratchVertex *vertices);
void func_800542BC(TriangleScratchVertex *vertices, s32 flags);
void drawMovingLineTask(void);
void func_8004F780(void);

void func_80068D1C(void);
void drawShrinkingTexturedQuadTask(void);
void drawLoweringQuadStripTask(void);
void drawSlidingQuadTask(void);
void func_80069E08(s32 duration);
void drawFallingParticlesTask(void);
void func_800632C4(void);
void drawPlayerSnapshotTrailTask(void);
void func_800618A0(void);
void func_80061E48(void);
void func_80062354(void);
void func_80065EC8(void);
void emitRadialEffectTask(void);
void func_80066D18(void);
void func_80067DD0(void);
void func_800659F0(void);
extern SVECTOR D_8012A5C0[];
extern GsCOORDINATE2 D_80129C60[];

extern s32 D_8013B624 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B628 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B62C __attribute__((section(".sdata.geometry_edge_projection")));

extern s32 D_8013B630 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B634 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B638 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B63C __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B640 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B644 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B648 __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B64C __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 D_8013B650 __attribute__((section(".sdata.geometry_edge_projection")));

extern const char gNoProcessMessage[16];

void func_8004EFE0(void);
void updatePlayerFollowingOrbitTask(void);
void updateOrbitingTaskObject(void);
void SetFarColor(s32 r, s32 g, s32 b);
void SetFogNearFar(s32 near, s32 far, s32 h);
void drawTaskObject(u16 index);
void updateMovingTaskObject(void);
void func_80049D48(s32 x, s32 y, const char *format, ...);
void func_8006B4A0(s32 x, s32 y, const char *format, ...);
void func_8006BEEC(s32 enabled);
void func_8006C970(s32 enabled);
void func_80063B58(void);
void func_80063DF4(void);
void drawFullScreenSpriteTask(void);
void updateFadingTextTask(void);
void drawFadingText(FadingTextResource *resource);
void loadTimedSpritePresentationResources(void);
void func_8006AB8C(void);
void updatePrimaryValueTransition(void);
void updatePaletteWhitePulse(void);
void updateTimedSpriteSequence(void);
void drawTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 borderW, s16 borderH,
                   s16 style, s16 clutOffset, s16 blendMode, GsOT *ot, u16 depth);
void updateValueInterpolationTask(void);
void func_80064754(s32 playerIndex);
s32 func_80065464(s32 playerIndex);
void createPlayerTrailTask(s32 playerIndex);
void func_80067D88(s32 playerIndex);
void func_800688C0(s32 playerIndex);

s32 isPointWithinCameraForwardRange(s32 x, s32 z, s16 player);
void initializeProximityParticleTask(void);
void func_8005CEF0(void);
void updateCollectibleTask(void);
void updateLandedTaskObject(void);

void setTextPalette(s32 palette);
void initSprite(GsSPRITE *sprite);
void bindGeometryRecordData(s16 *data);
s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z);
void selectGeometryRecord(s16 index);
s32 getGeometryPlacementSignedDistance(s16 index, s32 x, s32 z);
void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle);
void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit);
void enforceGeometryEdgeDistance(s32 limit);
void adjustDebugPositionAndDisplay(VECTOR *position);
void drawCyanBackgroundAndSetFog(void);
void createOscillatingQuadTask(s32 value, VECTOR *position, s32 amplitude, s32 r, s32 g, s32 b);
void createMovingLineTask(s32 index);
void createFlickeringDotsTask(s32 playerIndex);
void drawSnowBackgroundAndSetFog(s16 playerIndex);
s32 isPositionInCameraRange(s32 x, s32 z, s16 playerIndex);
void readVramPixelColor(s32 x, s32 y, TextureColor *out);
void unpackPsxColor(u32 color, UnpackedPsxColor *out);
u32 packTextureColor(TextureColor *color);
void queueScanlineCopyPackets(void);
void queueSelectedCameraOverlayPackets(void);
void queueOffsetScreenCopySprites(void);
void drawNoProcessMessage(void);
void emitProjectedGouraudTriangle(TriangleScratchVertex *vertices);
void emitProjectedTexturedTriangle(TriangleScratchVertex *vertices);
void emitProjectedGouraudTexturedTriangle(TriangleScratchVertex *vertices);
void emitProjectedGouraudQuad(TriangleScratchVertex *vertices);
void emitProjectedTexturedQuad(TriangleScratchVertex *vertices);
void emitProjectedGouraudTexturedQuad(TriangleScratchVertex *vertices);
void drawIndexedFlatTriangle(void);
void processGouraudTrianglePrimitive(void);
void processTexturedTrianglePrimitive(void);
void processTexturedGouraudTrianglePrimitive(void);
void drawIndexedFlatQuad(void);
void processGouraudQuadPrimitive(void);
void processTexturedQuadPrimitive(void);
void processTexturedGouraudQuadPrimitive(void);
void drawProjectedIndexedFlatTriangle(void);
void drawProjectedIndexedLitFlatTriangle(void);
void drawProjectedIndexedLitGouraudTriangle(void);
void drawProjectedIndexedGouraudTriangle(void);
void drawProjectedIndexedSingleNormalGouraudTriangle(void);
void drawProjectedIndexedColoredLitGouraudTriangle(void);
void drawProjectedIndexedTexturedTriangle(void);
void drawProjectedIndexedLitTexturedTriangle(void);
void drawProjectedIndexedLitTexturedGouraudTriangle(void);
void drawProjectedIndexedGouraudTexturedTriangle(void);
void drawProjectedIndexedLitGouraudTexturedTriangle(void);
void drawProjectedIndexedFlatQuad(void);
void drawProjectedIndexedGouraudQuad(void);
void drawProjectedIndexedTexturedQuad(void);
void drawProjectedIndexedTexturedGouraudQuad(void);
void queueCourseResourceSectorReads(void);
void drawTaskObjectAlternateLayer(u16 index);
void initializeSelectedCourseOffsetTask(void);
void updateSelectedCourseOffsetTask(void);
void computeSelectedTriangleAngles(s16 *pitch, s16 *yaw);
void updateCameraRotationMatrices(u16 playerIndex);
void prepareTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frame, u8 usePitch);
void setTaskVisualFrame(TaskVisualResource *resource, TaskVisualFrame *frames, s32 scale);
void initializePickupVisualTask(void);
void initializeObjectVisualTask(void);
void initializeInteractionVisualTask(void);
void initializeCourseVisualTask(void);
void initializePlacedObjectTask(void);
void updateBobbingTrailEmitterTask(void);
void initializeProximityEmitterTask(void);
void initializeMovingVisualTask(void);
void updateActivePlayerRegionCollisions(void);
void initializeCollectibleTask(void);
void initializeFallingTaskObject(void);
void initializeOrbitingTaskObject(void);
void updateAndDrawCurrentTaskObject(void);
void copySlot26TransformToCurrentTaskObject(void);
void updateCurrentPlayerCourseTrigger(void);
void updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex);
void resetSelectedRecordStates(void);
void captureSelectedPlayerStates(void);
s32 restoreSelectedPlayerState(void);
void addCopiedPrimitive(u32 *orderingTable, u32 *packet);
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
                   s32 x1, s32 y1, s32 x2, s32 y2, s32 r, s32 g, s32 b);
void addSemiTransparentGouraudTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                  s32 x1, s32 y1, s32 x2, s32 y2,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2);
void addFlatQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 x2, s32 y2, s32 x3, s32 y3, s32 r, s32 g, s32 b);
void addGouraudQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3);
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
void addSemiTransparentFlatQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                                          s32 x0, s32 y0, s32 x1, s32 y1,
                                          s32 x2, s32 y2, s32 x3, s32 y3,
                                          s32 r, s32 g, s32 b);
void addSemiTransparentGouraudQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                  s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3);
void addFlatLine(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 r, s32 g, s32 b);
void addFlatRectangle(u32 *orderingTable, s32 x, s32 y, s32 w, s32 h,
                   s32 r, s32 g, s32 b);
void addFlatRectangleWithDrawMode(u32 *orderingTable, s32 tpage, s32 flags,
                  s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b);
void addTexturedRectangle(u32 *orderingTable, s32 clut, s32 x, s32 y,
                   s32 w, s32 h, s32 r, s32 g, s32 b);
void addTexturedSprite(u32 *orderingTable, s32 tpage, s32 clut, s32 x,
                   s32 y, s32 w, s32 h, u16 u, u16 v,
                   s32 r, s32 g, s32 b);
void unusedVariadicStub(s32 unused0, s32 unused1, s32 unused2, ...);
s32 loadTimArchive(OffsetArchive *archive);
s32 loadTimImage(u32 *tim);
s32 randomNextLinear(void);
s32 randomLinearFromValue(s32 value);
void updatePlayerRenderSnapshots(void);
void startRacePlayerTasks(void);
void initializePlayerSnapshotTrailTask(void);
void startIndexedRenderTask(s32 index);
void startIndexedRenderTaskPair(s32 index);
void createFallingParticleTask(void);
void createPlayerParticleTask(s32 playerIndex);
void createPositionedEffectTask(s32 x, s32 y, s32 z);
void startRotatingQuadEffectTask(s32 variant, const VECTOR *position, s32 pitch, s32 yaw, s32 roll);
void initializeDelayedEightEntryEffectTask(void);
void createPlayerPairedQuadEffectTask(s32 playerIndex, s32 variant);
GameTask *createQuadAnimationTask(u16 unusedId, VECTOR *position);
GameTask *startAlternateQuadAnimationTask(s32 unused, const PositionEffectOrigin *position);
void startPositionEffectTask(const PositionEffectOrigin *position);
void createPlayerPositionTrailTask(s32 playerIndex);
void createPlayerRingEffectTask(s32 playerIndex);
s32 createPlayerTripleQuadEffectTask(s32 playerIndex);
void drawRisingTexturedStripTask(void);
void createRotatingQuadTrailTask(void);
void drawLoweringTexturedQuadTask(void);
void createQuadShrinkTask(void);
void startMiddleQuadShrinkTask(void);
void createRightQuadShrinkTask(void);
void drawPaletteCyclingStripTask(void);
void startSlidingQuadTask(s32 index);
void capturePaletteForColorCycle(void);
void updateQuadTransitionSequenceTask(void);
void startPrimaryValueTransition(s32 duration, s32 start, s32 end);
void startValueInterpolationTask(s32 duration, s32 start, s32 end);
void createFullScreenSpriteTask(s32 duration, s32 tpage, u16 clut, u16 u, u16 v,
                  const s32 *brightness);
void *getOffsetArchiveEntry(OffsetArchive *archive, s32 index);
void uploadTimImageAndPaletteAt(u32 *tim, s32 x, s32 y, s32 clutX, s32 clutY);
void createFadingTextTask(s32 duration, s32 x, s32 y, const char *text, s32 palette);
void finishTimedSpriteSequence(void);
void startTimedSpriteSequence(const TimedSpriteStep *steps);
void finishTimedSpritePresentation(void);
void initializeTimedSpritePresentation(void);
void createResourceLoadTask(void);
void addTextCharacter(s32 x, s32 y, u32 character);
void addStretchedTextCharacter(s32 x, s32 y, s32 width, u32 character);
void resetControllerEffects(void);
void updateLegacyControllerActuators(u16 controller);
void initPrimitiveTemplates(void);
void initTexturedQuadTemplate(void);
void setTexturedQuadTexture(s32 index, s32 palette);
void setTexturedQuadColor(s32 r, s32 g, s32 b);
void drawTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void drawStretchedTexturedQuad(s16 x, s16 y, s16 width, s16 height, s32 index, GsOT *ot, u16 depth);
void drawTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void drawTexturedQuadByIndexWithBlend(s16 x, s16 y, s32 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth);
void drawStretchedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth);
void drawTexturedQuadList(s16 x, s16 y, TexturedQuadListEntry *entry, s16 blend, GsOT *ot, u16 depth);
void drawSkewedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 unused, s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth);
void drawInclusiveTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void drawInclusiveTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void initSpritePrimitiveTemplate(void);
void setSpriteTextureBlend(s32 index, s32 palette, s32 blend);
void setSpriteColor(s32 r, s32 g, s32 b);
void drawSpriteTextureRegion(s16 x, s16 y, s32 index, GsOT *ot, u16 depth);
void addSpritePacket(SPRT *source, GsOT *ot, u16 depth);
void addDrawPagePacket(DR_TPAGE *source, GsOT *ot, u16 depth);
void drawRotatedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex, s16 blendMode, s16 angle, GsOT *ot, u16 depth);
void drawTransformedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex, s16 blendMode, s16 pivotX, s16 pivotY, s16 scaleX, s16 scaleY, s32 rotation, GsOT *ot, u16 depth);
void drawWrappedText8x8(s16 x, s16 y, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawLimitedText8x8(s16 x, s16 y, s16 limit, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawSizedTextGlyph(s16 x, s16 y, s16 width, s16 height, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawHalfStepSpriteBar(s16 x, s16 y, s16 count, s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawPackedTime(s16 x, s16 y, u32 value, s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawWrappedDoubleByteText(s16 x, s16 y, s16 columns, s8 *text, s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawUppercaseProportionalText(s16 x, s16 y, s16 count, const char *text, s16 palette,
                   s16 blend, GsOT *ot, u16 depth);
void drawWrappedText16x16(s16 x, s16 y, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);
void drawLimitedWrappedText16x16(s16 x, s16 y, s16 limit, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth);
void setTexturedQuadTextureBlend(s32 index, s32 palette, s32 blend);
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth);
void drawTexturedFrameStyle9(s16 x, s16 y, s16 w, s16 h, s16 palette,
                             s16 blend, GsOT *ot, u16 depth);
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth);
