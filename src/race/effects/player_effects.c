#include "psx/libetc.h"
#include "psx/stdarg.h"
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#include "../../graphics/render_effects_internal.h"
#define GAME_LOOP_ABSOLUTE_ADDRESSING
#include "game/engine/game_loop.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/offset_archive.h"
#include "game/engine/random.h"
#include "game/engine/task.h"
#include "game/graphics/draw_mode_sprite.h"
#include "game/graphics/graphics.h"
#include "game/graphics/quad_animation.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/menu/selected_records.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_geometry.h"
#include "game/race/player/race_player.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

#define PRESENTATION_SKIP_BUTTON 0x0800

/* Equal-channel color assignment for grayscale primitives. */
#define setGray0(p, shade) ((p)->r0 = (p)->g0 = (p)->b0 = (shade))

#define setGray1(p, shade) ((p)->r1 = (p)->g1 = (p)->b1 = (shade))
#define setGray2(p, shade) ((p)->r2 = (p)->g2 = (p)->b2 = (shade))
#define setGray3(p, shade) ((p)->r3 = (p)->g3 = (p)->b3 = (shade))

typedef struct TrickRingResource {
    s32 frame;
    s32 state;
} TrickRingResource;

/* One DMA tag covers the draw-mode command and the Gouraud quad. */
typedef struct TrickRingPacket {
    u32 tag;
    u16 tpage, drawMode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} TrickRingPacket;

/* One DMA tag covers the draw-mode command and the Gouraud triangle. */
typedef struct TrickTrianglePacket {
    u32 tag;
    u16 tpage, drawMode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
} TrickTrianglePacket;

/* One DMA tag covers the draw-mode command and the Gouraud line. */
typedef struct TrickRayPacket {
    u32 tag;
    u16 tpage, drawMode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
} TrickRayPacket;

typedef struct TrickSparkScratch {
    TrickRayPacket ray;
    POLY_FT4 spark;
} TrickSparkScratch;

typedef struct TriangleBurstResource {
    s32 frame;
    VECTOR origin;
    VECTOR positions[5];
    SVECTOR velocities[5];
    SVECTOR rotations[5];
} TriangleBurstResource;
typedef struct TriangleBurstScratch {
    GsCOORDINATE2 coordinate;
    SVECTOR vertices[3];
    s32 depth, cue, flags;
    MATRIX matrix;
} TriangleBurstScratch;
typedef struct ScreenPoint { s16 x, y; } ScreenPoint;


typedef struct RotatingQuadScratch {
    SVECTOR vertices[8];
    GsCOORDINATE2 coordinate;
    GsCOORDINATE2 parent;
    MATRIX localScreen;
    long depth, cue, flags;
    s32 textureAdvance, displacement;
    union {
        long packed;
        ScreenPoint point;
    } screen[4];
} RotatingQuadScratch;

typedef struct PlayerShadowScratch {
    GsCOORDINATE2 coordinate;
    SVECTOR vertices[4];
    MATRIX localScreen;
    VECTOR scale;
    SVECTOR rotation;
    long depth;
    long cue;
    long flags;
    s32 size;
} PlayerShadowScratch;

typedef struct RadialEffectScratch {
    MATRIX localScreen;
    long depth;
    long cue;
    long flags;
} RadialEffectScratch;

typedef struct PlayerRingEffectScratch {
    GsCOORDINATE2 coordinate;
    POLY_FT4 quad;
    SVECTOR vertices[4];
    VECTOR scale;
    long depth;
    long cue;
    long flags;
} PlayerRingEffectScratch;

typedef struct PlayerParticleScratch {
    MATRIX localScreen;
    POLY_FT4 quad;
    SVECTOR corners[4];
    SVECTOR vertices[4];
    long depth, cue, flags;
} PlayerParticleScratch;

typedef struct PlayerParticleTrailResource {
    s32 positions[10][3];
    s16 ages[10];
    s16 angles[10];
} PlayerParticleTrailResource;

typedef struct PlayerParticleTrailScratch {
    GsCOORDINATE2 coordinate;
    POLY_FT4 quad;
    SVECTOR vertices[4];
    VECTOR scale;
    SVECTOR rotation;
    long depth;
    long cue;
    long flags;
} PlayerParticleTrailScratch;

const VECTOR gInitialRaceOscillatingQuadPosition
    __attribute__((section(".rodata.race_oscillating_quad_position"))) = {
    -0x1B25, 0x3557, 0x9E5E, 0
};

const FadingTextGlyphBanks gFadingTextGlyphBanks
    __attribute__((section(".rodata.fading_text_glyph_banks"))) = {
    {D_8012EC04, D_8012ECC8, D_8012EECC, D_8012F0D0}
};

/* Shift-JIS captions with the original four-byte padding. */
const char gTimedSpritePresentationText[0x498]
    __attribute__((section(".rodata.timed_sprite_text"), aligned(4))) =
    "\x83\x58\x83\x6D\x83\x7B\x83\x4C\x83\x62\x83\x59  \x83\x76\x83\x89"
    "\x83\x58\x00\x00\x00\x00"
    "\x81\x7C\x83\x58\x83\x5E\x83\x62\x83\x74\x81\x7C\x00\x00\x00\x00"
    "\x83\x4C\x83\x83\x83\x58\x83\x67\x00\x00\x00\x00"
    "\x83\x58\x83\x89\x83\x62\x83\x56\x83\x85\x00\x00"
    "\x90\x58\x93\x63  \x90\xE7\x96\xBE\x00\x00"
    "\x83\x69\x83\x93\x83\x56\x81\x5B\x81\x5E\x8F\x97\x82\xCC\x8E\x71\x82\x60"
    "\x00\x00"
    "\x90\xCE\x90\xEC  \x90\xC3\x00\x00\x00\x00"
    "\x83\x8A\x83\x93\x83\x5F\x81\x5E\x83\x4A\x83\x47\x83\x66\x81\x5E\x8F\x97"
    "\x82\xCC\x8E\x71\x82\x61\x00\x00\x00\x00"
    "\x89\xC4\x8E\xF7  \x83\x8A\x83\x49\x00\x00"
    "\x83\x57\x83\x83\x83\x80\x81\x5E\x83\x56\x83\x6D\x83\x72\x83\x93\x81\x5E"
    "\x92\x53\x94\x43\x82\xCC\x90\xE6\x90\xB6\x00\x00\x00\x00"
    "\x90\x7A\x96\x4B\x95\x94  \x8F\x87\x88\xEA\x00\x00\x00\x00"
    "\x83\x67\x83\x7E\x83\x42\x81\x5E\x83\x8B\x83\x72\x81\x5B\x81\x5E\x8C\xA2"
    "\x93\x58\x92\xB7\x00\x00"
    "\x89\x93\x8B\xDF  \x8D\x46\x88\xEA\x00\x00"
    "\x93\x63\x8C\xFB  \x8D\x47\x8E\x71\x00\x00"
    "\x83\x6A\x83\x52\x83\x8B\x81\x5E\x83\x58\x83\x89\x83\x62\x83\x56\x83\x85"
    "\x82\xCC\x95\xEA\x81\x5E\x92\x6A\x82\xCC\x8E\x71\x00\x00"
    "\x8B\x67\x93\x63  \x8C\xC3\x93\xDE\x94\xFC\x00\x00\x00\x00"
    "\x83\x69\x83\x8C\x81\x5B\x83\x56\x83\x87\x83\x93\x00\x00\x00\x00"
    "\x82\x63\x82\x69  \x83\x69\x83\x43\x83\x4E\x00\x00\x00\x00"
    "\x83\x81\x83\x43\x83\x93\x83\x76\x83\x89\x83\x93\x83\x69\x81\x5B\x00\x00"
    "\x00\x00"
    "\x95\x9E\x95\x94  \x97\x52\x96\xAF\x8C\xCF\x00\x00\x00\x00"
    "\x83\x41\x83\x56\x83\x58\x83\x5E\x83\x93\x83\x67\x83\x76\x83\x89\x83\x93"
    "\x83\x69\x81\x5B\x00\x00"
    "\x8F\xAC\x90\x58  \x90\xAC\x97\x59\x00\x00"
    "\x83\x81\x83\x43\x83\x93\x83\x76\x83\x8D\x83\x4F\x83\x89\x83\x7D\x81\x5B"
    "\x00\x00"
    "\x83\x41\x83\x56\x83\x58\x83\x5E\x83\x93\x83\x67\x83\x76\x83\x8D\x83\x4F"
    "\x83\x89\x83\x7D\x81\x5B\x00\x00\x00\x00"
    "\x82\xDD\x82\xE2\x82\xB6\x82\xDC\x82\xDD\x82\xCC\x82\xE9\x00\x00"
    "\x83\x50\x83\x5C\x82\xDC\x82\xE9\x00\x00\x00\x00"
    "\x82\x52\x82\x63\x83\x4F\x83\x89\x83\x74\x83\x42\x83\x62\x83\x4E\x00\x00"
    "\x00\x00"
    "\x82\xED\x82\xBE  \x82\xB3\x82\xC6\x82\xB5\x00\x00\x00\x00"
    "\x8A\xE2\x8D\xE8  \x92\x71\x94\xFC\x00\x00"
    "\x90\xD4\x96\xD8  \x8B\x60\x8F\xAE\x00\x00"
    "\x91\xE5\x92\xCE\x92\xE9\x8D\x91\x00\x00\x00\x00"
    "\x82\x51\x82\x63\x83\x4F\x83\x89\x83\x74\x83\x42\x83\x62\x83\x4E\x00\x00"
    "\x00\x00"
    "\x82\xA8\x82\xA9\x82\xBF\x82\xA4\x00\x00\x00\x00"
    "\x83\x4C\x83\x83\x83\x89\x83\x4E\x83\x5E\x81\x5B\x8C\xB4\x88\xC4\x00\x00"
    "\x00\x00"
    "\x83\x51\x81\x5B\x83\x80\x92\x86\x8C\xB4\x89\xE6\x81\x5E\x83\x58\x83\x5E"
    "\x83\x62\x83\x74\x83\x8D\x81\x5B\x83\x8B\x8C\xB4\x89\xE6\x00\x00\x00\x00"
    "\x83\x82\x81\x5B\x83\x56\x83\x87\x83\x93\x00\x00"
    "\x96\x78\x94\x56\x93\xE0\x8C\x4E\x00\x00\x00\x00"
    "\x83\x54\x83\x45\x83\x93\x83\x68\x83\x76\x83\x8D\x83\x66\x83\x85\x81\x5B"
    "\x83\x54\x81\x5B\x00\x00"
    "\x8A\x7D\x88\xE4  \x8C\x4D\x00\x00\x00\x00"
    "\x82\x73\x82\x6E\x82\x6C\x82\x6E\x82\x67\x82\x68\x82\x6A\x82\x6E  "
    "\x82\x72\x82\x60\x82\x73\x82\x6E\x00\x00"
    "\x8E\x52\x96\x7B  \x8F\xAC\x95\x53\x8D\x87\x00\x00\x00\x00"
    "\x83\x76\x83\x8D\x83\x57\x83\x46\x83\x4E\x83\x67\x83\x8A\x81\x5B\x83\x5F"
    "\x81\x5B\x00\x00\x00\x00"
    "\x83\x66\x83\x42\x83\x8C\x83\x4E\x83\x5E\x81\x5B\x00\x00\x00\x00"
    "\x96\xEE\x8E\x52  \x8F\x47\x8E\xF7\x00\x00"
    "\x91\xBA\x93\x63  \x95\x90\x8E\x69\x00\x00"
    "\x82\x61\x82\x66\x82\x6C  \x83\x49\x81\x5B\x83\x76\x83\x6A\x83\x93"
    "\x83\x4F\x81\x45\x83\x47\x83\x93\x83\x66\x83\x42\x83\x93\x83\x4F\x00\x00"
    "\x83\x65\x83\x58\x83\x67\x83\x76\x83\x8C\x83\x43\x83\x84\x81\x5B\x00\x00"
    "\x00\x00"
    "\x8F\xAC\x91\x71  \x8D\x47\x8C\x76\x00\x00"
    "\x95\xBD\x96\xEC  \x90\xB9\x93\xF1\x00\x00"
    "\x8B\xE0\xE0\x56  \x8B\x4D\x95\x71\x00\x00"
    "\x8E\x4F\x89\x59  \x82\xE0\x82\xA6\x00\x00"
    "\x91\xAB\x97\xA7  \x8F\x83\x00\x00\x00\x00"
    "\x83\x65\x83\x58\x83\x67\x83\x76\x83\x8C\x83\x43\x83\x84\x81\x5B\x83\x60"
    "\x81\x5B\x83\x74\x00\x00"
    "\x8E\x52\x96\x7B  \x8D\x57\x8D\x4E\x00\x00"
    "\x83\x65\x83\x58\x83\x67\x83\x76\x83\x8C\x83\x43\x83\x84\x81\x5B\x83\x7D"
    "\x83\x6C\x81\x5B\x83\x57\x83\x83\x81\x5B\x00\x00\x00\x00"
    "\x93\xF1\x96\xD8  \x97\xB2\x8D\x4B\x00\x00"
    "\x83\x80\x81\x5B\x83\x72\x81\x5B\x81\x5E\x83\x43\x83\x78\x83\x93\x83\x67"
    "\x83\x4F\x83\x89\x83\x74\x83\x42\x83\x62\x83\x4E\x00\x00"
    "\x83\x41\x81\x5B\x83\x58\x83\x8A\x81\x5B\x83\x76\x83\x8D\x83\x5F\x83\x4E"
    "\x83\x56\x83\x87\x83\x93\x00\x00\x00\x00"
    "\x83\x7D\x83\x6A\x83\x85\x83\x41\x83\x8B\x83\x66\x83\x55\x83\x43\x83\x93"
    "\x00\x00"
    "\x89\x69\x93\x87  \x90\xAD\x92\x6A\x00\x00"
    "\x83\x5E\x83\x43\x83\x67\x83\x8B\x83\x66\x83\x55\x83\x43\x83\x93\x00\x00"
    "\x00\x00"
    "\x90\xCE\x88\xE4  \x8E\x6A\x90\x6C\x00\x00"
    "\x83\x76\x83\x8D\x83\x82\x81\x5B\x83\x56\x83\x87\x83\x93\x00\x00"
    "\x93\x87\x91\xBA  \x91\xE5\x8E\x4F\x00\x00"
    "\x83\x76\x83\x8D\x83\x66\x83\x85\x81\x5B\x83\x54\x81\x5B\x00\x00"
    "\x93\x63\x92\x86  \x97\x54\x94\x56\x00\x00";

char gTimedSpriteAlternateText[4][8]
    __attribute__((section(".data.timed_sprite_alternate_text"), aligned(4))) = {
    "\x83\x70\x83\x81\x83\x89\x00\x00",
    "\x82\xA2\x82\xA9\x82\x61\x00\x00",
    "\x82\xC2\x82\xE9\x93\x63\x00\x00",
    "\x8A\x5E\x88\xEE\x00\x00\x00\x00",
};

const TimedSpriteStep gTimedSpriteStepsAtFrame0[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 90, -80, -24, &gTimedSpritePresentationText[0x000], 12, 0},
    {0, 90, -48, 0, &gTimedSpritePresentationText[0x018], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame210[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 1230, -32, -64, &gTimedSpritePresentationText[0x028], 15, 0},
    {60, 120, -40, -32, &gTimedSpritePresentationText[0x034], 12, 0},
    {60, 120, -40, 0, &gTimedSpritePresentationText[0x040], 14, 0},
    {210, 120, -72, -32, &gTimedSpritePresentationText[0x04C], 12, 0},
    {210, 120, -32, 0, &gTimedSpritePresentationText[0x060], 14, 0},
    {360, 120, -96, -32, &gTimedSpritePresentationText[0x06C], 12, 0},
    {360, 120, -40, 0, &gTimedSpritePresentationText[0x088], 14, 0},
    {510, 120, -112, -32, &gTimedSpritePresentationText[0x094], 12, 0},
    {510, 120, -48, 0, &gTimedSpritePresentationText[0x0B4], 14, 0},
    {660, 120, -88, -32, &gTimedSpritePresentationText[0x0C4], 12, 0},
    {660, 120, -40, 0, &gTimedSpritePresentationText[0x0DC], 14, 0},
    {810, 120, -24, -32, gTimedSpriteAlternateText[0], 12, 0},
    {810, 120, -40, 0, &gTimedSpritePresentationText[0x0E8], 14, 0},
    {960, 120, -120, -32, &gTimedSpritePresentationText[0x0F4], 12, 0},
    {960, 120, -48, 0, &gTimedSpritePresentationText[0x114], 14, 0},
    {1110, 120, -48, -32, &gTimedSpritePresentationText[0x124], 12, 0},
    {1110, 120, -48, 0, &gTimedSpritePresentationText[0x134], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame1530[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -64, -32, &gTimedSpritePresentationText[0x144], 15, 0},
    {30, 90, -48, 0, &gTimedSpritePresentationText[0x158], 14, 0},
    {120, 120, -88, -32, &gTimedSpritePresentationText[0x168], 15, 0},
    {150, 90, -40, 0, &gTimedSpritePresentationText[0x180], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame1800[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -72, -32, &gTimedSpritePresentationText[0x18C], 15, 0},
    {30, 90, -24, 0, gTimedSpriteAlternateText[1], 14, 0},
    {120, 120, -96, -32, &gTimedSpritePresentationText[0x1A0], 15, 0},
    {150, 90, -56, 0, &gTimedSpritePresentationText[0x1BC], 14, 0},
    {150, 90, -32, 16, &gTimedSpritePresentationText[0x1CC], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame2130[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 270, -64, -64, &gTimedSpritePresentationText[0x1D8], 15, 0},
    {30, 120, -24, -32, gTimedSpriteAlternateText[2], 14, 0},
    {30, 120, -48, 0, &gTimedSpritePresentationText[0x1EC], 14, 0},
    {30, 120, -16, -16, gTimedSpriteAlternateText[3], 14, 0},
    {150, 120, -40, -32, &gTimedSpritePresentationText[0x1FC], 14, 0},
    {150, 120, -40, -16, &gTimedSpritePresentationText[0x208], 14, 0},
    {150, 120, -32, 0, &gTimedSpritePresentationText[0x214], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame2430[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -64, -32, &gTimedSpritePresentationText[0x220], 15, 0},
    {30, 90, -32, 0, &gTimedSpritePresentationText[0x234], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame2670[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -64, -48, &gTimedSpritePresentationText[0x240], 15, 0},
    {0, 120, -128, -32, &gTimedSpritePresentationText[0x254], 15, 0},
    {30, 90, -32, 0, &gTimedSpritePresentationText[0x208], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame2820[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -40, -32, &gTimedSpritePresentationText[0x278], 15, 0},
    {30, 90, -32, 0, &gTimedSpritePresentationText[0x284], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame2970[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 150, -88, -64, &gTimedSpritePresentationText[0x290], 15, 0},
    {30, 120, -32, -32, &gTimedSpritePresentationText[0x2A8], 14, 0},
    {30, 120, -104, -16, &gTimedSpritePresentationText[0x2B4], 14, 0},
    {30, 120, -48, 0, &gTimedSpritePresentationText[0x2D0], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame3150[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -80, -32, &gTimedSpritePresentationText[0x2E0], 15, 0},
    {30, 90, -48, 0, &gTimedSpritePresentationText[0x158], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame3390[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -48, -48, &gTimedSpritePresentationText[0x2F8], 15, 0},
    {30, 90, -40, -16, &gTimedSpritePresentationText[0x308], 14, 0},
    {30, 90, -40, 0, &gTimedSpritePresentationText[0x314], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame3510[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -136, -32, &gTimedSpritePresentationText[0x320], 15, 0},
    {30, 90, -104, 0, &gTimedSpritePresentationText[0x2B4], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame3720[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 150, -64, -64, &gTimedSpritePresentationText[0x344], 15, 0},
    {30, 120, -40, -32, &gTimedSpritePresentationText[0x358], 14, 0},
    {30, 120, -40, -16, &gTimedSpritePresentationText[0x364], 14, 0},
    {30, 120, -40, 0, &gTimedSpritePresentationText[0x370], 14, 0},
    {30, 120, -40, 16, &gTimedSpritePresentationText[0x37C], 14, 0},
    {30, 120, -32, 32, &gTimedSpritePresentationText[0x388], 14, 0},
    {150, 120, -88, -32, &gTimedSpritePresentationText[0x394], 15, 0},
    {180, 90, -40, 0, &gTimedSpritePresentationText[0x3AC], 14, 0},
    {270, 120, -112, -32, &gTimedSpritePresentationText[0x3B8], 15, 0},
    {300, 90, -40, 0, &gTimedSpritePresentationText[0x3D8], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame4110[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -120, -32, &gTimedSpritePresentationText[0x3E4], 15, 0},
    {30, 90, -96, 0, &gTimedSpritePresentationText[0x404], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

const TimedSpriteStep gTimedSpriteStepsAtFrame4230[]
    __attribute__((section(".data.timed_sprite_steps"))) = {
    {0, 120, -72, -32, &gTimedSpritePresentationText[0x420], 15, 0},
    {30, 90, -40, 0, &gTimedSpritePresentationText[0x434], 14, 0},
    {120, 120, -64, -32, &gTimedSpritePresentationText[0x440], 15, 0},
    {150, 90, -40, 0, &gTimedSpritePresentationText[0x454], 14, 0},
    {240, 120, -56, -32, &gTimedSpritePresentationText[0x460], 15, 0},
    {270, 90, -40, 0, &gTimedSpritePresentationText[0x470], 14, 0},
    {360, 150, -56, -32, &gTimedSpritePresentationText[0x47C], 15, 0},
    {390, 120, -40, 0, &gTimedSpritePresentationText[0x48C], 14, 0},
    {-1, 0, 0, 0, NULL, 0, 0},
};

s32 gTimedSpritePresentationFrame
    __attribute__((section(".sdata"))) = 0;

PlayerRenderSnapshot gPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots"))) = {0};

PlayerRenderSnapshot gPreviousPlayerRenderSnapshots[4]
    __attribute__((section(".data.player_render_snapshots"))) = {0};

MATRIX gPlayerRenderIdentityMatrix
    __attribute__((section(".data.player_render_snapshots"))) = {0};

POLY_FT4 gPlayerSnapshotTrailPacket
    __attribute__((section(".data.player_snapshot_trail"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 0, 0, 0,
    TEXTURED_QUAD_CODE | SEMI_TRANSPARENT_FLAG,
    0, 0, 64, 0, getClut(880, 243),
    0, 0, 127, 0, getTPage(0, 2, 704, 0),
    0, 0, 64, 31, 0,
    0, 0, 127, 31, 0,
};

SVECTOR gPlayerSnapshotTrailEndpoints[2]
    __attribute__((section(".data.player_snapshot_trail"))) = {
    {0, 0, -20, 0}, {0, 0, 20, 0},
};

POLY_FT4 gPlayerShadowPacket
    __attribute__((section(".data.player_shadow"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 48, 48, 48,
    TEXTURED_QUAD_CODE | SEMI_TRANSPARENT_FLAG,
    0, 0, 64, 32, getClut(880, 241),
    0, 0, 127, 32, getTPage(0, 2, 704, 0),
    0, 0, 64, 63, 0,
    0, 0, 127, 63, 0,
};

SVECTOR gPlayerShadowVertices[4]
    __attribute__((section(".data.player_shadow"))) = {
    {-45, 0, -20, 0}, {45, 0, -20, 0},
    {-45, 0, 20, 0}, {45, 0, 20, 0},
};

VECTOR gPlayerPairedQuadOffsets[2]
    __attribute__((section(".data.player_paired_quad_offsets"))) = {
    {-50, 0, 0, 0}, {50, 0, 0, 0}
};

SVECTOR gPlayerTripleQuadOffsets[3]
    __attribute__((section(".data.player_triple_quad_offsets"))) = {
    {-40, 0, 0, 0}, {40, 0, 0, 0}, {0, 0, 0, 0}
};

SVECTOR gRotatingQuadTrailVertices[4]
    __attribute__((section(".data.rotating_quad_trail_vertices"))) = {
    {-108, -8, 0, 0}, {108, -8, 0, 0},
    {-108, 8, 0, 0}, {108, 8, 0, 0}
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

PaletteColor gCapturedColorCyclePalette[16]
    __attribute__((section(".data.color_cycle_palette"), aligned(4))) = {0};

u16 gWhitePulsePalette[16]
    __attribute__((section(".bss.white_pulse_palette"), aligned(4)));

u16 gRadialQuadHalfSizes[4]
    __attribute__((section(".data.radial_effect_quads"))) = {32, 32, 16, 32};

u16 gRadialQuadPaletteOffsets[4]
    __attribute__((section(".data.radial_effect_quads"))) = {1, 0, 1, 2};

u16 gRadialQuadShades[4]
    __attribute__((section(".data.radial_effect_quads"))) = {32, 16, 32, 16};

RECT gWhitePulsePaletteRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};

RECT gColorCycleCaptureRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};

u16 gStretchedTextWidth __attribute__((section(".sdata.stretched_text"), aligned(2))) = 8;
u16 gTextGlyphPalette __attribute__((section(".sdata.stretched_text"), aligned(2))) = 0;

void updatePlayerRenderSnapshots(void) {
    GsCOORDINATE2 coordinate;
    u32 i;
    RacePlayer *player;
    GsInitCoordinate2(0, &coordinate);
    GsGetLs(&coordinate, &gPlayerRenderIdentityMatrix);
    for (i = 0; i < 2; i++) {
        player = &gRacePlayers[i];
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
        startPlayerShadowTask(playerIndex);
        startDelayedEightEntryEffectTask(playerIndex);
        createPlayerTrailTask(playerIndex);
        startPlayerTurnTripleQuadEffectTask(playerIndex);
        func_800688C0(playerIndex);
    }
}

void initializeRacePlayerEffects(void) {
    s32 playerIndex;
    VECTOR position;

    func_80014930(&gGameTasks[4], updatePlayerRenderSnapshots);
    playerSnapshotTaskGp = D_8013AE8C;
    if (D_8013AFD2 == 4) {
        playerIndex = 0;
        do {
            startPlayerShadowTask(playerIndex);
            createPlayerPositionTrailTask(playerIndex);
            func_800688C0(playerIndex);
        } while (++playerIndex < D_8013AFD2);
    } else {
        for (playerIndex = 0; playerIndex < D_8013AFD2; playerIndex++) {
            startPlayerShadowTask(playerIndex);
            startDelayedEightEntryEffectTask(playerIndex);
            createPlayerTrailTask(playerIndex);
            startPlayerTurnTripleQuadEffectTask(playerIndex);
            func_800688C0(playerIndex);
        }
    }
    if (D_8013AFCE == 6) {
        for (playerIndex = 0; playerIndex < D_8013AFCD; playerIndex++) {
            createPlayerParticleTask(playerIndex);
        }
    }
    if (D_8013AFCE == 4) {
        position = gInitialRaceOscillatingQuadPosition;
        createOscillatingQuadTask(0, &position, 0x200, 0, 255, 0);
        position.vx = -0x198F;
        position.vy = 0x3343;
        position.vz = 0x91FF;
        createOscillatingQuadTask(0, &position, 0x100, 255, 0, 128);
        position.vx = 0xF2E;
        position.vy = 0xB676;
        position.vz = 0x17E89;
        createOscillatingQuadTask(0, &position, 0x100, 255, 255, 0);
        position.vx = 0x18EC;
        position.vy = 0xB676;
        position.vz = 0x17ADC;
        createOscillatingQuadTask(0, &position, -0x200, 0, 128, 255);
        position.vx = -0x5AF;
        position.vy = 0xB676;
        position.vz = 0x179BC;
        createOscillatingQuadTask(0, &position, -0x100, 255, 0, 128);
        position.vx = -0x9B4;
        position.vy = 0xB5C2;
        position.vz = 0x17110;
        createOscillatingQuadTask(0, &position, 0x200, 0, 255, 0);
        for (playerIndex = 0; playerIndex < D_8013AFCD; playerIndex++) {
            createFlickeringDotsTask(playerIndex);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800612BC);

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80061304);

void drawPlayerSnapshotTrailTask(void) {
    s32 p, flag;
    PlayerSnapshotTrailResource *resource;
    POLY_FT4 *packet;
    MATRIX *scratch = (MATRIX *)getScratchAddr(0);
    s32 depth;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerSnapshotTrailResource *)&D_800D9504[D_8013AE88->resource14];
    packet = &gPlayerSnapshotTrailPacket;
    packet->r0 = resource->remainingFrames * 48 / 90;
    packet->g0 = resource->remainingFrames * 32 / 90;
    packet->b0 = resource->remainingFrames * 32 / 90;
    GsGetLs(&resource->current, scratch);
    ScaleMatrix(scratch, &resource->currentScale);
    GsSetLsMatrix(scratch);
    depth = RotTransPers(&gPlayerSnapshotTrailEndpoints[0], (s32 *)&packet->x0, &p, &flag);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[1], (s32 *)&packet->x1, &p, &flag);
    GsGetLs(&resource->previous, scratch);
    ScaleMatrix(scratch, &resource->previousScale);
    GsSetLsMatrix(scratch);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[0], (s32 *)&packet->x2, &p, &flag);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[1], (s32 *)&packet->x3, &p, &flag);
    if (depth >= 0 && flag >= 0) {
        addCopiedPrimitive(&D_8013AF6C[depth >> 4], (u32 *)packet);
    }
    if (--resource->remainingFrames == -1 || gPlayerSnapshotTrailPacket.y0 > 160) {
        deleteCurrentTaskSubtree();
        D_8013B300--;
    }
}

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
    setCurrentTaskCallback(drawPlayerSnapshotTrailTask);
}

void drawTrickTriangleRingTask(void) {
    TrickRingResource *resource;
    RacePlayer *player;
    TrickTrianglePacket *scratch;
    TrickTrianglePacket *packet;
    u32 *ot;
    s32 radius;
    s32 angle;
    s32 nextAngle;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TrickRingResource *)&D_800D9504[D_8013AE88->resource14];
    scratch = (TrickTrianglePacket *)getScratchAddr(0);
    packet = (TrickTrianglePacket *)GsOUT_PACKET_P;
    player = &gRacePlayers[D_8013AE88->data18];
    if (D_8013AE88->data18 == 0) {
        ot = D_800EC040[D_8013AF78].org;
    } else {
        ot = D_800EC0E0[D_8013AF78].org;
    }
    if (D_8013AFD6 && player->unk0C8 < 9) {
        deleteCurrentTaskSubtree();
    }
    switch (resource->state) {
    case 0:
    case 1:
        radius = 192 - resource->frame * 128 / 10;
        if (resource->frame == 10) {
            resource->state = 2;
            resource->frame = 0;
        }
        break;
    case 2:
        radius = 64;
        if (resource->frame == 10) {
            resource->state = 3;
            resource->frame = 0;
        }
        break;
    case 3:
        radius = 192 - (10 - resource->frame) * 128 / 10;
        if (resource->frame == 10) {
            deleteCurrentTaskSubtree();
        }
        break;
    case 4:
        if (player->unk0C8 == 0) {
            resource->state = 0;
            resource->frame = 0;
        }
        return;
    }
    if (D_8013AFAA & 0x400) {
        if (D_8013AE88->data18 != D_8013AF78) {
            goto advance;
        }
        scratch->tag = (1 + 6) << 24;
    } else {
        if (D_8013AE88->data18 != 0) {
            goto advance;
        }
        scratch->tag = (1 + 6) << 24;
    }
    scratch->code = POLY_G3_CODE | 2;
    scratch->tpage = getTPage(0, 1, 0, 0);
    scratch->drawMode = DRAW_MODE_COMMAND_HIGH;
    scratch->r0 = scratch->g0 = scratch->b0 = 0;
    if (player->unk0C8 >= 9) {
        scratch->r1 = scratch->g1 = 128;
        scratch->b1 = 0;
        scratch->r2 = scratch->g2 = 128;
        scratch->b2 = 0;
    } else {
        scratch->r1 = scratch->g1 = scratch->b1 = 64;
        scratch->r2 = scratch->g2 = scratch->b2 = 64;
    }
    angle = 0;
    do {
        s32 width = randomLinearFromValue(angle + resource->frame) % 8 + 8;
        setXY0(scratch, radius * rcos(angle) / 4096, radius * rsin(angle) / 4096 + 32);
        scratch->x1 = rcos(angle) * 280 / 4096;
        scratch->y1 = rsin(angle) * 210 / 4096 + 32;
        nextAngle = angle + width;
        scratch->x2 = rcos(nextAngle) * 280 / 4096;
        scratch->y2 = rsin(nextAngle) * 210 / 4096 + 32;
        *packet = *scratch;
        addPrim(ot, packet);
        packet++;
        {
            s32 gap = randomLinearFromValue((angle + resource->frame) * angle) % 64 + 16;
            angle = nextAngle + gap;
        }
    } while (angle < 0x1000);
    GsOUT_PACKET_P = (u32)packet;

advance:
    if (!D_8013AFD3) {
        resource->frame++;
    }
}

void startIndexedRenderTask(s32 index) {
    TaskResourceRecord *resource;
    func_80014930(D_8013B304, drawTrickTriangleRingTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
}

void drawTrickRingTask(void) {
    TrickRingResource *resource;
    RacePlayer *player;
    TrickRingPacket *scratch;
    TrickRingPacket *packet;
    u32 *ot;
    s32 angle;
    s32 nextAngle;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TrickRingResource *)&D_800D9504[D_8013AE88->resource14];
    scratch = (TrickRingPacket *)getScratchAddr(0);
    player = &gRacePlayers[D_8013AE88->data18];
    packet = (TrickRingPacket *)GsOUT_PACKET_P;
    if (D_8013AE88->data18 == 0) {
        ot = D_800EC040[D_8013AF78].org;
    } else {
        ot = D_800EC0E0[D_8013AF78].org;
    }
    if (D_8013AFD6 && player->unk0C8 < 9) {
        deleteCurrentTaskSubtree();
    }
    switch (resource->state) {
    case 0:
        if (player->unk0C8 > 0) {
            resource->state = 1;
            resource->frame = 0;
        }
        return;
    case 1:
        setGray2(scratch, resource->frame * 16);
        setGray3(scratch, resource->frame * 16);
        if (resource->frame == 10) {
            resource->state = 2;
            resource->frame = 0;
        }
        break;
    case 2:
        setGray2(scratch, 160);
        setGray3(scratch, 160);
        if (resource->frame == 30 || player->unk0C8 < 9) {
            resource->state = 3;
            resource->frame = 0;
        }
        break;
    case 3:
        setGray2(scratch, (10 - resource->frame) * 16);
        setGray3(scratch, (10 - resource->frame) * 16);
        if (resource->frame == 10) {
            resource->state = 4;
        }
        break;
    case 4:
        deleteCurrentTaskSubtree();
        return;
    }
    if (D_8013AFAA & 0x400) {
        if (D_8013AE88->data18 != D_8013AF78) {
            goto advance;
        }
    } else if (D_8013AE88->data18 != 0) {
        goto advance;
    }

    scratch->tag = (1 + 8) << 24;
    scratch->code = POLY_G4_CODE | 2;
    scratch->tpage = getTPage(0, 2, 0, 0);
    scratch->drawMode = DRAW_MODE_COMMAND_HIGH;
    setGray0(scratch, 0);
    setGray1(scratch, 0);
    angle = 0;
    do {
        setXY0(scratch, rcos(angle) / 64, rsin(angle) / 64 + 32);
        nextAngle = angle + 0x200;
        scratch->x1 = rcos(nextAngle) / 64;
        scratch->y1 = rsin(nextAngle) / 64 + 32;
        scratch->x2 = rcos(angle) * 240 / 4096;
        scratch->y2 = rsin(angle) * 208 / 4096 + 32;
        scratch->x3 = rcos(nextAngle) * 240 / 4096;
        scratch->y3 = rsin(nextAngle) * 208 / 4096 + 32;
        *packet = *scratch;
        angle += 0x200;
        addPrim(&ot[1], packet);
        packet++;
    } while (angle < 0x1000);
    GsOUT_PACKET_P = (u32)packet;

advance:
    if (!D_8013AFD3) {
        resource->frame++;
    }
    if (resource->frame >= 31) {
        deleteCurrentTaskSubtree();
    }
}


void drawTrickSparkRingTask(void) {
    s32 *frame;
    TrickSparkScratch *scratch;
    u8 *packet;
    RacePlayer *player;
    u32 *ot;
    s32 minimumRadius;
    s32 angle;
    s32 innerRadius;
    s32 outerRadius;
    s32 edgeRadius;
    s32 edgeAngle;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    frame = (s32 *)&D_800D9504[D_8013AE88->resource14].unk00;
    scratch = (TrickSparkScratch *)getScratchAddr(0);
    packet = (u8 *)GsOUT_PACKET_P;
    player = &gRacePlayers[D_8013AE88->data18];
    if (D_8013AE88->data18 == 0) {
        ot = D_800EC040[D_8013AF78].org;
    } else {
        ot = D_800EC0E0[D_8013AF78].org;
    }
    if (D_8013AFD6 && player->unk0C8 < 9) {
        deleteCurrentTaskSubtree();
    }
    if (player->unk0C8 < 9 && *frame < 30) {
        *frame = 30;
    }
    /* Initialize complete DMA tags before linking the packets into the ordering table. */
    if (D_8013AFAA & 0x400) {
        if (D_8013AE88->data18 != D_8013AF78) {
            goto advance;
        }
        scratch->ray.tag = (1 + 4) << 24;
    } else {
        if (D_8013AE88->data18 != 0) {
            goto advance;
        }
        scratch->ray.tag = (1 + 4) << 24;
    }

    scratch->ray.code = LINE_G2_CODE | 2;
    scratch->ray.drawMode = DRAW_MODE_COMMAND_HIGH;
    scratch->ray.tpage = getTPage(0, 1, 0, 0);
    scratch->spark.tag = 9 << 24;
    setcode(&scratch->spark, POLY_FT4_CODE | 2);
    scratch->spark.tpage = getTPage(0, 1, 704, 0);
    scratch->spark.clut = getClut(880, 241);
    scratch->ray.r0 = scratch->ray.g0 = scratch->ray.b0 = 0;
    setUV4(&scratch->spark, 0, 0, 63, 0, 0, 63, 63, 63);
    minimumRadius = 64;
    if (*frame < 30) {
        scratch->ray.r1 = 255;
        scratch->ray.g1 = 180;
        scratch->ray.b0 = 0;
        setRGB0(&scratch->spark, 0, 0, 255);
    } else {
        s32 green;

        scratch->ray.r1 = (40 - *frame) * 255 / 10;
        green = (40 - *frame) * 18;
        scratch->ray.b0 = 0;
        scratch->spark.r0 = 0;
        scratch->spark.g0 = 0;
        scratch->ray.g1 = green;
        scratch->spark.b0 = (40 - *frame) * 255 / 10;
        minimumRadius = (*frame - 30) * 128 / 10 + 64;
    }

    angle = 0;
    do {
        innerRadius = ((*frame * 16 + (angle + 0x1C96251) * (0x4AE9 % (angle + 1))) >> (angle & 15)) & 255;
        outerRadius = innerRadius + 128;
        if (innerRadius < minimumRadius) {
            innerRadius = minimumRadius;
        }
        if (outerRadius < minimumRadius) {
            outerRadius = minimumRadius;
        }
        setXY2(&scratch->ray,
               innerRadius * rcos(angle) / 4096, innerRadius * rsin(angle) / 4096,
               outerRadius * rcos(angle) / 4096, outerRadius * rsin(angle) / 4096);
        *(TrickRayPacket *)packet = scratch->ray;
        angle += 32;
        addPrim(ot, packet);
        packet += sizeof(TrickRayPacket);
    } while (angle < 0x1000);

    angle = 0;
    do {
        innerRadius = ((*frame * 16 + (angle + 0x1FBBCF1) * (0xD789 % (angle + 1))) >> (angle & 15)) & 255;
        outerRadius = innerRadius + 32;
        edgeRadius = innerRadius + 16;
        setXY0(&scratch->spark, outerRadius * rcos(angle) / 4096, outerRadius * rsin(angle) / 4096);
        edgeAngle = angle - 16;
        scratch->spark.x1 = edgeRadius * rcos(edgeAngle) / 4096;
        scratch->spark.y1 = edgeRadius * rsin(edgeAngle) / 4096;
        edgeAngle = angle + 16;
        scratch->spark.x2 = edgeRadius * rcos(edgeAngle) / 4096;
        scratch->spark.y2 = edgeRadius * rsin(edgeAngle) / 4096;
        scratch->spark.x3 = innerRadius * rcos(angle) / 4096;
        scratch->spark.y3 = innerRadius * rsin(angle) / 4096;
        *(POLY_FT4 *)packet = scratch->spark;
        angle += 64;
        addPrim(ot, packet);
        packet += sizeof(POLY_FT4);
    } while (angle < 0x1000);
    GsOUT_PACKET_P = (u32)packet;

advance:
    if (!D_8013AFD3) {
        ++*frame;
    }
    if (*frame >= 41) {
        deleteCurrentTaskSubtree();
    }
}

void startIndexedRenderTaskPair(s32 index) {
    TaskResourceRecord *resource;
    func_80014930(D_8013B304, drawTrickRingTask);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = &D_800D9504[D_8013AE8C->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
    func_80014930(D_8013B304, drawTrickSparkRingTask);
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
    scratch = (SPRT_8 *)getScratchAddr(0);
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

void drawRadialTriangleAndQuadEffect(void) {
    s16 angle = 0;
    s16 ray = 0;
    s16 i;

    do {
        s16 x, y, rightX, rightY, leftX, leftY, centerX, centerY;
        s16 red, green, blue;
        s32 cosine = rcos(angle);
        s32 radius = 64 + (ray & 1) * 32;

        x = radius * cosine / 4096;
        y = radius * rsin(angle) / 4096;
        rightX = rcos(angle + 128) / 128;
        rightY = rsin(angle + 128) / 128;
        leftX = rcos(angle - 128) / 128;
        leftY = rsin(angle - 128) / 128;
        centerY = centerX = 0;
        /* At least one of the three consecutive ray numbers has a nonzero remainder. */
        if ((s16)(ray % 3)) {
            red = 16;
            blue = 0;
            green = 0;
        } else if ((ray + 1) % 3) {
            red = 0;
            green = 16;
            blue = 0;
        } else if ((ray + 2) % 3) {
            red = 0;
            green = 0;
            blue = 16;
        }

        addSemiTransparentGouraudTriangle(D_8013AF6C, getTPage(0, 1, 0, 0), x, y,
            rightX, rightY, centerX, centerY, (u8)red, (u8)green, (u8)blue,
            0, 0, 0, 64, 64, 64);
        addSemiTransparentGouraudTriangle(D_8013AF6C, getTPage(0, 1, 0, 0), x, y,
            centerX, centerY, leftX, leftY, (u8)red, (u8)green, (u8)blue,
            64, 64, 64, 0, 0, 0);
        angle += 256;
        ray++;
    } while (angle < 4096);

    for (i = 0; i < 4; i++) {
        s16 halfSize = gRadialQuadHalfSizes[i];
        s16 centerY = i * 32;
        s16 centerX = -centerY;

        addTexturedQuad(D_8013AF6C, (s16)(centerX - halfSize), (s16)(centerY - halfSize),
            (s16)(halfSize + centerX), (s16)(centerY - halfSize),
            (s16)(centerX - halfSize), (s16)(halfSize + centerY),
            (s16)(halfSize + centerX), (s16)(halfSize + centerY),
            SEMI_TRANSPARENT_FLAG, getTPage(0, 1, 704, 0),
            (s16)getClut(880, 240 + gRadialQuadPaletteOffsets[i]),
            0, 0, 63, 0, 0, 63, 63, 63,
            (u8)gRadialQuadShades[i], (u8)gRadialQuadShades[i], (u8)gRadialQuadShades[i]);
    }
}

void drawPlayerParticlesTask(void) {
    PlayerParticleScratch *scratch;
    PlayerParticlePosition *positions;
    PlayerParticlePosition *position;
    RacePlayer *player;
    POLY_FT4 *packet;
    s32 i, j;
    s32 yawOffset, pitchOffset;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (PlayerParticleScratch *)getScratchAddr(0);
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    positions = (PlayerParticlePosition *)&D_800D9504[D_8013AE88->resource14];
    player = &gRacePlayers[D_8013AE88->data18];

    setVector(&scratch->corners[0], -4, -4, 0);
    setVector(&scratch->corners[1], 4, -4, 0);
    setVector(&scratch->corners[2], -4, 4, 0);
    setVector(&scratch->corners[3], 4, 4, 0);
    scratch->quad.u0 = scratch->quad.u2 = 0;
    scratch->quad.tag = TEXTURED_QUAD_PACKET_WORDS << 24;
    setcode(&scratch->quad, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG);
    scratch->quad.tpage = getTPage(0, 1, 704, 0);
    scratch->quad.clut = getClut(880, 249);
    scratch->quad.u1 = scratch->quad.u3 = 15;
    scratch->quad.v0 = scratch->quad.v1 = 128;
    scratch->quad.v2 = scratch->quad.v3 = 143;
    scratch->quad.b0 = 128;
    scratch->quad.g0 = 128;
    scratch->quad.r0 = 128;
    scratch->localScreen = GsIDMATRIX2;
    GsSetLsMatrix(&scratch->localScreen);

    yawOffset = -D_801241E8[D_8013AE88->data18].yaw;
    pitchOffset = D_801241E8[D_8013AE88->data18].pitch;
    for (i = 0; i < 40; i++) {
        if (D_8013AFAA & 0x400) {
            if (D_8013AE88->data18 != (gFrameCounter & 1)) {
                goto updatePosition;
            }
        } else if (D_8013AE88->data18 != 0) {
            goto updatePosition;
        }

        j = 0;
        position = &positions[i];
        for (; j < 4; j++) {
            s32 y;
            s32 x = (((u16)position->x + yawOffset) & 1023) - 512;
            scratch->vertices[j].vx = scratch->corners[j].vx + x;
            y = (((u16)position->y + pitchOffset) & 511) - 256;
            scratch->vertices[j].vy = scratch->corners[j].vy + y;
            scratch->vertices[j].vz = position->z & 1023;
        }
        scratch->depth = RotTransPers4(&scratch->vertices[0], &scratch->vertices[1],
            &scratch->vertices[2], &scratch->vertices[3],
            (long *)&scratch->quad.x0, (long *)&scratch->quad.x1,
            (long *)&scratch->quad.x2, (long *)&scratch->quad.x3,
            &scratch->cue, &scratch->flags);
        if (!(scratch->flags & 0x7FFBE000) && scratch->depth >= 0 && scratch->depth < 4096) {
            *packet = scratch->quad;
            addPrim(&D_8013AF6C[10], packet);
            packet++;
        }

    updatePosition:
        if (!D_8013AFD3) {
            s32 drift = randomNextLinear() % 16 + 8;
            s32 sine = rsin(D_8013AE88->data1A);
            s32 cosine = rcos(D_8013AE88->data1A * 5);
            positions[i].x += drift * (sine + cosine) / 8192;
            positions[i].y += 8;
            if (!D_8013AFD6) {
                positions[i].z -= player->unk0BC / 256;
            }
        }
    }
    GsOUT_PACKET_P = (u32)packet;
    if (!D_8013AFD3) {
        D_8013AE88->data1A++;
    }
}

void createPlayerParticleTask(s32 playerIndex) {
    PlayerParticlePosition *position;
    s32 i;
    func_80014930(&gGameTasks[4], drawPlayerParticlesTask);
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

void drawTriangleBurstTask(void) {
    u8 padding[0x50];
    ScreenPoint screen[3];
    TriangleBurstResource *resource;
    TriangleBurstScratch *scratch;
    s32 i, depth;
    if (D_8013AE88->resource14 == 0xFFFF) { while (1) {} }
    scratch = (TriangleBurstScratch *)getScratchAddr(0);
    resource = (TriangleBurstResource *)&D_800D9504[D_8013AE88->resource14];
    setVector(&scratch->vertices[0], -40, -40, 0);
    setVector(&scratch->vertices[1], 40, -40, 0);
    setVector(&scratch->vertices[2], 40, 40, 0);
    GsInitCoordinate2(0, &scratch->coordinate);
    for (i = 0; i < 5; i++) {
        RotMatrixYXZ(&resource->rotations[i], &scratch->coordinate.coord);
        setVector((VECTOR *)scratch->coordinate.coord.t, resource->positions[i].vx, resource->positions[i].vy, resource->positions[i].vz);
        GsGetLs(&scratch->coordinate, &scratch->matrix);
        GsSetLsMatrix(&scratch->matrix);
        depth = RotTransPers3(&scratch->vertices[0], &scratch->vertices[1], &scratch->vertices[2],
            (long *)&screen[0], (long *)&screen[1], (long *)&screen[2], (long *)&scratch->cue, (long *)&scratch->flags);
        scratch->depth = depth;
        if (scratch->flags >= 0 && depth >= 0 && depth < 4096) {
            func_8005FE7C(D_8013AF6C + (depth >> 2), screen[0].x, screen[0].y, screen[1].x, screen[1].y, screen[2].x, screen[2].y,
                10, getClut(640, 250), 128, 128, 191, 128, 128, 191, 255, 255, 255, 128, 128, 128, 64, 64, 64);
        }
        if (!D_8013AFD3) {
            resource->positions[i].vx += resource->velocities[i].vx;
            resource->positions[i].vy += resource->velocities[i].vy;
            resource->positions[i].vz += resource->velocities[i].vz;
            resource->velocities[i].vy += 4;
            resource->rotations[i].vx += 0x155;
            resource->rotations[i].vz += 0x155;
        }
    }
    if (!D_8013AFD3) resource->frame++;
    if (resource->frame == 30) deleteCurrentTaskSubtree();
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80063B58);

void createPositionedEffectTask(s32 x, s32 y, s32 z) {
    PositionedEffectResource *resource;

    createChildTaskWithReserve(func_80063B58);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (PositionedEffectResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->x = x >> 8;
    resource->y = y >> 8;
    resource->z = z >> 8;
}

void drawRotatingQuadEffectTask(void) {
    RotatingEffectResource *resource;
    RotatingQuadScratch *scratch;
    s32 i;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RotatingEffectResource *)&D_800D9504[D_8013AE88->resource14];
    scratch = (RotatingQuadScratch *)getScratchAddr(0);
    if (resource->elapsed <= 16) {
        scratch->textureAdvance = resource->elapsed * 63 / 16;
        scratch->displacement = resource->elapsed * 5;
        if (!D_8013AFD3) {
            resource->elapsed++;
        }
    } else {
        deleteCurrentTaskSubtree();
        return;
    }

    /* The base and rotating faces share the descending edge. */
    setVector(&scratch->vertices[0], -40, -scratch->displacement, -40);
    setVector(&scratch->vertices[1], -40, -scratch->displacement, 40);
    setVector(&scratch->vertices[2], 40, -scratch->displacement, -40);
    setVector(&scratch->vertices[3], 40, -scratch->displacement, 40);
    setVector(&scratch->vertices[4], 40, -scratch->displacement, -40);
    setVector(&scratch->vertices[5], 40, -scratch->displacement, 40);
    setVector(&scratch->vertices[6], 40, 0, -40);
    setVector(&scratch->vertices[7], 40, 0, 40);
    GsInitCoordinate2(&scratch->parent, &scratch->coordinate);
    GsInitCoordinate2(NULL, &scratch->parent);
    RotMatrix(&resource->rotation, &scratch->coordinate.coord);
    RotMatrix(&resource->parentRotation, &scratch->parent.coord);
    scratch->parent.coord.t[0] = resource->x;
    scratch->parent.coord.t[1] = resource->y;
    scratch->parent.coord.t[2] = resource->z;
    GsGetLs(&scratch->coordinate, &scratch->localScreen);
    GsSetLsMatrix(&scratch->localScreen);

    scratch->depth = RotTransPers4(&scratch->vertices[0], &scratch->vertices[1],
        &scratch->vertices[2], &scratch->vertices[3], &scratch->screen[0].packed,
        &scratch->screen[1].packed, &scratch->screen[2].packed, &scratch->screen[3].packed,
        &scratch->cue, &scratch->flags);
    if (scratch->flags >= 0 &&
        NormalClip(scratch->screen[0].packed, scratch->screen[1].packed, scratch->screen[2].packed) >= 0 &&
        scratch->depth >= 0 && scratch->depth < 4096) {
        addGouraudTexturedQuad(D_8013AF6C + (scratch->depth >> 2),
            scratch->screen[0].point.x, scratch->screen[0].point.y,
            scratch->screen[1].point.x, scratch->screen[1].point.y,
            scratch->screen[2].point.x, scratch->screen[2].point.y,
            scratch->screen[3].point.x, scratch->screen[3].point.y,
            getTPage(0, 0, 640, 0), getClut(640, 250), 128, 128, 191, 128, 128, 191, 191, 191,
            192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192);
    }

    /* Reuse the side quad at each quarter turn. */
    for (i = 0; i < 4; i++) {
        RotMatrix(&resource->rotation, &scratch->coordinate.coord);
        scratch->coordinate.flg = 0;
        GsGetLs(&scratch->coordinate, &scratch->localScreen);
        GsSetLsMatrix(&scratch->localScreen);
        scratch->depth = RotTransPers4(&scratch->vertices[4], &scratch->vertices[5],
            &scratch->vertices[6], &scratch->vertices[7], &scratch->screen[0].packed,
            &scratch->screen[1].packed, &scratch->screen[2].packed, &scratch->screen[3].packed,
            &scratch->cue, &scratch->flags);
        if (scratch->flags >= 0 &&
            NormalClip(scratch->screen[0].packed, scratch->screen[1].packed, scratch->screen[2].packed) >= 0 &&
            scratch->depth >= 0 && scratch->depth < 4096) {
            addGouraudTexturedQuad(D_8013AF6C + (scratch->depth >> 2),
                scratch->screen[0].point.x, scratch->screen[0].point.y,
                scratch->screen[1].point.x, scratch->screen[1].point.y,
                scratch->screen[2].point.x, scratch->screen[2].point.y,
                scratch->screen[3].point.x, scratch->screen[3].point.y,
                getTPage(0, 0, 640, 0), resource->clut, 192, resource->textureU, 255, resource->textureU,
                192, resource->textureU + scratch->textureAdvance,
                255, resource->textureU + scratch->textureAdvance,
                192, 192, 192, 192, 192, 192, 64, 64, 64, 64, 64, 64);
        }
        resource->rotation.vy += 0x400;
    }
}

void startRotatingQuadEffectTask(s32 variant, const VECTOR *position, s32 pitch, s32 yaw, s32 roll) {
    RotatingEffectResource *resource;
    createChildTaskWithReserve(drawRotatingQuadEffectTask);
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

void drawPlayerShadowTask(void) {
    RacePlayer *player = &gRacePlayers[D_8013AE88->data18];
    POLY_FT4 *packet = (POLY_FT4 *)GsOUT_PACKET_P;
    PlayerShadowScratch *scratch = (PlayerShadowScratch *)getScratchAddr(0);
    s32 i;
    long depth;

    if ((player->unk128 & 0x1000) && player->task->data1A == 7) {
        return;
    }
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0],
            player->worldCoordinate.coord.t[2], (s16)func_80059254())) {
        return;
    }
    *packet = gPlayerShadowPacket;
    for (i = 0; i < 4; i++) {
        ApplyMatrixSV(&player->model.effect.effectMatrix180, &gPlayerShadowVertices[i],
            &scratch->vertices[i]);
    }
    for (i = 3; i >= 0; i--) {
        scratch->vertices[i].vy = 0;
    }
    GsInitCoordinate2(0, &scratch->coordinate);
    scratch->coordinate.coord.t[0] = player->model.effect.effectMatrix180.t[0];
    scratch->coordinate.coord.t[2] = player->model.effect.effectMatrix180.t[2];
    scratch->coordinate.coord.t[1] = func_8004A764(player->unk92E,
        scratch->coordinate.coord.t[0], scratch->coordinate.coord.t[2]);
    computeSelectedTriangleAngles(&scratch->rotation.vx, &scratch->rotation.vz);
    scratch->rotation.vy = 0;
    RotMatrixZYX(&scratch->rotation, &scratch->coordinate.coord);
    scratch->size = 4096 -
        (scratch->coordinate.coord.t[1] - player->model.effect.effectMatrix180.t[1]) * 10;
    if (scratch->size < 2048) {
        scratch->size = 2048;
    }
    setVector(&scratch->scale, scratch->size, scratch->size, scratch->size);
    ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);
    GsGetLs(&scratch->coordinate, &scratch->localScreen);
    GsSetLsMatrix(&scratch->localScreen);
    depth = RotTransPers4(&scratch->vertices[0], &scratch->vertices[1],
        &scratch->vertices[2], &scratch->vertices[3], (long *)&packet->x0,
        (long *)&packet->x1, (long *)&packet->x2, (long *)&packet->x3,
        &scratch->cue, &scratch->flags);
    scratch->depth = depth;
    if (scratch->flags >= 0 && depth >= 0 && depth < 4096) {
        addPrim(&D_8013AF70[scratch->depth >> 2], packet);
    }
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void startPlayerShadowTask(s32 playerIndex) {
    func_80014930(D_8013B304, drawPlayerShadowTask);
    D_8013AE8C->data18 = playerIndex;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006479C);

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
        deleteCurrentTaskSubtree();
    } else {
        D_8013AE88->data18--;
    }
}

s32 startDelayedEightEntryEffectTask(s32 playerIndex) {
    func_80014930(&gGameTasks[4], initializeDelayedEightEntryEffectTask);
    D_8013AE8C->data18 = 1;
    D_8013AE8C->data1A = playerIndex;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800654B4);

void createPlayerPairedQuadEffectTask(s32 playerIndex, s32 variant) {
    PairedEffectResource *resource;
    RacePlayer *player;
    s32 i;
    func_80014930(D_8013B304, func_800654B4);
    D_8013AE8C->resource14 = allocateTaskResource();
    player = &gRacePlayers[playerIndex];
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

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800659F0);

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

void updateRadialEffectTask(void) {
    RadialEffectResource *resource;
    RadialEffectScratch *scratch;
    POLY_FT4 *packet;
    s32 i;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RadialEffectResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch = (RadialEffectScratch *)getScratchAddr(0);

    if (D_8013AFD3 == 0) {
        for (i = 0; i < 6; i++) {
            resource->positions[i].vx += resource->velocities[i].vx;
            resource->positions[i].vy += resource->velocities[i].vy;
            resource->positions[i].vz += resource->velocities[i].vz;
            resource->innerPositions[i].vx += resource->velocities[i].vx / 2;
            resource->innerPositions[i].vz += resource->velocities[i].vz / 2;
            if (resource->positions[i].vy < 0) {
                resource->velocities[i].vy += 4;
            } else {
                resource->positions[i].vy = 0;
                resource->velocities[i].vy = 0;
            }
        }
        resource->elapsed++;
    }

    if (isPointWithinCameraForwardRange(resource->coordinate.coord.t[0],
            resource->coordinate.coord.t[2], (s16)func_80059254())) {
        resource->coordinate.flg = 0;
        GsGetLs(&resource->coordinate, &scratch->localScreen);
        GsSetLsMatrix(&scratch->localScreen);
        for (i = 0; i < 6; i++) {
            u8 shade;

            /* Clear the DMA link while setting the packet length. */
            packet->tag = TEXTURED_QUAD_PACKET_WORDS << 24;
            setcode(packet, POLY_FT4_CODE);
            setSemiTrans(packet, 1);
            packet->tpage = getTPage(0, 1, 704, 0);
            packet->clut = getClut(880, 246);
            setUV4(packet, 128, 0, 191, 0, 128, 63, 191, 63);
            shade = 255 - resource->elapsed * 255 / 30;
            setGray0(packet, shade);

            if (i < 5) {
                *(volatile long *)&scratch->depth = RotTransPers4(
                    &resource->positions[i], &resource->positions[i + 1],
                    &resource->innerPositions[i], &resource->innerPositions[i + 1],
                    (long *)&packet->x0, (long *)&packet->x1, (long *)&packet->x2,
                    (long *)&packet->x3, &scratch->cue, &scratch->flags);
            } else {
                /* Close the ring with the first pair of vertices. */
                *(volatile long *)&scratch->depth = RotTransPers4(
                    &resource->positions[5], &resource->positions[0],
                    &resource->innerPositions[5], &resource->innerPositions[0],
                    (long *)&packet->x0, (long *)&packet->x1, (long *)&packet->x2,
                    (long *)&packet->x3, &scratch->cue, &scratch->flags);
            }
            if (*(volatile long *)&scratch->flags >= 0 && scratch->depth >= 0 && scratch->depth < 4096) {
                addPrim(&D_8013AF6C[scratch->depth >> 2], packet);
            }
            packet++;
        }
        GsOUT_PACKET_P = (u32)packet;
    }

    if (resource->elapsed >= 30) {
        deleteCurrentTaskSubtree();
    }
}

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
        func_80014930(D_8013B304, updateRadialEffectTask);
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
            deleteCurrentTaskSubtree();
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

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80066474);

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

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_80066D18);

void createPlayerPositionTrailTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerPositionTrailResource *resource;
    s32 i, j;

    player = &gRacePlayers[playerIndex];
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

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800676FC);

void emitPlayerTurnTripleQuadEffectTask(void) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i, sine;
    player = &gRacePlayers[D_8013AE88->data18];
    if (D_8013AFD3 != 0) return;
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)func_80059254())) return;
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) return;
    sine = rsin((s16)player->unk0BE - (s16)player->unk110);
    if (sine < 0) sine = -sine;
    if ((gFrameCounter & 3) == 0 && !(player->unk128 & 0x677DA21) && player->unk0BC >= 0x800 && sine >= 0x200) {
        func_80014930(D_8013B304, func_800676FC);
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
        resource->state = player->unk96E;
        resource->scale = rsin((s16)player->unk0BE - (s16)player->unk110);
        if (resource->scale < 0) resource->scale = -resource->scale;
        D_8013AE8C->data18 = D_8013AE88->data18;
    }
}

void startPlayerTurnTripleQuadEffectTask(s32 playerIndex) {
    func_80014930(D_8013B304, emitPlayerTurnTripleQuadEffectTask);
    D_8013AE8C->data18 = playerIndex;
}

void updatePlayerRingEffectTask(void) {
    PlayerRingEffectResource *resource;
    PlayerRingEffectScratch *scratch;
    POLY_FT4 *packet;
    s32 i, j, angle;
    s32 scale;
    u8 shade;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerRingEffectResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch = (PlayerRingEffectScratch *)getScratchAddr(0);
    if (isPointWithinCameraForwardRange(resource->position[0], resource->position[2],
                                        (s16)func_80059254())) {
        scratch->quad.tag = TEXTURED_QUAD_PACKET_WORDS << 24;
        setcode(&scratch->quad, POLY_FT4_CODE);
        setSemiTrans(&scratch->quad, 1);
        scratch->quad.clut = getClut(880, 248);
        scratch->quad.tpage = getTPage(0, 1, 704, 0);
        setUV4(&scratch->quad, 0, 64, 63, 64, 0, 127, 63, 127);
        shade = 128 - (resource->elapsed << 7) / 20;
        /* Preserve the halfword store order for the two top vertices. */
        scratch->vertices[0].vx = -20;
        scratch->vertices[0].vz = 0;
        scratch->vertices[0].vy = -20;
        setVector(&scratch->vertices[1], -20, 20, 0);
        scratch->vertices[2].vx = 20;
        scratch->vertices[2].vz = 0;
        scratch->vertices[2].vy = -20;
        setVector(&scratch->vertices[3], 20, 20, 0);
        /* The grayscale channels are written in BGR order. */
        scratch->quad.b0 = shade;
        scratch->quad.g0 = shade;
        scratch->quad.r0 = shade;
        GsInitCoordinate2(0, &scratch->coordinate);
        scale = 4096 + (resource->elapsed << 12) / 20;
        scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = scale;
        for (i = 0, angle = 0; i < 6; i++, angle += 0x2AA) {
            scratch->coordinate.coord.t[0] =
                resource->position[0] + (resource->elapsed * (rcos(angle) << 2)) / 4096;
            scratch->coordinate.coord.t[1] = resource->position[1];
            scratch->coordinate.coord.t[2] =
                resource->position[2] + (resource->elapsed * (rsin(angle) << 2)) / 4096;
            GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
            for (j = 0; j < 3; j++) {
                scratch->coordinate.coord.t[j] = scratch->coordinate.workm.t[j];
            }
            RotMatrix(&resource->rotations[i], &scratch->coordinate.coord);
            ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);
            GsSetLsMatrix(&scratch->coordinate.coord);
            scratch->depth = RotTransPers4(
                &scratch->vertices[0], &scratch->vertices[1],
                &scratch->vertices[2], &scratch->vertices[3],
                (long *)&scratch->quad.x0, (long *)&scratch->quad.x1,
                (long *)&scratch->quad.x2, (long *)&scratch->quad.x3,
                &scratch->cue, &scratch->flags);
            *packet = scratch->quad;
            if (!(scratch->flags & 0x7FFBE000) && scratch->depth >= 0 && scratch->depth < 4096) {
                addPrim(&D_8013AF6C[scratch->depth >> 2], packet);
            }
            packet++;
        }
        GsOUT_PACKET_P = (u32)packet;
    }
    if (D_8013AFD3 == 0) {
        resource->position[1] += (-rcos((resource->elapsed << 11) / 20) * 10) / 4096;
        if (D_8013AFD3 == 0) {
            resource->elapsed++;
        }
    }
    if (resource->elapsed >= 21) {
        deleteCurrentTaskSubtree();
    }
}

void createPlayerRingEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerRingEffectResource *resource;
    s32 i;
    player = &gRacePlayers[playerIndex];
    if (D_8013AFCE != 3) {
        func_80014930(D_8013B304, updatePlayerRingEffectTask);
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

void updatePlayerParticleTrailTask(void) {
    PlayerParticleTrailResource *resource;
    PlayerParticleTrailScratch *scratch;
    RacePlayer *player;
    POLY_FT4 *packet;
    s32 i, j;
    s32 shade;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (PlayerParticleTrailScratch *)getScratchAddr(0);
    resource = (PlayerParticleTrailResource *)&D_800D9504[D_8013AE88->resource14];
    player = &gRacePlayers[D_8013AE88->data18];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch->quad.tag = TEXTURED_QUAD_PACKET_WORDS << 24;
    setcode(&scratch->quad, POLY_FT4_CODE);
    setSemiTrans(&scratch->quad, 1);
    scratch->quad.clut = getClut(880, 248);
    scratch->quad.tpage = getTPage(0, 1, 704, 0);
    setUV4(&scratch->quad, 0, 64, 63, 64, 0, 127, 63, 127);
    setVector(&scratch->vertices[0], -20, -20, 0);
    setVector(&scratch->vertices[1], -20, 20, 0);
    setVector(&scratch->vertices[2], 20, -20, 0);
    setVector(&scratch->vertices[3], 20, 20, 0);

    if (D_8013AFD3 == 0) {
        for (j = 9; j > 0; j--) {
            s32 x;
            s32 *current;
            resource->ages[j] = resource->ages[j - 1];
            resource->angles[j] = resource->angles[j - 1];
            current = resource->positions[j];
            x = resource->positions[j - 1][0];
            current[0] = player->unk0FF ? x + 20 : x - 20;
            resource->positions[j][1] = resource->positions[j - 1][1] + randomNextLinear() % 8 - 8;
            resource->positions[j][2] = resource->positions[j - 1][2] + randomNextLinear() % 8 - 4;
        }
        if (player->unk0FC && isPointWithinCameraForwardRange(
                player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2],
                (s16)func_80059254())) {
            resource->ages[0] = 0;
            resource->angles[0] = randomNextLinear() % 4096;
            resource->positions[0][0] = player->unk0FF ? 60 : -60;
            resource->positions[0][1] = -20;
            resource->positions[0][2] = 0;
        } else {
            resource->ages[0] = 10;
        }
    }

    scratch->rotation.vy = 0;
    scratch->rotation.vx = 0;
    GsInitCoordinate2(&player->model.modelCoordinates[0], &scratch->coordinate);
    for (i = 0; i < 10; i++) {
        if (resource->ages[i] < 10) {
            scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = 4096 + (i << 10);
            shade = 32 - (i << 5) / 10;
            scratch->quad.b0 = shade;
            scratch->quad.g0 = shade;
            scratch->quad.r0 = shade;
            /* The original reads the reused loop index before resetting it below. */
            scratch->rotation.vz = resource->angles[j];
            scratch->coordinate.flg = 0;
            for (j = 0; j < 3; j++) {
                scratch->coordinate.coord.t[j] = resource->positions[i][j];
            }
            GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
            for (j = 0; j < 3; j++) {
                scratch->coordinate.coord.t[j] = scratch->coordinate.workm.t[j];
            }
            RotMatrix(&scratch->rotation, &scratch->coordinate.coord);
            ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);
            GsSetLsMatrix(&scratch->coordinate.coord);
            scratch->depth = RotTransPers4(
                &scratch->vertices[0], &scratch->vertices[1],
                &scratch->vertices[2], &scratch->vertices[3],
                (long *)&scratch->quad.x0, (long *)&scratch->quad.x1,
                (long *)&scratch->quad.x2, (long *)&scratch->quad.x3,
                &scratch->cue, &scratch->flags);
            if (!(scratch->flags & 0x7FFBE000) && scratch->depth >= 0 && scratch->depth < 4096) {
                *packet = scratch->quad;
                addPrim(&D_8013AF6C[scratch->depth >> 2], packet);
            }
            packet++;
            if (D_8013AFD3 == 0) {
                resource->positions[i][1] -= 2;
                if (resource->ages[i] < 11) {
                    resource->ages[i]++;
                }
            }
        }
    }
    GsOUT_PACKET_P = (u32)packet;
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_800688C0);

void beginPlayerTripleQuadEffectTask(void) {
    setCurrentTaskCallback(func_800676FC);
}

s32 createPlayerTripleQuadEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i;
    s32 state;
    player = &gRacePlayers[playerIndex];
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)func_80059254())) {
        return 0;
    }
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) {
        return 0;
    }
    func_80014930(D_8013B304, beginPlayerTripleQuadEffectTask);
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

void drawRotatingQuadTrailTask(void) {
    s32 i, j;
    s32 rotation;
    POLY_FT4 *packet;
    RotatingQuadTrailResource *resource;
    QuadScaleScratchpad *scratch;

    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (QuadScaleScratchpad *)getScratchAddr(0);
    resource = (RotatingQuadTrailResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    for (i = 7; i > 0; i--) {
        resource->angles[i] = resource->angles[i - 1];
        resource->scales[i] = resource->scales[i - 1];
    }
    if (resource->countdown >= 0) {
        resource->angles[0] = (resource->countdown * resource->countdown) << 8;
        resource->scales[0] = resource->countdown * 200 + 0x1000;
        rotation = resource->countdown << 8;
        resource->angles[1] = rotation + 128;
    } else {
        resource->angles[0] = 0;
        resource->scales[0] = 0x1000;
    }
    for (i = 0; i < 8; i++) {
        scratch->rotation.vx = 0;
        scratch->rotation.vy = 0;
        scratch->rotation.vz = resource->angles[i];
        scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = resource->scales[i];
        RotMatrix(&scratch->rotation, &scratch->matrix);
        ScaleMatrix(&scratch->matrix, &scratch->scale);
        for (j = 0; j < 4; j++) {
            ApplyMatrixSV(&scratch->matrix, &gRotatingQuadTrailVertices[j], &scratch->vertices[j]);
        }
        /* Initialize the complete DMA tag, including its link address. */
        packet->tag = TEXTURED_QUAD_PACKET_WORDS << 24;
        setcode(packet, POLY_FT4_CODE);
        setSemiTrans(packet, 1);
        setUV4(packet, 0, 0, 215, 0, 0, 15, 215, 15);
        if (i & 2) {
            packet->tpage = getTPage(0, 0, 960, 0);
            setGray0(packet, 255 - i * 16);
            packet->clut = getClut(944, 240);
        } else {
            setGray0(packet, 128 - i * 16);
            packet->tpage = getTPage(0, 1, 960, 0);
            packet->clut = getClut(944, 240);
        }
        setXY4(packet,
            scratch->vertices[0].vx, scratch->vertices[0].vy,
            scratch->vertices[1].vx, scratch->vertices[1].vy,
            scratch->vertices[2].vx, scratch->vertices[2].vy,
            scratch->vertices[3].vx, scratch->vertices[3].vy);
        addPrim(D_8013AF6C, packet);
        packet++;
    }
    GsOUT_PACKET_P = (u32)packet;
    if (--resource->countdown < -7) {
        setCurrentTaskCallback(drawRisingTexturedStripTask);
        resource->countdown = 0;
    }
}

void createRotatingQuadTrailTask(void) {
    RotatingQuadTrailResource *resource;
    s32 i;

    func_80014930(&gGameTasks[4], drawRotatingQuadTrailTask);
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
    scratch = (QuadScaleScratchpad *)getScratchAddr(0);
    resource = (QuadTransitionResource *)&D_800D9504[D_8013AE88->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch->rotation.vx = scratch->rotation.vy = scratch->rotation.vz = 0;
    scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = resource->frames * 1000 + 0x1000;
    RotMatrix(&scratch->rotation, &scratch->matrix);
    ScaleMatrix(&scratch->matrix, &scratch->scale);
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

void waitThenStartPaletteColorCycle(void) {
    if (++D_8013AE88->data1A >= 90) {
        setCurrentTaskCallback(capturePaletteForColorCycle);
        D_8013AE88->data1A = 0;
    }
}

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
        gWhitePulsePalette[i] = r | (g << 5) | (b << 10);
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

void startPaletteWhitePulseTask(s32 duration) {
    func_80014930(&gGameTasks[4], capturePaletteForColorCycle);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
}

void updateQuadTransitionSequenceTask(void) {
    s32 frame = D_8013B338;
    if (frame == 0) createRotatingQuadTrailTask();
    if (frame == 25) createQuadShrinkTask();
    if (frame == 30) startMiddleQuadShrinkTask();
    if (frame == 35) createRightQuadShrinkTask();
    if (frame == 40) startSlidingQuadTask(D_8013AE88->data1A);
    if (frame == 50) startPaletteWhitePulseTask(10);
    D_8013B338++;
    if (D_8013B338 == 100) deleteCurrentTaskSubtree();
}

void startQuadTransitionSequenceTask(s32 transitionIndex) {
    func_80014930(&gGameTasks[4], updateQuadTransitionSequenceTask);
    D_8013B338 = 0;
    D_8013AE8C->data1A = transitionIndex;
}

void updatePrimaryValueTransition(void) {
    s32 value;
    value = D_8013AE88->data1C +
        (D_8013AE88->data1E - D_8013AE88->data1C) *
        D_8013AE88->data18 / D_8013AE88->data1A;
    D_8013B35C = value;
    D_8013AE88->data18++;
    D_8013B364 = 0;
    if (D_8013AE88->data18 >= D_8013AE88->data1A) {
        deleteCurrentTaskSubtree();
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
        deleteCurrentTaskSubtree();
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
        deleteCurrentTaskSubtree();
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
    DrawModeSpritePacket *scratch = (DrawModeSpritePacket *)getScratchAddr(0), *packet;
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
        deleteCurrentTaskSubtree();
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
        deleteCurrentTaskSubtree();
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
        deleteCurrentTaskDescendants();
        deleteCurrentTaskSubtree();
    }
}

void updateTimedSpritePresentation(void) {
    s32 frame = D_8013AE88->data18;

    gTimedSpritePresentationFrame = frame;
    if (frame == 0) {
        startValueInterpolationTask(15, 0, 64);
        createFullScreenSpriteTask(120, getTPage(1, 1, 320, 256), getClut(320, 496),
            0, 0, &D_8013B360);
    }
    if (frame == 75) {
        startValueInterpolationTask(14, 64, 255);
    }
    if (frame == 90) {
        startValueInterpolationTask(30, 255, 0);
        startPrimaryValueTransition(30, 0, 128);
        createFullScreenSpriteTask(1410, getTPage(1, 1, 320, 0), getClut(320, 240),
            0, 0, &D_8013B35C);
    }
    if (frame == 180) {
        startPrimaryValueTransition(30, 128, 64);
    }
    if (frame == 0) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame0);
    }
    if (frame == 210) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame210);
    }
    if (frame == 1440) {
        startPrimaryValueTransition(30, 64, 0);
        startValueInterpolationTask(30, 0, 128);
        uploadTimImageAndPaletteAt(getOffsetArchiveEntry(DAT_ANIMATION_ARCHIVE, 0),
            320, 256, 320, 496);
        createFullScreenSpriteTask(660, getTPage(1, 1, 320, 256), getClut(320, 496),
            0, 0, &D_8013B360);
    }
    if (frame == 1530) {
        startValueInterpolationTask(30, 128, 64);
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame1530);
    }
    if (frame == 1800) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame1800);
    }
    if (frame == 2040) {
        startValueInterpolationTask(60, 64, 0);
        startPrimaryValueTransition(30, 0, 128);
        uploadTimImageAndPaletteAt(getOffsetArchiveEntry(DAT_ANIMATION_ARCHIVE, 1),
            320, 0, 320, 240);
        createFullScreenSpriteTask(600, getTPage(1, 1, 320, 0), getClut(320, 240),
            0, 0, &D_8013B35C);
    }
    if (frame == 2130) {
        startPrimaryValueTransition(30, 128, 64);
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame2130);
    }
    if (frame == 2430) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame2430);
    }
    if (frame == 2580) {
        startPrimaryValueTransition(60, 64, 0);
        startValueInterpolationTask(30, 0, 128);
        uploadTimImageAndPaletteAt(getOffsetArchiveEntry(DAT_ANIMATION_ARCHIVE, 2),
            320, 256, 320, 496);
        createFullScreenSpriteTask(780, getTPage(1, 1, 320, 256), getClut(320, 496),
            0, 0, &D_8013B360);
    }
    if (frame == 2670) {
        startValueInterpolationTask(30, 128, 64);
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame2670);
    }
    if (frame == 2820) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame2820);
    }
    if (frame == 2970) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame2970);
    }
    if (frame == 3150) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame3150);
    }
    if (frame == 3300) {
        startValueInterpolationTask(60, 64, 0);
        startPrimaryValueTransition(30, 0, 128);
        uploadTimImageAndPaletteAt(getOffsetArchiveEntry(DAT_ANIMATION_ARCHIVE, 3),
            320, 0, 320, 240);
        createFullScreenSpriteTask(390, getTPage(1, 1, 320, 0), getClut(320, 240),
            0, 0, &D_8013B35C);
    }
    if (frame == 3390) {
        startPrimaryValueTransition(30, 128, 64);
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame3390);
    }
    if (frame == 3510) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame3510);
    }
    if (frame == 3630) {
        startPrimaryValueTransition(60, 64, 0);
        startValueInterpolationTask(30, 0, 128);
        uploadTimImageAndPaletteAt(getOffsetArchiveEntry(DAT_ANIMATION_ARCHIVE, 4),
            320, 256, 320, 496);
        createFullScreenSpriteTask(1230, getTPage(1, 1, 320, 256), getClut(320, 496),
            0, 0, &D_8013B360);
    }
    if (frame == 3720) {
        startValueInterpolationTask(30, 128, 64);
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame3720);
    }
    if (frame == 4110) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame4110);
    }
    if (frame == 4230) {
        startTimedSpriteSequence(gTimedSpriteStepsAtFrame4230);
    }
    if (frame == 4710) {
        startValueInterpolationTask(60, 64, 0);
    }
    if (frame == 4740) {
        startPrimaryValueTransition(60, 0, 128);
        uploadTimImageAndPaletteAt(getOffsetArchiveEntry(DAT_ANIMATION_ARCHIVE, 5),
            320, 0, 320, 240);
        createFullScreenSpriteTask(300, getTPage(1, 1, 320, 0), getClut(320, 240),
            0, 0, &D_8013B35C);
    }

    if (frame == 4950 || (D_8013AEE8[0] & PRESENTATION_SKIP_BUTTON)) {
        D_8013B36C = 1;
        D_8013AE88->data18 = 0;
        deleteCurrentTaskDescendants();
        startPrimaryValueTransition(30, (s16)D_8013B35C, 0);
        startValueInterpolationTask(30, (s16)D_8013B360, 0);
        setCurrentTaskCallback(finishTimedSpritePresentation);
    } else {
        D_8013B368 = 0;
        D_8013B364 = 0;
        D_8013AE88->data18++;
    }
}

void initializeTimedSpritePresentation(void) {
    setSecondaryDisplayTaskMode(0);
    queueCdCommand(6, 1);
    D_8013B35C = 0;
    D_8013B360 = 0;
    D_8013B364 = 0;
    D_8013B368 = 0;
    D_8013B36C = 0;
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(updateTimedSpritePresentation);
}

void loadTimedSpritePresentationResources(void) {
    setSecondaryDisplayTaskMode(1);
    queueCdSectorCommand(1, DAT_ENTRY_177_SECTOR, DAT_ENTRY_177_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(1, DAT_ENTRY_178_SECTOR, DAT_ENTRY_178_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(1, DAT_ENTRY_232_SECTOR, DAT_ENTRY_232_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(1, DAT_ENTRY_233_SECTOR, DAT_ENTRY_233_SECTOR_COUNT, DAT_SCRATCH, 0);
    DrawSync(0);
    queueCdSectorCommand(0, DAT_PLAYER_ANIMATION_SECTOR, DAT_PLAYER_ANIMATION_SECTOR_COUNT, DAT_SCRATCH, 0);
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
    sprite->clut = getClut(0x300, gTextGlyphPalette + 0xF0);
    addPrim(D_8013AF6C, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

void addStretchedTextCharacter(s32 x, s32 y, s32 width, u32 character) {
    POLY_FT4 *quad;
    width = x + width - 1;
    quad = (POLY_FT4 *)GsOUT_PACKET_P;
    quad->tag = (sizeof(POLY_FT4) / sizeof(u32) - 1) << 24;
    setcode(quad, POLY_FT4_CODE);
    quad->clut = getClut(768, gTextGlyphPalette + 240);
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

/* %w and %p set glyph width and palette. A digit from 1 to 9 sets the numeric field width. */
void drawStretchedDebugText(s32 x, s32 y, const char *format, ...) {
    va_list args;
    u8 character;
    u8 digits;
    s32 i;
    s32 value;
    u8 byte;
    u32 digit;
    const u8 *string;
    DR_TPAGE *page;

    x = gStretchedTextWidth * x - 160;
    y = gStretchedTextWidth * y - 120;
    va_start(args, format);

    while ((character = *format) != 0) {
        if (character == '%') {
            format++;
            digits = 4;
            if ((u32)((character = *format) - '1') < 9) {
                digits = character - '0';
                format++;
            }

            switch ((u8)*format) {
            case '%':
                addStretchedTextCharacter((s16)x, (s16)y, (s16)gStretchedTextWidth, '%');
                x += gStretchedTextWidth;
                format++;
                break;
            case 'w':
                format++;
                gStretchedTextWidth = (s8)*format - '0';
                format++;
                break;
            case 'd':
                value = va_arg(args, s32);
                if (value < 0) {
                    addStretchedTextCharacter((s16)x, (s16)y, (s16)gStretchedTextWidth, '-');
                    value = -value;
                }
                for (i = digits; i > 0; i--) {
                    digit = value % 10 + '0';
                    value /= 10;
                    addStretchedTextCharacter((s16)(x + i * (s16)gStretchedTextWidth), (s16)y,
                                              (s16)gStretchedTextWidth, (u8)digit);
                }
                x += (digits + 1) * (s16)gStretchedTextWidth;
                format++;
                break;
            case 'D':
                byte = va_arg(args, u8);
                for (i = digits; i > 0; i--) {
                    digit = (u32)byte % 10 + '0';
                    byte = (u32)byte / 10;
                    addStretchedTextCharacter((s16)(x + i * (s16)gStretchedTextWidth), (s16)y,
                                              (s16)gStretchedTextWidth, (u8)digit);
                }
                x += (digits + 1) * (s16)gStretchedTextWidth;
                format++;
                break;
            case 'x':
                value = va_arg(args, s32);
                if (value < 0) {
                    addStretchedTextCharacter((s16)x, (s16)y, (s16)gStretchedTextWidth, '-');
                    value = -value;
                }
                for (i = digits; i > 0; i--) {
                    digit = value % 16;
                    value /= 16;
                    if (digit < 10) {
                        addStretchedTextCharacter((s16)(x + i * (s16)gStretchedTextWidth), (s16)y,
                                                  (s16)gStretchedTextWidth, (u8)(digit + '0'));
                    } else {
                        addStretchedTextCharacter((s16)(x + i * (s16)gStretchedTextWidth), (s16)y,
                                                  (s16)gStretchedTextWidth, (u8)(digit + 'a' - 10));
                    }
                }
                x += (digits + 1) * (s16)gStretchedTextWidth;
                format++;
                break;
            case 'X':
                byte = va_arg(args, u8);
                for (i = digits; i > 0; i--) {
                    digit = byte & 15;
                    byte >>= 4;
                    if (digit < 10) {
                        addStretchedTextCharacter((s16)(x + i * (s16)gStretchedTextWidth), (s16)y,
                                                  (s16)gStretchedTextWidth, (u8)(digit + '0'));
                    } else {
                        addStretchedTextCharacter((s16)(x + i * (s16)gStretchedTextWidth), (s16)y,
                                                  (s16)gStretchedTextWidth, (u8)(digit + 'a' - 10));
                    }
                }
                x += (digits + 1) * (s16)gStretchedTextWidth;
                format++;
                break;
            case 's':
                string = va_arg(args, const u8 *);
                while (*string) {
                    addStretchedTextCharacter((s16)x, (s16)y, (s16)gStretchedTextWidth, *string);
                    string++;
                    x += gStretchedTextWidth;
                }
                format++;
                break;
            case 'p':
                format++;
                gTextGlyphPalette = (s8)*format - '0';
                format++;
                break;
            }
        } else {
            addStretchedTextCharacter((s16)x, (s16)y, (s16)gStretchedTextWidth, character);
            x += gStretchedTextWidth;
            format++;
        }
    }
    va_end(args);

    page = (DR_TPAGE *)GsOUT_PACKET_P;
    setDrawTPage(page, 0, 0, getTPage(0, 0, 960, 256));
    addPrim(D_8013AF6C, page);
    GsOUT_PACKET_P = (u32)(page + 1);
}

INCLUDE_ASM("asm/nonmatchings/race/effects/player_effects", func_8006B9EC);
