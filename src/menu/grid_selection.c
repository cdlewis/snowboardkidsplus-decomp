#include "text_encoding.h"
#include "panel_tasks_internal.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"
#include "game/audio/music.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

#define PALETTE_SELECTION_CURRENCY_LABEL _("Ｇ")
#define CHILD_MOVEMENT(entry, resource) ((Movement2D *)&gTaskMemoryPool[(entry).task->resource])
#define FIRST_FOUR_READY(entries) \
    ((s16)(entries)[0].flags == 0 && (s16)(entries)[1].flags == 0 && \
     (s16)(entries)[2].flags == 0 && (s16)(entries)[3].flags == 0)

#define SPLIT_CHILDREN_READY(entries) \
    ((s16)(entries)[0].flags == 0 && (s16)(entries)[1].flags == 0 && (s16)(entries)[3].flags == 0)

typedef struct PaletteSelectionMapping {
    u8 entries[10][9];
    u8 padding[2];
} PaletteSelectionMapping;

extern MenuRgbColor gMenuColorCycleColor __attribute__((section(".data")));
extern s16 gFrameCounter __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

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
    {0x170, 0, 0, 0},
    {0x170, 1, 0, 0},
    {0x170, 2, 0, 0},
    {0x170, 3, 0, 0},
    {0x170, 4, 0, 0},
    {0x170, 5, 0, 0},
    {0x170, 6, 0, 0},
    {0x170, 7, 0, 0},
    {0x170, 9, 0, 0},
    {0x170, 10, 0, 0},
    {0x170, 11, 0, 0},
    {0x170, 12, 0, 0},
    {0x170, 13, 0, 0},
    {0xC7, 0, 0, 0},
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

/* Unreferenced copy of the glyph sequence in gCurrentPaletteLabel. */
const u16 gUnusedPaletteLabel[6]
    __attribute__((section(".data.unused_palette_label"), aligned(4))) = {
    0x15F, 0x160, 0x110, 0x33, 0, 0
};

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

const u16 gCurrentPaletteLabel[6]
    __attribute__((section(".data.current_palette_label"), aligned(4))) = {
    0x15F, 0x160, 0x110, 0x33, 0, 0
};

u8 gSplitBoardSelectionMapping[8]
    __attribute__((section(".data.split_board_selection_mapping"), aligned(4))) = {0};


void buildSplitBoardSelectionMapping(s16 playerIndex, s16 character, u8 spacingX, u8 spacingY,
                                     ByteMappingDescriptor *menu);
void drawPaletteSelectionPricePanel(void);

void initializeCountedSplitSelectionBoundsTask(void) {
    gCurrentTaskParent->data1C++;
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 0;
    gCurrentTask->data1C = 0;
    if (gCurrentTask->resource14 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setSecondaryDisplayTaskMode(2);
    {
        RECT *bounds = &gMenuModelDrawAreas[0];
        bounds->x = 160;
        bounds->y = 0;
        bounds->w = 160;
        bounds->h = 240;
    }
    {
        RECT *bounds = &gMenuModelDrawAreas[1];
        bounds->x = 160;
        bounds->y = 0;
        bounds->w = 160;
        bounds->h = 0;
    }
    {
        RECT *bounds = &gMenuModelDrawAreas[2];
        bounds->x = 24;
        bounds->y = 132;
        bounds->w = 120;
        bounds->h = 84;
    }
    {
        RECT *bounds = &gMenuModelDrawAreas[3];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    setCurrentTaskCallback(updateSplitBoardSelectionTask);
    gCurrentTask->unk10 |= 2;
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

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    playerIndex = (s16)gCurrentTask->data1A;
    player = gCurrentTask->data1A;
    character = gRacePlayers[playerIndex].unk009;
    palette = gRacePlayers[playerIndex].unk00D;
    selection = &gTaskPanelSelectionValues[playerIndex];

    switch (gCurrentTask->data18) {
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
        buildSplitBoardSelectionMapping((s16)player, character, 0, 20, mapping);
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
        createRegisteredSpacedChildTasks(gridMode, 0xB0, 0x40, (const u8 *)gBoardMenuSpriteEntries, mapping->mapping,
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
        gCurrentTask->data18 = 2;
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
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (SPLIT_CHILDREN_READY(entries) && (s16)entries[2].flags == 0) {
            configureMovement2D(CHILD_MOVEMENT(entries[5], resource16), -0x50000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[5].flags == 0) {
            GameTask *task;
            entries[0].command = 20;
            entries[1].command = 20;
            entries[5].command = 1;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = gCurrentTask;
            gSelectionMenuFadeStarted = 0;
            task->data18 = 5;
        }
        break;
    case 5: {
        s32 complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (input->selectedIndex != input->previousIndex) {
            s16 model;
            selection->unk02 = selection->value;
            model = mapSpacedGroupIndex(entries[0].task, input->selectedIndex);
            if ((u16)model < 5U) {
                selection->value = model;
                if (model != selection->unk02) {
                    entries[5].command = 13;
                    gCurrentTask->data1C = 0;
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (input->choiceResult < 0) {
                gCurrentTaskParent->data18 = 13;
                goto exitSelection;
            }
            if (mapSpacedGroupIndex(entries[0].task, input->selectedIndex) == 5) {
                boardMenuRememberedSelectionAbsolute = 0;
                gCurrentTaskParent->data18 = 11;
                playSoundEffect(0, 0x202, 32);
                gCurrentTask->data18 = 22;
            } else if (gRacePlayers[(s16)player].unk00A[0] == selection->value) {
                gCurrentTask->data18 = 24;
            } else {
                gCurrentTask->data18 = 13;
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
        gCurrentTask->data18 = 14;
        break;
    case 14:
        if ((s16)entries[7].flags >= 0) {
            entries[6].command = 99;
            if ((s16)entries[7].flags != 0) {
                gCurrentTask->data18 = 24;
            } else {
                gCurrentTask->data18 = 15;
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
        gPlayerSaveRecords[(s16)player].boardSelections[currentPlayer->unk009] = model;
        gCurrentTask->data18 = 19;
        setCurrentTaskCallback(updateSplitSelectionBoundsTransition);
        break;
    }
    case 16: {
        SpacedGroupResource *group;
        mapping = &descriptor;
        buildSplitBoardSelectionMapping((s16)player, character, 0, 20, mapping);
        group = (SpacedGroupResource *)&gTaskMemoryPool[entries[0].task->resource14];
        group->mapping = descriptor.mapping;
        group->spacingX = mapping->spacingX;
        group->spacingY = mapping->spacingY;
        group->count = mapping->count;
        gCurrentTask->data18 = 17;
        break;
    }
    case 17: {
        s16 model = mapSpacedGroupIndex(entries[0].task, input->selectedIndex);
        if ((u16)model < 5U) {
            selection->value = model;
            entries[5].command = 13;
        }
        createRegisteredCallbackTask((s16)player, 0, -28, drawCurrentTaskPlayerValuePanel, 2, 61);
        entries[8].task = createCountdownTask(15);
        gCurrentTask->data18 = 18;
        break;
    }
    case 18: {
        GameTask *countdown = entries[8].task;
        if (countdown->data1A == 1) {
            countdown->data18 = 1;
            entries[6].command = 99;
            gCurrentTask->data18 = 24;
        }
        break;
    }
    case 24: {
        GameTask *task;
        input->choiceResult = 0;
        entries[0].command = 21;
        configureColorCycleTask(2, 64, 128, 4, 0);
        task = gCurrentTask;
        gSelectionMenuFadeStarted = 1;
        task->data18 = 5;
        break;
    }
    case 22: {
        GameTask *task;
        task = gCurrentTask;
        entries[0].command = 10;
        task->data18 = 23;
        break;
    }
    case 23:
        if ((s16)entries[0].flags == 0) {
exitSelection:
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
        if (SPLIT_CHILDREN_READY(entries)) {
            GameTask *task;
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            configureMovement2D(CHILD_MOVEMENT(entries[5], resource16), -0x104000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, -1000);
            task = gCurrentTask;
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
            gCurrentTask->data1C = 0;
            gCurrentTask->data18 = 10;
        }
        break;
    case 10:
        if (SPLIT_CHILDREN_READY(entries) && (s16)entries[2].flags == 0) {
            gCurrentTask->data18 = 11;
        }
        break;
    case 11:
        gCurrentTask->data18 = 12;
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void updateSplitSelectionBoundsTransition(void) {
    SplitBoundsResource *movement;
    u8 index;
    s16 height;
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    movement = (SplitBoundsResource *)&gTaskMemoryPool[gCurrentTask->resource16];
    index = gRacePlayers[(s16)gCurrentTask->data1A].unk009;
    height = gMenuModelDrawAreas[0].h;
    switch (gCurrentTask->data18) {
    case 19:
        {
            RECT *bounds = &gMenuModelDrawAreas[1];
            bounds->x = 160;
            bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
            bounds->w = 160;
            bounds->h = gSplitSelectionBoundsConfigs.entries[index][1];
        }
        {
            RECT *bounds = &gMenuModelDrawAreas[0];
            bounds->x = 160;
            bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
            bounds->w = 160;
            bounds->h = 0;
        }
        movement->unk1C = 13;
        playSoundEffect(1, 0x102, 32);
        gCurrentTask->data18 = 20;
        break;
    case 20:
        height += gSplitSelectionBoundsConfigs.entries[index][2];
        if (height <= gSplitSelectionBoundsConfigs.entries[index][1]) {
            {
                RECT *bounds = &gMenuModelDrawAreas[1];
                bounds->x = 160;
                bounds->y = gSplitSelectionBoundsConfigs.entries[index][0] + height;
                bounds->w = 160;
                bounds->h = gSplitSelectionBoundsConfigs.entries[index][1] - height;
            }
            {
                RECT *bounds = &gMenuModelDrawAreas[0];
                bounds->x = 160;
                bounds->y = gSplitSelectionBoundsConfigs.entries[index][0];
                bounds->w = 160;
                bounds->h = height;
            }
        } else {
            movement->unk24 = 13;
            gCurrentTask->data18 = 21;
        }
        break;
    case 21:
        gCurrentTask->data18 = 16;
        setCurrentTaskCallback(updateSplitBoardSelectionTask);
        break;
    }
}

void drawCurrentTaskSpriteB7(void) {
    Movement2D *position;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                      0xB7, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

void drawCurrentTaskFramedSprite20B(void) {
    Movement2D *position;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    setTexturedQuadColor(128, 128, 128);
    setSpriteColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 140, position->y / 4096 + 8,
                           128, 92, 5, 1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawSpriteByIndexWithBlend(position->x / 4096 - 136, position->y / 4096 + 12,
                  0x20B, 1, 0, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void drawCurrentTaskTextPanel144x48(void) {
    Movement2D *position;
    s32 x, y;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16((s16)(x + 16), (s16)(y + 12), 10, gTaskPanel144x48Text, 8, -1,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDefaultTexturedPanel(x, y, 144, 48, 5, 0,
                            &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void drawCurrentTaskPlayerValuePanel(void) {
    Movement2D *position;
    s32 x, y, value;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    value = gRacePlayers[(s16)gCurrentTaskParent->data1A].unk00A[0];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16((s16)(x + 8), (s16)(y + 4), 10, gTaskPlayerValuePanelLabel, 8, -1,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDecimalNumber16((s16)(x + 56), (s16)(y + 4), 1, 0, value + 1, 8, -1,
                 &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawWrappedGlyphText16x16((s16)(x + 8), (s16)(y + 20), 10, gTaskPlayerValuePanelText, 8, -1,
                              &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDefaultTexturedPanel((s16)x, (s16)y, 144, 48, 5, 0,
                            &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void buildSplitBoardSelectionMapping(s16 playerIndex, s16 character, u8 spacingX, u8 spacingY,
                                     ByteMappingDescriptor *menu) {
    ByteMappingDescriptor *descriptor = menu;
    s16 board;
    u8 equippedBoard;

    descriptor->mapping = gSplitBoardSelectionMapping;
    descriptor->spacingX = spacingX;
    descriptor->spacingY = spacingY;
    descriptor->count = 0;
    equippedBoard = gRacePlayers[playerIndex].unk00A[0];
    for (board = 0; board < 5; board++) {
        descriptor = menu;
        if (equippedBoard != board) {
            if (hasStateGridFlag(&gPlayerSaveRecords[playerIndex], character, board) == 1) {
                ((u8 *)descriptor->mapping)[descriptor->count] = board;
                descriptor->textMapping = NULL;
                descriptor->textureMapping = NULL;
                descriptor->unk10 = 0;
                descriptor->count++;
            }
        }
    }

    ((u8 *)descriptor->mapping)[descriptor->count] = 5;
    descriptor->textMapping = NULL;
    descriptor->textureMapping = NULL;
    descriptor->unk10 = 0;
    descriptor->count++;
}

s32 findLastMappedByteIndexOrZero(const ByteMappingDescriptor *descriptor, s16 value) {
    s32 result = 0;
    s32 i;

    for (i = 0; i < descriptor->count; i++) {
        if (descriptor->mapping[i] == value) {
            result = i;
        }
    }
    return result;
}

void initializeCountedFourBoundsTask(void) {
    gCurrentTaskParent->data1C++;
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 0;
    if (gCurrentTask->resource14 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) { for (;;) {} }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    setSecondaryDisplayTaskMode(2);
    {
        RECT *bounds = &gMenuModelDrawAreas[0];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    {
        RECT *bounds = &gMenuModelDrawAreas[1];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    {
        RECT *bounds = &gMenuModelDrawAreas[2];
        bounds->x = 24;
        bounds->y = 132;
        bounds->w = 120;
        bounds->h = 84;
    }
    {
        RECT *bounds = &gMenuModelDrawAreas[3];
        bounds->x = 0;
        bounds->y = 0;
        bounds->w = -1;
        bounds->h = -1;
    }
    gPaletteSelectionCurrentLabelEnabled = 1;
    setCurrentTaskCallback(updatePlayerPaletteSelectionTask);
    gCurrentTask->unk10 |= 2;
}

void updatePlayerPaletteSelectionTask(void) {
    FlagSelectedMenuState *input;
    TaskPanelSelectionValue *selection;
    s32 complete;
    RegisteredTaskEntry *entries;
    s16 player;
    s32 character;
    s32 palette;

    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    input = (FlagSelectedMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    player = gCurrentTask->data1A;
    character = gRacePlayers[player].unk009;
    palette = gRacePlayers[player].unk00D;
    selection = &gTaskPanelSelectionValues[player];

    switch (gCurrentTask->data18) {
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
        createRegisteredCallbackTask(0, -180, 0, drawPaletteSelectionPricePanel, 4, 21);
        previewPalette = palette;
        createGridModelPreviewTask(0, previewCharacter, 0, previewPalette, 244, gPaletteSelectionMainPreviewY[previewCharacter] + 50, 384, 31);
        createGridModelPreviewTask(2, previewCharacter, 0, previewPalette, -240, gPaletteSelectionSidePreviewY[previewCharacter] + 118, 352, 41);
        gRacePlayers[2].unk0B4 = gPaletteSelectionSidePreviewRotation[previewCharacter];
        gCurrentTask->data18 = 2;
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
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if (FIRST_FOUR_READY(entries)) {
            configureMovement2D(CHILD_MOVEMENT(entries[4], resource16), -0x3C000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, 0);
            gCurrentTask->data18 = 4;
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
            task = gCurrentTask;
            gSelectionMenuFadeStarted = 0;
            task->data18 = 5;
        }
        break;
    case 5:
        playSoundEffect(0, 2, 32);
        gCurrentTask->data18 = 6;
        break;
    case 6:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (input->selectedIndex != input->previousIndex) {
            selection->unk02 = selection->value;
            if ((u16)mapSpacedGroupIndex(entries[0].task, input->selectedIndex) < 13) {
                selection->value = input->selectedIndex;
                if (selection->value != selection->unk02) {
                    copyPlayerPaletteRowWithTransition(2, character, selection->value, 0);
                }
            }
            input->previousIndex = input->selectedIndex;
        } else if (input->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (gSelectionMenuFadeStarted == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                gSelectionMenuFadeStarted = 1;
            }
            if (input->choiceResult < 0) {
                gCurrentTaskParent->data18 = 11;
                goto leave;
            }
            if (mapSpacedGroupIndex(entries[0].task, input->selectedIndex) == 13) {
                gCurrentTaskParent->data18 = 11;
                gCurrentTask->data18 = 16;
            } else if (selection->value == gRacePlayers[player].unk00D) {
                goto resume;
            } else if (gPlayerSaveRecords[player].money < 500) {
                playSoundEffect(0, 0x302, 32);
resume:
                gCurrentTask->data18 = 9;
            } else {
                createPlayerModelPreviewTask(3, 10, 1, 64, gPaletteSelectionPurchasePreviewY[character] - 50, 384, 51);
                gPaletteSelectionCurrentLabelEnabled = 0;
                entries[6].task = createCountdownTask(8);
                gCurrentTask->data18 = 10;
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
                    SaveRecord *records = gPlayerSaveRecords;
                    records[player].paletteSelections[racePlayer->unk009] =
                        (racePlayer->unk00D = selection->value);
                }
                entries[6].task = createCountdownTask(129);
                playSoundEffect(1, 0x202, 32);
                gCurrentTask->data18 = 7;
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
            gPaletteSelectionCurrentLabelEnabled = 1;
            entries[6].task = createValueTransitionTask(
                &gPlayerSaveRecords[player].money,
                gPlayerSaveRecords[player].money - 500, 100);
            gCurrentTask->data18 = 8;
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
        GameTask *task = gCurrentTask;
        entries[0].command = 21;
        input->choiceResult = 0;
        task->data18 = 6;
        break;
    }
    case 16: {
        GameTask *task = gCurrentTask;
        entries[0].command = 10;
        task->data18 = 17;
        break;
    }
    case 17:
        if ((s16)entries[0].flags == 0) {
leave:
            gCurrentTask->data18 = 18;
        }
        break;
    case 18:
        playSoundEffect(0, 0x202, 32);
        gCurrentTask->data18 = 11;
        break;
    case 11:
        if (FIRST_FOUR_READY(entries)) {
            GameTask *task;
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
            configureMovement2D(CHILD_MOVEMENT(entries[4], resource16), -0xF0000,
                                (gPaletteSelectionSidePreviewY[character] + 118) * 4096, 0x1E000, 0x1E000, -1000);
            task = gCurrentTask;
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
            gCurrentTask->data18 = 13;
        }
        break;
    case 13:
        if (FIRST_FOUR_READY(entries)) {
            gCurrentTask->data18 = 14;
        }
        break;
    case 14:
        gCurrentTask->data18 = 15;
        break;
    default:
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

void drawCurrentTaskSavedValuePanel(void) {
    Movement2D *position;
    s32 x, y;
    u16 index;
    SaveRecord *records;
    GsOT *ot;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    x = position->x / 4096;
    y = position->y / 4096;
    index = gCurrentTaskParent->data1A;
    if (gCurrentTask->data1A != 0) {
        drawSpriteByIndexWithBlend(x + 8, y - 22, 0xBB, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        drawSpriteByIndexWithBlend(x + 74, y + 140, 0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        records = gPlayerSaveRecords;
        drawDecimalNumber8x8(x + 74, y + 152, 6, 0,
            records[(s16)index].money,
            0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        ot = &gMainOrderingTables[gActiveGraphicsBuffer];
        drawWrappedText8x8(x + 122, y + 152, 2, PALETTE_SELECTION_CURRENCY_LABEL, 0, -1,
            ot, gCurrentTask->data1C);
        if (gCurrentTask->data1A == 1) {
            addTilePacketWithBlend(x + 68, y + 136, 68, 32, gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b,
                2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        } else {
            addTilePacketWithBlend(x + 68, y + 136, 68, 32, 128, 128, 128,
                2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
        }
    }
}

u8 gPaletteSelectionCurrentLabelEnabled __attribute__((section(".sdata"))) = 0;

void drawPaletteSelectionPricePanel(void) {
    Movement2D *position;
    s32 panelX, panelY;
    u16 priceX, priceY;
    s32 player;
    TaskPanelSelectionValue *selection;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    priceX = panelX = position->x / 4096;
    priceY = panelY = position->y / 4096;
    player = (s16)gCurrentTaskParent->data1A;
    selection = &gTaskPanelSelectionValues[player];
    if (gCurrentTask->data1A != 0) {
        if (gRacePlayers[player].unk00D == selection->value && gPaletteSelectionCurrentLabelEnabled != 0) {
            drawWrappedGlyphText16x16((s16)(panelX - 112), (s16)(panelY + 52), 4, gCurrentPaletteLabel, 12, 1,
                                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            if (gCurrentTask->data1A == 1) {
                addTilePacketWithBlend(panelX - 120, panelY + 48, 72, 22, gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b,
                              2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            } else {
                addTilePacketWithBlend(panelX - 120, panelY + 48, 72, 22, 128, 128, 128,
                              2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
        } else {
            drawDecimalNumber8x8((priceX -= 112) + 48, (priceY += 70) + 12, 4, 0, 500, 0, -1,
                                 &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            drawWrappedText8x8(priceX + 80, priceY + 12, 2, PALETTE_SELECTION_CURRENCY_LABEL, 0, -1,
                               &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            if (gCurrentTask->data1A == 1) {
                addTilePacketWithBlend(priceX + 44, priceY + 8, 48, 16, gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b,
                              2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            } else {
                addTilePacketWithBlend(priceX + 44, priceY + 8, 48, 16, 128, 128, 128,
                              2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            }
        }
    }
    setTexturedQuadColor(128, 128, 128);
    setSpriteColor(128, 128, 128);
    drawTexturedFrameStyle9(position->x / 4096 - 140, position->y / 4096 + 8,
                           128, 92, 5, 1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawSpriteByIndexWithBlend(position->x / 4096 - 136, position->y / 4096 + 12,
                  0x171, 4, 0, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
}

void initializeTaskGridPositions(s32 unused, GameTask *task, s32 x, s32 y,
                                 s32 unusedStep, s32 unusedDelay) {
    s32 i;
    TaskGridState *state;
    TaskGridEntry *entry;
    state = (TaskGridState *)&gTaskMemoryPool[task->resource14];
    entry = (TaskGridEntry *)&gTaskMemoryPool[task->resource16];
    for (i = 0; i < state->count; i++) {
        if (i < 8) {
            initMovement2D((Movement2D *)&gTaskMemoryPool[entry[i].task->resource14],
                           x + (i % 4) * 0x20000, y + (i / 4) * 0x28000);
        } else {
            initMovement2D((Movement2D *)&gTaskMemoryPool[entry[i].task->resource14],
                           x + 0xC000, y + 0x50000);
        }
    }
}

void configureFourColumnTaskGridMovement(s32 mode, GameTask *task, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay) {
    TaskGridState *state;
    TaskGridEntry *entries;
    s16 i, end;
    Movement2D *movement;
    state = (TaskGridState *)&gTaskMemoryPool[task->resource14];
    entries = (TaskGridEntry *)&gTaskMemoryPool[task->resource16];
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
            movement = (Movement2D *)&gTaskMemoryPool[entries[i].task->resource14];
            if (delay == -1000) {
                configureMovement2D(movement, x + (i % 4) * 0x20000,
                                    y + (i / 4) * 0x28000, stepX, stepY, delay);
            } else {
                configureMovement2D(movement, x + (i % 4) * 0x20000,
                                    y + (i / 4) * 0x28000, stepX, stepY, delay * (i / 4));
            }
        } else {
            movement = (Movement2D *)&gTaskMemoryPool[entries[i].task->resource14];
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
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    state = (GridMarkerState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    entries = (TaskGridEntry *)&gTaskMemoryPool[gCurrentTask->resource16];
    mode = (s16)state->state;
    colorMode = state->state;
    if (mode > 0) {
        if ((u32)(colorMode - 1) < 2) {
            setSpriteColor(state->red, state->green, state->blue);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&gTaskMemoryPool[entries[state->selectedIndex].task->resource14];
        if (state->selectedIndex < 8) {
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096, 0x16F, 8, -1,
                         &gMainOrderingTables[gActiveGraphicsBuffer], (u16)(state->depth - 1));
        } else {
            drawSpriteByIndexWithBlend(position->x / 4096, position->y / 4096, 0xB0, 0, -1,
                         &gMainOrderingTables[gActiveGraphicsBuffer], (u16)(state->depth - 1));
        }
    }
}

void drawRegisteredTaskGridSprite(void) {
    Movement2D *position;
    GridSpriteParent *parent;
    RegisteredTaskEntry *entry;
    RegisteredTaskEntry *entries;
    const GridSpriteEntry *sprite;
    if (gCurrentTask->resource14 == 0xFFFF) { for (;;) {} }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) { for (;;) {} }
    if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
    parent = (GridSpriteParent *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
    if ((u16)(gCurrentTask->data1E % 10U) == 0) {
        if (gCurrentTaskParent->resource14 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource14];
        entry = &entries[gCurrentTask->data1E / 10U];
    } else {
        if (gCurrentTaskParent->resource16 == 0xFFFF) { for (;;) {} }
        entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTaskParent->resource16];
        entry = &entries[gCurrentTask->data1E / 10U];
    }
    if (parent->mapping != 0) {
        if (entry->command == 99) {
            entry->task = 0;
            deleteCurrentTaskSubtree();
        } else {
            if (parent->mode == 1 && parent->selectedIndex != gCurrentTask->data1A) {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)(gCurrentTask->data1A + parent->count));
            } else {
                sprite = getMappedGridSpriteEntry(parent->sprites, parent->mapping,
                    (s16)gCurrentTask->data1A);
            }
            setSpriteColor(128, 128, 128);
            drawSpriteByIndexWithBlend((s16)(position->x / 4096), (s16)(position->y / 4096),
                sprite->sprite, sprite->color, 0, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
            entry->flags = 0;
        }
    }
}
