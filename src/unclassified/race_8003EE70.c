#include "../race/race_presentation_internal.h"

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

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80041DC0);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_8004213C);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80042558);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80042894);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80042DC0);

INCLUDE_ASM("asm/nonmatchings/unclassified/race_8003EE70", func_80043120);
