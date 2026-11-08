#include "panel_tasks_internal.h"
#include "game/menu/selected_sprite.h"

const u16 gTaskPanel144x48Text[8] __attribute__((section(".data.task_panel_144x48_text"))) = {
    0xF4, 0xC1, 0x96, 0xB1, 0x9F, 0x98, 0x31, 0
};

const u16 gTaskPlayerValuePanelLabel[6] __attribute__((section(".data.task_player_value_panel_label"))) = {
    0x18, 0x157, 0x2D, 0x8020, 0xA8, 0
};

const u16 gTaskPlayerValuePanelText[8] __attribute__((section(".data.task_player_value_panel_text"))) = {
    0xF4, 0xC1, 0x96, 0xB1, 0x9E, 0xA2, 0x33, 0
};

SplitSelectionBoundsConfigs gSplitSelectionBoundsConfigs __attribute__((section(".data.split_selection_bounds_configs"))) = {{
    {60, 120, 4}, {30, 180, 6}, {30, 180, 6}, {60, 120, 4},
    {30, 180, 6}, {30, 180, 6}, {60, 120, 4}, {30, 180, 6},
    {60, 120, 4}, {60, 120, 4}
}, {0, 0}};

void initializeCountedSplitSelectionBoundsTask(void) {
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    setSecondaryDisplayTaskMode(2);
    {
        RECT *bounds = &D_801300B8[0];
        bounds->x = 160;
        bounds->y = 0;
        bounds->w = 160;
        bounds->h = 240;
    }
    {
        RECT *bounds = &D_801300B8[1];
        bounds->x = 160;
        bounds->y = 0;
        bounds->w = 160;
        bounds->h = 0;
    }
    {
        RECT *bounds = &D_801300B8[2];
        bounds->x = 24;
        bounds->y = 132;
        bounds->w = 120;
        bounds->h = 84;
    }
    {
        RECT *bounds = &D_801300B8[3];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    setCurrentTaskCallback(func_80097FF4);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/grid_selection", func_80097FF4);

void updateSplitSelectionBoundsTransition(void) {
    SplitBoundsResource *movement;
    u8 index;
    s16 height;
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    movement = (SplitBoundsResource *)&D_800D9504[D_8013AE88->resource16];
    index = gRacePlayers[(s16)D_8013AE88->data1A].unk009;
    height = D_801300B8[0].h;
    switch (D_8013AE88->data18) {
    case 19:
        {
            RECT *bounds = &D_801300B8[1];
            bounds->x = 160;
            bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
            bounds->w = 160;
            bounds->h = gSplitSelectionBoundsConfigs.entries[index][1];
        }
        {
            RECT *bounds = &D_801300B8[0];
            bounds->x = 160;
            bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
            bounds->w = 160;
            bounds->h = 0;
        }
        movement->unk1C = 13;
        playSoundEffect(1, 0x102, 32);
        D_8013AE88->data18 = 20;
        break;
    case 20:
        height += gSplitSelectionBoundsConfigs.entries[index][2];
        if (height <= gSplitSelectionBoundsConfigs.entries[index][1]) {
            {
                RECT *bounds = &D_801300B8[1];
                bounds->x = 160;
                bounds->y = gSplitSelectionBoundsConfigs.entries[index][0] + height;
                bounds->w = 160;
                bounds->h = gSplitSelectionBoundsConfigs.entries[index][1] - height;
            }
            {
                RECT *bounds = &D_801300B8[0];
                bounds->x = 160;
                bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
                bounds->w = 160;
                bounds->h = height;
            }
        } else {
            movement->unk24 = 13;
            D_8013AE88->data18 = 21;
        }
        break;
    case 21:
        D_8013AE88->data18 = 16;
        setCurrentTaskCallback(func_80097FF4);
        break;
    }
}

void drawCurrentTaskSpriteB7(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->data1A != 0) {
        func_8006C998(position->x / 4096 + 8, position->y / 4096 - 22,
                      0xB7, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

void drawCurrentTaskFramedSprite20B(void) {
    Movement2D *position;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    setTexturedQuadColor(128, 128, 128);
    setSpriteColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 140, position->y / 4096 + 8,
                           128, 92, 5, 1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006C998(position->x / 4096 - 136, position->y / 4096 + 12,
                  0x20B, 1, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void drawCurrentTaskTextPanel144x48(void) {
    Movement2D *position;
    s32 x, y;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30((s16)(x + 16), (s16)(y + 12), 10, gTaskPanel144x48Text, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

void drawCurrentTaskPlayerValuePanel(void) {
    Movement2D *position;
    s32 x, y, value;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    value = gRacePlayers[(s16)D_8013AE98->data1A].unk00A[0];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006EF30((s16)(x + 8), (s16)(y + 4), 10, gTaskPlayerValuePanelLabel, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006E2C4((s16)(x + 56), (s16)(y + 4), 1, 0, value + 1, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    func_8006EF30((s16)(x + 8), (s16)(y + 20), 10, gTaskPlayerValuePanelText, 8, -1,
                 &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    drawDefaultTexturedPanel((s16)x, (s16)y, 144, 48, 5, 0,
                            &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

INCLUDE_ASM("asm/nonmatchings/menu/grid_selection", func_800995A8);

INCLUDE_ASM("asm/nonmatchings/menu/grid_selection", func_800996F8);

void initializeCountedFourBoundsTask(void) {
    D_8013AE98->data1C++;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    if (D_8013AE88->resource14 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) { for (;;) {} }
    D_8013AE88->resource16 = allocateTaskResource();
    setSecondaryDisplayTaskMode(2);
    {
        RECT *bounds = &D_801300B8[0];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    {
        RECT *bounds = &D_801300B8[1];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    {
        RECT *bounds = &D_801300B8[2];
        bounds->x = 24;
        bounds->y = 132;
        bounds->w = 120;
        bounds->h = 84;
    }
    {
        RECT *bounds = &D_801300B8[3];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    D_8013B517 = 1;
    setCurrentTaskCallback(func_80099880);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/grid_selection", func_80099880);

void drawCurrentTaskSavedValuePanel(void) {
    Movement2D *position;
    s32 x, y;
    u16 index;
    SaveRecord *records;
    GsOT *ot;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    index = D_8013AE98->data1A;
    if (D_8013AE88->data1A != 0) {
        func_8006C998(x + 8, y - 22, 0xBB, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(x + 74, y + 140, 0xBD, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        records = (SaveRecord *)&D_8013080C;
        func_8006D66C(x + 74, y + 152, 6, 0,
            ((GlobalFlagStatePrefix *)&records[(s16)index])->unk_3E4,
            0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        ot = &D_800EBFF0[D_8013AF78];
        drawWrappedText8x8(x + 122, y + 152, 2, "\x82" "f", 0, -1,
            ot, D_8013AE88->data1C);
        if (D_8013AE88->data1A == 1) {
            func_8007240C(x + 68, y + 136, 68, 32, D_8013B3A4, D_8013B3A5, D_8013B3A6,
                2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        } else {
            func_8007240C(x + 68, y + 136, 68, 32, 128, 128, 128,
                2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        }
    }
}

u8 D_8013B517 __attribute__((section(".sdata"))) = 0;

INCLUDE_ASM("asm/nonmatchings/menu/grid_selection", func_8009A87C);

void initializeTaskGridPositions(s32 unused, GameTask *task, s32 x, s32 y) {
    s32 i;
    TaskGridState *state;
    TaskGridEntry *entry;
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entry = (TaskGridEntry *)&D_800D9504[task->resource16];
    for (i = 0; i < state->count; i++) {
        if (i < 8) {
            initMovement2D((Movement2D *)&D_800D9504[entry[i].task->resource14],
                           x + (i % 4) * 0x20000, y + (i / 4) * 0x28000);
        } else {
            initMovement2D((Movement2D *)&D_800D9504[entry[i].task->resource14],
                           x + 0xC000, y + 0x50000);
        }
    }
}

void configureFourColumnTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    TaskGridState *state;
    TaskGridEntry *entries;
    s16 i, end;
    Movement2D *movement;
    state = (TaskGridState *)&D_800D9504[task->resource14];
    entries = (TaskGridEntry *)&D_800D9504[task->resource16];
    if (mode == 1) {
        i = state->selectedIndex;
        end = i + 1;
    } else {
        i = 0;
        end = state->count;
    }
    for (; i < end; i++) {
        if (mode == 2 && i == state->selectedIndex) continue;
        if (i < 8) {
            movement = (Movement2D *)&D_800D9504[entries[i].task->resource14];
            if (delay == -1000) {
                configureMovement2D(movement, x + (i % 4) * 0x20000,
                                    y + (i / 4) * 0x28000, stepX, stepY, delay);
            } else {
                configureMovement2D(movement, x + (i % 4) * 0x20000,
                                    y + (i / 4) * 0x28000, stepX, stepY, delay * (i / 4));
            }
        } else {
            movement = (Movement2D *)&D_800D9504[entries[i].task->resource14];
            if (delay == -1000) {
                configureMovement2D(movement, x + 0xC000, y + 0x50000, stepX, stepY, delay);
            } else {
                configureMovement2D(movement, x + 0xC000, y + 0x50000, stepX, stepY, delay * 2);
            }
        }
    }
}

void drawTaskGridSelectionMarker(void) {
    GridMarkerState *state;
    TaskGridEntry *entries;
    Movement2D *position;
    s32 mode;
    u32 colorMode;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (GridMarkerState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (TaskGridEntry *)&D_800D9504[D_8013AE88->resource16];
    mode = (s16)state->state;
    colorMode = state->state;
    if (mode > 0) {
        if ((u32)(colorMode - 1) < 2) {
            setSpriteColor(state->red, state->green, state->blue);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&D_800D9504[entries[state->selectedIndex].task->resource14];
        if (state->selectedIndex < 8) {
            func_8006C998(position->x / 4096, position->y / 4096, 0x16F, 8, -1,
                         &D_800EBFF0[D_8013AF78], (u16)(state->depth - 1));
        } else {
            func_8006C998(position->x / 4096, position->y / 4096, 0xB0, 0, -1,
                         &D_800EBFF0[D_8013AF78], (u16)(state->depth - 1));
        }
    }
}

void drawRegisteredTaskGridSprite(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&D_800D9504[D_8013AE98->resource14];
    if ((u16)(D_8013AE88->data1E % 10U) == 0) {
        if (D_8013AE98->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource14];
        entry = &entries[D_8013AE88->data1E / 10U];
    } else {
        if (D_8013AE98->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
        entry = &entries[D_8013AE88->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            func_800149A8();
        } else {
            if (parent->mode == 1 && parent->selectedIndex != D_8013AE88->data1A) {
                sprite = func_800736D0(parent->sprites, parent->mapping,
                    (s16)(D_8013AE88->data1A + parent->count));
            } else {
                sprite = func_800736D0(parent->sprites, parent->mapping, (s16)D_8013AE88->data1A);
            }
            setSpriteColor(128, 128, 128);
            func_8006C998((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, 0, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
            entry->flags = 0;
        }
    }
}
