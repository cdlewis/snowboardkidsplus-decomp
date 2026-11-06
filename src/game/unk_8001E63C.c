#include "common.h"
#define TASK_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/selected_record.h"
#include "game/graphics.h"
#include "game/linear_random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/selected_records.h"
#include "psx/libsnd.h"
#include "unk_8001E63C.h"


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


void func_800240D0(void);
void func_80026DAC(void);
void func_800205B8(void);
void func_800255C4(void);
void func_8002AE6C(void);
s32 func_80017268(void);
void func_80025840(void);
void func_80025A38(void);
void func_80025B08(void);
void func_80025DD8(void);
void func_80025FAC(void);
void func_80026010(void);
void func_800260E4(void);
void func_80026500(void);

TaskCallback gCurrentPlayerTaskPhaseCallbacks[8]
    __attribute__((section(".data.player_task_phases"))) = {
    func_80025840, func_80025A38, func_80025B08, func_80025DD8,
    func_80025FAC, func_80026010, func_800260E4, func_80026500
};


void (*currentPlayerStateCallbacks[74])(void) = {
    func_8001F0C4,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState2,
    func_8001F2D4,
    func_8001F388,
    func_8001F444,
    func_8001F6D8,
    func_8001FEE0,
    func_8001F718,
    func_8001F6F8,
    func_8001F7A0,
    func_8001F8E4,
    func_8001F970,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerState14,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    initializeCurrentPlayerDefaultState,
    func_8001F1B4,
    func_8001FDEC,
    func_8001FAC4,
    func_8001FC00,
    func_8001FCB4,
    func_8001FD70,
    NULL,
    func_8001FFA8,
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
    func_8001FA38,
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001E63C);

u8 readSelectedByteTable(s32 index) {
    return D_8012090C[D_8013AFCE][index];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001EC88);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001EEBC);

void resetCurrentPlayerForCallback205B8(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk954 = 0;
    player->unk126 = 0;
    player->unk128 &= ~0x200;
    D_8013B0D8->unk096 = 0;
    if (D_8013B0D8->unk965 != 0) {
        D_8013B0D8->unk965 = 0;
        func_8002E44C(7);
        startCurrentPlayerAnimation(0x12, 2);
    }
    setCurrentTaskCallback(func_800205B8);
}

void initializeCurrentPlayerDefaultState(void) {
    if (D_8013B0D8->animationIndex != D_800EB09C[D_8013B0D8->unk014][4]) {
        startCurrentPlayerAnimation(4, 2);
    }
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk08C = 0;
    D_8013B0D8->unk128 |= 0x200;
    clearCurrentPlayerTaskState();
    setCurrentTaskCallback(func_80021B20);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F1B4);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F2D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F388);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F444);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F528);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F6D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F6F8);

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
    setCurrentTaskCallback(func_80026DAC);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F7A0);

void initializeCurrentPlayerState14(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 15;
    player->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    func_8002E5A0(D_8013AE88->data18, 5);
    setCurrentTaskCallback(func_800273D4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F8E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001F970);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FA38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FAC4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FC00);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FCB4);

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
    setCurrentTaskCallback(func_800255C4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FDEC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FEE0);

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
    setCurrentTaskCallback(func_800240D0);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80020018);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800200F8);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80020380);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800204D8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800205B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80021258);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80021B20);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800240D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024288);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800245C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024804);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002497C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024F10);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025014);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800252CC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025508);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800255C4);

void updateCurrentPlayerFromTaskPhase(void) {
    RacePlayer *player;
    func_8002AE6C();
    func_80017268();
    gCurrentPlayerTaskPhaseCallbacks[D_8013AE88->data1A]();
    D_8013B0D8->unk946 = 0;
    player = D_8013B0D8;
    player->unk110 = player->unk0B6 + 0x400;
    player->unk8E4 = player->x >> 8;
    player->unk8E8 = player->y >> 8;
    player->unk8EC = player->z >> 8;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025840);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025A38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025B08);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025DD8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025FAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026010);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800260E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026500);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800266A8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026754);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026AA8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026BAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026D10);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026DAC);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80027014);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800271D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800273D4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80027624);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80027918);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80028234);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80028680);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80028C80);

void initializeActiveRacePlayers(void) {
    u16 i;
    for (i = 0; i < D_8013AFD2; i++) {
        RacePlayer *player = &D_80120E90[i];
        player->playerIndex = i;
        if (player->unk024) {
            currentPlayerAbsolute = player;
            func_800293F0();
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029208);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800293F0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029C30);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029DD0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029E6C);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029EE4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029FE4);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002A454);
