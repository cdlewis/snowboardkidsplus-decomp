#include "../race/player/player_actions_internal.h"

void resetGameOptionState(void) {
    D_8013B108 = 2;
    D_8013B109 = 0;
    D_8013B10A = 0;
    gGameOptionFlags = 0x3E;
}

void queueBackgroundTextureSectorCommands(void) {
    queueCdSectorCommand(2, 0x480, 0x4A, (u8 *)0x80180000, 0);
    queueCdSectorCommand(0, 0xC9B, 1, (u8 *)0x80180000, 0);
    queueCdSectorCommand(0, 0xC9C, 1, (u8 *)0x80180800, 0);
    queueCdSectorCommand(0, 0xC78, 13, (u8 *)0x80181000, 0);
    queueCdSectorCommand(0, 0xC85, 4, (u8 *)0x80187800, 0);
}

void uploadBufferedTexturePair(void) {
    uploadTimImageWithOffset((u32 *)0x80187800, NULL);
    uploadTimImageWithOffset((u32 *)0x80181000, NULL);
}

void uploadTextureArchiveEntry(s16 x, s16 y, s16 clutX, s16 clutY, s32 index) {
    OffsetArchive *archive = (OffsetArchive *)0x80189800;
    s32 offset;
    /* The archive word index includes the count and wraps as a signed halfword. */
    index = (s16)index + 1;
    index = (s16)index;
    offset = (s32)archive->offsets[index - 1];
    uploadTimImageAt(x, y, clutX, clutY,
                  (u32 *)((s16 *)archive + (offset >> 1)));
}

void initDefaultBackground(s16 mode, s16 unused, GameBackground *background,
                           GameBackgroundMap *map) {
    func_80071300(mode, unused, background, map,
                  (u8 *)0x80180000, (u8 *)0x80180800);
}

void queueStartupSectorReads(void) {
    queueCdSectorCommand(0, 1, 5, (u8 *)0x8017D000, 0);
    queueCdSectorCommand(0, 6, 18, (u8 *)0x801F0000, 0);
}

INCLUDE_ASM("asm/nonmatchings/graphics/background_resources", func_8002F394);

void uploadTimImageAt(s16 x, s16 y, s16 clutX, s16 clutY, u32 *tim) {
    GsIMAGE image;
    if (*tim == 0x10) {
        GsGetTimInfo(tim + 1, &image);
        image.px = x;
        image.py = y;
        LoadImage((RECT *)&image.px, image.pixel);
        if ((image.pmode >> 3) & 1) {
            image.cx = clutX;
            image.cy = clutY;
            LoadImage((RECT *)&image.cx, image.clut);
        }
    }
}

void initializeBackgroundCameraTask(void) {
    D_8013AF7A = 320;
    D_8013AF7C = 240;
    D_8013B10C = -1;
    D_8013B10E = -1;
    D_8013B106 = 0;
    configureTaskStateTransition(11, 0, 30);
    setSecondaryDisplayTaskMode(1);
    queueBackgroundTextureSectorCommands();
    D_8013AF84 = 400;
    initPlayerCamera(0);
    func_8003DEE0();
    func_80014838(func_8002F65C);
    D_8013AE8C->data18 = D_8013AE88->data18;
    D_8013AE8C->data1C = 0;
    func_8002FA58((s16)D_8013AE8C->data18);
    D_8013AE88->data1E = 0;
    setCurrentTaskCallback(func_8002F548);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/graphics/background_resources", func_8002F548);

void initializeViewAndSelectTaskCallback(void) {
    if (D_8013AE88->data1E != 0) {
        D_8013AF84 = 0x100;
        initPlayerCamera(0);
        func_8003DEE0();
        if (D_8013AFD7 != 0) {
            goto restore;
        }
    } else {
restore:
        func_800149A8();
        return;
    }
    setCurrentTaskCallback(func_8001DB7C);
}

INCLUDE_ASM("asm/nonmatchings/graphics/background_resources", func_8002F5F8);

void beginMode2TaskSequence(void) {
    configureTaskStateTransition(2, 0, 30);
    setCurrentTaskCallback(func_8002F5F8);
}

INCLUDE_ASM("asm/nonmatchings/graphics/background_resources", func_8002F65C);

INCLUDE_ASM("asm/nonmatchings/graphics/background_resources", func_8002F698);

void beginMode2SequenceAndFadeMusic(void) {
    configureTaskStateTransition(2, 0, 30);
    if (D_8013B106 == 1) {
        fadeOutMusicSequence(0x200);
        D_8013B106 = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/graphics/background_resources", func_8002FA58);

void queueMode11TransitionResources(void) {
    closeMusicSequence();
    func_8001816C();
    queueCdSectorCommand(0, 0x7CD, 0x34, (u8 *)0x80180000, 0);
    queueCdSectorCommand(3, 0x801, 3, (u8 *)0x8015E800, 0);
    queueCdSectorCommand(0, 0x804, 0x5D, (u8 *)0x80180000, 0);
    queueCdSectorCommand(4, 0x861, 3, (u8 *)0x80160000, 0);
    queueCdSectorCommand(0, 0x961, 0x68, (u8 *)0x80180000, 0);
    queueCdSectorCommand(5, 0x9C9, 4, (u8 *)0x80161800, 0);
    queueCdSectorCommand(9, 0x9CD, 4, (u8 *)0x80163800, 0);
    queueCdSectorCommand(0, 1, 5, (u8 *)0x8017D000, 0);
    queueCdSectorCommand(0, 6, 18, (u8 *)0x801F0000, 0);
    SetFogNearFar(5000, 6000, 400);
}
