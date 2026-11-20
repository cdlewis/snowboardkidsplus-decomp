#include "panel_tasks_internal.h"
#include "game/menu/selected_sprite.h"

typedef struct MainTailSelectionResource {
    s16 unk00;
    s16 selectedIndex;
} MainTailSelectionResource;

u8 gMainTailSelectionMapping[4] __attribute__((section(".data.main_tail_selection_mapping"), aligned(4))) = {
    0, 1, 2, 3,
};

/* Keep the array incomplete at the call site for absolute address scheduling. */
extern char gMainTailMoneyCurrencyLabel[] __attribute__((section(".data.main_tail_money_currency_label")));

const u8 gTimeRecordsStartPrompt[24] __attribute__((section(".rodata.time_records_start_prompt"), aligned(4))) = {
    0x83, 0x51, 0x81, 0x7C, 0x83, 0x80, 0x82, 0xF0,
    0x82, 0xCD, 0x82, 0xB6, 0x82, 0xDF, 0x82, 0xDC,
    0x82, 0xB7, 0x82, 0xA9, 0x81, 0x48, 0x00, 0x00,
};

const u8 gLapRecordsStartPrompt[24] __attribute__((section(".rodata.lap_records_start_prompt"), aligned(4))) = {
    0x83, 0x51, 0x81, 0x7C, 0x83, 0x80, 0x82, 0xF0,
    0x82, 0xCD, 0x82, 0xB6, 0x82, 0xDF, 0x82, 0xDC,
    0x82, 0xB7, 0x82, 0xA9, 0x81, 0x48, 0x00, 0x00,
};

void initializeMainTailDualResourceTask(void) {
    GameTask *task = D_8013AE88;
    GameTask *parent = D_8013AE98;
    task->data18 = 0;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009BDB8);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009BDB8);

void drawMainTailSelectionHeaderAndMoneyPanel(void) {
    Movement2D *position;
    MainTailSelectionResource *selection;
    u8 playerCount;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MainTailSelectionResource *)&D_800D9504[D_8013AE98->resource14];
    if (D_8013AE88->data1A == 0) {
        return;
    }
    func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                 0xB2, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    playerCount = D_8013AFCD;
    if (playerCount != 1) {
        return;
    }
    if (gMainTailSelectionMapping[selection->selectedIndex] == 3) {
        return;
    }
    func_8006C998(position->x / 4096 + 74, position->y / 4096 + 140,
                 0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                         6, 0, D_80130BF0[0], 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                       2, gMainTailMoneyCurrencyLabel, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    if (D_8013AE88->data1A == playerCount) {
        func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        func_8007240C(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

char gMainTailMoneyCurrencyLabel[3] __attribute__((section(".data.main_tail_money_currency_label"), aligned(4))) = "\x82" "f";
/* This byte immediately follows the three-byte currency label in the image. */
u8 D_8013B527 __attribute__((section(".data.main_tail_money_currency_label"))) = 0;

void initializeSelectionIndexedDualResourceTask(void) {
    GameTask *task = D_8013AE88;
    task->data18 = 0;
    if (D_8013B527 != 0) {
        task->data1A = D_8013B527 - 1;
    } else {
        task->data1A = D_8013B527;
    }
    D_8013AE98->data1C++;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009CA90);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009CA90);

void initializeModeDependentTaskResourcePair(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AFCD == 1) {
        D_8013AE88->data1A = 1;
    } else {
        D_8013AE88->data1A = 3;
    }
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009D354);
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009D354);

void drawRegisteredTaskGridSpriteWithNumber(void) {
    Movement2D *position;
    ByteTriplet *color;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    u16 mappedIndex;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    color = (ByteTriplet *)&D_800D9504[D_8013AE88->resource16];
    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = (GridSpriteParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) {
            for (;;) {}
        }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            func_800149A8();
        } else {
            sprite = func_800736D0(parent->sprites, parent->mapping, (s16)D_8013AE88->data1A);
            setSpriteColor(color->bytes[0], color->bytes[1], color->bytes[2]);
            func_8006C998((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            mappedIndex = func_8007373C(parent->mapping, (s16)D_8013AE88->data1A);
            setSpriteColor(128, 128, 128);
            if (mappedIndex < 10) {
                drawMenuDecimalNumber8x16(position->x / 4096 - 17, position->y / 4096 + 1,
                    2, 0, D_8013AE88->data1A + 1, 3, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            }
            entry->flags = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009DEF0);

void drawCurrentTaskTimeRecordsPrompt(void) {
    Movement2D *position;
    s16 x, y;
    s16 row;
    s16 palette;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    setSpriteColor(128, 128, 128);
    x = position->x / 4096;
    y = position->y / 4096 + 56;
    for (row = 0; row < 5; row++) {
        if (row < 3) {
            palette = 1;
        } else {
            palette = 4;
        }
        func_8006C998(x + 18, y, row + 0x136, palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&((SaveRecord *)&D_8013080C)->timeRecords[D_8013AFCE][row],
                     x + 38, y, palette, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 106, y,
                     (((SaveRecord *)&D_8013080C)->timeRecordCharacters[D_8013AFCE][row] & 15) * 3 + 0x14A,
                     D_80124284[((SaveRecord *)&D_8013080C)->timeRecordCharacters[D_8013AFCE][row] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 124, y,
                     (((SaveRecord *)&D_8013080C)->timeRecordCharacters[D_8013AFCE][row] >> 4) + 0x13B,
                     7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 20;
    }
    drawWrappedDoubleByteText(position->x / 4096 + 28, position->y / 4096 + 164, 14,
                              (s8 *)gTimeRecordsStartPrompt, 7, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    setTexturedQuadColor(128, 128, 128);
    drawDefaultTexturedPanel(position->x / 4096 + 24, position->y / 4096 + 160, 120, 24,
                             5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009E848);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009ECCC);

void initializeFlagSelectedDualResourceTask(void) {
    GameTask *task;
    GameTask *parent;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = func_8007D47C();
    task = D_8013AE88;
    parent = D_8013AE98;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009F224);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009F224);

void initializeModeDependentTailResourcePairTask(void) {
    GameTask *task = D_8013AE88;
    task->data18 = 0;
    if (D_8013AFCD == 1) {
        task->data1A = 1;
    } else {
        task->data1A = 3;
    }
    if (D_8013AE88->resource14 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        while (1) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    setCurrentTaskCallback(func_8009F9B8);
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009F9B8);

void drawCurrentTaskLapRecordsPrompt(void) {
    Movement2D *position;
    s16 x, y;
    s16 row;
    s16 palette;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    setSpriteColor(128, 128, 128);
    x = position->x / 4096;
    y = position->y / 4096 + 56;
    for (row = 0; row < 5; row++) {
        if (row < 3) {
            palette = 1;
        } else {
            palette = 4;
        }
        func_8006C998(x + 18, y, row + 0x136, palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawRaceTime(&((SaveRecord *)&D_8013080C)->lapRecords[D_8013AFCE][row],
                     x + 38, y, palette, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 106, y,
                     (((SaveRecord *)&D_8013080C)->lapRecordIcons[D_8013AFCE][row] & 15) * 3 + 0x14A,
                     D_80124284[((SaveRecord *)&D_8013080C)->lapRecordIcons[D_8013AFCE][row] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 124, y,
                     (((SaveRecord *)&D_8013080C)->lapRecordIcons[D_8013AFCE][row] >> 4) + 0x13B,
                     7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 20;
    }
    drawWrappedDoubleByteText(position->x / 4096 + 28, position->y / 4096 + 164, 14,
                              (s8 *)gLapRecordsStartPrompt, 7, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    setTexturedQuadColor(128, 128, 128);
    drawDefaultTexturedPanel(position->x / 4096 + 24, position->y / 4096 + 160, 120, 24,
                             5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

s16 findIdentityMappedIndexOrZero(ByteMappingDescriptor *descriptor, s16 value) {
    s16 result = 0;
    s16 i;

    for (i = 0; i < descriptor->count; i++) {
        if (value == (s16)func_8007373C(descriptor->mapping, value)) {
            result = i;
            break;
        }
    }
    return result;
}

void initializeTaskResourcePairWithOneTickTransition(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    configureTaskStateTransition(2, 0, 1);
    setCurrentTaskCallback(func_800A05C0);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_800A05C0);

void selectTaskCallbackFromModeState(void) {
    if (D_8013AFD8 != 0) {
        setCurrentTaskCallback(initializeMode1RacePlayers);
    } else if (D_8013AFD9 != 0) {
        setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
    }
}

void initializeMode1Task(void) {
    D_8013AFD8 = 1;
    D_8013AFCE = -1;
    D_8013B560 = 99;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013B0F0 = 0;
    setCurrentTaskCallback(initializeMode1RacePlayers);
}

void initializeMode1RacePlayers(void) {
    s32 i;
    s16 mode;
    D_8013B560 = D_8013AFCE;
    if ((s16)D_8013AFD8 < 7 && D_8013AFD8 != 0) D_8013AFCE = 0;
    else D_8013AFCE = 9;
    SetFogNearFar(8000, 10000, 400);
    D_8013AFD3 = 0;
    D_8013AFD6 = 0;
    D_8013AFE1 = 0;
    D_8013AFD9 = 0;
    D_8013AFE2 = 0;
    D_8013AFE0 = 0;
    gRacePlayers[0].unk008 = 0;
    gRacePlayers[1].unk008 = 1;
    gRacePlayers[2].unk008 = 1;
    gRacePlayers[3].unk008 = 1;
    gRacePlayers[0].characterId = 0;
    gRacePlayers[1].characterId = 1;
    gRacePlayers[2].characterId = 2;
    gRacePlayers[3].characterId = 3;
    gRacePlayers[0].unk015[1] = 0;
    gRacePlayers[1].unk015[1] = 0;
    gRacePlayers[2].unk015[1] = 0;
    gRacePlayers[3].unk015[1] = 0;
    gRacePlayers[0].unk015[2] = 3;
    gRacePlayers[1].unk015[2] = 5;
    gRacePlayers[2].unk015[2] = 6;
    gRacePlayers[3].unk015[2] = 7;
    gRacePlayers[0].unk025[3] = 0;
    gRacePlayers[1].unk025[3] = 1;
    gRacePlayers[2].unk025[3] = 2;
    gRacePlayers[3].unk025[3] = 3;
    gameOptionPlayerModeAbsolute = 2;
    mode = D_8013AFD8;
    if (mode != 0 && (mode < 7 || mode == 9)) {
        gRacePlayers[0].isActive = 1;
        gRacePlayers[1].isActive = 0;
        gRacePlayers[2].isActive = 0;
        gRacePlayers[3].isActive = 0;
        D_8013AFD2 = 1;
    } else {
        gRacePlayers[0].isActive = 1;
        gRacePlayers[1].isActive = 1;
        gRacePlayers[2].isActive = 1;
        gRacePlayers[3].isActive = 1;
        D_8013AFD2 = 4;
    }
    D_8013AFD0 = 5;
    D_8013AFDE = 100;
    for (i = 0; i < 4; i++) {
        gRacePlayers[i].unk025[1] = 0;
        gRacePlayers[i].unk025[2] = 0;
        gRacePlayers[i].unk01C = 0;
    }
    setCurrentTaskCallback(prepareRaceResourcesIfSelectionChanged);
}

void prepareRaceResourcesIfSelectionChanged(void) {
    configureTaskStateTransition(1, 0, 30);
    setSecondaryDisplayTaskMode(1);
    if (D_8013B560 != D_8013AFCE) {
        func_800180F4(2);
        func_800180F4(3);
        func_800180F4(4);
        func_800180F4(5);
        queueStartupSectorReads();
        setCurrentTaskCallback(preparePlayerResourcesTask);
        D_8013AE88->unk10 |= 2;
    } else {
        setCurrentTaskCallback(initializeRaceTaskState);
    }
}

void preparePlayerResourcesTask(void) {
    queuePlayerResourceReadsAndUploadTextures();
    setCurrentTaskCallback(initializeRaceTaskState);
    D_8013AE88->unk10 |= 2;
}

void initializeModeSectorReadResourcesTask(void) {
    ModeSectorReadState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (ModeSectorReadState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    D_8013B55E = 0;
    if (D_8013AFD8 != 0) {
        state->mode = 1;
    } else {
        state->mode = 2;
    }
    stopPlayerSound(0);
    stopPlayerSound(1);
    setCurrentTaskCallback(queueTaskModeSectorRead);
}

void queueTaskModeSectorRead(void) {
    ModeSectorReadState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (ModeSectorReadState *)&D_800D9504[D_8013AE88->resource14];
    if (state->mode == 1) {
        state->sectorIndex = D_8013AFD8 - 1;
    } else {
        state->sectorIndex = D_8013AFD9 - 1;
    }
    queueModeIndexedSectorRead(state->mode, state->sectorIndex);
    setCurrentTaskCallback(func_800A0EE8);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_800A0EE8);
