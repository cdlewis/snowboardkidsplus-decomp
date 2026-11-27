#include "common.h"
#include "game/engine/task.h"
#include "game/io/cd_queue.h"
#include "game/graphics/graphics.h"
#include "game/engine/game_options.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/race/player/race_player.h"
#include "game/audio/music.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"
#include "game/save/save_data.h"

typedef struct MainMenuSelection {
    s16 mode;
    s16 selectedIndex;
    s16 previousIndex;
    s16 choiceResult;
} MainMenuSelection;

extern MenuRgbColor gMenuColorCycleColor __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 D_8013AFCD __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *D_8013AE98 __attribute__((section(".data")));

extern const u16 D_8012F424[], D_8012F4C8[], D_8012F5CC[], D_8012F630[];
extern const u16 D_8012F698[], D_8012F6D4[], D_8012F70C[], D_8012F728[];
extern const u16 D_8012F750[], D_8012F490[], D_8012F544[];

const GridSpriteEntry gMainMenuSprites[6]
    __attribute__((section(".data.main_menu_sprites"), aligned(4))) = {
    {0xA7, 0, 0, 0},
    {0xA8, 0, 0, 0},
    {0xA9, 0, 0, 0},
    {0xAA, 0, 0, 0},
    {0xAB, 0, 0, 0},
    {0x16E, 0, 0, 0},
};

const u16 *gMainMenuTexts[11]
    __attribute__((section(".data.main_menu_texts"), aligned(4))) = {
    D_8012F424, D_8012F4C8, D_8012F5CC, D_8012F630, D_8012F698, D_8012F6D4,
    D_8012F70C, D_8012F728, D_8012F750, D_8012F490, D_8012F544,
};

const u8 gMainMenuSinglePlayerSpriteMapping[8]
    __attribute__((section(".data.main_menu_mappings"), aligned(4))) = {0, 1, 2, 3, 4};
const u8 gMainMenuMultiplayerSpriteMapping[8]
    __attribute__((section(".data.main_menu_mappings"), aligned(4))) = {0, 1, 2, 3, 5};
const u8 gMainMenuSinglePlayerTextMapping[8]
    __attribute__((section(".data.main_menu_mappings"), aligned(4))) = {0, 1, 2, 3, 4};
const u8 gMainMenuMultiplayerTextMapping[8]
    __attribute__((section(".data.main_menu_mappings"), aligned(4))) = {9, 10, 7, 6, 8};
const u8 gMainMenuParentStates[8]
    __attribute__((section(".data.main_menu_mappings"), aligned(4))) = {7, 9, 10, 11, 15};

s32 getValidatedSelectionIndex(void);
void drawSelectionHeaderAndMoneyPanel(void);

void updateMainMenuSelectionTask(void) {
    MainMenuSelection *selection;
    RegisteredTaskEntry *entries;
    s32 mode;
    s32 complete;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MainMenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        configureTaskStateTransition(11, 0, 30);
        setTaskBackgroundMode(2, -4096, 4096);
        setSecondaryDisplayTaskMode(0);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if (hasTaskStateValue6(0) == 0) {
            s32 singlePlayer = 1;
            u8 playerCount;

            if (D_8013AFCD == singlePlayer) {
                mode = singlePlayer;
            } else {
                mode = 3;
            }
            selection->mode = 0;
            selection->selectedIndex = getValidatedSelectionIndex();
            selection->previousIndex = selection->selectedIndex;
            selection->choiceResult = 0;
            if (D_8013AFCD == singlePlayer) {
                createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)gMainMenuSprites,
                                                 gMainMenuSinglePlayerSpriteMapping, NULL, 0, 0, 20,
                                                 -308, -96, 0, 20, 5, 0xB0,
                                                 drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor,
                                                 4, singlePlayer);
            } else {
                createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)gMainMenuSprites,
                                                 gMainMenuMultiplayerSpriteMapping, NULL, 0, 0, 20,
                                                 -308, -96, 0, 20, 5, 0xB0,
                                                 drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor,
                                                 4, singlePlayer);
            }
            playerCount = D_8013AFCD;
            if (playerCount == 1) {
                if (gRacePlayers[0].unk009 == 8 || gRacePlayers[0].unk009 == 9) {
                    createRegisteredTextPanelTask(-320, 8, gMainMenuTexts, gMainMenuSinglePlayerTextMapping, 4, 11);
                    gCurrentTask->data1A = 2;
                } else {
                    createRegisteredTextPanelTask(-320, 8, gMainMenuTexts, gMainMenuSinglePlayerTextMapping, 4, 11);
                    gCurrentTask->data1A = playerCount;
                }
            } else {
                createRegisteredTextPanelTask(-320, 8, gMainMenuTexts, gMainMenuMultiplayerTextMapping, 4, 11);
                gCurrentTask->data1A = 3;
            }
            createRegisteredCallbackTask(0, 172, -72, drawSelectionHeaderAndMoneyPanel, 4, 21);
            createRegisteredTexturePairTask(172, -72, NULL, 5, selection->selectedIndex, 0, 5, 31);
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            GameTask *task;

            entries[0].command = 20;
            entries[1].command = 20;
            entries[2].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            task = gCurrentTask;
            D_8013B41F = 0;
            task->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (selection->selectedIndex >= 0) {
            D_8013AFE2 = selection->selectedIndex;
        }
        if (selection->choiceResult == 0 || (s16)entries[0].flags != 0) {
            break;
        }
        if (selection->choiceResult > 0) {
            s32 selected = gMainMenuSinglePlayerSpriteMapping[selection->selectedIndex];

            if (gCurrentTask->data1A == 3 && (selected == 2 || selected == 3)) {
                selection->choiceResult = 0;
                entries[0].command = 21;
                entries[1].command = 22;
                entries[2].command = 22;
                configureColorCycleTask(2, 64, 128, 4, 0);
                D_8013B41F = 1;
                break;
            }
        }
        if (D_8013B41F == 0) {
            entries[1].command = 22;
            entries[2].command = 22;
            D_8013B41F = 1;
        }
        if (selection->choiceResult < 0) {
            GameTask *parent = D_8013AE98;
            GameTask *task = gCurrentTask;

            parent->data18 = 5;
            task->data18 = 5;
        } else {
            const u8 *parentState = &gMainMenuParentStates[D_8013AFE2];
            GameTask *parent = D_8013AE98;
            GameTask *task;

            parent->data18 = *parentState;
            task = gCurrentTask;
            entries[0].command = 22;
            task->data18 = 9;
        }
        break;
    case 9: {
        GameTask *task = gCurrentTask;
        entries[0].command = 10;
        task->data18 = 10;
        break;
    }
    case 10:
        if ((s16)entries[0].flags == 0) {
            gCurrentTask->data18 = 5;
        }
        break;
    case 5:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            queueMenuResourceReads((s16)D_8013AE98->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[1].task->resource14],
                                -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&gTaskMemoryPool[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            gCurrentTask->data18 = 6;
        }
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            GameTask *parent = D_8013AE98;

            if (parent->data18 != 15) {
                goto finish;
            }
            if (gCurrentTask->data1A == 3) {
                parent->data18 = 2;
            }
            configureTaskStateTransition(2, 0, 30);
            gCurrentTask->data18 = 7;
        }
        break;
    case 7:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            if (gCurrentTask->data1A == 3) {
                configureTaskStateTransition(1, 0, 30);
                setSecondaryDisplayTaskMode(1);
            }
        finish:
            gCurrentTask->data18 = 8;
        }
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 getValidatedSelectionIndex(void) {
    switch (D_8013AFE2) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        return D_8013AFE2;
    }
    return 0;
}

extern char gSelectionMoneyCurrencyLabel[] __attribute__((section(".data")));

void drawSelectionHeaderAndMoneyPanel(void) {
    Movement2D *position;
    u8 playerCount;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    position = (Movement2D *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->data1A == 0) {
        return;
    }
    drawSpriteByIndexWithBlend(position->x / 4096 + 8, position->y / 4096 - 22,
                 0xB1, 0, -1, &D_800EBFF0[gActiveGraphicsBuffer], gCurrentTask->data1C);
    playerCount = D_8013AFCD;
    if (playerCount != 1) {
        return;
    }
    drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                 0xBD, 10, -1, &D_800EBFF0[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                         6, 0, gPlayerSaveRecords[0].money, 0, -1, &D_800EBFF0[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                       2, gSelectionMoneyCurrencyLabel, 0, -1, &D_800EBFF0[gActiveGraphicsBuffer], gCurrentTask->data1C);
    if (gCurrentTask->data1A == playerCount) {
        addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                      &D_800EBFF0[gActiveGraphicsBuffer], gCurrentTask->data1C);
    } else {
        addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      128, 128, 128, 2, &D_800EBFF0[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

char gSelectionMoneyCurrencyLabel[4] __attribute__((section(".data.selection_money_currency_label"), aligned(4))) = "\x82" "f";

void initializeFlagSelectedCdAwareTask(void) {
    GameTask *task;
    GameTask *parent;
    gCurrentTask->data18 = 1;
    gCurrentTask->data1A = getCourseSelectionGroupIndex();
    task = gCurrentTask;
    parent = D_8013AE98;
    parent->data1C++;
    if (task->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    if (isCdCommandQueueIdle() != 1) {
        setSecondaryDisplayTaskMode(2);
    }
    setCurrentTaskCallback(updateCdAwareCourseSelectionTask);
    gCurrentTask->unk10 |= 2;
}
