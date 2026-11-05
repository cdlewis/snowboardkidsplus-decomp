#include "common.h"
#define CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
#include "game/controller_effects.h"
#define GS_PACKET_ABSOLUTE_ADDRESSING
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#define TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/race_player.h"
#include "game/selected_records.h"
#define SELECTED_RECORD_GP_ADDRESSING
#include "game/selected_record.h"
#include "psx/libsnd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

#define FLAT_RECTANGLE_CODE 0x60
#define TEXTURED_QUAD_CODE 0x2C
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

/* Geometry entries have a 0x1C-byte stride; the origin references a vertex. */
typedef struct GeometryPlacement {
    s16 unk00[8];
    s16 vertexIndex;
    u16 angle;
    s16 unk14[4];
} GeometryPlacement;
typedef struct SemiTransparentTrianglePacket {
    u32 tag;
    u16 tpage, drawModeCode;
    u8 r0, g0, b0, code;
    s16 x0, y0, x1, y1, x2, y2;
} SemiTransparentTrianglePacket;

typedef struct SelectedStateRecord {
    s16 state;
    s16 unk02;
    s32 unk04;
    s32 unk08;
    s32 unk0C;
    s32 unk10;
} SelectedStateRecord;
typedef struct ControllerEffectStep {
    u8 value;
    s8 delta;
    u8 pad[2];
    s32 duration;
} ControllerEffectStep;
typedef struct ControllerEffectState {
    u8 effect;
    u8 priority;
    u8 pad[2];
    ControllerEffectStep *step;
    s32 elapsed;
    s32 stepEnd;
    s32 effectEnd;
} ControllerEffectState;

typedef struct QuadVertex {
    s16 x, y, z, pad;
} QuadVertex;

typedef struct ShrinkingQuadResource {
    s32 countdown;
    const QuadVertex *vertices;
    const POLY_FT4 *packet;
    u8 pad0C[0xF4];
} ShrinkingQuadResource;

typedef struct SlidingQuadResource {
    s32 countdown;
    s32 unk04;
    const POLY_FT4 *packet;
    s32 textureV;
    u8 pad10[0xF0];
} SlidingQuadResource;

/* The effect callback uses the first three words as world coordinates. */
typedef struct PositionEffectOrigin {
    s32 x, y, z;
    s32 pad;
} PositionEffectOrigin;

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
    s32 unk0C;
    s16 palette;
    s16 pad12;
} TimedSpriteStep;

typedef struct TimedSpriteSequenceResource {
    s32 frame;
    s32 duration;
    const TimedSpriteStep *step;
    u8 pad0C[0xF4];
} TimedSpriteSequenceResource;

ControllerEffectState gControllerEffectStates[2] = {0};

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

extern GameTask *D_8013B304 __attribute__((section(".data")));
RECT gReadPixelRect __attribute__((section(".data.read_pixel_rect"))) = {0, 0, 1, 1};

/* The original stores coordinates through GP but passes an absolute address. */
extern RECT readPixelRectGp;

extern SelectedStateRecord *D_8012C6F4[];
extern s8 *D_8012BBB4[][4];
/* Shared palette storage and capture rectangle remain in their data owners. */
extern u32 D_8012CB94[8];
extern RECT D_8013B330 __attribute__((section(".data")));
extern u32 *D_8013AF6C __attribute__((section(".data")));
extern s32 D_8013B35C;
extern s32 D_8013B360;
extern s32 D_8013B364;
extern s32 D_8013B368;
extern s32 D_8013B36C;
extern u16 D_8013B376 __attribute__((section(".sdata")));


s32 func_80060DC8(u32 *tim);
void func_800692E8(void);
void func_8006999C(void);
void func_800618A0(void);
void func_8006624C(void);
void func_800659F0(void);
extern SVECTOR D_8012A5C0[];
extern GsCOORDINATE2 D_80129C60[];

const char gNoProcessMessage[16] = "NO PROCESS...";

void func_8001E3E4(SVECTOR *rotation, GsCOORDINATE2 *coordinate);
void func_800592DC(u16 index);
void func_80049D48(s32 x, s32 y, const char *format, ...);
void func_8006BE50(s32 textureIndex, s32 clutOffset);
void func_8006BF14(s16 x, s16 y, s32 frameIndex, GsOT *ot, u16 depth);
void func_8006C5F4(s16 x, s16 y, s32 frameIndex, GsOT *ot, u16 depth);
void func_8006C848(void);
void func_8006B1F4(void);
void func_80013DD0(s16 request, s16 mode);
void func_8006AB8C(void);
void func_80069F64(void);
void func_80069C04(void);
void func_8006A9CC(void);
void func_8006FE68(s16 x, s16 y, s16 w, s16 h, s16 borderW, s16 borderH,
                   s16 style, s16 clutOffset, s16 blendMode, GsOT *ot, u16 depth);
void func_8006A07C(void);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004A4C4);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004AC38);

void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle) {
    *x = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3] * 1280;
    *y = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3 + 1] * 1280;
    *z = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3 + 2] * 1280;
    *angle = ((GeometryPlacement *)D_8013B1F4)[index].angle;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004ADF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004B044);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004BC3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004C81C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004C968);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004D330);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004D7F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004D93C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004E09C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004E718);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004EBF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004EF70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004EFE0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004F374);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004F46C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004F69C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004F780);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004FA74);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004FB70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050080);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050504);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800508D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050B18);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80050F84);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800512A4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800512E8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005143C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800520E8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052314);

void drawNoProcessMessage(void) {
    func_80049D48(0, 30, gNoProcessMessage);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052530);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052680);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052840);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800529FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052C1C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052D84);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80052F90);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80053180);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80053408);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80053A34);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800542BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80054E70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80054F70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800550A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80055200);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80055390);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800554C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80055650);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800557F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800559F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80055C1C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80055E10);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80056034);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800562C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005652C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800567F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80056AC4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80056D18);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80056F90);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800572C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80057580);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800578A0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80057AC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80057D84);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005801C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058350);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058560);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800585BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058664);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058844);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800589F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058A78);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80058BD4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059254);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059290);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800592DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800593B0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059484);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059958);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059C3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059DBC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80059F68);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005A1C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005A360);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005A4D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005A598);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005A660);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005AF0C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B038);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B3A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B460);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B608);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B6C0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005B978);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005BA8C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005BC70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005BF00);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005C51C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005C7AC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005C8BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005CB08);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005CC38);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005CCEC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005CEF0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005D1B0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005D394);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005D828);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005DD70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005DE2C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005DF98);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005E174);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005E338);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005E454);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005E578);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005E728);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EBC4);

void updateAndDrawCurrentTaskObject(void) {
    func_8001E3E4(&D_8012A5C0[D_8013AE88->data18],
                  &D_80129C60[D_8013AE88->data18]);
    func_800592DC(D_8013AE88->data18);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EECC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EFA0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005F46C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005F730);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005F944);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FC5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FD5C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800600A8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060284);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800603C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060518);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006081C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060C68);

/* No callers are known; only the variadic argument layout is established. */
void unusedVariadicStub(s32 unused0, s32 unused1, s32 unused2, ...) {
}

s32 loadTimArchive(OffsetArchive *archive) {
    u32 i;
    u32 count = archive->count;
    for (i = 1; i < count + 1; i++) {
        if (!func_80060DC8((u32 *)((u8 *)archive + archive->offsets[i - 1]))) {
            return 0;
        }
    }
    return 1;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060DC8);

s32 randomNextLinear(void) {
    gLinearRandomState = gLinearRandomState * 1103515245 + 12345;
    return ((u32)gLinearRandomState >> 16) & 0x7FFF;
}

s32 randomLinearFromValue(s32 value) {
    value = value * 1103515245 + 12345;
    return (value / 65536) & 0x7FFF;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060E80);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060F74);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060FE4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800612BC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061304);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061348);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061594);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80062A70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80062B24);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80062DA8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80062EA0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800632C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063788);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063890);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063B58);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063D70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80063DF4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006435C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80064454);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80064754);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006479C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065330);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065464);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800654B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065818);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800659F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065DA0);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006624C);

void startPositionEffectTask(const PositionEffectOrigin *position) {
    PositionEffectOrigin *resource;
    func_80014930(D_8013B304, func_8006624C);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = 3;
    resource = (PositionEffectOrigin *)&D_800D9504[D_8013AE8C->resource14];
    *resource = *position;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80066474);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80066C3C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80066D18);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067620);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800676FC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067B30);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067D88);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80067DD0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80068230);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006836C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800688C0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800689EC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80068A10);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80068BC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80068D1C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069024);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800690C8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800692E8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800694F8);

void startMiddleQuadShrinkTask(void) {
    ShrinkingQuadResource *resource;

    func_80014930(&D_800D7C84, func_800692E8);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (ShrinkingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 5;
    resource->vertices = gMiddleShrinkQuadVertices;
    resource->packet = &gMiddleShrinkQuadPacket;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800695E0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069654);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800697DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006999C);

void startSlidingQuadTask(s32 index) {
    SlidingQuadResource *resource;
    func_80014930(&D_800D7C84, func_8006999C);
    D_8013AE8C->resource14 = allocateTaskResource();
    D_8013AE8C->data18 = index;
    resource = (SlidingQuadResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->countdown = 10;
    resource->packet = &gSlidingQuadPacket;
    resource->textureV = (index << 4) + 32;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069C04);

void capturePaletteForColorCycle(void) {
    StoreImage(&D_8013B330, D_8012CB94);
    DrawSync(0);
    setCurrentTaskCallback(func_80069C04);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069E08);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069E54);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069F1C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069F64);

void startPrimaryValueTransition(s32 duration, s32 start, s32 end) {
    func_80014930(D_8013AE88, func_80069F64);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
    D_8013AE8C->data1C = start;
    D_8013AE8C->data1E = end;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A07C);

void startValueInterpolationTask(s32 duration, s32 start, s32 end) {
    func_80014930(D_8013AE88, func_8006A07C);
    D_8013AE8C->data18 = 0;
    D_8013AE8C->data1A = duration;
    D_8013AE8C->data1C = start;
    D_8013AE8C->data1E = end;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A194);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A2F8);

void *getOffsetArchiveEntry(OffsetArchive *archive, s32 index) {
    u32 *offsets = archive->offsets;
    return (u8 *)archive + offsets[index];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A438);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A4E8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A7B4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A8A8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A9CC);

void startTimedSpriteSequence(const TimedSpriteStep *steps) {
    TimedSpriteSequenceResource *resource;
    func_80014930(D_8013AE88, func_8006A9CC);
    D_8013AE8C->resource14 = allocateTaskResource();
    resource = (TimedSpriteSequenceResource *)&D_800D9504[D_8013AE8C->resource14];
    resource->step = steps;
    resource->frame = 0;
}

void finishTimedSpritePresentation(void) {
    if (D_8013AE88->data18++ >= 30) {
        func_80013294(103, 20, 0, 1);
        D_8013AFE4 |= 0x100;
        func_800149E0();
        func_800149A8();
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006AB8C);

void initializeTimedSpritePresentation(void) {
    func_800719A8(0);
    func_80013DD0(6, 1);
    D_8013B35C = 0;
    D_8013B360 = 0;
    D_8013B364 = 0;
    D_8013B368 = 0;
    D_8013B36C = 0;
    D_8013AE88->data18 = 0;
    setCurrentTaskCallback(func_8006AB8C);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B1F4);

void createResourceLoadTask(void) {
    func_80014930(&D_800D7C84, func_8006B1F4);
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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B38C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006BD3C);

void initPrimitiveTemplates(void) {
    initTexturedQuadTemplate();
    func_8006C848();
}

void initTexturedQuadTemplate(void) {
    SetPolyFT4((P_TAG *)&gTexturedQuadTemplate);
    func_8006BE50(0, 0);
    setRGB0(&gTexturedQuadTemplate, 0x80, 0x80, 0x80);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006BE50);

void setTexturedQuadColor(s32 r, s32 g, s32 b) {
    setRGB0(&gTexturedQuadTemplate, r, g, b);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006BEEC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006BF14);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C018);

void drawTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame = &D_8012DFF4[index];

    func_8006BE50(frame->textureIndex, 0);
    func_8006BF14(x, y, frame->frameIndex, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C190);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C240);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C34C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C428);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C5F4);

void drawInclusiveTexturedQuadByIndex(s16 x, s16 y, s32 index, GsOT *ot, u16 depth) {
    TexturedQuadFrame *frame = &D_8012DFF4[index];

    func_8006BE50(frame->textureIndex, 0);
    func_8006C5F4(x, y, frame->frameIndex, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C798);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C848);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C8B0);

void setSpriteColor(s32 r, s32 g, s32 b) {
    setRGB0(&gSpriteTemplate, r, g, b);
    setRGB0(&gTexturedSpriteTemplate, r, g, b);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C970);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C998);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006CB70);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006CC90);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006CD28);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006CDA4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006CF38);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D134);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D29C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D41C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D538);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D66C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006D984);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006DC94);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006DFB4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006E2C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006E5E0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006E814);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006E998);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006EB00);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006EC84);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006EF30);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006F248);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006F57C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006F754);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006F8A4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006FC40);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006FDE4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006FE68);

/* Style 0 uses an eight-pixel border on each side. */
void drawDefaultTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 8, 8, 0, clutOffset, blendMode, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80070174);

/* Style 0x20D uses a sixteen-pixel border on each side. */
void drawStyle20DTexturedPanel(s16 x, s16 y, s16 w, s16 h, s16 clutOffset,
                              s16 blendMode, GsOT *ot, u16 depth) {
    func_8006FE68(x, y, w, h, 16, 16, 0x20D, clutOffset, blendMode, ot, depth);
}
