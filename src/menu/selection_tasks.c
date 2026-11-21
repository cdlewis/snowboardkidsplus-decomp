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
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

typedef struct NumberSelectionSpriteTable {
    GridSpriteEntry entries[3];
    u16 padding;
} NumberSelectionSpriteTable;

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
        func_80051A68();
    }
    initializeRacePlayerEffects();
    taskTransitionFlagGp = 0;
    createChildTaskWithReserve(initializeModeSectorReadResourcesTask);
    if (D_8013AFCD == 2) {
        D_8013AFAA |= 0x800;
        func_800148F4(&gGameTasks[3], initializeTaskPlayerCamera);
        D_8013AE8C->data18 = 1;
    }
    func_800148F4(&gGameTasks[3], initializeTaskPlayerCamera);
    D_8013AE8C->data18 = 0;
    func_80058BD4();
    createChildTask(updateRacePlayerFinishSlotsTask);
    for (player = D_8013AFD2 - 1; player >= 0; player--) {
        createChildTask(initializeCurrentPlayerModelAndHeight);
        D_8013AE8C->data18 = player;
        gRacePlayers[player].task = D_8013AE8C;
    }
    configureTaskStateTransition(2, 0, 15);

    D_8013AFDB = 0;
    clip = D_800EC158;
    width = D_8013AF7A;
    clip[0].x = 0;
    clip[0].y = 0;
    clip[0].w = width;
    /* Preserve the separate height reads for the full and player clips. */
    clip[0].h = *(volatile u16 *)&D_8013AF7C;
    if (D_8013AFCD == 1) {
        clip[1].x = 0;
        clip[1].y = 0;
        clip[1].w = width;
        D_8013AF7E = 0;
        height = D_8013AF7C;
        clip[1].h = height / 2 + 16;
        sixthHeight = (u32)height / 6;
    } else {
        D_8013AF7E = 0;
        clip[1].x = 0;
        clip[1].y = 0;
        clip[1].w = width;
        clip[2].x = 0;
        clip[2].w = width;
        height = D_8013AF7C;
        clip[1].h = height / 2 + 16;
        clip[2].y = height / 2 + 16;
        clip[2].h = height / 2 - 16;
        sixthHeight = (u32)height / 6;
    }
    D_8013AF80 = sixthHeight;
    initializeRacePaletteCycles();
    player = 0;
    if (D_8013AFCD) {
        do {
            copyPlayerPaletteRow(player);
            player++;
        } while (player < D_8013AFCD);
    }
    D_8013AFAA |= 0x48;
    if (D_8013AFD8) {
        setCurrentTaskCallback(advanceTaskAtModeCoursePosition);
    } else {
        setCurrentTaskCallback(advanceTaskWhenFlagB55ESet);
    }
}

void advanceTaskAtModeCoursePosition(void) {
    s16 reached = 0;

    switch (D_8013AFD8) {
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
        D_8013AFAA = (D_8013AFAA | 0x1090) & ~0x2000;
        resetPendingCdRequests();
        resetPaletteRecords();
        func_800149E0();
        D_8013AFD3 = 0;
        D_8013AFAA |= 0x48;
        switch (D_8013AFD8) {
        case 10:
            taskTransitionFlagGp = 0;
            createChildTaskWithReserve(initializeModeSectorReadResourcesTask);
            D_8013AE88->unk10 |= 1;
            break;
        case 11:
            D_8013AFAA |= 0x48;
            D_8013B10C = -1;
            D_8013B10E = -1;
            D_8013B106 = 0;
            D_8013AFD8 = 0;
            D_8013B562 = 0;
            SetFogNearFar(5000, 6000, 400);
            setCurrentTaskCallback(func_800A24EC);
            D_8013B108 = 2;
            return;
        default:
            createChildTask(initializeMode1RacePlayers);
            setCurrentTaskCallback(selectTaskBySelectedRecordMatch);
            D_8013AE88->unk10 |= 1;
            break;
        }
    }
}

void selectTaskBySelectedRecordMatch(void) {
    if (D_8013B560 != D_8013AFCE) {
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

INCLUDE_ASM("asm/nonmatchings/menu/selection_tasks", func_800A24EC);

void initializeNumberSelectionTask(void) {
    NumberPanelState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (NumberPanelState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    D_8013AFD3 = 1;
    state->number = D_8013AFD8 - 1;
    D_8013AE88->data18 = 0;
    setControllerFlag5(0);
    setControllerFlag5(1);
    resetPendingCdRequests();
    createChildTaskWithReserve(func_800A2C40);
    setCurrentTaskCallback(updateNumberSelectionTask);
}

void updateNumberSelectionTask(void) {
    NumberPanelState *state;
    RegisteredTaskEntry *entries;
    s32 complete;
    s16 selection;

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (NumberPanelState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        for (;;) {}
    }
    entries = (RegisteredTaskEntry *)&D_800D9504[D_8013AE88->resource16];

    switch (D_8013AE88->data18) {
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
        D_8013AE88->data18 = 1;
        break;
    case 1:
        /* This task tests the registered flags as a signed status. */
        if (*(s16 *)&entries[1].flags == 0) {
            entries[1].command = 20;
            configureColorCycleTask(1, 0, 128, 8, 0);
            D_8013B41F = 0;
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        complete = isTaskResourceTransitionComplete(0);
        if (complete == 1 && D_8013B41F == 0) {
            entries[1].command = 21;
            configureColorCycleTask(2, 64, 128, 4, 0);
            D_8013B41F = complete;
        }
        if (state->choiceResult != 0 && *(s16 *)&entries[1].flags == 0) {
            entries[1].command = 23;
            configureColorCycleTask(0, 0, 0, 0, 0);
            D_8013AE88->data18 = 4;
        }
        break;
    case 4:
        if (state->number == 8) {
            selection = state->selectedIndex + 1;
        } else {
            selection = state->selectedIndex;
        }
        if (selection == 0) {
            D_8013AFD8++;
            D_8013AE88->data18 = 6;
        } else {
            if (selection != 1) {
                D_8013AFD8 = state->number == 8 ? D_8013AFD8 + 1 : 11;
            }
            D_8013AE88->data18 = 6;
        }
        break;
    case 6:
        configureTaskStateTransition(7, 0, 30);
        D_8013AE88->data18 = 7;
        break;
    case 7:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013AE88->data18 = 8;
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
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    drawDecimalNumber8x16(23, -39, 1, 0, state->number + 1, 5, -1, ot, 2);
    func_8006C998(-40, -36, 404, 8, -1, ot, 2);
    drawDefaultTexturedPanel(-56, -48, 112, 96, 5, 0, ot, 2);
    func_80070C5C(128, ot, 2);
}

void drawOffsetSelectedTaskSprite(void) {
    SelectedSpriteResource *state;
    SelectedSpriteChild *children;
    Movement2D *position;
    u16 mode;
    if (D_8013AE88->resource14 == 0xFFFF) {
        while (1) {}
    }
    state = (SelectedSpriteResource *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 == 0xFFFF) {
        while (1) {}
    }
    children = (SelectedSpriteChild *)&D_800D9504[D_8013AE88->resource16];
    mode = state->mode;
    if ((s16)mode > 0) {
        if ((u32)(mode - 1) < 2) {
            setSpriteColor(state->r, state->g, state->b);
        } else {
            setSpriteColor(128, 128, 128);
        }
        position = (Movement2D *)&D_800D9504[children[state->selectedIndex].task->resource14];
        func_8006C998(position->x / 4096 - 8, position->y / 4096 + 2, state->sprite, 0, -1,
                     &D_800EBFF0[D_8013AF78], state->depth - 1);
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/selection_tasks", func_800A2C40);

void initializeSelectionMenuTask(void) {
    SelectionMenuState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (SelectionMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->previousSelection = state->selection = D_8013AFD9 - 1;
    state->unk04 = 0;
    state->unk06 = 16;
    state->unk08 = 0;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013AFCE = -1;
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

    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (SelectionMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->unk0A = 0;

    if (D_8013AEE8[0] & 0x8000) {
        selection = (u16)state->selection;
        if ((u16)(selection - 1) < 4 || (u32)(selection - 6) < 5) {
            state->selection = selection - 1;
        }
    }
    if (D_8013AEE8[0] & 0x2000) {
        selection = (u16)state->selection;
        if (selection < 4 || (u32)(selection - 5) < 5) {
            state->selection = selection + 1;
        }
    }
    if (D_8013AEE8[0] & 0x1000) {
        selection = (u16)state->selection;
        if ((u16)(selection - 5) < 3) {
            state->selection = selection - 5;
        } else if ((u32)(selection - 8) < 3) {
            state->selection = selection - 6;
        } else if ((s16)selection == 11) {
            state->selection = 8;
        }
    }
    if (D_8013AEE8[0] & 0x4000) {
        selection = (u16)state->selection;
        if (selection < 3) {
            state->selection = selection + 5;
        } else if ((u32)(selection - 2) < 3) {
            state->selection = selection + 6;
        } else if ((u32)(selection - 5) < 6) {
            state->selection = 11;
        }
    }
    if (D_8013AEE8[0] & 0x40) {
        state->unk0A = 1;
    }
    if (state->selection != state->previousSelection) {
        playSoundEffect(0, 0x100, 0x20);
        state->previousSelection = state->selection;
    }
    if (state->selection == 11) {
        if (D_8013AEE8[0] & 0xB0) {
            state->unk0A = 1;
            state->unk08 = 20;
        }
    } else {
        if (D_8013AEE8[0] & 0xB0) {
            D_8013AFD9 = state->selection + 1;
            playSoundEffect(0, 0, 0x20);
            state->unk08 = 20;
            D_8013B0F0 = 0;
            D_8013AE88->data18 = 0;
            setCurrentTaskCallback(finishSelectionMenuTask);
        }
    }
    drawPlaybackSelectionMenu(state);
    if (state->unk0A) {
        playSoundEffect(0, 0, 0x20);
        D_8013AFD9 = 0;
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishSelectionMenuTask);
    }
}

void finishSelectionMenuTask(void) {
    SelectionMenuState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (SelectionMenuState *)&D_800D9504[D_8013AE88->resource14];
    switch (D_8013AE88->data18) {
    case 0:
        if (state->unk08 == 0) {
            configureTaskStateTransition(2, 0, 30);
            startCdVolumeFade(103, 20, 0, 1);
            D_8013AE88->data18 = 1;
        }
        break;
    case 1:
        if (isTaskStateTransitionBelowMaximum(0) == 0) {
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        configureColorCycleTask(0, 0, 0, 0, 0);
        configureColorCycleTask(0, 0, 0, 0, 1);
        if (state->unk0A != 0) {
            D_8013B108 = 2;
            D_8013B109 = 1;
            deleteCurrentTaskSubtree();
        } else {
            queueStartupSectorReads();
            setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
            D_8013AE88->unk10 |= 2;
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
    func_8006EF30(-96, -88, 14, gPlaybackSelectionMenuInstructions, 13, -1, &D_800EBFF0[D_8013AF78], 4);
    drawDefaultTexturedPanel(-120, -96, 240, 48, 5, 0, &D_800EBFF0[D_8013AF78], 4);

    for (i = 0; i < 5; i++) {
        highlight = 0;
        if (state->selection == i) {
            highlight = state->unk04;
        }
        x = i * 48;
        drawMenuKindSprite(x - 120, -28, i + 1, 1, &D_800EBFF0[D_8013AF78], 4);
        func_8007240C(x - 122, -30, 36, 36, highlight + 128, highlight + 16, 16,
                      -1, &D_800EBFF0[D_8013AF78], 4);
        func_8007240C(x - 114, -22, 36, 36, highlight / 4 + 64, 8, 8,
                      -1, &D_800EBFF0[D_8013AF78], 4);
    }

    for (i = 0; i < 6; i++) {
        highlight = 0;
        if (state->selection == i + 5) {
            highlight = state->unk04;
        }
        x = i * 48;
        drawPaletteMappedIcon(x - 144, 28, i + 1, &D_800EBFF0[D_8013AF78], 4);
        func_8007240C(x - 146, 26, 36, 36, 16, highlight + 16, highlight + 128,
                      -1, &D_800EBFF0[D_8013AF78], 4);
        func_8007240C(x - 138, 34, 36, 36, 8, 8, highlight / 4 + 64,
                      -1, &D_800EBFF0[D_8013AF78], 4);
    }

    if (state->selection == 11) {
        highlight = state->unk04 + 32;
        setSpriteColor((u8)highlight, (u8)highlight, (u8)highlight);
        func_8006C998(-56, 84, 403, 0, -1, &D_800EBFF0[D_8013AF78], 4);
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
    func_8006C998(-56, 84, 402, highlight, -1, &D_800EBFF0[D_8013AF78], 4);
    func_80070C5C(128, &D_800EBFF0[D_8013AF78], 4);
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
    {{405, 9, {0, 0, 0}}, {406, 9, {0, 0, 0}}, {407, 9, {0, 0, 0}}},
    0,
};

const u8 gNumberSelectionMapping[4]
    __attribute__((section(".data.number_selection_mapping"), aligned(4))) = {0, 1, 2, 0};

const u8 gNumberSelectionFinalMapping[2]
    __attribute__((section(".data.number_selection_final_mapping"), aligned(2))) = {1, 2};

/* Preserve the padding after the adjacent assembly data block. */
const u16 gNumberSelectionTailPadding
    __attribute__((section(".data.number_selection_tail_padding"))) = 0;
