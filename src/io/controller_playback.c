#include "../menu/selection_tasks_internal.h"
#include "game/race/ui/race_hud.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/random.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/io/dat_entries.h"
#include "game/menu/selected_records.h"
#include "game/race/player/race_player.h"
#include "game/save/save_data.h"

extern u8 gModeSequenceStage __attribute__((section(".data")));
extern u8 gControllerPlaybackIndex __attribute__((section(".data")));
extern s16 gNextSelectedRecordProgressBonus __attribute__((section(".data")));
extern u8 gRaceResultCode __attribute__((section(".data")));
extern s16 gSelectedCourseIndex __attribute__((section(".data")));
extern u8 gRaceRuleset __attribute__((section(".data")));
extern u8 gMainMenuSelectionIndex __attribute__((section(".data")));
extern u8 gRacePauseState __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern u8 gSelectedPlayerReplayActive __attribute__((section(".data")));

extern u8 gMenuScreenId __attribute__((section(".data")));
extern u8 gMenuTransitionState __attribute__((section(".data")));
extern s16 gMenuMusicEnabled __attribute__((section(".data")));

ControllerPlaybackEntry gControllerPlaybackEntries[12] __attribute__((section(".data.controller_playback_entries"))) = {
    {0, 0, 0, 0},
    {0, 0, DAT_CONTROLLER_PLAYBACK_001_SECTOR, DAT_CONTROLLER_PLAYBACK_001_SECTOR_COUNT},
    {0, 0, DAT_CONTROLLER_PLAYBACK_002_SECTOR, DAT_CONTROLLER_PLAYBACK_002_SECTOR_COUNT},
    {9, 0, DAT_CONTROLLER_PLAYBACK_003_SECTOR, DAT_CONTROLLER_PLAYBACK_003_SECTOR_COUNT},
    {0, 0, DAT_CONTROLLER_PLAYBACK_004_SECTOR, DAT_CONTROLLER_PLAYBACK_004_SECTOR_COUNT},
    {0, 0, DAT_CONTROLLER_PLAYBACK_005_SECTOR, DAT_CONTROLLER_PLAYBACK_005_SECTOR_COUNT},
    {4, 0, DAT_CONTROLLER_PLAYBACK_006_SECTOR, DAT_CONTROLLER_PLAYBACK_006_SECTOR_COUNT},
    {1, 0, DAT_CONTROLLER_PLAYBACK_007_SECTOR, DAT_CONTROLLER_PLAYBACK_007_SECTOR_COUNT},
    {3, 0, DAT_CONTROLLER_PLAYBACK_008_SECTOR, DAT_CONTROLLER_PLAYBACK_008_SECTOR_COUNT},
    {2, 0, DAT_CONTROLLER_PLAYBACK_009_SECTOR, DAT_CONTROLLER_PLAYBACK_009_SECTOR_COUNT},
    {9, 0, DAT_CONTROLLER_PLAYBACK_010_SECTOR, DAT_CONTROLLER_PLAYBACK_010_SECTOR_COUNT},
    {0, 0, DAT_CONTROLLER_PLAYBACK_011_SECTOR, DAT_CONTROLLER_PLAYBACK_011_SECTOR_COUNT},
};

s16 gControllerPlaybackDurations[12] __attribute__((section(".data.controller_playback_durations"))) = {
    900, 384, 560, 592, 613, 606, 400, 288, 288, 286, 205, 272
};

void initializeControllerPlaybackRaceTask(void) {
    s32 i = 0;
    RacePlayer *player;
    selectionMenuRecordAbsolute = gSelectedCourseIndex;
    gSelectedCourseIndex = gControllerPlaybackEntries[gControllerPlaybackIndex].course;
    player = gRacePlayers;
    gRacePauseState = 0;
    gSelectedPlayerReplayActive = 0;
    gRaceResultCode = 0;
    gModeSequenceStage = 0;
    gMainMenuSelectionIndex = 0;
    gRaceRuleset = 0;
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
    selectedLapCountAbsolute = 5;
    player[0].unk01C = 0;
    player[1].unk01C = 0;
    player[2].unk01C = 0;
    player[3].unk01C = 0;
    gActiveRacePlayerCount = 3;
    playerCountAbsolute = 2;
    gNextSelectedRecordProgressBonus = 100;
    for (; i < 4; i++) {
        player[i].unk025[1] = 0;
        player[i].unk025[2] = 0;
    }
    resetGameplayRng();
    configureTaskStateTransition(1, 0, 30);
    setSecondaryDisplayTaskMode(1);
    setCurrentTaskCallback(initializeRaceTaskState);
    queueCdSectorCommand(0, gControllerPlaybackEntries[gControllerPlaybackIndex].sector, gControllerPlaybackEntries[gControllerPlaybackIndex].count, gControllerPlaybackRecords, 0);
    if (selectionMenuRecordAbsolute != gSelectedCourseIndex) {
        closeSoundBank(2);
        closeSoundBank(3);
        closeSoundBank(4);
        closeSoundBank(5);
        queuePlayerResourceReadsAndUploadTextures();
    }
    gCurrentTask->unk10 |= 2;
}


void advanceTaskWhenFlagB55ESet(void) {
    if (D_8013B55E != 0) {
        gCurrentTask->data18 = 0;
        setCurrentTaskCallback(updateControllerPlaybackUntilFade);
    }
}

void updateControllerPlaybackUntilFade(void) {
    updateRaceHud();
    if (gControllerPressedButtons[0] & 0x800) {
        gCurrentTask->data18 = gControllerPlaybackDurations[gControllerPlaybackIndex];
    }
    gCurrentTask->data18++;
    if (gControllerPlaybackDurations[gControllerPlaybackIndex] <= gCurrentTask->data18) {
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 20, 0, 1);
        gCurrentTask->data18 = 0;
        setCurrentTaskCallback(finishControllerPlaybackTransition);
    } else {
        playbackControllerInput(0);
        playbackControllerInput(1);
    }
}

void finishControllerPlaybackTransition(void) {
    switch (gCurrentTask->data18) {
    case 0:
        if (isTaskStateTransitionBelowMaximum(0)) {
            playbackControllerInput(0);
            playbackControllerInput(1);
        } else {
            stopPlayerSound(0);
            gGameStateFlags = (gGameStateFlags | 0x1090) & ~0x2000;
            resetPendingCdRequests();
            resetPaletteRecords();
            deleteCurrentTaskDescendants();
            gMenuTextureResourceId = -1;
            gGameStateFlags |= 0x48;
            queueMenuResourceReads(2);
            gMenuMusicEnabled = 0;
            playbackTransitionByteAbsolute = 0;
            setControllerFlag5(0);
            setControllerFlag5(1);
            SetFogNearFar(5000, 6000, 400);
            gMenuTransitionState = 1;
            gMenuScreenId = 28;
            gCurrentTask->data18 = 1;
        }
        break;
    case 1:
        setSecondaryDisplayTaskMode(1);
        configureTaskStateTransition(1, 0, 30);
        gCurrentTask->data18 = 2;
        break;
    case 2:
        D_8013B564 = 0;
        setCurrentTaskCallback(initializeGameOptionViewTask);
        gCurrentTask->unk10 |= 2;
        break;
    }
}
