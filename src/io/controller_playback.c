#include "../menu/selection_tasks_internal.h"

ControllerPlaybackEntry gControllerPlaybackEntries[12] __attribute__((section(".data.controller_playback_entries"))) = {
    {0, 0, 0, 0},
    {0, 0, 9037, 2},
    {0, 0, 9039, 2},
    {9, 0, 9041, 2},
    {0, 0, 9043, 2},
    {0, 0, 9045, 2},
    {4, 0, 9047, 2},
    {1, 0, 9049, 2},
    {3, 0, 9051, 2},
    {2, 0, 9053, 2},
    {9, 0, 9055, 2},
    {0, 0, 9057, 2},
};

s16 gControllerPlaybackDurations[12] __attribute__((section(".data.controller_playback_durations"))) = {
    900, 384, 560, 592, 613, 606, 400, 288, 288, 286, 205, 272
};

void initializeControllerPlaybackRaceTask(void) {
    s32 i = 0;
    RacePlayer *player;
    selectionMenuRecordAbsolute = D_8013AFCE;
    D_8013AFCE = gControllerPlaybackEntries[D_8013AFD9].course;
    player = D_80120E90;
    D_8013AFD3 = 0;
    D_8013AFD6 = 0;
    D_8013AFE1 = 0;
    D_8013AFD8 = 0;
    D_8013AFE2 = 0;
    D_8013AFE0 = 0;
    player[0].unk008 = 0;
    player[1].unk008 = 0;
    player[2].unk008 = 1;
    player[3].unk008 = 1;
    player[0].characterId = 0;
    player[1].characterId = 1;
    player[2].characterId = 2;
    player[3].characterId = 3;
    player[0].unk015[1] = 0;
    player[1].unk015[1] = 0;
    player[2].unk015[1] = 0;
    player[3].unk015[1] = 0;
    player[0].unk015[2] = 3;
    player[1].unk015[2] = 5;
    player[2].unk015[2] = 6;
    player[3].unk015[2] = 7;
    player[0].unk025[3] = 0;
    player[1].unk025[3] = 1;
    player[2].unk025[3] = 2;
    player[3].unk025[3] = 3;
    player[0].isActive = 1;
    player[1].isActive = 1;
    player[2].isActive = 1;
    player[3].isActive = 1;
    D_8013AFD0 = 5;
    player[0].unk01C = 0;
    player[1].unk01C = 0;
    player[2].unk01C = 0;
    player[3].unk01C = 0;
    D_8013AFD2 = 3;
    D_8013AFCD = 2;
    D_8013AFDE = 100;
    for (; i < 4; i++) {
        player[i].unk025[1] = 0;
        player[i].unk025[2] = 0;
    }
    resetGameplayRng();
    configureTaskStateTransition(1, 0, 30);
    setSecondaryDisplayTaskMode(1);
    setCurrentTaskCallback(initializeRaceTaskState);
    queueCdSectorCommand(0, gControllerPlaybackEntries[D_8013AFD9].sector, gControllerPlaybackEntries[D_8013AFD9].count, D_800E959C, 0);
    if (selectionMenuRecordAbsolute != D_8013AFCE) {
        func_800180F4(2);
        func_800180F4(3);
        func_800180F4(4);
        func_800180F4(5);
        queuePlayerResourceReadsAndUploadTextures();
    }
    D_8013AE88->unk10 |= 2;
}


void advanceTaskWhenFlagB55ESet(void) {
    if (D_8013B55E != 0) {
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(updateControllerPlaybackUntilFade);
    }
}

void updateControllerPlaybackUntilFade(void) {
    func_80040D5C();
    if (D_8013AEE8[0] & 0x800) {
        D_8013AE88->data18 = gControllerPlaybackDurations[D_8013AFD9];
    }
    D_8013AE88->data18++;
    if (gControllerPlaybackDurations[D_8013AFD9] <= D_8013AE88->data18) {
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        D_8013AE88->data18 = 0;
        setCurrentTaskCallback(finishControllerPlaybackTransition);
    } else {
        playbackControllerInput(0);
        playbackControllerInput(1);
    }
}

void finishControllerPlaybackTransition(void) {
    switch (D_8013AE88->data18) {
    case 0:
        if (func_800707F0(0)) {
            playbackControllerInput(0);
            playbackControllerInput(1);
        } else {
            stopPlayerSound(0);
            D_8013AFAA = (D_8013AFAA | 0x1090) & ~0x2000;
            resetPendingCdRequests();
            resetPaletteRecords();
            func_800149E0();
            D_8013B10C = -1;
            D_8013AFAA |= 0x48;
            func_8002FA58(2);
            D_8013B106 = 0;
            playbackTransitionByteAbsolute = 0;
            setControllerFlag5(0);
            setControllerFlag5(1);
            SetFogNearFar(5000, 6000, 400);
            D_8013B109 = 1;
            D_8013B108 = 28;
            D_8013AE88->data18 = 1;
        }
        break;
    case 1:
        setSecondaryDisplayTaskMode(1);
        configureTaskStateTransition(1, 0, 30);
        D_8013AE88->data18 = 2;
        break;
    case 2:
        D_8013B564 = 0;
        setCurrentTaskCallback(initializeGameOptionViewTask);
        D_8013AE88->unk10 |= 2;
        break;
    }
}
