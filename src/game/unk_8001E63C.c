#include "common.h"
#define TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/race_camera.h"
#include "game/selected_record.h"
#include "game/graphics.h"
#include "game/coordinate.h"
#include "game/linear_random.h"
#include "game/random.h"
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


void func_80045868(void);
void func_800252CC(void);
void func_800240D0(void);
void func_80024288(void);
void func_800245C4(void);
void func_80024804(void);
void func_80026DAC(void);
void func_800205B8(void);
void func_800255C4(void);
void func_8002AE6C(void);
s32 func_80017268(void);
void func_80025840(void);
void advanceCurrentPlayerTaskOnAnimationFlag(void);
void func_80025B08(void);
void func_80025DD8(void);
void func_80025FAC(void);
void func_800260E4(void);
void func_80026500(void);
void func_80026754(void);
void func_80026AA8(void);
void func_80026BAC(void);
void func_80027624(void);

TaskCallback gCurrentPlayerTaskPhaseCallbacks[8]
    __attribute__((section(".data.player_task_phases"))) = {
    func_80025840, advanceCurrentPlayerTaskOnAnimationFlag, func_80025B08, func_80025DD8,
    func_80025FAC, advanceCurrentPlayerTaskPhaseMovement, func_800260E4, func_80026500
};


TaskCallback gCurrentPlayerState30TaskPhaseCallbacks[3]
    __attribute__((section(".data.player_state30_task_phases"))) = {
    func_80026754, func_80026AA8, func_80026BAC
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
    func_8001FDEC,
    func_8001FAC4,
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

void initializeCurrentPlayerState22(void) {
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk128 |= 0x200;
    clearCurrentPlayerTaskState();
    D_8013B0D8->unk0C8 = 0;
    if (D_8013B0D8->animationIndex == D_800EB09C[D_8013B0D8->unk014][6] ||
        D_8013B0D8->animationIndex == D_800EB09C[D_8013B0D8->unk014][5]) {
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

    D_8013B0D8->unk060 = D_8013B0D8->unk104;
    D_8013B0D8->unk064 = D_8013B0D8->unk108;
    D_8013B0D8->unk050 = D_8013B0D8->x - D_8013B0D8->unk060;
    D_8013B0D8->unk058 = D_8013B0D8->z - D_8013B0D8->unk064;
    func_8002E5A0(D_8013AE88->data18, 8);
    setCurrentTaskCallback(func_80024288);
}

void initializeCurrentPlayerState4(void) {
    startCurrentPlayerAnimation(12, 2);
    D_8013B0D8->unk0B8 = 0;
    D_8013B0D8->unk128 = (D_8013B0D8->unk128 & 0xFF0C1FFB) | 0x12204;
    func_8002A5A0(0, 0);
    D_8013B0D8->unk060 = D_8013B0D8->unk104;
    D_8013B0D8->unk062 = -0x600;
    D_8013B0D8->unk064 = D_8013B0D8->unk108;
    D_8013B0D8->unk078 = 0;
    D_8013B0D8->unk050 = D_8013B0D8->x - D_8013B0D8->unk060;
    D_8013B0D8->unk058 = D_8013B0D8->z - D_8013B0D8->unk064;
    func_8002E5A0(D_8013AE88->data18, 4);
    setCurrentTaskCallback(func_800245C4);
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
    setCurrentTaskCallback(func_80024804);
}

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

void initializeCurrentPlayerState10(void) {
    RacePlayer *player;
    D_801241E8[D_8013AE88->data18].unk16[0] = 4;
    player = D_8013B0D8;
    player->unk128 &= ~0x200;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x6000;
    if (player->animationIndex != D_800EB09C[player->unk014][15]) {
        startCurrentPlayerAnimation(15, 1);
    }
    D_8013B0D8->unk08C = 30;
    D_8013B0D8->unk078 = 0;
    func_8002E5A0(D_8013AE88->data18, 2);
    setCurrentTaskCallback(func_800271D0);
}

void initializeCurrentPlayerState14(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 15;
    player->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    func_8002E5A0(D_8013AE88->data18, 5);
    setCurrentTaskCallback(func_800273D4);
}

void initializeCurrentPlayerState11(void) {
    RacePlayer *player;
    startCurrentPlayerAnimation(11, 2);
    player = D_8013B0D8;
    player->unk0B8 = 0;
    player->unk128 &= 0xFF0C1FFB;
    player->unk128 |= 0x22201;
    player->unk062 -= 0x800;
    func_8002A5A0(0, 0);
    D_8013B0D8->unk078 = 0;
    func_8002E5A0(D_8013AE88->data18, 9);
    setCurrentTaskCallback(func_80027014);
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
    setCurrentTaskCallback(func_80027624);
}

void initializeCurrentPlayerState40(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x6000;
    player->unk08C = 30;
    D_8013B0D8->unk08C += ((u32)randomNextMain() & 0xFF) >> 4;
    D_8013B0D8->unk078 = 0;
    startCurrentPlayerAnimation(15, 1);
    func_8002E5A0(D_8013AE88->data18, 5);
    setCurrentTaskCallback(func_800273D4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FAC4);

void initializeCurrentPlayerState25(void) {
    RacePlayer *player = D_8013B0D8;
    s32 reduction;
    player->unk08C = 240;
    reduction = player->unk935 * player->unk08C;
    player->unk128 = (player->unk128 & 0xFF0C1FFB) | 0x400000;
    player->unk08C = 240 - reduction / 8;
    startCurrentPlayerAnimation(1, 1);
    func_80014930(&gGameTasks[4], func_80045868);
    D_8013AE8C->data18 = D_8013AE88->data18;
    func_8002E5A0(D_8013AE88->data18, 6);
    setCurrentTaskCallback(func_800252CC);
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
    func_80014930(&D_800D7C84, func_80045B4C);
    D_8013AE8C->data18 = D_8013AE88->data18;
    setCurrentTaskCallback(func_80025508);
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
    setCurrentTaskCallback(func_800255C4);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8001FDEC);

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
    player->position8E4[0] = player->x >> 8;
    player->position8E4[1] = player->y >> 8;
    player->position8E4[2] = player->z >> 8;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025840);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025B08);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025DD8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025FAC);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800260E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026500);

void updateCurrentPlayerAndRequestState30(void) {
    RacePlayer *player;
    func_8002AE6C();
    gCurrentPlayerState30TaskPhaseCallbacks[D_8013AE88->data1A]();
    if (D_8013B0D8->unk0AC < 0x3C00) {
        D_8013B0D8->unk0AC += 0x200;
    }
    player = D_8013B0D8;
    player->unk110 = player->unk0B6 + 0x400;
    player->unk8E4 = player->x >> 8;
    player->unk8E8 = player->y >> 8;
    player->unk8EC = player->z >> 8;
    callCurrentPlayerStateCallbackIfDifferent(30);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026754);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026AA8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026BAC);

void updateCurrentPlayerTaskCountdown(void) {
    RacePlayer *player;
    func_8002AE6C();
    D_8013B0D8->unk946 = 0;
    player = D_8013B0D8;
    player->position8E4[0] = player->x >> 8;
    player->position8E4[1] = player->y >> 8;
    player->position8E4[2] = player->z >> 8;
    func_80017268();
    if (D_8013AE88->data18 == 0) {
        if (--D_8013AE88->data1A == 0) {
            func_8002E44C(6);
        }
    }
}

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
