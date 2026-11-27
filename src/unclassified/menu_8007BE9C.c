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
extern u8 gMainMenuSelectionIndex __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));

/* Glyph codes, formatting controls, and terminators for the help panel. */
const u16 gMainMenuSinglePlayerRaceHelpText[54]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x004D, 0x0041, 0x003E, 0x0055, 0x00DF, 0x00B5, 0x0042,
    0x0043, 0x0054, 0x0062, 0x00BE, 0x0094, 0x00B0, 0x0099, 0x00A3,
    0x0097, 0x008E, 0x00A4, 0x0002, 0x0109, 0x00A7, 0x00A6, 0x00BC,
    0x0094, 0x0025, 0x800A, 0xC007, 0x0004, 0x0109, 0x00B0, 0x00CD,
    0x00A7, 0x0074, 0x002C, 0x006A, 0x009E, 0x00BB, 0x00CF, 0x0038,
    0x00FB, 0x00FC, 0x00A5, 0x0047, 0x003E, 0x0086, 0x00C0, 0x00B4,
    0x00B8, 0x0095, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gMainMenuMultiplayerRaceHelpText[28]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x004D, 0x0041, 0x003E, 0x0055, 0x00DF, 0x00B5, 0x0042,
    0x0043, 0x0054, 0x0062, 0x00BE, 0x0094, 0x00B0, 0x0099, 0x00A3,
    0x0097, 0x008E, 0x00A4, 0x0002, 0x0109, 0x00A7, 0x00A6, 0x00BC,
    0x0094, 0x0025, 0x800A, 0x0000,
};

const u16 gMainMenuSinglePlayerRaceRulesHelpText[62]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x8020, 0x8020, 0x0034, 0x004E, 0x0085, 0x002C, 0x007E,
    0x0049, 0x006C, 0x004E, 0x0035, 0x800A, 0x8020, 0x8020, 0x0034,
    0x004D, 0x0040, 0x002C, 0x0055, 0x0049, 0x006C, 0x004E, 0x0035,
    0x800A, 0x8020, 0x8020, 0x0034, 0x0055, 0x0069, 0x003E, 0x0049,
    0x0073, 0x002C, 0x0062, 0x0035, 0x800A, 0xC007, 0x00AA, 0x0004,
    0x00A3, 0x00AA, 0x0073, 0x002C, 0x0062, 0x00C0, 0x0092, 0x00A0,
    0x00D2, 0x00B0, 0x009E, 0x0033, 0x00FB, 0x00FC, 0x00B4, 0x00B4,
    0x00B8, 0x0095, 0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gMainMenuMultiplayerRaceRulesHelpText[68]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x8020, 0x8020, 0x0034, 0x004E, 0x0085, 0x002C, 0x007E,
    0x0049, 0x006C, 0x004E, 0x0035, 0x800A, 0x8020, 0x8020, 0x0034,
    0x004D, 0x0040, 0x002C, 0x0055, 0x0049, 0x006C, 0x004E, 0x0035,
    0x800A, 0x8020, 0x8020, 0x0034, 0x0055, 0x0069, 0x003E, 0x0049,
    0x0073, 0x002C, 0x0062, 0x0035, 0x800A, 0xC007, 0x00AA, 0x0004,
    0x00A3, 0x00AA, 0x0073, 0x002C, 0x0062, 0x00C0, 0x0092, 0x00A0,
    0x00D2, 0x00B0, 0x009E, 0x0033, 0x0003, 0x0086, 0x006B, 0x0043,
    0x00AB, 0x00FB, 0x00FC, 0x00AB, 0x00B4, 0x00B8, 0x0095, 0x00B0,
    0x009F, 0x00BF, 0x0000, 0x0000,
};

const u16 gMainMenuCourseSelectionHelpText[50]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x004B, 0x002C, 0x004E, 0x00BE, 0x00D9, 0x00BF, 0x00CD,
    0x0038, 0x0002, 0x0101, 0x00AA, 0x0051, 0x0043, 0x0062, 0x00BE,
    0x0098, 0x00A0, 0x0094, 0x0064, 0x002C, 0x007E, 0x00CD, 0x009E,
    0x0033, 0x800A, 0xC006, 0x004E, 0x0085, 0x002C, 0x007E, 0x005D,
    0x0039, 0x006F, 0x00BE, 0x00A3, 0x0097, 0x0094, 0x0051, 0x0043,
    0x0061, 0x006F, 0x0072, 0x00C0, 0x0088, 0x0043, 0x006F, 0x0055,
    0x0025, 0x0000,
};

const u16 gMainMenuShopHelpText[52]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC006, 0x0034, 0x0083, 0x002C, 0x007E, 0x0081, 0x002C, 0x004E,
    0x0035, 0x800A, 0x0034, 0x0044, 0x003C, 0x0042, 0x0081, 0x002C,
    0x004E, 0x0035, 0x800A, 0x0034, 0x0081, 0x0069, 0x002C, 0x0052,
    0x0081, 0x002C, 0x004E, 0x0035, 0x800A, 0xC007, 0x00CD, 0x0093,
    0x00BC, 0x0093, 0x00BC, 0x00A6, 0x00B4, 0x00AA, 0x00BE, 0xC006,
    0x00EA, 0x0094, 0xC007, 0x009B, 0x00A5, 0x00C0, 0x00CD, 0x0098,
    0x00B0, 0x009E, 0x0033, 0x0000,
};

const u16 gMainMenuSaveHelpText[30]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x009B, 0x00BB, 0x00B0, 0x00CD, 0x00AA, 0x007D, 0x002C,
    0x0051, 0x00BE, 0xC006, 0x004F, 0x002C, 0x0081, 0xC007, 0x009D,
    0x00A4, 0x0051, 0x0043, 0x0055, 0x006A, 0x00A7, 0x00B4, 0x00CE,
    0x00B9, 0x00B0, 0x009E, 0x0033, 0x0000, 0x0000,
};

const u16 gMainMenuShinobinBoardUnavailableHelpText[28]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x00C5, 0x00BF, 0x00A9, 0x00BF, 0x00A6, 0x00C0, 0x00B8,
    0x0038, 0x004D, 0x005A, 0x0080, 0x006F, 0x0100, 0x00AA, 0x0083,
    0x002C, 0x007E, 0x00AB, 0x0096, 0x0093, 0x00A4, 0x00B0, 0x009F,
    0x00BF, 0x0033, 0x0000, 0x0000,
};

const u16 gMainMenuMultiplayerShopUnavailableHelpText[14]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0003, 0x0086, 0x006B, 0x0043, 0x00CD, 0x00AB, 0x0119,
    0x00BB, 0x00B0, 0x009F, 0x00BF, 0x0033, 0x0000,
};

const u16 gMainMenuMultiplayerCourseSelectionUnavailableHelpText[20]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0003, 0x0086, 0x006B, 0x0043, 0x00CD, 0x00AB, 0x0092,
    0x00A0, 0x00D1, 0x009B, 0x00A5, 0x00AB, 0x00CD, 0x0098, 0x00B0,
    0x009F, 0x00BF, 0x0033, 0x0000,
};

const u16 gMainMenuReturnToTitleHelpText[14]
    __attribute__((section(".data.main_menu_text_bodies"), aligned(4))) = {
    0xC007, 0x0051, 0x0043, 0x0055, 0x006A, 0x00A7, 0x00B4, 0x00CE,
    0x00B9, 0x00B0, 0x009E, 0x0033, 0x0000, 0x0000,
};

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
    gMainMenuSinglePlayerRaceHelpText,
    gMainMenuSinglePlayerRaceRulesHelpText,
    gMainMenuCourseSelectionHelpText,
    gMainMenuShopHelpText,
    gMainMenuSaveHelpText,
    gMainMenuShinobinBoardUnavailableHelpText,
    gMainMenuMultiplayerShopUnavailableHelpText,
    gMainMenuMultiplayerCourseSelectionUnavailableHelpText,
    gMainMenuReturnToTitleHelpText,
    gMainMenuMultiplayerRaceHelpText,
    gMainMenuMultiplayerRaceRulesHelpText,
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

            if (playerCountAbsolute == singlePlayer) {
                mode = singlePlayer;
            } else {
                mode = 3;
            }
            selection->mode = 0;
            selection->selectedIndex = getValidatedSelectionIndex();
            selection->previousIndex = selection->selectedIndex;
            selection->choiceResult = 0;
            if (playerCountAbsolute == singlePlayer) {
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
            playerCount = playerCountAbsolute;
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
            gSelectionMenuFadeStarted = 0;
            task->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            entries[2].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (selection->selectedIndex >= 0) {
            gMainMenuSelectionIndex = selection->selectedIndex;
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
                gSelectionMenuFadeStarted = 1;
                break;
            }
        }
        if (gSelectionMenuFadeStarted == 0) {
            entries[1].command = 22;
            entries[2].command = 22;
            gSelectionMenuFadeStarted = 1;
        }
        if (selection->choiceResult < 0) {
            GameTask *parent = gCurrentTaskParent;
            GameTask *task = gCurrentTask;

            parent->data18 = 5;
            task->data18 = 5;
        } else {
            const u8 *parentState = &gMainMenuParentStates[gMainMenuSelectionIndex];
            GameTask *parent = gCurrentTaskParent;
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
            queueMenuResourceReads((s16)gCurrentTaskParent->data18);
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
            GameTask *parent = gCurrentTaskParent;

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
        gCurrentTaskParent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

s32 getValidatedSelectionIndex(void) {
    switch (gMainMenuSelectionIndex) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        return gMainMenuSelectionIndex;
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
                 0xB1, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    playerCount = playerCountAbsolute;
    if (playerCount != 1) {
        return;
    }
    drawSpriteByIndexWithBlend(position->x / 4096 + 74, position->y / 4096 + 140,
                 0xBD, 10, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawDecimalNumber8x8(position->x / 4096 + 74, position->y / 4096 + 152,
                         6, 0, gPlayerSaveRecords[0].money, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    drawWrappedText8x8(position->x / 4096 + 122, position->y / 4096 + 152,
                       2, gSelectionMoneyCurrencyLabel, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    if (gCurrentTask->data1A == playerCount) {
        addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      gMenuColorCycleColor.r, gMenuColorCycleColor.g, gMenuColorCycleColor.b, 2,
                      &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    } else {
        addTilePacketWithBlend(position->x / 4096 + 68, position->y / 4096 + 136, 68, 32,
                      128, 128, 128, 2, &gMainOrderingTables[gActiveGraphicsBuffer], gCurrentTask->data1C);
    }
}

char gSelectionMoneyCurrencyLabel[4] __attribute__((section(".data.selection_money_currency_label"), aligned(4))) = "\x82" "f";

void initializeFlagSelectedCdAwareTask(void) {
    GameTask *task;
    GameTask *parent;
    gCurrentTask->data18 = 1;
    gCurrentTask->data1A = getCourseSelectionGroupIndex();
    task = gCurrentTask;
    parent = gCurrentTaskParent;
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
