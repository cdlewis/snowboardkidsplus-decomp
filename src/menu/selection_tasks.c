#include "selection_tasks_internal.h"

u16 gTextOneShotSoundKey __attribute__((section(".sdata.text_sound_state"))) = 0;

u8 gTextOneShotVoice __attribute__((section(".sdata.text_sound_state"))) = 0;

INCLUDE_ASM("asm/nonmatchings/menu/selection_tasks", func_800A1F50);

void advanceTaskAtModeCoursePosition(void) {
    s16 reached = 0;

    switch (D_8013AFD8) {
    case 1:
        if (D_80120E90[0].unk92E == 80) reached = 1;
        break;
    case 2:
    case 3:
    case 4:
    case 5:
        if (D_80120E90[0].unk92E == 156) reached = 1;
        break;
    case 6:
        if (D_80120E90[0].unk92E == 180) reached = 1;
        break;
    case 7:
        if (D_80120E90[0].unk92E == 22) reached = 1;
        break;
    case 8:
        if (D_80120E90[0].unk92E == 29) reached = 1;
        break;
    case 9:
        if (D_80120E90[0].unk92E == 54) reached = 1;
        break;
    }
    func_80040D5C();
    if (reached) {
        stopPlayerSound(0);
        stopPlayerSound(1);
        startCdVolumeFade(103, 20, 0, 1);
        taskTransitionFlagGp = 0;
        func_80014874(initializeNumberSelectionTask);
        setCurrentTaskCallback(completeModeSelectionTransitionWhenReady);
    }
}

void completeModeSelectionTransitionWhenReady(void) {
    if (modeTransitionReadyGp != 0) {
        D_8013AFAA = (D_8013AFAA | 0x1090) & ~0x2000;
        resetPendingCdRequests();
        resetPaletteRecords();
        func_800149E0();
        D_8013AFD3 = 0;
        D_8013AFAA |= 0x48;
        switch (D_8013AFD8) {
        case 10:
            modeTransitionReadyGp = 0;
            func_80014874(initializeModeSectorReadResourcesTask);
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
            func_80014838(initializeMode1RacePlayers);
            setCurrentTaskCallback(selectTaskBySelectedRecordMatch);
            D_8013AE88->unk10 |= 1;
            break;
        }
    }
}

void selectTaskBySelectedRecordMatch(void) {
    if (D_8013B560 != D_8013AFCE) {
        D_8013B562 = 0;
        setCurrentTaskCallback(func_8001B858);
    } else {
        D_8013B562 = 1;
        setCurrentTaskCallback(func_800A1F50);
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
    func_80014874(func_800A2C40);
    setCurrentTaskCallback(func_800A25F8);
}

INCLUDE_ASM("asm/nonmatchings/menu/selection_tasks", func_800A25F8);

void drawNumberSelectionPanel(NumberPanelState *state) {
    GsOT *ot = &D_800EBFF0[D_8013AF78];
    setSpriteColor(128, 128, 128);
    setTexturedQuadColor(128, 128, 128);
    func_8006D984(23, -39, 1, 0, state->number + 1, 5, -1, ot, 2);
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
    setCurrentTaskCallback(func_800A2D68);
}

INCLUDE_ASM("asm/nonmatchings/menu/selection_tasks", func_800A2D68);

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
        if (func_800707F0(0) == 0) {
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        configureColorCycleTask(0, 0, 0, 0, 0);
        configureColorCycleTask(0, 0, 0, 0, 1);
        if (state->unk0A != 0) {
            D_8013B108 = 2;
            D_8013B109 = 1;
            func_800149A8();
        } else {
            queueStartupSectorReads();
            setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
            D_8013AE88->unk10 |= 2;
        }
        return;
    }
    func_800A31C0(state);
}

INCLUDE_ASM("asm/nonmatchings/menu/selection_tasks", func_800A31C0);
