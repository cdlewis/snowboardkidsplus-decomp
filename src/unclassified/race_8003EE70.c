#include "../race/race_presentation_internal.h"

/* Full-width Shift-JIS labels used by the race HUD. */
const char gAttackTimeLimitLabel[24] __attribute__((section(".rodata.trick_attack_labels"), aligned(4))) =
    "\x82\x73\x82\x68\x82\x6C\x82\x64  \x82\x6B\x82\x68\x82\x6C\x82\x68\x82\x73";

const char gTrickAttackPointLabel[12] __attribute__((section(".rodata.trick_attack_labels"), aligned(4))) =
    "\x82\x6F\x82\x6E\x82\x68\x82\x6D\x82\x73";

const char gTargetTimeLabel[12] __attribute__((section(".rodata.target_time_label"), aligned(4))) =
    "TARGET TIME";

extern char gCourseCoinTotalFormat[] __attribute__((section(".data")));
extern char gCourseCoinCountMarker[] __attribute__((section(".data")));
extern char gCourseCoinCountFormat[] __attribute__((section(".data")));

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_8003EE70);

void addFourWordGpuPacket(GpuPacket4 *source, GsOT *ot, u16 depth) {
    GpuPacket4 *packet = (GpuPacket4 *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_800400C4);

void addTexturedSpritePacket(TexturedSpritePacket *source, GsOT *ot, u16 depth) {
    TexturedSpritePacket *packet = (TexturedSpritePacket *)GsGetWorkBase();
    *packet = *source;
    AddPrim(&ot->org[depth], packet);
    func_800BF2C8((u32)(packet + 1));
}

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_800401DC);

void drawMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, GsOT *ot, u16 depth) {
    s32 frameIndex = frame;
    s32 sprite;
    if (kind == 0) {
        frame = 5;
        sprite = 0x1BE;
    } else {
        s32 baseSprite = 0x1BE;
        u8 *palettes = gMenuSpritePaletteGroups.kind;
        frame = palettes[kind];
        sprite = (kind - 1) * 3 + (frameIndex + baseSprite);
    }
    func_8006C998(x, y, (s16)sprite, frame, -1, ot, depth);
}

void drawScaledMenuKindSprite(s16 x, s16 y, s16 kind, s32 frame, s16 scaleX, s16 scaleY, GsOT *ot, u16 depth) {
    s32 frameIndex = frame;
    s32 sprite;
    if (kind == 0) {
        frame = 5;
        sprite = 0x1BE;
    } else {
        s32 baseSprite = 0x1BE;
        u8 *palettes = gMenuSpritePaletteGroups.kind;
        frame = palettes[kind];
        sprite = (kind - 1) * 3 + (frameIndex + baseSprite);
    }
    if (scaleX > 0 && scaleY > 0) {
        drawTransformedSpriteByIndex((s16)(x + 16), (s16)(y + 16), (s16)sprite, frame, -1,
                     0x7FFF, 0x7FFF, scaleX, scaleY, 0, ot, depth);
    } else {
        func_8006C998(x, y, (s16)sprite, frame, -1, ot, depth);
    }
}

void drawPaletteMappedIcon(s16 x, s16 y, s16 index, struct GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        u8 *palettes = gMenuSpritePaletteGroups.mappedIconPalettes;
        palette = palettes[index];
        sprite = index + 461;
    }
    func_8006C998(x, y, sprite, palette, -1, ot, depth);
}

void drawScaledPaletteMappedIcon(s16 x, s16 y, s16 index, s16 scaleX, s16 scaleY, GsOT *ot, u16 depth) {
    s16 sprite;
    s16 palette;
    if (index == 0) {
        palette = 5;
        sprite = 446;
    } else {
        u8 *palettes = gMenuSpritePaletteGroups.mappedIconPalettes;
        palette = palettes[index];
        sprite = index + 461;
    }
    if (scaleX > 0 && scaleY > 0) {
        drawTransformedSpriteByIndex((s16)(x + 16), (s16)(y + 16), sprite, palette, -1,
                       0x7FFF, 0x7FFF, scaleX, scaleY, 0, ot, depth);
    } else {
        func_8006C998(x, y, sprite, palette, -1, ot, depth);
    }
}

void unpackUpperWordBytes(u32 packed, UnpackedWordBytes *out) {
    u32 byte;

    packed >>= 8;
    byte = packed & 0xFF;
    packed >>= 8;
    out->unk1 = packed;
    packed >>= 8;
    out->unk2 = byte;
    out->unk0 = packed;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_8004054C);

void initializeRaceRulesAndCounters(void) {
    RaceTime *times;
    u8 *flags;
    D_8013B620 = -1;
    switch (D_8013AFE2) {
    case 0:
        if (D_8013AFCD == 1) {
            D_8013B620 = 0;
        } else {
            D_8013B620 = 1;
        }
        break;
    case 1:
        if (D_8013AFE0 == 0) {
            if (D_8013AFCD == 1) {
                D_8013B620 = 2;
            } else {
                D_8013B620 = 3;
            }
            gRacePlayers[0].collectedCourseCoinMarkerCount = 0;
            gRacePlayers[0].totalCourseCoinMarkerCount = D_8012429C[D_8013AFCE];
            gRacePlayers[1].collectedCourseCoinMarkerCount = 0;
            gRacePlayers[1].totalCourseCoinMarkerCount = D_8012429C[D_8013AFCE];
        } else if (D_8013AFE0 == 1) {
            if (D_8013AFCD == 1) {
                D_8013B620 = 4;
                D_8013B184 = D_80124248[D_8013AFCE];
            } else {
                D_8013B620 = 5;
                D_8013B184 = D_80124248[D_8013AFCE];
                D_8013B198[0] = D_80124248[D_8013AFCE];
                D_8013B198[1] = D_80124248[D_8013AFCE];
            }
            gRacePlayers[0].scoreAttackPointTotal = 0;
            gRacePlayers[0].scoreAttackPointTarget = D_801242A8[D_8013AFCE];
            gRacePlayers[1].scoreAttackPointTotal = 0;
            gRacePlayers[1].scoreAttackPointTarget = D_801242A8[D_8013AFCE];
        } else if (D_8013AFE0 == 2) {
            if (D_8013AFCD == 1) {
                D_8013B620 = 6;
                D_8013B184 = D_8013B154;
            } else {
                D_8013B620 = 7;
                D_8013B184 = D_8013B154;
                D_8013B198[0] = D_8013B154;
                D_8013B198[1] = D_8013B154;
            }
        }
        break;
    case 2:
        D_8013B620 = 8;
        break;
    }
    if (D_8013AFD8 != 0 || D_8013AFD9 != 0) {
        D_8013B620 = 9;
    }
    flags = D_8013B1A0;
    flags[0] = 0;
    gRaceElapsedTime.minutes = 0;
    gRaceElapsedTime.seconds = 0;
    gRaceElapsedTime.centiseconds = 0;
    D_8013B18C = 0;
    D_8013B18D_gp = 0;
    D_8013B1A8_gp = 0;
    flags[1] = 0;
    times = D_8013B190;
    times[0].minutes = 0;
    times[0].seconds = 0;
    times[0].centiseconds = 0;
    times[1].minutes = 0;
    times[1].seconds = 0;
    times[1].centiseconds = 0;
}

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80040D5C);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80041554);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80041890);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_800418F4);

void drawTrickAttackHud(void) {
    s16 palette = 1;
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    u16 depth = palette;
    s16 x;
    s16 i;

    if (D_8013B184.minutes == 0 && D_8013B184.seconds < 10 && (gFrameCounter & 1)) {
        palette = 2;
    }

    setSpriteColor(128, 128, 128);
    if (D_8013B18D_gp) {
        func_8006C998(64, 84, 398, 5, -1, ot, depth);
        palette = 2;
    }
    drawHudDecimalNumber8x16(64, 80, 2, 1, D_8013B184.minutes, palette, -1, ot, depth);
    drawHudDecimalNumber8x16(88, 80, 2, 1, D_8013B184.seconds, palette, -1, ot, depth);
    drawHudDecimalNumber8x16(112, 80, 2, 1, D_8013B184.centiseconds >> 8, palette, -1, ot, depth);
    func_8006C998(80, 80, 25, palette, -1, ot, depth);
    func_8006C998(104, 80, 24, palette, -1, ot, depth);
    drawWrappedText8x8(56, 71, 10, gAttackTimeLimitLabel, 5, -1, ot, depth);

    if (gRacePlayers[0].trickAttackMakeCount < 10) {
        x = -112;
        for (i = 0; i < gRacePlayers[0].trickAttackMakeCount; i++) {
            drawTransformedSpriteByIndex(x, -72, 431, 13, -1, 0x7FFF, 0x7FFF, 0x555, 0x555, 0, ot, depth);
            x += 8;
        }
    } else {
        func_8006C998(-132, -96, 431, 13, -1, ot, depth);
        drawHudDecimalNumber8x16(-100, -89, 2, 0, gRacePlayers[0].trickAttackMakeCount, 1, -1, ot, depth);
    }

    drawHudDecimalNumber8x16(92, -88, 5, 0, gRacePlayers[0].trickAttackPointTotal, 1, -1, ot, depth);
    drawWrappedText8x8(96, -97, 5, gTrickAttackPointLabel, 5, -1, ot, depth);
    func_8006C998(-112, 64, 422, 2, -1, ot, depth);
    func_8006C998(-136, 64, 419, 14, -1, ot, depth);
}

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_8004213C);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80042558);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80042894);

void drawCourseCoinHud(void) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    char text[16];
    s16 digits;

    setSpriteColor(128, 128, 128);
    if (D_8013B18D_gp) {
        func_8006C998(72, 84, 398, 5, -1, ot, 1);
    }
    drawHudDecimalNumber8x16(72, 80, 2, 1, gRaceElapsedTime.minutes, 1, -1, ot, 1);
    drawHudDecimalNumber8x16(96, 80, 2, 1, gRaceElapsedTime.seconds, 1, -1, ot, 1);
    drawHudDecimalNumber8x16(120, 80, 2, 1, gRaceElapsedTime.centiseconds >> 8, 1, -1, ot, 1);
    func_8006C998(88, 80, 25, 1, -1, ot, 1);
    func_8006C998(112, 80, 24, 1, -1, ot, 1);
    drawLimitedText8x8(72, 71, 10, gLapTimeLabel, 5, -1, ot, 1);

    sprintf(text, gLapTimeFormat, D_80124220[D_8013AFCE].minutes,
            D_80124220[D_8013AFCE].seconds, D_80124220[D_8013AFCE].centiseconds >> 8);
    drawLimitedText8x8(72, -87, 20, text, 7, -1, ot, 1);
    drawLimitedText8x8(50, -96, 11, gTargetTimeLabel, 7, -1, ot, 1);

    drawPaletteMappedIcon(-136, -96, 1, ot, 1);
    sprintf(text, gCourseCoinCountFormat, gRacePlayers[0].collectedCourseCoinMarkerCount);
    digits = strlen(text);
    drawLimitedText8x8(-104, -72, 1, gCourseCoinCountMarker, 6, -1, ot, 1);
    drawHudDecimalNumber8x16(-96, -80, digits, 0, gRacePlayers[0].collectedCourseCoinMarkerCount, 1, -1, ot, 1);
    sprintf(text, gCourseCoinTotalFormat, gRacePlayers[0].totalCourseCoinMarkerCount);
    drawLimitedText8x8(-96 + digits * 8, -72, 10, text, 6, -1, ot, 1);

    func_8006C998(-112, 64, 423, 3, -1, ot, 1);
    func_8006C998(-136, 64, 420, 14, -1, ot, 1);
}

void drawSplitScreenCourseCoinHud(s16 player) {
    GsOT *hudOt = &D_800EBFF0[D_8013AF78];
    GsOT *timeOt;
    s32 hudY;
    s16 timeY;
    u16 depth;
    char text[16];

    if (player == 0) {
        timeOt = &D_800EC018[D_8013AF78];
        hudY = 0;
        timeY = 0;
    } else {
        timeOt = &D_800EC0B8[D_8013AF78];
        hudY = 104;
        timeY = 0;
    }

    depth = 1;
    setSpriteColor(128, 128, 128);
    if (func_8003D904(player)) {
        if (D_8013B1A0[player]) {
            func_8006C998(-112, timeY + 39, 398, 5, -1, timeOt, 1);
        }
        drawHudDecimalNumber8x16(-112, timeY + 36, 2, 1, D_8013B190[player].minutes, 1, -1, timeOt, 1);
        drawHudDecimalNumber8x16(-88, timeY + 36, 2, 1, D_8013B190[player].seconds, 1, -1, timeOt, 1);
        drawHudDecimalNumber8x16(-64, timeY + 36, 2, 1, D_8013B190[player].centiseconds >> 8, 1, -1, timeOt, 1);
        func_8006C998(-96, timeY + 36, 25, 1, -1, timeOt, 1);
        func_8006C998(-72, timeY + 36, 24, 1, -1, timeOt, 1);
        drawLimitedText8x8(-112, timeY + 28, 10, gLapTimeLabel, 5, -1, timeOt, 1);
        sprintf(text, gLapTimeFormat, D_80124220[D_8013AFCE].minutes,
                D_80124220[D_8013AFCE].seconds, D_80124220[D_8013AFCE].centiseconds >> 8);
        drawLimitedText8x8(-112, timeY - 28, 20, text, 7, -1, timeOt, 1);
        drawLimitedText8x8(-112, timeY - 36, 11, gTargetTimeLabel, 7, -1, timeOt, 1);
    }

    drawTransformedSpriteByIndex(112, (s16)(hudY - 80), 462, 5, -1, 0x7FFF, 0x7FFF,
                                 0xC00, 0xC00, 0, hudOt, depth);
    drawLimitedText8x8(124, hudY - 76, 1, gCourseCoinCountMarker, 6, -1, hudOt, depth);
    drawHudDecimalNumber8x16(132, hudY - 84, 2, 0,
                            gRacePlayers[player].collectedCourseCoinMarkerCount, 1, -1, hudOt, depth);
    drawTransformedSpriteByIndex(135, (s16)(hudY - 24), 423, 3, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth);
    drawTransformedSpriteByIndex(120, (s16)(hudY - 24), 420, 14, -1, 0x7FFF, 0x7FFF,
                                 0x1000, 0x1000, 0, hudOt, depth);
    if (player == 1) {
        hudY += 18;
    }
    func_8006C998(96, hudY - 121, 417, 12, -1, hudOt, depth);
}

char gCourseCoinTotalFormat[4] __attribute__((section(".data"), aligned(4))) = "/%d";
char gCourseCoinCountMarker[2] __attribute__((section(".data"), aligned(4))) = "*";
u16 D_8013B17A __attribute__((section(".data"))) = 1;
char gCourseCoinCountFormat[4] __attribute__((section(".data"), aligned(4))) = "%d";
