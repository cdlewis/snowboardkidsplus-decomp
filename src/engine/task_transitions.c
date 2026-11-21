#include "game/race/player/player_results.h"
#include "psx/libapi.h"
#include "psx/libetc.h"
#include "common.h"
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/graphics/quad_animation.h"
#include "game/race/camera/race_camera.h"
#include "game/race/ui/race_hud.h"
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
#define RACE_EXTRA_RESOURCE_BUFFER ((u8 *)0x801F3000)
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
extern VECTOR *D_8013AF88 __attribute__((section(".data")));
extern MATRIX *D_8013AF98 __attribute__((section(".data")));
extern u16 D_8013AF7A __attribute__((section(".data")));
extern u16 D_8013AF7C __attribute__((section(".data")));
extern u16 D_8013AF78 __attribute__((section(".data")));
extern s16 D_8013AF7E __attribute__((section(".data")));
extern s16 D_8013AF80 __attribute__((section(".data")));
extern u16 D_8013AFAA __attribute__((section(".data")));
extern s16 D_8013B104 __attribute__((section(".data")));
extern u8 D_8013B1AA __attribute__((section(".data")));
extern u8 D_8013B1AB __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));
extern GameTask *D_8013AE8C __attribute__((section(".data")));
extern GameTask *D_8013AE90 __attribute__((section(".data")));
extern GameTask *D_8013B304 __attribute__((section(".data.player_snapshot_tasks")));

extern u8 D_8013AEE4 __attribute__((section(".data")));

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
void func_80015A30(u16 controller);
void func_8001C9F0(void);
s16 func_8008C290(s16 characterId, s16 finishPosition);
void func_8005EBC4(void);
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
        blinkMask = gameOptionPlayerModeAbsolute == 1 ? 1 : 2;
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
            D_8013AF88->vx = D_8013AF88->vz = 0x1000;
            D_8013AF88->vy = currentPlayerAbsolute->unk0F8 * 512;
            ScaleMatrix(D_800EC170[i], D_8013AF88);
        }
        GsSetLsMatrix(D_800EC170[i]);
        if (!(taskTransitionFlagsAbsolute & 0x40) || i != 0) {
            if (view == 0) {
                GsSortObject4(objects, &D_800EC068[D_8013AF78], 2, getScratchAddr(0));
            } else {
                GsSortObject4(objects, &D_800EC108[D_8013AF78], 2, getScratchAddr(0));
            }
        }
        if (i == 0) {
            if (count != 1) {
                extra = objects + 12;
                if (view == 0) {
                    GsSortObject4(extra, &D_800EC018[D_8013AF78], 2, getScratchAddr(0));
                } else {
                    GsSortObject4(extra, &D_800EC0B8[D_8013AF78], 2, getScratchAddr(0));
                }
                extra++;
                if (view == 0) {
                    GsSortObject4(extra, &D_800EC018[D_8013AF78], 2, getScratchAddr(0));
                } else {
                    GsSortObject4(extra, &D_800EC0B8[D_8013AF78], 2, getScratchAddr(0));
                }
            }
            if (currentPlayerAbsolute->unk0FE) {
                if (currentPlayerAbsolute->unk0FF) {
                    setVector(D_8013AF88, currentPlayerAbsolute->timer.unk60,
                              currentPlayerAbsolute->timer.unk60, currentPlayerAbsolute->timer.unk60);
                } else {
                    setVector(D_8013AF88, -currentPlayerAbsolute->timer.unk60,
                              currentPlayerAbsolute->timer.unk60, -currentPlayerAbsolute->timer.unk60);
                }
                ScaleMatrix(&currentPlayerAbsolute->timer.coordinate.coord, D_8013AF88);
                GsSetLsMatrix(&currentPlayerAbsolute->timer.coordinate.coord);
                if (view == 0) {
                    GsSortObject4(&gPlayerTimerModelObject, &D_800EC018[D_8013AF78], 2, getScratchAddr(0));
                } else {
                    GsSortObject4(&gPlayerTimerModelObject, &D_800EC0B8[D_8013AF78], 2, getScratchAddr(0));
                }
            }
        }
    }
    SetSp(gRacePlayerSortSavedStackPointer);
}

void sortLitObject(GsDOBJ2 *object) {
    GsGetLw(object->coord2, D_8013AF98);
    GsSetLightMatrix(D_8013AF98);
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    GsSortObject4(object, &D_800EC018[D_8013AF78], 2, (u32 *)getScratchAddr(0));
}

void initializeRacePaletteCycles(void) {
    u16 palette;
    s16 row;

    palette = saveVram16Pixels(656, 244);
    if (palette != 32) {
        startPaletteCycle(1, palette, 1, 6, 1);
    }

    switch (D_8013AFCE) {
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
        func_80014930(&gGameTasks[4], initializeAlternatingVramRowTask);
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
    D_8013AFD7 = 0;
    D_8013AE88->data18 = 0;
    func_8002EE0C();
    resetGameOptionState();
    setCurrentTaskCallback(initializeGameOptionViewTask);
    D_8013AE88->unk10 |= 2;
}

void initializeGameOptionViewTask(void) {
    initPlayerCamera(0);
    func_8003DEE0();
    setControllerFlag0(0);
    setControllerFlag0(1);
    setCurrentTaskCallback(queueRaceStartupResourcesTask);
    createChildTask(dispatchGameOptionTransitionTask);
    D_8013AE88->unk10 |= 1;
}

void queueRaceStartupResourcesTask(void) {
    u32 i;

    if (D_8013AFD7) {
        switch (D_8013AFD7) {
        case 1:
            D_8013AFCE = 9;
            D_8013AFCD = 1;
            D_8013AFE2 = 0;
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
                    player->unk02C[0] = D_80120484[D_8013AFCE];
                    player->unk02C[1] = D_80120484[D_8013AFCE];
                    player->unk025[3] = i;
                }
            }
            break;
        case 2:
            D_8013AFE2 = 1;
            D_8013AFE0 = 0;
            D_8013AFCE = 0;
            D_8013AFCD = 1;
            activeRacePlayerCountGp = 1;
            gRacePlayers[0].characterId = 4;
            gRacePlayers[0].unk015[1] = 2;
            gRacePlayers[0].isActive = 1;
            break;
        case 3:
            D_8013AFE2 = 1;
            D_8013AFE0 = 1;
            D_8013AFCE = 1;
            D_8013AFCD = 1;
            activeRacePlayerCountGp = 1;
            gRacePlayers[0].characterId = 2;
            gRacePlayers[0].unk015[1] = 1;
            gRacePlayers[0].isActive = 1;
            break;
        case 4:
            D_8013AFE2 = 1;
            D_8013AFE0 = 2;
            D_8013AFCE = 7;
            D_8013AFCD = 1;
            activeRacePlayerCountGp = 1;
            gRacePlayers[0].characterId = 1;
            gRacePlayers[0].unk015[1] = 0;
            gRacePlayers[0].isActive = 1;
            break;
        case 5:
            D_8013AFCE = 3;
            D_8013AFCD = 2;
            D_8013AFE2 = 0;
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
                    player->unk02C[0] = D_80120484[D_8013AFCE];
                    player->unk02C[1] = D_80120484[D_8013AFCE];
                    player->unk025[3] = i;
                }
            }
            break;
        }
        if (D_8013AFD7) {
            D_8013AFD0 = D_80123D80[D_8013AFCE].unk02;
        }
        resetGameplayRng();
        D_8013AFD3 = 0;
        D_8013AFD4 = 0;
        D_8013AFD5 = 0;
        D_8013AFD6 = 0;
        D_8013AFE1 = 0;
        D_8013AFD8 = 0;
        D_8013AFD9 = 0;
        D_8013AFCC = 0;
        D_8013AFE4 = 0;
        queuePlayerResourceReadsAndUploadTextures();
        initializeActiveRacePlayers();
        initializeRaceRulesAndCounters();
        D_8013B1A8 = 1;
    }

    queueCourseResourceSectorReads();
    closeMusicSequence();
    func_800180F4(1);
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
    if (D_8013AFCE != 7) {
        queueCdSectorCommand(0, DAT_ENTRY_004_SECTOR, DAT_ENTRY_004_SECTOR_COUNT,
            RACE_EXTRA_RESOURCE_BUFFER, 0);
    }
    if (D_8013AFD7) {
        queueCdSectorCommand(0, gRacePlaybackSectorRanges[D_8013AFD7 - 1].sector,
            gRacePlaybackSectorRanges[D_8013AFD7 - 1].sectorCount, D_800E959C, 0);
    }
    queueCdSectorCommand(0, DAT_ENTRY_010_SECTOR, DAT_ENTRY_010_SECTOR_COUNT, DAT_SCRATCH, 0);
    if (!D_8013AFD8 && !D_8013AFD9) {
        setCurrentTaskCallback(initializeRacePresentationTask);
    } else {
        setCurrentTaskCallback(initializeModeRaceTasks);
    }
    D_8013AE88->unk10 |= 2;
}

void initializeRacePresentationTask(void) {
    s16 i;

    setSecondaryDisplayTaskMode(0);
    if (!D_8013B1AB) {
        func_80051A68();
    }
    initializeRacePlayerEffects();
    if (D_8013AFCD == 2) {
        func_800148F4(&gGameTasks[3], initCurrentPlayerCameraTask);
        D_8013AE8C->data18 = 1;
    }
    func_800148F4(&gGameTasks[3], initCurrentPlayerCameraTask);
    D_8013AE8C->data18 = 0;
    func_80058BD4();
    createChildTask(updateRacePlayerFinishSlotsTask);
    for (i = activeRacePlayerCountGp - 1; i >= 0; i--) {
        createChildTask(initializeCurrentPlayerModelAndHeight);
        D_8013AE8C->data18 = i;
        gRacePlayers[i].task = D_8013AE8C;
    }
    configureTaskStateTransition(1, 0, 15);
    if (!D_8013AFD7) {
        queueCdCommand(1, 1);
    } else {
        gFrameCounter = 0;
    }

    D_8013AFDB = 0;
    D_800EC158[0].x = 0;
    D_800EC158[0].y = 0;
    D_800EC158[0].w = D_8013AF7A;
    D_800EC158[0].h = D_8013AF7C;

    if (D_8013AFCD == 2) {
        D_8013AF7E = 64;
        D_8013AF80 = D_8013AF7C / 4;
        D_800EC158[1].x = 0;
        D_800EC158[1].y = 0;
        D_800EC158[1].w = D_8013AF7A - 64;
        D_800EC158[1].h = D_8013AF7C / 2 - 1;
        D_800EC158[2].x = 0;
        D_800EC158[2].y = D_8013AF7C / 2 + 1;
        D_800EC158[2].w = D_8013AF7A - 64;
        D_800EC158[2].h = D_8013AF7C / 2 - 1;
    }

    initializeRacePaletteCycles();
    for (i = 0; i < D_8013AFCD; i++) {
        copyPlayerPaletteRow(i);
    }
    resetTaskControlState();
    D_8013AFAA |= 0x48;
    clearControllerFlag0(0);
    clearControllerFlag0(1);
    if (D_8013AFD7 || D_8013AEC4[0]) {
        setControllerFlag5(0);
    } else {
        clearControllerFlag5(0);
    }
    if (D_8013AFD7 || D_8013AEC4[1]) {
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
    D_8013AE88->data1A = 60;
    setCurrentTaskCallback(waitThenStartSplitScreenTransition);
}

void waitThenStartSplitScreenTransition(void) {
    GameTask *task = D_8013AE88;

    if (--task->data1A == 0) {
        task->data1A = 16;
        setCurrentTaskCallback(updateSplitScreenTransitionTask);
    }
}

void updateSplitScreenTransitionTask(void) {
    D_8013AF7E -= 4;
    D_801241E8[D_8013AE88->data18].unk10 += 16;
    D_8013AF80 -= 4;
    if (D_8013AF80 < 0) D_8013AF80 = 0;
    if (D_8013AE88->data18 == 0) {
        D_800EC158[1].h += 8;
        if (D_800EC158[1].h > 240) D_800EC158[1].h = 240;
        D_800EC158[1].w = D_8013AF7A - D_8013AF7E;
        D_800EC158[2].y += 8;
        D_800EC158[2].h -= 8;
        if (D_800EC158[2].h < 0) {
            D_800EC158[2].y = 240;
            D_800EC158[2].h = 0;
            D_8013AFAA |= 0x1000;
        }
    } else {
        D_800EC158[2].y -= 8;
        D_800EC158[2].h += 8;
        if (D_800EC158[2].h > 240) {
            D_800EC158[2].y = 0;
            D_800EC158[2].h = 240;
        }
        D_800EC158[2].w = D_8013AF7A - D_8013AF7E;
        D_800EC158[1].h -= 8;
        if (D_800EC158[1].h < 0) {
            D_800EC158[1].h = 0;
            D_8013AFAA |= 0x5000;
        }
    }
    if (--D_8013AE88->data1A == 0) deleteCurrentTaskSubtree();
}

void updateActiveRaceTask(void) {
    u16 i;
    u16 finishedCount;
    s32 pauseRequested;

    if (D_8013AFD3 & RACE_PAUSE_RESUME_PENDING) {
        D_8013AFD3 = 0;
    }
    updateRaceHud();
    if (!(D_8013AFE4 & 1) && D_8013B1AB) {
        D_8013AE88->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        setControllerFlag5(0);
        setControllerFlag5(1);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
        return;
    }
    if (D_8013B1AA) {
        D_8013AE88->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        if (D_8013AFE2 == 2) {
            D_8013AEE4 = 0;
        }
        setControllerFlag5(0);
        setControllerFlag5(1);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
        return;
    }

    updateCpuRacerSpeedControl();
    finishedCount = 0;
    for (i = 0; i < D_8013AFCD; i++) {
        if (gRacePlayers[i].unk128 & 0x40) {
            finishedCount++;
        }
    }
    if (finishedCount == D_8013AFCD) {
        queueCdCommand(2, 1);
        if (D_8013AFE2 == 0) {
            if (D_8013AFCD == 2) {
                if (!(gRacePlayers[0].unk100[0] | gRacePlayers[1].unk100[0])) {
                    return;
                }
                createChildTaskWithReserve(beginTask60TickCountdown);
                if (gRacePlayers[0].unk100[0] < gRacePlayers[1].unk100[0]) {
                    D_8013AE8C->data18 = 0;
                } else {
                    D_8013AE8C->data18 = 1;
                }
            }
            D_8013AFE1 = 1;
        } else if (D_8013AFCD == 2) {
            if (!(gRacePlayers[0].unk100[0] | gRacePlayers[1].unk100[0])) {
                return;
            }
            updateAttackResultsAndRecords();
            createChildTaskWithReserve(beginTask60TickCountdown);
            if (gRacePlayers[0].unk100[0] < gRacePlayers[1].unk100[0]) {
                D_8013AE8C->data18 = 0;
            } else {
                D_8013AE8C->data18 = 1;
            }
        }

        if (D_8013AFE2 == 2 && (D_8013AEE4 & 2)) {
            func_80015A30(0);
        }
        D_8013AE88->data18 = 60;
        setCurrentTaskCallback(finishRaceTransitionAfterCountdown);
        return;
    }

    if (!(D_8013AFE4 & 1)) {
        if (D_8013AFD3) {
            goto pause;
        }
        if (!D_8013AFD7) {
            if (D_8013AFCD == 1) {
                if ((D_8013AEE8[0] & RACE_PAUSE_START) || !isControllerInputSupported(0)) {
                    playSoundEffect(0, 0, 32);
                    D_8013AFD3 = 1;
                    func_80014930(&gGameTasks[4], initializeRacePauseMenuTask);
                }
            } else {
                pauseRequested = 0;
                if ((D_8013AEE8[0] & RACE_PAUSE_START) || !isControllerInputSupported(0) ||
                    (D_8013AEE8[1] & RACE_PAUSE_START) || !isControllerInputSupported(1)) {
                    pauseRequested = 1;
                }
                if (pauseRequested) {
                    playSoundEffect(0, 0, 32);
                    D_8013AFD3 = ((D_8013AEE8[0] & RACE_PAUSE_START) ||
                                  !isControllerInputSupported(0)) ? 1 : 2;
                    func_80014930(&gGameTasks[1], initializeRacePauseMenuTask);
                }
            }
        }
    }
    if (D_8013AFD3) {
        goto pause;
    }
    if (D_8013AFE2 == 2 && (D_8013AEE4 & 2)) {
        func_80015A30(0);
    }
    if (D_8013AFD7) {
        if (D_8013AEE4 & 2) {
            if (gFrameCounter < RACE_DEMO_FRAME_LIMIT) {
                drawStretchedDebugText(4, 4, gRaceVsyncDebugFormat, gFrameCounter);
                func_80015A30(0);
                return;
            }
        } else if (gFrameCounter < RACE_DEMO_FRAME_LIMIT &&
                   !(D_8013AEE8[0] & RACE_PAUSE_START)) {
            goto continueDemo;
        }
        D_8013AE88->data18 = 45;
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
    D_8013AE88 = D_8013AE90->previous;
}

void finishRaceTransitionAfterCountdown(void) {
    s16 i;
    if (D_8013AFE2 == 2 && (D_8013AEE4 & 2)) func_80015A30(0);
    if (--D_8013AE88->data18 == 0) {
        D_8013B1A8 = 0;
        D_8013AFE4 |= 8;
        if (D_8013AFCD == 1) updateAttackResultsAndRecords();
        for (i = 0; i < D_8013AFCD; i++) {
            if (gRacePlayers[i].unk935 == 0) {
                createFallingParticleTask();
                D_8013AE8C->data18 = 0;
                break;
            }
        }
        if (D_8013AFE2 == 2) {
            if (D_8013AEE4 & 8) D_8013AEE4 = 0;
            else {
                D_8013AEE4 = 4;
                D_8013AEE2 = D_8013AEDC[0];
            }
        }
        setCurrentTaskCallback(func_8001C9F0);
    } else updateRaceHud();
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001C9F0);

void begin30TickTransitionOnFlag10(void) {
    if (D_8013AFE4 & 0x10) {
        D_8013AE88->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
    }
}

void initializeRaceRestartTask(void) {
    u16 i;
    D_8013AF08[1] = 0;
    D_8013AF08[0] = 0;
    D_8013AFE4 = 1;
    stopPlayerSound(0);
    if (D_8013AFCD == 2) {
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
    if (D_8013AFE4 & 0x100) {
        D_8013AFE4 = 0;
        D_8013AFAA |= 0x90;
        setCurrentTaskCallback(initializeGameOptionViewTask);
    }
}

void startResourceLoadWhenFlagAFE4Bit9Set(void) {
    if (D_8013AFE4 & 0x200) {
        createResourceLoadTask();
        setCurrentTaskCallback(advanceTaskWhenFlagAFE4Bit8Set);
    }
}

void beginResourceLoadOrUnlockNotification(void) {
    D_8013AFAA |= 0x48;
    if (D_8013AFE4 & 0x200) {
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
    D_8013AFD6 = 0;
    D_8013B20C = 0;
    resetPaletteRecords();
    deleteCurrentTaskDescendants();
    savedTask = D_8013AE88;
    D_8013AE88 = D_8013B304;
    deleteCurrentTaskDescendants();
    D_8013AE88 = savedTask;
    D_8013AFAA |= 0x90;
    if (requestLoad) {
        D_8013AFE4 |= 0x200;
    }
    D_8013AFE4 |= 0x80;
    setCurrentTaskCallback(beginResourceLoadOrUnlockNotification);
}

void startTaskTransitionWhenFlagAEE8Set(void) {
    if (D_8013AE88->data18 == 0) {
        if (D_8013AEE8[0] & 0xB0) {
            D_8013AE88->data18 = 30;
            configureTaskStateTransition(2, 0, 30);
            setCurrentTaskCallback(finishTaskTransitionAfterCountdown);
        }
    } else {
        D_8013AE88->data18--;
    }
}

void finishTaskTransitionAfterCountdown(void) {
    if (D_8013AE88->data18 == 0) {
        resetPendingCdRequests();
        D_8013AFAA |= 0x90;
        D_8013AFD6 = 0;
        D_8013B20C = 0;
        D_8013AFE4 &= ~0x10;
        resetPaletteRecords();
        deleteCurrentTaskDescendants();
        if (D_8013AE88->data1A == 3) {
            resetStateAndBeginResourceLoadTransition(0);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        D_8013AE88->data18--;
    }
}

void finishRaceExitAfterCountdown(void) {
    RacePlayer *player;
    GameTask *savedTask;
    s16 unlockTier;
    s32 transitionIndex;

    if (D_8013AFD7) {
        playbackControllerInput(0);
    }
    if (D_8013AFD6) {
        playbackControllerInput(0);
    }
    if (--D_8013AE88->data18 == 0) {
        stopPlayerSound(0);
        if (!D_8013AFD7 && !D_8013B1AA && !D_8013B1AB && !D_8013AFE2 && D_8013AFCD == 1) {
            player = &gRacePlayers[0];
            if (player->unk935 < 3) {
                unlockTier = func_8008C290(player->characterId, player->unk935);
                transitionIndex = unlockTier;
                switch (transitionIndex) {
                case 1:
                case 2:
                case 3:
                    func_80014930(&gGameTasks[4], func_8005EBC4);
                    D_8013AE8C->data18 = unlockTier;
                    D_8013AE88->data18 = 30;
                    D_8013AE88->data1A = unlockTier;
                    configureTaskStateTransition(1, 0, D_8013AE88->data18);
                    D_8013AFE4 |= 0x40;
                    queueCdCommand(7, 0);
                    savedTask = D_8013AE88;
                    D_8013AE88 = D_8013B304;
                    deleteCurrentTaskDescendants();
                    D_8013AE88 = savedTask;
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
        D_8013AFAA = (D_8013AFAA | 0x1090) & ~0x2000;
        D_8013AFD6 = 0;
        D_8013B20C = 0;
        D_8013AFE4 &= ~0x10;
        resetPaletteRecords();
        deleteCurrentTaskDescendants();

        if (D_8013AFE2 == 2) {
            if (D_8013AEE4 & 4) {
                setCurrentTaskCallback(initializeDemoPlaybackTask);
                return;
            }
            D_800EC158[0].x = 0;
            D_800EC158[0].y = 0;
            D_800EC158[0].w = D_8013AF7A;
            D_800EC158[0].h = D_8013AF7C;
        }
        if (D_8013AFCD == 2) {
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
    if (--D_8013AE88->data18 == 0) {
        if (D_8013B1AB) {
            setCurrentTaskCallback(initializeRaceRestartTask);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    }
}

void startThreeFrameScreenBlend(void) {
    func_80070C5C(255, &D_800EBFF0[D_8013AF78], 0);
    D_8013AE88->data18 = 3;
    setCurrentTaskCallback(updateThreeFrameScreenBlend);
}

void initializeDemoPlaybackTask(void) {
    initPlayerCamera(0);
    func_8003DEE0();
    initializeRacePlayerEffects();
    func_800148F4(&gGameTasks[3], initPlayerZeroTask);
    D_8013AE8C->data18 = 0;
    func_80058BD4();
    createChildTask(updateRacePlayerFinishSlotsTask);
    createChildTask(initializeCurrentPlayerModelAndHeight);
    D_8013AE8C->data18 = 0;
    gRacePlayers[0].task = D_8013AE8C;
    configureTaskStateTransition(1, 0, 0);
    queueCdCommand(0, 1);
    D_8013AFDB = 0;
    D_800EC158[0].x = 0;
    D_800EC158[0].y = 32;
    D_800EC158[0].w = D_8013AF7A;
    D_800EC158[0].h = D_8013AF7C - 32;
    initializeRacePaletteCycles();
    D_8013AFE4 = 0;
    D_8013AFAA |= 0x48;
    resetSecondaryRng();
    D_8013AFD6 = 1;
    D_8013AFE8 = D_8013AEEC[0];
    initializeActiveRacePlayers();
    setControllerFlag5(0);
    setControllerFlag5(1);
    D_8013AEE4 = 0;
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 0;
    createChildTaskWithReserve(drawFixedSpriteAndBottomBarTask);
    setCurrentTaskCallback(updateSelectedPlayerPlaybackTask);
}

void updateSelectedPlayerPlaybackTask(void) {
    GameTask *savedTask;
    s16 buttons = 0;
    if (gSelectedPlayerRestoreFrames[D_8013AE88->data1A] == D_8013AE88->data18) {
        stopPlayerSound(0);
        savedTask = D_8013AE88;
        D_8013AE88 = D_8013B304;
        deleteCurrentTaskDescendants();
        D_8013AE88 = savedTask;
        startRacePlayerTasks();
        if ((s16)restoreSelectedPlayerState()) {
            resetGameplayRng();
            if (D_8013AE88->data1A != 0) configureTaskStateTransition(0, 0x1000, 0);
            D_8013AE88->data1A++;
            gRacePlayers[0].unk970 = 0xFFFF;
            if (gRacePlayers[0].task->next->depth != 2 && gRacePlayers[0].unk0FE) {
                if (gRacePlayers[0].unk0FC) func_80014930(gRacePlayers[0].task, updatePlayerTimerTask);
                else func_80014930(gRacePlayers[0].task, fadeOutPlayerTimerTask);
                D_8013AE8C->data18 = 0;
            }
        } else buttons = 0x800;
    }
    D_8013AE88->data18++;
    D_8013AFE8 = D_8013AEEC[0];
    if (D_8013AEE8[0] & 0x8F0) buttons = 0x800;
    playbackControllerInput(0);
    D_8013AEE8[0] |= buttons;
    if (D_8013AEDC[0] == D_8013AEE2 || (D_8013AEE8[0] & 0x800)) {
        D_8013AE88->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        D_8013AEE4 = 0;
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(finishRaceExitAfterCountdown);
    }
}

void initializeAlternatingVramRowTask(void) {
    D_8013AE88->data18 = 0;
    D_8013AE88->data1A = 6;
    setCurrentTaskCallback(animateAlternatingVramRow);
}

void animateAlternatingVramRow(void) {
    RECT rect;
    if (--D_8013AE88->data1A == 0) {
        rect.x = 640;
        if (D_8013AE88->data18 == 0) {
            rect.y = 503;
            D_8013AE88->data1A = 4;
        } else {
            rect.y = 504;
            D_8013AE88->data1A = 6;
        }
        rect.w = 16;
        rect.h = 1;
        MoveImage(&rect, 640, 502);
        D_8013AE88->data18 ^= 1;
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
            DAT_SCRATCH, (s32)&D_801204D4[i]);
        if (i < D_8013AFCD && !D_8013AFD7 && !D_8013AFD8 && !D_8013AFD9) {
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

    D_8013AFD3 = 0;
    D_8013AFD4 = 0;
    D_8013AFD5 = 0;
    D_8013AFD6 = 0;
    D_8013AFE1 = 0;
    D_8013AFD8 = 0;
    D_8013AFD9 = 0;
    D_8013AFCC = 0;
    for (i = 0; i < 4; i++) {
        if (i < D_8013AFCD) {
            gRacePlayers[i].unk008 = 0;
        } else {
            gRacePlayers[i].unk008 = 1;
        }
        gRacePlayers[i].unk02C[0] = D_80120484[D_8013AFCE];
        gRacePlayers[i].unk02C[1] = D_80120484[D_8013AFCE];
        if (D_8013AFCD != 1) {
            gRacePlayers[i].unk02C[0] = 0;
            gRacePlayers[i].unk02C[1] = 0;
        }
        gRacePlayers[i].unk025[3] = i;
        gRacePlayers[i].characterId = gRacePlayers[i].unk009;
        gRacePlayers[i].unk015[0] = gRacePlayers[i].unk00A[0];
        gRacePlayers[i].unk015[1] = gRacePlayers[i].unk00A[1];
        if (i < D_8013AFCD) {
            gRacePlayers[i].unk015[2] = gPlayerResourceTextureSelections.entries[i][gRacePlayers[i].unk015[1]];
        }
    }

    if (D_8013AFCE == 8) {
        if (D_8013AFCD == 1) {
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

    if (D_8013AFE2 == 0) {
        s8 active = 1;
        for (i = 4; --i >= 0;) {
            gRacePlayers[i].isActive = active;
        }
        activeRacePlayerCountGp = i = 4;
        D_8013AFDE = 100;
    } else {
        for (i = 0; i < 4; i++) {
            if (i < D_8013AFCD) {
                gRacePlayers[i].isActive = 1;
            } else {
                gRacePlayers[i].isActive = 0;
            }
        }
        activeRacePlayerCountGp = D_8013AFCD;
        D_8013AFD0 = 1;
        D_8013AFDE = 100;
    }

    for (i = 0; i < 4; i++) {
        gRacePlayers[i].unk025[1] = 0;
        gRacePlayers[i].unk025[2] = 0;
    }

    opponentIndex = 0;
    for (i = D_8013AFCD; i < activeRacePlayerCountGp; i++) {
        do {
            gRacePlayers[i].characterId = gCourseOpponentCharacters[D_8013AFCE][opponentIndex++];
            available = 1;
            for (j = 0; j < D_8013AFCD; j++) {
                if (gRacePlayers[i].characterId == gRacePlayers[j].characterId) {
                    available = 0;
                }
            }
        } while (!available);
        if (gRacePlayers[i].characterId < 8) {
            gRacePlayers[i].unk015[1] = gCourseOpponentBoardSelections[D_8013AFCE];
            gRacePlayers[i].unk015[2] = (u8)randomNextMain() % 14u;
        } else {
            gRacePlayers[i].unk015[1] = 14;
            gRacePlayers[i].unk015[2] = 19;
        }
    }

    setCurrentTaskCallback(initializeRaceTaskState);
    queuePlayerResourceReadsAndUploadTextures();
    D_8013AE88->unk10 |= 2;
}

void initializeRaceTaskState(void) {
    D_8013AFE4 = 1;
    if (D_8013AFE2 == 2) {
        resetSecondaryRng();
    }
    initializeActiveRacePlayers();
    initializeRaceRulesAndCounters();
    deleteCurrentTaskSubtree();
}

void initializeSelectedCourseTransition(void) {
    D_8013AFD0 = D_80123D80[D_8013AFCE].unk02;
    if (D_8013AFE2 != 0 || D_8013AFCD == 1 || D_8013AFDA == 0) {
        configureTaskStateTransition(1, 0, 30);
        setTaskBackgroundMode(3, D_8013B104, 0);
        setCurrentTaskCallback(prepareRacePlayersAndQueueResources);
    }
}

void armTaskCleanupOnFlagAFAABit6(void) {
    if (D_8013AFAA & 0x40) {
        setCurrentTaskCallback(processTaskWhenFlagAFAABit7Set);
    }
}

void processTaskWhenFlagAFAABit7Set(void) {
    if (D_8013AFAA & 0x80) {
        deleteCurrentTaskDescendants();
        D_8013AFAA &= ~0xC0;
        setCurrentTaskCallback(armTaskCleanupOnFlagAFAABit6);
    }
}

void armTaskCallbackResetOnFlag3(void) {
    if (D_8013AFAA & 0x8) {
        setCurrentTaskCallback(resetTaskCallbackOnFlag4);
    }
}

void resetTaskCallbackOnFlag4(void) {
    if (D_8013AFAA & 0x10) {
        deleteCurrentTaskDescendants();
        setCurrentTaskCallback(armTaskCallbackResetOnFlag3);
    }
}

void armSavedStateRestoreOnFlag3(void) {
    if (D_8013AFAA & 0x8) {
        setCurrentTaskCallback(restoreSavedStateAndHandleFlag4);
    }
}

void restoreSavedStateAndHandleFlag4(void) {
    if (D_8013AFD6) {
        D_8013AEEC[0] = D_8013AFE8;
    }
    if (D_8013AFAA & 0x10) {
        D_8013AFAA &= ~0x18;
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
    D_8013B0D8->modelPartCount = *(u8 *)objects;
    objects = ++model;
    if (!D_8013B1AB && !D_8013B562 && !raceFinishFlagAbsolute) {
        loadCurrentPlayerModelBlock();
        if (D_8013AE88->data18) {
            offsetTmdObjectTextures(0, (TmdObject *)objects, &gPlayerRootTextureOffsets[D_8013AE88->data18]);
            for (i = 1; i < 14; i++) {
                offsetTmdObjectTextures(i, (TmdObject *)objects, &gPlayerPartTextureOffsets[D_8013AE88->data18]);
            }
        }
    }
    GsInitCoordinate2(0, &D_8013B0D8->worldCoordinate);
    GsInitCoordinate2(&D_8013B0D8->worldCoordinate, &D_8013B0D8->modelCoordinate);
    for (i = 0; i < 14; i++) {
        GsLinkObject4(objects, &D_8013B0D8->modelObjects[i], i);
        object = &D_8013B0D8->modelObjects[i];
        GsInitCoordinate2(&D_8013B0D8->modelCoordinate, &D_8013B0D8->model.modelCoordinates[i]);
        object->attribute = 0;
        object->coord2 = &D_8013B0D8->model.modelCoordinates[i];
    }
}

void setCoordinateRotation(SVECTOR *rotation, GsCOORDINATE2 *coordinate) {
    D_8013AF98->t[0] = coordinate->coord.t[0];
    D_8013AF98->t[1] = coordinate->coord.t[1];
    D_8013AF98->t[2] = coordinate->coord.t[2];
    RotMatrix(rotation, D_8013AF98);
    coordinate->coord = *D_8013AF98;
    coordinate->flg = 0;
}

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate) {
    D_8013AF98->t[0] = coordinate->coord.t[0];
    D_8013AF98->t[1] = coordinate->coord.t[1];
    D_8013AF98->t[2] = coordinate->coord.t[2];
    RotMatrixZYX(rotation, D_8013AF98);
    coordinate->coord = *D_8013AF98;
    coordinate->flg = 0;
}

u16 advanceCurrentPlayerInputSequence(u8 sequence[][2], s16 slot) {
    u8 input = D_8013AEEC[D_8013AE88->data18] >> 12;
    switch (D_8013B0D8->unk0CA[slot]) {
    case 0:
        if (input != sequence[D_8013B0D8->unk0D6[slot]][0]) {
            D_8013B0D8->unk0D6[slot] = 0;
            break;
        }
        if (sequence[D_8013B0D8->unk0D6[slot]][1] != 0)
            return sequence[D_8013B0D8->unk0D6[slot]][1];
        D_8013B0D8->unk0CA[slot]++;
    case 1:
        if (input != sequence[D_8013B0D8->unk0D6[slot]][0] && input != 0) {
            D_8013B0D8->unk0CA[slot] = 0;
            D_8013B0D8->unk0D6[slot]++;
        }
        break;
    }
    return 0;
}
