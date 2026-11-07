#include "common.h"
#include "game/angles.h"
#define CONTROLLER_INPUT_ABSOLUTE_ADDRESSING
#include "game/controller.h"
#include "game/cd_queue.h"
#define CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
#include "game/controller_effects.h"
#include "game/controller.h"
#define GS_PACKET_ABSOLUTE_ADDRESSING
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/game_options.h"
#include "game/game_task_scheduler.h"
#define TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/race_player.h"
#include "game/race_camera.h"
#include "game/selected_records.h"
#define SELECTED_RECORD_GP_ADDRESSING
#include "game/selected_record.h"
#include "psx/libsnd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "game/quad_animation.h"
#include "game/coordinate.h"
#include "game/course_offset.h"
#include "psx/libapi.h"

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
typedef struct ModelLitGouraudTriangle {
    u32 command;
    TriangleColor colors[3];
    s16 unk10;
    s16 vertexIndices[3];
} ModelLitGouraudTriangle;
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

typedef struct DrawModeSpritePacket {
    u32 tag;
    u16 tpage, drawModeCode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} DrawModeSpritePacket;

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

const FadingTextGlyphBanks gFadingTextGlyphBanks
    __attribute__((section(".rodata.fading_text_glyph_banks"))) = {
    {D_8012EC04, D_8012ECC8, D_8012EECC, D_8012F0D0}
};

/* Interleaved texture U coordinate and advance width for A through Z. */
u8 gUppercaseGlyphMetrics[52] __attribute__((section(".data.uppercase_glyph_metrics"))) = {
    0xC0, 0x08, 0xC8, 0x08, 0xD0, 0x08, 0xD8, 0x08,
    0xE0, 0x07, 0xE8, 0x07, 0xF0, 0x08, 0xC0, 0x08,
    0xC8, 0x04, 0xCC, 0x06, 0xD4, 0x08, 0xDC, 0x07,
    0xE4, 0x0A, 0xF0, 0x08, 0x80, 0x08, 0x88, 0x08,
    0x90, 0x09, 0x9C, 0x08, 0xA4, 0x07, 0xAC, 0x08,
    0xB4, 0x08, 0xBC, 0x09, 0xC8, 0x0A, 0xD4, 0x09,
    0xE0, 0x0A, 0xEC, 0x08,
};

/* Sector ranges for the selected course and its one-sector metadata block. */
CourseSectorRange gCourseResourceSectors[10][3] __attribute__((section(".data.course_sector_resources"))) = {
    {{0x0C9D, 0x000F}, {0x0CAC, 0x003E}, {0x0CEA, 0x005A}},
    {{0x0D48, 0x0010}, {0x0D58, 0x004E}, {0x0DA6, 0x0060}},
    {{0x0E09, 0x000B}, {0x0E14, 0x0037}, {0x0E4B, 0x0060}},
    {{0x0EAC, 0x0011}, {0x0EBD, 0x0055}, {0x0F12, 0x0060}},
    {{0x0F73, 0x000C}, {0x0F7F, 0x0039}, {0x0FB8, 0x0060}},
    {{0x101C, 0x0011}, {0x102D, 0x0055}, {0x1082, 0x0060}},
    {{0x10E3, 0x000C}, {0x10EF, 0x003F}, {0x112E, 0x0062}},
    {{0x11A4, 0x0005}, {0x11A9, 0x0018}, {0x11C1, 0x003C}},
    {{0x11FE, 0x0004}, {0x1202, 0x0017}, {0x1219, 0x0046}},
    {{0x1265, 0x0005}, {0x126A, 0x0019}, {0x1283, 0x005E}}
};

CourseSectorRange gCourseMetadataSectors[10] __attribute__((section(".data.course_sector_resources"))) = {
    {0x0D44, 0x0001},
    {0x0E06, 0x0001},
    {0x0EAB, 0x0001},
    {0x0F72, 0x0001},
    {0x1018, 0x0001},
    {0x10E2, 0x0001},
    {0x1190, 0x0001},
    {0x11FD, 0x0001},
    {0x1264, 0x0001},
    {0x12E4, 0x0001}
};

CourseSectorRange gCourseOptionalResourceSectors[10] __attribute__((section(".data.course_sector_resources"))) = {
    {0x0D45, 0x0003},
    {0x0E07, 0x0002},
    {0x0000, 0x0000},
    {0x0000, 0x0000},
    {0x1019, 0x0003},
    {0x0000, 0x0000},
    {0x1191, 0x0003},
    {0x0000, 0x0000},
    {0x125F, 0x0005},
    {0x12E1, 0x0003}
};

u8 gCourseMetadataBuffer[0x800] __attribute__((section(".data.course_sector_resources"), aligned(4))) = {0};

/* Screen copies selected by the saved camera state. */
TexturedSpritePacket gCameraOverlaySpritesBuffer0[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, 96, -120, 0, 0, 0, 64, 240},
};
TexturedSpritePacket gCameraOverlaySpritesBuffer1[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG, 96, -120, 0, 0, 0, 64, 240},
};
POLY_FT4 gCameraOverlayQuadsBuffer0[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, 101, -120, 0, 8, 0, 159, -120, 55, 8, getTPage(2, 0, 256, 0), 101, 119, 0, 231, 0, 159, 119, 55, 231, 0},
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 8, 8, 0, 101, -120, 255, 8, getTPage(2, 0, 0, 0), -160, 119, 8, 231, 0, 101, 119, 255, 231, 0},
};
POLY_FT4 gCameraOverlayQuadsBuffer1[2] __attribute__((section(".data.camera_overlay_packets"))) = {
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, 101, -120, 0, 8, 0, 159, -120, 55, 8, getTPage(2, 0, 256, 256), 101, 119, 0, 231, 0, 159, 119, 55, 231, 0},
    {9 << 24, 128, 128, 128, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG, -160, -120, 8, 8, 0, 101, -120, 255, 8, getTPage(2, 0, 0, 256), -160, 119, 8, 231, 0, 101, 119, 255, 231, 0},
};

/* Semi-transparent screen copies displaced one pixel horizontally. */
TexturedSpritePacket gScreenCopyPlusBuffer1[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -159, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     97, -120, 0, 0, 0, 64, 240}
};
TexturedSpritePacket gScreenCopyPlusBuffer0[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -159, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     97, -120, 0, 0, 0, 64, 240}
};
TexturedSpritePacket gScreenCopyMinusBuffer1[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -161, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 0),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     95, -120, 0, 0, 0, 64, 240}
};
TexturedSpritePacket gScreenCopyMinusBuffer0[2] __attribute__((section(".data.offset_screen_copy_sprites"))) = {
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 0, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     -161, -120, 0, 0, 0, 256, 240},
    {5 << 24, ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(2, 0, 256, 256),
     128, 128, 128, TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG,
     95, -120, 0, 0, 0, 64, 240}
};

struct VisualPlacementHeightData gVisualPlacementHeightData __attribute__((section(".data.visual_placement_heights"))) = {
    0,
    {0, 56, 88, 96, -16, -40, -64, -88,
     -112, -136, -160, -184, -208, -232, -256, -280},
    0
};

extern SVECTOR *D_8013B688;
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern MATRIX D_8012A6B0, D_8012A6D0;
extern MATRIX *D_8013AF9C __attribute__((section(".data")));
extern POLY_FT4 D_8012967C;
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

EmitterPlacement gProximityEmitterPlacements[2]
    __attribute__((section(".data.proximity_emitter_placements"))) = {
    {0x2D8, 0x66D0, 0xA4E8, 0x2C, 0},
    {0x65E0, 0xAB40, 0xEFA0, 0x52, 0},
};

MovingVisualPlacement gMovingVisualPlacements[9]
    __attribute__((section(".data.moving_visual_placements"))) = {
    {24, 0, -2487616, 5385268, -2052416, 5742644},
    {26, 0, -2352960, 6135860, -2678592, 5676084},
    {24, 0, -3052352, 6169652, -2539328, 6403636},
    {29, 0, -2818880, 6921780, -3435840, 6783540},
    {29, 0, -3475776, 7003188, -3229504, 7422516},
    {29, 0, -3573496, 6820636, -3384720, 6964300},
    {27, 0, -2870196, 6550352, -3431836, 6553112},
    {26, 0, -3162140, 6098112, -2780108, 5860364},
    {24, 0, -2426136, 5658860, -2373948, 5250364},
};

OrbitingTaskPlacement gOrbitingTaskPlacements[2]
    __attribute__((section(".data.orbiting_task_placements"))) = {
    {99, 640, 23253, 42781},
    {99, 832, 22656, 45410},
};

u32 gObjectSortSavedStackPointer
    __attribute__((section(".sdata.object_sort_saved_stack"))) = 0;
u32 gAlternateObjectSortSavedStackPointer
    __attribute__((section(".sdata.alternate_object_sort_saved_stack"))) = 0;

ControllerEffectState gControllerEffectStates[2] = {0};

PlayerRenderSnapshot gPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots"))) = {0};
PlayerRenderSnapshot gPreviousPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots"))) = {0};
MATRIX gPlayerRenderIdentityMatrix
    __attribute__((section(".data.player_render_snapshots"))) = {0};

VECTOR gPlayerPairedQuadOffsets[2]
    __attribute__((section(".data.player_paired_quad_offsets"))) = {
    {-50, 0, 0, 0}, {50, 0, 0, 0}
};

SVECTOR gPlayerTripleQuadOffsets[3]
    __attribute__((section(".data.player_triple_quad_offsets"))) = {
    {-40, 0, 0, 0}, {40, 0, 0, 0}, {0, 0, 0, 0}
};

POLY_FT4 gQuadShrinkPacket __attribute__((section(".data.quad_shrink"))) = {
    0x09000000, 0x80, 0x80, 0x80, 0x2C,
    0, 0, 0, 0x10, getClut(944, 240),
    0, 0, 0x27, 0x10, getTPage(0, 0, 960, 0),
    0, 0, 0, 0x1F, 0,
    0, 0, 0x27, 0x1F, 0
};
SVECTOR gQuadShrinkVertices[4] __attribute__((section(".data.quad_shrink"))) = {
    {-144, 24, 0, 0}, {-104, 24, 0, 0},
    {-144, 39, 0, 0}, {-104, 39, 0, 0}
};

SVECTOR gRightQuadShrinkVertices[4] __attribute__((section(".data.right_quad_vertices"))) = {
    {-64, 24, 0, 0}, {-24, 24, 0, 0},
    {-64, 39, 0, 0}, {-24, 39, 0, 0}
};
POLY_FT4 gRightQuadShrinkPacket __attribute__((section(".data.right_quad_packet"))) = {
    0x09000000, 0x80, 0x80, 0x80, 0x2C,
    0, 0, 0x50, 0x10, getClut(944, 240),
    0, 0, 0x77, 0x10, getTPage(0, 0, 960, 0),
    0, 0, 0x50, 0x1F, 0,
    0, 0, 0x77, 0x1F, 0
};

QuadAnimationFrame gQuadAnimationFrames[5] __attribute__((section(".data.quad_animation"))) = {
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 0, 184, 31, 31},
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 32, 184, 31, 31},
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 64, 184, 31, 31},
    {2, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 96, 184, 31, 31},
    {-1, 8092, 128, 128, 128, 2, getTPage(0, 0, 832, 0), getClut(912, 244), 96, 184, 31, 31}
};

u16 gTextClut = 0;

QuadAnimationStep gAlternateQuadAnimationSteps[5]
    __attribute__((section(".data.alternate_quad_steps"))) = {
    {2, 8192, 128, 128, 128, 0,
     getTPage(0, 0, 704, 0), getClut(880, 250), 0, 192, 63, 63},
    {2, 8192, 128, 128, 128, 0,
     getTPage(0, 0, 704, 0), getClut(880, 250), 64, 192, 63, 63},
    {2, 8192, 255, 255, 255, SEMI_TRANSPARENT_FLAG,
     getTPage(0, 0, 704, 0), getClut(880, 250), 128, 192, 63, 63},
    {2, 8192, 255, 255, 255, SEMI_TRANSPARENT_FLAG,
     getTPage(0, 0, 704, 0), getClut(880, 250), 192, 192, 63, 63},
    {-1, 8192, 255, 255, 255, SEMI_TRANSPARENT_FLAG,
     getTPage(0, 0, 704, 0), getClut(880, 250), 96, 192, 63, 63},
};

QuadVertex gMiddleShrinkQuadVertices[4]
    __attribute__((section(".data.middle_shrink_vertices"))) = {
    {-104, 24, 0, 0}, {-64, 24, 0, 0},
    {-104, 39, 0, 0}, {-64, 39, 0, 0},
};

POLY_FT4 gMiddleShrinkQuadPacket
    __attribute__((section(".data.middle_shrink_packet"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 128, 128, 128, TEXTURED_QUAD_CODE,
    0, 0, 40, 16, getClut(944, 240),
    0, 0, 79, 16, getTPage(0, 0, 960, 0),
    0, 0, 40, 31, 0,
    0, 0, 79, 31, 0,
};

POLY_FT4 gSlidingQuadPacket
    __attribute__((section(".data.sliding_quad_packet"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 128, 128, 128, TEXTURED_QUAD_CODE,
    160, 24, 0, 0, 0,
    320, 24, 159, 0, getTPage(0, 0, 960, 0),
    160, 39, 0, 0, 0,
    159, 39, 159, 0, 0,
};

char gDebugPositionXFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vx:%8d";
char gDebugPositionYFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vy:%8d";
char gDebugPositionZFormat[8] __attribute__((section(".data.debug_position_formats"))) = "vz:%8d";

/* Incomplete declarations retain the original absolute address loads. */
extern char debugPositionXFormatAbsolute[];
extern char debugPositionYFormatAbsolute[];
extern char debugPositionZFormatAbsolute[];

extern GameTask *D_8013B304 __attribute__((section(".data")));
RECT gReadPixelRect __attribute__((section(".data.read_pixel_rect"))) = {0, 0, 1, 1};

/* The original stores coordinates through GP but passes an absolute address. */
extern RECT readPixelRectGp;

/* Shared collision regions remain in player_camera_state_tail. */
extern PlayerCollisionRegion D_80129A18[2];
extern u8 D_8013B2B5, D_8013B2B6;
extern VisualPlacement *D_8013B2BC;
extern u8 D_8013B2B9, D_8013B2BA;
/* The course lookup base precedes its valid pointer entries in shared frame data. */
extern TaskVisualFrame *D_80129540[];
void func_8005E728(void);
void func_8005B038(void);
extern PickupPlacement *D_8013B2C0;

/* Shared texture metadata remains in its original data owner. */
extern TextureRegionSet D_8012DDA4[];
extern TextureRegion *D_8013B6AC;

extern TaskSnapshot *D_8013B2E4;
extern RacePlayer *D_8013B2E8;
extern SelectedStateRecord *D_8012C6F4[];
TaskVisualFrame *gCourseVisualFrames
    __attribute__((section(".sdata.course_visual_frames"))) = 0;

CourseTriggerRecord gCourse0Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {49, 7, 0},
    {82, 7, 0},
    {102, 3, 0},
    {139, 1, 0},
    {142, 7, 0},
    {153, 7, 0},
    {166, 7, 0},
    {-1, 0, 0},
};
CourseTriggerRecord gCourse1Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {39, 7, 0},
    {52, 7, 0},
    {103, 7, 0},
    {-1, 0, 0},
};
CourseTriggerRecord gCourse2Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {6, 7, 0},
    {33, 1, 0},
    {44, 1, 0},
    {66, 1, 0},
    {79, 1, 0},
    {106, 1, 0},
    {121, 1, 0},
    {136, 1, 0},
    {140, 1, 0},
    {-1, 0, 0},
};
CourseTriggerRecord gCourse3Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {16, 7, 0},
    {19, 7, 0},
    {34, 3, 0},
    {72, 1, 0},
    {129, 3, 0},
    {140, 1, 0},
    {157, 1, 0},
    {183, 3, 0},
    {272, 0, 0},
    {257, 0, 0},
    {246, 1, 0},
    {234, 1, 0},
    {226, 0, 0},
    {-1, 7, 0},
};
CourseTriggerRecord gCourse4Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {34, 1, 0},
    {70, 1, 0},
    {86, 2, 0},
    {105, 2, 0},
    {137, 3, 0},
    {150, 0, 0},
    {156, 3, 0},
    {186, 3, 0},
    {-1, 7, 0},
};
CourseTriggerRecord gCourse5Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {30, 3, 0},
    {46, 3, 0},
    {119, 0, 0},
    {145, 0, 0},
    {180, 3, 0},
    {183, 3, 0},
    {261, 3, 0},
    {294, 3, 0},
    {303, 3, 0},
    {-1, 7, 0},
};
CourseTriggerRecord gCourse6Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {26, 3, 0},
    {41, 0, 0},
    {76, 3, 0},
    {97, 7, 0},
    {132, 3, 0},
    {175, 3, 0},
    {224, 3, 0},
    {-1, 7, 0},
};
CourseTriggerRecord gCourse8Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {21, 7, 0},
    {31, 7, 0},
    {50, 0, 0},
    {-1, 7, 0},
};
CourseTriggerRecord gCourse9Triggers[]
    __attribute__((section(".data.course_triggers"), aligned(4))) = {
    {23, 7, 0},
    {56, 7, 0},
    {-1, 7, 0},
};
CourseTriggerRecord gEmptyCourseTriggers[]
    __attribute__((section(".data.empty_course_triggers"), aligned(4))) = {
    {-1, 7, 0},
};
CourseTriggerRecord *gCourseTriggers[]
    __attribute__((section(".data.course_triggers"))) = {
    gCourse0Triggers, gCourse1Triggers, gCourse2Triggers, gCourse3Triggers,
    gCourse4Triggers, gCourse5Triggers, gCourse6Triggers, gEmptyCourseTriggers,
    gCourse8Triggers, gCourse9Triggers,
};

extern s8 *D_8012BBB4[][4];
PaletteColor gCapturedColorCyclePalette[16]
    __attribute__((section(".data.color_cycle_palette"), aligned(4))) = {0};
u16 gWhitePulsePalette[16]
    __attribute__((section(".bss.white_pulse_palette"), aligned(4)));
RECT gWhitePulsePaletteRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};
RECT gColorCycleCaptureRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};
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
void func_80061348(void);
void func_800618A0(void);
void func_80061E48(void);
void func_80062354(void);
void func_80065EC8(void);
void emitRadialEffectTask(void);
void func_80066D18(void);
void func_80067DD0(void);
void func_800659F0(void);
extern s32 D_8013B660;
extern SVECTOR D_8012A5C0[];
extern GsCOORDINATE2 D_80129C60[];

s32 D_8013B624 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B628 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B62C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B630 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B634 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B638 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B63C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B640 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B644 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B648 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B64C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;
s32 D_8013B650 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

const char gNoProcessMessage[16] = "NO PROCESS...";

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
void func_8006FE68(s16 x, s16 y, s16 w, s16 h, s16 borderW, s16 borderH,
                   s16 style, s16 clutOffset, s16 blendMode, GsOT *ot, u16 depth);
void updateValueInterpolationTask(void);
void func_80064754(s32 playerIndex);
s32 func_80065464(s32 playerIndex);
void createPlayerTrailTask(s32 playerIndex);
void func_80067D88(s32 playerIndex);
void func_800688C0(s32 playerIndex);
void func_8006FE68(s16 x, s16 y, s16 w, s16 h, s16 borderW, s16 borderH,
                   s16 style, s16 palette, s16 blend, GsOT *ot, u16 depth);

s32 isPointWithinCameraForwardRange(s32 x, s32 z, s16 player);
void initializeProximityParticleTask(void);
void func_8005CEF0(void);
void updateCollectibleTask(void);
void updateLandedTaskObject(void);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800497B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80049CDC);

void setTextPalette(s32 palette) {
    gTextClut = (u8)palette + 0xF0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80049D48);

void initSprite(GsSPRITE *sprite) {
    sprite->r = 0x80;
    sprite->g = 0x80;
    sprite->b = 0x80;
    sprite->attribute = 0;
    sprite->x = 0;
    sprite->y = 0;
    sprite->w = 0;
    sprite->h = 0;
    sprite->tpage = 0;
    sprite->u = 0;
    sprite->v = 0;
    sprite->cx = 0;
    sprite->cy = 0;
    sprite->mx = 0;
    sprite->my = 0;
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
    sprite->rotate = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004A124);

void bindGeometryRecordData(s16 *data) {
    s16 count;

    count = *data++;
    D_8013B1EC = data;
    /* Section lengths wrap as signed 16-bit halfword counts. */
    data += (s16)(count * 3);
    D_8013B1F8 = count;
    count = *data++;
    D_8013B1F0 = (SelectedRecord *)data;
    data += (s16)(count * 6);
    D_8013B1FA = count;
    D_8013B1FC = *data++;
    D_8013B1F4 = data;
}

s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s16 neighbor;

    x0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3] * 5;
    x1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3] * 5;
    x2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3] * 5;
    x3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3] * 5;
    z0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3 + 2] * 5;
    z1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3 + 2] * 5;
    z2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3 + 2] * 5;
    z3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3 + 2] * 5;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[0];
    if (neighbor >= 0 && (x0 - x1) * (z - z1) - (z0 - z1) * (x - x1) > 0) return neighbor;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[1];
    if (neighbor >= 0 && (x3 - x2) * (z - z2) - (z3 - z2) * (x - x2) > 0) return neighbor;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[3];
    if (neighbor >= 0 && (x2 - x0) * (z - z0) - (z2 - z0) * (x - x0) > 0) return neighbor;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[2];
    if (neighbor >= 0 && (x1 - x3) * (z - z3) - (z1 - z3) * (x - x3) > 0) return neighbor;
    return index;
}

void selectGeometryRecord(s16 index) {
    s16 *vertices;
    D_8013B1FE = index;
    vertices = D_8013B1EC;
    D_8013B200 = &vertices[D_8013B1F0[index].vertexIndices[0] * 3];
    D_8013B204 = &vertices[D_8013B1F0[index].vertexIndices[1] * 3];
    D_8013B208 = &vertices[D_8013B1F0[index].vertexIndices[2] * 3];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004A764);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004AAA0);

s32 getGeometryPlacementSignedDistance(s16 index, s32 x, s32 z) {
    s32 vertexIndex;
    s32 dx, dz;
    s32 sine, cosine;
    vertexIndex = ((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3;
    dx = x - D_8013B1EC[vertexIndex] * 5;
    dz = z - D_8013B1EC[vertexIndex + 2] * 5;
    sine = rsin((s16)((GeometryPlacement *)D_8013B1F4)[index].angle);
    cosine = rcos((s16)((GeometryPlacement *)D_8013B1F4)[index].angle);
    return (-sine * dx + cosine * -dz) / 4096;
}

void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle) {
    *x = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3] * 1280;
    *y = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3 + 1] * 1280;
    *z = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3 + 2] * 1280;
    *angle = ((GeometryPlacement *)D_8013B1F4)[index].angle;
}

void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit) {
    s32 length, nx, nz, perpendicularX;
    s32 tangent, normal;
    length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
    nx = (D_8013B644 << 12) / length;
    nz = (D_8013B63C << 12) / length;
    D_8013B62C = length;
    D_8013B624 = nx;
    D_8013B628 = nz;
    perpendicularX = -nx;
    D_8013B638 = (perpendicularX * D_8013B630 + nz * D_8013B638) / 4096;
    if (D_8013B638 < limit) {
        tangent = (nz * *velocityX + nx * *velocityZ) / 4096;
        normal = (perpendicularX * *velocityX + nz * *velocityZ) / 4096;
        if (normal < 0) {
            normal = -normal;
            *velocityX = (nz * tangent - nx * normal) / 4096;
            *velocityZ = (D_8013B624 * tangent + D_8013B628 * normal) / 4096;
        }
        D_8013B644 = limit - D_8013B638;
        D_8013B648 += -D_8013B624 * D_8013B644 / 4096;
        D_8013B650 += D_8013B628 * D_8013B644 / 4096;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004B044);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004BC3C);

void enforceGeometryEdgeDistance(s32 limit) {
    s32 length, nx, nz, perpendicularX;
    length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
    nx = (D_8013B644 << 12) / length;
    nz = (D_8013B63C << 12) / length;
    perpendicularX = -nx;
    D_8013B638 = (perpendicularX * D_8013B630 + nz * D_8013B638) / 4096;
    if (D_8013B638 < limit) {
        D_8013B644 = limit - D_8013B638;
        D_8013B648 += perpendicularX * D_8013B644 / 4096;
        D_8013B650 += nz * D_8013B644 / 4096;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004C968);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004D330);

void adjustDebugPositionAndDisplay(VECTOR *position) {
    if (D_8013AEEC[1] & 0x1000) position->vx += 32;
    if (D_8013AEEC[1] & 0x4000) position->vx -= 32;
    if (D_8013AEEC[1] & 0x8000) position->vz += 32;
    if (D_8013AEEC[1] & 0x2000) position->vz -= 32;
    if (D_8013AEEC[1] & 8) position->vy += 32;
    if (D_8013AEEC[1] & 2) position->vy -= 32;
    func_8006B4A0(0, 10, debugPositionXFormatAbsolute, position->vx);
    func_8006B4A0(0, 11, debugPositionYFormatAbsolute, position->vy);
    func_8006B4A0(0, 12, debugPositionZFormatAbsolute, position->vz);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004D93C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004E09C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004E718);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004EBF8);

void drawCyanBackgroundAndSetFog(void) {
    addFlatRectangle(&D_8013AF6C[4095], -160, -120, 320, 240, 0, 235, 230);
    SetFarColor(80, 128, 104);
    SetFogNearFar(5500, 10000, 400);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004EFE0);

void createOscillatingQuadTask(s32 value, VECTOR *position, s32 amplitude, s32 r, s32 g, s32 b) {
    OscillatingQuadResource *resource;
    func_80014930(&gGameTasks[4], func_8004EFE0);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (OscillatingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->unk68 = value;
    resource->r = r;
    resource->g = g;
    resource->b = b;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->elapsed = 0;
    resource->amplitude = amplitude;
    GsInitCoordinate2(0, &resource->coordinate);
    resource->x = position->vx;
    resource->y = position->vy;
    resource->z = position->vz;
}

void drawMovingLineTask(void) {
    PlayerCamera *camera;
    PlayerCamera *cameras;
    MovingLineResource *resource;
    MovingLinePacket *line;
    u32 brightness;
    s32 yaw;
    s16 pitch;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    cameras = D_801241E8;
    camera = &cameras[D_8013AE88->data18];
    resource = (MovingLineResource *)&D_800D9504[D_8013AE88->resource14];
    yaw = -((camera->yaw & 0xFFF) << 11) / 4096;
    pitch = camera->pitch;
    line = (MovingLinePacket *)GsOUT_PACKET_P;
    line->tag = (sizeof(MovingLinePacket) / sizeof(u32) - 1) << 24;
    line->code = GOURAUD_LINE_CODE | SEMI_TRANSPARENT_FLAG;
    line->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    line->tpage = getTPage(0, 1, 0, 0);
    line->x0 = ((u32)yaw + resource->x) % 320 - 160;
    line->y0 = ((u32)(pitch / 2) + resource->y) % 240 - 120;
    line->r1 = line->g1 = line->b1 = 0;
    line->x1 = line->x0 + 64;
    line->y1 = line->y0 - 32;
    brightness = 256;
    if (resource->elapsed < 16) {
        brightness = resource->elapsed * 16;
    }
    if (resource->duration - 16 < resource->elapsed) {
        brightness = (resource->duration - resource->elapsed) * 16;
    }
    line->b0 = brightness / 2;
    line->r0 = line->g0 = brightness * 255 / 256;
    addPrim(&D_8013AF6C[4094], line);
    GsOUT_PACKET_P = (u32)(line + 1);
    if (!D_8013AFD3) {
        resource->x -= 8;
        resource->y += 4;
        if (++resource->elapsed == resource->duration) {
            func_800149A8();
        }
    }
}

void createMovingLineTask(s32 index) {
    MovingLineResource *resource;
    func_80014930(&gGameTasks[4], drawMovingLineTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = (MovingLineResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = randomNextLinear() % 320;
    resource->y = 0;
    resource->elapsed = 0;
    resource->duration = randomNextLinear() % 15 + 15;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004F780);

void createFlickeringDotsTask(s32 playerIndex) {
    FlickeringDotsResource *resource;
    s32 i;
    func_80014930(&gGameTasks[4], func_8004F780);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = playerIndex;
    resource = (FlickeringDotsResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 25; i++) {
        resource->positions[i].x = randomNextLinear() % 320;
        resource->positions[i].y = randomNextLinear() % 120;
    }
    resource->frame = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004FB70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050080);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050504);

void drawSnowBackgroundAndSetFog(s16 playerIndex) {
    s16 y;
    s16 x;
    y = D_801241E8[playerIndex].pitch / 2;
    x = -160 - ((D_801241E8[playerIndex].yaw >> 1) & 255);
    if (y > 240) y = 240;
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)x, (s16)y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)(x + 256), (s16)y, 256, 64, 0, 0, 128, 128, 128);
    addTexturedSprite(&D_8013AF6C[1024], 26, 0x7C29, (s16)(x + 512), (s16)y, 256, 64, 0, 0, 128, 128, 128);
    if ((s16)y >= -119) {
        addFlatRectangle(&D_8013AF6C[1024], -160, -120, 320, (s16)(y + 120), 0, 168, 248);
    }
    if ((s16)y < -120) y = -184;
    addFlatRectangle(&D_8013AF6C[1024], -160, (s16)(y + 64), 320, (s16)(120 - y), 248, 248, 248);
    SetFarColor(248, 248, 248);
    SetFogNearFar(8000, 10000, 400);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050B18);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050F84);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800512A4);

s32 isPositionInCameraRange(s32 x, s32 z, s16 playerIndex) {
    s32 centerX, centerZ;
    s32 scaledX, scaledZ;
    centerX = gPlayerCameraViews[playerIndex].vpx / 256 +
        ((D_8013B660 / 2) * rsin(D_801241E8[playerIndex].yaw)) / 4096;
    centerZ = gPlayerCameraViews[playerIndex].vpz / 256 +
        ((D_8013B660 / 2) * rcos(D_801241E8[playerIndex].yaw)) / 4096;
    scaledX = x / 256;
    scaledZ = z / 256;
    return SquareRoot0((centerX - scaledX) * (centerX - scaledX) +
        (centerZ - scaledZ) * (centerZ - scaledZ)) < D_8013B660;
}

s32 isPointWithinCameraForwardRange(s32 x, s32 z, s16 player) {
    s32 cameraX, cameraZ;
    s32 scaledX, scaledZ;
    cameraX = gPlayerCameraViews[player].vpx / 256 + ((D_8013B660 / 2) * rsin(D_801241E8[player].yaw)) / 4096;
    cameraZ = gPlayerCameraViews[player].vpz / 256 + ((D_8013B660 / 2) * rcos(D_801241E8[player].yaw)) / 4096;
    scaledX = x / 256;
    scaledZ = z / 256;
    return SquareRoot0((cameraX - scaledX) * (cameraX - scaledX) + (cameraZ - scaledZ) * (cameraZ - scaledZ)) < (D_8013B660 * 3) / 4;
}

void readVramPixelColor(s32 x, s32 y, TextureColor *out) {
    u16 pixel;
    readPixelRectGp.x = x;
    readPixelRectGp.y = y;
    StoreImage(&gReadPixelRect, (u32 *)&pixel);
    out->r = pixel << 3;
    out->g = (pixel >> 2) & 0xF8;
    out->b = (pixel >> 7) & 0xF8;
    out->stp = pixel >> 15;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005162C);

void unpackPsxColor(u32 color, UnpackedPsxColor *out) {
    out->r = color & 0x1F;
    out->g = (color & 0x3E0) >> 5;
    out->b = (color & 0x7C00) >> 10;
    out->stp = (color & 0x8000) >> 15;
}

u32 packTextureColor(TextureColor *color) {
    u32 rgb = color->r >> 3;
    u32 stp = color->stp & 1;
    u32 g = (color->g & 0xF8) << 2;
    u32 b;
    rgb |= g;
    b = (color->b & 0xF8) << 7;
    rgb |= b;
    stp <<= 15;
    return stp | rgb;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80051700);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80051770);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80051A68);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80051D3C);

void queueSelectedCameraOverlayPackets(void) {
    DR_STP *packet;
    switch (selectedCameraOverlayModeGp) {
    case 1:
        if (D_8013AF78) {
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer1[0]);
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer1[1]);
        } else {
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer0[0]);
            addPrim(&D_8013AF6C[2], &gCameraOverlaySpritesBuffer0[1]);
        }
        break;
    case 2:
        if (D_8013AF78) {
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer1[1]);
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer1[0]);
        } else {
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer0[1]);
            addPrim(&D_8013AF6C[2], &gCameraOverlayQuadsBuffer0[0]);
        }
        break;
    }
    packet = (DR_STP *)GsOUT_PACKET_P;
    SetDrawStp(packet, 1);
    addPrim(&D_8013AF6C[4095], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void queueOffsetScreenCopySprites(void) {
    DR_STP *packet;
    if (D_8013AF78) {
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer1[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer1[1]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer1[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer1[1]);
    } else {
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer0[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyPlusBuffer0[1]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer0[0]);
        addPrim(&D_8013AF6C[2], &gScreenCopyMinusBuffer0[1]);
    }
    packet = (DR_STP *)GsOUT_PACKET_P;
    SetDrawStp(packet, 1);
    addPrim(&D_8013AF6C[4095], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawNoProcessMessage(void) {
    func_80049D48(0, 30, gNoProcessMessage);
}

void emitProjectedFlatTriangle(TriangleScratchVertex *vertices) {
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 4 << 24;
    setcode(packet, scratch->code | POLY_F3_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudTriangle(TriangleScratchVertex *vertices) {
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, vertices[0].color.r, vertices[0].color.g, vertices[0].color.b);
        setRGB1(packet, vertices[1].color.r, vertices[1].color.g, vertices[1].color.b);
        setRGB2(packet, vertices[2].color.r, vertices[2].color.g, vertices[2].color.b);
    }
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedTexturedTriangle(TriangleScratchVertex *vertices) {
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 7 << 24;
    setcode(packet, scratch->code | POLY_FT3_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudTexturedTriangle(TriangleScratchVertex *vertices) {
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)0x1F800000;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        packet->r0 = vertices[0].color.r;
        packet->r1 = vertices[1].color.r;
        packet->r2 = vertices[2].color.r;
        packet->g0 = vertices[0].color.g;
        packet->g1 = vertices[1].color.g;
        packet->g2 = vertices[2].color.g;
        packet->b0 = vertices[0].color.b;
        packet->b1 = vertices[1].color.b;
        packet->b2 = vertices[2].color.b;
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedFlatQuad(TriangleScratchVertex *vertices) {
    QuadScratchpad *scratch = QUAD_SCRATCHPAD;
    POLY_F4 *packet = (POLY_F4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 5 << 24;
    setcode(packet, scratch->code | POLY_F4_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudQuad(TriangleScratchVertex *vertices) {
    GouraudQuadScratchpad *scratch = (GouraudQuadScratchpad *)0x1F800000;
    POLY_G4 *packet = (POLY_G4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&vertices[3].color, scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        packet->r0 = vertices[0].color.r;
        packet->g0 = vertices[0].color.g;
        packet->b0 = vertices[0].color.b;
        packet->r1 = vertices[1].color.r;
        packet->g1 = vertices[1].color.g;
        packet->b1 = vertices[1].color.b;
        packet->r2 = vertices[2].color.r;
        packet->g2 = vertices[2].color.g;
        packet->b2 = vertices[2].color.b;
        packet->r3 = vertices[3].color.r;
        packet->g3 = vertices[3].color.g;
        packet->b3 = vertices[3].color.b;
    }
    packet->tag = 8 << 24;
    setcode(packet, scratch->code | POLY_G4_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedTexturedQuad(TriangleScratchVertex *vertices) {
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)0x1F800000;
    POLY_FT4 *packet = (POLY_FT4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_FT4_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->u3 = vertices[3].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->v3 = vertices[3].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudTexturedQuad(TriangleScratchVertex *vertices) {
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)0x1F800000;
    POLY_GT4 *packet = (POLY_GT4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&vertices[3].color, scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        packet->r0 = vertices[0].color.r;
        packet->r1 = vertices[1].color.r;
        packet->r2 = vertices[2].color.r;
        packet->r3 = vertices[3].color.r;
        packet->g0 = vertices[0].color.g;
        packet->g1 = vertices[1].color.g;
        packet->g2 = vertices[2].color.g;
        packet->g3 = vertices[3].color.g;
        packet->b0 = vertices[0].color.b;
        packet->b1 = vertices[1].color.b;
        packet->b2 = vertices[2].color.b;
        packet->b3 = vertices[3].color.b;
    }
    packet->tag = 12 << 24;
    setcode(packet, scratch->code | POLY_GT4_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->u3 = vertices[3].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->v3 = vertices[3].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80053408);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80053A34);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800542BC);

void drawIndexedFlatTriangle(void) {
    ModelFlatTriangle *primitive = D_8013B680;
    TRIANGLE_SCRATCHPAD->emitTriangle = emitProjectedFlatTriangle;
    TRIANGLE_SCRATCHPAD->r = primitive->r;
    TRIANGLE_SCRATCHPAD->g = primitive->g;
    TRIANGLE_SCRATCHPAD->b = primitive->b;
    TRIANGLE_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    TRIANGLE_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    TRIANGLE_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    func_80053A34(TRIANGLE_SCRATCHPAD->vertices, 0);
}

void processGouraudTrianglePrimitive(void) {
    GouraudTrianglePrimitive *primitive = (GouraudTrianglePrimitive *)D_8013B680;
    *(void (**)(TriangleScratchVertex *))0x1F800004 = emitProjectedGouraudTriangle;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    func_80053A34((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedTrianglePrimitive(void) {
    TexturedTrianglePrimitive *primitive = (TexturedTrianglePrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->r = primitive->r;
    ((TriangleTextureState *)0x1F800000)->g = primitive->g;
    ((TriangleTextureState *)0x1F800000)->b = primitive->b;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedTexturedTriangle;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    func_80053A34((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedGouraudTrianglePrimitive(void) {
    TexturedGouraudTrianglePrimitive *primitive = (TexturedGouraudTrianglePrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedGouraudTexturedTriangle;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    func_80053A34((TriangleScratchVertex *)0x1F800070, 0);
}

void drawIndexedFlatQuad(void) {
    ModelFlatQuad *primitive = (ModelFlatQuad *)D_8013B680;
    QUAD_SCRATCHPAD->r = primitive->r;
    QUAD_SCRATCHPAD->g = primitive->g;
    QUAD_SCRATCHPAD->b = primitive->b;
    QUAD_SCRATCHPAD->emitQuad = emitProjectedFlatQuad;
    QUAD_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    QUAD_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    QUAD_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    QUAD_SCRATCHPAD->vertices[3].position = D_8013B688[primitive->vertexIndices[3]];
    func_800542BC(QUAD_SCRATCHPAD->vertices, 0);
}

void processGouraudQuadPrimitive(void) {
    GouraudQuadPrimitive *primitive = (GouraudQuadPrimitive *)D_8013B680;
    QUAD_SCRATCHPAD->emitQuad = emitProjectedGouraudQuad;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F8000A0)->position = D_8013B688[primitive->vertexIndices[3]];
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    ((TriangleScratchVertex *)0x1F8000A0)->color = primitive->colors[3];
    func_800542BC((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedQuadPrimitive(void) {
    TexturedQuadPrimitive *primitive = (TexturedQuadPrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->r = primitive->r;
    ((TriangleTextureState *)0x1F800000)->g = primitive->g;
    ((TriangleTextureState *)0x1F800000)->b = primitive->b;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedTexturedQuad;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F8000A0)->position = D_8013B688[primitive->vertexIndices[3]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    *(UVPair *)0x1F8000A8 = primitive->uv[3].coords;
    func_800542BC((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedGouraudQuadPrimitive(void) {
    TexturedGouraudQuadPrimitive *primitive = (TexturedGouraudQuadPrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedGouraudTexturedQuad;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F8000A0)->position = D_8013B688[primitive->vertexIndices[3]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    *(UVPair *)0x1F8000A8 = primitive->uv[3].coords;
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    ((TriangleScratchVertex *)0x1F8000A0)->color = primitive->colors[3];
    func_800542BC((TriangleScratchVertex *)0x1F800070, 0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800559F4);

void drawProjectedIndexedFlatTriangle(void) {
    ModelFlatTriangle *primitive = D_8013B680;
    POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 4 << 24;
    setcode(packet, scratch->code | POLY_F3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitFlatTriangle(void) {
    ModelLitFlatTriangle *primitive = (ModelLitFlatTriangle *)D_8013B680;
    POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 4 << 24;
    setcode(packet, scratch->code | POLY_F3_CODE);
    gte_ldv0(&primitive->unk08);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitGouraudTriangle(void) {
    ModelLitGouraudTriangle *primitive = (ModelLitGouraudTriangle *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertices[0].vertexIndex], &D_8013B688[primitive->vertices[1].vertexIndex], &D_8013B688[primitive->vertices[2].vertexIndex]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    gte_ldv0(&primitive->vertices[0].unk00);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->vertices[1].unk00);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->vertices[2].unk00);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedGouraudTriangle(void) {
    GouraudTrianglePrimitive *primitive = (GouraudTrianglePrimitive *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitGouraudTriangle(void) {
    ModelLitGouraudTriangle *primitive = (ModelLitGouraudTriangle *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    gte_ldv0(&primitive->unk10);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->unk10);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->unk10);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedColoredLitGouraudTriangle(void) {
    ModelColoredLitGouraudTriangle *primitive = (ModelColoredLitGouraudTriangle *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertices[0].vertexIndex], &D_8013B688[primitive->vertices[1].vertexIndex], &D_8013B688[primitive->vertices[2].vertexIndex]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    gte_ldv0(&primitive->vertices[0].unk00);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->vertices[1].unk00);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->vertices[2].unk00);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedTexturedTriangle(void) {
    TexturedTrianglePrimitive *primitive = (TexturedTrianglePrimitive *)D_8013B680;
    POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 7 << 24;
    setcode(packet, scratch->code | POLY_FT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitTexturedTriangle(void) {
    ModelLitTexturedTriangle *primitive = (ModelLitTexturedTriangle *)D_8013B680;
    POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    gte_ldv0(&primitive->unk14);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    packet->tag = 7 << 24;
    setcode(packet, scratch->code | POLY_FT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitTexturedGouraudTriangle(void) {
    ModelLitTexturedGouraudTriangle *primitive = (ModelLitTexturedGouraudTriangle *)D_8013B680;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    LitTexturedTriangleScratchpad *scratch = (LitTexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertices[0].vertexIndex], &D_8013B688[primitive->vertices[1].vertexIndex], &D_8013B688[primitive->vertices[2].vertexIndex]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
        setRGB1(packet, primitive->r, primitive->g, primitive->b);
        setRGB2(packet, primitive->r, primitive->g, primitive->b);
    }
    gte_ldv0(&primitive->vertices[0].unk00);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->vertices[1].unk00);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->vertices[2].unk00);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedGouraudTexturedTriangle(void) {
    TexturedGouraudTrianglePrimitive *primitive = (TexturedGouraudTrianglePrimitive *)D_8013B680;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitGouraudTexturedTriangle(void) {
    ModelLitGouraudTexturedTriangle *primitive = (ModelLitGouraudTexturedTriangle *)D_8013B680;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    gte_ldv0(&primitive->unk1C);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->unk1C);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->unk1C);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedFlatQuad(void) {
    ModelFlatQuad *primitive = (ModelFlatQuad *)D_8013B680;
    POLY_F4 *packet = (POLY_F4 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 5 << 24;
    setcode(packet, scratch->code | POLY_F4_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedGouraudQuad(void) {
    GouraudQuadPrimitive *primitive = (GouraudQuadPrimitive *)D_8013B680;
    POLY_G4 *packet = (POLY_G4 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&primitive->colors[3], scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
        setRGB3(packet, primitive->colors[3].r, primitive->colors[3].g, primitive->colors[3].b);
    }
    packet->tag = 8 << 24;
    setcode(packet, scratch->code | POLY_G4_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedTexturedQuad(void) {
    TexturedQuadPrimitive *primitive = (TexturedQuadPrimitive *)D_8013B680;
    POLY_FT4 *packet = (POLY_FT4 *)GsOUT_PACKET_P;
    IndexedTexturedQuadScratchpad *scratch = (IndexedTexturedQuadScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_FT4_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->u1 = primitive->uv[1].coords.u;
    packet->v1 = primitive->uv[1].coords.v;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v2 = primitive->uv[2].coords.v;
    packet->u3 = primitive->uv[3].coords.u;
    packet->v3 = primitive->uv[3].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedTexturedGouraudQuad(void) {
    TexturedGouraudQuadPrimitive *primitive = (TexturedGouraudQuadPrimitive *)D_8013B680;
    POLY_GT4 *packet = (POLY_GT4 *)GsOUT_PACKET_P;
    IndexedTexturedQuadScratchpad *scratch = (IndexedTexturedQuadScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&primitive->colors[3], scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
        setRGB3(packet, primitive->colors[3].r, primitive->colors[3].g, primitive->colors[3].b);
    }
    packet->tag = 12 << 24;
    setcode(packet, scratch->code | POLY_GT4_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->u1 = primitive->uv[1].coords.u;
    packet->v1 = primitive->uv[1].coords.v;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v2 = primitive->uv[2].coords.v;
    packet->u3 = primitive->uv[3].coords.u;
    packet->v3 = primitive->uv[3].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058350);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058560);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800585BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058664);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058844);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800589F0);

void queueCourseResourceSectorReads(void) {
    queueCdSectorCommand(0, gCourseResourceSectors[D_8013AFCE][0].sector, gCourseResourceSectors[D_8013AFCE][0].count, (void *)0x801C0000, 0);
    queueCdSectorCommand(0, gCourseResourceSectors[D_8013AFCE][1].sector, gCourseResourceSectors[D_8013AFCE][1].count, (void *)0x801C8800, 0);
    queueCdSectorCommand(2, gCourseResourceSectors[D_8013AFCE][2].sector, gCourseResourceSectors[D_8013AFCE][2].count, (void *)0x80180000, 0);
    queueCdSectorCommand(0, gCourseMetadataSectors[D_8013AFCE].sector, gCourseMetadataSectors[D_8013AFCE].count, gCourseMetadataBuffer, 0);
    if ((u16)gCourseOptionalResourceSectors[D_8013AFCE].sector != 0) {
        queueCdSectorCommand(0, gCourseOptionalResourceSectors[D_8013AFCE].sector, gCourseOptionalResourceSectors[D_8013AFCE].count, (void *)0x801F0800, 0);
    }
    if (D_8013AFCE == 6) {
        queueCdSectorCommand(16, 0x1194, 16, (void *)0x80180000, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058BD4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059254);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059290);

void drawTaskObject(u16 index) {
    GsDOBJ2 *object;
    GsDOBJ2 *objects;
    gObjectSortSavedStackPointer = SetSp(0x1F8003FC);
    objects = D_80129A80;
    object = &objects[index];
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    if (func_80059254()) {
        GsSortObject4(object, &D_800EC0B8[D_8013AF78], 2, (u32 *)0x1F800000);
    } else {
        GsSortObject4(object, &D_800EC018[D_8013AF78], 2, (u32 *)0x1F800000);
    }
    SetSp(gObjectSortSavedStackPointer);
}

void drawTaskObjectAlternateLayer(u16 index) {
    GsDOBJ2 *object;
    GsDOBJ2 *objects;
    gAlternateObjectSortSavedStackPointer = SetSp(0x1F8003FC);
    objects = D_80129A80;
    object = &objects[index];
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    if (func_80059254()) {
        GsSortObject4(object, &D_800EC130[D_8013AF78], 2, (u32 *)0x1F800000);
    } else {
        GsSortObject4(object, &D_800EC090[D_8013AF78], 2, (u32 *)0x1F800000);
    }
    SetSp(gAlternateObjectSortSavedStackPointer);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059484);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059958);

void initializeSelectedCourseOffsetTask(void) {
    CourseOffsetResource *resource;
    SelectedCourseRecord *records;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    records = D_80123D80;
    resource->rotation.vx = 0;
    resource->rotation.vy = records[D_8013AFCE].unk16;
    resource->rotation.vz = 0;
    D_8013AF94->vx = 0;
    D_8013AF94->vy = 0;
    D_8013AF94->vz = -0x330;
    RotMatrix(&resource->rotation, D_8013AF98);
    ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
    resource->position.vx = records[D_8013AFCE].unk18[3] + (D_8013AF88->vx << 8);
    resource->position.vy = records[D_8013AFCE].unk18[4];
    resource->position.vz = records[D_8013AFCE].unk18[5] + (D_8013AF88->vz << 8);
    if (D_8013AFCE != 8) {
        D_8013AE88->data1C = 4;
    } else {
        D_8013AE88->data1C = 0x19;
    }
    D_8013AE88->data1E = 0x16;
    setCurrentTaskCallback(updateSelectedCourseOffsetTask);
}

void updateSelectedCourseOffsetTask(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        RotMatrix(&resource->rotation, D_8013AF98);
        D_8013AF90->vx = 0;
        D_8013AF90->vy = 0;
        D_8013AF90->vz = 0x1000;
        ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
        resource->position.vx += D_8013AF88->vx;
        resource->position.vz += D_8013AF88->vz;
        resource->coordinate.coord.t[0] = resource->position.vx >> 8;
        resource->coordinate.coord.t[1] = resource->position.vy >> 8;
        resource->coordinate.coord.t[2] = resource->position.vz >> 8;
        if (D_8013AE88->data1E != 0) {
            D_8013AE88->data1E--;
        }
    }
    if (D_8013AE88->data1E == 0) {
        if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
            D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
            func_8001E3E4(&resource->rotation, &resource->coordinate);
            drawTaskObject(D_8013AE88->data1C);
        }
    }
    if (resource->linkedTask->data1A == 6) {
        setCurrentTaskCallback(updatePlayerFollowingOrbitTask);
    }
}

void updatePlayerFollowingOrbitTask(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (D_80120E90[D_8013AE88->data18].unk12C != 7) {
            D_8013AF94->vx = -100;
            D_8013AF94->vy = 0;
            D_8013AF94->vz = 0;
            RotMatrix(&resource->rotation, D_8013AF98);
            ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
            resource->position.vx = resource->coordinate.coord.t[0] + D_8013AF88->vx;
            resource->position.vz = resource->coordinate.coord.t[2] + D_8013AF88->vz;
            D_8013AE88->data1A = 0x40;
            setCurrentTaskCallback(updateOrbitingTaskObject);
            updateOrbitingTaskObject();
            return;
        }
        resource->coordinate.coord.t[0] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[0];
        resource->coordinate.coord.t[1] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[1];
        resource->coordinate.coord.t[2] = D_80120E90[D_8013AE88->data18].worldCoordinate.coord.t[2];
        if (resource->linkedTask->data1A == 7) {
            resource->rotation.vy = D_80120E90[D_8013AE88->data18].unk110;
        }
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
}

void updateOrbitingTaskObject(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->rotation.vy -= 0x20;
        D_8013AF94->vx = 100;
        D_8013AF94->vy = 0;
        D_8013AF94->vz = 0;
        RotMatrix(&resource->rotation, D_8013AF98);
        ApplyMatrix(D_8013AF98, D_8013AF94, D_8013AF88);
        resource->coordinate.coord.t[0] = resource->position.vx + D_8013AF88->vx;
        resource->coordinate.coord.t[2] = resource->position.vz + D_8013AF88->vz;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
    if (!D_8013AFD3) {
        if (--D_8013AE88->data1A == 0) {
            D_8013AE88->data1A = 0x40;
            setCurrentTaskCallback(updateMovingTaskObject);
        }
    }
}

void updateMovingTaskObject(void) {
    CourseOffsetResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (CourseOffsetResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        RotMatrix(&resource->rotation, D_8013AF98);
        D_8013AF90->vx = 0;
        D_8013AF90->vy = 0;
        D_8013AF90->vz = 8;
        ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
        resource->coordinate.coord.t[0] += D_8013AF88->vx;
        resource->coordinate.coord.t[2] += D_8013AF88->vz;
        D_8013AE88->data1A--;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[D_8013AE88->data1C].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(D_8013AE88->data1C);
    }
    if (D_8013AE88->data1A == 0) {
        func_800149A8();
    }
}

void computeSelectedTriangleAngles(s16 *pitch, s16 *yaw) {
    VECTOR normal;
    s32 x, y, z;
    computeSelectedTriangleNormal(&normal);
    *yaw = func_8004A124(0, 0, normal.vx, normal.vy);
    x = normal.vx;
    y = normal.vy;
    z = normal.vz;
    x /= 16;
    y /= 16;
    z /= 16;
    *pitch = -func_8004A124(0, 0, z, SquareRoot0(x*x + y*y));
}

void updateCameraRotationMatrices(u16 playerIndex) {
    PlayerCamera *cameras = D_801241E8;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = cameras[playerIndex].yaw;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, &D_8012A6B0);
    D_8013AF90->vx = cameras[func_80059254()].pitch;
    D_8013AF90->vy = cameras[func_80059254()].yaw;
    D_8013AF90->vz = 0;
    RotMatrixYXZ(D_8013AF90, &D_8012A6D0);
}


void prepareTaskVisualQuad(TaskVisualResource *resource, TaskVisualFrame *frame, u8 usePitch) {
    if (usePitch == 0) {
        *D_8013AF9C = D_8012A6B0;
    } else {
        *D_8013AF9C = D_8012A6D0;
    }
    resource->frame = frame;
    ScaleMatrix(D_8013AF9C, (VECTOR *)&resource->scaleX);
    D_8012967C.r0 = resource->r0;
    D_8012967C.g0 = resource->g0;
    D_8012967C.b0 = resource->b0;
    D_8012967C.tpage = resource->frame->tpage;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005A788);

void setTaskVisualFrame(TaskVisualResource *resource, TaskVisualFrame *frames, s32 scale) {
    u16 index;
    resource->scaleX = scale;
    resource->scaleY = scale;
    resource->scaleZ = scale;
    index = resource->framePosition >> 8;
    resource->vertices[0].x = (-frames[index].width - 1) / 2;
    resource->vertices[0].y = -frames[index].height;
    resource->vertices[0].z = 0;
    resource->vertices[1].x = (frames[index].width - 1) / 2;
    resource->vertices[1].y = -frames[index].height;
    resource->vertices[1].z = 0;
    resource->vertices[2].x = (-frames[index].width - 1) / 2;
    resource->vertices[2].y = -1;
    resource->vertices[2].z = 0;
    resource->vertices[3].x = (frames[index].width - 1) / 2;
    resource->vertices[3].y = -1;
    resource->vertices[3].z = 0;
    resource->frame = &frames[index];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005AC4C);

void initializePickupVisualTask(void) {
    TaskVisualResource *resource;
    u16 i;
    if ((s16)getModeFilteredGameOptionFlag2() == 0) {
        func_800149A8();
    } else {
        for (i = D_8013B2B5; i < D_8013B2B5 + D_8013B2B6; i++) {
            if (D_8013B2BC[i].flags & 0x40) {
                D_8013B2BC[i].flags &= 0xBF;
            }
        }
        if (D_8013AE88->resource14 != 0xFFFF) {
            while (1) {}
        }
        D_8013AE88->resource14 = allocateTaskResource();
        resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->unkC6 = 0x600;
        resource->unkC8 = 0x80;
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->framePosition = 0;
        resource->scaleX = 0x1800;
        resource->scaleY = 0x1800;
        resource->scaleZ = 0x1800;
        setCurrentTaskCallback(func_8005B038);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B038);

void initializeObjectVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x100;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->scaleX = 0x3800;
    resource->scaleY = 0x3800;
    resource->scaleZ = 0x3800;
    setCurrentTaskCallback(func_8005B460);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B460);

void initializeInteractionVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x100;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE;
    resource->unkC8 = 0;
    resource->framePosition = 0;
    resource->scaleX = 0x2000;
    resource->scaleY = 0x2000;
    resource->scaleZ = 0x2000;
    setCurrentTaskCallback(func_8005B6C0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B6C0);

void initializeCourseVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    if (D_8013AFCE == 2) {
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->scaleX = 0x6000;
        resource->scaleY = 0x6000;
        resource->scaleZ = 0x6000;
        resource->unkC6 = 0;
        resource->unkC8 = 0;
        resource->framePosition = 0;
        gCourseVisualFrames = D_8013B29C;
    } else {
        resource->unkC6 = 0x200;
        resource->unkC8 = 0x40;
        resource->r0 = resource->g0 = resource->b0 = 0x80;
        resource->code = POLY_FT4_CODE;
        resource->scaleX = 0x2000;
        resource->scaleY = 0x2000;
        resource->scaleZ = 0x2000;
        resource->framePosition = 0;
        gCourseVisualFrames = D_801294AC;
    }
    setCurrentTaskCallback(updateCourseVisualTask);
}

void updateCourseVisualTask(void) {
    TaskVisualResource *resource;
    u8 i;
    s32 height;
    s16 *heightOffsets;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    prepareTaskVisualQuad(resource, gCourseVisualFrames, 0);
    for (i = D_8013B2B9; i < D_8013B2B9 + D_8013B2BA; i++) {
        if (isPointWithinCameraForwardRange(D_8013B2BC[i].x, D_8013B2BC[i].z, (s16)func_80059254())) {
            resource->coordinate.coord.t[0] = D_8013B2BC[i].x;
            resource->coordinate.coord.t[1] = D_8013B2BC[i].y;
            resource->coordinate.coord.t[2] = D_8013B2BC[i].z;
            height = D_8013B2BC[i].flags & 0xF;
            if (height != 0) {
                heightOffsets = gVisualPlacementHeightData.offsets;
                resource->coordinate.coord.t[1] -= heightOffsets[height];
            }
            func_8005A788(resource, D_80129540[D_8013B2BC[i].unk01], 2);
        }
    }
    if (D_8013AFD3 == 0) {
        if (D_8013AFCE == 9) resource->framePosition += resource->unkC8;
        if (resource->framePosition >= resource->unkC6) resource->framePosition &= 0xFF;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005BC70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005BF00);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005C51C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005C7AC);

void updateBobbingTrailEmitterTask(void) {
    BobbingTrailEmitterResource *resource;
    EmitterTrailResource *trail;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (BobbingTrailEmitterResource *)&D_800D9504[D_8013AE88->resource14];
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        if (!D_8013AFD3) {
            func_8002BF88(&resource->position, 0x7000, 0x18000, 2);
            resource->heading.value += resource->headingStep;
            if (resource->heading.halves.low > resource->baseHeading.halves.low + 0x140) resource->headingStep = -8;
            if (resource->heading.halves.low < resource->baseHeading.halves.low - 0x140) resource->headingStep = 8;
            resource->phase += 0x40;
            resource->rotation.vy = resource->heading.halves.low;
            resource->coordinate.coord.t[1] = 0xB654 - (rsin(resource->phase) >> 9);
            if (--resource->countdown == 0) {
                resource->countdown = 15;
                playPositionalSoundEffect(resource->position.x, resource->position.y, resource->position.z, 1, 0xA00, 0x18);
                if ((s16)func_800148B4(initializeVisualTrailTask) == 1) {
                    D_8013AE8C->data18 = 0xFF;
                    D_8013AE8C->resource14 = allocateTaskResource();
                    trail = (EmitterTrailResource *)&D_800D9504[D_8013AE8C->resource14];
                    GsInitCoordinate2(0, &trail->coordinate);
                    trail->geometryIndex = resource->geometryIndex;
                    trail->coordinate.coord.t[0] = resource->coordinate.coord.t[0];
                    trail->coordinate.coord.t[1] = resource->coordinate.coord.t[1] - 0x100;
                    trail->coordinate.coord.t[2] = resource->coordinate.coord.t[2];
                    trail->angle = (resource->heading.value + 0x800) & 0xFFF;
                }
            }
        }
        D_80129A80[10].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        D_8013AF88->vx = 0x3000;
        D_8013AF88->vy = 0x3000;
        D_8013AF88->vz = 0x3000;
        ScaleMatrix(&resource->coordinate.coord, D_8013AF88);
        drawTaskObject(10);
    }
}

void initializeProximityEmitterTask(void) {
    ProximityEmitterResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (ProximityEmitterResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->coordinate.coord.t[0] = gProximityEmitterPlacements[D_8013AE88->data18].x;
    resource->coordinate.coord.t[1] = gProximityEmitterPlacements[D_8013AE88->data18].y;
    resource->coordinate.coord.t[2] = gProximityEmitterPlacements[D_8013AE88->data18].z;
    resource->geometryIndex = gProximityEmitterPlacements[D_8013AE88->data18].geometryIndex;
    resource->unkC0 = 0;
    resource->angle = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle + 0x400;
    resource->unkC4 = 0;
    resource->countdown = 1;
    setCurrentTaskCallback(updateProximityEmitterTask);
}

void updateProximityEmitterTask(void) {
    ProximityEmitterResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (ProximityEmitterResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
            if (--resource->countdown == 0) {
                resource->countdown = 0x20;
                func_800148B4(initializeProximityParticleTask);
            }
        }
    }
}

void initializeProximityParticleTask(void) {
    EmittedParticleResource *resource;
    ProximityEmitterResource *parent;
    s32 xOffset, zOffset;
    s32 sine, cosine;
    if (D_8013AE88->resource14 != 0xFFFF) { while (1) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (EmittedParticleResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) { while (1) {} }
    parent = (ProximityEmitterResource *)&D_800D9504[D_8013AE98->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->geometryIndex = parent->geometryIndex;
    resource->rotation = 0;
    resource->angle = parent->angle;
    resource->unkC4 = 0;
    xOffset = ((u8)((u8)randomNextSecondary() % 60) - 30) * 10;
    zOffset = ((u8)((u8)randomNextSecondary() % 60) - 30) * 10;
    resource->positionX = (parent->coordinate.coord.t[0] + xOffset) << 8;
    resource->positionY = parent->coordinate.coord.t[1] << 8;
    resource->positionZ = (parent->coordinate.coord.t[2] + zOffset) << 8;
    sine = rsin(resource->angle);
    cosine = rcos(resource->angle);
    resource->velocityX = (sine * 2560) / 4096;
    resource->velocityZ = (cosine * 2560) / 4096;
    resource->velocityY = -4096;
    resource->gravity = 80;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 90;
    setCurrentTaskCallback(func_8005CEF0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005CEF0);

void initializeMovingVisualTask(void) {
    TaskVisualResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (TaskVisualResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->unkC6 = 0x800;
    resource->unkC8 = 0x80;
    resource->r0 = resource->g0 = resource->b0 = 0x80;
    resource->code = POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG;
    resource->framePosition = 0;
    resource->scaleX = 0x1000;
    resource->scaleY = 0x1000;
    resource->scaleZ = 0x1000;
    resource->unkC4 = gMovingVisualPlacements[D_8013AE88->data18].geometryIndex;
    resource->position.x = gMovingVisualPlacements[D_8013AE88->data18].startX;
    resource->position.z = gMovingVisualPlacements[D_8013AE88->data18].startZ;
    resource->velocityY = func_8004A124(resource->position.x, resource->position.z,
        gMovingVisualPlacements[D_8013AE88->data18].endX, gMovingVisualPlacements[D_8013AE88->data18].endZ);
    resource->unkDC = 0x10;
    resource->unkE0 = 0x80;
    resource->unkD8 = 0;
    resource->bounceCount = 0x400;
    resource->groundY = 0;
    resource->unkC4 = getAdjacentGeometryRegionForPoint(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8);
    resource->position.y = func_8004A764(resource->unkC4, resource->position.x >> 8, resource->position.z >> 8) << 8;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    setCurrentTaskCallback(func_8005D394);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005D394);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005D828);

void updateActivePlayerRegionCollisions(void) {
    u16 i;
    PlayerCollisionRegion *region;
    RacePlayer *players;
    if (!D_8013AFD3) {
        i = 0;
        region = &D_80129A18[D_8013AE88->data18];
        players = D_80120E90;
        do {
            if (players[i].unk024 != 0) {
                func_8005D828(&players[i], region);
            }
            i++;
        } while (i < 4);
    }
}

void initializeCollectibleTask(void) {
    PickupResource *resource;
    s32 height;
    s16 *heightOffsets;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PickupResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    resource->geometryIndex = D_8013B2C0[D_8013AE88->data18].geometryIndex;
    resource->rotation.vx = 0;
    resource->rotation.vy = ((GeometryPlacement *)geometryPlacementDataAbsolute)[resource->geometryIndex].angle;
    resource->rotation.vz = 0;
    resource->coordinate.coord.t[0] = D_8013B2C0[D_8013AE88->data18].x;
    resource->coordinate.coord.t[2] = D_8013B2C0[D_8013AE88->data18].z;
    height = func_8004A764(resource->geometryIndex, resource->coordinate.coord.t[0], resource->coordinate.coord.t[2]);
    D_8013B2C0[D_8013AE88->data18].y = height;
    resource->coordinate.coord.t[1] = height;
    resource->position.x = resource->coordinate.coord.t[0] << 8;
    resource->position.y = resource->coordinate.coord.t[1] << 8;
    resource->position.z = resource->coordinate.coord.t[2] << 8;
    if (gameOptionPlayerModeAbsolute == 1) {
        resource->collisionRange = 0x2C00;
    } else {
        resource->collisionRange = 0x3400;
    }
    setCurrentTaskCallback(updateCollectibleTask);
}

void updateCollectibleTask(void) {
    PickupResource *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (PickupResource *)&D_800D9504[D_8013AE88->resource14];
    if (isPositionInCameraRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[10].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(10);
        if (!D_8013AFD3) {
            func_8002BF88(&resource->position, 0x2800, 0x8000, 2);
            for (i = 0; i < D_8013B1D4; i++) {
                if (arePositionsWithinXZRange(&resource->position, &D_8012450C[i].position, resource->collisionRange)) {
                    *D_8012450C[i].active = 0;
                    D_80120E90[D_8012450C[i].playerIndex].unk95E++;
                    playSoundEffect(1, 0xF00, 0x20);
                    createNineIndexedChildTasks(0x15, 1);
                    D_8013AE88->callback = deleteCurrentTask;
                    D_8013AE88->unk10 |= 1;
                    break;
                }
            }
        }
    }
}

void initializeFallingTaskObject(void) {
    FallingTaskResource *resource;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (FallingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->coordinate);
    D_8013AE88->data1A = D_8013B2BC[D_8013AE88->data18].unk01;
    resource->geometryIndex = D_80120E90[D_8013AE88->data18].unk92E;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->position.x = D_80120E90[D_8013AE88->data18].x;
    resource->position.y = D_80120E90[D_8013AE88->data18].y - 0x2000;
    resource->position.z = D_80120E90[D_8013AE88->data18].z;
    resource->coordinate.coord.t[0] = resource->position.x >> 8;
    resource->coordinate.coord.t[1] = resource->position.y >> 8;
    resource->coordinate.coord.t[2] = resource->position.z >> 8;
    resource->verticalVelocity = -0xA00;
    resource->groundHeight = func_8004A764(resource->geometryIndex, resource->position.x >> 8, resource->position.z >> 8) << 8;
    computeSelectedTriangleAngles(&resource->groundPitch, &resource->groundRoll);
    setCurrentTaskCallback(updateFallingTaskObject);
}

void updateFallingTaskObject(void) {
    FallingTaskResource *resource;
    u8 landed = 0;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        resource->verticalVelocity += 0x180;
        resource->position.y += resource->verticalVelocity;
        if (resource->position.y > resource->groundHeight) {
            resource->position.y = resource->groundHeight;
            resource->rotation.vx = resource->groundPitch;
            resource->rotation.vz = resource->groundRoll;
            landed = 1;
        }
        resource->coordinate.coord.t[1] = resource->position.y >> 8;
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[9].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(9);
    }
    if (landed) {
        D_8013AE88->data1A = 0x708;
        setCurrentTaskCallback(updateLandedTaskObject);
    }
}

void updateLandedTaskObject(void) {
    FallingTaskResource *resource;
    u16 i;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FallingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    if (!D_8013AFD3) {
        if (--D_8013AE88->data1A == 0) {
            func_800149A8();
            return;
        }
        for (i = 0; i < D_8013AFD2; i++) {
            if (addPlayerFlagsIfWithinRadius(&resource->position, 0xC00, 4, i)) {
                func_800149A8();
                return;
            }
        }
    }
    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0], resource->coordinate.coord.t[2], (s16)func_80059254())) {
        D_80129A80[9].coord2 = &resource->coordinate;
        func_8001E3E4(&resource->rotation, &resource->coordinate);
        drawTaskObject(9);
    }
}

void initializeOrbitingTaskObject(void) {
    OrbitingTaskResource *resource;
    u16 i;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (OrbitingTaskResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->groundCoordinate);
    GsInitCoordinate2(&resource->groundCoordinate, &resource->coordinate);
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = 0;
    resource->phase = (0x1000 / D_8013AE88->data1A) * D_8013AE88->data1C;
    resource->geometryIndex = gOrbitingTaskPlacements[D_8013AE88->data18].geometryIndex;
    resource->groundCoordinate.coord.t[0] = gOrbitingTaskPlacements[D_8013AE88->data18].x;
    resource->groundCoordinate.coord.t[2] = gOrbitingTaskPlacements[D_8013AE88->data18].z;
    resource->groundCoordinate.coord.t[1] = func_8004A764(resource->geometryIndex,
        resource->groundCoordinate.coord.t[0], resource->groundCoordinate.coord.t[2]);
    computeSelectedTriangleAngles(&resource->groundRotation.vx, &resource->groundRotation.vz);
    resource->groundRotation.vy = 0;
    for (i = 0; i < D_8013AFD2; i++) {
        resource->playerCooldowns[i] = 0;
    }
    setCurrentTaskCallback(func_8005E728);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005E728);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EBC4);

void updateAndDrawCurrentTaskObject(void) {
    func_8001E3E4(&D_8012A5C0[D_8013AE88->data18],
                  &D_80129C60[D_8013AE88->data18]);
    drawTaskObject(D_8013AE88->data18);
}

void copySlot26TransformToCurrentTaskObject(void) {
    D_80129C60[D_8013AE88->data18].coord.t[0] = D_80129C60[26].coord.t[0];
    D_80129C60[D_8013AE88->data18].coord.t[1] = D_80129C60[26].coord.t[1];
    D_80129C60[D_8013AE88->data18].coord.t[2] = D_80129C60[26].coord.t[2];
    D_8012A5C0[D_8013AE88->data18].vx = D_8012A5C0[26].vx;
    D_8012A5C0[D_8013AE88->data18].vy = D_8012A5C0[26].vy;
    D_8012A5C0[D_8013AE88->data18].vz = D_8012A5C0[26].vz;
    setCurrentTaskCallback(updateAndDrawCurrentTaskObject);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EFA0);

void updateCurrentPlayerCourseTrigger(void) {
    s32 i = 0;
    CourseTriggerRecord *record;
    s32 x, y, z;
    u16 angle;
    s32 sine, cosine;
    s32 mask;
    if (currentPlayerAbsolute->unk128 & 0x1000) currentPlayerAbsolute->unk948 = 0;
    currentPlayerAbsolute->unk950[0] = 0;
    record = gCourseTriggers[D_8013AFCE];
    for (;; record++, i++) {
        if (record->geometryIndex == -1) return;
        mask = 1 << i;
        if (!(currentPlayerAbsolute->unk948 & mask) && record->geometryIndex >= currentPlayerAbsolute->unk92E && record->geometryIndex - 3 < currentPlayerAbsolute->unk92E) {
            getGeometryPlacementTransform(record->geometryIndex, &x, &y, &z, &angle);
            x -= currentPlayerAbsolute->x;
            y -= currentPlayerAbsolute->y;
            z -= currentPlayerAbsolute->z;
            sine = rsin(-(s16)angle);
            cosine = rcos(-(s16)angle);
            z = (-sine * x + cosine * z) / 4096;
            if (z > 0 && z < 0x1E000) {
                currentPlayerAbsolute->unk948 |= mask;
                currentPlayerAbsolute->unk950[0] = 1;
                currentPlayerAbsolute->unk950[3] = record->value;
                return;
            }
        }
    }
}

void updatePlayerField94CFromSelectedTable(u16 tableIndex, s16 sampleIndex,
                                          u16 playerIndex) {
    s32 delta;
    RacePlayer *player;

    delta = D_8012BBB4[D_8013AFCE][tableIndex][sampleIndex] * 20;
    player = &D_80120E90[playerIndex];
    delta -= player->unk94C;
    if (delta > 30) {
        delta = 30;
    }
    if (delta < -30) {
        delta = -30;
    }
    player->unk94C += delta;
}

void resetSelectedRecordStates(void) {
    SelectedStateRecord *record = D_8012C6F4[D_8013AFCE];

    for (;;) {
        if (record->state == -2) {
            return;
        }
        record->state = -1;
        record++;
    }
}

void captureSelectedPlayerStates(void) {
    s8 split = 0;
    s32 i = 0;
    SelectedStateRecord *record = D_8012C6F4[D_8013AFCE];
    for (;; record++, i++) {
        GameTask *task;
        u16 inputIndex;
        if (record->state == -2) goto done;
        if (record->state == -1 && record->unk02 == D_80120E90[0].unk92E) {
            inputIndex = D_8013AEDC[0];
            record->state = inputIndex - 1;
            if (D_8013AED4[0][D_8013AEDC[0]-1].repeatCount) {
                record->state = inputIndex;
                split = 1;
            }
            D_8013B2E8[i] = D_80120E90[0];
            task = D_80120E90[0].task;
            D_8013B2E4[i].callback = task->callback;
            D_8013B2E4[i].data18 = task->data18;
            D_8013B2E4[i].data1A = task->data1A;
            D_8013B2E4[i].data1C = task->data1C;
            D_8013B2E4[i].data1E = task->data1E;
        }
    }
done:
    if (split) {
        D_8013AED4[0][D_8013AEDC[0]-1].repeatCount--;
        D_8013AED4[0][D_8013AEDC[0]].repeatCount = 0;
        D_8013AED4[0][D_8013AEDC[0]].horizontal = D_8013AED4[0][D_8013AEDC[0]-1].horizontal;
        D_8013AED4[0][D_8013AEDC[0]].vertical = D_8013AED4[0][D_8013AEDC[0]-1].vertical;
        D_8013AED4[0][D_8013AEDC[0]].buttons = D_8013AED4[0][D_8013AEDC[0]-1].buttons;
        D_8013AED4[0][D_8013AEDC[0]].pressedButtons = D_8013AED4[0][D_8013AEDC[0]-1].pressedButtons;
        D_8013AEDC[0]++;
    }
}

s32 restoreSelectedPlayerState(void) {
    s32 i = 0;
    SelectedStateRecord *record = D_8012C6F4[D_8013AFCE];
    for (;;) {
        if (record->state == -2) return 0;
        if (record->state != -1) {
            GameTask *task = D_80120E90[0].task;
            D_80120E90[0] = D_8013B2E8[i];
            D_80120E90[0].task = task;
            task->callback = D_8013B2E4[i].callback;
            task->data18 = D_8013B2E4[i].data18;
            task->data1A = D_8013B2E4[i].data1A;
            task->data1C = D_8013B2E4[i].data1C;
            task->data1E = D_8013B2E4[i].data1E;
            D_8013AEDC[0] = record->state;
            D_8013AEE0[0] = D_8013AED4[0][D_8013AEDC[0]].repeatCount;
            record->state = -1;
            D_801241E8[0].x = record->unk04;
            D_801241E8[0].y = record->unk08;
            D_801241E8[0].z = record->unk0C;
            D_8013B20C = record->unk10;
            D_801241E8[0].unk16 = 8;
            return 1;
        }
        record++;
        i++;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FAE0);

void addCopiedPrimitive(u32 *orderingTable, u32 *packet) {
    u32 length = ((P_TAG *)packet)->len;
    u32 *out = (u32 *)GsOUT_PACKET_P;
    u8 remaining = length;

    if (length != 0xFF) {
        do {
            *out++ = *packet++;
        } while (--remaining != 0xFF);
    }
    {
        u32 *start = (u32 *)GsOUT_PACKET_P;

        GsOUT_PACKET_P = (u32)out;
        addPrim(orderingTable, start);
    }
}

void addFlatTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 x2, s32 y2, s32 r, s32 g, s32 b) {
    POLY_F3 *triangle = (POLY_F3 *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(POLY_F3) / sizeof(u32) - 1) << 24;
    setcode(triangle, 0x20);
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r, g, b);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addGouraudTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 r0, s32 g0, s32 b0,
                  s32 r1, s32 g1, s32 b1, s32 r2, s32 g2, s32 b2) {
    POLY_G3 *triangle = (POLY_G3 *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    setcode(triangle, POLY_G3_CODE);
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r0, g0, b0);
    setRGB1(triangle, r1, g1, b1);
    setRGB2(triangle, r2, g2, b2);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addTexturedTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 x2, s32 y2, u8 tpage, s32 clut,
                   s32 u0, s32 v0, s32 u1, s32 v1, s32 u2, s32 v2,
                   s32 r, s32 g, s32 b) {
    POLY_FT3 *triangle = (POLY_FT3 *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    setcode(triangle, POLY_FT3_CODE);
    triangle->tpage = tpage;
    triangle->clut = clut;
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    triangle->u0 = u0;
    triangle->v0 = v0;
    triangle->u1 = u1;
    triangle->v1 = v1;
    triangle->u2 = u2;
    triangle->v2 = v2;
    setRGB0(triangle, r, g, b);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FE7C);

void addSemiTransparentTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                   s32 x1, s32 y1, s32 x2, s32 y2, s32 r, s32 g, s32 b) {
    SemiTransparentTrianglePacket *triangle = (SemiTransparentTrianglePacket *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    triangle->code = 0x22;
    triangle->tpage = tpage & 0xFF;
    triangle->drawModeCode = 0xE100;
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r, g, b);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addSemiTransparentGouraudTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                  s32 x1, s32 y1, s32 x2, s32 y2,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2) {
    SemiTransparentGouraudTrianglePacket *triangle = (SemiTransparentGouraudTrianglePacket *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    triangle->code = POLY_G3_CODE | SEMI_TRANSPARENT_FLAG;
    triangle->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    triangle->tpage = tpage & 0xFF;
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r0, g0, b0);
    setRGB1(triangle, r1, g1, b1);
    setRGB2(triangle, r2, g2, b2);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addFlatQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 x2, s32 y2, s32 x3, s32 y3, s32 r, s32 g, s32 b) {
    POLY_F4 *poly = (POLY_F4 *)GsOUT_PACKET_P;

    poly->tag = (sizeof(POLY_F4) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_F4_CODE);
    setXY2(poly, x0, y0, x1, y1);
    poly->x2 = x2;
    poly->y2 = y2;
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r, g, b);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addGouraudQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3) {
    POLY_G4 *poly = (POLY_G4 *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_G4_CODE);
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r0, g0, b0);
    setRGB1(poly, r1, g1, b1);
    setRGB2(poly, r2, g2, b2);
    setRGB3(poly, r3, g3, b3);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addTexturedQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3, s32 flags, u8 tpage, s32 clut,
                  s32 u0, s32 v0, s32 u1, s32 v1, s32 u2, s32 v2,
                  s32 u3, s32 v3, s32 r, s32 g, s32 b) {
    POLY_FT4 *poly = (POLY_FT4 *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_FT4_CODE | flags);
    poly->tpage = tpage;
    poly->clut = clut;
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->x3 = x3;
    poly->y3 = y3;
    poly->u0 = u0;
    poly->v0 = v0;
    poly->u1 = u1;
    poly->v1 = v1;
    poly->u2 = u2;
    poly->v2 = v2;
    poly->u3 = u3;
    poly->v3 = v3;
    setRGB0(poly, r, g, b);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addGouraudTexturedQuad(u32 *orderingTable, s16 x0, s16 y0, s16 x1, s16 y1,
                  s16 x2, s16 y2, s16 x3, s16 y3, u8 tpage, s16 clut,
                  u8 u0, u8 v0, u8 u1, u8 v1, u8 u2, u8 v2,
                  u8 u3, u8 v3, u8 r0, u8 g0, u8 b0,
                  u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2,
                  u8 r3, u8 g3, u8 b3) {
    POLY_GT4 *poly = (POLY_GT4 *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_GT4_CODE);
    poly->tpage = tpage;
    poly->clut = clut;
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->pad3 = 0;
    poly->x3 = x3;
    poly->y3 = y3;
    poly->pad5 = 0;
    poly->u0 = u0; poly->v0 = v0;
    poly->u1 = u1; poly->v1 = v1;
    poly->u2 = u2; poly->v2 = v2;
    poly->u3 = u3; poly->v3 = v3;
    setRGB0(poly, r0, g0, b0);
    setRGB1(poly, r1, g1, b1);
    poly->pad1 = 0;
    setRGB2(poly, r2, g2, b2);
    poly->pad2 = 0;
    setRGB3(poly, r3, g3, b3);
    poly->pad4 = 0;
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addSemiTransparentFlatQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                                          s32 x0, s32 y0, s32 x1, s32 y1,
                                          s32 x2, s32 y2, s32 x3, s32 y3,
                                          s32 r, s32 g, s32 b) {
    FlatQuadPacket *poly = (FlatQuadPacket *)GsOUT_PACKET_P;
    poly->tag = (sizeof(FlatQuadPacket) / sizeof(u32) - 1) << 24;
    poly->code = POLY_F4_CODE | SEMI_TRANSPARENT_FLAG;
    poly->tpage = tpage & 0xFF;
    poly->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    setXY2(poly, x0, y0, x1, y1);
    poly->x2 = x2;
    poly->y2 = y2;
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r, g, b);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addSemiTransparentGouraudQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                  s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3) {
    GouraudQuadPacket *poly = (GouraudQuadPacket *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    poly->code = POLY_G4_CODE | SEMI_TRANSPARENT_FLAG;
    poly->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    poly->tpage = tpage & 0xFF;
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r0, g0, b0);
    setRGB1(poly, r1, g1, b1);
    setRGB2(poly, r2, g2, b2);
    setRGB3(poly, r3, g3, b3);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addFlatLine(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 r, s32 g, s32 b) {
    LINE_F2 *line = (LINE_F2 *)GsOUT_PACKET_P;

    line->tag = (sizeof(LINE_F2) / sizeof(u32) - 1) << 24;
    setcode(line, 0x40);
    setXY2(line, x0, y0, x1, y1);
    setRGB0(line, r, g, b);
    addPrim(orderingTable, line);
    GsOUT_PACKET_P = (u32)(line + 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060A04);

void addFlatRectangle(u32 *orderingTable, s32 x, s32 y, s32 w, s32 h,
                   s32 r, s32 g, s32 b) {
    TILE *tile = (TILE *)GsOUT_PACKET_P;
    tile->tag = (sizeof(TILE) / sizeof(u32) - 1) << 24;
    setcode(tile, 0x60);
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    addPrim(orderingTable, tile);
    GsOUT_PACKET_P = (u32)(tile + 1);
}

void addFlatRectangleWithDrawMode(u32 *orderingTable, s32 tpage, s32 flags,
                  s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b) {
    FlatRectanglePacket *tile = (FlatRectanglePacket *)GsOUT_PACKET_P;
    tile->tag = (sizeof(FlatRectanglePacket) / sizeof(u32) - 1) << 24;
    tile->code = FLAT_RECTANGLE_CODE | flags;
    tile->tpage = tpage;
    tile->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    addPrim(orderingTable, tile);
    GsOUT_PACKET_P = (u32)(tile + 1);
}

void addTexturedRectangle(u32 *orderingTable, s32 clut, s32 x, s32 y,
                   s32 w, s32 h, s32 r, s32 g, s32 b) {
    SPRT *sprite = (SPRT *)GsOUT_PACKET_P;
    sprite->tag = (sizeof(SPRT) / sizeof(u32) - 1) << 24;
    setcode(sprite, 0x64);
    sprite->clut = clut;
    setXY0(sprite, x, y);
    setWH(sprite, w, h);
    setRGB0(sprite, r, g, b);
    addPrim(orderingTable, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

void addTexturedSprite(u32 *orderingTable, s32 tpage, s32 clut, s32 x,
                   s32 y, s32 w, s32 h, u16 u, u16 v,
                   s32 r, s32 g, s32 b) {
    DrawModeSpritePacket *sprite = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    sprite->tag = (sizeof(*sprite) / sizeof(u32) - 1) << 24;
    sprite->code = 0x64;
    sprite->tpage = tpage;
    sprite->drawModeCode = 0xE100;
    sprite->x0 = x;
    sprite->clut = clut;
    sprite->y0 = y;
    setWH(sprite, w, h);
    sprite->u0 = u;
    sprite->v0 = v;
    setRGB0(sprite, r, g, b);
    addPrim(orderingTable, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

/* No callers are known; only the variadic argument layout is established. */
void unusedVariadicStub(s32 unused0, s32 unused1, s32 unused2, ...) {
}

s32 loadTimArchive(OffsetArchive *archive) {
    u32 i;
    u32 count = archive->count;
    for (i = 1; i < count + 1; i++) {
        if (!loadTimImage((u32 *)((u8 *)archive + archive->offsets[i - 1]))) {
            return 0;
        }
    }
    return 1;
}

s32 loadTimImage(u32 *tim) {
    GsIMAGE image;
    if (*tim != 0x10) {
        return 0;
    }
    GsGetTimInfo(tim + 1, &image);
    LoadImage((RECT *)&image.px, image.pixel);
    DrawSync(0);
    LoadImage((RECT *)&image.cx, image.clut);
    DrawSync(0);
    return 1;
}

s32 randomNextLinear(void) {
    gLinearRandomState = gLinearRandomState * 1103515245 + 12345;
    return ((u32)gLinearRandomState >> 16) & 0x7FFF;
}

s32 randomLinearFromValue(s32 value) {
    value = value * 1103515245 + 12345;
    return (value / 65536) & 0x7FFF;
}

void updatePlayerRenderSnapshots(void) {
    GsCOORDINATE2 coordinate;
    u32 i;
    RacePlayer *player;
    GsInitCoordinate2(0, &coordinate);
    GsGetLs(&coordinate, &gPlayerRenderIdentityMatrix);
    for (i = 0; i < 2; i++) {
        player = &D_80120E90[i];
        gPreviousPlayerRenderSnapshots[i] = gPlayerRenderSnapshots[i];
        gPlayerRenderSnapshots[i].position.vx = player->x >> 8;
        gPlayerRenderSnapshots[i].position.vy = player->y >> 8;
        gPlayerRenderSnapshots[i].position.vz = player->z >> 8;
        gPlayerRenderSnapshots[i].unk10 = player->unk0B4;
        gPlayerRenderSnapshots[i].unk12 = player->unk0B6;
        gPlayerRenderSnapshots[i].unk14 = player->unk0B8;
        gPlayerRenderSnapshots[i].unk18 = player->unk0BE;
    }
}

void startRacePlayerTasks(void) {
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < D_8013AFD2; playerIndex++) {
        func_80064754(playerIndex);
        func_80065464(playerIndex);
        createPlayerTrailTask(playerIndex);
        func_80067D88(playerIndex);
        func_800688C0(playerIndex);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060FE4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800612BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061304);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061348);

void initializePlayerSnapshotTrailTask(void) {
    PlayerSnapshotTrailResource *resource;
    SVECTOR rotation;
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    resource = (PlayerSnapshotTrailResource *)&D_800D9504[D_8013AE88->resource14];
    GsInitCoordinate2(0, &resource->current);
    GsInitCoordinate2(0, &resource->previous);
    rotation.vx = gPlayerRenderSnapshots[D_8013AE88->data1C].unk10;
    rotation.vy = gPlayerRenderSnapshots[D_8013AE88->data1C].unk18 + 0x400;
    rotation.vz = gPlayerRenderSnapshots[D_8013AE88->data1C].unk14;
    resource->currentScale.vx = 0x1000;
    resource->currentScale.vy = 0x1000;
    resource->currentScale.vz = (((s16)gPlayerRenderSnapshots[D_8013AE88->data1C].unk18 &
        (0x3FF - (s16)gPlayerRenderSnapshots[D_8013AE88->data1C].unk12) & 0x3FF) << 2) + 0x1000;
    RotMatrix(&rotation, &resource->current.coord);
    resource->current.coord.t[0] = gPlayerRenderSnapshots[D_8013AE88->data1C].position.vx;
    resource->current.coord.t[1] = gPlayerRenderSnapshots[D_8013AE88->data1C].position.vy;
    resource->current.coord.t[2] = gPlayerRenderSnapshots[D_8013AE88->data1C].position.vz;
    rotation.vx = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk10;
    rotation.vy = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk18 + 0x400;
    rotation.vz = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk14;
    resource->previousScale.vx = 0x1000;
    resource->previousScale.vy = 0x1000;
    resource->previousScale.vz = (((s16)gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk18 &
        (0x3FF - (s16)gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].unk12) & 0x3FF) << 2) + 0x1000;
    RotMatrix(&rotation, &resource->previous.coord);
    resource->previous.coord.t[0] = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].position.vx;
    resource->previous.coord.t[1] = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].position.vy;
    resource->previous.coord.t[2] = gPreviousPlayerRenderSnapshots[D_8013AE88->data1C].position.vz;
    resource->remainingFrames = D_8013AE88->data1A;
    setCurrentTaskCallback(func_80061348);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800618A0);

void startIndexedRenderTask(s32 index) {
    TaskResourceRecord *resource;
    func_80014930(D_8013B304, func_800618A0);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061E48);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80062354);

void startIndexedRenderTaskPair(s32 index) {
    TaskResourceRecord *resource;
    func_80014930(D_8013B304, func_80061E48);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
    func_80014930(D_8013B304, func_80062354);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
}

void drawFallingParticlesTask(void) {
    s32 i;
    FallingParticleResource *resource;
    SPRT_8 *packet;
    SPRT_8 *scratch;
    DR_TPAGE *page;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (SPRT_8 *)0x1F800000;
    resource = (FallingParticleResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (SPRT_8 *)GsOUT_PACKET_P;
    setcode(scratch, SPRT_8_CODE);
    setRGB0(scratch, 128, 128, 128);
    scratch->v0 = 232;
    for (i = 0; i < 40; i++) {
        scratch->x0 = (resource->x[i] + rcos(resource->frame * 32 + i * 256) / 64) % 320 - 160;
        scratch->y0 = resource->y[i] - 120;
        resource->y[i] += 2 + (i & 3);
        if (resource->y[i] >= 240) resource->y[i] = -4;
        scratch->u0 = (((u8)resource->frame + i) & 3) * 8 + 216U;
        scratch->clut = getClut(992, 240 + i % 5);
        scratch->tag = (3 << 24) | getaddr(D_8013AF6C);
        *packet = *scratch;
        setaddr(D_8013AF6C, packet);
        packet++;
    }
    page = (DR_TPAGE *)packet;
    setlen(page, 1);
    page->code[0] = ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(0, 0, 768, 256);
    addPrim(D_8013AF6C, page);
    GsOUT_PACKET_P = (u32)(page + 1);
    resource->frame++;
}

void createFallingParticleTask(void) {
    FallingParticleResource *resource;
    s32 i;
    func_80014930(&gGameTasks[4], drawFallingParticlesTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (FallingParticleResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 40; i++) {
        resource->x[i] = randomNextLinear() % 320;
        resource->y[i] = -(randomNextLinear() % 244);
    }
    resource->frame = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80062EA0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800632C4);

void createPlayerParticleTask(s32 playerIndex) {
    PlayerParticlePosition *position;
    s32 i;
    func_80014930(&gGameTasks[4], func_800632C4);
    D_8013AE8C->resource14 = allocateTaskResource();
    position = (PlayerParticlePosition *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 40; i++) {
        position[i].x = randomNextLinear() % 1024;
        position[i].y = randomNextLinear() % 512;
        position[i].z = randomNextLinear() % 1024;
    }
    D_8013AE8C->data18 = playerIndex;
    D_8013AE8C->data1A = 0;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063890);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063B58);

void createPositionedEffectTask(s32 x, s32 y, s32 z) {
    PositionedEffectResource *resource;

    func_80014874(func_80063B58);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PositionedEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = x >> 8;
    resource->y = y >> 8;
    resource->z = z >> 8;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063DF4);

void startRotatingQuadEffectTask(s32 variant, const VECTOR *position, s32 pitch, s32 yaw, s32 roll) {
    RotatingEffectResource *resource;
    func_80014874(func_80063DF4);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RotatingEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = position->vx >> 8;
    resource->y = position->vy >> 8;
    resource->z = position->vz >> 8;
    resource->rotation.vx = 0;
    resource->rotation.vy = yaw;
    resource->rotation.vz = 0;
    resource->parentRotation.vx = pitch;
    resource->parentRotation.vy = 0;
    resource->parentRotation.vz = roll;
    resource->elapsed = 0;
    resource->clut = variant == 0 ? 0x3EA8 : 0x3EE8;
    resource->textureU = variant == 0 ? 0x40 : 0x80;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80064454);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80064754);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006479C);

void initializeDelayedEightEntryEffectTask(void) {
    EightEntryEffectResource *primary;
    EightEntryEffectSecondaryResource *secondary;
    s32 i;
    if (D_8013AE88->data18 == 0) {
        func_80014930(D_8013B304, func_8006479C);
        D_8013AE8C->resource14 = allocateTaskResource();
        primary = (EightEntryEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        D_8013AE8C->resource16 = allocateTaskResource();
        secondary = (EightEntryEffectSecondaryResource *)&D_800D9504[D_8013AE8C->resource16];
        D_8013AE8C->data18 = D_8013AE88->data1A;
        D_8013AE8C->data1A = 0;
        for (i = 0; i < 8; i++) {
            primary->positions[i].vx = 0;
            primary->positions[i].vy = 0;
            primary->positions[i].vz = 0;
            secondary->vectors[i].vx = 0;
            secondary->vectors[i].vy = 0;
            secondary->vectors[i].vz = 0;
            primary->rotations[i].vx = 0;
            primary->rotations[i].vy = 0;
            primary->rotations[i].vz = 0;
            secondary->values40[i] = 0;
            secondary->values50[i] = 0;
            secondary->values60[i] = 0;
            primary->values[i] = 32;
        }
        primary->elapsed = 0;
        func_800149A8();
    } else {
        D_8013AE88->data18--;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065464);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800654B4);

void createPlayerPairedQuadEffectTask(s32 playerIndex, s32 variant) {
    PairedEffectResource *resource;
    RacePlayer *player;
    s32 i;
    func_80014930(D_8013B304, func_800654B4);
    D_8013AE8C->resource14 = allocateTaskResource();
    player = &D_80120E90[playerIndex];
    resource = (PairedEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 2; i++) {
        gPlayerPairedQuadOffsets[i].vz = randomNextLinear() % 40 - 20;
        ApplyMatrixLV(&player->model.effect.effectMatrix180, &gPlayerPairedQuadOffsets[i], &resource->positions[i]);
        resource->positions[i].vx += player->model.effect.effectMatrix180.t[0];
        resource->positions[i].vy += player->model.effect.effectMatrix180.t[1];
        resource->positions[i].vz += player->model.effect.effectMatrix180.t[2];
        resource->positions[i].vx -= player->worldCoordinate.coord.t[0] + player->unk084;
        resource->positions[i].vy -= player->worldCoordinate.coord.t[1] + player->unk086;
        resource->positions[i].vz -= player->worldCoordinate.coord.t[2] + player->unk088;
    }
    resource->elapsed = 0;
    resource->clut = variant == 0 ? getClut(800, 250) : getClut(800, 251);
    D_8013AE8C->data18 = playerIndex;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800659F0);

GameTask *createQuadAnimationTask(u16 unusedId, VECTOR *position) {
    PrimaryQuadAnimationResource *resource;
    func_80014930(D_8013B304, func_800659F0);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PrimaryQuadAnimationResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = position->vx;
    resource->y = position->vy;
    resource->z = position->vz;
    resource->elapsed = 0;
    resource->frame = 0;
    resource->frames = gQuadAnimationFrames;
    return D_8013AE8C;
}

GameTask *startAlternateQuadAnimationTask(s32 unused, const PositionEffectOrigin *position) {
    QuadAnimationResource *resource;
    GameTask *task;
    func_80014930(D_8013B304, func_800659F0);
    D_8013AE8C->resource14 = allocateTaskResource();
    task = D_8013AE8C;
    resource = (QuadAnimationResource *)&D_800D9504[task->resource14];
    resource->position.x = position->x;
    resource->position.y = position->y;
    resource->position.z = position->z;
    resource->elapsed = 0;
    resource->frame = 0;
    resource->steps = gAlternateQuadAnimationSteps;
    return task;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065EC8);

void emitRadialEffectTask(void) {
    PositionEffectOrigin *origin;
    RadialEffectResource *resource;
    s32 i;
    s32 angle;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    origin = (PositionEffectOrigin *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AFD3 == 0 && (gFrameCounter & 3) == 0) {
        func_80014930(D_8013B304, func_80065EC8);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (RadialEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        GsInitCoordinate2(0, &resource->coordinate);
        resource->coordinate.coord.t[0] = origin->x;
        resource->coordinate.coord.t[1] = origin->y;
        resource->coordinate.coord.t[2] = origin->z;
        for (i = 0, angle = 0; i < 6; i++) {
            resource->positions[i].vx = resource->innerPositions[i].vx = 0;
            resource->positions[i].vz = resource->innerPositions[i].vz = 0;
            resource->positions[i].vy = resource->innerPositions[i].vy = 0;
            resource->velocities[i].vx = rcos(angle) / 512;
            resource->velocities[i].vz = rsin(angle) / 512;
            angle += 0x2AA;
            resource->velocities[i].vy = -32;
        }
        resource->elapsed = 0;
        if (--D_8013AE88->data18 == 0) {
            func_800149A8();
        }
    }
}

void startPositionEffectTask(const PositionEffectOrigin *position) {
    PositionEffectOrigin *resource;
    func_80014930(D_8013B304, emitRadialEffectTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = 3;
    resource = (PositionEffectOrigin *)&D_800D9504[D_8013AE8C->resource14];
    *resource = *position;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80066474);

void createPlayerTrailTask(s32 playerIndex) {
    PlayerTrailResource *first;
    PlayerTrailResource *second;
    s32 i;
    func_80014930(D_8013B304, func_80066474);
    D_8013AE8C->resource14 = allocateTaskResource();
    first = (PlayerTrailResource *)&D_800D9504[D_8013AE8C->resource14];
    D_8013AE8C->resource16 = allocateTaskResource();
    second = (PlayerTrailResource *)&D_800D9504[D_8013AE8C->resource16];
    for (i = 0; i < 16; i++) {
        first->positions[i].x = first->positions[i].y = first->positions[i].z = 0;
        second->positions[i].x = second->positions[i].y = second->positions[i].z = 0;
        second->state[i] = 0;
    }
    first->state[0] = 0;
    D_8013AE8C->data18 = playerIndex;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80066D18);

void createPlayerPositionTrailTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerPositionTrailResource *resource;
    s32 i, j;

    player = &D_80120E90[playerIndex];
    func_80014930(D_8013B304, func_80066D18);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = playerIndex;
    resource = (PlayerPositionTrailResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            resource->positions[i][j] = player->worldCoordinate.coord.t[j];
        }
    }
    resource->unk4E = 1;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800676FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067B30);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067D88);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067DD0);

void createPlayerRingEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerRingEffectResource *resource;
    s32 i;
    player = &D_80120E90[playerIndex];
    if (D_8013AFCE != 3) {
        func_80014930(D_8013B304, func_80067DD0);
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (PlayerRingEffectResource *)&D_800D9504[D_8013AE8C->resource14];
        for (i = 0; i < 3; i++) {
            resource->position[i] = player->worldCoordinate.coord.t[i];
        }
        resource->position[1] = func_8004A764(player->unk92E, resource->position[0], resource->position[2]);
        for (i = 0; i < 6; i++) {
            resource->rotations[i].vy = 0;
            resource->rotations[i].vx = 0;
            resource->rotations[i].vz = randomNextLinear() % 4096;
        }
        resource->elapsed = 0;
        D_8013AE8C->data18 = playerIndex;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006836C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800688C0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800689EC);

s32 createPlayerTripleQuadEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i;
    s32 state;
    player = &D_80120E90[playerIndex];
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)func_80059254())) {
        return 0;
    }
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) {
        return 0;
    }
    func_80014930(D_8013B304, func_800689EC);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PlayerTripleQuadEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    vertex = gPlayerTripleQuadOffsets;
    for (i = 0; i < 3; i++) {
        ApplyMatrix(&player->model.effect.effectMatrix180, vertex, (VECTOR *)&resource->positions[i]);
        resource->positions[i].x += player->worldCoordinate.coord.t[0];
        resource->positions[i].y += player->worldCoordinate.coord.t[1];
        resource->positions[i].z += player->worldCoordinate.coord.t[2];
        vertex++;
    }
    resource->elapsed = 0;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = randomNextLinear() % 4096;
    state = player->unk96E;
    resource->state = state;
    resource->scale = 0;
    D_8013AE8C->data18 = playerIndex;
    return state;
}

void drawRisingTexturedStripTask(void) {
    POLY_FT4 *packet;
    RotatingQuadTrailResource *resource;
    s32 y;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RotatingQuadTrailResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    packet->tag = 9 << 24;
    setcode(packet, POLY_FT4_CODE);
    packet->clut = getClut(0x3B0, 0xF0);
    packet->tpage = getTPage(0, 0, 0x3C0, 0);
    packet->u0 = 0;
    packet->v0 = 0;
    packet->v1 = 0;
    packet->u2 = 0;
    packet->v2 = 15;
    packet->u1 = 215;
    packet->u3 = 215;
    packet->v3 = 15;
    packet->x0 = -108;
    packet->x1 = 108;
    packet->x2 = -108;
    packet->x3 = 108;
    y = -8 - resource->countdown * 8;
    setRGB0(packet, 128, 128, 128);
    packet->y1 = y;
    packet->y0 = y;
    packet->y3 = y + 15;
    packet->y2 = y + 15;
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (D_8013B338 >= 50) {
        if (++resource->countdown > 10) {
            resource->countdown = 10;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80068D1C);

void createRotatingQuadTrailTask(void) {
    RotatingQuadTrailResource *resource;
    s32 i;

    func_80014930(&gGameTasks[4], func_80068D1C);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (RotatingQuadTrailResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 30;
    for (i = 0; i < 8; i++) {
        resource->angles[i] = resource->countdown * 256;
        resource->scales[i] = resource->countdown * 400 + 0x1000;
    }
}

void drawLoweringTexturedQuadTask(void) {
    POLY_FT4 *packet;
    ShrinkingQuadResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    resource = (ShrinkingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->x0 = resource->vertices[0].x;
    packet->y0 = resource->vertices[0].y + (resource->countdown << 6) / 10;
    packet->x1 = resource->vertices[1].x;
    packet->y1 = resource->vertices[1].y + (resource->countdown << 6) / 10;
    packet->x2 = resource->vertices[2].x;
    packet->y2 = resource->vertices[2].y + (resource->countdown << 6) / 10;
    packet->x3 = resource->vertices[3].x;
    packet->y3 = resource->vertices[3].y + (resource->countdown << 6) / 10;
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (D_8013B338 >= 50) {
        if (++resource->countdown > 10) { resource->countdown = 10; }
    }
}

void drawShrinkingTexturedQuadTask(void) {
    s32 i;
    POLY_FT4 *packet;
    QuadTransitionResource *resource;
    QuadScaleScratchpad *scratch;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    scratch = (QuadScaleScratchpad *)0x1F800000;
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch->rotation.vx = scratch->rotation.vy = scratch->rotation.vz = 0;
    scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = resource->frames * 1000 + 0x1000;
    RotMatrix((SVECTOR *)0x1F800020, &scratch->matrix);
    ScaleMatrix(&scratch->matrix, (VECTOR *)0x1F800028);
    for (i = 0; i < 4; i++) {
        ApplyMatrixSV(&scratch->matrix, &resource->vertices[i], &scratch->vertices[i]);
    }
    *packet = *resource->packet;
    packet->x0 = scratch->vertices[0].vx;
    packet->y0 = scratch->vertices[0].vy;
    packet->x1 = scratch->vertices[1].vx;
    packet->y1 = scratch->vertices[1].vy;
    packet->x2 = scratch->vertices[2].vx;
    packet->y2 = scratch->vertices[2].vy;
    packet->x3 = scratch->vertices[3].vx;
    packet->y3 = scratch->vertices[3].vy;
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (--resource->frames <= 0) {
        setCurrentTaskCallback(drawLoweringTexturedQuadTask);
        resource->frames = 0;
    }
}

void createQuadShrinkTask(void) {
    QuadTransitionResource *resource;

    func_80014930(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->frames = 5;
    resource->vertices = gQuadShrinkVertices;
    resource->packet = &gQuadShrinkPacket;
}

void startMiddleQuadShrinkTask(void) {
    ShrinkingQuadResource *resource;

    func_80014930(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ShrinkingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 5;
    resource->vertices = gMiddleShrinkQuadVertices;
    resource->packet = &gMiddleShrinkQuadPacket;
}

void createRightQuadShrinkTask(void) {
    QuadTransitionResource *resource;

    func_80014930(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->frames = 5;
    resource->vertices = gRightQuadShrinkVertices;
    resource->packet = &gRightQuadShrinkPacket;
}

void drawPaletteCyclingStripTask(void) {
    POLY_FT4 *packet;
    PaletteStripResource *resource;
    s32 paletteRow;
    s32 paletteOffset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PaletteStripResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    packet->x0 = packet->x2 = -16;
    packet->x1 = packet->x3 = 143;
    packet->y0 = packet->y1 = 88;
    packet->y2 = packet->y3 = 103;
    paletteOffset = D_8013AE88->data18 * 3;
    paletteRow = resource->frame % 3 + 242;
    packet->clut = getClut(944, paletteOffset + paletteRow);
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    resource->frame++;
}

void drawLoweringQuadStripTask(void) {
    POLY_FT4 *packet;
    SlidingQuadResource *resource;
    s32 offset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    packet->x0 = packet->x2 = -16;
    packet->x1 = packet->x3 = 143;
    offset = (resource->countdown << 6) / 10;
    packet->y0 = packet->y1 = offset + 24;
    packet->y2 = packet->y3 = offset + 39;
    packet->clut = getClut(944, D_8013AE88->data18 * 3 + 242);
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (D_8013B338 >= 50) {
        if (++resource->countdown > 10) {
            setCurrentTaskCallback(drawPaletteCyclingStripTask);
            resource->countdown = 0;
        }
    }
}

void drawSlidingQuadTask(void) {
    POLY_FT4 *packet;
    SlidingQuadResource *resource;
    s32 offset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    offset = resource->countdown * resource->countdown;
    packet->x0 = packet->x2 = offset - 16;
    packet->x1 = packet->x3 = offset + 143;
    packet->clut = getClut(944, D_8013AE88->data18 * 3 + 242);
    addPrim(D_8013AF6C, packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
    if (--resource->countdown <= 0) {
        setCurrentTaskCallback(drawLoweringQuadStripTask);
        resource->countdown = 0;
    }
}

void startSlidingQuadTask(s32 index) {
    SlidingQuadResource *resource;
    func_80014930(&gGameTasks[4], drawSlidingQuadTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 10;
    resource->packet = &gSlidingQuadPacket;
    resource->textureV = (index << 4) + 32;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069BAC);

void updatePaletteWhitePulse(void) {
    s32 amount, i;
    u8 r, g, b;

    if (D_8013AE88->data18 < 10) {
        amount = D_8013AE88->data18;
    } else if (D_8013AE88->data18 < 20) {
        amount = 20 - D_8013AE88->data18;
    } else {
        amount = 0;
    }
    for (i = 0; i < 16; i++) {
        r = (gCapturedColorCyclePalette[i].bytes[0] & 31);
        g = ((gCapturedColorCyclePalette[i].value >> 5) & 31);
        b = ((gCapturedColorCyclePalette[i].value >> 10) & 31);
        if (i != 0) {
            r += (31 - r) * amount / 10;
            g += (31 - g) * amount / 10;
            b += (31 - b) * amount / 10;
        }
        gWhitePulsePalette[i] = (u8)r | ((u8)g << 5) | ((u8)b << 10);
    }
    LoadImage(&gWhitePulsePaletteRect, (u32 *)gWhitePulsePalette);
    DrawSync(0);
    D_8013AE88->data18 = (D_8013AE88->data18 + 1) % 60;
}

void capturePaletteForColorCycle(void) {
    StoreImage(&gColorCycleCaptureRect, (u32 *)gCapturedColorCyclePalette);
    DrawSync(0);
    setCurrentTaskCallback(updatePaletteWhitePulse);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069E08);

void updateQuadTransitionSequenceTask(void) {
    s32 frame = D_8013B338;
    if (frame == 0) createRotatingQuadTrailTask();
    if (frame == 25) createQuadShrinkTask();
    if (frame == 30) startMiddleQuadShrinkTask();
    if (frame == 35) createRightQuadShrinkTask();
    if (frame == 40) startSlidingQuadTask(D_8013AE88->data1A);
    if (frame == 50) func_80069E08(10);
    D_8013B338++;
    if (D_8013B338 == 100) func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069F1C);

void updatePrimaryValueTransition(void) {
    s32 value;
    value = D_8013AE88->data1C +
        (D_8013AE88->data1E - D_8013AE88->data1C) *
        D_8013AE88->data18 / D_8013AE88->data1A;
    D_8013B35C = value;
    D_8013AE88->data18++;
    D_8013B364 = 0;
    if (D_8013AE88->data18 >= D_8013AE88->data1A) {
        func_800149A8();
        D_8013B364 = 1;
    }
}

void startPrimaryValueTransition(s32 duration, s32 start, s32 end) {
    func_80014930(D_8013AE88, updatePrimaryValueTransition);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
    D_8013AE8C->data1C = start;
    D_8013AE8C->data1E = end;
}

void updateValueInterpolationTask(void) {
    s32 step;
    s32 start = D_8013AE88->data1C;
    step = (D_8013AE88->data1E - start) * D_8013AE88->data18;
    step /= D_8013AE88->data1A;
    D_8013B360 = start + step;
    D_8013AE88->data18++;
    D_8013B364 = 0;
    if (D_8013AE88->data18 >= D_8013AE88->data1A) {
        func_800149A8();
        D_8013B364 = 1;
    }
}

void startValueInterpolationTask(s32 duration, s32 start, s32 end) {
    func_80014930(D_8013AE88, updateValueInterpolationTask);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
    D_8013AE8C->data1C = start;
    D_8013AE8C->data1E = end;
}

void drawFullScreenSpriteTask(void) {
    ScreenSpriteResource *resource;
    DrawModeSpritePacket *packets;
    s32 i;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    packets = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    resource = (ScreenSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    for (i = 0; i < 2; i++) {
        resource->sprites[i].r0 = resource->sprites[i].g0 = resource->sprites[i].b0 = *(const u8 *)resource->brightness;
    }
    for (i = 0; i < 2; i++) {
        packets[i] = resource->sprites[i];
        addPrim(&D_8013AF6C[1], &packets[i]);
    }
    GsOUT_PACKET_P = (u32)(packets + 2);
    if (++resource->elapsed >= resource->duration || (D_8013AFE4 & 0x100)) {
        func_800149A8();
    }
}

void createFullScreenSpriteTask(s32 duration, s32 tpage, u16 clut, u16 u, u16 v,
                  const s32 *brightness) {
    ScreenSpriteResource *resource;
    s32 i;

    func_80014930(&gGameTasks[4], drawFullScreenSpriteTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ScreenSpriteResource *)&D_800D9504[D_8013AE8C->resource14];
    for (i = 0; i < 2; i++) {
        resource->sprites[i].tag = (sizeof(DrawModeSpritePacket) / sizeof(u32) - 1) << 24;
        resource->sprites[i].code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
        resource->sprites[i].drawModeCode = DRAW_MODE_COMMAND_HIGH;
        resource->sprites[i].tpage = tpage | getTPage(0, 1, 0, 0);
        tpage += 2;
        resource->sprites[i].clut = clut;
        resource->sprites[i].r0 = resource->sprites[i].g0 = resource->sprites[i].b0 = 0x60;
        resource->sprites[i].u0 = u;
        resource->sprites[i].v0 = v;
        resource->sprites[i].h = 240;
        resource->sprites[i].y0 = -120;
    }
    resource->sprites[0].w = 256;
    resource->sprites[1].w = 64;
    resource->sprites[0].x0 = -160;
    resource->sprites[1].x0 = 96;
    resource->elapsed = 0;
    resource->duration = duration;
    resource->brightness = brightness;
}

void *getOffsetArchiveEntry(OffsetArchive *archive, s32 index) {
    u32 *offsets = archive->offsets;
    return (u8 *)archive + offsets[index];
}

void uploadTimImageAndPaletteAt(u32 *tim, s32 x, s32 y, s32 clutX, s32 clutY) {
    GsIMAGE image;
    RECT rect;

    GsGetTimInfo(tim + 1, &image);
    rect.x = x;
    rect.y = y;
    rect.w = image.pw;
    rect.h = image.ph;
    LoadImage(&rect, image.pixel);
    rect.x = clutX;
    rect.y = clutY;
    rect.w = image.cw;
    rect.h = image.ch;
    LoadImage(&rect, image.clut);
    DrawSync(0);
}

void drawFadingText(FadingTextResource *resource) {
    FadingTextGlyphBanks tables = gFadingTextGlyphBanks;
    DrawModeSpritePacket *scratch = (DrawModeSpritePacket *)0x1F800000, *packet;
    const s8 *text, *glyph;
    s32 table, index;
    scratch->tag = (sizeof(*scratch) / 4 - 1) << 24;
    scratch->code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
    scratch->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    scratch->clut = resource->clut;
    scratch->x0 = resource->x;
    scratch->y0 = resource->y;
    scratch->r0 = scratch->g0 = scratch->b0 = *(u8 *)&resource->brightness;
    scratch->w = 16;
    scratch->h = 16;
    text = (const s8 *)resource->text;
    packet = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    while (*text) {
        for (table = 0; table < 4; table++) {
            glyph = tables.entries[table];
            for (index = 0; glyph[index]; index += 2) {
                if (glyph[index] == text[0] && glyph[index + 1] == text[1]) {
                    scratch->u0 = ((index >> 1) & 15) << 4;
                    scratch->v0 = (index >> 1) & ~15;
                    switch (table) {
                        case 0:
                            scratch->tpage = getTPage(0, 0, 960, 256);
                            scratch->v0 += 160;
                            break;
                        case 1:
                            scratch->tpage = getTPage(0, 0, 896, 256);
                            break;
                        case 2:
                            scratch->tpage = getTPage(0, 0, 768, 256);
                            break;
                        case 3:
                            scratch->tpage = getTPage(0, 0, 704, 256);
                            break;
                    }
                    *packet = *scratch;
                    packet->tpage |= getTPage(0, 1, 0, 0);
                    addPrim(&D_8013AF6C[0], packet);
                    packet++;
                    *packet = *scratch;
                    packet->clut = getClut(768, 248);
                    packet->tpage |= getTPage(0, 2, 0, 0);
                    addPrim(&D_8013AF6C[0], packet);
                    packet++;
                    goto nextCharacter;
                }
            }
        }
    nextCharacter:
        scratch->x0 += 16;
        text += 2;
    }
    GsOUT_PACKET_P = (u32)packet;
}

void updateFadingTextTask(void) {
    FadingTextResource *resource;
    s32 remaining;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FadingTextResource *)&D_800D9504[D_8013AE88->resource14];
    setSpriteColor(255, 255, 255);
    remaining = resource->elapsed;
    if (remaining < 30 || (remaining = resource->duration - remaining) < 30) {
        resource->brightness = remaining * 255 / 30;
    } else {
        resource->brightness = 255;
    }
    drawFadingText(resource);
    D_8013B368 = 0;
    if (++resource->elapsed == resource->duration) {
        func_800149A8();
        D_8013B368 = 1;
    }
}

void createFadingTextTask(s32 duration, s32 x, s32 y, const char *text, s32 palette) {
    FadingTextResource *resource;
    func_80014930(D_8013AE88, updateFadingTextTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (FadingTextResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->duration = duration;
    resource->elapsed = 0;
    resource->x = x;
    resource->y = y;
    resource->text = text;
    resource->clut = getClut(768, palette + 240);
}

void finishTimedSpriteSequence(void) {
    TimedSpriteSequenceResource *resource;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    if (--resource->duration < 0) {
        func_800149A8();
    }
}

void updateTimedSpriteSequence(void) {
    TimedSpriteSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    if (resource->frame >= resource->step->frame) {
        createFadingTextTask(resource->step->duration, resource->step->x,
            resource->step->y, resource->step->text, resource->step->palette);
        resource->duration = resource->step->duration;
        resource->step++;
    }
    resource->frame++;
    if (resource->step->frame == -1) {
        setCurrentTaskCallback(finishTimedSpriteSequence);
    }
}

void startTimedSpriteSequence(const TimedSpriteStep *steps) {
    TimedSpriteSequenceResource *resource;
    func_80014930(D_8013AE88, updateTimedSpriteSequence);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->step = steps;
    resource->frame = 0;
}

void finishTimedSpritePresentation(void) {
    if (D_8013AE88->data18++ >= 30) {
        startCdVolumeFade(103, 20, 0, 1);
        D_8013AFE4 |= 0x100;
        func_800149E0();
        func_800149A8();
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006AB8C);

void initializeTimedSpritePresentation(void) {
    setSecondaryDisplayTaskMode(0);
    queueCdCommand(6, 1);
    D_8013B35C = 0;
    D_8013B360 = 0;
    D_8013B364 = 0;
    D_8013B368 = 0;
    D_8013B36C = 0;
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(func_8006AB8C);
}

void loadTimedSpritePresentationResources(void) {
    setSecondaryDisplayTaskMode(1);
    queueCdSectorCommand(1, 0xC56, 0x11, (void *)0x80180000, 0);
    queueCdSectorCommand(1, 0xC67, 0x11, (void *)0x80180000, 0);
    queueCdSectorCommand(1, 0x12E5, 0x26, (void *)0x80180000, 0);
    queueCdSectorCommand(1, 0x130B, 0x26, (void *)0x80180000, 0);
    DrawSync(0);
    queueCdSectorCommand(0, 0x1EF, 0xE3, (void *)0x80180000, 0);
    setCurrentTaskCallback(initializeTimedSpritePresentation);
    D_8013AE88->unk10 |= 2;
}

void createResourceLoadTask(void) {
    func_80014930(&gGameTasks[4], loadTimedSpritePresentationResources);
}

void addTextCharacter(s32 x, s32 y, u32 character) {
    SPRT_8 *sprite = (SPRT_8 *)GsOUT_PACKET_P;

    setcode(sprite, SPRT_8_CODE);
    setRGB0(sprite, 255, 255, 255);
    sprite->u0 = (character & 31) << 3;
    sprite->v0 = (character >> 3) & 24;
    setXY0(sprite, x, y);
    sprite->tag = (sizeof(SPRT_8) / sizeof(u32) - 1) << 24;
    sprite->clut = getClut(0x300, D_8013B376 + 0xF0);
    addPrim(D_8013AF6C, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

void addStretchedTextCharacter(s32 x, s32 y, s32 width, u32 character) {
    POLY_FT4 *quad;
    width = x + width - 1;
    quad = (POLY_FT4 *)GsOUT_PACKET_P;
    quad->tag = (sizeof(POLY_FT4) / sizeof(u32) - 1) << 24;
    setcode(quad, POLY_FT4_CODE);
    quad->clut = getClut(768, D_8013B376 + 240);
    quad->tpage = getTPage(0, 0, 960, 256);
    setRGB0(quad, 255, 255, 255);
    quad->y0 = y;
    quad->y1 = y;
    quad->y2 = y + 7;
    quad->y3 = y + 7;
    quad->x1 = width;
    quad->x3 = width;
    quad->u0 = (character & 31) << 3;
    quad->v0 = (character >> 3) & 24;
    quad->x0 = x;
    quad->x2 = x;
    quad->u1 = quad->u0 + 7;
    quad->v1 = quad->v0;
    quad->u2 = quad->u0;
    quad->v2 = quad->v0 + 7;
    quad->u3 = quad->u0 + 7;
    quad->v3 = quad->v0 + 7;
    addPrim(D_8013AF6C, quad);
    GsOUT_PACKET_P = (u32)(quad + 1);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B4A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B9EC);

void resetControllerEffects(void) {
    s32 i;
    for (i = 0; i < 2; i++) {
        D_8013AF08[i] = -1;
        gControllerEffectStates[i].effect = 0;
        gControllerEffectStates[i].priority = 0;
        gControllerEffectStates[i].step = 0;
        gControllerEffectStates[i].elapsed = 0;
        gControllerEffectStates[i].stepEnd = 0;
        gControllerEffectStates[i].effectEnd = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006BA4C);

void updateLegacyControllerActuators(u16 controller) {
    if (D_8013AFD3) {
        D_8013AF00[controller].control = 0x40;
        D_8013AF00[controller].value = 0;
    } else if (D_8013AEF0[controller] >= 25) {
        D_8013AF00[controller].control = 0x40;
        D_8013AF00[controller].value = 1;
    } else {
        D_8013AF00[controller].control = 0x40;
        D_8013AF00[controller].value = 0;
    }
}

void initPrimitiveTemplates(void) {
    initTexturedQuadTemplate();
    initSpritePrimitiveTemplate();
}

void initTexturedQuadTemplate(void) {
    SetPolyFT4((P_TAG *)&gTexturedQuadTemplate);
    setTexturedQuadTexture(0, 0);
    setRGB0(&gTexturedQuadTemplate, 0x80, 0x80, 0x80);
}

void setTexturedQuadTexture(s32 index, s32 palette) {
    TextureRegionSet *texture = &D_8012DDA4[index];
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    quad->tpage = GetTPage(texture->depth, 1, texture->x, texture->y);
    quad->clut = GetClut(texture->clutX, texture->clutY + palette);
    D_8013B6AC = texture->regions;
}

void setTexturedQuadColor(s32 r, s32 g, s32 b) {
    setRGB0(&gTexturedQuadTemplate, r, g, b);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006BEEC);

void drawTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region = &D_8013B6AC[index];
    quad->x0 = x;
    quad->y0 = y;
    quad->x1 = x + region->w;
    quad->y1 = y;
    quad->x2 = x;
    quad->y2 = y + region->h;
    quad->x3 = x + region->w;
    quad->y3 = y + region->h;
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = region->u + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = region->v + region->h;
    quad->u3 = region->u + region->w;
    quad->v3 = region->v + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawStretchedTexturedQuad(s16 x, s16 y, s16 width, s16 height, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region;
    s16 right;
    quad->y0 = y;
    quad->y1 = y;
    y += height;
    quad->y2 = y;
    quad->y3 = y;
    right = x + width;
    quad->x0 = x;
    quad->x1 = right;
    quad->x2 = x;
    quad->x3 = right;
    region = &D_8013B6AC[index];
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = region->u + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = region->v + region->h;
    quad->u3 = region->u + region->w;
    quad->v3 = region->v + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame = &D_8012DFF4[index];

    setTexturedQuadTexture(frame->textureIndex, 0);
    drawTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

void drawTexturedQuadByIndexWithBlend(s16 x, s16 y, s32 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame;
    s32 semiTrans;
    s32 blend;

    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    frame = &D_8012DFF4[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    func_8006BEEC(semiTrans);
    drawTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

void drawStretchedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame;
    TextureRegion *region;
    s32 semiTrans;
    s32 blend;
    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    frame = &D_8012DFF4[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    func_8006BEEC(semiTrans);
    region = &D_8013B6AC[frame->frameIndex];
    if (width == 0) width = region->w;
    if (height == 0) height = region->h;
    drawStretchedTexturedQuad(x, y, width, height, frame->frameIndex, ot, depth);
}

void drawTexturedQuadList(s16 x, s16 y, TexturedQuadListEntry *entry, s16 blend, GsOT *ot, u16 depth) {
    while (entry->palette < 255) {
        drawStretchedTexturedQuadByIndexWithBlend(x + entry->x, y + entry->y, entry->width, entry->height,
                     entry->quadIndex, entry->palette, blend, ot, depth);
        entry++;
    }
}

void drawSkewedTexturedQuadByIndexWithBlend(s16 x, s16 y, s16 width, s16 height, s16 unused, s16 index, s16 palette, s16 blendMode, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame;
    TextureRegion *region;
    POLY_FT4 *quad;
    s32 semiTrans;
    s32 blend;
    if (blendMode >= 0) {
        blend = blendMode;
        semiTrans = 1;
    } else {
        blend = 0;
        semiTrans = 0;
    }
    frame = &D_8012DFF4[index];
    setTexturedQuadTextureBlend(frame->textureIndex, palette, blend);
    func_8006BEEC(semiTrans);
    region = &D_8013B6AC[frame->frameIndex];
    if (width == 0) width = region->w;
    if (height == 0) height = region->h;
    quad = &gTexturedQuadTemplate;
    /* Start with a rectangle, then skew its corners by eight pixels. */
    setXY0(quad, x, y);
    quad->x1 = x + width;
    quad->y1 = y;
    quad->x2 = x;
    quad->y2 = y + height;
    quad->x3 = x + width;
    quad->y3 = y + height;
    quad->x0 = x - 8;
    quad->y0 = y + 8;
    quad->x1 = x + width - 8;
    quad->y1 = y - 8;
    quad->x2 = x + 8;
    quad->y2 = y + height + 8;
    quad->x3 = x + width + 8;
    quad->y3 = y + height - 8;
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = region->u + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = region->v + region->h;
    quad->u3 = region->u + region->w;
    quad->v3 = region->v + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawInclusiveTexturedQuad(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    TextureRegion *region = &D_8013B6AC[index];
    u16 extent;

    /* Inclusive edges use a wrapping 16-bit decrement of each dimension. */
    quad->x0 = x;
    quad->y0 = y;
    extent = region->w + (u16)-1;
    quad->x1 = x + extent;
    quad->y1 = y;
    quad->x2 = x;
    extent = region->h + (u16)-1;
    quad->y2 = y + extent;
    extent = region->w + (u16)-1;
    quad->x3 = x + extent;
    extent = region->h + (u16)-1;
    quad->y3 = y + extent;
    quad->u0 = region->u;
    quad->v0 = region->v;
    quad->u1 = (u8)(region->u - 1) + region->w;
    quad->v1 = region->v;
    quad->u2 = region->u;
    quad->v2 = (u8)(region->v - 1) + region->h;
    quad->u3 = (u8)(region->u - 1) + region->w;
    quad->v3 = (u8)(region->v - 1) + region->h;
    GsSortPoly(quad, ot, depth);
}

void drawInclusiveTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame = &D_8012DFF4[index];

    setTexturedQuadTexture(frame->textureIndex, 0);
    drawInclusiveTexturedQuad(x, y, frame->frameIndex, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C798);

void initSpritePrimitiveTemplate(void) {
    SPRT *sprite = &gSpritePrimitiveTemplate.sprite;
    SetSprt((P_TAG *)sprite);
    SetDrawTPage(&gSpritePrimitiveTemplate.drawPage, 1, 1, GetTPage(0, 0, 0, 0));
    setRGB0(sprite, 0x80, 0x80, 0x80);
}

void setSpriteTextureBlend(s32 index, s32 palette, s32 blend) {
    TextureRegionSet *texture = &D_8012DDA4[index];
    SpritePrimitiveTemplate *sprite = &gSpritePrimitiveTemplate;
    SetDrawTPage(&sprite->drawPage, 1, 1,
                 GetTPage(texture->depth, blend, texture->x, texture->y));
    sprite->sprite.clut = GetClut(texture->clutX, texture->clutY + palette);
    D_8013B6AC = texture->regions;
}

void setSpriteColor(s32 r, s32 g, s32 b) {
    setRGB0(&gSpriteTemplate, r, g, b);
    setRGB0(&gTexturedSpriteTemplate, r, g, b);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C970);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C998);

void drawSpriteTextureRegion(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    SPRT *sprite = (SPRT *)GsGetWorkBase();
    DR_TPAGE *page;
    TextureRegion *region;
    *sprite = gSpritePrimitiveTemplate.sprite;
    region = &D_8013B6AC[index];
    sprite->x0 = x;
    sprite->y0 = y;
    sprite->u0 = region->u;
    sprite->v0 = region->v;
    sprite->w = region->w;
    sprite->h = region->h;
    AddPrim(&ot->org[depth], sprite++);
    page = (DR_TPAGE *)sprite;
    *page = gSpritePrimitiveTemplate.drawPage;
    AddPrim(&ot->org[depth], page);
    func_800BF2C8((u32)(page + 1));
}

void addSpritePacket(SPRT *source, GsOT *ot, u16 depth) {
    SPRT *packet = (SPRT *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

void addDrawPagePacket(DR_TPAGE *source, GsOT *ot, u16 depth) {
    DR_TPAGE *packet = (DR_TPAGE *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

void drawRotatedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex, s16 blendMode, s16 angle, GsOT *ot, u16 depth) {
    s32 palette = paletteIndex;
    s32 blend;
    s32 transparent;
    TexturedQuadFrame *frame;
    TextureRegionSet *texture;
    TextureRegion *region;
    GsSPRITE *sprite;
    if (blendMode >= 0) {
        blend = blendMode;
        transparent = 1;
    } else {
        blend = 0;
        transparent = 0;
    }
    frame = &D_8012DFF4[index];
    texture = &D_8012DDA4[frame->textureIndex];
    region = &texture->regions[frame->frameIndex];
    sprite = &gRotatedSpriteTemplate;
    sprite->attribute = 0;
    if (transparent == 1) {
        sprite->attribute = 0x40000000;
        sprite->attribute |= blend << 28;
    }
    sprite->x = x;
    sprite->y = y;
    sprite->w = region->w;
    sprite->h = region->h;
    sprite->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    sprite->u = region->u;
    sprite->v = region->v;
    sprite->cx = texture->clutX;
    sprite->cy = texture->clutY + palette;
    sprite->r = gTexturedSpriteTemplate.r0;
    sprite->g = gTexturedSpriteTemplate.g0;
    sprite->b = gTexturedSpriteTemplate.b0;
    sprite->mx = region->w / 2;
    sprite->my = region->h / 2;
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
    sprite->rotate = -(angle << 12);
    GsSortSprite(sprite, ot, depth);
}

void drawTransformedSpriteByIndex(s32 x, s32 y, s32 index, s32 paletteIndex, s16 blendMode, s16 pivotX, s16 pivotY, s16 scaleX, s16 scaleY, s32 rotation, GsOT *ot, u16 depth) {
    s32 palette = paletteIndex;
    s32 blend;
    s32 transparent;
    TexturedQuadFrame *frame;
    TextureRegionSet *texture;
    TextureRegion *region;
    GsSPRITE *sprite;
    if (blendMode >= 0) {
        blend = blendMode;
        transparent = 1;
    } else {
        blend = 0;
        transparent = 0;
    }
    sprite = &gRotatedSpriteTemplate;
    frame = &D_8012DFF4[index];
    texture = &D_8012DDA4[frame->textureIndex];
    region = &texture->regions[frame->frameIndex];
    sprite->attribute = 0;
    sprite->attribute |= texture->depth << 24;
    if (transparent == 1) {
        sprite->attribute |= 0x40000000;
        sprite->attribute |= blend << 28;
    }
    sprite->x = x;
    sprite->y = y;
    sprite->w = region->w;
    sprite->h = region->h;
    sprite->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    sprite->u = region->u;
    sprite->v = region->v;
    sprite->cx = texture->clutX;
    sprite->cy = texture->clutY + palette;
    sprite->r = gTexturedSpriteTemplate.r0;
    sprite->g = gTexturedSpriteTemplate.g0;
    sprite->b = gTexturedSpriteTemplate.b0;
    if (pivotX == 0x7FFF) {
        sprite->mx = region->w / 2;
    } else {
        sprite->mx = pivotX;
    }
    if (pivotY == 0x7FFF) {
        sprite->my = region->h / 2;
    } else {
        sprite->my = pivotY;
    }
    sprite->scalex = scaleX;
    sprite->scaley = scaleY;
    sprite->rotate = rotation;
    GsSortSprite(sprite, ot, depth);
}

void drawWrappedText8x8(s16 x, s16 y, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 currentX = x;
    s16 currentY = y;
    s16 count = 0;
    s16 defaultPalette = palette;
    while (*(const s8 *)text) {
        if (count >= columns) {
            currentX = x;
            currentY += 8;
            count = 0;
        }
        if ((s8)*text == '\n') {
            count = columns;
            text++;
        } else {
            if ((s8)*text == 'c') {
                if ((u32)((u8)text[1] - '0') < 10) {
                    palette = (s8)text[1] - '0';
                } else {
                    palette = defaultPalette;
                }
            } else {
                func_8006D29C(currentX, currentY, 8, 8, text, palette, blend, ot, depth);
                currentX += 8;
                count++;
            }
            text += 2;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D29C);

void drawLimitedText8x8(s16 x, s16 y, s16 limit, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 count = 0;
    s16 startY = y;
    while (*(const s8 *)text && count < limit) {
        if ((s8)*text == '\n') {
            count = limit;
        } else {
            drawSizedTextGlyph(x, startY, 8, 8, text, palette, blend, ot, depth);
            x += 8;
            count++;
        }
        text++;
    }
}

void drawSizedTextGlyph(s16 x, s16 y, s16 width, s16 height, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s16 u, v;
    s32 blendMode;
    SPRT *sprite;
    if (blend >= 0) {
        blendMode = blend;
        semiTrans = 1;
    } else {
        blendMode = 0;
        semiTrans = 0;
    }
    u = (((s8)*text - 32) % 32) * 8;
    v = (((s8)*text - 32) / 32) * 8;
    if (u >= 0 && v >= 0) {
        setSpriteTextureBlend(0, palette, blendMode);
        func_8006C970(semiTrans);
        sprite = &gSpriteTemplate;
        sprite->x0 = x;
        sprite->y0 = y;
        sprite->u0 = u;
        sprite->v0 = v;
        sprite->w = width;
        sprite->h = height;
        addSpritePacket(sprite, ot, depth);
        addDrawPagePacket(&gSpritePrimitiveTemplate.drawPage, ot, depth);
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D66C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D984);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006DC94);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006DFB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006E2C4);

void drawHalfStepSpriteBar(s16 x, s16 y, s16 count, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 semiTrans;
    s32 blendMode;
    SpritePrimitiveTemplate *template;
    SPRT *packet;
    DR_TPAGE *page;
    s32 i;
    if (count > 0) {
        if (blend >= 0) {
            blendMode = blend;
            semiTrans = 1;
        } else {
            blendMode = 0;
            semiTrans = 0;
        }
        template = &gSpritePrimitiveTemplate;
        SetDrawTPage(&template->drawPage, 1, 1, GetTPage(0, blendMode, 0x340, 0x100));
        template->sprite.clut = GetClut(0x320, palette + 0xF0);
        SetSemiTrans(&template->sprite, semiTrans);
        template->sprite.w = 8;
        template->sprite.h = 8;
        template->sprite.x0 = x;
        template->sprite.y0 = y;
        template->sprite.u0 = 0x70;
        template->sprite.v0 = 0;
        packet = (SPRT *)GsGetWorkBase();
        for (i = 0; i < count / 2; i++) {
            *packet = template->sprite;
            AddPrim(&ot->org[depth], packet++);
            template->sprite.x0 += 8;
        }
        if ((s16)(count % 2) > 0) {
            *packet = template->sprite;
            packet->u0 = 0x78;
            AddPrim(&ot->org[depth], packet);
            packet++;
        }
        page = (DR_TPAGE *)packet;
        *page = gSpritePrimitiveTemplate.drawPage;
        AddPrim(&ot->org[depth], page);
        func_800BF2C8((u32)(page + 1));
    }
}

void drawPackedTime(s16 x, s16 y, u32 value, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s32 field;
    value >>= 8;
    field = value & 255;
    func_8006DFB4(x + 48, y, 2, 1, field, palette, blend, ot, depth);
    value >>= 8;
    func_8006C998(x + 40, y, 24, palette, blend, ot, depth);
    field = value & 255;
    func_8006DFB4(x + 24, y, 2, 1, field, 1, -1, ot, depth);
    func_8006C998(x + 16, y, 25, palette, blend, ot, depth);
    value >>= 8;
    func_8006DFB4(x, y, 2, 1, value, 1, -1, ot, depth);
}

void drawWrappedDoubleByteText(s16 x, s16 y, s16 columns, s8 *text, s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 startX = x;
    s16 savedColumns = columns;
    s16 column;
    s16 currentPalette = palette;
    s16 originalPalette = palette;
    column = 0;
    while (*text) {
        if (column >= columns) {
            x = startX;
            y += 16;
            column = 0;
        }
        if (*text == '\n') {
            column = savedColumns;
            text++;
            continue;
        }
        if (*text == 'c') {
            if ((u32)((u8)text[1] - '0') < 10) {
                currentPalette = text[1] - '0';
            } else {
                currentPalette = originalPalette;
            }
        } else {
            func_8006EB00(x, y, 8, 16, text, currentPalette, blend, ot, depth);
            x += 8;
            column++;
        }
        text += 2;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006EB00);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006EC84);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006EF30);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006F248);

void drawUppercaseProportionalText(s16 x, s16 y, s16 count, const char *text, s16 palette,
                   s16 blend, GsOT *ot, u16 depth) {
    DrawModeSpritePacket *packet = (DrawModeSpritePacket *)0x1F800000;
    s16 i;
    s8 glyph;
    packet->tag = (sizeof(*packet) / sizeof(u32) - 1) << 24;
    packet->drawModeCode = DRAW_MODE_COMMAND_HIGH;
    packet->code = TEXTURED_SPRITE_CODE;
    setRGB0(packet, gTexturedSpriteTemplate.r0, gTexturedSpriteTemplate.g0, gTexturedSpriteTemplate.b0);
    packet->tpage = getTPage(0, 0, 832, 256);
    if (blend >= 0) {
        packet->code = TEXTURED_SPRITE_CODE | SEMI_TRANSPARENT_FLAG;
        packet->tpage = (blend << 5) | getTPage(0, 0, 832, 256);
    }
    packet->clut = getClut(800, palette + 240);
    packet->x0 = x;
    packet->y0 = y;
    packet->h = 16;
    for (i = 0; *(const s8 *)text; i++, text++) {
        if (i >= count) {
            break;
        }
        if ((u8)(*text - 'A') < 26) {
            glyph = *text - 'A';
            packet->u0 = gUppercaseGlyphMetrics[glyph * 2];
            if (*(const s8 *)text >= 'O') {
                packet->v0 = 0x90;
            } else if (*(const s8 *)text >= 'H') {
                packet->v0 = 0x80;
            } else {
                packet->v0 = 0x70;
            }
            packet->w = gUppercaseGlyphMetrics[glyph * 2 + 1];
            addTexturedSpritePacket((TexturedSpritePacket *)packet, ot, depth);
            packet->x0 += packet->w;
        } else {
            packet->x0 += 8;
        }
    }
}

void drawWrappedText16x16(s16 x, s16 y, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 count = 0;
    s16 startX = x;
    while (*(const s8 *)text) {
        if ((s8)*text == 'c') {
            if ((u32)((u8)text[1] - '0') < 10) {
                palette = (s8)text[1] - '0' + 8;
            }
            text += 2;
            if (!*(const s8 *)text) break;
        }
        if (count >= columns) {
            x = startX;
            y += 16;
            count = 0;
        }
        func_8006F8A4(x, y, 16, 16, text, palette, blend, ot, depth);
        x += 16;
        text += 2;
        count++;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006F8A4);

void drawLimitedWrappedText16x16(s16 x, s16 y, s16 limit, s16 columns, const char *text,
                   s16 palette, s16 blend, GsOT *ot, u16 depth) {
    s16 count = 0;
    s16 total = 0;
    s16 startX = x;
    while (*(const s8 *)text && total < limit) {
        if ((s8)text[0] == '\r' && (s8)text[1] == '\n') {
            x = startX;
            y += 16;
            count = 0;
            text += 2;
            continue;
        } else if ((s8)text[0] == 'c') {
            if ((u32)((u8)text[1] - '0') < 10) {
                palette = (s8)text[1] - '0' + 8;
            }
            text += 2;
            continue;
        } else {
            if (count >= columns) {
                x = startX;
                y += 16;
                count = 0;
            }
            func_8006F8A4(x, y, 16, 16, text, palette, blend, ot, depth);
            x += 16;
            count++;
            total++;
        }
        text += 2;
    }
}

void setTexturedQuadTextureBlend(s32 index, s32 palette, s32 blend) {
    TextureRegionSet *texture = &D_8012DDA4[index];
    POLY_FT4 *quad = &gTexturedQuadTemplate;
    quad->tpage = GetTPage(texture->depth, blend, texture->x, texture->y);
    quad->clut = GetClut(texture->clutX, texture->clutY + palette);
    D_8013B6AC = texture->regions;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006FE68);

/* Style 0 uses an eight-pixel border on each side. */
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 8, 8, 0, clutOffset, blendMode, ot, depth);
}

void drawTexturedFrameStyle9(s16 x, s16 y, s16 w, s16 h, s16 palette,
                             s16 blend, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 8, 8, 9, palette, blend, ot, depth);
}

/* Style 0x20D uses a sixteen-pixel border on each side. */
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 16, 16, 0x20D, clutOffset, blendMode, ot, depth);
}
