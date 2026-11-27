#include "panel_tasks_internal.h"
#include "game/menu/selected_sprite.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/menu/selected_records.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"

typedef struct Mode11MenuSelection {
    s16 mode;
    s16 selectedIndex;
    s16 previousIndex;
    s16 choiceResult;
} Mode11MenuSelection;

extern u8 D_8013AFD8 __attribute__((section(".data")));
extern u8 D_8013AFD9 __attribute__((section(".data")));
extern s16 D_8013AFDE __attribute__((section(".data")));
extern u8 D_8013AFE1 __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern u8 D_8013AFE0 __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern u8 D_8013AFD6 __attribute__((section(".data")));

extern u8 D_8013B108 __attribute__((section(".data")));
extern u8 D_8013B564 __attribute__((section(".data")));
extern s16 D_8013B106 __attribute__((section(".data")));
extern u8 D_8013B0F0 __attribute__((section(".data")));
extern u16 D_8013AFD0 __attribute__((section(".data")));

extern const GridSpriteEntry gMode11SelectionSprites[];
extern const u8 gMode11SelectionMapping[];

void initializeTaskResourcePairWithOneTickTransition(void) {
    gCurrentTask->data18 = 0;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    configureTaskStateTransition(2, 0, 1);
    setCurrentTaskCallback(updateMode11SelectionMenuTask);
    gCurrentTask->unk10 |= 2;
}

void updateMode11SelectionMenuTask(void) {
    Mode11MenuSelection *selection;
    RegisteredTaskEntry *entries;
    s32 complete;
    s32 index;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    selection = (Mode11MenuSelection *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 0:
        setSecondaryDisplayTaskMode(0);
        selection->mode = 0;
        selection->selectedIndex = 0;
        selection->previousIndex = 0;
        selection->choiceResult = 0;
        createRegisteredSpacedChildTasks(1, 0xB0, 0x40, (const u8 *)gMode11SelectionSprites,
                                         gMode11SelectionMapping, NULL,
                                         1, 0, 20, -48, -24, 0, 24, 3, 403,
                                         drawSelectedTaskSprite,
                                         drawRegisteredTaskGridSpriteWithColor, 3, 11);
        gCurrentTask->data18 = 1;
        break;
    case 1:
        if ((s16)entries[1].flags == 0) {
            configureTaskStateTransition(1, 0, 30);
            queueCdCommand(3, 0);
            D_8013B564 = 1;
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        if (hasTaskStateValue6(0) == 0) {
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            gSelectionMenuFadeStarted = 0;
            gCurrentTask->data18 = 3;
        }
        break;
    case 3:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && gSelectionMenuFadeStarted == 0) {
            entries[1].command = 21;
            configureColorCycleTask(2, 64, 128, 4, 0);
            gSelectionMenuFadeStarted = complete;
        }
        if (selection->choiceResult != 0 && (s16)entries[1].flags == 0) {
            entries[1].command = 22;
            if (selection->choiceResult < 0) {
                D_8013B108 = 2;
                gCurrentTask->data18 = 4;
            } else {
                index = selection->selectedIndex;
                if (index == 2) {
                    D_8013B108 = index;
                }
                gCurrentTask->data18 = 7;
            }
        }
        break;
    case 7:
        entries[1].command = 10;
        gCurrentTask->data18 = 8;
        break;
    case 8:
        if ((s16)entries[1].flags == 0) {
            if (selection->selectedIndex != 2) {
                if (selection->selectedIndex == 0) {
                    D_8013B108 = 27;
                }
                index = selection->selectedIndex;
                if (index == 1) {
                    D_8013B108 = 28;
                    D_8013AFD9 = index;
                }
            }
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        configureTaskStateTransition(2, 0, 30);
        if (D_8013B108 != 28) {
            startCdVolumeFade(103, 20, 0, 1);
        }
        D_8013B106 = 0;
        configureColorCycleTask(0, 0, 0, 0, 0);
        gCurrentTask->data18 = 5;
        break;
    case 5:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            gCurrentTask->data18 = 6;
        }
        break;
    case 6:
    default:
        deleteCurrentTaskSubtree();
        return;
    }

    if (gCurrentTask->data18 >= 2) {
        drawScreenDarkeningOverlay(128, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        drawFullScreenTexturedBackground();
    }
}

const GridSpriteEntry gMode11SelectionSprites[6]
    __attribute__((section(".data.mode11_selection_sprites"), aligned(4))) = {
    {400, 0, 0, 0},
    {401, 0, 0, 0},
    {402, 0, 0, 0},
    {400, 2, 0, 0},
    {401, 2, 0, 0},
    {402, 2, 0, 0},
};

const u8 gMode11SelectionMapping[8]
    __attribute__((section(".data.mode11_selection_mapping"), aligned(4))) = {
    0, 1, 2, 3, 4, 5, 0, 0,
};

const u32 gMode11SelectionJumpTablePadding
    __attribute__((section(".rodata.mode11_selection_jump_table_padding"))) = 0;

void selectTaskCallbackFromModeState(void) {
    if (D_8013AFD8 != 0) {
        setCurrentTaskCallback(initializeMode1RacePlayers);
    } else if (D_8013AFD9 != 0) {
        setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
    }
}

void initializeMode1Task(void) {
    D_8013AFD8 = 1;
    gSelectedCourseIndex = -1;
    D_8013B560 = 99;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013B0F0 = 0;
    setCurrentTaskCallback(initializeMode1RacePlayers);
}

void initializeMode1RacePlayers(void) {
    s32 i;
    s16 mode;
    D_8013B560 = gSelectedCourseIndex;
    if ((s16)D_8013AFD8 < 7 && D_8013AFD8 != 0) gSelectedCourseIndex = 0;
    else gSelectedCourseIndex = 9;
    SetFogNearFar(8000, 10000, 400);
    gRacePauseState = 0;
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
    if (D_8013B560 != gSelectedCourseIndex) {
        closeSoundBank(2);
        closeSoundBank(3);
        closeSoundBank(4);
        closeSoundBank(5);
        queueStartupSectorReads();
        setCurrentTaskCallback(preparePlayerResourcesTask);
        gCurrentTask->unk10 |= 2;
    } else {
        setCurrentTaskCallback(initializeRaceTaskState);
    }
}

void preparePlayerResourcesTask(void) {
    queuePlayerResourceReadsAndUploadTextures();
    setCurrentTaskCallback(initializeRaceTaskState);
    gCurrentTask->unk10 |= 2;
}

void initializeModeSectorReadResourcesTask(void) {
    ModeSectorReadState *state;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    state = (ModeSectorReadState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
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
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (ModeSectorReadState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (state->mode == 1) {
        state->sectorIndex = D_8013AFD8 - 1;
    } else {
        state->sectorIndex = D_8013AFD9 - 1;
    }
    queueModeIndexedSectorRead(state->mode, state->sectorIndex);
    setCurrentTaskCallback(prepareLoadedTextSequenceTask);
    gCurrentTask->unk10 |= 2;
}

void prepareLoadedTextSequenceTask(void) {
    parseLoadedTextSequence();
    setCurrentTaskCallback(initializeTextSequenceTask);
}
