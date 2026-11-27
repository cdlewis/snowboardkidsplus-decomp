#include "text_encoding.h"
#include "psx/libetc.h"
#include "psx/stdarg.h"
#include "../../graphics/render_effects_internal.h"
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

extern s8 gActiveRacePlayerCount __attribute__((section(".data")));

#define TRAIL_FACING_BACKWARD 0x400
#define TURN_TRAIL_SAMPLES 8
#define TURN_TRAIL_LIFETIME 32
#define PRESENTATION_SKIP_BUTTON 0x0800
#define PAIRED_QUAD_PROJECTION_FLAGS 0x7FFBE000

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


typedef struct TurnTrailHistory {
    VECTOR positions[TURN_TRAIL_SAMPLES];
    SVECTOR velocities[TURN_TRAIL_SAMPLES];
    s32 ages[TURN_TRAIL_SAMPLES];
    s32 elapsed;
} TurnTrailHistory;

typedef struct TurnTrailEdges {
    SVECTOR offsets[TURN_TRAIL_SAMPLES];
    u16 tpages[TURN_TRAIL_SAMPLES];
    u16 cluts[TURN_TRAIL_SAMPLES];
    u8 textureU[TURN_TRAIL_SAMPLES];
} TurnTrailEdges;

typedef union TrailScreenPoint {
    ScreenPoint xy;
    long packed;
} TrailScreenPoint;

typedef struct TurnTrailScratch {
    GsCOORDINATE2 coordinate;
    TrailScreenPoint edge[TURN_TRAIL_SAMPLES], base[TURN_TRAIL_SAMPLES];
    long depth[TURN_TRAIL_SAMPLES], flags[TURN_TRAIL_SAMPLES], cue;
    s32 emitting, strength, side, facing;
    SVECTOR offset;
    u8 u[4], v[4];
    u16 tpage, clut;
} TurnTrailScratch;

typedef struct BoardTrailScratch {
    MATRIX localScreen;
    u8 pad20[0x20];
    SVECTOR offset;
    GsCOORDINATE2 coordinate;
    union {
        ScreenPoint point;
        long packed;
    } screen[4];
    long depth[4];
    long averageDepth, cue;
    union {
        s32 region;
        long flags;
    } result;
    ScreenPoint uv[4];
} BoardTrailScratch;

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

typedef struct PairedQuadScratchpad {
    GsCOORDINATE2 coordinate;
    u8 padding[0x20];
    s32 depth;
    long cue;
    long flags;
} PairedQuadScratchpad;

typedef struct QuadAnimationScratch {
    GsCOORDINATE2 coordinate;
    VECTOR scale;
    long depth;
    long cue;
    long flags;
    union {
        const QuadAnimationFrame *volatile current;
        const volatile QuadAnimationFrame *forScale;
    } frame;
} QuadAnimationScratch;

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
    s32 unkA0;
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

typedef struct PlayerPositionTrailScratch {
    GsCOORDINATE2 coordinate;
    POLY_FT4 quad;
    SVECTOR rotation;
    VECTOR scale;
    long depth;
    long cue, flags;
    s32 size;
    s32 shade;
    union {
        SVECTOR vector;
        s16 components[4];
    } offset;
} PlayerPositionTrailScratch;

typedef struct PlayerTripleQuadScratch {
    u32 pad[8];
    VECTOR scale;
    GsCOORDINATE2 coordinate;
    POLY_FT4 quad;
    long depth, cue, flags;
} PlayerTripleQuadScratch;

extern s16 gFrameCounter __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u32 *gCurrentPlayerOrderingTable __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern u32 gTaskTransitionFlags __attribute__((section(".data")));
extern u8 gSelectedPlayerReplayActive __attribute__((section(".data")));

extern u16 gGameStateFlags __attribute__((section(".data")));
extern u32 *gCurrentPlayerDepthOffsetOrderingTable __attribute__((section(".data")));

const VECTOR gInitialRaceOscillatingQuadPosition
    __attribute__((section(".rodata.race_oscillating_quad_position"))) = {
    -0x1B25, 0x3557, 0x9E5E, 0
};

const FadingTextGlyphBanks gFadingTextGlyphBanks
    __attribute__((section(".rodata.fading_text_glyph_banks"))) = {
    {gDoubleByteGlyphCodes16x16Page0, gDoubleByteGlyphCodes16x16Page1, gDoubleByteGlyphCodes16x16Page2, gDoubleByteGlyphCodes16x16Page3}
};

/* Shift-JIS captions with the original four-byte padding. */
const char gTimedSpritePresentationText[0x498]
    __attribute__((section(".rodata.timed_sprite_text"), aligned(4))) =
    _("スノボキッズ  プラス\000\000\000\000−スタッフ−\000\000\000\000キャスト\000\000\000\000スラッシュ\000\000森田  千明\000\000ナンシー／女の子Ａ\000\000石"
      "川  静\000\000\000\000リンダ／カエデ／女の子Ｂ\000\000\000\000夏樹  リオ\000\000ジャム／シノビン／担任の先生\000\000\000\000諏訪部  順一\000\000\000"
      "\000トミィ／ルビー／犬店長\000\000遠近  孝一\000\000田口  宏子\000\000ニコル／スラッシュの母／男の子\000\000吉田  古奈美\000\000\000\000ナレ"
      "ーション\000\000\000\000ＤＪ  ナイク\000\000\000\000メインプランナー\000\000\000\000服部  由民狐\000\000\000\000アシスタントプランナー\000\000小森  成"
      "雄\000\000メインプログラマー\000\000アシスタントプログラマー\000\000\000\000みやじまみのる\000\000ケソまる\000\000\000\000３Ｄグラフィック\000\000\000\000わ"
      "だ  さとし\000\000\000\000岩崎  智美\000\000赤木  義尚\000\000大槻帝国\000\000\000\000２Ｄグラフィック\000\000\000\000おかちう\000\000\000\000キャラクター"
      "原案\000\000\000\000ゲーム中原画／スタッフロール原画\000\000\000\000モーション\000\000堀之内君\000\000\000\000サウンドプロデューサー\000\000笠井  勲\000"
      "\000\000\000ＴＯＭＯＨＩＫＯ  ＳＡＴＯ\000\000山本  小百合\000\000\000\000プロジェクトリーダー\000\000\000\000ディレクター\000\000\000\000矢山  秀樹"
      "\000\000村田  武司\000\000ＢＧＭ  オープニング・エンディング\000\000テストプレイヤー\000\000\000\000小倉  宏計\000\000平野  聖二\000\000金澤"
      "  貴敏\000\000三浦  もえ\000\000足立  純\000\000\000\000テストプレイヤーチーフ\000\000山本  晃康\000\000テストプレイヤーマネージャー\000\000"
      "\000\000二木  隆幸\000\000ムービー／イベントグラフィック\000\000アースリープロダクション\000\000\000\000マニュアルデザイン\000\000永島  政男"
      "\000\000タイトルデザイン\000\000\000\000石井  史人\000\000プロモーション\000\000島村  大三\000\000プロデューサー\000\000田中  裕之\000\000");

char gTimedSpriteAlternateText[4][8]
    __attribute__((section(".data.timed_sprite_alternate_text"), aligned(4))) = {
    _("パメラ\000\000"),
    _("いかＢ\000\000"),
    _("つる田\000\000"),
    _("蛙稲\000\000\000\000"),
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

/* Unreferenced duplicate endpoints and scale beside the snapshot trail data. */
SVECTOR D_8012C864[2]
    __attribute__((section(".data.player_snapshot_trail_tail"))) = {
    {0, 0, -20, 0}, {0, 0, 20, 0},
};

VECTOR D_8012C874
    __attribute__((section(".data.player_snapshot_trail_tail"))) = {
    0x2000, 0x2000, 0x2000, 0,
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

SVECTOR gPlayerPairedQuadVertices[4]
    __attribute__((section(".data.player_paired_quad_vertices"))) = {
    {-10, -10, 0, 0}, {10, -10, 0, 0},
    {-10, 10, 0, 0}, {10, 10, 0, 0},
};

VECTOR gPlayerPairedQuadOffsets[2]
    __attribute__((section(".data.player_paired_quad_offsets"))) = {
    {-50, 0, 0, 0}, {50, 0, 0, 0}
};

SVECTOR gPlayerPositionTrailVertices[4]
    __attribute__((section(".data.player_position_trail_vertices"))) = {
    {-20, -20, 0, 0}, {20, -20, 0, 0},
    {-20, 20, 0, 0}, {20, 20, 0, 0},
};

SVECTOR gPlayerTripleQuadVertices[4]
    __attribute__((section(".data.player_triple_quad_vertices"))) = {
    {-20, -20, 0, 0}, {20, -20, 0, 0},
    {-20, 20, 0, 0}, {20, 20, 0, 0},
};

SVECTOR gPlayerTripleQuadOffsets[3]
    __attribute__((section(".data.player_triple_quad_offsets"))) = {
    {-40, 0, 0, 0}, {40, 0, 0, 0}, {0, 0, 0, 0}
};

POLY_FT4 gRotatingQuadTrailPacket
    __attribute__((section(".data.rotating_quad_trail_packet"))) = {
    TEXTURED_QUAD_PACKET_WORDS << 24, 0, 0, 0,
    TEXTURED_QUAD_CODE | SEMI_TRANSPARENT_FLAG,
    0, 0, 0, 0, getClut(944, 240),
    0, 0, 215, 0, getTPage(0, 0, 960, 0),
    0, 0, 0, 15, 0,
    0, 0, 215, 15, 0,
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

SVECTOR gQuadAnimationVertices[4]
    __attribute__((section(".data.quad_animation_vertices"))) = {
    {-20, -20, 0, 0}, {20, -20, 0, 0},
    {-20, 20, 0, 0}, {20, 20, 0, 0},
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

s32 gPlayerSnapshotTrailCount __attribute__((section(".data.player_snapshot_tasks"))) = 0;

GameTask *gPlayerSnapshotTask __attribute__((section(".data.player_snapshot_tasks"))) = 0;

u16 gWhitePulsePalette[16]
    __attribute__((section(".bss.white_pulse_palette"), aligned(4)));

u16 gRadialQuadHalfSizes[4]
    __attribute__((section(".data.radial_effect_quads"))) = {32, 32, 16, 32};

u16 gRadialQuadPaletteOffsets[4]
    __attribute__((section(".data.radial_effect_quads"))) = {1, 0, 1, 2};

u16 gRadialQuadShades[4]
    __attribute__((section(".data.radial_effect_quads"))) = {32, 16, 32, 16};

SVECTOR gPlayerTurnTrailOrigin
    __attribute__((section(".data.player_turn_trail_origin"), aligned(4))) = {0, 0, 0, 0};

RECT gWhitePulsePaletteRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};

RECT gColorCycleCaptureRect
    __attribute__((section(".data.color_cycle_rects"))) = {944, 240, 8, 1};

s32 gQuadTransitionFrame
    __attribute__((section(".sdata.quad_transition_frame"), aligned(4))) = 0;

u16 gStretchedTextWidth __attribute__((section(".sdata.stretched_text"), aligned(2))) = 8;
u16 gTextGlyphPalette __attribute__((section(".sdata.stretched_text"), aligned(2))) = 0;

extern s32 playerSnapshotTrailCountGp;


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
    for (playerIndex = 0; playerIndex < gActiveRacePlayerCount; playerIndex++) {
        startPlayerShadowTask(playerIndex);
        startDelayedEightEntryEffectTask(playerIndex);
        createPlayerTrailTask(playerIndex);
        startPlayerTurnTripleQuadEffectTask(playerIndex);
        createPlayerParticleTrailTask(playerIndex);
    }
}

void initializeRacePlayerEffects(void) {
    s32 playerIndex;
    VECTOR position;

    createChildTaskForParentWithReserve(&gGameTasks[4], updatePlayerRenderSnapshots);
    playerSnapshotTaskGp = gCreatedTask;
    if (gActiveRacePlayerCount == 4) {
        playerIndex = 0;
        do {
            startPlayerShadowTask(playerIndex);
            createPlayerPositionTrailTask(playerIndex);
            createPlayerParticleTrailTask(playerIndex);
        } while (++playerIndex < gActiveRacePlayerCount);
    } else {
        for (playerIndex = 0; playerIndex < gActiveRacePlayerCount; playerIndex++) {
            startPlayerShadowTask(playerIndex);
            startDelayedEightEntryEffectTask(playerIndex);
            createPlayerTrailTask(playerIndex);
            startPlayerTurnTripleQuadEffectTask(playerIndex);
            createPlayerParticleTrailTask(playerIndex);
        }
    }
    if (gSelectedCourseIndex == 6) {
        for (playerIndex = 0; playerIndex < playerCountAbsolute; playerIndex++) {
            createPlayerParticleTask(playerIndex);
        }
    }
    if (gSelectedCourseIndex == 4) {
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
        for (playerIndex = 0; playerIndex < playerCountAbsolute; playerIndex++) {
            createFlickeringDotsTask(playerIndex);
        }
    }
}

void clearPlayerSnapshotTasks(void) {
    GameTask *savedTask = gCurrentTask;
    gCurrentTask = playerSnapshotTaskGp;
    deleteCurrentTaskDescendants();
    gCurrentTask = savedTask;
    playerSnapshotTrailCountGp = 0;
}

void createPlayerSnapshotTrailTask(TaskCallback callback) {
    createChildTaskForParentWithReserve(playerSnapshotTaskGp, callback);
    gCreatedTask->data18 = 0x101;
    playerSnapshotTrailCountGp++;
}

void drawPlayerSnapshotTrailTask(void) {
    long p, flag;
    PlayerSnapshotTrailResource *resource;
    POLY_FT4 *packet;
    MATRIX *scratch = (MATRIX *)getScratchAddr(0);
    s32 depth;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerSnapshotTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = &gPlayerSnapshotTrailPacket;
    packet->r0 = resource->remainingFrames * 48 / 90;
    packet->g0 = resource->remainingFrames * 32 / 90;
    packet->b0 = resource->remainingFrames * 32 / 90;
    GsGetLs(&resource->current, scratch);
    ScaleMatrix(scratch, &resource->currentScale);
    GsSetLsMatrix(scratch);
    depth = RotTransPers(&gPlayerSnapshotTrailEndpoints[0], (long *)&packet->x0, &p, &flag);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[1], (long *)&packet->x1, &p, &flag);
    GsGetLs(&resource->previous, scratch);
    ScaleMatrix(scratch, &resource->previousScale);
    GsSetLsMatrix(scratch);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[0], (long *)&packet->x2, &p, &flag);
    depth += RotTransPers(&gPlayerSnapshotTrailEndpoints[1], (long *)&packet->x3, &p, &flag);
    if (depth >= 0 && flag >= 0) {
        addCopiedPrimitive(&gCurrentPlayerOrderingTable[depth >> 4], (u32 *)packet);
    }
    if (--resource->remainingFrames == -1 || gPlayerSnapshotTrailPacket.y0 > 160) {
        deleteCurrentTaskSubtree();
        gPlayerSnapshotTrailCount--;
    }
}

void initializePlayerSnapshotTrailTask(void) {
    PlayerSnapshotTrailResource *resource;
    SVECTOR rotation;
    if (gCurrentTask->resource14 != 0xFFFF) {
        while (1) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    resource = (PlayerSnapshotTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    GsInitCoordinate2(0, &resource->current);
    GsInitCoordinate2(0, &resource->previous);
    rotation.vx = gPlayerRenderSnapshots[gCurrentTask->data1C].unk10;
    rotation.vy = gPlayerRenderSnapshots[gCurrentTask->data1C].unk18 + 0x400;
    rotation.vz = gPlayerRenderSnapshots[gCurrentTask->data1C].unk14;
    resource->currentScale.vx = 0x1000;
    resource->currentScale.vy = 0x1000;
    resource->currentScale.vz = (((s16)gPlayerRenderSnapshots[gCurrentTask->data1C].unk18 &
        (0x3FF - (s16)gPlayerRenderSnapshots[gCurrentTask->data1C].unk12) & 0x3FF) << 2) + 0x1000;
    RotMatrix(&rotation, &resource->current.coord);
    resource->current.coord.t[0] = gPlayerRenderSnapshots[gCurrentTask->data1C].position.vx;
    resource->current.coord.t[1] = gPlayerRenderSnapshots[gCurrentTask->data1C].position.vy;
    resource->current.coord.t[2] = gPlayerRenderSnapshots[gCurrentTask->data1C].position.vz;
    rotation.vx = gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].unk10;
    rotation.vy = gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].unk18 + 0x400;
    rotation.vz = gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].unk14;
    resource->previousScale.vx = 0x1000;
    resource->previousScale.vy = 0x1000;
    resource->previousScale.vz = (((s16)gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].unk18 &
        (0x3FF - (s16)gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].unk12) & 0x3FF) << 2) + 0x1000;
    RotMatrix(&rotation, &resource->previous.coord);
    resource->previous.coord.t[0] = gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].position.vx;
    resource->previous.coord.t[1] = gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].position.vy;
    resource->previous.coord.t[2] = gPreviousPlayerRenderSnapshots[gCurrentTask->data1C].position.vz;
    resource->remainingFrames = gCurrentTask->data1A;
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TrickRingResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    scratch = (TrickTrianglePacket *)getScratchAddr(0);
    packet = (TrickTrianglePacket *)GsOUT_PACKET_P;
    player = &gRacePlayers[gCurrentTask->data18];
    if (gCurrentTask->data18 == 0) {
        ot = gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer].org;
    } else {
        ot = gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer].org;
    }
    if (gSelectedPlayerReplayActive && player->unk0C8 < 9) {
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
    if (gGameStateFlags & 0x400) {
        if (gCurrentTask->data18 != gActiveGraphicsBuffer) {
            goto advance;
        }
        scratch->tag = (1 + 6) << 24;
    } else {
        if (gCurrentTask->data18 != 0) {
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
    GsOUT_PACKET_P = (PACKET *)packet;

advance:
    if (!gRacePauseState) {
        resource->frame++;
    }
}

void startIndexedRenderTask(s32 index) {
    TaskMemoryBlock *resource;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawTrickTriangleRingTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = index;
    resource = &gTaskMemoryPool[gCreatedTask->resource14];
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TrickRingResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    scratch = (TrickRingPacket *)getScratchAddr(0);
    player = &gRacePlayers[gCurrentTask->data18];
    packet = (TrickRingPacket *)GsOUT_PACKET_P;
    if (gCurrentTask->data18 == 0) {
        ot = gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer].org;
    } else {
        ot = gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer].org;
    }
    if (gSelectedPlayerReplayActive && player->unk0C8 < 9) {
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
    if (gGameStateFlags & 0x400) {
        if (gCurrentTask->data18 != gActiveGraphicsBuffer) {
            goto advance;
        }
    } else if (gCurrentTask->data18 != 0) {
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
    GsOUT_PACKET_P = (PACKET *)packet;

advance:
    if (!gRacePauseState) {
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    frame = (s32 *)&gTaskMemoryPool[gCurrentTask->resource14].unk00;
    scratch = (TrickSparkScratch *)getScratchAddr(0);
    packet = (u8 *)GsOUT_PACKET_P;
    player = &gRacePlayers[gCurrentTask->data18];
    if (gCurrentTask->data18 == 0) {
        ot = gPlayer1Layer7OrderingTables[gActiveGraphicsBuffer].org;
    } else {
        ot = gPlayer2Layer7OrderingTables[gActiveGraphicsBuffer].org;
    }
    if (gSelectedPlayerReplayActive && player->unk0C8 < 9) {
        deleteCurrentTaskSubtree();
    }
    if (player->unk0C8 < 9 && *frame < 30) {
        *frame = 30;
    }
    /* Initialize complete DMA tags before linking the packets into the ordering table. */
    if (gGameStateFlags & 0x400) {
        if (gCurrentTask->data18 != gActiveGraphicsBuffer) {
            goto advance;
        }
        scratch->ray.tag = (1 + 4) << 24;
    } else {
        if (gCurrentTask->data18 != 0) {
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
    GsOUT_PACKET_P = (PACKET *)packet;

advance:
    if (!gRacePauseState) {
        ++*frame;
    }
    if (*frame >= 41) {
        deleteCurrentTaskSubtree();
    }
}

void startIndexedRenderTaskPair(s32 index) {
    TaskMemoryBlock *resource;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawTrickRingTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = index;
    resource = &gTaskMemoryPool[gCreatedTask->resource14];
    resource->unk00 = 0;
    resource->unk04 = 0;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawTrickSparkRingTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = index;
    resource = &gTaskMemoryPool[gCreatedTask->resource14];
    resource->unk00 = 0;
}

void drawFallingParticlesTask(void) {
    s32 i;
    FallingParticleResource *resource;
    SPRT_8 *packet;
    SPRT_8 *scratch;
    DR_TPAGE *page;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (SPRT_8 *)getScratchAddr(0);
    resource = (FallingParticleResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
        scratch->tag = (3 << 24) | getaddr(gCurrentPlayerOrderingTable);
        *packet = *scratch;
        setaddr(gCurrentPlayerOrderingTable, packet);
        packet++;
    }
    page = (DR_TPAGE *)packet;
    setlen(page, 1);
    page->code[0] = ((u32)DRAW_MODE_COMMAND_HIGH << 16) | getTPage(0, 0, 768, 256);
    addPrim(gCurrentPlayerOrderingTable, page);
    GsOUT_PACKET_P = (PACKET *)(page + 1);
    resource->frame++;
}

void createFallingParticleTask(void) {
    FallingParticleResource *resource;
    s32 i;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawFallingParticlesTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (FallingParticleResource *)&gTaskMemoryPool[gCreatedTask->resource14];
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

        addSemiTransparentGouraudTriangle(gCurrentPlayerOrderingTable, getTPage(0, 1, 0, 0), x, y,
            rightX, rightY, centerX, centerY, (u8)red, (u8)green, (u8)blue,
            0, 0, 0, 64, 64, 64);
        addSemiTransparentGouraudTriangle(gCurrentPlayerOrderingTable, getTPage(0, 1, 0, 0), x, y,
            centerX, centerY, leftX, leftY, (u8)red, (u8)green, (u8)blue,
            64, 64, 64, 0, 0, 0);
        angle += 256;
        ray++;
    } while (angle < 4096);

    for (i = 0; i < 4; i++) {
        s16 halfSize = gRadialQuadHalfSizes[i];
        s16 centerY = i * 32;
        s16 centerX = -centerY;

        addTexturedQuad(gCurrentPlayerOrderingTable, (s16)(centerX - halfSize), (s16)(centerY - halfSize),
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (PlayerParticleScratch *)getScratchAddr(0);
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    positions = (PlayerParticlePosition *)&gTaskMemoryPool[gCurrentTask->resource14];
    player = &gRacePlayers[gCurrentTask->data18];

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

    yawOffset = -gPlayerCameras[gCurrentTask->data18].yaw;
    pitchOffset = gPlayerCameras[gCurrentTask->data18].pitch;
    for (i = 0; i < 40; i++) {
        if (gGameStateFlags & 0x400) {
            if (gCurrentTask->data18 != (gFrameCounter & 1)) {
                goto updatePosition;
            }
        } else if (gCurrentTask->data18 != 0) {
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
            addPrim(&gCurrentPlayerOrderingTable[10], packet);
            packet++;
        }

    updatePosition:
        if (!gRacePauseState) {
            s32 drift = randomNextLinear() % 16 + 8;
            s32 sine = rsin(gCurrentTask->data1A);
            s32 cosine = rcos(gCurrentTask->data1A * 5);
            positions[i].x += drift * (sine + cosine) / 8192;
            positions[i].y += 8;
            if (!gSelectedPlayerReplayActive) {
                positions[i].z -= player->unk0BC / 256;
            }
        }
    }
    GsOUT_PACKET_P = (PACKET *)packet;
    if (!gRacePauseState) {
        gCurrentTask->data1A++;
    }
}

void createPlayerParticleTask(s32 playerIndex) {
    PlayerParticlePosition *position;
    s32 i;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawPlayerParticlesTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    position = (PlayerParticlePosition *)&gTaskMemoryPool[gCreatedTask->resource14];
    for (i = 0; i < 40; i++) {
        position[i].x = randomNextLinear() % 1024;
        position[i].y = randomNextLinear() % 512;
        position[i].z = randomNextLinear() % 1024;
    }
    gCreatedTask->data18 = playerIndex;
    gCreatedTask->data1A = 0;
}

void drawTriangleBurstTask(void) {
    u8 padding[0x50];
    ScreenPoint screen[3];
    TriangleBurstResource *resource;
    TriangleBurstScratch *scratch;
    s32 i, depth;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    scratch = (TriangleBurstScratch *)getScratchAddr(0);
    resource = (TriangleBurstResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
            addGouraudTexturedTriangle(gCurrentPlayerOrderingTable + (depth >> 2),
                                       screen[0].x, screen[0].y, screen[1].x, screen[1].y,
                                       screen[2].x, screen[2].y, 10, getClut(640, 250),
                                       128, 128, 191, 128, 128, 191,
                                       255, 255, 255, 128, 128, 128, 64, 64, 64);
        }
        if (!gRacePauseState) {
            resource->positions[i].vx += resource->velocities[i].vx;
            resource->positions[i].vy += resource->velocities[i].vy;
            resource->positions[i].vz += resource->velocities[i].vz;
            resource->velocities[i].vy += 4;
            resource->rotations[i].vx += 0x155;
            resource->rotations[i].vz += 0x155;
        }
    }
    if (!gRacePauseState) resource->frame++;
    if (resource->frame == 30) deleteCurrentTaskSubtree();
}

void initTriangleBurstTask(void) {
    TriangleBurstResource *resource;
    s32 angle;
    s32 i;
    s32 phase;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TriangleBurstResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    resource->frame = 0;
    phase = randomNextLinear() % 4096;
    for (i = 0; i < 4; i++) {
        resource->positions[i] = resource->origin;
        angle = phase + i * 1024;
        resource->velocities[i].vx = rcos(angle) / 128;
        resource->velocities[i].vz = rsin(angle) / 128;
        resource->velocities[i].vy = -32;
        resource->rotations[i].vx = randomNextLinear() % 4096;
        resource->rotations[i].vy = i * 1024;
        resource->rotations[i].vz = 0;
    }

    phase = randomNextLinear() % 4096;
    resource->positions[4] = resource->origin;
    resource->velocities[4].vx = rcos(phase) / 512;
    resource->velocities[4].vz = rsin(phase) / 512;
    resource->velocities[4].vy = -40;
    resource->rotations[4].vx = randomNextLinear() % 4096;
    resource->rotations[4].vy = 0;
    resource->rotations[4].vz = 1024;
    setCurrentTaskCallback(drawTriangleBurstTask);
}

void createPositionedEffectTask(s32 x, s32 y, s32 z) {
    TriangleBurstResource *resource;

    createChildTaskWithReserve(initTriangleBurstTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (TriangleBurstResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->origin.vx = x >> 8;
    resource->origin.vy = y >> 8;
    resource->origin.vz = z >> 8;
}

void drawRotatingQuadEffectTask(void) {
    RotatingEffectResource *resource;
    RotatingQuadScratch *scratch;
    s32 i;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RotatingEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    scratch = (RotatingQuadScratch *)getScratchAddr(0);
    if (resource->elapsed <= 16) {
        scratch->textureAdvance = resource->elapsed * 63 / 16;
        scratch->displacement = resource->elapsed * 5;
        if (!gRacePauseState) {
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
        addGouraudTexturedQuad(gCurrentPlayerOrderingTable + (scratch->depth >> 2),
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
            addGouraudTexturedQuad(gCurrentPlayerOrderingTable + (scratch->depth >> 2),
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
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (RotatingEffectResource *)&gTaskMemoryPool[gCreatedTask->resource14];
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
    RacePlayer *player = &gRacePlayers[gCurrentTask->data18];
    POLY_FT4 *packet = (POLY_FT4 *)GsOUT_PACKET_P;
    PlayerShadowScratch *scratch = (PlayerShadowScratch *)getScratchAddr(0);
    s32 i;
    long depth;

    if ((player->unk128 & 0x1000) && player->task->data1A == 7) {
        return;
    }
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0],
            player->worldCoordinate.coord.t[2], (s16)getRenderCameraIndex())) {
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
    scratch->coordinate.coord.t[1] = getCourseHeightAtPoint(player->unk92E,
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
        addPrim(&gCurrentPlayerDepthOffsetOrderingTable[scratch->depth >> 2], packet);
    }
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
}

void startPlayerShadowTask(s32 playerIndex) {
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawPlayerShadowTask);
    gCreatedTask->data18 = playerIndex;
}

void updatePlayerTurnTrailTask(void) {
    TurnTrailHistory *trail;
    TurnTrailEdges *edge;
    RacePlayer *player;
    POLY_FT3 *packet;
    TurnTrailScratch *scratch;
    s32 i;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    trail = (TurnTrailHistory *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    edge = (TurnTrailEdges *)&gTaskMemoryPool[gCurrentTask->resource16];
    packet = (POLY_FT3 *)GsOUT_PACKET_P;
    player = &gRacePlayers[gCurrentTask->data18];
    scratch = (TurnTrailScratch *)getScratchAddr(0);
    scratch->emitting = 0;

    switch (player->unk96E) {
        case 0:
            scratch->tpage = getTPage(0, 1, 704, 0);
            scratch->clut = getClut(880, 244);
            scratch->u[0] = 128;
            scratch->v[0] = 0;
            scratch->emitting = 1;
            break;
        case 0x100:
            scratch->tpage = getTPage(0, 0, 704, 0);
            scratch->clut = getClut(880, 247);
            scratch->u[0] = 192;
            scratch->v[0] = 0;
            scratch->emitting = 1;
            if (player->surfaceState.fields.unk136 != 0 && player->surfaceState.fields.unk136 != 6) {
                scratch->emitting = 0;
            }
            break;
        case 0x200:
            scratch->tpage = getTPage(0, 0, 704, 0);
            scratch->clut = getClut(880, 245);
            scratch->u[0] = 128;
            scratch->v[0] = 0;
            scratch->emitting = 1;
            break;
        case 0x801:
            scratch->tpage = getTPage(0, 1, 704, 0);
            scratch->clut = getClut(880, 246);
            scratch->u[0] = 128;
            scratch->v[0] = 0;
            scratch->emitting = 1;
            break;
        default:
            scratch->tpage = getTPage(0, 1, 704, 0);
            scratch->clut = getClut(880, 244);
            scratch->u[0] = 128;
            scratch->v[0] = 0;
            scratch->emitting = 0;
            break;
    }
    if (player->unk128 & 0x677DA21) {
        scratch->emitting = 0;
    }
    if (player->unk0BC < 0x800) {
        scratch->emitting = 0;
    }
    if (player->unk128 & 0x100) {
        scratch->emitting = 0;
    }
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0],
            player->worldCoordinate.coord.t[2], (s16)gCurrentTask->data18)) {
        scratch->emitting = 0;
    }
    if ((s16)player->unk92C < 0x800) {
        scratch->strength = (s16)player->unk92C - 100;
        scratch->side = 1;
    } else {
        scratch->strength = 0x1000 - (s16)player->unk92C - 100;
        scratch->side = -1;
    }
    if (scratch->strength >= 128) {
        scratch->strength = 128;
    }
    if (player->unk128 & TRAIL_FACING_BACKWARD) {
        scratch->facing = -1;
    } else {
        scratch->facing = 1;
    }

    if (!gRacePauseState) {
        /* Slot zero follows the board. Older samples move back from slot one. */
        for (i = TURN_TRAIL_SAMPLES - 1; i >= 2; i--) {
            trail->positions[i] = trail->positions[i - 1];
            edge->offsets[i] = edge->offsets[i - 1];
            trail->velocities[i] = trail->velocities[i - 1];
            trail->ages[i] = trail->ages[i - 1];
            edge->tpages[i] = edge->tpages[i - 1];
            edge->cluts[i] = edge->cluts[i - 1];
            edge->textureU[i] = edge->textureU[i - 1];
        }

        setVector(&scratch->offset, scratch->facing * 40, 0, scratch->side * -15);
        ApplyMatrix(&player->model.effect.effectMatrix180, &scratch->offset, &trail->positions[0]);
        applyVector(&trail->positions[0], player->x >> 8, player->y >> 8, player->z >> 8, +=);
        setVector(&edge->offsets[0], 0, 0, 0);
        edge->tpages[0] = scratch->tpage;
        edge->cluts[0] = scratch->clut;
        edge->textureU[0] = scratch->u[0];
        if (!scratch->emitting) {
            trail->ages[0] = TURN_TRAIL_LIFETIME;
        } else {
            trail->ages[0] = 0;
        }

        if (scratch->emitting && scratch->strength > 0 && !(trail->elapsed & 1)) {
            setVector(&scratch->offset, scratch->facing * 40, 0, scratch->side * -15);
            ApplyMatrix(&player->model.effect.effectMatrix180, &scratch->offset, &trail->positions[1]);
            applyVector(&trail->positions[1], player->x >> 8, player->y >> 8, player->z >> 8, +=);
            setVector(&edge->offsets[1], 0, 0, 0);
            edge->tpages[1] = scratch->tpage;
            edge->cluts[1] = scratch->clut;
            edge->textureU[1] = scratch->u[0];
            if (player->unk128 & TRAIL_FACING_BACKWARD) {
                trail->velocities[1].vx = scratch->strength / 4;
            } else {
                trail->velocities[1].vx = -scratch->strength / 4;
            }
            trail->velocities[1].vy = 0;
            trail->velocities[1].vz = scratch->side * scratch->strength / 4;
            trail->ages[i] = 0;
            if (gCurrentTask->data1C == 1) {
                trail->velocities[1].vz = -32;
            }
            ApplyMatrixSV(&player->model.effect.effectMatrix180, &trail->velocities[1], &trail->velocities[1]);
        }

        for (i = 1; i < TURN_TRAIL_SAMPLES; i++) {
            if (trail->ages[i] < TURN_TRAIL_LIFETIME) {
                edge->offsets[i].vx += trail->velocities[i].vx;
                edge->offsets[i].vy += trail->velocities[i].vy;
                edge->offsets[i].vz += trail->velocities[i].vz;
                trail->velocities[i].vx = trail->velocities[i].vx * 3 / 4;
                trail->velocities[i].vy += 4;
                trail->velocities[i].vz = trail->velocities[i].vz * 3 / 4;
                if (edge->offsets[i].vy > 0) {
                    edge->offsets[i].vy = 0;
                    trail->ages[i] = TURN_TRAIL_LIFETIME;
                }
                if (trail->ages[i] < TURN_TRAIL_LIFETIME) {
                    trail->ages[i]++;
                }
            }
        }
    }

    GsInitCoordinate2(NULL, &scratch->coordinate);
    for (i = 0; i < TURN_TRAIL_SAMPLES; i++) {
        setVector((VECTOR *)scratch->coordinate.coord.t,
            trail->positions[i].vx, trail->positions[i].vy, trail->positions[i].vz);
        GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
        GsSetLsMatrix(&scratch->coordinate.workm);
        scratch->depth[i] = RotTransPers(&edge->offsets[i], &scratch->edge[i].packed, &scratch->cue, &scratch->flags[i]);
        RotTransPers(&gPlayerTurnTrailOrigin, &scratch->base[i].packed, &scratch->cue, &scratch->flags[i]);
    }

    for (i = 0; i < TURN_TRAIL_SAMPLES - 1; i++) {
        if (trail->ages[i] < TURN_TRAIL_LIFETIME && trail->ages[i + 1] < TURN_TRAIL_LIFETIME &&
            scratch->flags[i] >= 0 && scratch->depth[i] >= 0 && scratch->depth[i] < 4096 && edge->tpages[i]) {
            scratch->tpage = edge->tpages[i];
            scratch->clut = edge->cluts[i];
            scratch->u[0] = edge->textureU[i];
            scratch->u[1] = scratch->u[0] + 63;
            scratch->v[1] = scratch->v[0];
            scratch->u[2] = scratch->u[0];
            scratch->v[2] = scratch->v[0] + 63;
            scratch->u[3] = scratch->u[1];
            scratch->v[3] = scratch->v[2];

            packet->tag = (sizeof(*packet) / sizeof(packet->tag) - 1) << 24;
            setcode(packet, POLY_FT3_CODE | SEMI_TRANSPARENT_FLAG);
            packet->tpage = scratch->tpage;
            packet->clut = scratch->clut;
            packet->r0 = 128;
            packet->b0 = 128;
            packet->g0 = 128;
            setXY3(packet, scratch->edge[i].xy.x, scratch->edge[i].xy.y,
                scratch->base[i + 1].xy.x, scratch->base[i + 1].xy.y,
                scratch->base[i].xy.x, scratch->base[i].xy.y);
            packet->u0 = scratch->u[0];
            packet->u1 = scratch->u[3];
            packet->u2 = scratch->u[2];
            packet->v0 = scratch->v[0];
            packet->v1 = scratch->v[3];
            packet->v2 = scratch->v[2];
            addPrim(gCurrentPlayerOrderingTable + (scratch->depth[i] >> 2), packet);
            packet++;

            packet->tag = (sizeof(*packet) / sizeof(packet->tag) - 1) << 24;
            setcode(packet, POLY_FT3_CODE | SEMI_TRANSPARENT_FLAG);
            packet->tpage = scratch->tpage;
            packet->clut = scratch->clut;
            packet->r0 = 128;
            packet->b0 = 128;
            packet->g0 = 128;
            setXY3(packet, scratch->edge[i].xy.x, scratch->edge[i].xy.y,
                scratch->edge[i + 1].xy.x, scratch->edge[i + 1].xy.y,
                scratch->base[i + 1].xy.x, scratch->base[i + 1].xy.y);
            packet->u0 = scratch->u[0];
            packet->u1 = scratch->u[1];
            packet->u2 = scratch->u[3];
            packet->v0 = scratch->v[0];
            packet->v1 = scratch->v[1];
            packet->v2 = scratch->v[3];
            addPrim(gCurrentPlayerOrderingTable + (scratch->depth[i] >> 2), packet);
            packet++;
        }
    }
    GsOUT_PACKET_P = (PACKET *)packet;
    if (!gRacePauseState) {
        trail->elapsed++;
    }
    if (gCurrentTask->data1A == 2) {
        deleteCurrentTaskSubtree();
    }
}

void initializeDelayedEightEntryEffectTask(void) {
    TurnTrailHistory *primary;
    TurnTrailEdges *secondary;
    s32 i;
    if (gCurrentTask->data18 == 0) {
        createChildTaskForParentWithReserve(gPlayerSnapshotTask, updatePlayerTurnTrailTask);
        gCreatedTask->resource14 = allocateTaskMemoryBlock();
        primary = (TurnTrailHistory *)&gTaskMemoryPool[gCreatedTask->resource14];
        gCreatedTask->resource16 = allocateTaskMemoryBlock();
        secondary = (TurnTrailEdges *)&gTaskMemoryPool[gCreatedTask->resource16];
        gCreatedTask->data18 = gCurrentTask->data1A;
        gCreatedTask->data1A = 0;
        for (i = 0; i < TURN_TRAIL_SAMPLES; i++) {
            primary->positions[i].vx = 0;
            primary->positions[i].vy = 0;
            primary->positions[i].vz = 0;
            secondary->offsets[i].vx = 0;
            secondary->offsets[i].vy = 0;
            secondary->offsets[i].vz = 0;
            primary->velocities[i].vx = 0;
            primary->velocities[i].vy = 0;
            primary->velocities[i].vz = 0;
            secondary->tpages[i] = 0;
            secondary->cluts[i] = 0;
            secondary->textureU[i] = 0;
            primary->ages[i] = TURN_TRAIL_LIFETIME;
        }
        primary->elapsed = 0;
        deleteCurrentTaskSubtree();
    } else {
        gCurrentTask->data18--;
    }
}

s32 startDelayedEightEntryEffectTask(s32 playerIndex) {
    createChildTaskForParentWithReserve(&gGameTasks[4], initializeDelayedEightEntryEffectTask);
    gCreatedTask->data18 = 1;
    gCreatedTask->data1A = playerIndex;
    return 0;
}

void drawPlayerPairedQuadEffectTask(void) {
    PairedEffectResource *resource;
    RacePlayer *player;
    POLY_FT4 *packet;
    PairedQuadScratchpad *scratch;
    s32 i, j;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PairedEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    player = &gRacePlayers[gCurrentTask->data18];
    scratch = (PairedQuadScratchpad *)getScratchAddr(0);
    if (isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0],
            player->worldCoordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        for (i = 0; i < 2; i++) {
            GsInitCoordinate2(NULL, &scratch->coordinate);
            setVector((VECTOR *)scratch->coordinate.coord.t,
                resource->positions[i].vx + (player->worldCoordinate.coord.t[0] + player->unk084),
                resource->positions[i].vy + (player->worldCoordinate.coord.t[1] + player->unk086),
                resource->positions[i].vz + (player->worldCoordinate.coord.t[2] + player->unk088));
            GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
            for (j = 0; j < 3; j++) {
                scratch->coordinate.coord.t[j] = scratch->coordinate.workm.t[j];
            }
            GsSetLsMatrix(&scratch->coordinate.coord);
            scratch->depth = RotTransPers4(&gPlayerPairedQuadVertices[0], &gPlayerPairedQuadVertices[1],
                &gPlayerPairedQuadVertices[2], &gPlayerPairedQuadVertices[3],
                (long *)&packet->x0, (long *)&packet->x1, (long *)&packet->x2,
                (long *)&packet->x3, &scratch->cue, &scratch->flags) - 10;

            packet->tag = TEXTURED_QUAD_PACKET_WORDS << 24;
            setcode(packet, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG);
            setRGB0(packet, 128, 128, 128);
            packet->tpage = getTPage(0, 0, 832, 256);
            packet->clut = resource->clut;
            setUV4(packet, 112, (resource->elapsed % 5) * 16 + 80,
                127, packet->v0, 112, packet->v0 + 15, 127, packet->v0 + 15);
            if (!(scratch->flags & PAIRED_QUAD_PROJECTION_FLAGS) &&
                scratch->depth >= 0 && scratch->depth < 4096) {
                addPrim(gCurrentPlayerOrderingTable + (scratch->depth >> 2), packet);
            }
            packet++;
        }
        GsOUT_PACKET_P = (PACKET *)packet;
    }
    if (!gRacePauseState) {
        resource->elapsed++;
    }
    if (resource->elapsed == 10) {
        deleteCurrentTaskSubtree();
    }
}

void createPlayerPairedQuadEffectTask(s32 playerIndex, s32 variant) {
    PairedEffectResource *resource;
    RacePlayer *player;
    s32 i;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawPlayerPairedQuadEffectTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    player = &gRacePlayers[playerIndex];
    resource = (PairedEffectResource *)&gTaskMemoryPool[gCreatedTask->resource14];
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
    gCreatedTask->data18 = playerIndex;
}

void drawQuadAnimationTask(void) {
    PrimaryQuadAnimationResource *resource;
    POLY_FT4 *packet;
    QuadAnimationScratch *scratch;
    const QuadAnimationFrame *frame;
    s32 duration;
    s32 i;
    long depth;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (QuadAnimationScratch *)getScratchAddr(0);
    resource = (PrimaryQuadAnimationResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch->frame.current = frame = &resource->frames[resource->frame];
    duration = frame->duration;
    if (duration == -1) {
        deleteCurrentTaskSubtree();
        return;
    }
    if (duration == -2) {
        resource->frame = 0;
        resource->elapsed = 0;
        scratch->frame.current = &resource->frames[resource->frame];
    }
    if (gRacePauseState == 0) {
        resource->elapsed++;
    }
    if (resource->elapsed == scratch->frame.current->duration) {
        resource->elapsed = 0;
        resource->frame++;
    }

    if (isPointWithinCameraForwardRange(resource->x, resource->z, (s16)getRenderCameraIndex())) {
        const volatile QuadAnimationFrame *scaleFrame = scratch->frame.forScale;
        scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = scaleFrame->scale;
        GsInitCoordinate2(NULL, (GsCOORDINATE2 *)getScratchAddr(0));
        scratch->coordinate.coord.t[0] = resource->x;
        scratch->coordinate.coord.t[1] = resource->y;
        scratch->coordinate.coord.t[2] = resource->z;
        GsGetLs((GsCOORDINATE2 *)getScratchAddr(0), &((GsCOORDINATE2 *)getScratchAddr(0))->workm);
        for (i = 0; i < 3; i++) {
            ((GsCOORDINATE2 *)getScratchAddr(0))->coord.t[i] = ((GsCOORDINATE2 *)getScratchAddr(0))->workm.t[i];
        }
        ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);
        GsSetLsMatrix(&scratch->coordinate.coord);
        scratch->depth = RotTransPers4(
            &gQuadAnimationVertices[0], &gQuadAnimationVertices[1],
            &gQuadAnimationVertices[2], &gQuadAnimationVertices[3],
            (long *)&packet->x0, (long *)&packet->x1, (long *)&packet->x2, (long *)&packet->x3,
            &scratch->cue, &scratch->flags);

        packet->tag = TEXTURED_QUAD_PACKET_WORDS << 24;
        setcode(packet, POLY_FT4_CODE | scratch->frame.current->blend);
        packet->tpage = scratch->frame.current->tpage;
        packet->clut = scratch->frame.current->clut;
        setRGB0(packet, scratch->frame.current->r, scratch->frame.current->g, scratch->frame.current->b);
        packet->u0 = scratch->frame.current->u;
        frame = scratch->frame.current;
        packet->u1 = frame->u + frame->w;
        packet->u2 = packet->u0;
        packet->u3 = packet->u1;
        packet->v0 = scratch->frame.current->v;
        packet->v1 = scratch->frame.current->v;
        packet->v3 = packet->v2 = packet->v0 + scratch->frame.current->h;

        depth = scratch->depth -= 10;
        if (scratch->flags >= 0 && depth >= 0 && depth < 4096) {
            addPrim(&gCurrentPlayerOrderingTable[scratch->depth >> 2], packet);
        }
        GsOUT_PACKET_P = (PACKET *)(packet + 1);
    }
}

GameTask *createQuadAnimationTask(u16 unusedId, VECTOR *position) {
    PrimaryQuadAnimationResource *resource;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawQuadAnimationTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (PrimaryQuadAnimationResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->x = position->vx;
    resource->y = position->vy;
    resource->z = position->vz;
    resource->elapsed = 0;
    resource->frame = 0;
    resource->frames = gQuadAnimationFrames;
    return gCreatedTask;
}

GameTask *startAlternateQuadAnimationTask(s32 unused, const PositionEffectOrigin *position) {
    QuadAnimationResource *resource;
    GameTask *task;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, drawQuadAnimationTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    task = gCreatedTask;
    resource = (QuadAnimationResource *)&gTaskMemoryPool[task->resource14];
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RadialEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch = (RadialEffectScratch *)getScratchAddr(0);

    if (gRacePauseState == 0) {
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
            resource->coordinate.coord.t[2], (s16)getRenderCameraIndex())) {
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
                addPrim(&gCurrentPlayerOrderingTable[scratch->depth >> 2], packet);
            }
            packet++;
        }
        GsOUT_PACKET_P = (PACKET *)packet;
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
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    origin = (PositionEffectOrigin *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gRacePauseState == 0 && (gFrameCounter & 3) == 0) {
        createChildTaskForParentWithReserve(gPlayerSnapshotTask, updateRadialEffectTask);
        gCreatedTask->resource14 = allocateTaskMemoryBlock();
        resource = (RadialEffectResource *)&gTaskMemoryPool[gCreatedTask->resource14];
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
        if (--gCurrentTask->data18 == 0) {
            deleteCurrentTaskSubtree();
        }
    }
}

void startPositionEffectTask(const PositionEffectOrigin *position) {
    PositionEffectOrigin *resource;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, emitRadialEffectTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = 3;
    resource = (PositionEffectOrigin *)&gTaskMemoryPool[gCreatedTask->resource14];
    *resource = *position;
}

void updatePlayerTrailTask(void) {
    PlayerTrailResource *left, *right;
    RacePlayer *player;
    POLY_FT4 *packet;
    BoardTrailScratch *scratch;
    s32 i, axis;
    s32 surfaceState;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    left = (PlayerTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    right = (PlayerTrailResource *)&gTaskMemoryPool[gCurrentTask->resource16];
    player = &gRacePlayers[gCurrentTask->data18];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch = (BoardTrailScratch *)getScratchAddr(0);

    if ((player->unk128 & 0x677DB21) && !(player->unk128 & 8)) {
        right->state.suppressed[0] = 1;
        surfaceState = player->unk96E;
    } else {
        right->state.suppressed[0] = 0;
        surfaceState = player->unk96E;
    }
    if (surfaceState && surfaceState != 0x801 && surfaceState != 0x200) {
        right->state.suppressed[0] = 1;
    }
    if (player->unk128 & 8) {
        right->state.suppressed[0] = 0;
    }

    /* Keep the last sixteen edge pairs, sampling every other update. */
    if (!gRacePauseState && (left->state.frame & 1)) {
        for (i = PLAYER_TRAIL_SAMPLES - 1; i > 0; i--) {
            for (axis = 0; axis < 3; axis++) {
                left->samples.axes[i][axis] = left->samples.axes[i - 1][axis];
                right->samples.axes[i][axis] = right->samples.axes[i - 1][axis];
            }
            right->state.suppressed[i] = right->state.suppressed[i - 1];
        }
    }

    if ((s16)player->unk0BE - (s16)player->unk110 >= 0) {
        setVector(&scratch->offset, -40, 0, -20);
        ApplyMatrix(&player->model.effect.effectMatrix180, &scratch->offset, (VECTOR *)left->samples.axes[0]);
        setVector(&scratch->offset, 40, 0, 20);
        ApplyMatrix(&player->model.effect.effectMatrix180, &scratch->offset, (VECTOR *)right->samples.axes[0]);
    } else {
        setVector(&scratch->offset, 40, 0, -20);
        ApplyMatrix(&player->model.effect.effectMatrix180, &scratch->offset, (VECTOR *)left->samples.axes[0]);
        setVector(&scratch->offset, -40, 0, 20);
        ApplyMatrix(&player->model.effect.effectMatrix180, &scratch->offset, (VECTOR *)right->samples.axes[0]);
    }
    for (i = 0; i < 3; i++) {
        left->samples.axes[0][i] += player->model.effect.effectMatrix180.t[i];
        right->samples.axes[0][i] += player->model.effect.effectMatrix180.t[i];
    }
    scratch->result.region = (s16)getAdjacentGeometryRegionForPoint(player->unk92E,
        left->samples.points[0].x, left->samples.points[0].z);
    left->samples.points[0].y = getCourseHeightAtPoint(scratch->result.region,
        left->samples.points[0].x, left->samples.points[0].z);
    scratch->result.region = (s16)getAdjacentGeometryRegionForPoint(player->unk92E,
        right->samples.points[0].x, right->samples.points[0].z);
    right->samples.points[0].y = getCourseHeightAtPoint(scratch->result.region,
        right->samples.points[0].x, right->samples.points[0].z);

    /* The newest edge pair supplies the trailing vertices of the first quad. */
    GsInitCoordinate2(NULL, &scratch->coordinate);
    scratch->offset.vx = scratch->offset.vy = scratch->offset.vz = 0;
    for (axis = 0; axis < 3; axis++) {
        scratch->coordinate.coord.t[axis] = left->samples.axes[0][axis];
    }
    GsGetLs(&scratch->coordinate, &scratch->localScreen);
    GsSetLsMatrix(&scratch->localScreen);
    scratch->depth[2] = RotTransPers(&scratch->offset, &scratch->screen[2].packed,
        &scratch->cue, &scratch->result.flags);
    for (axis = 0; axis < 3; axis++) {
        scratch->coordinate.coord.t[axis] = right->samples.axes[0][axis];
    }
    GsGetLs(&scratch->coordinate, &scratch->localScreen);
    GsSetLsMatrix(&scratch->localScreen);
    scratch->depth[3] = RotTransPers(&scratch->offset, &scratch->screen[3].packed,
        &scratch->cue, &scratch->result.flags);

    switch (player->unk128 & TRAIL_FACING_BACKWARD) {
    case TRAIL_FACING_BACKWARD:
        scratch->uv[0].x = 64;
        scratch->uv[0].y = 0;
        scratch->uv[1].x = 127;
        scratch->uv[1].y = 0;
        scratch->uv[2].x = 64;
        scratch->uv[2].y = 31;
        scratch->uv[3].x = 127;
        scratch->uv[3].y = 31;
        break;
    case 0:
    default:
        scratch->uv[0].x = 127;
        scratch->uv[0].y = 0;
        scratch->uv[1].x = 64;
        scratch->uv[1].y = 0;
        scratch->uv[2].x = 127;
        scratch->uv[2].y = 31;
        scratch->uv[3].x = 64;
        scratch->uv[3].y = 31;
        break;
    }

    for (i = 1; i < PLAYER_TRAIL_SAMPLES; i++) {
        packet->tag = TEXTURED_QUAD_PACKET_WORDS << 24;
        setcode(packet, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG);
        packet->tpage = getTPage(0, 2, 704, 0);
        packet->clut = getClut(880, 243);
        setUV4(packet, scratch->uv[0].x, scratch->uv[0].y, scratch->uv[1].x, scratch->uv[1].y,
            scratch->uv[2].x, scratch->uv[2].y, scratch->uv[3].x, scratch->uv[3].y);
        setGray0(packet, 48 - 48 * i / (PLAYER_TRAIL_SAMPLES - 1));
        if (!isPointWithinCameraForwardRange(left->samples.points[i].x, left->samples.points[i].z,
                (s16)getRenderCameraIndex())) {
            continue;
        }
        for (axis = 0; axis < 3; axis++) {
            scratch->coordinate.coord.t[axis] = left->samples.axes[i][axis];
        }
        GsGetLs(&scratch->coordinate, &scratch->localScreen);
        GsSetLsMatrix(&scratch->localScreen);
        scratch->depth[0] = RotTransPers(&scratch->offset, &scratch->screen[0].packed,
            &scratch->cue, &scratch->result.flags);
        for (axis = 0; axis < 3; axis++) {
            scratch->coordinate.coord.t[axis] = right->samples.axes[i][axis];
        }
        GsGetLs(&scratch->coordinate, &scratch->localScreen);
        GsSetLsMatrix(&scratch->localScreen);
        scratch->depth[1] = RotTransPers(&scratch->offset, &scratch->screen[1].packed,
            &scratch->cue, &scratch->result.flags);
        setXY4(packet,
            scratch->screen[0].point.x, scratch->screen[0].point.y,
            scratch->screen[1].point.x, scratch->screen[1].point.y,
            scratch->screen[2].point.x, scratch->screen[2].point.y,
            scratch->screen[3].point.x, scratch->screen[3].point.y);
        if (player->unk128 & TRAIL_FACING_BACKWARD) {
            if (NormalClip(scratch->screen[2].packed, scratch->screen[3].packed,
                    scratch->screen[0].packed) < 0) {
                goto advance;
            }
        } else if (NormalClip(scratch->screen[2].packed, scratch->screen[3].packed,
                scratch->screen[0].packed) >= 0) {
            goto advance;
        }
        scratch->averageDepth = (scratch->depth[0] + scratch->depth[1]) / 2;
        if (!right->state.suppressed[i] && scratch->averageDepth >= 0 && scratch->averageDepth < 4096) {
            addPrim(&gCurrentPlayerDepthOffsetOrderingTable[(scratch->averageDepth >> 2) + 4], packet);
        }
advance:
        packet++;
        scratch->screen[2].point = scratch->screen[0].point;
        scratch->screen[3].point = scratch->screen[1].point;
    }
    GsOUT_PACKET_P = (PACKET *)packet;
    left->state.frame++;
}

void createPlayerTrailTask(s32 playerIndex) {
    PlayerTrailResource *first;
    PlayerTrailResource *second;
    s32 i;
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, updatePlayerTrailTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    first = (PlayerTrailResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    gCreatedTask->resource16 = allocateTaskMemoryBlock();
    second = (PlayerTrailResource *)&gTaskMemoryPool[gCreatedTask->resource16];
    for (i = 0; i < PLAYER_TRAIL_SAMPLES; i++) {
        first->samples.points[i].x = first->samples.points[i].y = first->samples.points[i].z = 0;
        second->samples.points[i].x = second->samples.points[i].y = second->samples.points[i].z = 0;
        second->state.suppressed[i] = 0;
    }
    first->state.frame = 0;
    gCreatedTask->data18 = playerIndex;
}

void updatePlayerPositionTrailTask(void) {
    PlayerPositionTrailResource *resource;
    RacePlayer *player;
    POLY_FT4 *packet;
    PlayerPositionTrailScratch *scratch;
    s32 i, j;
    s32 trailScale;
    u16 state;
    s16 lateral;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerPositionTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    player = &gRacePlayers[gCurrentTask->data18];
    scratch = (PlayerPositionTrailScratch *)getScratchAddr(0);
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0],
            player->worldCoordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        return;
    }

    if (resource->inactive) {
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                resource->leftPositions[i][j] = player->worldCoordinate.coord.t[j];
            }
            for (j = 0; j < 3; j++) {
                resource->rightPositions[i][j] = player->worldCoordinate.coord.t[j];
            }
        }
    }
    if (player->unk0BC < 0x400) {
        resource->inactive = 1;
    }
    if (!resource->inactive) {
        if (player->unk128 & 0x677DA21) {
            resource->inactive = 1;
        }
    } else {
        if (!(player->unk128 & 0x677DA21)) {
            resource->inactive = 0;
        }
    }
    state = player->unk96E;
    if (state != 0 && state != 0x801 && state != 0x200) {
        resource->inactive = 1;
    }
    if (player->unk128 & 8) {
        resource->inactive = 0;
    }
    if (player->unk128 & 0x100) {
        resource->inactive = 1;
    }
    if (resource->inactive) {
        return;
    }

    scratch->quad.tag = TEXTURED_QUAD_PACKET_WORDS << 24;
    setcode(&scratch->quad, POLY_FT4_CODE | SEMI_TRANSPARENT_FLAG);
    setUV4(&scratch->quad, 0, 64, 63, 64, 0, 127, 63, 127);
    scratch->quad.clut = getClut(880, 248);
    scratch->quad.tpage = getTPage(0, 1, 704, 0);
    if (gRacePauseState == 0) {
        for (i = 2; i > 0; i--) {
            for (j = 0; j < 3; j++) {
                resource->leftPositions[i][j] = resource->leftPositions[i - 1][j];
                resource->rightPositions[i][j] = resource->rightPositions[i - 1][j];
                resource->angles[i] = resource->angles[i - 1];
            }
        }
        lateral = -randomNextLinear() % 40;
        scratch->offset.vector.vy = 0;
        scratch->offset.vector.vx = lateral;
        scratch->offset.vector.vz = randomNextLinear() % 10 - 5;
        ApplyMatrixSV(&player->model.effect.effectMatrix180, &scratch->offset.vector,
            &scratch->offset.vector);
        for (j = 0; j < 3; j++) {
            resource->leftPositions[0][j] = player->model.effect.effectMatrix180.t[j] +
                scratch->offset.components[j];
        }

        scratch->offset.vector.vx = randomNextLinear() % 40;
        scratch->offset.vector.vz = randomNextLinear() % 10 - 5;
        ApplyMatrixSV(&player->model.effect.effectMatrix180, &scratch->offset.vector,
            &scratch->offset.vector);
        for (j = 0; j < 3; j++) {
            resource->rightPositions[0][j] = player->model.effect.effectMatrix180.t[j] +
                scratch->offset.components[j];
        }
        resource->angles[0] = randomNextLinear() % 4096;
    }

    GsInitCoordinate2(0, &scratch->coordinate);
    scratch->rotation.vx = scratch->rotation.vy = 0;
    scratch->size = player->unk0BC;
    if (scratch->size > 2048) {
        scratch->size = 2048;
    }
    scratch->shade = scratch->size / 64;
    for (i = 0; i < 3; i++) {
        scratch->rotation.vz = resource->angles[i];
        RotMatrix(&scratch->rotation, &scratch->coordinate.coord);
        trailScale = (i << 10) + 2048;
        scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = scratch->size + trailScale;
        ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);
        for (j = 0; j < 3; j++) {
            scratch->coordinate.coord.t[j] = resource->leftPositions[i][j];
        }
        GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
        for (j = 0; j < 3; j++) {
            scratch->coordinate.coord.t[j] = scratch->coordinate.workm.t[j];
        }
        GsSetLsMatrix(&scratch->coordinate.coord);
        scratch->depth = RotTransPers4(
            &gPlayerPositionTrailVertices[0], &gPlayerPositionTrailVertices[1],
            &gPlayerPositionTrailVertices[2], &gPlayerPositionTrailVertices[3],
            (long *)&scratch->quad.x0, (long *)&scratch->quad.x1,
            (long *)&scratch->quad.x2, (long *)&scratch->quad.x3,
            &scratch->cue, &scratch->flags);
        scratch->quad.r0 = scratch->quad.g0 = scratch->quad.b0 =
            scratch->shade - (scratch->shade * i) / 3;
        *packet = scratch->quad;
        scratch->depth -= 4;
        if (!(scratch->flags & PAIRED_QUAD_PROJECTION_FLAGS) && scratch->depth >= 0 &&
                scratch->depth < 4096) {
            addPrim(&gCurrentPlayerOrderingTable[(scratch->depth >> 2) - 4], packet);
        }
        packet++;
    }
    for (i = 0; i < 3; i++) {
        scratch->rotation.vz = resource->angles[i];
        RotMatrix(&scratch->rotation, &scratch->coordinate.coord);
        scratch->scale.vx = scratch->scale.vy = scratch->scale.vz =
            scratch->size + (i << 10);
        ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);
        for (j = 0; j < 3; j++) {
            scratch->coordinate.coord.t[j] = resource->rightPositions[i][j];
        }
        GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
        for (j = 0; j < 3; j++) {
            scratch->coordinate.coord.t[j] = scratch->coordinate.workm.t[j];
        }
        GsSetLsMatrix(&scratch->coordinate.coord);
        scratch->depth = RotTransPers4(
            &gPlayerPositionTrailVertices[0], &gPlayerPositionTrailVertices[1],
            &gPlayerPositionTrailVertices[2], &gPlayerPositionTrailVertices[3],
            (long *)&scratch->quad.x0, (long *)&scratch->quad.x1,
            (long *)&scratch->quad.x2, (long *)&scratch->quad.x3,
            &scratch->cue, &scratch->flags);
        scratch->quad.r0 = scratch->quad.g0 = scratch->quad.b0 =
            scratch->shade - (scratch->shade * i) / 3;
        *packet = scratch->quad;
        scratch->depth -= 4;
        if (!(scratch->flags & PAIRED_QUAD_PROJECTION_FLAGS) && scratch->depth >= 0 &&
                scratch->depth < 4096) {
            addPrim(&gCurrentPlayerOrderingTable[(scratch->depth >> 2)], packet);
        }
        packet++;
    }
    GsOUT_PACKET_P = (PACKET *)packet;
}

void createPlayerPositionTrailTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerPositionTrailResource *resource;
    s32 i, j;

    player = &gRacePlayers[playerIndex];
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, updatePlayerPositionTrailTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = playerIndex;
    resource = (PlayerPositionTrailResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            resource->leftPositions[i][j] = player->worldCoordinate.coord.t[j];
        }
    }
    resource->inactive = 1;
}

void updatePlayerTripleQuadEffectTask(void) {
    PlayerTripleQuadScratch *scratch;
    PlayerTripleQuadEffectResource *resource;
    RacePlayer *player;
    POLY_FT4 *packet;
    s32 quadIndex;
    s32 axis, scale, size;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerTripleQuadEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    player = &gRacePlayers[gCurrentTask->data18];
    scratch = (PlayerTripleQuadScratch *)getScratchAddr(0);
    if (isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0],
            player->worldCoordinate.coord.t[2], gCurrentTask->data18)) {
        GsInitCoordinate2(NULL, &scratch->coordinate);
        RotMatrix(&resource->rotation, &scratch->coordinate.coord);
        scale = resource->elapsed * 1000 + 4096;
        size = resource->scale;
        scratch->scale.vx = scratch->scale.vy = scratch->scale.vz = size + scale;
        ScaleMatrix(&scratch->coordinate.coord, &scratch->scale);

        scratch->quad.tag = TEXTURED_QUAD_PACKET_WORDS << 24;
        setcode(&scratch->quad, POLY_FT4_CODE);
        setSemiTrans(&scratch->quad, 1);
        setUV4(&scratch->quad, 0, 64, 63, 64, 0, 127, 63, 127);
        scratch->quad.clut = getClut(880, 248);
        scratch->quad.tpage = getTPage(0, 1, 704, 0);

        for (quadIndex = 0; quadIndex < 3; quadIndex++) {
            for (axis = 0; axis < 3; axis++) {
                scratch->coordinate.coord.t[axis] = resource->positions[quadIndex][axis];
            }
            GsGetLs(&scratch->coordinate, &scratch->coordinate.workm);
            for (axis = 0; axis < 3; axis++) {
                scratch->coordinate.coord.t[axis] = scratch->coordinate.workm.t[axis];
            }
            GsSetLsMatrix(&scratch->coordinate.coord);
            scratch->depth = RotTransPers4(&gPlayerTripleQuadVertices[0], &gPlayerTripleQuadVertices[1],
                &gPlayerTripleQuadVertices[2], &gPlayerTripleQuadVertices[3],
                (long *)&scratch->quad.x0, (long *)&scratch->quad.x1,
                (long *)&scratch->quad.x2, (long *)&scratch->quad.x3,
                &scratch->cue, &scratch->flags);

            if ((s16)resource->state != 0x200) {
                s32 shade = 32 - resource->elapsed * 4;
                scratch->quad.tpage = getTPage(0, 1, 704, 0);
                scratch->quad.r0 = scratch->quad.b0 = scratch->quad.g0 = shade;
            } else {
                scratch->quad.tpage = getTPage(0, 1, 704, 0);
                scratch->quad.r0 = scratch->quad.g0 = 30 - resource->elapsed * 30 / 8;
                scratch->quad.b0 = 28 - resource->elapsed * 28 / 8;
            }

            *packet = scratch->quad;
            if (!(scratch->flags & 0x7FFBE000) && scratch->depth >= 0 && scratch->depth < 4096) {
                addPrim(&gCurrentPlayerOrderingTable[scratch->depth >> 2], packet);
            }
            packet++;
            if (gRacePauseState == 0) {
                resource->positions[quadIndex][1] -= 4;
            }
        }
        GsOUT_PACKET_P = (PACKET *)packet;
    }
    if (gRacePauseState == 0) {
        resource->elapsed++;
    }
    if (resource->elapsed >= 8) {
        deleteCurrentTaskSubtree();
    }
}

void emitPlayerTurnTripleQuadEffectTask(void) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i, sine;
    player = &gRacePlayers[gCurrentTask->data18];
    if (gRacePauseState != 0) return;
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)getRenderCameraIndex())) return;
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) return;
    sine = rsin((s16)player->unk0BE - (s16)player->unk110);
    if (sine < 0) sine = -sine;
    if ((gFrameCounter & 3) == 0 && !(player->unk128 & 0x677DA21) && player->unk0BC >= 0x800 && sine >= 0x200) {
        createChildTaskForParentWithReserve(gPlayerSnapshotTask, updatePlayerTripleQuadEffectTask);
        gCreatedTask->resource14 = allocateTaskMemoryBlock();
        resource = (PlayerTripleQuadEffectResource *)&gTaskMemoryPool[gCreatedTask->resource14];
        vertex = gPlayerTripleQuadOffsets;
        for (i = 0; i < 3; i++) {
            ApplyMatrix(&player->model.effect.effectMatrix180, vertex, (VECTOR *)&resource->positions[i]);
            resource->positions[i][0] += player->worldCoordinate.coord.t[0];
            resource->positions[i][1] += player->worldCoordinate.coord.t[1];
            resource->positions[i][2] += player->worldCoordinate.coord.t[2];
            vertex++;
        }
        resource->elapsed = 0;
        resource->rotation.vx = 0;
        resource->rotation.vy = 0;
        resource->rotation.vz = randomNextLinear() % 4096;
        resource->state = player->unk96E;
        resource->scale = rsin((s16)player->unk0BE - (s16)player->unk110);
        if (resource->scale < 0) resource->scale = -resource->scale;
        gCreatedTask->data18 = gCurrentTask->data18;
    }
}

void startPlayerTurnTripleQuadEffectTask(s32 playerIndex) {
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, emitPlayerTurnTripleQuadEffectTask);
    gCreatedTask->data18 = playerIndex;
}

void updatePlayerRingEffectTask(void) {
    PlayerRingEffectResource *resource;
    PlayerRingEffectScratch *scratch;
    POLY_FT4 *packet;
    s32 i, j, angle;
    s32 scale;
    u8 shade;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PlayerRingEffectResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    scratch = (PlayerRingEffectScratch *)getScratchAddr(0);
    if (isPointWithinCameraForwardRange(resource->position[0], resource->position[2],
                                        (s16)getRenderCameraIndex())) {
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
                addPrim(&gCurrentPlayerOrderingTable[scratch->depth >> 2], packet);
            }
            packet++;
        }
        GsOUT_PACKET_P = (PACKET *)packet;
    }
    if (gRacePauseState == 0) {
        resource->position[1] += (-rcos((resource->elapsed << 11) / 20) * 10) / 4096;
        if (gRacePauseState == 0) {
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
    if (gSelectedCourseIndex != 3) {
        createChildTaskForParentWithReserve(gPlayerSnapshotTask, updatePlayerRingEffectTask);
        gCreatedTask->resource14 = allocateTaskMemoryBlock();
        resource = (PlayerRingEffectResource *)&gTaskMemoryPool[gCreatedTask->resource14];
        for (i = 0; i < 3; i++) {
            resource->position[i] = player->worldCoordinate.coord.t[i];
        }
        resource->position[1] = getCourseHeightAtPoint(player->unk92E, resource->position[0], resource->position[2]);
        for (i = 0; i < 6; i++) {
            resource->rotations[i].vy = 0;
            resource->rotations[i].vx = 0;
            resource->rotations[i].vz = randomNextLinear() % 4096;
        }
        resource->elapsed = 0;
        gCreatedTask->data18 = playerIndex;
    }
}

void updatePlayerParticleTrailTask(void) {
    PlayerParticleTrailResource *resource;
    PlayerParticleTrailScratch *scratch;
    RacePlayer *player;
    POLY_FT4 *packet;
    s32 i, j;
    s32 shade;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (PlayerParticleTrailScratch *)getScratchAddr(0);
    resource = (PlayerParticleTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    player = &gRacePlayers[gCurrentTask->data18];
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

    if (gRacePauseState == 0) {
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
                (s16)getRenderCameraIndex())) {
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
                addPrim(&gCurrentPlayerOrderingTable[scratch->depth >> 2], packet);
            }
            packet++;
            if (gRacePauseState == 0) {
                resource->positions[i][1] -= 2;
                if (resource->ages[i] < 11) {
                    resource->ages[i]++;
                }
            }
        }
    }
    GsOUT_PACKET_P = (PACKET *)packet;
}

void createPlayerParticleTrailTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerParticleTrailResource *resource;
    s32 i;
    s32 j;

    player = &gRacePlayers[playerIndex];
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, updatePlayerParticleTrailTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (PlayerParticleTrailResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 3; j++) {
            resource->positions[i][j] = player->worldCoordinate.coord.t[j];
        }
        resource->ages[i] = 10;
        resource->angles[i] = randomNextLinear() % 4096;
    }
    resource->unkA0 = 0;
    gCreatedTask->data18 = playerIndex;
}

void beginPlayerTripleQuadEffectTask(void) {
    setCurrentTaskCallback(updatePlayerTripleQuadEffectTask);
}

s32 createPlayerTripleQuadEffectTask(s32 playerIndex) {
    RacePlayer *player;
    PlayerTripleQuadEffectResource *resource;
    SVECTOR *vertex;
    s32 i;
    s32 state;
    player = &gRacePlayers[playerIndex];
    if (!isPointWithinCameraForwardRange(player->worldCoordinate.coord.t[0], player->worldCoordinate.coord.t[2], (s16)getRenderCameraIndex())) {
        return 0;
    }
    if (!(player->unk128 & 8) && player->unk96E != 0 && player->unk96E != 0x801 && player->unk96E != 0x200) {
        return 0;
    }
    createChildTaskForParentWithReserve(gPlayerSnapshotTask, beginPlayerTripleQuadEffectTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (PlayerTripleQuadEffectResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    vertex = gPlayerTripleQuadOffsets;
    for (i = 0; i < 3; i++) {
        ApplyMatrix(&player->model.effect.effectMatrix180, vertex, (VECTOR *)&resource->positions[i]);
        resource->positions[i][0] += player->worldCoordinate.coord.t[0];
        resource->positions[i][1] += player->worldCoordinate.coord.t[1];
        resource->positions[i][2] += player->worldCoordinate.coord.t[2];
        vertex++;
    }
    resource->elapsed = 0;
    resource->rotation.vx = 0;
    resource->rotation.vy = 0;
    resource->rotation.vz = randomNextLinear() % 4096;
    state = player->unk96E;
    resource->state = state;
    resource->scale = 0;
    gCreatedTask->data18 = playerIndex;
    return state;
}

void drawRisingTexturedStripTask(void) {
    POLY_FT4 *packet;
    RotatingQuadTrailResource *resource;
    s32 y;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (RotatingQuadTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    addPrim(gCurrentPlayerOrderingTable, packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
    if (gQuadTransitionFrame >= 50) {
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    scratch = (QuadScaleScratchpad *)getScratchAddr(0);
    resource = (RotatingQuadTrailResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
        addPrim(gCurrentPlayerOrderingTable, packet);
        packet++;
    }
    GsOUT_PACKET_P = (PACKET *)packet;
    if (--resource->countdown < -7) {
        setCurrentTaskCallback(drawRisingTexturedStripTask);
        resource->countdown = 0;
    }
}

void createRotatingQuadTrailTask(void) {
    RotatingQuadTrailResource *resource;
    s32 i;

    createChildTaskForParentWithReserve(&gGameTasks[4], drawRotatingQuadTrailTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (RotatingQuadTrailResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->countdown = 30;
    for (i = 0; i < 8; i++) {
        resource->angles[i] = resource->countdown * 256;
        resource->scales[i] = resource->countdown * 400 + 0x1000;
    }
}

void drawLoweringTexturedQuadTask(void) {
    POLY_FT4 *packet;
    ShrinkingQuadResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    resource = (ShrinkingQuadResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    addPrim(gCurrentPlayerOrderingTable, packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
    if (gQuadTransitionFrame >= 50) {
        if (++resource->countdown > 10) { resource->countdown = 10; }
    }
}

void drawShrinkingTexturedQuadTask(void) {
    s32 i;
    POLY_FT4 *packet;
    QuadTransitionResource *resource;
    QuadScaleScratchpad *scratch;
    if (gCurrentTask->resource14 == 0xFFFF) { while (1) {} }
    scratch = (QuadScaleScratchpad *)getScratchAddr(0);
    resource = (QuadTransitionResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    addPrim(gCurrentPlayerOrderingTable, packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
    if (--resource->frames <= 0) {
        setCurrentTaskCallback(drawLoweringTexturedQuadTask);
        resource->frames = 0;
    }
}

void createQuadShrinkTask(void) {
    QuadTransitionResource *resource;

    createChildTaskForParentWithReserve(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (QuadTransitionResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->frames = 5;
    resource->vertices = gQuadShrinkVertices;
    resource->packet = &gQuadShrinkPacket;
}

void startMiddleQuadShrinkTask(void) {
    ShrinkingQuadResource *resource;

    createChildTaskForParentWithReserve(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (ShrinkingQuadResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->countdown = 5;
    resource->vertices = gMiddleShrinkQuadVertices;
    resource->packet = &gMiddleShrinkQuadPacket;
}

void createRightQuadShrinkTask(void) {
    QuadTransitionResource *resource;

    createChildTaskForParentWithReserve(&gGameTasks[4], drawShrinkingTexturedQuadTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (QuadTransitionResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->frames = 5;
    resource->vertices = gRightQuadShrinkVertices;
    resource->packet = &gRightQuadShrinkPacket;
}

void drawPaletteCyclingStripTask(void) {
    POLY_FT4 *packet;
    PaletteStripResource *resource;
    s32 paletteRow;
    s32 paletteOffset;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (PaletteStripResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    packet->x0 = packet->x2 = -16;
    packet->x1 = packet->x3 = 143;
    packet->y0 = packet->y1 = 88;
    packet->y2 = packet->y3 = 103;
    paletteOffset = gCurrentTask->data18 * 3;
    paletteRow = resource->frame % 3 + 242;
    packet->clut = getClut(944, paletteOffset + paletteRow);
    addPrim(gCurrentPlayerOrderingTable, packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
    resource->frame++;
}

void drawLoweringQuadStripTask(void) {
    POLY_FT4 *packet;
    SlidingQuadResource *resource;
    s32 offset;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingQuadResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    packet->x0 = packet->x2 = -16;
    packet->x1 = packet->x3 = 143;
    offset = (resource->countdown << 6) / 10;
    packet->y0 = packet->y1 = offset + 24;
    packet->y2 = packet->y3 = offset + 39;
    packet->clut = getClut(944, gCurrentTask->data18 * 3 + 242);
    addPrim(gCurrentPlayerOrderingTable, packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
    if (gQuadTransitionFrame >= 50) {
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

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (SlidingQuadResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    packet = (POLY_FT4 *)GsOUT_PACKET_P;
    *packet = *resource->packet;
    packet->v0 = packet->v1 = resource->textureV;
    packet->v2 = packet->v3 = resource->textureV + 15;
    offset = resource->countdown * resource->countdown;
    packet->x0 = packet->x2 = offset - 16;
    packet->x1 = packet->x3 = offset + 143;
    packet->clut = getClut(944, gCurrentTask->data18 * 3 + 242);
    addPrim(gCurrentPlayerOrderingTable, packet);
    GsOUT_PACKET_P = (PACKET *)(packet + 1);
    if (--resource->countdown <= 0) {
        setCurrentTaskCallback(drawLoweringQuadStripTask);
        resource->countdown = 0;
    }
}

void startSlidingQuadTask(s32 index) {
    SlidingQuadResource *resource;
    createChildTaskForParentWithReserve(&gGameTasks[4], drawSlidingQuadTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    gCreatedTask->data18 = index;
    resource = (SlidingQuadResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->countdown = 10;
    resource->packet = &gSlidingQuadPacket;
    resource->textureV = (index << 4) + 32;
}

void waitThenStartPaletteColorCycle(void) {
    if (++gCurrentTask->data1A >= 90) {
        setCurrentTaskCallback(capturePaletteForColorCycle);
        gCurrentTask->data1A = 0;
    }
}

void updatePaletteWhitePulse(void) {
    s32 amount, i;
    u8 r, g, b;

    if (gCurrentTask->data18 < 10) {
        amount = gCurrentTask->data18;
    } else if (gCurrentTask->data18 < 20) {
        amount = 20 - gCurrentTask->data18;
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
    gCurrentTask->data18 = (gCurrentTask->data18 + 1) % 60;
}

void capturePaletteForColorCycle(void) {
    StoreImage(&gColorCycleCaptureRect, (u32 *)gCapturedColorCyclePalette);
    DrawSync(0);
    setCurrentTaskCallback(updatePaletteWhitePulse);
}

void startPaletteWhitePulseTask(s32 duration) {
    createChildTaskForParentWithReserve(&gGameTasks[4], capturePaletteForColorCycle);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1A = duration;
}

void updateQuadTransitionSequenceTask(void) {
    s32 frame = gQuadTransitionFrame;
    if (frame == 0) createRotatingQuadTrailTask();
    if (frame == 25) createQuadShrinkTask();
    if (frame == 30) startMiddleQuadShrinkTask();
    if (frame == 35) createRightQuadShrinkTask();
    if (frame == 40) startSlidingQuadTask(gCurrentTask->data1A);
    if (frame == 50) startPaletteWhitePulseTask(10);
    gQuadTransitionFrame++;
    if (gQuadTransitionFrame == 100) deleteCurrentTaskSubtree();
}

void startQuadTransitionSequenceTask(s32 transitionIndex) {
    createChildTaskForParentWithReserve(&gGameTasks[4], updateQuadTransitionSequenceTask);
    gQuadTransitionFrame = 0;
    gCreatedTask->data1A = transitionIndex;
}

void updatePrimaryValueTransition(void) {
    s32 value;
    value = gCurrentTask->data1C +
        (gCurrentTask->data1E - gCurrentTask->data1C) *
        gCurrentTask->data18 / gCurrentTask->data1A;
    gTimedSpritePrimaryBrightness = value;
    gCurrentTask->data18++;
    gTimedSpriteInterpolationComplete = 0;
    if (gCurrentTask->data18 >= gCurrentTask->data1A) {
        deleteCurrentTaskSubtree();
        gTimedSpriteInterpolationComplete = 1;
    }
}

void startPrimaryValueTransition(s32 duration, s32 start, s32 end) {
    createChildTaskForParentWithReserve(gCurrentTask, updatePrimaryValueTransition);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1A = duration;
    gCreatedTask->data1C = start;
    gCreatedTask->data1E = end;
}

void updateValueInterpolationTask(void) {
    s32 step;
    s32 start = gCurrentTask->data1C;
    step = (gCurrentTask->data1E - start) * gCurrentTask->data18;
    step /= gCurrentTask->data1A;
    gTimedSpriteSecondaryBrightness = start + step;
    gCurrentTask->data18++;
    gTimedSpriteInterpolationComplete = 0;
    if (gCurrentTask->data18 >= gCurrentTask->data1A) {
        deleteCurrentTaskSubtree();
        gTimedSpriteInterpolationComplete = 1;
    }
}

void startValueInterpolationTask(s32 duration, s32 start, s32 end) {
    createChildTaskForParentWithReserve(gCurrentTask, updateValueInterpolationTask);
    gCreatedTask->data18 = 0;
    gCreatedTask->data1A = duration;
    gCreatedTask->data1C = start;
    gCreatedTask->data1E = end;
}

void drawFullScreenSpriteTask(void) {
    ScreenSpriteResource *resource;
    DrawModeSpritePacket *packets;
    s32 i;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    packets = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    resource = (ScreenSpriteResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    for (i = 0; i < 2; i++) {
        resource->sprites[i].r0 = resource->sprites[i].g0 = resource->sprites[i].b0 = *(const u8 *)resource->brightness;
    }
    for (i = 0; i < 2; i++) {
        packets[i] = resource->sprites[i];
        addPrim(&gCurrentPlayerOrderingTable[1], &packets[i]);
    }
    GsOUT_PACKET_P = (PACKET *)(packets + 2);
    if (++resource->elapsed >= resource->duration || (gTaskTransitionFlags & 0x100)) {
        deleteCurrentTaskSubtree();
    }
}

void createFullScreenSpriteTask(s32 duration, s32 tpage, u16 clut, u16 u, u16 v,
                  const s32 *brightness) {
    ScreenSpriteResource *resource;
    s32 i;

    createChildTaskForParentWithReserve(&gGameTasks[4], drawFullScreenSpriteTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (ScreenSpriteResource *)&gTaskMemoryPool[gCreatedTask->resource14];
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
                    addPrim(&gCurrentPlayerOrderingTable[0], packet);
                    packet++;
                    *packet = *scratch;
                    packet->clut = getClut(768, 248);
                    packet->tpage |= getTPage(0, 2, 0, 0);
                    addPrim(&gCurrentPlayerOrderingTable[0], packet);
                    packet++;
                    goto nextCharacter;
                }
            }
        }
    nextCharacter:
        scratch->x0 += 16;
        text += 2;
    }
    GsOUT_PACKET_P = (PACKET *)packet;
}

void updateFadingTextTask(void) {
    FadingTextResource *resource;
    s32 remaining;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (FadingTextResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    setSpriteColor(255, 255, 255);
    remaining = resource->elapsed;
    if (remaining < 30 || (remaining = resource->duration - remaining) < 30) {
        resource->brightness = remaining * 255 / 30;
    } else {
        resource->brightness = 255;
    }
    drawFadingText(resource);
    gFadingTextComplete = 0;
    if (++resource->elapsed == resource->duration) {
        deleteCurrentTaskSubtree();
        gFadingTextComplete = 1;
    }
}

void createFadingTextTask(s32 duration, s32 x, s32 y, const char *text, s32 palette) {
    FadingTextResource *resource;
    createChildTaskForParentWithReserve(gCurrentTask, updateFadingTextTask);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (FadingTextResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->duration = duration;
    resource->elapsed = 0;
    resource->x = x;
    resource->y = y;
    resource->text = text;
    resource->clut = getClut(768, palette + 240);
}

void finishTimedSpriteSequence(void) {
    TimedSpriteSequenceResource *resource;

    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimedSpriteSequenceResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (--resource->duration < 0) {
        deleteCurrentTaskSubtree();
    }
}

void updateTimedSpriteSequence(void) {
    TimedSpriteSequenceResource *resource;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TimedSpriteSequenceResource *)&gTaskMemoryPool[gCurrentTask->resource14];
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
    createChildTaskForParentWithReserve(gCurrentTask, updateTimedSpriteSequence);
    gCreatedTask->resource14 = allocateTaskMemoryBlock();
    resource = (TimedSpriteSequenceResource *)&gTaskMemoryPool[gCreatedTask->resource14];
    resource->step = steps;
    resource->frame = 0;
}

void finishTimedSpritePresentation(void) {
    if (gCurrentTask->data18++ >= 30) {
        startCdVolumeFade(103, 20, 0, 1);
        gTaskTransitionFlags |= 0x100;
        deleteCurrentTaskDescendants();
        deleteCurrentTaskSubtree();
    }
}

void updateTimedSpritePresentation(void) {
    s32 frame = gCurrentTask->data18;

    gTimedSpritePresentationFrame = frame;
    if (frame == 0) {
        startValueInterpolationTask(15, 0, 64);
        createFullScreenSpriteTask(120, getTPage(1, 1, 320, 256), getClut(320, 496),
            0, 0, &gTimedSpriteSecondaryBrightness);
    }
    if (frame == 75) {
        startValueInterpolationTask(14, 64, 255);
    }
    if (frame == 90) {
        startValueInterpolationTask(30, 255, 0);
        startPrimaryValueTransition(30, 0, 128);
        createFullScreenSpriteTask(1410, getTPage(1, 1, 320, 0), getClut(320, 240),
            0, 0, &gTimedSpritePrimaryBrightness);
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
            0, 0, &gTimedSpriteSecondaryBrightness);
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
            0, 0, &gTimedSpritePrimaryBrightness);
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
            0, 0, &gTimedSpriteSecondaryBrightness);
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
            0, 0, &gTimedSpritePrimaryBrightness);
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
            0, 0, &gTimedSpriteSecondaryBrightness);
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
            0, 0, &gTimedSpritePrimaryBrightness);
    }

    if (frame == 4950 || (gControllerPressedButtons[0] & PRESENTATION_SKIP_BUTTON)) {
        gTimedSpritePresentationExiting = 1;
        gCurrentTask->data18 = 0;
        deleteCurrentTaskDescendants();
        startPrimaryValueTransition(30, (s16)gTimedSpritePrimaryBrightness, 0);
        startValueInterpolationTask(30, (s16)gTimedSpriteSecondaryBrightness, 0);
        setCurrentTaskCallback(finishTimedSpritePresentation);
    } else {
        gFadingTextComplete = 0;
        gTimedSpriteInterpolationComplete = 0;
        gCurrentTask->data18++;
    }
}

void initializeTimedSpritePresentation(void) {
    setSecondaryDisplayTaskMode(0);
    queueCdCommand(6, 1);
    gTimedSpritePrimaryBrightness = 0;
    gTimedSpriteSecondaryBrightness = 0;
    gTimedSpriteInterpolationComplete = 0;
    gFadingTextComplete = 0;
    gTimedSpritePresentationExiting = 0;
    gCurrentTask->data18 = 0;
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
    gCurrentTask->unk10 |= 2;
}

void createResourceLoadTask(void) {
    createChildTaskForParentWithReserve(&gGameTasks[4], loadTimedSpritePresentationResources);
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
    addPrim(gCurrentPlayerOrderingTable, sprite);
    GsOUT_PACKET_P = (PACKET *)(sprite + 1);
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
    addPrim(gCurrentPlayerOrderingTable, quad);
    GsOUT_PACKET_P = (PACKET *)(quad + 1);
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
    addPrim(gCurrentPlayerOrderingTable, page);
    GsOUT_PACKET_P = (PACKET *)(page + 1);
}

void setTextGlyphPalette(u16 palette) {
    palette &= 7;
    gTextGlyphPalette = palette;
}
