#include "common.h"
#define TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define RACE_PLAYER_STATE_FLAG_ABSOLUTE_ADDRESSING
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#define PLAYER_STATE_RESULT_ABSOLUTE_ADDRESSING
#include "game/race/player/race_player.h"
#include "game/race/camera/race_camera.h"
#include "game/menu/selected_record.h"
#include "game/graphics/graphics.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_offset.h"
#include "game/graphics/quad_animation.h"
#include "game/engine/angles.h"
#include "game/engine/linear_random.h"
#include "game/engine/random.h"
#include "game/engine/offset_archive.h"
#include "game/audio/music.h"
#include "game/save/save_data.h"
#include "game/engine/game_options.h"
#include "game/menu/selected_records.h"
#include "psx/libsnd.h"
#include "player_update_internal.h"
#include "game/io/dat_resources.h"
#include "../race_presentation_internal.h"


typedef struct PlayerVariantAttributeTable {
    u16 values[15][9];
    u16 padding;
} PlayerVariantAttributeTable;

PlayerVariantAttributeTable gPlayerVariantAttributes
    __attribute__((section(".data.player_variant_attributes"), aligned(4))) = {
    {
        {0x2B60, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2A00, 0x001F, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2C00, 0x001F, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
        {0x2A60, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2C00, 0x001F, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2D00, 0x001F, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
        {0x2E00, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2E00, 0x001F, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2F00, 0x001F, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
        {0x2E80, 0x03E8, 0x00C0, 0x0FB0, 0x0200, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2B00, 0x0000, 0x0000, 0x0BF0, 0x0200, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2F00, 0x001F, 0x00D0, 0x0CD0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
        {0x2CF0, 0x001F, 0x0070, 0x0FB0, 0x01F0, 0x0008, 0x0010, 0x0030, 0x0030},
        {0x2EA9, 0x002E, 0x0076, 0x0DE0, 0x0160, 0x0009, 0x0011, 0x0030, 0x0030},
        {0x2F2A, 0x003E, 0x0080, 0x0B90, 0x00D0, 0x000A, 0x0012, 0x0030, 0x0050},
    },
    0,
};

u16 gPlayerCharacterAttributes[10][7]
    __attribute__((section(".data.player_character_attributes"), aligned(4))) = {
    {0x02F0, 0x0700, 0x00F9, 0x0B90, 0x00BF, 0x0008, 0x0020},
    {0x00D0, 0x0500, 0x00CF, 0x0CA0, 0x00FF, 0x0007, 0x0016},
    {0x0100, 0x0600, 0x00DC, 0x0C50, 0x00F0, 0x0008, 0x001B},
    {0x02FD, 0x0650, 0x00DE, 0x0A50, 0x0090, 0x0008, 0x0025},
    {0x0340, 0x0800, 0x011C, 0x0420, 0x0073, 0x0009, 0x002A},
    {0x02FF, 0x0650, 0x00DE, 0x0A50, 0x0090, 0x0008, 0x0025},
    {0x0200, 0x0550, 0x00CF, 0x0B90, 0x00C4, 0x0009, 0x0020},
    {0x0320, 0x0770, 0x010D, 0x06CD, 0x0086, 0x0009, 0x0027},
    {0x0310, 0x0650, 0x00DA, 0x0B90, 0x00F0, 0x0008, 0x0020},
    {0x02F0, 0x0520, 0x00CF, 0x06A0, 0x00B8, 0x0008, 0x0020},
};

u8 D_8012089C[28] = {
    0, 3, 1, 3, 0, 0, 0, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0
};

u8 D_801208B8[28] = {
    0, 3, 1, 3, 0, 0, 0, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0
};

u8 D_801208D4[28] = {
    2, 3, 1, 3, 0, 0, 2, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0
};

u8 D_801208F0[28] = {
    3, 3, 1, 3, 0, 0, 3, 1, 3, 3, 1, 4, 1, 4, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0
};

u8 *D_8012090C[10] = {
    D_8012089C, D_8012089C, D_8012089C, D_801208D4, D_801208B8, D_801208F0, D_8012089C, D_8012089C, D_801208F0, D_8012089C
};


u8 gPlayerState30Animation20Countdowns[12]
    __attribute__((section(".data.player_state30_animation_countdowns"), aligned(4))) = {
    20, 0, 20, 0, 50, 0, 0, 0, 0, 30, 0, 0
};

u8 gPlayerState30Animation22Countdowns[12]
    __attribute__((section(".data.player_state30_animation_countdowns"), aligned(4))) = {
    30, 30, 30, 0, 0, 0, 30, 0, 0, 30, 0, 0
};

u16 gPlayerTrickRewards[16]
    __attribute__((section(".data.player_trick_rewards"), aligned(4))) = {
    0, 100, 100, 120, 120, 150, 150, 150, 150, 220, 250, 300, 350, 400, 500, 0,
};

u16 gPlayerDirectionalTrickRewards[10]
    __attribute__((section(".data.player_trick_rewards"), aligned(4))) = {
    0, 20, 20, 30, 30, 20, 20, 30, 30, 0,
};

u16 gPlayerCombinedTrickBonuses[10]
    __attribute__((section(".data.player_trick_rewards"), aligned(4))) = {
    0, 10, 20, 20, 30, 10, 20, 20, 30, 0,
};

void updateCurrentPlayerMotionEffects(void);
void initializeScaledTaskCoordinateResource(void);
void updateCurrentPlayerState24MotionAndScale(void);
void func_80024288(void);
void func_8002497C(void);
void func_800205B8(void);
void func_80028680(void);
void updateCurrentPlayerAirborneDragUntilLanding(void);
void func_8002AE6C(void);
void func_80028C80(void);
void func_80015C94(u16 playerIndex);
void func_8001596C(u16 playerIndex);
void moveCurrentPlayerToTaskTarget(void);
void advanceCurrentPlayerTaskOnAnimationFlag(void);
void updateCurrentPlayerTaskDecelerationSteps(void);
void func_80025FAC(void);
void moveCurrentPlayerForwardUntilState22(void);
void updateCurrentPlayerState30TaskUntilStopped(void);
void func_80028680(void);
void updateCurrentPlayerState12MotionAndTimer(void);
s16 func_80028234(void);
s16 func_8002D3C4(s16 value);

TaskCallback gCurrentPlayerTaskPhaseCallbacks[8]
    __attribute__((section(".data.player_task_phases"))) = {
    moveCurrentPlayerToTaskTarget, advanceCurrentPlayerTaskOnAnimationFlag, updateCurrentPlayerTaskJumpAndSpawnCourseOffset, updateCurrentPlayerTaskDecelerationSteps,
    func_80025FAC, advanceCurrentPlayerTaskPhaseMovement, updateCurrentPlayerTaskLapTransition, moveCurrentPlayerForwardUntilState22
};


TaskCallback gCurrentPlayerState30TaskPhaseCallbacks[3]
    __attribute__((section(".data.player_state30_task_phases"))) = {
    updateCurrentPlayerState30TaskUntilStopped, advanceCurrentPlayerState30TaskOnResult, updateCurrentPlayerState30AnimationAndHeight
};


void (*currentPlayerStateCallbacks[74])(void) = {
    resetCurrentPlayerForCallback205B8,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState2,
    initializeCurrentPlayerState3,
    initializeCurrentPlayerState4,
    initializeCurrentPlayerState5,
    func_8001F6D8,
    initializeCurrentPlayerState7,
    resetCurrentPlayerForCallback26DAC,
    func_8001F6F8,
    initializeCurrentPlayerState10,
    initializeCurrentPlayerState11,
    initializeCurrentPlayerState12,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState14,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState22,
    initializeCurrentPlayerState23,
    initializeCurrentPlayerState24,
    initializeCurrentPlayerState25,
    resetCurrentPlayerForCallback25508,
    resetCurrentPlayerForCallback255C4,
    NULL,
    resetCurrentPlayerForCallback240D0,
    func_80020018,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState40,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    func_800200F8,
};

void clearCurrentPlayerSequenceState(void) {
    s32 i;
    for (i = 0; i < 6; i++) {
        D_8013B0D8->unk0CA[i] = 0;
        D_8013B0D8->unk0D6[i] = 0;
    }
}



void func_800293F0(void);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_8001E63C);

u8 readSelectedByteTable(s32 index) {
    return D_8012090C[D_8013AFCE][index];
}

void updateCurrentPlayerMotionEffects(void) {
    s32 difference;
    s16 angle;
    s16 turn;
    RacePlayer *player;

    difference = (s16)(getXZHeading(0, 0,
        D_8013B0D8->x - D_8013B0D8->unk050,
        D_8013B0D8->z - D_8013B0D8->unk058) - (u16)D_8013B0D8->unk110) & 0xFFF;
    angle = difference;
    if (difference > 0x800) {
        angle = 0x1000 - difference;
    }
    if (angle > 0x400) {
        angle = 0x800 - angle;
    }
    player = D_8013B0D8;
    player->unk972 = angle * 3 / 2;
    player->unk96C = player->unk0BC / 768;
    if (player->unk96C != 0) {
        player->unk96C += angle >> 6;
        turn = player->unk126;
        if (turn < 0) {
            turn = -turn;
        }
        player->unk96C += turn >> 4;
        if (player->unk96C > 28) {
            player->unk96C = 28;
        }
    }
    switch (readSelectedByteTable(D_8013B0D8->unk136)) {
    case 0:
        D_8013B0D8->unk96E = 0;
        break;
    case 1:
        D_8013B0D8->unk96E = 0x801;
        D_8013B0D8->unk972 = 0;
        D_8013B0D8->unk96C = 32;
        break;
    case 2:
        D_8013B0D8->unk96E = 0x100;
        break;
    case 3:
        D_8013B0D8->unk96E = 0x200;
        break;
    case 4:
        D_8013B0D8->unk96E = 0x100;
        if (D_8013B0D8->unk136 == 13) {
            D_8013B0D8->unk96E = 0x300;
            if (D_8013B0D8->unk96C > 10 && (gFrameCounter & 3) == 0) {
                func_8002E5A0(D_8013AE88->data18, 15);
            }
        }
        D_8013B0D8->unk972 = 0;
        break;
    }
    if (D_8013B0D8->unk128 & 0x2000000) {
        D_8013B0D8->unk96E = 0x100;
    }
}


void updateCurrentPlayerPartTransforms(void) {
    u16 i;
    u8 parent;
    for (i = 0; i < 12; i++) {
        parent = D_8013B0D8->unk13E[i];
        RotMatrix((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i].coord);
        if (i == 0) {
            D_8013AF90->vx = 0;
            D_8013AF90->vy = 0;
            D_8013AF90->vz = 0;
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[0].coord, D_8013AF90, D_8013AF88);
            D_8013B0D8->model.modelCoordinates[0].coord.t[0] = D_8013AF88->vx + D_8013B0D8->unk62C[0].x;
            D_8013B0D8->model.modelCoordinates[0].coord.t[1] = D_8013AF88->vy + D_8013B0D8->unk62C[0].y;
            D_8013B0D8->model.modelCoordinates[0].coord.t[2] = D_8013AF88->vz + D_8013B0D8->unk62C[0].z;
        } else {
            ApplyMatrix(&D_8013B0D8->model.modelCoordinates[parent].coord, (SVECTOR *)&D_8013B0D8->unk62C[i], D_8013AF88);
            if (D_8013B0D8->unk128 & 0x200000) {
                D_8013AF88->vy = D_8013AF88->vy * D_8013B0D8->unk0F8 / 8;
            }
            D_8013B0D8->model.modelCoordinates[i].coord.t[0] = D_8013AF88->vx + D_8013B0D8->model.modelCoordinates[parent].coord.t[0];
            D_8013B0D8->model.modelCoordinates[i].coord.t[1] = D_8013AF88->vy + D_8013B0D8->model.modelCoordinates[parent].coord.t[1];
            D_8013B0D8->model.modelCoordinates[i].coord.t[2] = D_8013AF88->vz + D_8013B0D8->model.modelCoordinates[parent].coord.t[2];
        }
        func_8001E3E4((SVECTOR *)&D_8013B0D8->unk5BC[i], &D_8013B0D8->model.modelCoordinates[i]);
    }
}

void resetCurrentPlayerForCallback205B8(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk954 = 0;
    player->unk126 = 0;
    player->unk128 &= ~0x200;
    D_8013B0D8->unk096 = 0;
    if (D_8013B0D8->unk965 != 0) {
        D_8013B0D8->unk965 = 0;
        playCurrentPlayerSoundEffect(7);
        startCurrentPlayerAnimation(0x12, 2);
    }
    setCurrentTaskCallback(func_800205B8);
}

void initializeCurrentPlayerDefaultState(void) {
    if (D_8013B0D8->animationIndex != D_800EB09C[D_8013B0D8->characterId][4]) {
        startCurrentPlayerAnimation(4, 2);
    }
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk08C = 0;
    D_8013B0D8->unk128 |= 0x200;
    clearCurrentPlayerTaskState();
    setCurrentTaskCallback(func_80021B20);
}

void initializeCurrentPlayerState22(void) {
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk128 |= 0x200;
    clearCurrentPlayerTaskState();
    D_8013B0D8->unk0C8 = 0;
    if (D_8013B0D8->animationIndex == D_800EB09C[D_8013B0D8->characterId][6] ||
        D_8013B0D8->animationIndex == D_800EB09C[D_8013B0D8->characterId][5]) {
        startCurrentPlayerAnimation(1, 5);
    } else {
        D_8013B0D8->animationFlags |= 4;
    }
    setCurrentTaskCallback(func_80021B20);
}

void initializeCurrentPlayerState2(void) {
    D_8013B0D8->unk08C = 0xB84;
    D_8013B0D8->unk128 |= 0x10;
    if (D_8013AE88->data1A) {
        startCurrentPlayerAnimation(3, 6);
    } else {
        startCurrentPlayerAnimation(3, 2);
    }
    clearCurrentPlayerSequenceState();
    D_8013B0D8->unk0C8 = 0;
    D_8013B0D8->unk13C = 0;
    setCurrentTaskCallback(func_80021258);
}

void initializeCurrentPlayerState3(void) {
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x2000;
    if (((D_8013B0D8->unk102 - (s16)D_8013B0D8->unk110) & 0xFFF) > 0x800) {
        startCurrentPlayerAnimation(0x18, 2);
    } else {
        startCurrentPlayerAnimation(0x17, 2);
    }

    D_8013B0D8->unk060 = D_8013B0D8->unk104.half[0];
    D_8013B0D8->unk064 = D_8013B0D8->unk108.half[0];
    D_8013B0D8->unk050 = D_8013B0D8->x - D_8013B0D8->unk060;
    D_8013B0D8->unk058 = D_8013B0D8->z - D_8013B0D8->unk064;
    func_8002E5A0(D_8013AE88->data18, 8);
    setCurrentTaskCallback(func_80024288);
}

void initializeCurrentPlayerState4(void) {
    startCurrentPlayerAnimation(12, 2);
    D_8013B0D8->unk0B8 = 0;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x12204;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk060 = D_8013B0D8->unk104.half[0];
    D_8013B0D8->unk062 = -0x600;
    D_8013B0D8->unk064 = D_8013B0D8->unk108.half[0];
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk050 = D_8013B0D8->x - D_8013B0D8->unk060;
    D_8013B0D8->unk058 = D_8013B0D8->z - D_8013B0D8->unk064;
    func_8002E5A0(D_8013AE88->data18, 4);
    setCurrentTaskCallback(updateCurrentPlayerState4MotionAndLanding);
}

void initializeCurrentPlayerState5(void) {
    D_8013B0D8->unk08C = 30;
    if (D_8013B0D8->unk128 & 0x800000) {
        D_8013B0D8->unk08C = 90;
    }
    startCurrentPlayerAnimation(13, 2);
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk128 &= 0xFF0C1FFB;
    D_8013B0D8->unk128 &= ~0x200;
    D_8013B0D8->unk128 |= 0x12000;
    D_8013B0D8->unk08C -= D_8013B0D8->unk08C * D_8013B0D8->unk935 / 8;
    func_800183FC(D_8013AE88->data18, 1, 0xB00, 0x20);
    func_8002E5A0(D_8013AE88->data18, 9);
    setCurrentTaskCallback(updateCurrentPlayerState5MotionAndTimer);
}

void initializeCurrentPlayerTimedEffectState(s32 value) {
    RacePlayer *player = D_8013B0D8;
    player->unk08C = 60;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x42000;
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = value;
    player->unk078 = 0;
    player->unk10C = player->unk92E;
    switch (player->unk136) {
    case 6:
        createPlayerRingEffectTask(D_8013AE88->data18);
        func_800183FC(D_8013AE88->data18, 1, 0xF00, 0x30);
        break;
    case 7:
    case 12:
        startPositionEffectTask((const PositionEffectOrigin *)gRacePlayers[D_8013AE88->data18].worldCoordinate.coord.t);
        func_800183FC(D_8013AE88->data18, 1, 1, 0x30);
        break;
    case 14:
        createPlayerRingEffectTask(D_8013AE88->data18);
        break;
    }
    D_8013B0D8->unk08C -= D_8013B0D8->unk08C * D_8013B0D8->unk935 / 8;
    startCurrentPlayerAnimation(16, 2);
    func_8002E5A0(D_8013AE88->data18, 1);
    D_8013B0D8->unk064 = 0;
    D_8013B0D8->unk062 = 0;
    D_8013B0D8->unk060 = 0;
    D_8013B0D8->unk0BC = 0;
    D_8013B0D8->unk094 = 0;
    setCurrentTaskCallback(func_8002497C);
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_8001F6D8);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_8001F6F8);

void resetCurrentPlayerForCallback26DAC(void) {
    RacePlayer *player;
    if (D_8013B0D8->unk128 & 0x10000) {
        startCurrentPlayerAnimation(8, 2);
    } else {
        startCurrentPlayerAnimation(7, 2);
    }
    player = D_8013B0D8;
    player->unk078 = 0x400;
    player->unk0BC = 0;
    player->unk094 = 0;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 &= ~0x200;
    player->unk128 |= 0xA000;
    D_8013AE88->data1A = 0;
    setCurrentTaskCallback(updateCurrentPlayerState8Steering);
}

void initializeCurrentPlayerState10(void) {
    RacePlayer *player;
    D_801241E8[D_8013AE88->data18].unk16 = 4;
    player = D_8013B0D8;
    player->unk128 &= ~0x200;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x6000;
    if (player->animationIndex != D_800EB09C[player->characterId][15]) {
        startCurrentPlayerAnimation(15, 1);
    }
    D_8013B0D8->unk08C = 30;
    D_8013B0D8->unk078 = 0;
    func_8002E5A0(D_8013AE88->data18, 2);
    setCurrentTaskCallback(updateCurrentPlayerState10Movement);
}

void initializeCurrentPlayerState14(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 15;
    player->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    func_8002E5A0(D_8013AE88->data18, 5);
    setCurrentTaskCallback(updateCurrentPlayerTimedMotion);
}

void initializeCurrentPlayerState11(void) {
    RacePlayer *player;
    startCurrentPlayerAnimation(11, 2);
    player = D_8013B0D8;
    player->unk0B8 = 0;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x22201;
    player->unk062 -= 0x800;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk078 = 0;
    func_8002E5A0(D_8013AE88->data18, 9);
    setCurrentTaskCallback(updateCurrentPlayerState11Motion);
}

void initializeCurrentPlayerState12(void) {
    startCurrentPlayerAnimation(14, 2);
    D_8013B0D8->unk08C = 30;
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk090 = 0;
    D_8013B0D8->unk128 &= 0xFF0C1FFB;
    D_8013B0D8->unk128 &= ~0x200;
    D_8013B0D8->unk128 |= 0x22000;
    D_8013B0D8->unk08C -= D_8013B0D8->unk08C * D_8013B0D8->unk935 / 8;
    D_8013AE88->data1A = 0;
    func_800183FC(D_8013AE88->data18, 1, 0xB00, 0x20);
    func_8002E5A0(D_8013AE88->data18, 4);
    setCurrentTaskCallback(updateCurrentPlayerState12MotionAndTimer);
}

void initializeCurrentPlayerState40(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 30;
    D_8013B0D8->unk08C += ((u32)randomNextMain() & 0xFF) >> 4;
    D_8013B0D8->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    func_8002E5A0(D_8013AE88->data18, 5);
    setCurrentTaskCallback(updateCurrentPlayerTimedMotion);
}

void initializeCurrentPlayerState24(void) {
    RacePlayer *player;
    u16 i;
    D_8013B0D8->unk0F8 = 1;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x202000;
    startCurrentPlayerAnimation(1, 1);
    for (i = 0; i < 12; i++) {
        D_8013B0D8->unk5BC[i].z = 0;
        D_8013B0D8->unk5BC[i].x = 0;
    }
    updateCurrentPlayerPartTransforms();
    D_8013AE88->data1A = 0;
    D_8013AE88->data1C = 0;
    player = D_8013B0D8;
    player->unk08C = 90;
    player->unk078 = 0;
    player->unk064 = 0;
    player->unk062 = 0;
    player->unk060 = 0;
    player->unk0BC = 0;
    player->unk094 = 0;
    player->unk08C = 90 - (player->unk08C * player->unk935) / 8;
    func_8002E5A0(D_8013AE88->data18, 7);
    D_8013AF88->vx = D_8013B0D8->worldCoordinate.coord.t[0];
    D_8013AF88->vy = D_8013B0D8->worldCoordinate.coord.t[1] - 32;
    D_8013AF88->vz = D_8013B0D8->worldCoordinate.coord.t[2];
    createQuadAnimationTask(D_8013AE88->data18, D_8013AF88);
    setCurrentTaskCallback(updateCurrentPlayerState24MotionAndScale);
}

void initializeCurrentPlayerState25(void) {
    RacePlayer *player = D_8013B0D8;
    s32 reduction;
    player->unk08C = 240;
    reduction = player->unk935 * player->unk08C;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x400000;
    player->unk08C = 240 - reduction / 8;
    startCurrentPlayerAnimation(1, 1);
    func_80014930(&gGameTasks[4], initializeScaledTaskCoordinateResource);
    D_8013AE8C->data18 = D_8013AE88->data18;
    func_8002E5A0(D_8013AE88->data18, 6);
    setCurrentTaskCallback(updateCurrentPlayerState25MotionAndTimer);
}

void resetCurrentPlayerForCallback25508(void) {
    startCurrentPlayerAnimation(15, 1);
    D_8013B0D8->unk0B8 = 0;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x6204;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk062 = -0x2000;
    D_8013B0D8->unk064 = 0;
    D_8013B0D8->unk060 = 0;
    D_8013B0D8->unk0BC = 0;
    D_8013B0D8->unk094 = 0;
    func_8002E5A0(D_8013AE88->data18, 3);
    func_80014930(&gGameTasks[4], initializePlayerParticleHistoryTask);
    D_8013AE8C->data18 = D_8013AE88->data18;
    setCurrentTaskCallback(updateCurrentPlayerVerticalMotionUntilLanding);
}

void resetCurrentPlayerForCallback255C4(void) {
    RacePlayer *player;
    startCurrentPlayerAnimation(0xB, 2);
    player = D_8013B0D8;
    player->unk0B8 = 0;
    player->unk078 = 0;
    player->unk062 = -0x1800;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x822204;
    D_8013AE88->data1C = 0;
    func_8002E5A0(D_8013AE88->data18, 6);
    setCurrentTaskCallback(updateCurrentPlayerAirborneDragUntilLanding);
}

void initializeCurrentPlayerState23(void) {
    s32 reduction;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x102000;
    startCurrentPlayerAnimation(12, 2);
    D_8013B0D8->unk08C = 90;
    reduction = D_8013B0D8->unk935 * D_8013B0D8->unk08C;
    D_8013B0D8->unk08C = 90 - reduction / 8;
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk064 = 0;
    D_8013B0D8->unk062 = 0;
    D_8013B0D8->unk060 = 0;
    D_8013B0D8->unk0BC = 0;
    D_8013B0D8->unk094 = 0;
    func_80014930(&gGameTasks[4], initializePlayerState23CoordinateEffectTask);
    D_8013AE8C->data18 = D_8013AE88->data18;
    func_8002E5A0(D_8013AE88->data18, 6);
    func_800183FC(D_8013AE88->data18, 1, 0xD00, 0x20);
    setCurrentTaskCallback(updateCurrentPlayerState23MotionAndTimer);
}

void initializeCurrentPlayerState7(void) {
    RacePlayer *player = D_8013B0D8;
    D_8013AE88->data1A = 0;
    player->unk08C = 20;
    player->unk090 = 10;
    player->unk126 = D_80123D80[D_8013AFCE].unk16;
    if (player->unk128 & 0x400) {
        player->unk126 += 0x800;
    }
    D_8013B0D8->unk134 = 0;
    D_8013AFE4 |= 4;
    startCurrentPlayerAnimation(1, 1);
    func_800183FC(D_8013AE88->data18, 1, 0x401, 0x20);
    setCurrentTaskCallback(updateCurrentPlayerFromTaskPhase);
}

void resetCurrentPlayerForCallback240D0(void) {
    RacePlayer *player = D_8013B0D8;
    GameTask *task = D_8013AE88;

    player->unk08C = 0;
    player->unk090 = 0;
    player->unk128 &= ~0x201;
    task->data1A = 0;
    task->data1C = 0;
    player->unk954 = 0;
    player->unk126 = 0;
    D_8013B0D8->unk0BE = D_8013B0D8->unk138;
    startCurrentPlayerAnimation(0x22, 2);
    setCurrentTaskCallback(updateCurrentPlayerState29MotionAndAnimation);
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80020018);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_800200F8);

s32 callCurrentPlayerStateCallbackIfDifferent(u16 state) {
    s32 result;
    if (D_8013B0D8->unk12C == state) {
        result = 0;
    } else {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        result = 1;
    }
    return result;
}

void initializeCurrentPlayerModelAndHeight(void) {
    RacePlayer *player;
    if (D_8013AFE2 == 0) {
        func_80014930(&gGameTasks[4], initializeTaskCoordinateResource);
        D_8013AE8C->data18 = D_8013AE88->data18;
    }
    player = &gRacePlayers[D_8013AE88->data18];
    player->unk0B6 = -0x400;
    player->unk0B4 = 0;
    player->unk0B8 = 0;
    player->unk962 = -1;
    D_8013B0D8 = player;
    initializeCurrentPlayerModelObjects((DAT_PLAYER_MODEL_BUFFERS)[D_8013AE88->data18]);
    if (!D_8013AFD8 && !D_8013AFD9) {
        D_8013B0D8->unk92E = 0;
    } else {
        D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    }
    D_8013B0D8->y = D_8013B0D8->unk054 = func_8004A764(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    func_80028680();
    startCurrentPlayerAnimation(1, 1);
    setCurrentTaskCallback(updateCurrentPlayerAndResetState);
}

void updateCurrentPlayerAndResetState(void) {
    func_8002AE6C();
    updateCurrentPlayerAnimation();
    if (D_8013AFE4 == 0) {
        if (D_8013B0D8->unk008 == 0) {
            if (D_8013AFD7 || D_8013AFD9) {
                func_80015C94(D_8013AE88->data18);
            } else if (D_8013AFE2 == 2 && D_8013AFD6 == 0) {
                func_8001596C(0);
                resetSelectedRecordStates();
            }
        }
        D_8013B0D8->unk12C = 0;
        resetCurrentPlayerForCallback205B8();
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_800205B8);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80021258);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80021B20);

void updateCurrentPlayerState29MotionAndAnimation(void) {
    RacePlayer *player;
    func_8002AE6C();
    player = D_8013B0D8;
    player->unk09E = player->unk09A;
    player->unk110 = player->unk138;
    player->unk112 = player->unk13A;
    if (player->unk128 & 0x400) {
        player->unk110 += 0x800;
        player->unk112 = -(player->unk112 & 0xFFF);
    }
    D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    D_8013B0D8->unk0B8 = D_8013B0D8->unk112;
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_800EB22C.vx = D_8013B0D8->unk060;
    D_800EB22C.vz = D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    D_8013B0D8->unk060 = D_800EB22C.vx;
    D_8013B0D8->unk064 = D_800EB22C.vz;
    func_8002AE38(0, (s16)D_8013B0D8->unk0A6, (s16)D_8013B0D8->unk0A8, (s16)D_8013B0D8->unk0AA);
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    func_80028680();
    updateCurrentPlayerAnimation();
    if (D_8013B0D8->animationFlags & 0x10) {
        startCurrentPlayerAnimation(0x23, 1);
    }
    D_8013B0D8->unk128 &= ~0x2000000;
    if ((u16)callCurrentPlayerStateCallbackIfDifferent(0x1D)) {
        if (D_8013B0D8->unk12C == 0x16) {
            if (D_8013B0D8->unk062 < 0) {
                D_8013B0D8->unk062 = -D_8013B0D8->unk062;
            }
            D_8013B0D8->unk92C = 0;
            startCurrentPlayerAnimation(1, 1);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80024288);

void updateCurrentPlayerState4MotionAndLanding(void) {
    RacePlayer *player;
    func_8002AE6C();
    player = D_8013B0D8;
    player->unk130 = 0x1800;
    if (player->unk128 & 1) {
        D_800EB22C.vx = player->unk060;
        D_800EB22C.vz = player->unk064;
        clampVectorToCurrentPlayerLimit(&D_800EB22C);
        D_8013B0D8->unk060 = D_800EB22C.vx;
        D_8013B0D8->unk064 = D_800EB22C.vz;
        D_8013B0D8->unk062 += 0x280;
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 15;
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 15;
    } else {
        player->unk09E = player->unk09A;
        func_8002AE38(0, 0xC0, 0xC0, 0xC0);
    }
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    if (!(D_8013B0D8->unk128 & 1)) {
        func_80028680();
    } else if (func_80028234()) {
        func_8002E5A0(D_8013AE88->data18, 9);
        func_800183FC(D_8013AE88->data18, 1, 0xB01, 0x20);
        createPlayerTripleQuadEffectTask(D_8013AE88->data18);
    }
    updateCurrentPlayerAnimation();
    if (D_8013B0D8->animationFlags & 0x10) {
        if (!(D_8013B0D8->unk128 & 1)) {
            if (!func_8002D3C4(0)) {
                D_8013B0D8->unk12C = 5;
            }
            currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        }
    }
}

void updateCurrentPlayerState5MotionAndTimer(void) {
    RacePlayer *player;
    RacePlayer *motion;
    RacePlayer *grounded;
    s16 input;
    func_8002AE6C();
    smoothAndScaleCurrentPlayerInput(0, 0);
    if (!(D_8013B0D8->unk128 & 1)) {
        func_8002AE38(0, 0x100, 0x100, 0x100);
    }
    motion = D_8013B0D8;
    motion->unk09E = 0x280;
    if (motion->unk062 > 0x7800) {
        motion->unk062 = 0x7800;
    }
    D_8013B0D8->y += D_8013B0D8->unk062;
    func_80028680();
    updateCurrentPlayerAnimation();
    player = D_8013B0D8;
    if (!(player->unk128 & 1)) {
        input = countCurrentPlayerInputGroups();
        D_8013B0D8->unk08C--;
        D_8013B0D8->unk08C -= input;
        if (D_8013B0D8->unk08C < 0) {
            D_8013B0D8->unk08C = 0;
        }
        grounded = D_8013B0D8;
        if (grounded->animationFlags & 0x10) {
            grounded->unk128 &= ~0x200;
            if (grounded->unk08C == 0) {
                grounded->unk12C = 8;
                currentPlayerStateCallbacks[8]();
                return;
            }
        }
    } else {
        player->unk128 |= 0x200;
        startCurrentPlayerAnimation(13, 2);
        if (D_8013B0D8->unk08C < 30) {
            D_8013B0D8->unk08C = 30;
        }
    }
    callCurrentPlayerStateCallbackIfDifferent(5);
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_8002497C);

void updateCurrentPlayerState23MotionAndTimer(void) {
    RacePlayer *player;
    func_8002AE6C();
    player = D_8013B0D8;
    if (!(player->unk128 & 1)) {
        player->unk128 &= ~0x200;
        func_80028680();
    } else {
        player->unk128 |= 0x200;
        player->unk062 += 0x240;
        if (player->unk062 > 0x7800) {
            player->unk062 = 0x7800;
        }
        D_8013B0D8->y += D_8013B0D8->unk062;
        func_80028234();
    }
    D_8013B0D8->unk08C -= 1 + countCurrentPlayerInputGroups();
    if (D_8013B0D8->unk08C < 0) {
        D_8013B0D8->unk08C = 0;
    }
    if (D_8013B0D8->unk08C == 0 && D_8013B0D8->unk12C != 24) {
        D_8013B0D8->unk12C = 4;
        D_8013B0D8->unk108.word = 0;
        D_8013B0D8->unk104.word = 0;
    }
    callCurrentPlayerStateCallbackIfDifferent(23);
}

void updateCurrentPlayerState24MotionAndScale(void) {
    RacePlayer *player;
    u16 timerStep;
    s16 landed;

    func_8002AE6C();
    smoothAndScaleCurrentPlayerInput(0, 0);
    if (D_8013B0D8->unk128 & 1) {
        D_8013B0D8->unk062 += 0x280;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->unk128 |= 0x200;
    } else {
        D_8013B0D8->unk128 &= ~0x200;
    }
    if (!(D_8013B0D8->unk128 & 1)) {
        func_80028680();
    } else if ((landed = func_80028234()) != 0) {
        func_8002E5A0(D_8013AE88->data18, 9);
        func_800183FC(D_8013AE88->data18, 1, 0xB01, 0x20);
        createPlayerTripleQuadEffectTask(D_8013AE88->data18);
    }

    switch (D_8013AE88->data1A) {
    case 0:
        if (D_8013B0D8->unk0F8 != 1) {
            D_8013B0D8->unk0F8--;
        }
        timerStep = 1 + countCurrentPlayerInputGroups();
        if (timerStep != 1) {
            D_8013B0D8->unk0F8 = 3;
        }
        D_8013B0D8->unk08C -= timerStep;
        if (D_8013B0D8->unk08C < 0) {
            D_8013B0D8->unk08C = 0;
        }
        if (D_8013B0D8->unk08C == 0) {
            D_8013AE88->data1A++;
            D_8013B0D8->unk0F8 = 1;
        }
        break;
    case 1:
        if (++D_8013B0D8->unk0F8 == 7) {
            D_8013B0D8->unk0F8 = 2;
            D_8013AE88->data1A++;
        }
        break;
    case 2:
        if (++D_8013B0D8->unk0F8 == 8) {
            player = D_8013B0D8;
            player->unk12C = 0;
            player->unk128 &= 0xFF0C1FFB;
            currentPlayerStateCallbacks[0]();
            D_8013B0D8->unk0F6 = 60;
            return;
        }
        break;
    }
    updateCurrentPlayerPartTransforms();
}

void updateCurrentPlayerState25MotionAndTimer(void) {
    func_8002AE6C();
    if (!(D_8013B0D8->unk128 & 1)) {
        D_8013B0D8->unk128 &= ~0x200;
        smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk094, 0);
        func_8002A8E4(0, 0, (s16)D_8013B0D8->unk0A6, (s16)D_8013B0D8->unk0A8, (s16)D_8013B0D8->unk0AA);
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        func_80028680();
    } else {
        D_8013B0D8->unk128 |= 0x200;
        D_800EB22C.vx = D_8013B0D8->unk060;
        D_800EB22C.vz = D_8013B0D8->unk064;
        clampVectorToCurrentPlayerLimit(&D_800EB22C);
        D_8013B0D8->unk060 = D_800EB22C.vx;
        D_8013B0D8->unk064 = D_800EB22C.vz;
        D_8013B0D8->unk062 += D_8013B0D8->unk09C;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        if (func_80028234()) {
            func_8002E5A0(D_8013AE88->data18, 10);
            func_800183FC(D_8013AE88->data18, 1, 0xB01, 0x18);
            createPlayerTripleQuadEffectTask(D_8013AE88->data18);
        }
    }
    D_8013B0D8->unk08C -= 1 + countCurrentPlayerInputGroups();
    if (D_8013B0D8->unk08C < 0) {
        D_8013B0D8->unk08C = 0;
    }
    if (D_8013B0D8->unk08C == 0) {
        D_8013B0D8->unk128 &= 0xFF0C1FFB;
        if (D_8013B0D8->unk12C != 24) {
            if (!(D_8013B0D8->unk128 & 0x200)) {
                D_8013B0D8->unk12C = 0;
            } else {
                D_8013B0D8->unk12C = 1;
            }
        }
    }
    callCurrentPlayerStateCallbackIfDifferent(25);
}

void updateCurrentPlayerVerticalMotionUntilLanding(void) {
    RacePlayer *player;
    func_8002AE6C();
    player = D_8013B0D8;
    player->unk062 += 0x1C0;
    player->y += player->unk062;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
    if (player->unk062 > 0) {
        if (func_80028234() != 0) {
            D_8013B0D8->unk12C = 12;
            currentPlayerStateCallbacks[12]();
            return;
        }
    } else {
        player->unk946 = 0;
    }
    updateCurrentPlayerAnimation();
}

void updateCurrentPlayerAirborneDragUntilLanding(void) {
    func_8002AE6C();
    smoothAndScaleCurrentPlayerInput(0, 0);
    if (D_8013B0D8->unk062 < 0) {
        D_8013B0D8->unk062 += 0x150;
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 64;
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 64;
    } else {
        if (D_8013AE88->data1C == 0) {
            D_8013AE88->data1C++;
            func_80014930(&gGameTasks[4], initializePlayerCoordinateEffectTask);
            D_8013AE8C->data18 = D_8013AE88->data18;
        }
        D_8013B0D8->unk0B6 += 0x40;
        D_8013B0D8->unk062 += 0x18;
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 48;
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 48;
    }
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    D_8013B0D8->worldCoordinate.coord.t[0] = D_8013B0D8->x >> 8;
    D_8013B0D8->worldCoordinate.coord.t[1] = D_8013B0D8->y >> 8;
    D_8013B0D8->worldCoordinate.coord.t[2] = D_8013B0D8->z >> 8;
    if (func_80028234() != 0) {
        D_8013B0D8->unk12C = 8;
        currentPlayerStateCallbacks[8]();
    } else {
        updateCurrentPlayerAnimation();
    }
}

void updateCurrentPlayerFromTaskPhase(void) {
    RacePlayer *player;
    func_8002AE6C();
    updateCurrentPlayerAnimation();
    gCurrentPlayerTaskPhaseCallbacks[D_8013AE88->data1A]();
    D_8013B0D8->unk946 = 0;
    player = D_8013B0D8;
    player->unk110 = player->unk0B6 + 0x400;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
}

void moveCurrentPlayerToTaskTarget(void) {
    s16 angle;
    s32 difference;
    RacePlayer *player;
    difference = (D_8013B0D8->unk126 - (u16)D_8013B0D8->unk0B6) & 0xFFF;
    angle = difference;
    if (difference > 0x800) {
        angle = difference | 0xF000;
    }
    smoothAndScaleCurrentPlayerInput(0x100, 0);
    player = D_8013B0D8;
    if (player->unk090 != 0) {
        player->unk0B6 += angle / player->unk090;
        player->x += ((s32)D_80123D80[D_8013AFCE].unk18[0] - player->x) / player->unk090;
        player->z += ((s32)D_80123D80[D_8013AFCE].unk18[2] - player->z) / player->unk090;
    }
    player = D_8013B0D8;
    if (--player->unk08C == 0) {
        player->x = D_80123D80[D_8013AFCE].unk18[0];
        player->z = D_80123D80[D_8013AFCE].unk18[2];
        D_8013AE88->data1A++;
        player->unk08C = 0;
        startCurrentPlayerAnimation(3, 2);
        D_801241E8[D_8013AE88->data18].unk16 = 3;
    }
}

void advanceCurrentPlayerTaskOnAnimationFlag(void) {
    RacePlayer *player;
    if (D_8013B0D8->animationFlags & 0x10) {
        D_8013AE88->data1A++;
        func_800183FC(D_8013AE88->data18, 1, 0x601, 0x20);
        player = D_8013B0D8;
        player->unk0B6 = player->unk126;
        player->x = D_80123D80[D_8013AFCE].unk18[0];
        player->z = D_80123D80[D_8013AFCE].unk18[2];
        player->unk062 = -0xC00;
        player->unk08C = 30;
        startCurrentPlayerAnimation(4, 2);
        if (D_8013B0D8->unk128 & 0x400) {
            D_8013B0D8->unk090 = -0x800;
        } else {
            D_8013B0D8->unk090 = 0x800;
        }
    }
}

void updateCurrentPlayerTaskJumpAndSpawnCourseOffset(void) {
    RacePlayer *player;
    RacePlayer *landingPlayer;
    CourseOffsetResource *resource;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    player = D_8013B0D8;
    player->x += D_8013AF88->vx;
    player->z += D_8013AF88->vz;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = player->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    landingPlayer = D_8013B0D8;
    landingPlayer->x += D_8013AF88->vx;
    landingPlayer->z += D_8013AF88->vz;
    landingPlayer->unk062 += 0x140;
    landingPlayer->y += landingPlayer->unk062;
    if (landingPlayer->unk062 > 0 && landingPlayer->y > (s32)D_80123D80[D_8013AFCE].unk18[1]) {
        landingPlayer->y = D_80123D80[D_8013AFCE].unk18[1];
        if (landingPlayer->animationIndex != D_800EB09C[landingPlayer->characterId][1]) {
            startCurrentPlayerAnimation(1, 1);
            func_800183FC(D_8013AE88->data18, 1, 0x701, 0x20);
            D_8013AFE4 &= ~4;
        }
        updateCurrentPlayerMotionEffects();
        D_8013B0D8->unk128 &= ~0x200;
    } else {
        D_8013B0D8->unk128 |= 0x200;
    }
    player = D_8013B0D8;
    if (--player->unk08C == 0) {
        D_8013AE88->data1A++;
        player->unk08C = 0x20;
        func_80014930(&gGameTasks[4], initializeSelectedCourseOffsetTask);
        D_8013AE8C->data18 = D_8013AE88->data18;
        D_8013AE8C->resource14 = allocateTaskResource();
        resource = (CourseOffsetResource *)&D_800D9504[D_8013AE8C->resource14];
        resource->linkedTask = D_8013AE88;
        D_8013B0D8->unk128 &= ~0x20;
    }
}

void updateCurrentPlayerTaskDecelerationSteps(void) {
    RacePlayer *player = D_8013B0D8;
    RacePlayer *firstStep;
    RacePlayer *second;
    RacePlayer *last;
    player->unk0B6 -= 0x10;
    if (player->unk090 > 0) {
        player->unk090 -= 0x20;
    } else {
        player->unk090 += 0x20;
    }
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    firstStep = D_8013B0D8;
    firstStep->x += D_8013AF88->vx;
    firstStep->z += D_8013AF88->vz;
    firstStep->unk0B6 -= 0x10;
    if (firstStep->unk090 > 0) {
        firstStep->unk090 -= 0x20;
    } else {
        firstStep->unk090 += 0x20;
    }
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    second = D_8013B0D8;
    second->x += D_8013AF88->vx;
    second->z += D_8013AF88->vz;
    updateCurrentPlayerMotionEffects();
    if (--D_8013B0D8->unk08C == 0) {
        D_8013AE88->data1A++;
        startCurrentPlayerAnimation(25, 2);
        last = D_8013B0D8;
        last->unk08C = 19;
        last->unk128 |= 0x200;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80025FAC);

void advanceCurrentPlayerTaskPhaseMovement(void) {
    RacePlayer *player;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    player = D_8013B0D8;
    player->x += D_8013AF88->vx;
    player->z += D_8013AF88->vz;
    if (--player->unk08C == 0) {
        D_8013AE88->data1A++;
        player->unk08C = 100;
    }
}

void updateCurrentPlayerTaskLapTransition(void) {
    RacePlayer *player;
    RacePlayer *movingPlayer;
    RacePlayer *placedPlayer;
    RacePlayer *phasePlayer;
    const SelectedCourseRecord *course;
    GameTask *task;
    s16 segment;

    if (D_8013B0D8->unk090 > 0) {
        if (D_8013B0D8->unk090 < 0x2800) {
            D_8013B0D8->unk090 += 0x80;
        }
        D_8013AF90->vx = 0x100;
        D_8013AF90->vy = D_8013B0D8->unk110;
        D_8013AF90->vz = 0;
    } else {
        if (D_8013B0D8->unk090 >= -0x27FF) {
            D_8013B0D8->unk090 -= 0x80;
        }
        D_8013AF90->vx = -0x100;
        D_8013AF90->vy = D_8013B0D8->unk110;
        D_8013AF90->vz = 0;
    }
    RotMatrixYXZ(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = D_8013B0D8->unk090;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    movingPlayer = D_8013B0D8;
    movingPlayer->x += D_8013AF88->vx;
    movingPlayer->y += D_8013AF88->vy;
    movingPlayer->z += D_8013AF88->vz;
    if (--movingPlayer->unk08C == 0) {
        D_8013AE88->data1A++;
        movingPlayer->unk0B6 = D_80123D80[D_8013AFCE].unk14;
        movingPlayer->unk110 = movingPlayer->unk0BE = movingPlayer->unk0B6 + 0x400;
        D_8013AF90->vx = 0;
        D_8013AF90->vy = movingPlayer->unk110;
        D_8013AF90->vz = 0;
        RotMatrix(D_8013AF90, D_8013AF98);
        D_8013AF88->vx = 0;
        D_8013AF88->vy = 0;
        D_8013AF88->vz = -0x28000;
        ApplyMatrixLV(D_8013AF98, D_8013AF88, (VECTOR *)&D_8013B0D8->x);
        course = &D_80123D80[D_8013AFCE];
        placedPlayer = D_8013B0D8;
        placedPlayer->x += course->unk08;
        placedPlayer->y += course->unk0C - 0x2000;
        placedPlayer->z += course->unk10;
        segment = course->unk04;
        placedPlayer->unk934++;
        placedPlayer->unk92E = segment;
        /* Preserve the complete four-word position vector. */
        *(VECTOR *)&D_8013B0D8->unk050 = *(VECTOR *)&D_8013B0D8->x;
        phasePlayer = D_8013B0D8;
        phasePlayer->unk08C = 40;
        phasePlayer->animationFlags &= ~0x10;
        phasePlayer->unk128 &= ~0x04000400;
        if (D_8013B0D8->unk008 == 0) {
            PlayerCamera *cameras = D_801241E8;
            cameras[D_8013AE88->data18].unk16 = 7;
            task = D_8013AE88;
            cameras[task->data18].yaw = D_8013B0D8->unk110;
            cameras[task->data18].pitch = 0;
            cameras[task->data18].unk18 = D_8013B0D8->unk92E;
            updatePlayerCameraPosition(task->data18);
            clampPlayerCameraHeight(D_8013AE88->data18);
            if (D_8013AE88->data18 == 0) {
                configureTaskStateTransition(1, 0x1000, 60);
            } else {
                configureTaskStateTransition(1, 0x2000, 60);
            }
            if (D_8013B0D8->unk934 >= selectedFinishLapCountAbsolute - 1) {
                func_80014874(initializeTaskMotionResource);
                D_8013AE8C->data18 = D_8013AE88->data18;
            }
        }
    }
    player = D_8013B0D8;
    if (player->unk08C == 60 && player->unk008 == 0) {
        if (D_8013AE88->data18 == 0) {
            configureTaskStateTransition(2, 0x1000, player->unk08C);
        } else {
            configureTaskStateTransition(2, 0x2000, player->unk08C);
        }
    }
}

void moveCurrentPlayerForwardUntilState22(void) {
    RacePlayer *player;
    D_8013AF90->vx = 0;
    D_8013AF90->vy = D_8013B0D8->unk110;
    D_8013AF90->vz = 0;
    RotMatrix(D_8013AF90, D_8013AF98);
    D_8013AF90->vx = 0;
    D_8013AF90->vy = 0;
    D_8013AF90->vz = 0x1000;
    ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);
    player = D_8013B0D8;
    player->x += D_8013AF88->vx;
    player->z += D_8013AF88->vz;
    if (--player->unk08C == 0) {
        player->unk12C = 22;
        player->unk128 = 0;
        player->unk0E8 = 0;
        switch (player->unk935) {
        case 1:
            player->x += player->unk060;
            player->z += player->unk064;
            break;
        case 2:
            player->x += player->unk060 * 2;
            player->z += player->unk064 * 2;
            break;
        case 3:
            player->x += player->unk060 * 4;
            player->z += player->unk064 * 4;
            break;
        }
        D_8013B0D8->unk062 = 0;
        D_8013B0D8->unk094 = 0;
        D_8013B0D8->unk943 = 0;
        startCurrentPlayerAnimation(4, 2);
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    }
}

void updateCurrentPlayerAndRequestState30(void) {
    RacePlayer *player;
    func_8002AE6C();
    gCurrentPlayerState30TaskPhaseCallbacks[D_8013AE88->data1A]();
    if (D_8013B0D8->unk0AC < 0x3C00) {
        D_8013B0D8->unk0AC += 0x200;
    }
    player = D_8013B0D8;
    player->unk110 = player->unk0B6 + 0x400;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
    callCurrentPlayerStateCallbackIfDifferent(30);
}

void updateCurrentPlayerState30TaskUntilStopped(void) {
    RacePlayer *player;
    RacePlayer *steeringPlayer;
    GameTask *task = D_8013AE88;
    RacePlayer *animationPlayer;
    RacePlayer *movingPlayer;
    RacePlayer *stoppedPlayer;
    RacePlayer *finishedPlayer;
    s16 turn;
    s32 difference;

    if (task->data1E != 0) {
        player = D_8013B0D8;
        difference = (player->unk0BE - player->unk110) & 0xFFF;
        turn = difference;
        if (difference > 0x800) {
            turn = difference | 0xF000;
        }
        if (turn > 0x40) {
            turn = 0x40;
        }
        if (turn < -0x40) {
            turn = -0x40;
        }
        player->unk0B6 -= turn;
        if (task->data1C) {
            player->unk08C = 31;
        } else {
            player->unk08C = -31;
        }
        D_8013B0D8->unk124 = 63;
        turn = smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk0BC, (s16)D_8013B0D8->unk08C);
        steeringPlayer = D_8013B0D8;
        turn -= steeringPlayer->unk126;
        if (turn > 0x30) {
            turn = 0x30;
        }
        if (turn < -0x30) {
            turn = -0x30;
        }
        steeringPlayer->unk126 += turn;
        if ((s16)steeringPlayer->unk126 == 0) {
            if (steeringPlayer->animationIndex != D_800EB09C[steeringPlayer->characterId][1]) {
                startCurrentPlayerAnimation(1, 1);
            }
        } else {
            if ((s16)steeringPlayer->unk126 >= 0) {
                func_8001758C(6, (s16)steeringPlayer->unk126, 0x118);
                if ((s16)D_8013B0D8->unk126 >= 0) {
                    goto updateAnimation;
                }
            }
            func_8001758C(5, -(s16)D_8013B0D8->unk126, 0x118);
        }
    } else {
        smoothAndScaleCurrentPlayerInput(0, 0);
    }

updateAnimation:
    animationPlayer = D_8013B0D8;
    if (animationPlayer->animationIndex != D_800EB09C[animationPlayer->characterId][6] &&
        animationPlayer->animationIndex != D_800EB09C[animationPlayer->characterId][5]) {
        updateCurrentPlayerAnimation();
    }
    func_8002AE38(0, 0x200, 0x180, 0x180);
    movingPlayer = D_8013B0D8;
    movingPlayer->x += movingPlayer->unk060;
    movingPlayer->y += movingPlayer->unk062;
    movingPlayer->z += movingPlayer->unk064;
    func_80028680();
    updateCurrentPlayerMotionEffects();

    stoppedPlayer = D_8013B0D8;
    if ((stoppedPlayer->unk060 | stoppedPlayer->unk064) == 0 && !(stoppedPlayer->unk128 & 1)) {
        if (stoppedPlayer->unk008 == 0) {
            PlayerCamera *cameras = D_801241E8;
            PlayerCamera *camera = &cameras[D_8013AE88->data18];
            if (camera->unk16 != 6 && D_8013AFD6 == 0) {
                camera->unk16 = 5;
            }
        }
        finishedPlayer = D_8013B0D8;
        finishedPlayer->unk128 |= 0x100;
        D_8013AE88->data1A++;
        if (finishedPlayer->animationIndex != D_800EB09C[finishedPlayer->characterId][1]) {
            startCurrentPlayerAnimation(1, 1);
        }
    }
}

void advanceCurrentPlayerState30TaskOnResult(void) {
    if (D_8013AFE1 != 0) {
        if (D_8013AFE2 == 0) {
            if (D_8013B0D8->unk935 == 0) {
                D_8013AE88->data1C = 0;
                startCurrentPlayerAnimation(0x13, 2);
            } else {
                D_8013AE88->data1C = 2;
                startCurrentPlayerAnimation(0x15, 2);
            }
        } else {
            if (D_8013AE88->data18 + 1 == D_8013AFE1) {
                D_8013AE88->data1C = 0;
                startCurrentPlayerAnimation(0x13, 2);
            } else {
                D_8013AE88->data1C = 2;
                startCurrentPlayerAnimation(0x15, 2);
            }
        }
        D_8013AE88->data1A++;
        D_8013B0D8->unk08C = 0;
    } else {
        smoothAndScaleCurrentPlayerInput(0, 0);
        func_80028680();
        updateCurrentPlayerAnimation();
    }
}

void updateCurrentPlayerState30AnimationAndHeight(void) {
    RacePlayer *player = D_8013B0D8;
    if (player->unk08C == 0) {
        if (player->animationFlags & 0x10) {
            if (!(D_8013AE88->data1C & 1)) {
                D_8013AE88->data1C++;
                if (!(D_8013AE88->data1C & 2)) {
                    if (!D_8013AFD6) playCurrentPlayerSoundEffect(6);
                    startCurrentPlayerAnimation(0x14, 2);
                } else {
                    startCurrentPlayerAnimation(0x16, 2);
                }
            } else {
                if (!(D_8013AE88->data1C & 2)) {
                    player->unk08C = gPlayerState30Animation20Countdowns[player->characterId];
                    startCurrentPlayerAnimation(0x14, 2);
                } else {
                    player->unk08C = gPlayerState30Animation22Countdowns[player->characterId];
                    startCurrentPlayerAnimation(0x16, 2);
                }
            }
        }
        updateCurrentPlayerAnimation();
    } else {
        player->unk08C--;
    }
    D_8013B0D8->unk946 = 0;
    if (func_8002D3C4(0)) {
        D_8013B0D8->unk92E = getAdjacentGeometryRegionForPoint(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
        D_8013B0D8->y = func_8004A764(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8) << 8;
    }
}

void updateCurrentPlayerTaskCountdown(void) {
    RacePlayer *player;
    func_8002AE6C();
    D_8013B0D8->unk946 = 0;
    player = D_8013B0D8;
    player->worldCoordinate.coord.t[0] = player->x >> 8;
    player->worldCoordinate.coord.t[1] = player->y >> 8;
    player->worldCoordinate.coord.t[2] = player->z >> 8;
    updateCurrentPlayerAnimation();
    if (D_8013AE88->data18 == 0) {
        if (--D_8013AE88->data1A == 0) {
            playCurrentPlayerSoundEffect(6);
        }
    }
}

void updateCurrentPlayerState8Steering(void) {
    s16 angle;
    RacePlayer *player;
    s32 difference;
    s16 delta;
    func_8002AE6C();
    if (D_8013B0D8->unk128 & 1) {
        D_8013B0D8->unk128 |= 0x200;
    } else {
        D_8013B0D8->unk128 &= ~0x200;
    }
    player = D_8013B0D8;
    if (player->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 && !(player->unk128 & 0x40)) {
        angle = getXZHeading(player->x, player->z, D_80123D80[D_8013AFCE].unk40[0], D_80123D80[D_8013AFCE].unk40[1]);
        player = D_8013B0D8;
        if (player->unk128 & 0x400) {
            angle += 0x800;
        }
        delta = angle - player->unk110;
        difference = delta & 0xFFF;
        angle = difference;
        if (difference > 0x800) {
            angle = difference | 0xF000;
        }
        if ((s16)angle > 0x66) {
            angle = 0x66;
        }
        if ((s16)angle < -0x66) {
            angle = -0x66;
        }
        player->unk0B6 += angle;
    } else {
        if (player->unk008 == 0 || player->unk92E != player->unk962) {
            goto movement;
        }
        angle = ((GeometryPlacement *)geometryPlacementDataAbsolute)[player->unk92E].angle;
        if (player->unk128 & 0x400) {
            angle += 0x800;
        }
        delta = angle - player->unk110;
        difference = delta & 0xFFF;
        angle = difference;
        if (difference > 0x800) {
            angle = difference | 0xF000;
        }
        if ((s16)angle > 0x66) {
            angle = 0x66;
        }
        if ((s16)angle < -0x66) {
            angle = -0x66;
        }
        player->unk0B6 += angle;
    }
movement:
    smoothAndScaleCurrentPlayerInput(0, 0);
    D_8013B0D8->unk09E = 0x280;
    func_80028680();
    updateCurrentPlayerAnimation();
    if (++D_8013AE88->data1A < 15) {
        D_8013B0D8->unk128 |= 8;
    } else {
        D_8013B0D8->unk128 &= ~8;
    }
    if (D_8013B0D8->animationFlags & 0x10) {
        D_8013B0D8->unk962 = D_8013B0D8->unk92E;
        D_8013B0D8->unk128 &= 0xFF0C1FFB;
        if (D_8013B0D8->unk12C != 24) {
            D_8013B0D8->unk12C = 0;
        }
    }
    if ((u16)callCurrentPlayerStateCallbackIfDifferent(8)) {
        D_8013B0D8->unk128 &= ~8;
    }
}

void updateCurrentPlayerState11Motion(void) {
    func_8002AE6C();
    D_8013B0D8->unk130 = 0x1800;
    D_800EB22C.vx = D_8013B0D8->unk060;
    D_800EB22C.vz = D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    D_8013B0D8->unk060 = D_800EB22C.vx;
    D_8013B0D8->unk064 = D_800EB22C.vz;
    if (!(D_8013B0D8->unk128 & 1)) {
        D_8013B0D8->unk09E = D_8013B0D8->unk09C;
        func_8002AE38(0, 0x100, 0x100, 0x100);
    } else {
        D_8013B0D8->unk060 -= D_8013B0D8->unk060 / 16;
        D_8013B0D8->unk062 += D_8013B0D8->unk09C;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->unk064 -= D_8013B0D8->unk064 / 16;
    }
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    if (!(D_8013B0D8->unk128 & 1)) {
        func_80028680();
    } else {
        func_80028234();
    }
    updateCurrentPlayerAnimation();
    if (!(D_8013B0D8->unk128 & 1) && (D_8013B0D8->animationFlags & 0x10)) {
        D_8013B0D8->unk12C = 12;
        currentPlayerStateCallbacks[12]();
    }
}

void updateCurrentPlayerState10Movement(void) {
    func_8002AE6C();
    smoothAndScaleCurrentPlayerInput(D_8013B0D8->unk094, 0);
    if (D_8013B0D8->unk062 < 0) {
        D_8013B0D8->unk060 = 0;
        D_8013B0D8->unk062 = 0;
        D_8013B0D8->unk064 = 0;
    }
    D_8013B0D8->unk130 = 0x2000;
    D_8013B0D8->unk09E = D_8013B0D8->unk09A;
    func_8002AE38(0, 0, 0, 0);
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    D_8013B0D8->unk110 = getXZHeading(0, 0, D_8013B0D8->unk060, D_8013B0D8->unk064);
    if (D_8013B0D8->unk128 & 0x400) {
        D_8013B0D8->unk110 += 0x800;
    }
    D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    func_80028C80();
    updateCurrentPlayerAnimation();
    if (D_8013B0E4 != 0) {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    } else if (!(D_8013B0D8->unk128 & 0x201)) {
        if (D_8013B0D8->unk136 != 3 && D_8013B0D8->unk136 != 6 &&
            D_8013B0D8->unk136 != 12 && D_8013B0D8->unk136 != 16 &&
            D_8013B0D8->unk136 != 8 && D_8013B0D8->unk136 != 7 &&
            D_8013B0D8->unk136 != 14 && D_8013B0D8->unk136 != 15) {
            D_801241E8[D_8013AE88->data18].unk16 = 1;
            initializeCurrentPlayerState14();
        } else {
            D_8013B0D8->unk96E = 0x500;
            D_8013B0D8->unk972 = 0;
            D_8013B0D8->unk96C = 0x20;
        }
    }
}

void updateCurrentPlayerTimedMotion(void) {
    func_8002AE6C();
    if (!(D_8013B0D8->unk128 & 1)) {
        smoothAndScaleCurrentPlayerInput(0x400, 0);
        D_8013B0D8->unk09E = 0x280;
        D_8013B0D8->unk128 &= ~0x200;
        func_8002AE38(0, 0, 0, 0);
    } else {
        D_8013B0D8->unk062 += D_8013B0D8->unk09C;
        if (D_8013B0D8->unk062 > 0x7800) {
            D_8013B0D8->unk062 = 0x7800;
        }
        D_8013B0D8->unk128 |= 0x200;
    }
    D_800EB22C.vx = D_8013B0D8->unk060;
    D_800EB22C.vz = D_8013B0D8->unk064;
    clampVectorToCurrentPlayerLimit(&D_800EB22C);
    D_8013B0D8->unk060 = D_800EB22C.vx;
    D_8013B0D8->unk064 = D_800EB22C.vz;
    D_8013B0D8->x += D_8013B0D8->unk060;
    D_8013B0D8->y += D_8013B0D8->unk062;
    D_8013B0D8->z += D_8013B0D8->unk064;
    if (!(D_8013B0D8->unk128 & 1)) {
        func_80028680();
        D_8013B0D8->unk96E = 0x500;
        D_8013B0D8->unk972 = 0;
        D_8013B0D8->unk96C = 0x20;
    } else {
        func_80028234();
    }
    D_8013B0D8->unk110 = getXZHeading(0, 0, D_8013B0D8->unk060, D_8013B0D8->unk064);
    if (D_8013B0D8->unk128 & 0x400) {
        D_8013B0D8->unk110 += 0x800;
    }
    D_8013B0D8->unk0B6 = D_8013B0D8->unk110 - 0x400;
    updateCurrentPlayerAnimation();
    if (D_8013B0E4) {
        currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
    } else if (D_8013B0D8->unk08C == 0) {
        D_8013B0D8->unk12C = 12;
        currentPlayerStateCallbacks[12]();
    } else {
        D_8013B0D8->unk08C--;
    }
}

void updateCurrentPlayerState12MotionAndTimer(void) {
    RacePlayer *player;
    RacePlayer *grounded;
    s16 input;

    func_8002AE6C();
    smoothAndScaleCurrentPlayerInput(0, 0);
    player = D_8013B0D8;
    if (!(player->unk128 & 1)) {
        player->unk09E = 0x280;
        func_8002AE38(0, 0x180, 0x180, 0x180);
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        func_80028680();
        updateCurrentPlayerAnimation();
        if (++D_8013AE88->data1A >= 8) {
            D_8013B0D8->unk128 |= 8;
        } else {
            D_8013B0D8->unk128 &= ~8;
        }
        updateCurrentPlayerMotionEffects();
        if (D_8013B0D8->unk96E != 0x300 && D_8013B0D8->unk96E != 0x801) {
            D_8013B0D8->unk972 = 0x400;
        }
        if (D_8013B0E4) {
            D_8013B0D8->unk128 &= ~8;
            currentPlayerStateCallbacks[D_8013B0D8->unk12C]();
        } else {
            input = countCurrentPlayerInputGroups();
            D_8013B0D8->unk08C--;
            D_8013B0D8->unk08C -= input;
            if (D_8013B0D8->unk08C < 0) {
                D_8013B0D8->unk08C = 0;
            }
            grounded = D_8013B0D8;
            if (grounded->animationFlags & 0x10) {
                grounded->unk128 &= ~0x200;
                if (grounded->unk08C == 0) {
                    grounded->unk12C = 8;
                    grounded->unk128 &= ~8;
                    currentPlayerStateCallbacks[grounded->unk12C]();
                }
            }
        }
    } else {
        player->unk062 += player->unk09C;
        if (player->unk062 > 0x7800) {
            player->unk062 = 0x7800;
        }
        D_8013B0D8->x += D_8013B0D8->unk060;
        D_8013B0D8->y += D_8013B0D8->unk062;
        D_8013B0D8->z += D_8013B0D8->unk064;
        if (func_80028234()) {
            func_8002E5A0(D_8013AE88->data18, 9);
            func_800183FC(D_8013AE88->data18, 1, 0xB01, 0x20);
            createPlayerTripleQuadEffectTask(D_8013AE88->data18);
        }
        D_8013B0D8->unk08C = 30;
        D_8013B0D8->unk128 |= 0x200;
        D_8013B0D8->unk090 = 7;
        D_8013B0D8->unk128 &= ~8;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80027918);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80028234);

/* Average angles on the 4096-unit circle, taking the shorter arc. */
s32 averageWrappedAngle(s16 first, u16 second) {
    s16 difference = first - second;
    s16 midpoint;
    if (difference > -0x800 && difference < 0x800) {
        return first - difference / 2;
    }
    midpoint = (s16)(first + second) / 2 - 0x800;
    if (midpoint & 0x800) {
        return midpoint | 0xF000;
    }
    return midpoint & 0x7FF;
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80028680);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80028C80);

void initializeActiveRacePlayers(void) {
    u16 i;
    for (i = 0; i < D_8013AFD2; i++) {
        RacePlayer *player = &gRacePlayers[i];
        player->playerIndex = i;
        if (player->isActive) {
            currentPlayerAbsolute = player;
            func_800293F0();
        }
    }
}

void initializeCurrentPlayerAttributes(void) {
    u16 (*variant)[9] = gPlayerVariantAttributes.values;
    u16 (*character)[7] = gPlayerCharacterAttributes;
    RacePlayer *player = currentPlayerAbsolute;
    player->unk130 = player->unk098 = variant[player->unk015[1]][0] + character[player->characterId][0];
    player->unk09A = variant[player->unk015[1]][1] + character[player->characterId][1];
    player->unk09C = variant[player->unk015[1]][2] + character[player->characterId][2];
    player->unk0A0 = variant[player->unk015[1]][3] + character[player->characterId][3];
    player->unk0A6 = variant[player->unk015[1]][4] + character[player->characterId][4];
    player->unk0A2 = variant[player->unk015[1]][5] + character[player->characterId][5];
    player->unk0A4 = variant[player->unk015[1]][6] + character[player->characterId][6];
    player->unk0A8 = variant[player->unk015[1]][7];
    player->unk0AA = variant[player->unk015[1]][8];
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_800293F0);

s16 isPlayerNearSelectedRecord(s16 index) {
    s16 i;
    s32 distance;
    if (gRacePlayers[index].unk92E != (s16)D_80123D80[D_8013AFCE].unk00) return 0;
    if (gRacePlayers[index].unk128 & 0x40) return 0;
    for (i = 0; i < 4; i++) {
        if (gRacePlayers[i].isActive && gRacePlayers[i].playerIndex != index &&
            (gRacePlayers[i].unk128 & 0x20)) return 0;
    }
    distance = gRacePlayers[index].x - D_80123D80[D_8013AFCE].unk18[0];
    if (distance < 0) distance = -distance;
    if (distance > 0x6000) return 0;
    distance = gRacePlayers[index].z - D_80123D80[D_8013AFCE].unk18[2];
    if (distance < 0) distance = -distance;
    return distance <= 0x6000;
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80029DD0);

INCLUDE_ASM("asm/nonmatchings/race/player/player_update", func_80029E6C);

void assignCurrentPlayerFinishSlot(s32 value) {
    RacePlayer *player = currentPlayerAbsolute;
    u32 flags = player->unk128;
    if (!(flags & 0x40) && player->unk934 >= selectedFinishLapCountAbsolute - 1 &&
        player->unk92E == (s16)D_80123D80[D_8013AFCE].unk00 && !(flags & 0x1000)) {
        player->unk128 = flags | 0x40;
        if (!D_8013AFD6) {
            if (!player->unk008) {
                func_80014874(beginSound601TaskSequence);
                D_8013AE8C->data18 = value;
            }
            currentPlayerAbsolute->unk100[0] = D_8013AFDB;
            D_8013AFDB++;
        }
    }
}

void awardCurrentPlayerTrickRewards(void) {
    s32 reward = 0;
    s32 points = 0;
    s32 sound;
    s16 trick;
    RacePlayer *player;

    if (currentPlayerAbsolute->unk0C8 >= 9 ||
        (currentPlayerAbsolute->unk0C8 != 0 && currentPlayerAbsolute->unk0C4 != 0)) {
        currentPlayerAbsolute->unk965 = 1;
    }

    if (currentPlayerAbsolute->unk0C8 != 0) {
        reward += gPlayerTrickRewards[currentPlayerAbsolute->unk0C8];
        if (currentPlayerAbsolute->unk0C8 < 9) {
            points += 10;
        } else {
            points += 40;
        }
        trick = currentPlayerAbsolute->unk0C8 < 10 ? currentPlayerAbsolute->unk0C8 : 9;
        currentPlayerAbsolute->unk0E6 |= 1 << trick;
    }

    if (currentPlayerAbsolute->unk0C4 != 0) {
        if (currentPlayerAbsolute->unk0C8 != 0) {
            reward += gPlayerCombinedTrickBonuses[currentPlayerAbsolute->unk0C4];
            if (currentPlayerAbsolute->unk0C4 & 1) {
                points += 5;
            } else {
                points += 10;
            }
        } else {
            reward += gPlayerDirectionalTrickRewards[currentPlayerAbsolute->unk0C4];
            points++;
        }
        reward += currentPlayerAbsolute->unk0C6;
    }

    if (reward != 0 && D_8013AFE2 == 0) {
        addPlayerCappedCounters(D_8013AE88->data18, reward);
        if (currentPlayerAbsolute->unk008 == 0 && (D_8013AFD9 == 0 || D_8013AE88->data18 == 0)) {
            createActivePlayerSlidingNumberTask(reward, currentPlayerAbsolute->playerIndex);
            sound = 0x201;
            if (reward >= 100) {
                sound = 0x301;
            }
            if (reward >= 120) {
                sound = 0x401;
            }
            if (reward >= 200) {
                sound = 0x501;
            }
            playSoundEffect(0, sound, 0x40);
        }
    }

    if (D_8013AFE2 == 1 && D_8013AFE0 == 2 && points != 0) {
        if (points > 9999) {
            points = 9999;
        }
        player = currentPlayerAbsolute;
        player->trickAttackMakeCount++;
        player->trickAttackPointTotal += points;
        if (currentPlayerAbsolute->trickAttackMakeCount > 99) {
            currentPlayerAbsolute->trickAttackMakeCount = 99;
        }
        if (currentPlayerAbsolute->unk0E4 < currentPlayerAbsolute->trickAttackMakeCount) {
            currentPlayerAbsolute->unk0E4 = currentPlayerAbsolute->trickAttackMakeCount;
        }
        sound = 0x201;
        if (points >= 10) {
            sound = 0x301;
        }
        if (points >= 15) {
            sound = 0x401;
        }
        if (points >= 20) {
            sound = 0x501;
        }

        if (currentPlayerAbsolute->unk0E6 == 0x3FE) {
            currentPlayerAbsolute->unk0E6 = 0x3FF;
            currentPlayerAbsolute->trickAttackPointTotal += 300;
            createPlayerNumberDisplayTask(points, 1, (s16)D_8013AE88->data18);
            sound = 0x501;
        } else {
            createPlayerNumberDisplayTask(points, 0, (s16)D_8013AE88->data18);
        }
        playSoundEffect(0, sound, 0x40);
        if (currentPlayerAbsolute->trickAttackPointTotal > 9999) {
            currentPlayerAbsolute->trickAttackPointTotal = 9999;
        }
    }
}

void computeSelectedTriangleNormal(VECTOR *normal) {
    VECTOR first, second;

    first.vx = D_8013B204[0] - D_8013B200[0];
    first.vy = D_8013B204[1] - D_8013B200[1];
    first.vz = D_8013B204[2] - D_8013B200[2];
    second.vx = D_8013B208[0] - D_8013B204[0];
    second.vy = D_8013B208[1] - D_8013B204[1];
    second.vz = D_8013B208[2] - D_8013B204[2];
    OuterProduct0(&first, &second, normal);
}

void computeVectorYawAndPitch(VECTOR *vector, s16 *yaw, s16 *pitch) {
    s32 x, y, z;
    s16 angle;
    x = vector->vx;
    y = vector->vy;
    z = vector->vz;
    angle = getXZHeading(0, 0, x, z);
    if (angle >= 0x800) angle -= 0x1000;
    *yaw = angle;
    x /= 16;
    y /= 16;
    z /= 16;
    angle = getXZHeading(0, 0, SquareRoot0(x*x + z*z), y);
    if (angle >= 0x800) angle -= 0x1000;
    *pitch = angle;
}
