#include "common.h"
#define CONTROLLER_EFFECTS_ABSOLUTE_ADDRESSING
#include "game/controller_effects.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/task.h"
#include "game/selected_records.h"
#define SELECTED_RECORD_GP_ADDRESSING
#include "game/selected_record.h"
#include "psx/libsnd.h"
#include "psx/libgpu.h"
#define GS_PACKET_ABSOLUTE_ADDRESSING
#include "psx/libgs.h"

#define FLAT_RECTANGLE_CODE 0x60
#define DRAW_MODE_COMMAND_HIGH 0xE100

/* Draw mode and a flat rectangle share a single DMA packet tag. */
typedef struct FlatRectanglePacket {
    u32 tag;
    u16 tpage, drawModeCommand;
    u8 r0, g0, b0, code;
    s16 x0, y0, w, h;
} FlatRectanglePacket;

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

ControllerEffectState gControllerEffectStates[2] = {0};

u16 gTextClut = 0;

extern SelectedStateRecord *D_8012C6F4[];

const char gNoProcessMessage[16] = "NO PROCESS...";

void func_80049D48(s32 x, s32 y, const char *format, ...);
void func_8006BE50(s32 textureIndex, s32 clutOffset);
void func_8006C848(void);
void func_8006B1F4(void);
void func_80069F64(void);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8004AD10);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800515A8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005AB84);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EE64);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EECC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005EFA0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005F46C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005F640);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FB34);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FBC0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FC5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FD5C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FE7C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8005FFF8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800600A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800601C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060284);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800603C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060518);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060740);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80060D40);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80061DD8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065E34);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80065EC8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006624C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800663E8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006956C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800695E0);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069654);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800697DC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006999C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069B24);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069C04);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80069DC8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A128);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A958);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006A9CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006AAAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006AB18);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006AB8C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B19C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B1F4);

void createResourceLoadTask(void) {
    func_80014930(&D_800D7C84, func_8006B1F4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006B2E4);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C104);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C190);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C240);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C34C);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C428);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C5F4);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_8006C70C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800700F8);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_80070174);

INCLUDE_ASM("asm/nonmatchings/game/unk_800497B8", func_800701F4);
