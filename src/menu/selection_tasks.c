#include "selection_tasks_internal.h"
#include "game/race/ui/race_hud.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/menu/movement.h"
#include "game/menu/selected_records.h"
#include "game/menu/selected_sprite.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_objects.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

typedef struct NumberSelectionSpriteTable {
    GridSpriteEntry entries[3];
    u16 padding;
} NumberSelectionSpriteTable;

extern u8 gModeSequenceStage __attribute__((section(".data")));
extern u8 gControllerPlaybackIndex __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u8 D_8013AFDB __attribute__((section(".data")));
extern u16 gDisplayWidth __attribute__((section(".data")));
extern u16 gDisplayHeight __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern s16 gPlayerViewWidthReduction __attribute__((section(".data")));
extern s16 gPlayerViewVerticalOffset __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gPauseTaskResumeAnchor __attribute__((section(".data")));

extern u8 gMenuScreenId __attribute__((section(".data")));
extern u8 gMenuTransitionState __attribute__((section(".data")));
extern s16 gMenuMusicEnabled __attribute__((section(".data")));
extern u8 gTitleMenuResourcesInitialized __attribute__((section(".data")));

u16 gSceneOneShotSoundKey __attribute__((section(".sdata.scene_sound_state"))) = 0;

u8 gSceneOneShotVoice __attribute__((section(".sdata.scene_sound_state"))) = 0;

/* Keep the table definitions after the call sites for address scheduling. */
extern const NumberSelectionSpriteTable gNumberSelectionSprites;
extern const u16 gPlaybackSelectionMenuInstructions[];
extern const u8 gNumberSelectionMapping[];
extern const u8 gNumberSelectionFinalMapping[];

void initializeModeRaceTasks(void) {
    s16 player;
    RECT *clip;
    u16 width;
    u16 height;
    u16 sixthHeight;

    if (!D_8013B562) {
        initializeCourseModels();
    }
    initializeRacePlayerEffects();
    taskTransitionFlagGp = 0;
    createChildTaskWithReserve(initializeModeSectorReadResourcesTask);
    if (playerCountAbsolute == 2) {
        gGameStateFlags |= 0x800;
        createChildTaskForParent(&gGameTasks[3], initializeTaskPlayerCamera);
        gCreatedTask->data18 = 1;
    }
    createChildTaskForParent(&gGameTasks[3], initializeTaskPlayerCamera);
    gCreatedTask->data18 = 0;
    initializeCourseObjects();
    createChildTask(updateRacePlayerFinishSlotsTask);
    for (player = gActiveRacePlayerCount - 1; player >= 0; player--) {
        createChildTask(initializeCurrentPlayerModelAndHeight);
        gCreatedTask->data18 = player;
        gRacePlayers[player].task = gCreatedTask;
    }
    configureTaskStateTransition(2, 0, 15);

    D_8013AFDB = 0;
    clip = gViewClipRects;
    width = gDisplayWidth;
    clip[0].x = 0;
    clip[0].y = 0;
    clip[0].w = width;
    /* Preserve the separate height reads for the full and player clips. */
    clip[0].h = *(volatile u16 *)&gDisplayHeight;
    if (playerCountAbsolute == 1) {
        clip[1].x = 0;
        clip[1].y = 0;
        clip[1].w = width;
        gPlayerViewWidthReduction = 0;
        height = gDisplayHeight;
        clip[1].h = height / 2 + 16;
        sixthHeight = (u32)height / 6;
    } else {
        gPlayerViewWidthReduction = 0;
        clip[1].x = 0;
        clip[1].y = 0;
        clip[1].w = width;
        clip[2].x = 0;
        clip[2].w = width;
        height = gDisplayHeight;
        clip[1].h = height / 2 + 16;
        clip[2].y = height / 2 + 16;
        clip[2].h = height / 2 - 16;
        sixthHeight = (u32)height / 6;
    }
    gPlayerViewVerticalOffset = sixthHeight;
    initializeRacePaletteCycles();
    player = 0;
    if (playerCountAbsolute) {
        do {
            copyPlayerPaletteRow(player);
            player++;
        } while (player < playerCountAbsolute);
    }
    gGameStateFlags |= 0x48;
    if (gModeSequenceStage) {
        setCurrentTaskCallback(advanceTaskAtModeCoursePosition);
    } else {
        setCurrentTaskCallback(advanceTaskWhenFlagB55ESet);
    }
}

void advanceTaskAtModeCoursePosition(void) {
    s16 reached = 0;

    switch (gModeSequenceStage) {
    case 1:
        if (gRacePlayers[0].unk92E == 80) reached = 1;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        if (gRacePlayers[0].unk92E == 156) reached = 1;
        break;
    case 6:
        if (gRacePlayers[0].unk92E == 180) reached = 1;
        break;
    case 7:
        if (gRacePlayers[0].unk92E == 22) reached = 1;
        break;
    case 8:
        if (gRacePlayers[0].unk92E == 29) reached = 1;
        break;
    case 9:
        if (gRacePlayers[0].unk92E == 54) reached = 1;
        break;
    }
    updateRaceHud();
    if (reached) {
        stopPlayerSound(0);
        stopPlayerSound(1);
        startCdVolumeFade(103, 20, 0, 1);
        taskTransitionFlagGp = 0;
        createChildTaskWithReserve(initializeNumberSelectionTask);
        setCurrentTaskCallback(completeModeSelectionTransitionWhenReady);
    }
}

void completeModeSelectionTransitionWhenReady(void) {
    if (taskTransitionFlagGp != 0) {
        gGameStateFlags = (gGameStateFlags | 0x1090) & ~0x2000;
        resetPendingCdRequests();
        resetPaletteRecords();
        deleteCurrentTaskDescendants();
        gRacePauseState = 0;
        gGameStateFlags |= 0x48;
        switch (gModeSequenceStage) {
        case 10:
            taskTransitionFlagGp = 0;
            createChildTaskWithReserve(initializeModeSectorReadResourcesTask);
            gCurrentTask->unk10 |= 1;
            break;
        case 11:
            gGameStateFlags |= 0x48;
            gMenuTextureResourceId = -1;
            gMenuGridResourceId = -1;
            gMenuMusicEnabled = 0;
            gModeSequenceStage = 0;
            D_8013B562 = 0;
            SetFogNearFar(5000, 6000, 400);
            setCurrentTaskCallback(scheduleMode1TaskTransition);
            gMenuScreenId = 2;
            return;
        default:
            createChildTask(initializeMode1RacePlayers);
            setCurrentTaskCallback(selectTaskBySelectedRecordMatch);
            gCurrentTask->unk10 |= 1;
            break;
        }
    }
}

void selectTaskBySelectedRecordMatch(void) {
    if (D_8013B560 != gSelectedCourseIndex) {
        D_8013B562 = 0;
        setCurrentTaskCallback(queueRaceStartupResourcesTask);
    } else {
        D_8013B562 = 1;
        setCurrentTaskCallback(initializeModeRaceTasks);
    }
}

void startMode1TaskTransition(void) {
    setSecondaryDisplayTaskMode(1);
    configureTaskStateTransition(1, 0, 30);
    setCurrentTaskCallback(initializeGameOptionViewTask);
}

void scheduleMode1TaskTransition(void) {
    setCurrentTaskCallback(startMode1TaskTransition);
}

void initializeNumberSelectionTask(void) {
    NumberPanelState *state;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    state = (NumberPanelState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource16 = allocateTaskMemoryBlock();
    gRacePauseState = 1;
    state->number = gModeSequenceStage - 1;
    gCurrentTask->data18 = 0;
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetPendingCdRequests();
    createChildTaskWithReserve(skipRemainingRaceTasksDuringNumberSelection);
    setCurrentTaskCallback(updateNumberSelectionTask);
}

void updateNumberSelectionTask(void) {
    NumberPanelState *state;
    RegisteredTaskEntry *entries;
    s32 complete;
    s16 selection;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberPanelState *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&gTaskMemoryPool[gCurrentTask->resource16];

    switch (gCurrentTask->data18) {
    case 0:
        state->mode = 0;
        state->choiceResult = 0;
        if (state->number == 8) {
            state->selectedIndex = 1;
            state->previousIndex = 1;
            createRegisteredSpacedChildTasks(1, 0xB0, 0,
                                             (const u8 *)&gNumberSelectionSprites, gNumberSelectionFinalMapping, NULL,
                                             0, 0, 20, -40, 4, 0, 20, 2, 403,
                                             drawOffsetSelectedTaskSprite,
                                             drawRegisteredTaskGridSpriteWithColor, 1, 11);
        } else {
            state->selectedIndex = 0;
            state->previousIndex = 0;
            createRegisteredSpacedChildTasks(1, 0xB0, 0,
                                             (const u8 *)&gNumberSelectionSprites, gNumberSelectionMapping, NULL,
                                             0, 0, 20, -40, -16, 0, 20, 3, 403,
                                             drawOffsetSelectedTaskSprite,
                                             drawRegisteredTaskGridSpriteWithColor, 1, 11);
        }
        gCurrentTask->data18 = 1;
        break;
    case 1:
        /* This task tests the registered flags as a signed status. */
        if (*(s16 *)&entries[1].flags == 0) {
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
        if (state->choiceResult != 0 && *(s16 *)&entries[1].flags == 0) {
            entries[1].command = 23;
            configureColorCycleTask(0, 0, 0, 0, 0);
            gCurrentTask->data18 = 4;
        }
        break;
    case 4:
        if (state->number == 8) {
            selection = state->selectedIndex + 1;
        } else {
            selection = state->selectedIndex;
        }
        if (selection == 0) {
            gModeSequenceStage++;
            gCurrentTask->data18 = 6;
        } else {
            if (selection != 1) {
                gModeSequenceStage = state->number == 8 ? gModeSequenceStage + 1 : 11;
            }
            gCurrentTask->data18 = 6;
        }
        break;
    case 6:
        configureTaskStateTransition(7, 0, 30);
        gCurrentTask->data18 = 7;
        break;
    case 7:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            gCurrentTask->data18 = 8;
        }
        break;
    case 8:
    default:
        taskTransitionFlagGp = 1;
        deleteCurrentTaskSubtree();
        return;
    }
    drawNumberSelectionPanel(state);
}

void drawNumberSelectionPanel(NumberPanelState *state) {
    GsOT *ot = &gMainOrderingTables[gActiveGraphicsBuffer];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawDecimalNumber8x16(23, -39, 1, 0, state->number + 1, 5, -1, ot, 2);
    drawSpriteByIndexWithBlend(-40, -36, 404, 8, -1, ot, 2);
    drawDefaultTexturedPanel(-56, -48, 112, 96, 5, 0, ot, 2);
    drawScreenDarkeningOverlay(128, ot, 2);
}

void drawOffsetSelectedTaskSprite(void) {
    SelectedSpriteResource *state;
    SelectedSpriteChild *children;
    Movement2D *position;
    u16 mode;
    if (gCurrentTask->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = (SelectedSpriteResource *)&gTaskMemoryPool[gCurrentTask->resource14];
    if (gCurrentTask->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (SelectedSpriteChild *)&gTaskMemoryPool[gCurrentTask->resource16];
    mode = state->mode;
    if ((s16)mode > 0) {
        if ((u32)(mode - 1) < 2) {
            setSpriteColor(state->r, state->g, state->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&gTaskMemoryPool[children[state->selectedIndex].task->resource14];
        drawSpriteByIndexWithBlend(position->x / 4096 - 8, position->y / 4096 + 2, state->sprite, 0, -1,
                     &gMainOrderingTables[gActiveGraphicsBuffer], state->depth - 1);
    }
}

void skipRemainingRaceTasksDuringNumberSelection(void) {
    if (gRacePauseState) {
        gCurrentTask = gPauseTaskResumeAnchor->previous;
    }
}

void initializeSelectionMenuTask(void) {
    SelectionMenuState *state;
    if (gCurrentTask->resource14 != 0xFFFF) {
        for (;;) {}
    }
    gCurrentTask->resource14 = allocateTaskMemoryBlock();
    state = (SelectionMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    state->previousSelection = state->selection = gControllerPlaybackIndex - 1;
    state->unk04 = 0;
    state->unk06 = 16;
    state->unk08 = 0;
    setControllerFlag5(0);
    setControllerFlag5(1);
    gSelectedCourseIndex = -1;
    selectionMenuRecordAbsolute = 99;
    setSecondaryDisplayTaskMode(0);
    configureTaskStateTransition(1, 0, 30);
    if (D_8013B564 == 0) {
        queueCdCommand(3, 0);
        D_8013B564 = 1;
    }
    setCurrentTaskCallback(updateSelectionMenuTask);
}

void updateSelectionMenuTask(void) {
    SelectionMenuState *state;
    u32 selection;

    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (SelectionMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    state->unk0A = 0;

    if (gControllerPressedButtons[0] & 0x8000) {
        selection = (u16)state->selection;
        if ((u16)(selection - 1) < 4 || (u32)(selection - 6) < 5) {
            state->selection = selection - 1;
        }
    }
    if (gControllerPressedButtons[0] & 0x2000) {
        selection = (u16)state->selection;
        if (selection < 4 || (u32)(selection - 5) < 5) {
            state->selection = selection + 1;
        }
    }
    if (gControllerPressedButtons[0] & 0x1000) {
        selection = (u16)state->selection;
        if ((u16)(selection - 5) < 3) {
            state->selection = selection - 5;
        } else if ((u32)(selection - 8) < 3) {
            state->selection = selection - 6;
        } else if ((s16)selection == 11) {
            state->selection = 8;
        }
    }
    if (gControllerPressedButtons[0] & 0x4000) {
        selection = (u16)state->selection;
        if (selection < 3) {
            state->selection = selection + 5;
        } else if ((u32)(selection - 2) < 3) {
            state->selection = selection + 6;
        } else if ((u32)(selection - 5) < 6) {
            state->selection = 11;
        }
    }
    if (gControllerPressedButtons[0] & 0x40) {
        state->unk0A = 1;
    }
    if (state->selection != state->previousSelection) {
        playSoundEffect(0, 0x100, 0x20);
        state->previousSelection = state->selection;
    }
    if (state->selection == 11) {
        if (gControllerPressedButtons[0] & 0xB0) {
            state->unk0A = 1;
            state->unk08 = 20;
        }
    } else {
        if (gControllerPressedButtons[0] & 0xB0) {
            gControllerPlaybackIndex = state->selection + 1;
            playSoundEffect(0, 0, 0x20);
            state->unk08 = 20;
            gTitleMenuResourcesInitialized = 0;
            gCurrentTask->data18 = 0;
            setCurrentTaskCallback(finishSelectionMenuTask);
        }
    }
    drawPlaybackSelectionMenu(state);
    if (state->unk0A) {
        playSoundEffect(0, 0, 0x20);
        gControllerPlaybackIndex = 0;
        gCurrentTask->data18 = 0;
        setCurrentTaskCallback(finishSelectionMenuTask);
    }
}

void finishSelectionMenuTask(void) {
    SelectionMenuState *state;
    if (gCurrentTask->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (SelectionMenuState *)&gTaskMemoryPool[gCurrentTask->resource14];
    switch (gCurrentTask->data18) {
    case 0:
        if (state->unk08 == 0) {
            configureTaskStateTransition(2, 0, 30);
            startCdVolumeFade(103, 20, 0, 1);
            gCurrentTask->data18 = 1;
        }
        break;
    case 1:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            gCurrentTask->data18 = 2;
        }
        break;
    case 2:
        configureColorCycleTask(0, 0, 0, 0, 0);
        configureColorCycleTask(0, 0, 0, 0, 1);
        if (state->unk0A != 0) {
            gMenuScreenId = 2;
            gMenuTransitionState = 1;
            deleteCurrentTaskSubtree();
        } else {
            queueStartupSectorReads();
            setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
            gCurrentTask->unk10 |= 2;
        }
        return;
    }
    drawPlaybackSelectionMenu(state);
}

void drawPlaybackSelectionMenu(SelectionMenuState *state) {
    s16 i;
    s16 highlight;
    s16 x;

    if (state->unk08 != 0) {
        if (state->unk08 & 1) {
            state->unk04 = 96;
        } else {
            state->unk04 = 0;
        }
        state->unk08--;
    } else {
        state->unk04 += state->unk06;
        if (state->unk04 < 0) {
            state->unk04 = 0;
            state->unk06 = -state->unk06;
        } else if (state->unk04 > 96) {
            state->unk04 = 96;
            state->unk06 = -state->unk06;
        }
    }

    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawWrappedGlyphText16x16(-96, -88, 14, gPlaybackSelectionMenuInstructions, 13, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    drawDefaultTexturedPanel(-120, -96, 240, 48, 5, 0, &gMainOrderingTables[gActiveGraphicsBuffer], 4);

    for (i = 0; i < 5; i++) {
        highlight = 0;
        if (state->selection == i) {
            highlight = state->unk04;
        }
        x = i * 48;
        drawMenuKindSprite(x - 120, -28, i + 1, 1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        addTilePacketWithBlend(x - 122, -30, 36, 36, highlight + 128, highlight + 16, 16,
                      -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        addTilePacketWithBlend(x - 114, -22, 36, 36, highlight / 4 + 64, 8, 8,
                      -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }

    for (i = 0; i < 6; i++) {
        highlight = 0;
        if (state->selection == i + 5) {
            highlight = state->unk04;
        }
        x = i * 48;
        drawPaletteMappedIcon(x - 144, 28, i + 1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        addTilePacketWithBlend(x - 146, 26, 36, 36, 16, highlight + 16, highlight + 128,
                      -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        addTilePacketWithBlend(x - 138, 34, 36, 36, 8, 8, highlight / 4 + 64,
                      -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    }

    if (state->selection == 11) {
        highlight = state->unk04 + 32;
        setSpriteColor((u8)highlight, (u8)highlight, (u8)highlight);
        drawSpriteByIndexWithBlend(-56, 84, 403, 0, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
        highlight = 0;
        if (state->unk06 > 0) {
            state->unk06 = 8;
        } else {
            state->unk06 = -8;
        }
    } else {
        highlight = 2;
        if (state->unk06 > 0) {
            state->unk06 = 16;
        } else {
            state->unk06 = -16;
        }
    }
    setSpriteColor(128, 128, 128);
    drawSpriteByIndexWithBlend(-56, 84, 402, highlight, -1, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    drawScreenDarkeningOverlay(128, &gMainOrderingTables[gActiveGraphicsBuffer], 4);
    drawFullScreenTexturedBackground();
}

const u16 gPlaybackSelectionMenuInstructions[]
    __attribute__((section(".data.playback_selection_menu_instructions"), aligned(4))) = {
    0x00E7, 0x00E8, 0x0049, 0x002C, 0x00CE, 0x00E3, 0x00C0, 0x00CE,
    0x0039, 0x800A, 0x015A, 0x0084, 0x0052, 0x0070, 0x00BF, 0x0097,
    0x009E, 0x00A5, 0x009A, 0x00CB, 0x009D, 0x0094, 0x0033, 0,
};

const NumberSelectionSpriteTable gNumberSelectionSprites
    __attribute__((section(".data.number_selection_sprite_table"), aligned(4))) = {
    {{405, 9, 0, 0}, {406, 9, 0, 0}, {407, 9, 0, 0}},
    0,
};

const u8 gNumberSelectionMapping[4]
    __attribute__((section(".data.number_selection_mapping"), aligned(4))) = {0, 1, 2, 0};

const u8 gNumberSelectionFinalMapping[2]
    __attribute__((section(".data.number_selection_final_mapping"), aligned(2))) = {1, 2};

/* Preserve the padding after the adjacent assembly data block. */
const u16 gNumberSelectionTailPadding
    __attribute__((section(".data.number_selection_tail_padding"))) = 0;
