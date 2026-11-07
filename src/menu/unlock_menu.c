#include "selection_tasks_internal.h"

void initializeUnlockMenuResourceTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuAnimation(state);
    state->unk8C = 0;
    configureTaskStateTransition(1, 0, 30);
    applySaveRecordUnlockOverrides(0);
    setCurrentTaskCallback(func_800A3D9C);
}

void resetUnlockMenuAnimation(UnlockMenuState *state) {
    s16 i;
    state->unk04[0] = state->unk20[0] = 0;
    for (i = 1; i < 7; i++) {
        state->unk04[i] = state->unk3C[i] = state->unk04[0];
        state->unk20[i] = state->unk58[i] = state->unk20[0];
    }
    state->unk3C[0] = state->unk04[0];
    state->unk58[0] = state->unk20[0];
    state->unk74[0] = 0;
    state->unk74[1] = 1;
    state->unk74[2] = 2;
    state->unk74[3] = 3;
}

void runTaskResource14Update(void) {
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {
        }
    }
    func_800A42CC();
}

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A3D9C);

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A42CC);

void finishUnlockMenuTask(void) {
    UnlockMenuState *state;
    s16 i;
    if (D_8013AE88->resource14 == 0xFFFF) { for (;;) {} }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    func_800A42CC();
    switch (D_8013AE88->data18) {
    case 0:
        for (i = 0; i < 5; i++) {
            state->unk58[i] = 0;
            state->unk20[i] = 0;
        }
        D_8013AE88->data18 = 1;
        break;
    case 1:
        for (i = 0; i < 5; i++) {
            state->unk20[i] += 0x4000;
            state->unk04[i] += state->unk20[i];
            state->unk58[i] += 0x4000;
            state->unk3C[i] += state->unk58[i];
        }
        if (state->unk04[0] > 0x168000) {
            if ((u16)state->unk8C != 0) {
                setSecondaryDisplayTaskMode(3);
                queueMode11TransitionResources();
                queueBackgroundTextureSectorCommands();
                D_8013B10C = -1;
                func_8002FA58(2);
            }
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        if (isCdCommandQueueIdle() == 1) {
            configureTaskStateTransition(2, 0, 10);
            D_8013AE88->data18 = 3;
        }
        break;
    case 3:
        if (func_800707F0(0) == 0) { func_800149A8(); }
        break;
    }
}

void initializeUnlockMenuTask(void) {
    UnlockMenuState *state;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    state->selection = 0;
    state->previousSelection = 0;
    resetUnlockMenuOffsets(state);
    func_800A4C28();
    setCurrentTaskCallback(func_800A499C);
}

void resetUnlockMenuOffsets(UnlockMenuState *state) {
    s16 i;
    for (i = 0; i < 6; i++) {
        i[state->unk3C] = 0;
        i[state->unk04] = 0;
        i[state->unk58] = 0;
        i[state->unk20] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A499C);

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A4C28);

s32 hasUnlockNotificationFlag(s16 index) {
    if (index == 11) {
        index = 10;
    }
    return (s16)getUnlockNotificationFlag(index) != 0;
}

void startUnlockActionAfterCountdown(void) {
    if (--D_8013AE88->data18 == 0) {
        if (D_8013AF3A < 11) {
            func_80014874(func_800A624C);
        } else {
            func_80014874(startUnlockResourceLoad);
        }
        setCurrentTaskCallback(prepareUnlockSelectionResume);
        D_8013AE88->unk10 |= 1;
    }
}

void animateUnlockMenuTransition(void) {
    UnlockMenuState *state;
    s16 i;
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (UnlockMenuState *)&D_800D9504[D_8013AE88->resource14];
    func_800A4C28();
    switch (D_8013AE88->data18) {
    case 0:
        for (i = 0; i < 6; i++) {
            state->unk58[i] = 0;
            state->unk20[i] = 0;
        }
        D_8013AE88->data18 = 1;
        break;
    case 1:
        for (i = 0; i < 6; i++) {
            state->unk20[i] += 0x4000;
            state->unk04[i] += state->unk20[i];
            state->unk58[i] += 0x4000;
            state->unk3C[i] += state->unk58[i];
        }
        if (state->unk04[0] > 0x168000) {
            D_8013AE88->data18 = 2;
        }
        break;
    case 2:
        state->selection = 5;
        state->previousSelection = 5;
        resetUnlockMenuAnimation(state);
        setCurrentTaskCallback(func_800A3D9C);
        break;
    }
}

void resumeUnlockSelectionTask(void) {
    setSecondaryDisplayTaskMode(0);
    setCurrentTaskCallback(func_800A499C);
}

void prepareUnlockSelectionResume(void) {
    setSecondaryDisplayTaskMode(3);
    D_8013B10C = -1;
    func_8002FA58(2);
    setCurrentTaskCallback(resumeUnlockSelectionTask);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/unlock_menu", func_800A525C);

void startUnlockResourceLoad(void) {
    resetPendingCdRequests();
    D_8013AFE4 &= ~0x100;
    createResourceLoadTask();
    setCurrentTaskCallback(func_800A525C);
}
