#include "panel_tasks_internal.h"

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
    setCurrentTaskCallback(func_8009BDB8);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009BDB8);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009C568);

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
    setCurrentTaskCallback(func_8009CA90);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009CA90);

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
    setCurrentTaskCallback(func_8009D354);
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009D354);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009DB70);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009DEF0);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009E468);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009E848);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009ECCC);

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
    setCurrentTaskCallback(func_8009F224);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009F224);

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
    setCurrentTaskCallback(func_8009F9B8);
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_8009F9B8);

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_800A008C);

s16 findIdentityMappedIndexOrZero(ByteMappingDescriptor *descriptor, s16 value) {
    s16 result = 0;
    s16 i;

    for (i = 0; i < descriptor->count; i++) {
        if (value == (s16)func_8007373C(descriptor->mapping, value)) {
            result = i;
            break;
        }
    }
    return result;
}

void initializeTaskResourcePairWithOneTickTransition(void) {
    D_8013AE88->data18 = 0;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
    configureTaskStateTransition(2, 0, 1);
    setCurrentTaskCallback(func_800A05C0);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_800A05C0);

void selectTaskCallbackFromModeState(void) {
    if (D_8013AFD8 != 0) {
        setCurrentTaskCallback(initializeMode1RacePlayers);
    } else if (D_8013AFD9 != 0) {
        setCurrentTaskCallback(initializeControllerPlaybackRaceTask);
    }
}

void initializeMode1Task(void) {
    D_8013AFD8 = 1;
    D_8013AFCE = -1;
    D_8013B560 = 99;
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013B0F0 = 0;
    setCurrentTaskCallback(initializeMode1RacePlayers);
}

void initializeMode1RacePlayers(void) {
    s32 i;
    s16 mode;
    D_8013B560 = D_8013AFCE;
    if ((s16)D_8013AFD8 < 7 && D_8013AFD8 != 0) D_8013AFCE = 0;
    else D_8013AFCE = 9;
    SetFogNearFar(8000, 10000, 400);
    D_8013AFD3 = 0;
    D_8013AFD6 = 0;
    D_8013AFE1 = 0;
    D_8013AFD9 = 0;
    D_8013AFE2 = 0;
    D_8013AFE0 = 0;
    D_80120E90[0].unk008 = 0;
    D_80120E90[1].unk008 = 1;
    D_80120E90[2].unk008 = 1;
    D_80120E90[3].unk008 = 1;
    D_80120E90[0].characterId = 0;
    D_80120E90[1].characterId = 1;
    D_80120E90[2].characterId = 2;
    D_80120E90[3].characterId = 3;
    D_80120E90[0].unk015[1] = 0;
    D_80120E90[1].unk015[1] = 0;
    D_80120E90[2].unk015[1] = 0;
    D_80120E90[3].unk015[1] = 0;
    D_80120E90[0].unk015[2] = 3;
    D_80120E90[1].unk015[2] = 5;
    D_80120E90[2].unk015[2] = 6;
    D_80120E90[3].unk015[2] = 7;
    D_80120E90[0].unk025[3] = 0;
    D_80120E90[1].unk025[3] = 1;
    D_80120E90[2].unk025[3] = 2;
    D_80120E90[3].unk025[3] = 3;
    gameOptionPlayerModeAbsolute = 2;
    mode = D_8013AFD8;
    if (mode != 0 && (mode < 7 || mode == 9)) {
        D_80120E90[0].isActive = 1;
        D_80120E90[1].isActive = 0;
        D_80120E90[2].isActive = 0;
        D_80120E90[3].isActive = 0;
        D_8013AFD2 = 1;
    } else {
        D_80120E90[0].isActive = 1;
        D_80120E90[1].isActive = 1;
        D_80120E90[2].isActive = 1;
        D_80120E90[3].isActive = 1;
        D_8013AFD2 = 4;
    }
    D_8013AFD0 = 5;
    D_8013AFDE = 100;
    for (i = 0; i < 4; i++) {
        D_80120E90[i].unk025[1] = 0;
        D_80120E90[i].unk025[2] = 0;
        D_80120E90[i].unk01C = 0;
    }
    setCurrentTaskCallback(prepareRaceResourcesIfSelectionChanged);
}

void prepareRaceResourcesIfSelectionChanged(void) {
    configureTaskStateTransition(1, 0, 30);
    setSecondaryDisplayTaskMode(1);
    if (D_8013B560 != D_8013AFCE) {
        func_800180F4(2);
        func_800180F4(3);
        func_800180F4(4);
        func_800180F4(5);
        queueStartupSectorReads();
        setCurrentTaskCallback(preparePlayerResourcesTask);
        D_8013AE88->unk10 |= 2;
    } else {
        setCurrentTaskCallback(initializeRaceTaskState);
    }
}

void preparePlayerResourcesTask(void) {
    queuePlayerResourceReadsAndUploadTextures();
    setCurrentTaskCallback(initializeRaceTaskState);
    D_8013AE88->unk10 |= 2;
}

void initializeModeSectorReadResourcesTask(void) {
    ModeSectorReadState *state;
    if (D_8013AE88->resource14 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource14 = allocateTaskResource();
    state = (ModeSectorReadState *)&D_800D9504[D_8013AE88->resource14];
    if (D_8013AE88->resource16 != 0xFFFF) {
        for (;;) {}
    }
    D_8013AE88->resource16 = allocateTaskResource();
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
    if (D_8013AE88->resource14 == 0xFFFF) {
        for (;;) {}
    }
    state = (ModeSectorReadState *)&D_800D9504[D_8013AE88->resource14];
    if (state->mode == 1) {
        state->sectorIndex = D_8013AFD8 - 1;
    } else {
        state->sectorIndex = D_8013AFD9 - 1;
    }
    queueModeIndexedSectorRead(state->mode, state->sectorIndex);
    setCurrentTaskCallback(func_800A0EE8);
    D_8013AE88->unk10 |= 2;
}

INCLUDE_ASM("asm/nonmatchings/menu/resource_tasks", func_800A0EE8);
