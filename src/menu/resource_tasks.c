#include "panel_tasks_internal.h"
#include "game/menu/selected_sprite.h"
#include "game/menu/task_widgets.h"
#include "game/engine/system_runtime.h"
#include "game/audio/music.h"
#include "game/engine/byte_search.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/menu/menu_selection_state.h"
#include "game/menu/movement.h"
#include "game/menu/selected_records.h"
#include "game/race/player/race_player.h"
#include "game/race/ui/race_time.h"
#include "game/save/save_data.h"

typedef struct MainTailSelectionResource {
    s16 unk00;
    s16 selectedIndex;
} MainTailSelectionResource;

extern u16 D_8013AFD0 __attribute__((section(".data")));

extern ByteMappingDescriptor D_801325FC[];
extern ByteMappingDescriptor D_80132638[];
extern const u16 *D_801325D0[];
extern const u16 D_80131F20[];
extern const u16 D_80131F94[];
extern const u16 D_80132004[];
extern const u16 D_80132070[];
extern const u16 D_80132090[];
extern const u16 D_80132104[];
extern const u16 D_80132178[];
extern u8 mainTailSelectionGp;

const GridSpriteEntry gMainTailSelectionSprites[4]
    __attribute__((section(".data.main_tail_selection_sprites"), aligned(4))) = {
    {0xAC, 0, {0, 0, 0}},
    {0xAD, 0, {0, 0, 0}},
    {0xAE, 0, {0, 0, 0}},
    {0xAF, 0, {0, 0, 0}},
};

const u16 *gMainTailSelectionTexts[7]
    __attribute__((section(".data.main_tail_selection_texts"), aligned(4))) = {
    D_80131F20, D_80131F94, D_80132004, D_80132070,
    D_80132090, D_80132104, D_80132178,
};

u8 gMainTailSelectionMapping[4] __attribute__((section(".data.main_tail_selection_mapping"), aligned(4))) = {
    0, 1, 2, 3,
};

const u8 gMainTailMultiplayerTextMapping[4]
    __attribute__((section(".data.main_tail_multiplayer_text_mapping"), aligned(4))) = {
    4, 5, 6, 3,
};

const u8 gMainTailSelectionParentStates[4]
    __attribute__((section(".data.main_tail_selection_parent_states"), aligned(4))) = {
    20, 21, 22, 6,
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

static const u32 gSelectionIndexedMenuJumpTablePadding
    __attribute__((section(".rodata.selection_indexed_menu_padding"))) = 0;

void drawMainTailSelectionHeaderAndMoneyPanel(void);
void drawMainTailSelectionItemDetails(void);
void drawRegisteredTaskGridSpriteWithNumber(void);
void drawCurrentTaskCountRecordsPrompt(void);
void drawCurrentTaskStatsRecordsPrompt(void);

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
    setCurrentTaskCallback(updateMainTailSelectionMenuTask);
    D_8013AE88->unk10 |= 2;
}

void updateMainTailSelectionMenuTask(void) {
    FlagSelectedMenuState *selection;
    RegisteredTaskEntry *entries;
    s32 mode;
    s32 complete;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        configureTaskStateTransition(11, 0, 30);
        setTaskBackgroundMode(2, -4096, 4096);
        setSecondaryDisplayTaskMode(0);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if (hasTaskStateValue6(0) == 0) {
            if (D_8013AFCD == 1) {
                mode = 1;
            } else {
                mode = 3;
            }
            selection->mode = 0;
            if (mainTailSelectionGp != 0) {
                selection->selectedIndex = mainTailSelectionGp - 1;
            } else {
                selection->selectedIndex = 0;
            }
            selection->previousIndex = selection->selectedIndex;
            selection->choiceResult = 0;
            D_8013B104 = 0;
            createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)gMainTailSelectionSprites,
                                             gMainTailSelectionMapping, NULL, 0, 0, 20,
                                             -308, -96, 0, 24, 4, 0xB0,
                                             drawSelectedTaskSprite, drawRegisteredTaskGridSpriteWithColor, 4, 1);
            if (D_8013AFCD == 1) {
                createRegisteredTextPanelTask(-320, 8, gMainTailSelectionTexts, NULL, 4, 11);
            } else {
                createRegisteredTextPanelTask(-320, 8, gMainTailSelectionTexts, gMainTailMultiplayerTextMapping, 4, 11);
            }
            createRegisteredCallbackTask(0, 172, -72, drawMainTailSelectionHeaderAndMoneyPanel, 4, 21);
            createRegisteredTexturePairTask(172, -72, NULL, 4, selection->selectedIndex, 1, 5, 31);
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                -0x8000, -0x48000, -0x1E000, 0, 7);
            D_8013AE88->data18 = 3;
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
            task = D_8013AE88;
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
            mainTailSelectionGp = selection->selectedIndex + 1;
        }
        if (selection->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                D_8013B41F = 1;
            }
            if (selection->choiceResult < 0) {
                D_8013AE98->data18 = 6;
                D_8013AE88->data18 = 5;
            } else {
                s16 selectedIndex = selection->selectedIndex;
                const u8 *parentState = &gMainTailSelectionParentStates[selectedIndex];
                GameTask *parent = D_8013AE98;
                parent->data18 = *parentState;
                D_8013AE88->data18 = 9;
            }
            if (D_8013AE98->data18 == 6) {
                mainTailSelectionGp = 0;
            } else {
                mainTailSelectionGp = selection->selectedIndex + 1;
            }
        }
        break;
    case 9: {
        GameTask *task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 10;
        break;
    }
    case 10:
        if ((s16)entries[0].flags == 0) {
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            queueMenuResourceReads((s16)D_8013AE98->data18);
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
            entries[0].command = 23;
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x140000, 0x8000, -0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
            D_8013AE88->data18 = 6;
        }
        break;
    case 6:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            D_8013AE88->data18 = 7;
        }
        break;
    case 7:
        D_8013AE88->data18 = 8;
        break;
    default: {
        GameTask *parent = D_8013AE98;
        D_8013AFE0 = mainTailSelectionGp - 1;
        parent->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
    }
}

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
    setCurrentTaskCallback(updateSelectionIndexedMenuTask);
    D_8013AE88->unk10 |= 2;
}

void updateSelectionIndexedMenuTask(void) {
    s32 mode;
    ByteMappingDescriptor *group;
    FlagSelectedMenuState *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s16 selectedIndex;
    u16 nextState;
    GameTask *nextParent;
    u16 course;
    u8 players;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        players = D_8013AFCD;
        if (players == 1) {
            mode = 1;
        } else {
            mode = 3;
        }
        if (D_8013AE88->data1A == 0 && players == 2) {
            group = &D_80132638[D_8013AE88->data1A];
        } else {
            group = &D_801325FC[D_8013AE88->data1A];
        }
        selection->mode = 0;
        selectedIndex = findIdentityMappedIndexOrZero(group, D_8013B104);
        selection->selectedIndex = selectedIndex;
        selection->previousIndex = selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)D_8012F798,
                                         group->mapping, NULL, 0, 0, 20,
                                         -312, -88, group->spacingX, group->spacingY,
                                         group->count, 149, drawSelectedTaskSprite,
                                         drawRegisteredTaskGridSpriteWithNumber, 4, 1);
        createRegisteredTextPanelTask(-320, 8, D_801325D0, group->textMapping, 4, 11);
        createRegisteredCallbackTask(0, 172, -72, drawMainTailSelectionItemDetails, 4, 21);
        createRegisteredTexturePairTask(172, -72, group->textureMapping, group->count,
                                        selection->selectedIndex, 1, 5, 31);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x84000, -0x58000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 4);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0, 4);
            configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0, 4);
            D_8013AE88->data18 = 3;
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
            task = D_8013AE88;
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
        if (selection->choiceResult != 0 && (s16)entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                entries[2].command = 22;
                D_8013B41F = 1;
            }
            if (selection->choiceResult < 0) {
                nextParent = D_8013AE98;
                nextState = 9;
                goto exitSelection;
            }
            D_8013AE88->data18 = 13;
        }
        break;
    case 13: {
        GameTask *task = D_8013AE88;
        entries[0].command = 10;
        task->data18 = 14;
        break;
    }
    case 14:
        if ((s16)entries[0].flags == 0) {
            D_8013AE88->data18 = 5;
        }
        break;
    case 5: {
        GameTask *task = D_8013AE88;
        group = &D_801325FC[task->data1A];
        selectedIndex = selection->selectedIndex;
        if (group->mapping[selectedIndex] == 10) {
            GameTask *parent = D_8013AE98;
            D_8013B104 = 0;
            parent->data18 = 9;
            task->data18 = 6;
        } else {
            selectedIndex = D_8013B104 = func_8007373C(group->mapping, selectedIndex);
            course = D_8012D46C[selectedIndex];
            D_8013AFD0 = 1;
            D_8013AFCE = course;
            if (D_8013AFCD == 1) {
                D_8013AE88->data18 = 9;
            } else {
                nextState = 8;
                nextParent = D_8013AE98;
            exitSelection:
                nextParent->data18 = nextState;
                D_8013AE88->data18 = 6;
            }
        }
        break;
    }
    case 9:
        createChildTaskWithReserve(initializeModeDependentTaskResourcePair);
        D_8013AE8C->data18 = 1;
        D_8013AE88->data18 = 10;
        break;
    case 11: {
        GameTask *parent = D_8013AE98;
        GameTask *task = D_8013AE88;
        parent->data18 = 8;
        task->data18 = 8;
        break;
    }
    case 6:
        queueMenuResourceReads((s16)D_8013AE98->data18);
        moveTaskChildrenWithSpacing(0, entries[0].task, -0x138000, -0x58000, 0x1E000, 0x1E000, -1000);
        entries[0].command = 23;
        configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                            -0x140000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[entries[3].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 7;
        break;
    case 7:
        if ((s16)entries[0].flags == 0 && (s16)entries[1].flags == 0 &&
            (s16)entries[2].flags == 0 && (s16)entries[3].flags == 0) {
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        D_8013AE88->data18 = 12;
        break;
    case 10:
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

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
    setCurrentTaskCallback(updateModeDependentRecordsMenuTask);
}

void updateModeDependentRecordsMenuTask(void) {
    FlagSelectedMenuState *parentSelection;
    MainTailSelectionResource *selection;
    RegisteredTaskEntry *parentEntries;
    RegisteredTaskEntry *entries;
    GameTask *parent;
    GameTask *task;
    s32 complete;
    u16 parentNextState;
    GameTask *transitionTask;
    GameTask *resumeTask;

    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = D_8013AE98;
    parentSelection = (FlagSelectedMenuState *)&D_800D9504[parent->resource14];
    if (parent->resource16 == 0xFFFF) {
        for (;;) {}
    }
    parentEntries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
    task = D_8013AE88;
    if (task->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MainTailSelectionResource *)&D_800D9504[task->resource14];
    if (task->resource16 == 0xFFFF) {
        for (;;) {}
    }
    task = D_8013AE88;
    entries = (RegisteredTaskEntry *)&D_800D9504[task->resource16];
    selection->selectedIndex = parentSelection->selectedIndex;

    switch (task->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x138000, -0x58000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[parentEntries[1].task->resource14],
                            -0x140000, 0x8000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if ((s16)parentEntries[0].flags == 0) {
            parentEntries[0].command = 23;
            configureTaskGridMovement(1, parentEntries[0].task, -0x84000, -0x58000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if ((s16)parentEntries[0].flags == 0) {
            switch (D_8013AE98->data1A) {
            case 0:
                createRegisteredCallbackTask(0, -344, -120, drawCurrentTaskTimeRecordsPrompt, 3, 1);
                break;
            case 1:
                createRegisteredCallbackTask(0, -344, -120, drawCurrentTaskCountRecordsPrompt, 3, 1);
                break;
            case 2:
                createRegisteredCallbackTask(0, -344, -120, drawCurrentTaskStatsRecordsPrompt, 3, 1);
                break;
            }
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0) {
            configureMovement2D((Movement2D *)&D_800D9504[entries[0].task->resource14],
                                -0xA4000, -0x78000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[0].flags == 0) {
            createInputTimedSpriteTask(-40, 44, D_80123840, 2, D_8013AE88->data1A, 0xB0, 0x40, 3, 11);
            entries[0].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = D_8013AE88;
            D_8013B41F = 0;
            transitionTask->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if ((s16)entries[1].flags <= 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                D_8013B41F = 1;
            }
            if ((s16)entries[1].flags >= 0) {
                s16 panelValues[6];

                panelValues[0] = 21;
                panelValues[1] = 5;
                panelValues[2] = 22;
                panelValues[3] = 5;
                panelValues[4] = 23;
                panelValues[5] = 5;
                createRegisteredMovingPanelTask(D_8013AE88->data1A, panelValues, -120, 32, 0, 16, -120, 64, 5, 21);
                D_8013AE88->data18 = 6;
            } else {
                goto dismiss;
            }
        }
        break;
    case 6:
        if ((s16)entries[2].flags >= 0) {
            if ((s16)entries[2].flags == 0) {
                D_8013AE88->data18 = 7;
            } else {
            dismiss:
                D_8013AE88->data18 = 10;
            }
        }
        break;
    case 10:
        configureMovement2D((Movement2D *)&D_800D9504[entries[0].task->resource14],
                            -0x158000, -0x78000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 11;
        break;
    case 11:
        if ((s16)entries[0].flags == 0) {
            moveTaskChildrenWithSpacing(1, parentEntries[0].task, -0x84000, -0x58000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 12;
        }
        break;
    case 12:
        if ((s16)parentEntries[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x84000, -0x58000, 0x1E000, 0x1E000, 0);
            configureMovement2D((Movement2D *)&D_800D9504[parentEntries[1].task->resource14],
                                -0x8C000, 0x8000, 0x1E000, 0, 0);
            D_8013AE88->data18 = 13;
        }
        break;
    case 13:
        if ((s16)parentEntries[0].flags == 0) {
            parentEntries[0].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = D_8013AE88;
            D_8013B41F = 0;
            transitionTask->data18 = 14;
        }
        break;
    case 14:
        resumeTask = D_8013AE98;
        parentNextState = 4;
        parentSelection->choiceResult = 0;
        goto finish;
    case 7:
        configureMovement2D((Movement2D *)&D_800D9504[entries[0].task->resource14],
                            -0x158000, -0x78000, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(1, parentEntries[0].task, -0x138000, -0x58000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[parentEntries[1].task->resource14],
                            -0x140000, 0x8000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[parentEntries[2].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[parentEntries[3].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 8;
        break;
    case 8:
        if ((s16)entries[0].flags == 0 && (s16)parentEntries[0].flags == 0 &&
            (s16)parentEntries[1].flags == 0 && (s16)parentEntries[2].flags == 0 &&
            (s16)parentEntries[3].flags == 0) {
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        resumeTask = D_8013AE98;
        parentNextState = 11;
    finish:
        resumeTask->data18 = parentNextState;
        /* fall through */
    default:
        deleteCurrentTaskSubtree();
        break;
    }
}

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
            deleteCurrentTaskSubtree();
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

void drawMainTailSelectionItemDetails(void) {
    Movement2D *position;
    MainTailSelectionResource *selection;
    s16 index;
    ByteMappingDescriptor *group;
    SelectionItemStats *stats;

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
                 0xB3, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    group = &D_801325FC[D_8013AE98->data1A];
    index = func_8007373C(group->mapping, selection->selectedIndex);
    if ((u16)index >= 10) {
        return;
    }
    stats = &D_8012FCE4[index];
    if (stats->price && stats->rating) {
        func_8006C998(position->x / 4096 + 84, position->y / 4096 + 128,
                     0xBC, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawDecimalNumber8x8(position->x / 4096 + 88, position->y / 4096 + 136,
                             4, 0, stats->price, 0, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 122, position->y / 4096 + 138,
                     0xBF, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        func_8006C998(position->x / 4096 + 84, position->y / 4096 + 148,
                     0xBE, 10, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
        drawHalfStepSpriteBar(position->x / 4096 + 88, position->y / 4096 + 156,
                              stats->rating, 5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
    if (D_8013AE88->data1A == 1) {
        func_8007240C(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      D_8013B3A4, D_8013B3A5, D_8013B3A6, 2,
                      &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    } else {
        func_8007240C(position->x / 4096 + 80, position->y / 4096 + 124, 56, 44,
                      128, 128, 128, 2, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    }
}

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

/* Keep the array incomplete at the call site for absolute address scheduling. */
extern char gCountRecordsHitLabel[] __attribute__((section(".data.count_records_hit_label")));

void drawCurrentTaskCountRecordsPrompt(void) {
    Movement2D *position;
    s16 x, y;
    s16 row;
    s16 palette, textPalette;

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
            textPalette = 2;
        } else {
            palette = 4;
            textPalette = 4;
        }
        func_8006C998(x + 18, y, row + 0x136, palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawHudDecimalNumber8x16(x + 38, y, 2, 0,
                                ((SaveRecord *)&D_8013080C)->countRecordValues[D_8013AFCE][row],
                                palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(x + 58, y + 8, 3, gCountRecordsHitLabel, textPalette, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 106, y,
                     (((SaveRecord *)&D_8013080C)->countRecordIcons[D_8013AFCE][row] & 15) * 3 + 0x14A,
                     D_80124284[((SaveRecord *)&D_8013080C)->countRecordIcons[D_8013AFCE][row] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 124, y,
                     (((SaveRecord *)&D_8013080C)->countRecordIcons[D_8013AFCE][row] >> 4) + 0x13B,
                     7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 20;
    }
    drawWrappedDoubleByteText(position->x / 4096 + 28, position->y / 4096 + 164, 14,
                              (s8 *)gTimeRecordsStartPrompt, 7, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    setTexturedQuadColor(128, 128, 128);
    drawDefaultTexturedPanel(position->x / 4096 + 24, position->y / 4096 + 160, 120, 24,
                             5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

char gCountRecordsHitLabel[4] __attribute__((section(".data.count_records_hit_label"), aligned(4))) = "HIT";

/* Keep the array incomplete at the call site for absolute address scheduling. */
extern const char gStatsRecordsPlayerSuffixLabel[] __attribute__((section(".data.stats_records_player_suffix")));

void drawCurrentTaskStatsRecordsPrompt(void) {
    Movement2D *position;
    s16 x, y;
    s16 row;
    s16 palette, textPalette;

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
            textPalette = 2;
        } else {
            palette = 4;
            textPalette = 4;
        }
        func_8006C998(x + 18, y, row + 0x136, palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawHudDecimalNumber8x16(x + 34, y, 5, 0,
                                ((SaveRecord *)&D_8013080C)->statsRecordValues[D_8013AFCE][row],
                                palette, -1, &D_800EBFF0[D_8013AF78], 1);
        drawLimitedText8x8(x + 78, y + 8, 1, gStatsRecordsPlayerSuffixLabel, textPalette, -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 106, y,
                     (((SaveRecord *)&D_8013080C)->statsRecordIcons[D_8013AFCE][row] & 15) * 3 + 0x14A,
                     D_80124284[((SaveRecord *)&D_8013080C)->statsRecordIcons[D_8013AFCE][row] & 15],
                     -1, &D_800EBFF0[D_8013AF78], 1);
        func_8006C998(x + 124, y,
                     (((SaveRecord *)&D_8013080C)->statsRecordIcons[D_8013AFCE][row] >> 4) + 0x13B,
                     7, -1, &D_800EBFF0[D_8013AF78], 1);
        y += 20;
    }
    drawWrappedDoubleByteText(position->x / 4096 + 28, position->y / 4096 + 164, 14,
                              (s8 *)gTimeRecordsStartPrompt, 7, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
    setTexturedQuadColor(128, 128, 128);
    drawDefaultTexturedPanel(position->x / 4096 + 24, position->y / 4096 + 160, 120, 24,
                             5, -1, &D_800EBFF0[D_8013AF78], D_8013AE88->data1C);
}

const char gStatsRecordsPlayerSuffixLabel[4] __attribute__((section(".data.stats_records_player_suffix"), aligned(4))) = "P";

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
    setCurrentTaskCallback(updateFlagSelectedMenuTask);
    D_8013AE88->unk10 |= 2;
}

void updateFlagSelectedMenuTask(void) {
    s32 mode;
    ByteMappingDescriptor *group;
    FlagSelectedMenuState *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s16 selectedIndex;
    GameTask *task;
    GameTask *parent;
    GameTask *selectionTask;
    u16 value;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (FlagSelectedMenuState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
    case 0:
        setMusicSequenceEnabled(1);
        if (D_8013AFCD == 1) {
            mode = 1;
        } else {
            mode = 3;
        }
        group = &D_8012FD10[D_8013AE88->data1A];
        selection->mode = 0;
        selectedIndex = findIdentityMappedIndexOrZero(group, D_8013B104);
        selection->selectedIndex = selectedIndex;
        selection->previousIndex = selectedIndex;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(mode, 0xB0, 0x40, (const u8 *)D_8012F798,
                                         group->mapping, NULL, 0, 0, 20,
                                         -308, -96, group->spacingX, group->spacingY,
                                         group->count, 149, drawSelectedTaskSprite,
                                         drawRegisteredSelectionItemStatus, 4, 1);
        createRegisteredCallbackTask(0, 172, -72, drawSelectionItemDetails, 4, 11);
        createRegisteredTexturePairTask(172, -72, group->textureMapping,
                                        group->count, selection->selectedIndex, 1, 5, 21);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        if (*(s16 *)&entries[0].flags == 0 && *(s16 *)&entries[1].flags == 0 &&
            *(s16 *)&entries[2].flags == 0) {
            group = &D_8012FD10[D_8013AE88->data1A];
            moveTaskChildrenWithSpacing(0, entries[0].task, -0x80000, -0x60000, 0x1E000, 0, 1);
            configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0, group->count);
            configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                                -0x8000, -0x48000, 0x1E000, 0, group->count);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (*(s16 *)&entries[0].flags == 0 && *(s16 *)&entries[1].flags == 0 &&
            *(s16 *)&entries[2].flags == 0) {
            entries[0].command = 20;
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 21;
            entries[1].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (selection->choiceResult != 0 && *(s16 *)&entries[0].flags == 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                entries[1].command = 22;
                D_8013B41F = 1;
            }
            if (selection->choiceResult < 0) {
                value = 6;
                parent = D_8013AE98;
                task = D_8013AE88;
                parent->data18 = value;
                task->data18 = value;
            } else {
                D_8013AE88->data18 = 13;
            }
        }
        break;
    case 13:
        entries[0].command = 10;
        D_8013AE88->data18 = 14;
        break;
    case 14:
        if (*(s16 *)&entries[0].flags == 0) {
            D_8013AE88->data18 = 5;
        }
        break;
    case 5:
        selectionTask = D_8013AE88;
        group = &D_8012FD10[selectionTask->data1A];
        selectedIndex = selection->selectedIndex;
        if (group->mapping[selectedIndex] == 10) {
            GameTask *menuParent = D_8013AE98;
            D_8013B104 = 0;
            menuParent->data18 = 6;
            selectionTask->data18 = 6;
        } else {
            selectedIndex = D_8013B104 = func_8007373C(group->mapping, selectedIndex);
            value = D_8012D46C[selectedIndex];
            D_8013AFD0 = 1;
            D_8013AFCE = value;
            task = D_8013AE88;
            task->data18 = 10;
        }
        break;
    case 10:
        createChildTaskWithReserve(initializeModeDependentTailResourcePairTask);
        D_8013AE8C->data18 = 1;
        D_8013AE88->data18 = 11;
        break;
    case 12:
        value = 8;
        parent = D_8013AE98;
        task = D_8013AE88;
        parent->data18 = value;
        task->data18 = value;
        break;
    case 6:
        queueMenuResourceReads((s16)D_8013AE98->data18);
        moveTaskChildrenWithSpacing(0, entries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
        entries[0].command = 23;
        configureMovement2D((Movement2D *)&D_800D9504[entries[1].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[entries[2].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 7;
        break;
    case 7:
        if (*(s16 *)&entries[0].flags == 0 && *(s16 *)&entries[1].flags == 0 &&
            *(s16 *)&entries[2].flags == 0) {
            D_8013AE88->data18 = 8;
        }
        break;
    case 8:
        D_8013AE88->data18 = 9;
        break;
    case 11:
        break;
    default:
        D_8013AE98->data1C--;
        deleteCurrentTaskSubtree();
        break;
    }
}

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
    setCurrentTaskCallback(updateLapRecordsMenuTask);
}

void updateLapRecordsMenuTask(void) {
    FlagSelectedMenuState *parentSelection;
    MainTailSelectionResource *selection;
    RegisteredTaskEntry *parentEntries;
    RegisteredTaskEntry *entries;
    GameTask *parent;
    GameTask *task;
    s32 complete;
    s16 panelValues[6];
    u16 parentNextState;
    GameTask *transitionTask;
    GameTask *resumeTask;

    if (D_8013AE98->resource14 == 0xFFFF) {
        for (;;) {}
    }
    parent = D_8013AE98;
    parentSelection = (FlagSelectedMenuState *)&D_800D9504[parent->resource14];
    if (parent->resource16 == 0xFFFF) {
        for (;;) {}
    }
    parentEntries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE98->resource16];
    task = D_8013AE88;
    if (task->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (MainTailSelectionResource *)&D_800D9504[task->resource14];
    if (task->resource16 == 0xFFFF) {
        for (;;) {}
    }
    task = D_8013AE88;
    entries = (RegisteredTaskEntry *)&D_800D9504[task->resource16];
    selection->selectedIndex = parentSelection->selectedIndex;

    switch (task->data18) {
    case 0:
        moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 1;
        break;
    case 1:
        if ((s16)parentEntries[0].flags == 0) {
            parentEntries[0].command = 23;
            configureTaskGridMovement(1, parentEntries[0].task, -0x80000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if ((s16)parentEntries[0].flags == 0) {
            createRegisteredCallbackTask(0, -340, -128, drawCurrentTaskLapRecordsPrompt, 3, 1);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if ((s16)entries[0].flags == 0) {
            configureMovement2D((Movement2D *)&D_800D9504[entries[0].task->resource14],
                                -0xA0000, -0x80000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if ((s16)entries[0].flags == 0) {
            createInputTimedSpriteTask(-36, 36, D_80123840, 2, D_8013AE88->data1A, 0xB0, 0x40, 3, 11);
            entries[0].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = D_8013AE88;
            D_8013B41F = 0;
            transitionTask->data18 = 5;
        }
        break;
    case 5:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[0].command = 22;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if ((s16)entries[1].flags <= 0) {
            if (D_8013B41F == 0) {
                entries[0].command = 22;
                D_8013B41F = 1;
            }
            if ((s16)entries[1].flags >= 0) {
                panelValues[0] = 21;
                panelValues[1] = 5;
                panelValues[2] = 22;
                panelValues[3] = 5;
                panelValues[4] = 23;
                panelValues[5] = 5;
                createRegisteredMovingPanelTask(D_8013AE88->data1A, panelValues, -116, 24, 0, 16, -116, 56, 5, 21);
                D_8013AE88->data18 = 6;
            } else {
                D_8013AE88->data18 = 11;
            }
        }
        break;
    case 6:
        if ((s16)entries[2].flags >= 0) {
            if ((s16)entries[2].flags == 0) {
                D_8013AE88->data18 = 7;
            } else {
                D_8013AE88->data18 = 11;
            }
        }
        break;
    case 11:
        configureMovement2D((Movement2D *)&D_800D9504[entries[0].task->resource14],
                            -0x154000, -0x80000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 12;
        break;
    case 12:
        if ((s16)entries[0].flags == 0) {
            moveTaskChildrenWithSpacing(1, parentEntries[0].task, -0x80000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 13;
        }
        break;
    case 13:
        if ((s16)parentEntries[0].flags == 0) {
            moveTaskChildrenWithSpacing(2, parentEntries[0].task, -0x80000, -0x60000, 0x1E000, 0x1E000, 0);
            D_8013AE88->data18 = 14;
        }
        break;
    case 14:
        if ((s16)parentEntries[0].flags == 0) {
            parentEntries[0].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            transitionTask = D_8013AE88;
            D_8013B41F = 0;
            transitionTask->data18 = 15;
        }
        break;
    case 15:
        resumeTask = D_8013AE98;
        parentNextState = 4;
        parentSelection->choiceResult = 0;
        goto finish;
    case 7:
        configureMovement2D((Movement2D *)&D_800D9504[entries[0].task->resource14],
                            -0x154000, -0x80000, 0x1E000, 0x1E000, -1000);
        configureTaskGridMovement(1, parentEntries[0].task, -0x134000, -0x60000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[parentEntries[1].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        configureMovement2D((Movement2D *)&D_800D9504[parentEntries[2].task->resource14],
                            0xAC000, -0x48000, 0x1E000, 0x1E000, -1000);
        D_8013AE88->data18 = 8;
        break;
    case 8:
        if ((s16)entries[0].flags == 0 && (s16)parentEntries[0].flags == 0 &&
            (s16)parentEntries[1].flags == 0 && (s16)parentEntries[2].flags == 0) {
            D_8013AE88->data18 = 9;
        }
        break;
    case 9:
        resumeTask = D_8013AE98;
        parentNextState = 12;
    finish:
        resumeTask->data18 = parentNextState;
        /* fall through */
    default:
        deleteCurrentTaskSubtree();
        break;
    }
}

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
        if (value == func_8007373C(descriptor->mapping, value)) {
            result = i;
            break;
        }
    }
    return result;
}
