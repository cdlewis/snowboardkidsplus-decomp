#include "../race/player/player_actions_internal.h"
#include "game/menu/save_record_tasks.h"
#include "game/engine/system_runtime.h"
#include "game/menu/menu_selection_state.h"
#include "../race/race_internal.h"
#include "game/audio/music.h"
#include "game/engine/game_options.h"
#include "game/engine/offset_archive.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/io/cd_queue.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/race/camera/race_camera.h"
#include "game/race/player/race_player.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"

extern u16 D_8013AF7A __attribute__((section(".data")));
extern u16 D_8013AF7C __attribute__((section(".data")));
extern u32 gGameOptionFlags __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *gCurrentTaskParent __attribute__((section(".data")));
extern u8 gDemoRaceIndex __attribute__((section(".data")));

void resetGameOptionState(void) {
    D_8013B108 = 2;
    D_8013B109 = 0;
    D_8013B10A = 0;
    gGameOptionFlags = 0x3E;
}

void queueBackgroundTextureSectorCommands(void) {
    queueCdSectorCommand(2, DAT_BACKGROUND_TEXTURES_SECTOR, DAT_BACKGROUND_TEXTURES_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(0, DAT_BACKGROUND_DEFINITION_SECTOR, DAT_BACKGROUND_DEFINITION_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(0, DAT_BACKGROUND_MAP_SECTOR, DAT_BACKGROUND_MAP_SECTOR_COUNT, DAT_BACKGROUND_MAP, 0);
    queueCdSectorCommand(0, DAT_BACKGROUND_TEXTURE_FIRST_SECTOR, DAT_BACKGROUND_TEXTURE_FIRST_SECTOR_COUNT, DAT_BACKGROUND_TEXTURE_FIRST, 0);
    queueCdSectorCommand(0, DAT_BACKGROUND_TEXTURE_SECOND_SECTOR, DAT_BACKGROUND_TEXTURE_SECOND_SECTOR_COUNT, DAT_BACKGROUND_TEXTURE_SECOND, 0);
}

void uploadBufferedTexturePair(void) {
    uploadTimImageWithOffset(DAT_BACKGROUND_TEXTURE_SECOND, NULL);
    uploadTimImageWithOffset(DAT_BACKGROUND_TEXTURE_FIRST, NULL);
}

void uploadTextureArchiveEntry(s16 x, s16 y, s16 clutX, s16 clutY, s32 index) {
    OffsetArchive *archive = DAT_TEXTURE_ARCHIVE;
    s32 offset;
    /* The archive word index includes the count and wraps as a signed halfword. */
    index = (s16)index + 1;
    index = (s16)index;
    offset = (s32)archive->offsets[index - 1];
    uploadTimImageAt(x, y, clutX, clutY,
                  (u32 *)((s16 *)archive + (offset >> 1)));
}

void initDefaultBackground(s16 mode, s16 unused, GsBG *background, GsMAP *map) {
    initTiledBackground(mode, unused, background, map,
                        (BackgroundCellHeader *)DAT_SCRATCH, (BackgroundMapResource *)DAT_BACKGROUND_MAP);
}

void queueStartupSectorReads(void) {
    queueCdSectorCommand(0, DAT_STARTUP_MODELS_SECTOR, DAT_STARTUP_MODELS_SECTOR_COUNT, DAT_STARTUP_MODELS, 0);
    queueCdSectorCommand(0, DAT_STARTUP_TEXTURES_SECTOR, DAT_STARTUP_TEXTURES_SECTOR_COUNT, DAT_STARTUP_TEXTURES, 0);
}

void queueBackgroundTextureArchiveRead(void) {
    OffsetArchive *destination = DAT_TEXTURE_ARCHIVE;
    queueCdSectorCommand(2, DAT_BACKGROUND_TEXTURES_SECTOR, DAT_BACKGROUND_TEXTURES_SECTOR_COUNT, destination, 0);
}

void uploadTimImageAt(s16 x, s16 y, s16 clutX, s16 clutY, u32 *tim) {
    GsIMAGE image;
    if (*tim == 0x10) {
        GsGetTimInfo(tim + 1, &image);
        image.px = x;
        image.py = y;
        LoadImage((RECT *)&image.px, image.pixel);
        if ((image.pmode >> 3) & 1) {
            image.cx = clutX;
            image.cy = clutY;
            LoadImage((RECT *)&image.cx, image.clut);
        }
    }
}

void initializeBackgroundCameraTask(void) {
    D_8013AF7A = 320;
    D_8013AF7C = 240;
    D_8013B10C = -1;
    D_8013B10E = -1;
    D_8013B106 = 0;
    configureTaskStateTransition(11, 0, 30);
    setSecondaryDisplayTaskMode(1);
    queueBackgroundTextureSectorCommands();
    D_8013AF84 = 400;
    initPlayerCamera(0);
    initializeDefaultSceneLighting();
    createChildTask(scheduleMode2SequenceWhenCdQueueIdle);
    gCreatedTask->data18 = gCurrentTask->data18;
    gCreatedTask->data1C = 0;
    queueMenuResourceReads((s16)gCreatedTask->data18);
    gCurrentTask->data1E = 0;
    setCurrentTaskCallback(scheduleBackgroundViewAfterChildTasks);
    gCurrentTask->unk10 |= 2;
}

void scheduleBackgroundViewAfterChildTasks(void) {
    setCurrentTaskCallback(initializeViewAndSelectTaskCallback);
    gCurrentTask->unk10 |= 1;
}

void initializeViewAndSelectTaskCallback(void) {
    if (gCurrentTask->data1E != 0) {
        D_8013AF84 = 0x100;
        initPlayerCamera(0);
        initializeDefaultSceneLighting();
        if (gDemoRaceIndex != 0) {
            goto restore;
        }
    } else {
restore:
        deleteCurrentTaskSubtree();
        return;
    }
    setCurrentTaskCallback(prepareRacePlayersAndQueueResources);
}

void waitForBackgroundMenuTransition(void) {
    if (isTaskStateTransitionBelowMaximum(0) == 0) {
        setCurrentTaskCallback(dispatchBackgroundMenuTask);
    }
}

void beginMode2TaskSequence(void) {
    configureTaskStateTransition(2, 0, 30);
    setCurrentTaskCallback(waitForBackgroundMenuTransition);
}

void scheduleMode2SequenceWhenCdQueueIdle(void) {
    setCurrentTaskCallback(beginMode2TaskSequence);
    gCurrentTask->unk10 |= 2;
}

void dispatchBackgroundMenuTask(void) {
    GameTask *task = gCurrentTask;

    if (task->data1C == 0) {
        D_8013B108 = task->data18;
        switch (task->data18) {
        case 2:
            configureColorCycleTask(0, 0, 0, 0, 0);
            configureColorCycleTask(0, 0, 0, 0, 1);
            setMusicSequenceEnabled(0);
            deleteCurrentTaskSubtree();
            return;
        case 3:
            createChildTaskWithReserve(initializePlayerModeSelectionTask);
            return;
        case 4:
            createChildTaskWithReserve(initializeSaveRecordTask);
            return;
        case 30:
            createChildTaskWithReserve(startUnlockNotification0Task);
            return;
        case 5:
            createChildTaskWithReserve(initializeDualResourceSequenceTask);
            return;
        case 6:
            createChildTaskWithReserve(initializeDualResourceTaskAndNotifyParent);
            return;
        case 7:
            createChildTaskWithReserve(initializeFlagSelectedCdAwareTask);
            return;
        case 8:
            createChildTaskWithReserve(initializeCountedPlayerGroupedSelectionBoundsTask);
            return;
        case 9:
            createChildTaskWithReserve(initializeMainTailDualResourceTask);
            return;
        case 10:
            createChildTaskWithReserve(initializeFlagSelectedDualResourceTask);
            return;
        case 11:
            createChildTaskWithReserve(initializeCountedTaskResourcePair);
            return;
        case 12:
            createChildTaskWithReserve(initializeAlternateCountedTaskResourcePair);
            return;
        case 16:
            createChildTaskWithReserve(initializeCountedGroupedSelectionBoundsTask);
            return;
        case 17:
            createChildTaskWithReserve(initializeCountedSelectionBoundsTask);
            return;
        case 13:
            createChildTaskWithReserve(initializeCountedTaskResourcePairEC50);
            return;
        case 18:
            createChildTaskWithReserve(initializeCountedDisplayBoundsTask);
            return;
        case 19:
            createChildTaskWithReserve(initializeCountedSplitSelectionBoundsTask);
            return;
        case 14:
            createChildTaskWithReserve(initializeCountedFourBoundsTask);
            return;
        case 15:
            createChildTaskWithReserve(initializeMemoryCardSaveMenuTask);
            return;
        case 20:
        case 21:
        case 22:
            createChildTaskWithReserve(initializeSelectionIndexedDualResourceTask);
            return;
        case 29:
            createChildTaskWithReserve(initializeNumberAdjustmentTask);
            return;
        case 23:
            queueMenuResourceReads(gCurrentTask->data18);
            if (isCdCommandQueueIdle() == 1) {
                configureTaskStateTransition(1, 0, 30);
                setTaskBackgroundMode(3, gSelectedCourseMenuItem, 0);
                goto selectFallback;
            }
            return;
        case 24:
            configureColorCycleTask(0, 0, 0, 0, 0);
            configureColorCycleTask(0, 0, 0, 0, 1);
            setMusicSequenceEnabled(0);
            gDemoRaceIndex = 0;
            D_8013B109 = 3;
            switch (gMainMenuSelectionIndex) {
            case 1:
                D_8013B108 = 9;
                break;
            case 0:
            case 2:
                D_8013B108 = 6;
                break;
            default:
                D_8013B108 = 2;
                gMainMenuSelectionIndex = 0;
                break;
            }
            gCurrentTaskParent->data1E = 1;
            deleteCurrentTaskSubtree();
            return;
        }
    selectFallback:
        gCurrentTask->data18 = 24;
    }
}

void beginMode2SequenceAndFadeMusic(void) {
    configureTaskStateTransition(2, 0, 30);
    if (D_8013B106 == 1) {
        fadeOutMusicSequence(0x200);
        D_8013B106 = 0;
    }
}

void queueMenuResourceReads(s16 resource) {
    switch (resource) {
    case 12:
    case 13:
        resource = 11;
        break;
    case 21:
    case 22:
        resource = 20;
        break;
    case 10:
        resource = 7;
        break;
    case 19:
        resource = 18;
        break;
    case 15:
        resource = 5;
        break;
    case 26:
        resource = 2;
        break;
    }

    if (resource == D_8013B10C || resource == D_8013B10E) {
        return;
    }

    D_8013B10E = -1;
    switch (resource) {
    case 2:
        queueCdSectorCommand(2, DAT_MODE2_TEXTURES_SECTOR, DAT_MODE2_TEXTURES_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 2;
        break;
    case 5:
        queueCdSectorCommand(0, DAT_ENTRY_018_SECTOR, DAT_ENTRY_018_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 5;
        break;
    case 6:
        queueCdSectorCommand(0, DAT_ENTRY_014_SECTOR, DAT_ENTRY_014_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 6;
        break;
    case 7:
        queueCdSectorCommand(0, DAT_ENTRY_015_SECTOR, DAT_ENTRY_015_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 7;
        break;
    case 9:
        queueCdSectorCommand(0, DAT_ENTRY_017_SECTOR, DAT_ENTRY_017_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 9;
        break;
    case 11:
        queueCdSectorCommand(0, DAT_ENTRY_016_SECTOR, DAT_ENTRY_016_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 11;
        break;
    case 20:
        queueCdSectorCommand(0, DAT_ENTRY_019_SECTOR, DAT_ENTRY_019_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 20;
        break;
    case 18:
        queueGridResourceSectorReads(0, gRacePlayers[0].unk009, 0);
        queueGridResourceSectorReads(1, gRacePlayers[0].unk009, 1);
        queueGridResourceSectorReads(2, gRacePlayers[0].unk009, 2);
        queueGridResourceSectorReads(3, gRacePlayers[0].unk009, 3);
        queueGridResourceSectorReads(4, gRacePlayers[0].unk009, 4);
        D_8013B10E = 18;
        break;
    case 14:
        queueGridResourceSectorReads(0, gRacePlayers[0].unk009, gRacePlayers[0].unk00A[0]);
        queueGridResourceSectorReads(1, 10, 0);
        D_8013B10E = 14;
        break;
    case 23:
        queueCdSectorCommand(1, DAT_ENTRY_182_SECTOR, DAT_ENTRY_182_SECTOR_COUNT, DAT_TEXTURE_ARCHIVE, 0);
        D_8013B10C = 23;
        break;
    }
}

void queueMode11TransitionResources(void) {
    closeMusicSequence();
    closeAllSoundBanks();
    queueCdSectorCommand(0, DAT_MODE11_RESOURCE_07CD_SECTOR, DAT_MODE11_RESOURCE_07CD_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(3, DAT_MODE11_RESOURCE_0801_SECTOR, DAT_MODE11_RESOURCE_0801_SECTOR_COUNT, DAT_SOUND_HEADER_0, 0);
    queueCdSectorCommand(0, DAT_MODE11_RESOURCE_0804_SECTOR, DAT_MODE11_RESOURCE_0804_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(4, DAT_MODE11_RESOURCE_0861_SECTOR, DAT_MODE11_RESOURCE_0861_SECTOR_COUNT, DAT_SOUND_HEADER_1, 0);
    queueCdSectorCommand(0, DAT_MODE11_RESOURCE_0961_SECTOR, DAT_MODE11_RESOURCE_0961_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(5, DAT_MODE11_RESOURCE_09C9_SECTOR, DAT_MODE11_RESOURCE_09C9_SECTOR_COUNT, DAT_SOUND_HEADER_2, 0);
    queueCdSectorCommand(9, DAT_MODE11_RESOURCE_09CD_SECTOR, DAT_MODE11_RESOURCE_09CD_SECTOR_COUNT, DAT_MUSIC_SEQUENCE, 0);
    queueCdSectorCommand(0, DAT_STARTUP_MODELS_SECTOR, DAT_STARTUP_MODELS_SECTOR_COUNT, DAT_STARTUP_MODELS, 0);
    queueCdSectorCommand(0, DAT_STARTUP_TEXTURES_SECTOR, DAT_STARTUP_TEXTURES_SECTOR_COUNT, DAT_STARTUP_TEXTURES, 0);
    SetFogNearFar(5000, 6000, 400);
}
