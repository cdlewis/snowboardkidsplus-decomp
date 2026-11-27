#include "game/race/player/player_results.h"
#include "psx/libapi.h"
#include "psx/libetc.h"
#include "common.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/quad_animation.h"
#include "game/race/camera/race_camera.h"
#include "game/race/course/course_objects.h"
#include "game/race/ui/race_hud.h"
#include "game/race/ui/race_results.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "game/io/cd_queue.h"
#include "game/io/controller.h"
#include "game/io/controller_effects.h"
#include "game/race/player/race_player.h"
#include "game/race/player/player_resource_textures.h"
#include "game/engine/game_options.h"
#include "game/menu/selected_records.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"

#include "game/save/save_data.h"
#include "game/engine/game_loop.h"
#include "game/engine/game_task_scheduler.h"
#include "game/engine/system_runtime.h"

#include "game/race/player/player_resource_textures.h"
#include "game/engine/random.h"
#include "game/io/dat_entries.h"
#include "game/io/dat_resources.h"
#include "game/io/movie_stream.h"

#define RACE_SOUND_HEADER ((u8 *)0x80165800)
#define RACE_PLAYER_SOUND_HEADERS ((u8 (*)[0x1000])0x80167000)
#define RACE_PAUSE_START 0x0800
#define RACE_PAUSE_RESUME_PENDING 0x80
#define RACE_DEMO_FRAME_LIMIT 946

typedef struct RacePlayerSoundSectorRanges {
    s16 bodySector;
    s16 bodySectorCount;
    s16 headerSector;
    s16 headerSectorCount;
} RacePlayerSoundSectorRanges;

extern s16 gFrameCounter __attribute__((section(".data")));
extern VECTOR *gScratchVector0 __attribute__((section(".data")));
extern MATRIX *gScratchMatrix0 __attribute__((section(".data")));
extern u16 gDisplayWidth __attribute__((section(".data")));
extern u16 gDisplayHeight __attribute__((section(".data")));
extern u16 gActiveGraphicsBuffer __attribute__((section(".data")));
extern s16 gPlayerViewWidthReduction __attribute__((section(".data")));
extern s16 gPlayerViewVerticalOffset __attribute__((section(".data")));
extern u16 gGameStateFlags __attribute__((section(".data")));
extern u8 D_8013B1AA __attribute__((section(".data")));
extern u8 D_8013B1AB __attribute__((section(".data")));
extern GameTask *gCurrentTask __attribute__((section(".data")));
extern GameTask *gCreatedTask __attribute__((section(".data")));
extern GameTask *D_8013AE90 __attribute__((section(".data")));
extern GameTask *D_8013B304 __attribute__((section(".data.player_snapshot_tasks")));

extern u8 gControllerInputRecordState __attribute__((section(".data")));
extern s16 gCdCommandActive __attribute__((section(".data")));
extern RaceTime gRaceElapsedTime __attribute__((section(".data")));

const char gRaceVsyncDebugFormat[16]
    __attribute__((section(".rodata.race_vsync_debug_format"), aligned(4))) = "VSYNC=%4d";

RacePlayerSoundSectorRanges gRacePlayerSoundSectorRanges[10]
    __attribute__((section(".data.race_startup_sector_ranges"))) = {
    {
        DAT_PLAYER_SOUND_BODY_00_SECTOR, DAT_PLAYER_SOUND_BODY_00_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_00_SECTOR, DAT_PLAYER_SOUND_HEADER_00_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_01_SECTOR, DAT_PLAYER_SOUND_BODY_01_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_01_SECTOR, DAT_PLAYER_SOUND_HEADER_01_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_02_SECTOR, DAT_PLAYER_SOUND_BODY_02_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_02_SECTOR, DAT_PLAYER_SOUND_HEADER_02_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_03_SECTOR, DAT_PLAYER_SOUND_BODY_03_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_03_SECTOR, DAT_PLAYER_SOUND_HEADER_03_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_04_SECTOR, DAT_PLAYER_SOUND_BODY_04_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_04_SECTOR, DAT_PLAYER_SOUND_HEADER_04_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_05_SECTOR, DAT_PLAYER_SOUND_BODY_05_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_05_SECTOR, DAT_PLAYER_SOUND_HEADER_05_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_06_SECTOR, DAT_PLAYER_SOUND_BODY_06_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_06_SECTOR, DAT_PLAYER_SOUND_HEADER_06_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_07_SECTOR, DAT_PLAYER_SOUND_BODY_07_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_07_SECTOR, DAT_PLAYER_SOUND_HEADER_07_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_08_SECTOR, DAT_PLAYER_SOUND_BODY_08_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_08_SECTOR, DAT_PLAYER_SOUND_HEADER_08_SECTOR_COUNT,
    },
    {
        DAT_PLAYER_SOUND_BODY_09_SECTOR, DAT_PLAYER_SOUND_BODY_09_SECTOR_COUNT,
        DAT_PLAYER_SOUND_HEADER_09_SECTOR, DAT_PLAYER_SOUND_HEADER_09_SECTOR_COUNT,
    },
};

ResourceSectorRange gRacePlaybackSectorRanges[5]
    __attribute__((section(".data.race_startup_sector_ranges"))) = {
    {DAT_RACE_CONTROLLER_PLAYBACK_00_SECTOR, DAT_RACE_CONTROLLER_PLAYBACK_00_SECTOR_COUNT},
    {DAT_RACE_CONTROLLER_PLAYBACK_01_SECTOR, DAT_RACE_CONTROLLER_PLAYBACK_01_SECTOR_COUNT},
    {DAT_RACE_CONTROLLER_PLAYBACK_02_SECTOR, DAT_RACE_CONTROLLER_PLAYBACK_02_SECTOR_COUNT},
    {DAT_RACE_CONTROLLER_PLAYBACK_03_SECTOR, DAT_RACE_CONTROLLER_PLAYBACK_03_SECTOR_COUNT},
    {DAT_RACE_CONTROLLER_PLAYBACK_04_SECTOR, DAT_RACE_CONTROLLER_PLAYBACK_04_SECTOR_COUNT},
};

u8 gCourseOpponentCharacters[10][4] __attribute__((section(".data.course_race_setup"))) = {
    {0, 1, 2, 5},
    {4, 7, 6, 1},
    {0, 2, 5, 4},
    {1, 2, 6, 3},
    {1, 2, 3, 5},
    {4, 1, 5, 6},
    {1, 2, 7, 4},
    {0, 1, 2, 3},
    {8, 0, 1, 3},
    {3, 5, 0, 2},
};

u8 gCourseOpponentBoardSelections[12] __attribute__((section(".data.course_race_setup"))) = {
    1, 1, 4, 4, 4, 7, 7, 0, 7, 1, 0, 0,
};

u8 D_80120484[12] __attribute__((section(".data.course_race_setup"))) = {
    20, 30, 40, 40, 60, 80, 128, 0, 255, 10, 0, 0,
};

u8 gRaceStartupPlayerPresets1[4][3] __attribute__((section(".data.course_race_setup"))) = {
    {0, 1, 0},
    {3, 1, 1},
    {7, 0, 2},
    {1, 0, 3},
};

u8 gRaceStartupPlayerPresets5[4][3] __attribute__((section(".data.course_race_setup"))) = {
    {3, 2, 4},
    {5, 0, 5},
    {6, 0, 6},
    {4, 0, 7},
};

u16 gSelectedPlayerRestoreFrames[14] __attribute__((section(".data.selected_player_restore_frames"))) = {
    0, 104, 205, 309, 412, 515, 616, 718, 821, 928, 1036, 1126, 1330, 0
};

s16 gPlayerResourceTexturePositions[4][2] __attribute__((section(".data.player_resource_texture_positions"))) = {
    {0, 0}, {16, 0}, {32, 0}, {48, 0}
};

PlayerResourceTextureSelections gPlayerResourceTextureSelections __attribute__((section(".data.player_resource_texture_selections"))) = {
    {
        {0, 6, 1, 4, 3, 8, 5, 11, 13, 14, 15, 16, 17, 18, 19},
        {0, 6, 1, 4, 3, 8, 5, 11, 13, 14, 15, 16, 17, 18, 19}
    },
    {0, 0}
};

ModelTextureOffset gPlayerRootTextureOffsets[4] __attribute__((section(".data.player_model_texture_offsets"))) = {
    {0, 0, 0}, {0, 64, 1}, {0, 128, 2}, {0, 192, 3}
};

ModelTextureOffset gPlayerPartTextureOffsets[4] __attribute__((section(".data.player_model_texture_offsets"))) = {
    {0, 0, 0}, {0, 0x8000, 0x2000}, {1, 0, 4}, {1, 0x8000, 0x2004}
};

GsDOBJ2 gPlayerTimerModelObject
    __attribute__((section(".data.player_timer_model_object"))) = {0};

u32 gRacePlayerSortSavedStackPointer
    __attribute__((section(".sdata.race_player_sort_saved_stack"))) = 0;

extern u8 D_8013AFCC;
extern u8 D_8013AFD4;


void initializeAlternatingVramRowTask(void);
void animateAlternatingVramRow(void);
void initializeRaceTaskState(void);
void updateActiveRaceTask(void);
void finishRaceTransitionAfterCountdown(void);
void dispatchPostRaceResultsTask(void);
void processTaskWhenFlagAFAABit7Set(void);

void runMovieAndGameLoop(void) {
    func_800131B4();
    while (1) {
        initializeGameSystems();
        playMovie(0);
        playMovie(1);
        runGameLoop();
        ResetGraph(3);
        StopCallback();
    }
}

void initCurrentPlayerModelParts(void) {
    s32 i;
    s16 *values;
    RacePlayer *player;
    for (i = 0; i < 14; i++) {
        currentPlayerAbsolute->unk13E[i] = D_80120200[i];
        currentPlayerAbsolute->unk14C[i] = D_80120210[i];
    }
    values = D_801203C4[currentPlayerAbsolute->characterId];
    player = currentPlayerAbsolute;
    for (i = 0; i < 14; i++) {
        player->unk5BC[i].x = player->unk5BC[i].y = player->unk5BC[i].z = 0;
        player->unk62C[i].x = *values++;
        player->unk62C[i].y = -*values++;
        player->unk62C[i].z = -*values++;
    }
}

void sortRacePlayerModelForView(u16 playerIndex, u16 view) {
    GsDOBJ2 *objects;
    GsDOBJ2 *extra;
    RacePlayer *player;
    s16 timer;
    u16 drawRootOnly;
    u16 count;
    u16 i;
    u16 blinkMask;

    gRacePlayerSortSavedStackPointer = SetSp((u32)getScratchAddr(255));
    player = &gRacePlayers[playerIndex];
    currentPlayerAbsolute = player;
    objects = player->modelObjects;
    timer = player->surfaceState.fields.unk134;
    drawRootOnly = 0;
    if (timer != 0) {
        blinkMask = playerCountAbsolute == 1 ? 1 : 2;
        drawRootOnly = 1;
        if (timer < 16) {
            drawRootOnly = !(blinkMask & (u16)timer);
        }
        if (timer >= 165 && (blinkMask & (u16)timer)) {
            drawRootOnly = 0;
        }
    }

    count = 1;
    if (!drawRootOnly) {
        count = 12;
        if (currentPlayerAbsolute->unk12C == 25) {
            count = 1;
        }
    }

    for (i = 0; i < count; i++) {
        GsGetLs(objects[i].coord2, D_800EC170[i]);
    }
    for (i = 0; i < count; i++, objects++) {
        if (i == 0 && currentPlayerAbsolute->unk0FE) {
            currentPlayerAbsolute->timer.coordinate.coord = *D_800EC170[0];
        }
        if (currentPlayerAbsolute->unk128 & 0x200000) {
            gScratchVector0->vx = gScratchVector0->vz = 0x1000;
            gScratchVector0->vy = currentPlayerAbsolute->unk0F8 * 512;
            ScaleMatrix(D_800EC170[i], gScratchVector0);
        }
        GsSetLsMatrix(D_800EC170[i]);
        if (!(taskTransitionFlagsAbsolute & 0x40) || i != 0) {
            if (view == 0) {
                GsSortObject4(objects, &gPlayer1ModelOrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
            } else {
                GsSortObject4(objects, &gPlayer2ModelOrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
            }
        }
        if (i == 0) {
            if (count != 1) {
                extra = objects + 12;
                if (view == 0) {
                    GsSortObject4(extra, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
                } else {
                    GsSortObject4(extra, &gPlayer2OrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
                }
                extra++;
                if (view == 0) {
                    GsSortObject4(extra, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
                } else {
                    GsSortObject4(extra, &gPlayer2OrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
                }
            }
            if (currentPlayerAbsolute->unk0FE) {
                if (currentPlayerAbsolute->unk0FF) {
                    setVector(gScratchVector0, currentPlayerAbsolute->timer.unk60,
                              currentPlayerAbsolute->timer.unk60, currentPlayerAbsolute->timer.unk60);
                } else {
                    setVector(gScratchVector0, -currentPlayerAbsolute->timer.unk60,
                              currentPlayerAbsolute->timer.unk60, -currentPlayerAbsolute->timer.unk60);
                }
                ScaleMatrix(&currentPlayerAbsolute->timer.coordinate.coord, gScratchVector0);
                GsSetLsMatrix(&currentPlayerAbsolute->timer.coordinate.coord);
                if (view == 0) {
                    GsSortObject4(&gPlayerTimerModelObject, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
                } else {
                    GsSortObject4(&gPlayerTimerModelObject, &gPlayer2OrderingTables[gActiveGraphicsBuffer], 2, getScratchAddr(0));
                }
            }
        }
    }
    SetSp(gRacePlayerSortSavedStackPointer);
}

void sortLitObject(GsDOBJ2 *object) {
    GsGetLw(object->coord2, gScratchMatrix0);
    GsSetLightMatrix(gScratchMatrix0);
    GsGetLs(object->coord2, gScratchMatrix0);
    GsSetLsMatrix(gScratchMatrix0);
    GsSortObject4(object, &gPlayer1OrderingTables[gActiveGraphicsBuffer], 2, (u32 *)getScratchAddr(0));
}

void initializeRacePaletteCycles(void) {
    u16 palette;
    s16 row;

    palette = saveVram16Pixels(656, 244);
    if (palette != 32) {
        startPaletteCycle(1, palette, 1, 6, 1);
    }

    switch (gSelectedCourseIndex) {
    case 0:
        palette = saveVram16Pixels(672, 498);
        if (palette != 32) {
            startPaletteCycle(1, palette, 0, 15, 1);
        }
        break;
    case 1:
        for (row = 0; row < 6; row++) {
            palette = saveVram16Pixels(656, (s16)(483 + row));
            if (palette != 32) {
                startPaletteCycle(1, palette, 0, 15, 1);
            }
        }
        break;
    case 2:
        palette = saveVram16Pixels(656, 495);
        if (palette != 32) {
            startPaletteCycle(0, palette, 0, 15, 1);
        }
        createChildTaskForParentWithReserve(&gGameTasks[4], initializeAlternatingVramRowTask);
        break;
    case 3:
        palette = saveVram16Pixels(688, 501);
        if (palette != 32) {
            startPaletteCycle(1, palette, 0, 15, 1);
        }
        break;
    case 4:
        palette = saveVram16Pixels(672, 497);
        if (palette != 32) {
            startPaletteCycle(0, palette, 0, 15, 1);
        }
        break;
    case 5:
        palette = saveVram16Pixels(640, 498);
        if (palette != 32) {
            startPaletteCycle(1, palette, 0, 15, 1);
        }
        palette = saveVram16Pixels(656, 504);
        if (palette != 32) {
            startPaletteCycle(1, palette, 0, 15, 1);
        }
        palette = saveVram16Pixels(656, 509);
        if (palette != 32) {
            startPaletteCycle(1, palette, 0, 15, 1);
        }
        break;
    case 7:
        palette = saveVram16Pixels(688, 490);
        if (palette != 32) {
            startPaletteCycle(1, palette, 1, 14, 2);
        }
        break;
    }
}

void initializeGameOptionTask(void) {
    queueCdSectorCommand(2, DAT_TRANSITION_TEXTURES_SECTOR, DAT_TRANSITION_TEXTURES_SECTOR_COUNT, DAT_SCRATCH, 0);
    D_8013B562 = 0;
    gDemoRaceIndex = 0;
    gCurrentTask->data18 = 0;
    initializeNewGameSaveData();
    resetGameOptionState();
    setCurrentTaskCallback(initializeGameOptionViewTask);
    gCurrentTask->unk10 |= 2;
}

void initializeGameOptionViewTask(void) {
    initPlayerCamera(0);
    initializeDefaultSceneLighting();
    setControllerFlag0(0);
    setControllerFlag0(1);
    setCurrentTaskCallback(queueRaceStartupResourcesTask);
    createChildTask(dispatchGameOptionTransitionTask);
    gCurrentTask->unk10 |= 1;
}

void queueRaceStartupResourcesTask(void) {
    u32 i;

    if (gDemoRaceIndex) {
        switch (gDemoRaceIndex) {
        case 1:
            gSelectedCourseIndex = 9;
            gPlayerCount = 1;
            gMainMenuSelectionIndex = 0;
            activeRacePlayerCountGp = 4;
            for (i = 0; i < 4; i++) {
                RacePlayer *player = &gRacePlayers[i];
                u8 characterId = gRaceStartupPlayerPresets1[i][0];

                player->unk015[0] = 0;
                player->characterId = characterId;
                player->unk015[1] = gRaceStartupPlayerPresets1[i][1];
                player->unk015[2] = gRaceStartupPlayerPresets1[i][2];
                player->unk015[3] = 0;
                player->isActive = 1;
                if (i == 0) {
                    gRacePlayers[0].unk008 = 0;
                } else {
                    player->unk008 = 1;
                    player->unk02C[0] = D_80120484[gSelectedCourseIndex];
                    player->unk02C[1] = D_80120484[gSelectedCourseIndex];
                    player->unk025[3] = i;
                }
            }
            break;
        case 2:
            gMainMenuSelectionIndex = 1;
            gRaceRuleset = 0;
            gSelectedCourseIndex = 0;
            gPlayerCount = 1;
            activeRacePlayerCountGp = 1;
            gRacePlayers[0].characterId = 4;
            gRacePlayers[0].unk015[1] = 2;
            gRacePlayers[0].isActive = 1;
            break;
        case 3:
            gMainMenuSelectionIndex = 1;
            gRaceRuleset = 1;
            gSelectedCourseIndex = 1;
            gPlayerCount = 1;
            activeRacePlayerCountGp = 1;
            gRacePlayers[0].characterId = 2;
            gRacePlayers[0].unk015[1] = 1;
            gRacePlayers[0].isActive = 1;
            break;
        case 4:
            gMainMenuSelectionIndex = 1;
            gRaceRuleset = 2;
            gSelectedCourseIndex = 7;
            gPlayerCount = 1;
            activeRacePlayerCountGp = 1;
            gRacePlayers[0].characterId = 1;
            gRacePlayers[0].unk015[1] = 0;
            gRacePlayers[0].isActive = 1;
            break;
        case 5:
            gSelectedCourseIndex = 3;
            gPlayerCount = 2;
            gMainMenuSelectionIndex = 0;
            activeRacePlayerCountGp = 4;
            for (i = 0; i < 4; i++) {
                RacePlayer *player = &gRacePlayers[i];
                u8 characterId = gRaceStartupPlayerPresets5[i][0];

                player->unk015[0] = 0;
                player->characterId = characterId;
                player->unk015[1] = gRaceStartupPlayerPresets5[i][1];
                player->unk015[2] = gRaceStartupPlayerPresets5[i][2];
                player->unk015[3] = 0;
                player->isActive = 1;
                if (i == 0) {
                    gRacePlayers[0].unk008 = 0;
                } else {
                    player->unk008 = 1;
                    player->unk02C[0] = D_80120484[gSelectedCourseIndex];
                    player->unk02C[1] = D_80120484[gSelectedCourseIndex];
                    player->unk025[3] = i;
                }
            }
            break;
        }
        if (gDemoRaceIndex) {
            gSelectedLapCount = gSelectedCourseRecords[gSelectedCourseIndex].unk02;
        }
        resetGameplayRng();
        gRacePauseState = 0;
        D_8013AFD4 = 0;
        D_8013AFD5 = 0;
        gSelectedPlayerReplayActive = 0;
        gRaceResultCode = 0;
        gModeSequenceStage = 0;
        gControllerPlaybackIndex = 0;
        D_8013AFCC = 0;
        gTaskTransitionFlags = 0;
        queuePlayerResourceReadsAndUploadTextures();
        initializeActiveRacePlayers();
        initializeRaceRulesAndCounters();
        D_8013B1A8 = 1;
    }

    queueCourseResourceSectorReads();
    closeMusicSequence();
    closeSoundBank(1);
    queueCdSectorCommand(0, DAT_ENTRY_125_SECTOR, DAT_ENTRY_125_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(4, DAT_ENTRY_126_SECTOR, DAT_ENTRY_126_SECTOR_COUNT,
        RACE_SOUND_HEADER, 0);
    for (i = 0; i < activeRacePlayerCountGp; i++) {
        queueCdSectorCommand(0, gRacePlayerSoundSectorRanges[gRacePlayers[i].characterId].bodySector,
            gRacePlayerSoundSectorRanges[gRacePlayers[i].characterId].bodySectorCount, DAT_SCRATCH, 0);
        queueCdSectorCommand(i + 5, gRacePlayerSoundSectorRanges[gRacePlayers[i].characterId].headerSector,
            gRacePlayerSoundSectorRanges[gRacePlayers[i].characterId].headerSectorCount,
            RACE_PLAYER_SOUND_HEADERS[i], 0);
    }
    queueCdSectorCommand(2, DAT_ENTRY_002_SECTOR, DAT_ENTRY_002_SECTOR_COUNT, DAT_SCRATCH, 0);
    queueCdSectorCommand(2, DAT_ENTRY_003_SECTOR, DAT_ENTRY_003_SECTOR_COUNT, DAT_SCRATCH, 0);
    if (gSelectedCourseIndex != 7) {
        queueCdSectorCommand(0, DAT_ENTRY_004_SECTOR, DAT_ENTRY_004_SECTOR_COUNT,
            RACE_EXTRA_MODEL_ARCHIVE, 0);
    }
    if (gDemoRaceIndex) {
        queueCdSectorCommand(0, gRacePlaybackSectorRanges[gDemoRaceIndex - 1].sector,
            gRacePlaybackSectorRanges[gDemoRaceIndex - 1].sectorCount, D_800E959C, 0);
    }
    queueCdSectorCommand(0, DAT_ENTRY_010_SECTOR, DAT_ENTRY_010_SECTOR_COUNT, DAT_SCRATCH, 0);
    if (!gModeSequenceStage && !gControllerPlaybackIndex) {
        setCurrentTaskCallback(initializeRacePresentationTask);
    } else {
        setCurrentTaskCallback(initializeModeRaceTasks);
    }
    gCurrentTask->unk10 |= 2;
}

void initializeRacePresentationTask(void) {
    s16 i;

    setSecondaryDisplayTaskMode(0);
    if (!D_8013B1AB) {
        initializeCourseModels();
    }
    initializeRacePlayerEffects();
    if (gPlayerCount == 2) {
        createChildTaskForParent(&gGameTasks[3], initCurrentPlayerCameraTask);
        gCreatedTask->data18 = 1;
    }
    createChildTaskForParent(&gGameTasks[3], initCurrentPlayerCameraTask);
    gCreatedTask->data18 = 0;
    initializeCourseObjects();
    createChildTask(updateRacePlayerFinishSlotsTask);
    for (i = activeRacePlayerCountGp - 1; i >= 0; i--) {
        createChildTask(initializeCurrentPlayerModelAndHeight);
        gCreatedTask->data18 = i;
        gRacePlayers[i].task = gCreatedTask;
    }
    configureTaskStateTransition(1, 0, 15);
    if (!gDemoRaceIndex) {
        queueCdCommand(1, 1);
    } else {
        gFrameCounter = 0;
    }

    D_8013AFDB = 0;
    gViewClipRects[0].x = 0;
    gViewClipRects[0].y = 0;
    gViewClipRects[0].w = gDisplayWidth;
    gViewClipRects[0].h = gDisplayHeight;

    if (gPlayerCount == 2) {
        gPlayerViewWidthReduction = 64;
        gPlayerViewVerticalOffset = gDisplayHeight / 4;
        gViewClipRects[1].x = 0;
        gViewClipRects[1].y = 0;
        gViewClipRects[1].w = gDisplayWidth - 64;
        gViewClipRects[1].h = gDisplayHeight / 2 - 1;
        gViewClipRects[2].x = 0;
        gViewClipRects[2].y = gDisplayHeight / 2 + 1;
        gViewClipRects[2].w = gDisplayWidth - 64;
        gViewClipRects[2].h = gDisplayHeight / 2 - 1;
    }

    initializeRacePaletteCycles();
    for (i = 0; i < gPlayerCount; i++) {
        copyPlayerPaletteRow(i);
    }
    resetTaskControlState();
    gGameStateFlags |= 0x48;
    clearControllerFlag0(0);
    clearControllerFlag0(1);
    if (gDemoRaceIndex || D_8013AEC4[0]) {
        setControllerFlag5(0);
    } else {
        clearControllerFlag5(0);
    }
    if (gDemoRaceIndex || D_8013AEC4[1]) {
        setControllerFlag5(1);
    } else {
        clearControllerFlag5(1);
    }

    for (i = 0; i < activeRacePlayerCountGp; i++) {
        gRacePlayers[i].unk01C = 0;
        gRacePlayers[i].unk020 = 0;
    }
    setCurrentTaskCallback(updateActiveRaceTask);
}

void beginTask60TickCountdown(void) {
    gCurrentTask->data1A = 60;
    setCurrentTaskCallback(waitThenStartSplitScreenTransition);
}

void waitThenStartSplitScreenTransition(void) {
    GameTask *task = gCurrentTask;

    if (--task->data1A == 0) {
        task->data1A = 16;
        setCurrentTaskCallback(updateSplitScreenTransitionTask);
    }
}

void updateSplitScreenTransitionTask(void) {
    gPlayerViewWidthReduction -= 4;
    gPlayerCameras[gCurrentTask->data18].unk10 += 16;
    gPlayerViewVerticalOffset -= 4;
    if (gPlayerViewVerticalOffset < 0) gPlayerViewVerticalOffset = 0;
    if (gCurrentTask->data18 == 0) {
        gViewClipRects[1].h += 8;
        if (gViewClipRects[1].h > 240) gViewClipRects[1].h = 240;
        gViewClipRects[1].w = gDisplayWidth - gPlayerViewWidthReduction;
        gViewClipRects[2].y += 8;
        gViewClipRects[2].h -= 8;
        if (gViewClipRects[2].h < 0) {
            gViewClipRects[2].y = 240;
            gViewClipRects[2].h = 0;
            gGameStateFlags |= 0x1000;
        }
    } else {
        gViewClipRects[2].y -= 8;
        gViewClipRects[2].h += 8;
        if (gViewClipRects[2].h > 240) {
            gViewClipRects[2].y = 0;
            gViewClipRects[2].h = 240;
        }
        gViewClipRects[2].w = gDisplayWidth - gPlayerViewWidthReduction;
        gViewClipRects[1].h -= 8;
        if (gViewClipRects[1].h < 0) {
            gViewClipRects[1].h = 0;
            gGameStateFlags |= 0x5000;
        }
    }
    if (--gCurrentTask->data1A == 0) deleteCurrentTaskSubtree();
}

void updateActiveRaceTask(void) {
    u16 i;
    u16 finishedCount;
    s32 pauseRequested;

    if (gRacePauseState & RACE_PAUSE_RESUME_PENDING) {
        gRacePauseState = 0;
    }
    updateRaceHud();
    if (!(gTaskTransitionFlags & 1) && D_8013B1AB) {
        gCurrentTask->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        setControllerFlag5(0);
        setControllerFlag5(1);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
        return;
    }
    if (D_8013B1AA) {
        gCurrentTask->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        if (gMainMenuSelectionIndex == 2) {
            gControllerInputRecordState = 0;
        }
        setControllerFlag5(0);
        setControllerFlag5(1);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
        return;
    }

    updateCpuRacerSpeedControl();
    finishedCount = 0;
    for (i = 0; i < gPlayerCount; i++) {
        if (gRacePlayers[i].unk128 & 0x40) {
            finishedCount++;
        }
    }
    if (finishedCount == gPlayerCount) {
        queueCdCommand(2, 1);
        if (gMainMenuSelectionIndex == 0) {
            if (gPlayerCount == 2) {
                if (!(gRacePlayers[0].unk100[0] | gRacePlayers[1].unk100[0])) {
                    return;
                }
                createChildTaskWithReserve(beginTask60TickCountdown);
                if (gRacePlayers[0].unk100[0] < gRacePlayers[1].unk100[0]) {
                    gCreatedTask->data18 = 0;
                } else {
                    gCreatedTask->data18 = 1;
                }
            }
            gRaceResultCode = 1;
        } else if (gPlayerCount == 2) {
            if (!(gRacePlayers[0].unk100[0] | gRacePlayers[1].unk100[0])) {
                return;
            }
            updateAttackResultsAndRecords();
            createChildTaskWithReserve(beginTask60TickCountdown);
            if (gRacePlayers[0].unk100[0] < gRacePlayers[1].unk100[0]) {
                gCreatedTask->data18 = 0;
            } else {
                gCreatedTask->data18 = 1;
            }
        }

        if (gMainMenuSelectionIndex == 2 && (gControllerInputRecordState & 2)) {
            recordControllerInput(0);
        }
        gCurrentTask->data18 = 60;
        setCurrentTaskCallback(finishRaceTransitionAfterCountdown);
        return;
    }

    if (!(gTaskTransitionFlags & 1)) {
        if (gRacePauseState) {
            goto pause;
        }
        if (!gDemoRaceIndex) {
            if (gPlayerCount == 1) {
                if ((gControllerPressedButtons[0] & RACE_PAUSE_START) || !isControllerInputSupported(0)) {
                    playSoundEffect(0, 0, 32);
                    gRacePauseState = 1;
                    createChildTaskForParentWithReserve(&gGameTasks[4], initializeRacePauseMenuTask);
                }
            } else {
                pauseRequested = 0;
                if ((gControllerPressedButtons[0] & RACE_PAUSE_START) || !isControllerInputSupported(0) ||
                    (gControllerPressedButtons[1] & RACE_PAUSE_START) || !isControllerInputSupported(1)) {
                    pauseRequested = 1;
                }
                if (pauseRequested) {
                    playSoundEffect(0, 0, 32);
                    gRacePauseState = ((gControllerPressedButtons[0] & RACE_PAUSE_START) ||
                                  !isControllerInputSupported(0)) ? 1 : 2;
                    createChildTaskForParentWithReserve(&gGameTasks[1], initializeRacePauseMenuTask);
                }
            }
        }
    }
    if (gRacePauseState) {
        goto pause;
    }
    if (gMainMenuSelectionIndex == 2 && (gControllerInputRecordState & 2)) {
        recordControllerInput(0);
    }
    if (gDemoRaceIndex) {
        if (gControllerInputRecordState & 2) {
            if (gFrameCounter < RACE_DEMO_FRAME_LIMIT) {
                drawStretchedDebugText(4, 4, gRaceVsyncDebugFormat, gFrameCounter);
                recordControllerInput(0);
                return;
            }
        } else if (gFrameCounter < RACE_DEMO_FRAME_LIMIT &&
                   !(gControllerPressedButtons[0] & RACE_PAUSE_START)) {
            goto continueDemo;
        }
        gCurrentTask->data18 = 45;
        configureTaskStateTransition(2, 0, 45);
        startCdVolumeFade(103, 40, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
    }
    return;

continueDemo:
    playbackControllerInput(0);
    setTextPalette(0);
    drawDemonstrationOverlay();
    return;

pause:
    gCurrentTask = D_8013AE90->previous;
}

void finishRaceTransitionAfterCountdown(void) {
    s16 i;
    if (gMainMenuSelectionIndex == 2 && (gControllerInputRecordState & 2)) recordControllerInput(0);
    if (--gCurrentTask->data18 == 0) {
        D_8013B1A8 = 0;
        gTaskTransitionFlags |= 8;
        if (gPlayerCount == 1) updateAttackResultsAndRecords();
        for (i = 0; i < gPlayerCount; i++) {
            if (gRacePlayers[i].unk935 == 0) {
                createFallingParticleTask();
                gCreatedTask->data18 = 0;
                break;
            }
        }
        if (gMainMenuSelectionIndex == 2) {
            if (gControllerInputRecordState & 8) gControllerInputRecordState = 0;
            else {
                gControllerInputRecordState = 4;
                D_8013AEE2 = gControllerInputRecordIndices[0];
            }
        }
        setCurrentTaskCallback(dispatchPostRaceResultsTask);
    } else updateRaceHud();
}

void dispatchPostRaceResultsTask(void) {
    u8 padding[8];
    s16 i;
    u32 score;

    if (gCdCommandActive != 0) {
        return;
    }
    queueCdCommand(3, 0);
    if (gPlayerCount == 1 || gRacePlayers[0].unk935 < gRacePlayers[1].unk935) {
        gPlayerCameras[0].unk16 = 6;
    } else {
        gPlayerCameras[1].unk16 = 6;
    }
    switch (gMainMenuSelectionIndex) {
    case 0:
        if (gPlayerCount == 1) {
            createChildTaskWithReserve(initializeResultDisplayTask);
            if (gRacePlayers[0].unk935 != 3) {
                if (gPlayerSaveRecords[0].bestPlace[gSelectedCourseIndex] == 0) {
                    gPlayerSaveRecords[0].bestPlace[gSelectedCourseIndex] = gRacePlayers[0].unk935 + 1;
                } else if (gRacePlayers[0].unk935 + 1 < gPlayerSaveRecords[0].bestPlace[gSelectedCourseIndex]) {
                    gPlayerSaveRecords[0].bestPlace[gSelectedCourseIndex] = gRacePlayers[0].unk935 + 1;
                }
                score = gRacePlayers[0].unk01C + getSelectedCoursePrize(gSelectedCourseIndex, gRacePlayers[0].unk935);
                if (gPlayerSaveRecords[0].bestScore[gSelectedCourseIndex] < score) {
                    gPlayerSaveRecords[0].bestScore[gSelectedCourseIndex] = score;
                }
            }
        } else {
            createChildTaskWithReserve(initializeTaskFade);
            for (i = 0; i < 4; i++) {
                if (gRacePlayers[i].unk935 == 0) {
                    if (++gRacePlayers[i].unk02A >= 100) {
                        gRacePlayers[i].unk02A = 99;
                    }
                }
            }
        }
        break;
    case 1:
        if (gRaceRuleset == 0) {
            if (gPlayerCount == 1) {
                createChildTaskWithReserve(initializeRaceResultsTask);
                gPlayerRaceTimes[0] = gRaceElapsedTime;
                gPlayerAttackTimeExpired[0] = gAttackTimeExpired;
            } else {
                createChildTaskWithReserve(initializeRaceTimeComparisonPanel);
            }
        } else if (gRaceRuleset == 1) {
            if (gPlayerCount == 1) {
                createChildTaskWithReserve(initializeTaskMenuFadeIn);
                gPlayerAttackTimeExpired[0] = gAttackTimeExpired;
            } else {
                createChildTaskWithReserve(initializeRaceTimeComparisonPanel);
            }
        } else if (gRaceRuleset == 2) {
            if (gPlayerCount == 1) {
                createChildTaskWithReserve(initializeStatsMenuTask);
                gPlayerAttackTimeExpired[0] = gAttackTimeExpired;
            } else {
                createChildTaskWithReserve(initializeRaceTimeComparisonPanel);
            }
        }
        break;
    case 2:
        createChildTaskWithReserve(initializeTaskMenuFade);
        break;
    }
    setControllerFlag5(0);
    setControllerFlag5(1);
    setCurrentTaskCallback(begin30TickTransitionOnFlag10);
}

void begin30TickTransitionOnFlag10(void) {
    if (gTaskTransitionFlags & 0x10) {
        gCurrentTask->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
    }
}

void initializeRaceRestartTask(void) {
    u16 i;
    D_8013AF08[1] = 0;
    D_8013AF08[0] = 0;
    gTaskTransitionFlags = 1;
    stopPlayerSound(0);
    if (gPlayerCount == 2) {
        stopPlayerSound(1);
    }
    for (i = 0; i < activeRacePlayerCountGp; i++) {
        gRacePlayers[i].isActive = 1;
    }
    initializeActiveRacePlayers();
    initializeRaceRulesAndCounters();
    setCurrentTaskCallback(initializeRacePresentationTask);
}

void advanceTaskWhenFlagAFE4Bit8Set(void) {
    if (gTaskTransitionFlags & 0x100) {
        gTaskTransitionFlags = 0;
        gGameStateFlags |= 0x90;
        setCurrentTaskCallback(initializeGameOptionViewTask);
    }
}

void startResourceLoadWhenFlagAFE4Bit9Set(void) {
    if (gTaskTransitionFlags & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    }
}

void beginResourceLoadOrUnlockNotification(void) {
    gGameStateFlags |= 0x48;
    if (gTaskTransitionFlags & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    } else {
        startUnlockNotification(gRacePlayers[0].characterId + 1);
        setCurrentTaskCallback(startResourceLoadWhenFlagAFE4Bit9Set);
    }
}

void resetStateAndBeginResourceLoadTransition(u8 requestLoad) {
    GameTask *savedTask;
    resetPendingCdRequests();
    gSelectedPlayerReplayActive = 0;
    D_8013B20C = 0;
    resetPaletteRecords();
    deleteCurrentTaskDescendants();
    savedTask = gCurrentTask;
    gCurrentTask = D_8013B304;
    deleteCurrentTaskDescendants();
    gCurrentTask = savedTask;
    gGameStateFlags |= 0x90;
    if (requestLoad) {
        gTaskTransitionFlags |= 0x200;
    }
    gTaskTransitionFlags |= 0x80;
    setCurrentTaskCallback(beginResourceLoadOrUnlockNotification);
}

void startTaskTransitionWhenFlagAEE8Set(void) {
    if (gCurrentTask->data18 == 0) {
        if (gControllerPressedButtons[0] & 0xB0) {
            gCurrentTask->data18 = 30;
            configureTaskStateTransition(2, 0, 30);
            setCurrentTaskCallback(finishTaskTransitionAfterCountdown);
        }
    } else {
        gCurrentTask->data18--;
    }
}

void finishTaskTransitionAfterCountdown(void) {
    if (gCurrentTask->data18 == 0) {
        resetPendingCdRequests();
        gGameStateFlags |= 0x90;
        gSelectedPlayerReplayActive = 0;
        D_8013B20C = 0;
        gTaskTransitionFlags &= ~0x10;
        resetPaletteRecords();
        deleteCurrentTaskDescendants();
        if (gCurrentTask->data1A == 3) {
            resetStateAndBeginResourceLoadTransition(0);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        gCurrentTask->data18--;
    }
}

void finishRaceExitAfterCountdown(void) {
    RacePlayer *player;
    GameTask *savedTask;
    s16 unlockTier;
    s32 transitionIndex;

    if (gDemoRaceIndex) {
        playbackControllerInput(0);
    }
    if (gSelectedPlayerReplayActive) {
        playbackControllerInput(0);
    }
    if (--gCurrentTask->data18 == 0) {
        stopPlayerSound(0);
        if (!gDemoRaceIndex && !D_8013B1AA && !D_8013B1AB && !gMainMenuSelectionIndex && gPlayerCount == 1) {
            player = &gRacePlayers[0];
            if (player->unk935 < 3) {
                unlockTier = updateRaceUnlockProgress(player->characterId, player->unk935);
                transitionIndex = unlockTier;
                switch (transitionIndex) {
                case 1:
                case 2:
                case 3:
                    createChildTaskForParentWithReserve(&gGameTasks[4], initializeCourseUnlockScene);
                    gCreatedTask->data18 = unlockTier;
                    gCurrentTask->data18 = 30;
                    gCurrentTask->data1A = unlockTier;
                    configureTaskStateTransition(1, 0, gCurrentTask->data18);
                    gTaskTransitionFlags |= 0x40;
                    queueCdCommand(7, 0);
                    savedTask = gCurrentTask;
                    gCurrentTask = D_8013B304;
                    deleteCurrentTaskDescendants();
                    gCurrentTask = savedTask;
                    startQuadTransitionSequenceTask(transitionIndex - 1);
                    player->unk0F8 = 8;
                    setCurrentTaskCallback(startTaskTransitionWhenFlagAEE8Set);
                    return;
                case 4:
                    {
                        s32 pairedState = gCourseOpponentCharacters[8][0];
                        if (pairedState == 9) {
                            updatePairedStateFlag(1);
                        } else if (pairedState == 8) {
                            updatePairedStateFlag(0);
                        }
                    }
                    resetStateAndBeginResourceLoadTransition(1);
                    return;
                }
            }
        }

        D_8013B1AA = 0;
        resetPendingCdRequests();
        gGameStateFlags = (gGameStateFlags | 0x1090) & ~0x2000;
        gSelectedPlayerReplayActive = 0;
        D_8013B20C = 0;
        gTaskTransitionFlags &= ~0x10;
        resetPaletteRecords();
        deleteCurrentTaskDescendants();

        if (gMainMenuSelectionIndex == 2) {
            if (gControllerInputRecordState & 4) {
                setCurrentTaskCallback(initializeDemoPlaybackTask);
                return;
            }
            gViewClipRects[0].x = 0;
            gViewClipRects[0].y = 0;
            gViewClipRects[0].w = gDisplayWidth;
            gViewClipRects[0].h = gDisplayHeight;
        }
        if (gPlayerCount == 2) {
            startThreeFrameScreenBlend();
        } else if (D_8013B1AB) {
            setCurrentTaskCallback(initializeRaceRestartTask);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        updateRaceHud();
    }
}

void updateThreeFrameScreenBlend(void) {
    if (--gCurrentTask->data18 == 0) {
        if (D_8013B1AB) {
            setCurrentTaskCallback(initializeRaceRestartTask);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        drawScreenDarkeningOverlay(255, &gMainOrderingTables[gActiveGraphicsBuffer], 0);
    }
}

void startThreeFrameScreenBlend(void) {
    drawScreenDarkeningOverlay(255, &gMainOrderingTables[gActiveGraphicsBuffer], 0);
    gCurrentTask->data18 = 3;
    setCurrentTaskCallback(updateThreeFrameScreenBlend);
}

void initializeDemoPlaybackTask(void) {
    initPlayerCamera(0);
    initializeDefaultSceneLighting();
    initializeRacePlayerEffects();
    createChildTaskForParent(&gGameTasks[3], initPlayerZeroTask);
    gCreatedTask->data18 = 0;
    initializeCourseObjects();
    createChildTask(updateRacePlayerFinishSlotsTask);
    createChildTask(initializeCurrentPlayerModelAndHeight);
    gCreatedTask->data18 = 0;
    gRacePlayers[0].task = gCreatedTask;
    configureTaskStateTransition(1, 0, 0);
    queueCdCommand(0, 1);
    D_8013AFDB = 0;
    gViewClipRects[0].x = 0;
    gViewClipRects[0].y = 32;
    gViewClipRects[0].w = gDisplayWidth;
    gViewClipRects[0].h = gDisplayHeight - 32;
    initializeRacePaletteCycles();
    gTaskTransitionFlags = 0;
    gGameStateFlags |= 0x48;
    resetSecondaryRng();
    gSelectedPlayerReplayActive = 1;
    D_8013AFE8 = gControllerHeldButtons[0];
    initializeActiveRacePlayers();
    setControllerFlag5(0);
    setControllerFlag5(1);
    gControllerInputRecordState = 0;
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 0;
    createChildTaskWithReserve(drawFixedSpriteAndBottomBarTask);
    setCurrentTaskCallback(updateSelectedPlayerPlaybackTask);
}

void updateSelectedPlayerPlaybackTask(void) {
    GameTask *savedTask;
    s16 buttons = 0;
    if (gSelectedPlayerRestoreFrames[gCurrentTask->data1A] == gCurrentTask->data18) {
        stopPlayerSound(0);
        savedTask = gCurrentTask;
        gCurrentTask = D_8013B304;
        deleteCurrentTaskDescendants();
        gCurrentTask = savedTask;
        startRacePlayerTasks();
        if ((s16)restoreSelectedPlayerState()) {
            resetGameplayRng();
            if (gCurrentTask->data1A != 0) configureTaskStateTransition(0, 0x1000, 0);
            gCurrentTask->data1A++;
            gRacePlayers[0].unk970 = 0xFFFF;
            if (gRacePlayers[0].task->next->depth != 2 && gRacePlayers[0].unk0FE) {
                if (gRacePlayers[0].unk0FC) createChildTaskForParentWithReserve(gRacePlayers[0].task, updatePlayerTimerTask);
                else createChildTaskForParentWithReserve(gRacePlayers[0].task, fadeOutPlayerTimerTask);
                gCreatedTask->data18 = 0;
            }
        } else buttons = 0x800;
    }
    gCurrentTask->data18++;
    D_8013AFE8 = gControllerHeldButtons[0];
    if (gControllerPressedButtons[0] & 0x8F0) buttons = 0x800;
    playbackControllerInput(0);
    gControllerPressedButtons[0] |= buttons;
    if (gControllerInputRecordIndices[0] == D_8013AEE2 || (gControllerPressedButtons[0] & 0x800)) {
        gCurrentTask->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        gControllerInputRecordState = 0;
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
    }
}

void initializeAlternatingVramRowTask(void) {
    gCurrentTask->data18 = 0;
    gCurrentTask->data1A = 6;
    setCurrentTaskCallback(animateAlternatingVramRow);
}

void animateAlternatingVramRow(void) {
    RECT rect;
    if (--gCurrentTask->data1A == 0) {
        rect.x = 640;
        if (gCurrentTask->data18 == 0) {
            rect.y = 503;
            gCurrentTask->data1A = 4;
        } else {
            rect.y = 504;
            gCurrentTask->data1A = 6;
        }
        rect.w = 16;
        rect.h = 1;
        MoveImage(&rect, 640, 502);
        gCurrentTask->data18 ^= 1;
    }
}

void queuePlayerResourceReadsAndUploadTextures(void) {
    u16 i;
    u32 *offset;
    for (i = 0; i < activeRacePlayerCountGp; i++) {
        queueCdSectorCommand(0, gGridResourceFirstSectorRanges[gRacePlayers[i].characterId][gRacePlayers[i].unk015[0]].sector,
            gGridResourceFirstSectorRanges[gRacePlayers[i].characterId][gRacePlayers[i].unk015[0]].sectorCount,
            (u8 *)DAT_PLAYER_MODEL_BUFFERS[i], 0);
        queueCdSectorCommand(1, gGridResourceSecondSectorRanges[gRacePlayers[i].characterId][gRacePlayers[i].unk015[0]].sector,
            gGridResourceSecondSectorRanges[gRacePlayers[i].characterId][gRacePlayers[i].unk015[0]].sectorCount,
            DAT_SCRATCH, (s32)&gPlayerTextureOrigins[i]);
        if (i < gPlayerCount && !gDemoRaceIndex && !gModeSequenceStage && !gControllerPlaybackIndex) {
            offset = &(DAT_STARTUP_TEXTURES)->offsets[gPlayerResourceTextureSelections.entries[i][gRacePlayers[i].unk015[1]]];
        } else {
            offset = &(DAT_STARTUP_TEXTURES)->offsets[gRacePlayers[i].unk015[2]];
        }
        uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *offset), gPlayerResourceTexturePositions[i]);
    }
}

void prepareRacePlayersAndQueueResources(void) {
    s32 i;
    s32 j;
    s32 opponentIndex;
    s32 available;
    u8 specialCharacters;

    gRacePauseState = 0;
    D_8013AFD4 = 0;
    D_8013AFD5 = 0;
    gSelectedPlayerReplayActive = 0;
    gRaceResultCode = 0;
    gModeSequenceStage = 0;
    gControllerPlaybackIndex = 0;
    D_8013AFCC = 0;
    for (i = 0; i < 4; i++) {
        if (i < gPlayerCount) {
            gRacePlayers[i].unk008 = 0;
        } else {
            gRacePlayers[i].unk008 = 1;
        }
        gRacePlayers[i].unk02C[0] = D_80120484[gSelectedCourseIndex];
        gRacePlayers[i].unk02C[1] = D_80120484[gSelectedCourseIndex];
        if (gPlayerCount != 1) {
            gRacePlayers[i].unk02C[0] = 0;
            gRacePlayers[i].unk02C[1] = 0;
        }
        gRacePlayers[i].unk025[3] = i;
        gRacePlayers[i].characterId = gRacePlayers[i].unk009;
        gRacePlayers[i].unk015[0] = gRacePlayers[i].unk00A[0];
        gRacePlayers[i].unk015[1] = gRacePlayers[i].unk00A[1];
        if (i < gPlayerCount) {
            gRacePlayers[i].unk015[2] = gPlayerResourceTextureSelections.entries[i][gRacePlayers[i].unk015[1]];
        }
    }

    if (gSelectedCourseIndex == 8) {
        if (gPlayerCount == 1) {
            if (gRacePlayers[0].characterId >= 8) {
                if (gRacePlayers[0].characterId == 8) {
                    gCourseOpponentCharacters[8][0] = 9;
                } else {
                    gCourseOpponentCharacters[8][0] = 8;
                }
            } else {
                if ((s16)getGlobalFlagMask(1) != 0) {
                    goto selectRandom;
                }
                gCourseOpponentCharacters[8][0] = 9;
            }
        } else {
            specialCharacters = 0;
            if (gRacePlayers[0].characterId >= 8) {
                specialCharacters = gRacePlayers[0].characterId - 7;
            }
            if (gRacePlayers[1].characterId >= 8) {
                specialCharacters |= gRacePlayers[1].characterId - 7;
            }
            if (specialCharacters != 0) {
                if (specialCharacters == 1) {
                    gCourseOpponentCharacters[8][0] = 9;
                } else if (specialCharacters == 2) {
                    gCourseOpponentCharacters[8][0] = 8;
                }
            } else {
                if ((s16)getGlobalFlagMask(1) == 0) {
                    gCourseOpponentCharacters[8][0] = 9;
                } else {
selectRandom:
                    gCourseOpponentCharacters[8][0] = 8 + (randomNextMain() & 1);
                }
            }
        }
    }

    if (gMainMenuSelectionIndex == 0) {
        s8 active = 1;
        for (i = 4; --i >= 0;) {
            gRacePlayers[i].isActive = active;
        }
        activeRacePlayerCountGp = i = 4;
        D_8013AFDE = 100;
    } else {
        for (i = 0; i < 4; i++) {
            if (i < gPlayerCount) {
                gRacePlayers[i].isActive = 1;
            } else {
                gRacePlayers[i].isActive = 0;
            }
        }
        activeRacePlayerCountGp = gPlayerCount;
        gSelectedLapCount = 1;
        D_8013AFDE = 100;
    }

    for (i = 0; i < 4; i++) {
        gRacePlayers[i].unk025[1] = 0;
        gRacePlayers[i].unk025[2] = 0;
    }

    opponentIndex = 0;
    for (i = gPlayerCount; i < activeRacePlayerCountGp; i++) {
        do {
            gRacePlayers[i].characterId = gCourseOpponentCharacters[gSelectedCourseIndex][opponentIndex++];
            available = 1;
            for (j = 0; j < gPlayerCount; j++) {
                if (gRacePlayers[i].characterId == gRacePlayers[j].characterId) {
                    available = 0;
                }
            }
        } while (!available);
        if (gRacePlayers[i].characterId < 8) {
            gRacePlayers[i].unk015[1] = gCourseOpponentBoardSelections[gSelectedCourseIndex];
            gRacePlayers[i].unk015[2] = (u8)randomNextMain() % 14u;
        } else {
            gRacePlayers[i].unk015[1] = 14;
            gRacePlayers[i].unk015[2] = 19;
        }
    }

    setCurrentTaskCallback(initializeRaceTaskState);
    queuePlayerResourceReadsAndUploadTextures();
    gCurrentTask->unk10 |= 2;
}

void initializeRaceTaskState(void) {
    gTaskTransitionFlags = 1;
    if (gMainMenuSelectionIndex == 2) {
        resetSecondaryRng();
    }
    initializeActiveRacePlayers();
    initializeRaceRulesAndCounters();
    deleteCurrentTaskSubtree();
}

void initializeSelectedCourseTransition(void) {
    gSelectedLapCount = gSelectedCourseRecords[gSelectedCourseIndex].unk02;
    if (gMainMenuSelectionIndex != 0 || gPlayerCount == 1 || D_8013AFDA == 0) {
        configureTaskStateTransition(1, 0, 30);
        setTaskBackgroundMode(3, selectedCourseMenuItemAbsolute, 0);
        setCurrentTaskCallback(prepareRacePlayersAndQueueResources);
    }
}

void armTaskCleanupOnFlagAFAABit6(void) {
    if (gGameStateFlags & 0x40) {
        setCurrentTaskCallback(processTaskWhenFlagAFAABit7Set);
    }
}

void processTaskWhenFlagAFAABit7Set(void) {
    if (gGameStateFlags & 0x80) {
        deleteCurrentTaskDescendants();
        gGameStateFlags &= ~0xC0;
        setCurrentTaskCallback(armTaskCleanupOnFlagAFAABit6);
    }
}

void armTaskCallbackResetOnFlag3(void) {
    if (gGameStateFlags & 0x8) {
        setCurrentTaskCallback(resetTaskCallbackOnFlag4);
    }
}

void resetTaskCallbackOnFlag4(void) {
    if (gGameStateFlags & 0x10) {
        deleteCurrentTaskDescendants();
        setCurrentTaskCallback(armTaskCallbackResetOnFlag3);
    }
}

void armSavedStateRestoreOnFlag3(void) {
    if (gGameStateFlags & 0x8) {
        setCurrentTaskCallback(restoreSavedStateAndHandleFlag4);
    }
}

void restoreSavedStateAndHandleFlag4(void) {
    if (gSelectedPlayerReplayActive) {
        gControllerHeldButtons[0] = D_8013AFE8;
    }
    if (gGameStateFlags & 0x10) {
        gGameStateFlags &= ~0x18;
        setCurrentTaskCallback(armSavedStateRestoreOnFlag3);
    }
}

void initializeCurrentPlayerModelObjects(u32 *model) {
    s32 i;
    GsDOBJ2 *object;
    u32 *objects;

    model++;
    objects = model;
    GsMapModelingData(objects);
    objects = ++model;
    gCurrentRacePlayer->modelPartCount = *(u8 *)objects;
    objects = ++model;
    if (!D_8013B1AB && !D_8013B562 && !selectedPlayerReplayActiveAbsolute) {
        loadCurrentPlayerModelBlock();
        if (gCurrentTask->data18) {
            offsetTmdObjectTextures(0, (TmdObject *)objects, &gPlayerRootTextureOffsets[gCurrentTask->data18]);
            for (i = 1; i < 14; i++) {
                offsetTmdObjectTextures(i, (TmdObject *)objects, &gPlayerPartTextureOffsets[gCurrentTask->data18]);
            }
        }
    }
    GsInitCoordinate2(0, &gCurrentRacePlayer->worldCoordinate);
    GsInitCoordinate2(&gCurrentRacePlayer->worldCoordinate, &gCurrentRacePlayer->modelCoordinate);
    for (i = 0; i < 14; i++) {
        GsLinkObject4(objects, &gCurrentRacePlayer->modelObjects[i], i);
        object = &gCurrentRacePlayer->modelObjects[i];
        GsInitCoordinate2(&gCurrentRacePlayer->modelCoordinate, &gCurrentRacePlayer->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &gCurrentRacePlayer->model.modelCoordinates[i];
    }
}

void setCoordinateRotation(SVECTOR *rotation, GsCOORDINATE2 *coordinate) {
    gScratchMatrix0->t[0] = coordinate->coord.t[0];
    gScratchMatrix0->t[1] = coordinate->coord.t[1];
    gScratchMatrix0->t[2] = coordinate->coord.t[2];
    RotMatrix(rotation, gScratchMatrix0);
    coordinate->coord = *gScratchMatrix0;
    coordinate->flg = 0;
}

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate) {
    gScratchMatrix0->t[0] = coordinate->coord.t[0];
    gScratchMatrix0->t[1] = coordinate->coord.t[1];
    gScratchMatrix0->t[2] = coordinate->coord.t[2];
    RotMatrixZYX(rotation, gScratchMatrix0);
    coordinate->coord = *gScratchMatrix0;
    coordinate->flg = 0;
}

u16 advanceCurrentPlayerInputSequence(u8 sequence[][2], s16 slot) {
    u8 input = gControllerHeldButtons[gCurrentTask->data18] >> 12;
    switch (gCurrentRacePlayer->unk0CA[slot]) {
    case 0:
        if (input != sequence[gCurrentRacePlayer->unk0D6[slot]][0]) {
            gCurrentRacePlayer->unk0D6[slot] = 0;
            break;
        }
        if (sequence[gCurrentRacePlayer->unk0D6[slot]][1] != 0)
            return sequence[gCurrentRacePlayer->unk0D6[slot]][1];
        gCurrentRacePlayer->unk0CA[slot]++;
    case 1:
        if (input != sequence[gCurrentRacePlayer->unk0D6[slot]][0] && input != 0) {
            gCurrentRacePlayer->unk0CA[slot] = 0;
            gCurrentRacePlayer->unk0D6[slot]++;
        }
        break;
    }
    return 0;
}
