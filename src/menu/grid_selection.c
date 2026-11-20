#include "panel_tasks_internal.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"

#define CHILD_MOVEMENT(entry, resource) ((Movement2D *)&D_800D9504[(entry).task->resource])
#define FIRST_FOUR_READY(entries) \
    ((s16)(entries)[0].flags == 0 && (s16)(entries)[1].flags == 0 && \
     (s16)(entries)[2].flags == 0 && (s16)(entries)[3].flags == 0)

#define SPLIT_CHILDREN_READY(entries) \
    ((s16)(entries)[0].flags == 0 && (s16)(entries)[1].flags == 0 && (s16)(entries)[3].flags == 0)

typedef struct PaletteSelectionMapping {
    u8 entries[10][9];
    u8 padding[2];
} PaletteSelectionMapping;

const s8 gPaletteSelectionMainPreviewY[12]
    __attribute__((section(".data.palette_selection_main_preview_y"), aligned(4))) = {
    0, 0, 0, 0, 8, 0, 0, 6, 0, 0, 0, 0
};

const s8 gPaletteSelectionSidePreviewY[12]
    __attribute__((section(".data.palette_selection_side_preview_y"), aligned(4))) = {
    8, 0, 0, 0, 24, 0, 4, 20, -4, -4, 0, 0
};

const u8 gPaletteSelectionSidePreviewRotation[12]
    __attribute__((section(".data.palette_selection_side_preview_rotation"), aligned(4))) = {
    0, 200, 200, 200, 200, 200, 150, 200, 200, 200, 0, 0
};

const s8 gPaletteSelectionPurchasePreviewY[12]
    __attribute__((section(".data.palette_selection_purchase_preview_y"), aligned(4))) = {
    0, 20, 15, 15, 0, 10, 10, 0, 25, 20, 0, 0
};

const GridSpriteEntry gPaletteSelectionSprites[14]
    __attribute__((section(".data.palette_selection_sprites"), aligned(4))) = {
    {0x170, 0, {0, 0, 0}},
    {0x170, 1, {0, 0, 0}},
    {0x170, 2, {0, 0, 0}},
    {0x170, 3, {0, 0, 0}},
    {0x170, 4, {0, 0, 0}},
    {0x170, 5, {0, 0, 0}},
    {0x170, 6, {0, 0, 0}},
    {0x170, 7, {0, 0, 0}},
    {0x170, 9, {0, 0, 0}},
    {0x170, 10, {0, 0, 0}},
    {0x170, 11, {0, 0, 0}},
    {0x170, 12, {0, 0, 0}},
    {0x170, 13, {0, 0, 0}},
    {0xC7, 0, {0, 0, 0}},
};

const PaletteSelectionMapping gPaletteSelectionSpriteMapping
    __attribute__((section(".data.palette_selection_sprite_mapping"), aligned(4))) = {{
    {8, 9, 3, 1, 0, 6, 2, 12, 13},
    {3, 10, 1, 8, 0, 4, 2, 6, 13},
    {9, 8, 11, 2, 1, 0, 3, 6, 13},
    {9, 3, 0, 10, 4, 1, 6, 8, 13},
    {9, 6, 2, 3, 4, 5, 8, 1, 13},
    {3, 7, 5, 1, 6, 9, 11, 8, 13},
    {4, 10, 7, 0, 8, 9, 5, 11, 13},
    {3, 6, 5, 2, 1, 4, 10, 8, 13},
    {9, 0, 2, 3, 6, 11, 8, 7, 13},
    {2, 10, 6, 0, 1, 9, 8, 4, 13},
}, {0, 0}};

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

extern u8 D_8013B4BD __attribute__((section(".data")));

s16 func_80073760(GameTask *task, s16 index);
void func_800995A8(s16 player, s16 character, u8 spacingX, u8 spacingY, ByteMappingDescriptor *descriptor);
void func_8009A87C(void);

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
    setCurrentTaskCallback(updateSplitBoardSelectionTask);
    D_8013AE88->unk10 |= 2;
}

void updateSplitBoardSelectionTask(void) {
    FlagSelectedMenuState *input;
    RegisteredTaskEntry *entries;
    TaskPanelSelectionValue *selection;
    ByteMappingDescriptor descriptor;
    ByteMappingDescriptor *mapping;
    s16 panelValues[6];
    u16 player;
    s16 playerIndex;
    u8 character;
    u8 palette;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    playerIndex = (s16)D_8013AE88->data1A;
    player = D_8013AE88->data1A;
    character = gRacePlayers[playerIndex].unk009;
    palette = gRacePlayers[playerIndex].unk00D;
    selection = &D_8013B508[playerIndex];

    switch (D_8013AE88->data18) {
    case 0: {
        s16 previewCharacter;
        s16 previewPalette;
        u8 model;
        s16 gridMode = 1;
        s32 gridIndex = 1;
        RacePlayer *previewPlayer;
        RacePlayer *players;
        const s8 *mainPreviewY;

        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        mapping = &descriptor;
        func_800995A8((s16)player, character, 0, 20, mapping);
        input->selectedIndex = 0;
        input->mode = 0;
        input->previousIndex = 0;
        input->choiceResult = 0;
        model = mapping->mapping[input->selectedIndex];
        if (model < 5U) {
            selection->value = model;
        } else {
            selection->value = gRacePlayers[(s16)player].unk00A[0];
        }
        selection->unk02 = selection->value;
        createRegisteredSpacedChildTasks(gridMode, 0xB0, 0x40, (const u8 *)D_80131A78, mapping->mapping,
                                         NULL, 0, 0, 20, -304, -96, mapping->spacingX,
                                         mapping->spacingY, mapping->count, 0xB0,
                                         drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, gridIndex);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSpriteB7, 4, 11);
        previewCharacter = character;
        previewPalette = palette;
        players = gRacePlayers;
        previewPlayer = &players[(s16)player];
        mainPreviewY = &gPaletteSelectionMainPreviewY[previewCharacter];
        createGridModelPreviewTask(0, previewCharacter, previewPlayer->unk00A[0], previewPalette,
                                   244, *mainPreviewY + 50, 384, 31);
        createRegisteredCallbackTask(0, -180, 0, drawCurrentTaskFramedSprite20B, 4, 21);
        createGridModelPreviewTask(2, previewCharacter, selection->value, previewPalette,
                                   -260, gPaletteSelectionSidePreviewY[previewCharacter] + 118, 448, 51);
        createGridModelPreviewTask(1, previewCharacter, previewPlayer->unk00A[0], previewPalette,
                                   64, *mainPreviewY + 50, 384, 41);
        D_8013AE88->data18 = 2;
        break;
    }
    case 2:
        if (SPLIT_CHILDREN_READY(entries) && (s16)entries[2].flags == 0 &&
            (s16)entries[5].flags == 0 && (s16)entries[4].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x7C000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D(CHILD_MOVEMENT(entries[1], resource14), -0x8000, -0x48000, 0x1E000, 0, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[3], resource16), 0x40000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[2], resource14), 0, 0, 0x1E000, 0x1E000, 6);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (SPLIT_CHILDREN_READY(entries) && (s16)entries[2].flags == 0) {
            configureMovement2D(CHILD_MOVEMENT(entries[5], resource16), -0x50000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[5].flags == 0) {
            GameTask *task;
            entries[0].command = 20;
            entries[1].command = 20;
            entries[5].command = 1;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = D_8013AE88;
            D_8013B41F = 0;
            task->data18 = 5;
        }
        break;
    case 5: {
        s32 complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (input->selectedIndex != input->previousIndex) {
            s16 model;
            selection->unk02 = selection->value;
            model = func_80073760(entries[0].task, input->selectedIndex);
            if ((u16)model < 5U) {
                selection->value = model;
                if (model != selection->unk02) {
                    entries[5].command = 13;
                    D_8013AE88->data1C = 0;
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                D_8013B41F = 1;
            }
            if (input->choiceResult < 0) {
                D_8013AE98->data18 = 13;
                goto exitSelection;
            }
            if (func_80073760(entries[0].task, input->selectedIndex) == 5) {
                D_8013B4BD = 0;
                D_8013AE98->data18 = 11;
                playSoundEffect(0, 0x202, 32);
                D_8013AE88->data18 = 22;
            } else if (gRacePlayers[(s16)player].unk00A[0] == selection->value) {
                D_8013AE88->data18 = 24;
            } else {
                D_8013AE88->data18 = 13;
            }
        }
        break;
    }
    case 13:
        createRegisteredCallbackTask((s16)player, 0, -28, drawCurrentTaskTextPanel144x48, 2, 61);
        panelValues[0] = 0x125;
        panelValues[2] = 0x126;
        panelValues[4] = 0xB0;
        panelValues[1] = 0;
        panelValues[3] = 0;
        panelValues[5] = 0;
        createRegisteredMovingPanelTask(1, panelValues, 24, -13, 0, 16, 24, 19, 3, 71);
        D_8013AE88->data18 = 14;
        break;
    case 14:
        if ((s16)entries[7].flags >= 0) {
            entries[6].command = 99;
            if ((s16)entries[7].flags != 0) {
                D_8013AE88->data18 = 24;
            } else {
                D_8013AE88->data18 = 15;
            }
        }
        break;
    case 15: {
        RacePlayer *players = gRacePlayers;
        RacePlayer *currentPlayer = &players[(s16)player];
        u8 model;
        selection->unk02 = currentPlayer->unk00A[0];
        model = selection->value;
        currentPlayer->unk00A[0] = model;
        ((SaveRecord *)&D_8013080C)[(s16)player].unk_3EC[currentPlayer->unk009] = model;
        D_8013AE88->data18 = 19;
        setCurrentTaskCallback(updateSplitSelectionBoundsTransition);
        break;
    }
    case 16: {
        SpacedGroupResource *group;
        mapping = &descriptor;
        func_800995A8((s16)player, character, 0, 20, mapping);
        group = (SpacedGroupResource *)&D_800D9504[entries[0].task->resource14];
        group->data48 = descriptor.mapping;
        group->spacingX = mapping->spacingX;
        group->spacingY = mapping->spacingY;
        group->count = mapping->count;
        D_8013AE88->data18 = 17;
        break;
    }
    case 17: {
        s16 model = func_80073760(entries[0].task, input->selectedIndex);
        if ((u16)model < 5U) {
            selection->value = model;
            entries[5].command = 13;
        }
        createRegisteredCallbackTask((s16)player, 0, -28, drawCurrentTaskPlayerValuePanel, 2, 61);
        entries[8].task = createCountdownTask(15);
        D_8013AE88->data18 = 18;
        break;
    }
    case 18: {
        GameTask *countdown = entries[8].task;
        if (countdown->data1A == 1) {
            countdown->data18 = 1;
            entries[6].command = 99;
            D_8013AE88->data18 = 24;
        }
        break;
    }
    case 24: {
        GameTask *task;
        input->choiceResult = 0;
        entries[0].command = 21;
        configureColorCycleTask(2, 64, 128, 4, 0);
        task = D_8013AE88;
        D_8013B41F = 1;
        task->data18 = 5;
        break;
    }
    case 22: {
        GameTask *task;
        task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 23;
        break;
    }
    case 23:
        if ((s16)entries[0].flags == 0) {
exitSelection:
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        if (SPLIT_CHILDREN_READY(entries)) {
            GameTask *task;
            queueMenuResourceReads((s16)D_8013AE98->data18);
            configureMovement2D(CHILD_MOVEMENT(entries[5], resource16), -0x104000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, -1000);
            task = D_8013AE88;
            entries[2].command = 23;
            task->data18 = 9;
        }
        break;
    case 9:
        if ((s16)entries[5].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x130000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(entries[1], resource14), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[3], resource16), 0xF4000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[2], resource14), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data1C = 0;
            D_8013AE88->data18 = 10;
        }
        break;
    case 10:
        if (SPLIT_CHILDREN_READY(entries) && (s16)entries[2].flags == 0) {
            D_8013AE88->data18 = 11;
        }
        break;
    case 11:
        D_8013AE88->data18 = 12;
        break;
    default:
        D_8013AE98->data1C--;
        func_800149A8();
        break;
    }
}

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
        setCurrentTaskCallback(updateSplitBoardSelectionTask);
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
    drawDecimalNumber16((s16)(x + 56), (s16)(y + 4), 1, 0, value + 1, 8, -1,
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
    setCurrentTaskCallback(updatePlayerPaletteSelectionTask);
    D_8013AE88->unk10 |= 2;
}

void updatePlayerPaletteSelectionTask(void) {
    FlagSelectedMenuState *input;
    TaskPanelSelectionValue *selection;
    s32 complete;
    RegisteredTaskEntry *entries;
    s16 player;
    s32 character;
    s32 palette;

    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];
    player = D_8013AE88->data1A;
    character = gRacePlayers[player].unk009;
    palette = gRacePlayers[player].unk00D;
    selection = &D_8013B508[player];

    switch (D_8013AE88->data18) {
    case 0: {
        s16 previewCharacter;
        s16 previewPalette;
        setMusicSequenceEnabled(1);
        setSecondaryDisplayTaskMode(0);
        selection->value = gRacePlayers[player].unk00D;
        selection->unk02 = selection->value;
        input->mode = 0;
        input->selectedIndex = selection->value;
        input->previousIndex = input->selectedIndex;
        input->choiceResult = 0;
        previewCharacter = character;
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40,
                                         (const u8 *)gPaletteSelectionSprites, gPaletteSelectionSpriteMapping.entries[previewCharacter],
                                         NULL, 0, 0, 20, -316, -96, 5, 5, 9, -1,
                                         drawTaskGridSelectionMarker, drawRegisteredTaskGridSprite, 4, 1);
        createRegisteredCallbackTask(0, 172, -72, drawCurrentTaskSavedValuePanel, 4, 11);
        createRegisteredCallbackTask(0, -180, 0, func_8009A87C, 4, 21);
        previewPalette = palette;
        createGridModelPreviewTask(0, previewCharacter, 0, previewPalette, 244, gPaletteSelectionMainPreviewY[previewCharacter] + 50, 384, 31);
        createGridModelPreviewTask(2, previewCharacter, 0, previewPalette, -240, gPaletteSelectionSidePreviewY[previewCharacter] + 118, 352, 41);
        gRacePlayers[2].unk0B4 = gPaletteSelectionSidePreviewRotation[previewCharacter];
        D_8013AE88->data18 = 2;
        break;
    }
    case 2:
        if (FIRST_FOUR_READY(entries) && (s16)entries[4].flags == 0) {
            initializeTaskGridPositions(0, entries[0].task, -0x13C000, -0x60000, 0x1E000, 0);
            configureFourColumnTaskGridMovement(0, entries[0].task, -0x88000, -0x60000, 0x1E000, 0x1E000, 1);
            configureMovement2D(CHILD_MOVEMENT(entries[1], resource14), -0x8000, -0x48000, 0x1E000, 0x1E000, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[2], resource14), 0, 0, 0x1E000, 0x1E000, 6);
            configureMovement2D(CHILD_MOVEMENT(entries[3], resource16), 0x40000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0x1E000, 6);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (FIRST_FOUR_READY(entries)) {
            configureMovement2D(CHILD_MOVEMENT(entries[4], resource16), -0x3C000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[4].flags == 0) {
            GameTask *task;
            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            entries[3].command = 3;
            entries[4].command = 1;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = D_8013AE88;
            D_8013B41F = 0;
            task->data18 = 5;
        }
        break;
    case 5:
        playSoundEffect(0, 2, 32);
        D_8013AE88->data18 = 6;
        break;
    case 6:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (input->selectedIndex != input->previousIndex) {
            selection->unk02 = selection->value;
            if ((u16)func_80073760(entries[0].task, input->selectedIndex) < 13) {
                selection->value = input->selectedIndex;
                if (selection->value != selection->unk02) {
                    copyPlayerPaletteRowWithTransition(2, character, selection->value, 0);
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                D_8013B41F = 1;
            }
            if (input->choiceResult < 0) {
                D_8013AE98->data18 = 11;
                goto leave;
            }
            if (func_80073760(entries[0].task, input->selectedIndex) == 13) {
                D_8013AE98->data18 = 11;
                D_8013AE88->data18 = 16;
            } else if (selection->value == gRacePlayers[player].unk00D) {
                goto resume;
            } else if (((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[player])->unk_3E4 < 500) {
                playSoundEffect(0, 0x302, 32);
resume:
                D_8013AE88->data18 = 9;
            } else {
                createPlayerModelPreviewTask(3, 10, 1, 64, gPaletteSelectionPurchasePreviewY[character] - 50, 384, 51);
                D_8013B517 = 0;
                entries[6].task = createCountdownTask(8);
                D_8013AE88->data18 = 10;
            }
        }
        break;
    case 10: {
        GameTask *countdown;
        if ((s16)entries[5].flags == 0) {
            countdown = entries[6].task;
            if (countdown->data1A == 1) {
                countdown->data18 = 1;
                entries[5].command = 1;
                copyPlayerPaletteRowWithTransition(0, character, selection->value, 128);
                {
                    RacePlayer *players = gRacePlayers;
                    RacePlayer *racePlayer = &players[player];
                    SaveRecord *records = (SaveRecord *)&D_8013080C;
                    records[player].unk_3F6[racePlayer->unk009] =
                        (racePlayer->unk00D = selection->value);
                }
                entries[6].task = createCountdownTask(129);
                playSoundEffect(1, 0x202, 32);
                D_8013AE88->data18 = 7;
            }
        }
        break;
    }
    case 7: {
        GameTask *countdown;
        spawnPaletteBurstTask(gPaletteSelectionSpriteMapping.entries[character][selection->value]);
        countdown = entries[6].task;
        if (countdown->data1A == 1) {
            countdown->data18 = 1;
            entries[5].command = 99;
            createTimedSpriteB5Task(16, -50, 15);
            D_8013B517 = 1;
            entries[6].task = createValueTransitionTask(
                &((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[player])->unk_3E4,
                ((GlobalFlagStatePrefix *)&((SaveRecord *)&D_8013080C)[player])->unk_3E4 - 500, 100);
            D_8013AE88->data18 = 8;
        }
        break;
    }
    case 8: {
        GameTask *transition;
        if (gFrameCounter & 1) {
            playSoundEffect(1, 2, 32);
        }
        transition = entries[6].task;
        if (transition->data1A != 1) {
            break;
        }
        transition->data18 = 1;
        playSoundEffect(0, 0x102, 32);
        /* fall through */
    }
    case 9: {
        GameTask *task = D_8013AE88;
        entries[0].command = 21;
        input->choiceResult = 0;
        task->data18 = 6;
        break;
    }
    case 16: {
        GameTask *task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 17;
        break;
    }
    case 17:
        if ((s16)entries[0].flags == 0) {
leave:
            D_8013AE88->data18 = 18;
        }
        break;
    case 18:
        playSoundEffect(0, 0x202, 32);
        D_8013AE88->data18 = 11;
        break;
    case 11:
        if (FIRST_FOUR_READY(entries)) {
            GameTask *task;
            queueMenuResourceReads((s16)D_8013AE98->data18);
            configureMovement2D(CHILD_MOVEMENT(entries[4], resource16), -0xF0000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, -1000);
            task = D_8013AE88;
            entries[2].command = 23;
            task->data18 = 12;
        }
        break;
    case 12:
        if ((s16)entries[4].flags == 0) {
            configureFourColumnTaskGridMovement(0, entries[0].task, -0x13C000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D(CHILD_MOVEMENT(entries[1], resource14), 0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[2], resource14), -0xB4000, 0, 0x1E000, 0x1E000, -1000);
            configureMovement2D(CHILD_MOVEMENT(entries[3], resource16), 0xF4000,
                                (gPaletteSelectionMainPreviewY[character] + 50) * 4096, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 13;
        }
        break;
    case 13:
        if (FIRST_FOUR_READY(entries)) {
            D_8013AE88->data18 = 14;
        }
        break;
    case 14:
        D_8013AE88->data18 = 15;
        break;
    default:
        D_8013AE98->data1C--;
        func_800149A8();
        break;
    }
}

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
        drawDecimalNumber8x8(x + 74, y + 152, 6, 0,
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

void initializeTaskGridPositions(s32 unused, GameTask *task, s32 x, s32 y,
                                 s32 unusedStep, s32 unusedDelay) {
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
