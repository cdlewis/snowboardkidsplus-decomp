#include "../menu/panel_tasks_internal.h"

ResourceSectorRange gMode1SectorRanges[10] __attribute__((section(".data.indexed_sector_ranges"))) = {
    {DAT_MODE1_TEXT_000_SECTOR, DAT_MODE1_TEXT_000_SECTOR_COUNT},
    {DAT_MODE1_TEXT_001_SECTOR, DAT_MODE1_TEXT_001_SECTOR_COUNT},
    {DAT_MODE1_TEXT_002_SECTOR, DAT_MODE1_TEXT_002_SECTOR_COUNT},
    {DAT_MODE1_TEXT_003_SECTOR, DAT_MODE1_TEXT_003_SECTOR_COUNT},
    {DAT_MODE1_TEXT_004_SECTOR, DAT_MODE1_TEXT_004_SECTOR_COUNT},
    {DAT_MODE1_TEXT_005_SECTOR, DAT_MODE1_TEXT_005_SECTOR_COUNT},
    {DAT_MODE1_TEXT_006_SECTOR, DAT_MODE1_TEXT_006_SECTOR_COUNT},
    {DAT_MODE1_TEXT_007_SECTOR, DAT_MODE1_TEXT_007_SECTOR_COUNT},
    {DAT_MODE1_TEXT_008_SECTOR, DAT_MODE1_TEXT_008_SECTOR_COUNT},
    {DAT_MODE1_TEXT_009_SECTOR, DAT_MODE1_TEXT_009_SECTOR_COUNT}
};

ResourceSectorRange gOtherModeSectorRanges[11] __attribute__((section(".data.indexed_sector_ranges"))) = {
    {DAT_OTHER_MODE_TEXT_000_SECTOR, DAT_OTHER_MODE_TEXT_000_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_001_SECTOR, DAT_OTHER_MODE_TEXT_001_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_002_SECTOR, DAT_OTHER_MODE_TEXT_002_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_003_SECTOR, DAT_OTHER_MODE_TEXT_003_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_004_SECTOR, DAT_OTHER_MODE_TEXT_004_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_005_SECTOR, DAT_OTHER_MODE_TEXT_005_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_006_SECTOR, DAT_OTHER_MODE_TEXT_006_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_007_SECTOR, DAT_OTHER_MODE_TEXT_007_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_008_SECTOR, DAT_OTHER_MODE_TEXT_008_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_009_SECTOR, DAT_OTHER_MODE_TEXT_009_SECTOR_COUNT},
    {DAT_OTHER_MODE_TEXT_010_SECTOR, DAT_OTHER_MODE_TEXT_010_SECTOR_COUNT}
};

void initializeTextSequenceTask(void) {
    TextSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    resource->state = 2;
    resource->entryIndex = 0;
    resource->progress = 0;
    resource->duration = 0;
    resource->intensity = 0x80;
    resource->unk3A = 0;
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(1, 0, 30);
    if (resource->mode != 1 || resource->unk0A != 9) {
        queueCdCommand(3, 0);
    }
    setCurrentTaskCallback(func_800A0FDC);
}

INCLUDE_ASM("asm/nonmatchings/text/text_sequence", func_800A0FDC);

void animateTextSequenceExitTask(void) {
    TextSequenceResource *resource;
    u16 previous;
    s16 delta;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    resource->velocity += 0x1000;
    resource->y -= resource->velocity;
    resource->displacement += resource->velocity;
    if (resource->intensity != 0) resource->intensity -= 4;
    previous = D_800EC158[1].h;
    D_800EC158[1].h = resource->displacement / 4096 - (u32)(-D_8013AF7C / 2 + 4);
    if (D_800EC158[1].h > D_8013AF7C) D_800EC158[1].h = D_8013AF7C;
    delta = D_800EC158[1].h - previous;
    resource->unk3A += delta;
    D_8013AF80 -= delta / 2;
    if (D_8013AF80 < 0) D_8013AF80 = 0;
    D_800EC158[2].y += delta;
    D_800EC158[2].h -= delta;
    if (D_800EC158[2].h < 0) {
        D_800EC158[2].y = 240;
        D_800EC158[2].h = 0;
    }
    D_801241E8[0].unk10 += delta * 2;
    if (D_801241E8[0].unk10 > -320) D_801241E8[0].unk10 = -320;
    if (resource->displacement / 4096 >= 141) {
        taskTransitionFlagGp = 1;
        D_8013B1A8 = 1;
        D_8013AFCD = 1;
        D_8013AFE4 = 0;
        D_8013AFAA |= 0x1000;
        queueCdCommand(D_80124164[D_8013AFCE], 0);
        func_800149A8();
    } else {
        func_800A1774(resource);
    }
}

void advanceModeAfterTextSequenceFadeTask(void) {
    TextSequenceResource *resource;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    resource = (TextSequenceResource *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 1:
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        resource->intensity = 0;
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (func_800707F0(0) == 0) {
            D_8013B55E = 2;
            D_8013AFD8++;
            func_800149A8();
        }
        break;
    }
    func_800A1774(resource);
}

void queueModeIndexedSectorRead(s16 mode, s16 index) {
    if (mode == 1) {
        queueCdSectorCommand(0, gMode1SectorRanges[index].sector,
                      gMode1SectorRanges[index].sectorCount,
                      INDEXED_SECTOR_READ_DESTINATION, 0);
    } else {
        queueCdSectorCommand(0, gOtherModeSectorRanges[index].sector,
                      gOtherModeSectorRanges[index].sectorCount,
                      INDEXED_SECTOR_READ_DESTINATION, 0);
    }
}

INCLUDE_ASM("asm/nonmatchings/text/text_sequence", func_800A1670);

INCLUDE_ASM("asm/nonmatchings/text/text_sequence", func_800A1774);
