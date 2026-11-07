#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/engine/task.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/race/camera/race_camera.h"
#include "game/engine/linear_random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/io/cd_queue.h"
#define CONTROLLER_STATE_ABSOLUTE_ADDRESSING
#include "game/io/controller.h"
#include "game/race/player/race_player.h"
#include "game/race/player/player_resource_textures.h"
#define GAME_OPTIONS_GP_ADDRESSING
#include "game/engine/game_options.h"
#define SELECTED_RECORDS_GP_ADDRESSING
#define SELECTED_RECORD_COUNTER_GP_ADDRESSING
#include "game/menu/selected_records.h"
#include "psx/libsnd.h"
#include "psx/libcd.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libetc.h"
#include "psx/libmcrd.h"
#include "psx/libcd.h"
#include "psx/libpress.h"

#define GLOBAL_FLAGS_GP_ADDRESSING
#include "game/save/save_data.h"
#include "game/engine/game_loop.h"
#include "game/engine/system_runtime.h"

#include "game/race/player/player_resource_textures.h"

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

extern u16 D_8013AFAA __attribute__((section(".data")));
extern u8 D_8013B562 __attribute__((section(".data")));
extern u8 D_80120EA4[];
extern s16 D_8013B104 __attribute__((section(".data")));
extern u8 D_8013AFE2;
extern u8 D_8013B1AB __attribute__((section(".data")));
extern u16 D_8013AFE8;


void func_800131B4(void);
void func_80019D3C(s32 movieIndex);
void func_8002EE0C(void);
void finishTaskTransitionAfterCountdown(void);
void updateThreeFrameScreenBlend(void);
void func_8001DB7C(void);
void resetStateAndBeginResourceLoadTransition(u8 loadResources);
void initializeRaceRestartTask(void);
void func_8001BD50(void);
void func_8001C0DC(void);
void func_8003DEE0(void);
void func_8001B858(void);
s32 func_80014838(TaskCallback callback);
void func_8001D100(void);
void advanceTaskWhenFlagAFE4Bit8Set(void);
void startResourceLoadWhenFlagAFE4Bit9Set(void);
void func_80015A30(u16 controller);
void func_8003CAF0(void);
void func_80040D5C(void);
void func_8001C9F0(void);
void func_80060FE4(void);
void func_80058BD4(void);
void func_8001B4EC(void);

void runMovieAndGameLoop(void) {
    func_800131B4();
    while (1) {
        initializeGameSystems();
        func_80019D3C(0);
        func_80019D3C(1);
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
        D_8013B0D8->unk13E[i] = D_80120200[i];
        D_8013B0D8->unk14C[i] = D_80120210[i];
    }
    values = D_801203C4[D_8013B0D8->characterId];
    player = D_8013B0D8;
    for (i = 0; i < 14; i++) {
        player->unk5BC[i].x = player->unk5BC[i].y = player->unk5BC[i].z = 0;
        player->unk62C[i].x = *values++;
        player->unk62C[i].y = -*values++;
        player->unk62C[i].z = -*values++;
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001AF64);

void sortLitObject(GsDOBJ2 *object) {
    GsGetLw(object->coord2, D_8013AF98);
    GsSetLightMatrix(D_8013AF98);
    GsGetLs(object->coord2, D_8013AF98);
    GsSetLsMatrix(D_8013AF98);
    GsSortObject4(object, &D_800EC018[D_8013AF78], 2, (u32 *)0x1F800000);
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001B4EC);

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
    setCurrentTaskCallback(func_8001B858);
    func_80014838(dispatchGameOptionTransitionTask);
    D_8013AE88->unk10 |= 1;
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001B858);

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001BD50);

void beginTask60TickCountdown(void) {
    D_8013AE88->data1A = 60;
    setCurrentTaskCallback(func_8001C0DC);
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001C0DC);

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
    if (--D_8013AE88->data1A == 0) func_800149A8();
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001C2E0);

void finishRaceTransitionAfterCountdown(void) {
    s16 i;
    if (D_8013AFE2 == 2 && (D_8013AEE4 & 2)) func_80015A30(0);
    if (--D_8013AE88->data18 == 0) {
        D_8013B1A8 = 0;
        D_8013AFE4 |= 8;
        if (gameOptionPlayerModeGp == 1) func_8003CAF0();
        for (i = 0; i < gameOptionPlayerModeGp; i++) {
            if (D_80120E90[i].unk935 == 0) {
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
    } else func_80040D5C();
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001C9F0);

void begin30TickTransitionOnFlag10(void) {
    if (D_8013AFE4 & 0x10) {
        D_8013AE88->data18 = 30;
        configureTaskStateTransition(2, 0, 30);
        startCdVolumeFade(103, 26, 0, 1);
        setCurrentTaskCallback(func_8001D100);
    }
}

void initializeRaceRestartTask(void) {
    u16 i;
    D_8013AF08[1] = 0;
    D_8013AF08[0] = 0;
    D_8013AFE4 = 1;
    stopPlayerSound(0);
    if (gameOptionPlayerModeGp == 2) {
        stopPlayerSound(1);
    }
    for (i = 0; i < activeRacePlayerCountGp; i++) {
        D_80120E90[i].isActive = 1;
    }
    initializeActiveRacePlayers();
    initializeRaceRulesAndCounters();
    setCurrentTaskCallback(func_8001BD50);
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
        startUnlockNotification(D_80120EA4[0] + 1);
        setCurrentTaskCallback(startResourceLoadWhenFlagAFE4Bit9Set);
    }
}

void resetStateAndBeginResourceLoadTransition(u8 requestLoad) {
    GameTask *savedTask;
    resetPendingCdRequests();
    D_8013AFD6 = 0;
    D_8013B20C = 0;
    resetPaletteRecords();
    func_800149E0();
    savedTask = D_8013AE88;
    D_8013AE88 = D_8013B304;
    func_800149E0();
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
        func_800149E0();
        if (D_8013AE88->data1A == 3) {
            resetStateAndBeginResourceLoadTransition(0);
        } else {
            setCurrentTaskCallback(initializeGameOptionViewTask);
        }
    } else {
        D_8013AE88->data18--;
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001D100);

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
    func_80060FE4();
    func_800148F4(&gGameTasks[3], initPlayerZeroTask);
    D_8013AE8C->data18 = 0;
    func_80058BD4();
    func_80014838(updateRacePlayerFinishSlotsTask);
    func_80014838(initializeCurrentPlayerModelAndHeight);
    D_8013AE8C->data18 = 0;
    D_80120E90[0].task = D_8013AE8C;
    configureTaskStateTransition(1, 0, 0);
    queueCdCommand(0, 1);
    D_8013AFDB = 0;
    D_800EC158[0].x = 0;
    D_800EC158[0].y = 32;
    D_800EC158[0].w = D_8013AF7A;
    D_800EC158[0].h = D_8013AF7C - 32;
    func_8001B4EC();
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
    func_80014874(drawFixedSpriteAndBottomBarTask);
    setCurrentTaskCallback(updateSelectedPlayerPlaybackTask);
}

void updateSelectedPlayerPlaybackTask(void) {
    GameTask *savedTask;
    s16 buttons = 0;
    if (gSelectedPlayerRestoreFrames[D_8013AE88->data1A] == D_8013AE88->data18) {
        stopPlayerSound(0);
        savedTask = D_8013AE88;
        D_8013AE88 = D_8013B304;
        func_800149E0();
        D_8013AE88 = savedTask;
        startRacePlayerTasks();
        if ((s16)restoreSelectedPlayerState()) {
            resetGameplayRng();
            if (D_8013AE88->data1A != 0) configureTaskStateTransition(0, 0x1000, 0);
            D_8013AE88->data1A++;
            D_80120E90[0].unk970 = 0xFFFF;
            if (D_80120E90[0].task->next->depth != 2 && D_80120E90[0].unk0FE) {
                if (D_80120E90[0].unk0FC) func_80014930(D_80120E90[0].task, updatePlayerTimerTask);
                else func_80014930(D_80120E90[0].task, fadeOutPlayerTimerTask);
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
        setCurrentTaskCallback(func_8001D100);
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001D89C);

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
        queueCdSectorCommand(0, gGridResourceFirstSectorRanges[D_80120E90[i].characterId][D_80120E90[i].unk015[0]].sector,
            gGridResourceFirstSectorRanges[D_80120E90[i].characterId][D_80120E90[i].unk015[0]].sectorCount,
            (u8 *)DAT_PLAYER_MODEL_BUFFERS[i], 0);
        queueCdSectorCommand(1, gGridResourceSecondSectorRanges[D_80120E90[i].characterId][D_80120E90[i].unk015[0]].sector,
            gGridResourceSecondSectorRanges[D_80120E90[i].characterId][D_80120E90[i].unk015[0]].sectorCount,
            DAT_SCRATCH, (s32)&D_801204D4[i]);
        if (i < gameOptionPlayerModeGp && !D_8013AFD7 && !D_8013AFD8 && !D_8013AFD9) {
            offset = &(DAT_STARTUP_TEXTURES)->offsets[gPlayerResourceTextureSelections.entries[i][D_80120E90[i].unk015[1]]];
        } else {
            offset = &(DAT_STARTUP_TEXTURES)->offsets[D_80120E90[i].unk015[2]];
        }
        uploadTimImageWithOffset((u32 *)((u8 *)DAT_STARTUP_TEXTURES + *offset), gPlayerResourceTexturePositions[i]);
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001DB7C);

void initializeRaceTaskState(void) {
    D_8013AFE4 = 1;
    if (D_8013AFE2 == 2) {
        resetSecondaryRng();
    }
    initializeActiveRacePlayers();
    initializeRaceRulesAndCounters();
    func_800149A8();
}

void initializeSelectedCourseTransition(void) {
    D_8013AFD0 = D_80123D80[D_8013AFCE].unk02;
    if (D_8013AFE2 != 0 || D_8013AFCD == 1 || D_8013AFDA == 0) {
        configureTaskStateTransition(1, 0, 30);
        setTaskBackgroundMode(3, D_8013B104, 0);
        setCurrentTaskCallback(func_8001DB7C);
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001E0C0);

void processTaskWhenFlagAFAABit7Set(void) {
    if (D_8013AFAA & 0x80) {
        func_800149E0();
        D_8013AFAA &= ~0xC0;
        setCurrentTaskCallback(func_8001E0C0);
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001E148);

void resetTaskCallbackOnFlag4(void) {
    if (D_8013AFAA & 0x10) {
        func_800149E0();
        setCurrentTaskCallback(func_8001E148);
    }
}

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001E1B8);

void restoreSavedStateAndHandleFlag4(void) {
    if (D_8013AFD6) {
        D_8013AEEC[0] = D_8013AFE8;
    }
    if (D_8013AFAA & 0x10) {
        D_8013AFAA &= ~0x18;
        setCurrentTaskCallback(func_8001E1B8);
    }
}

/* Model setup uses the GP-relative current player pointer. */
#undef D_8013B0D8

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

INCLUDE_ASM("asm/nonmatchings/engine/task_transitions", func_8001E3E4);

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
