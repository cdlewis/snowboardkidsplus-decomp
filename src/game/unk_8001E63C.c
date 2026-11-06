#include "common.h"
#define TASK_TRANSITION_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_FLAGS_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#define RACE_FINISH_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/race_camera.h"
#include "game/selected_record.h"
#include "game/graphics.h"
#include "game/coordinate.h"
#include "game/quad_animation.h"
#include "game/angles.h"
#include "game/linear_random.h"
#include "game/random.h"
#include "game/offset_archive.h"
#include "game/music.h"
#include "game/global_flags.h"
#include "game/game_options.h"
#include "game/selected_records.h"
#include "psx/libsnd.h"
#include "unk_8001E63C.h"


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


void initializeScaledTaskCoordinateResource(void);
void func_8001EEBC(void);
void func_80025014(void);
void func_800252CC(void);
void func_800240D0(void);
void func_80024288(void);
void func_800245C4(void);
void func_80024804(void);
void func_80026DAC(void);
void func_800205B8(void);
void func_80028680(void);
void func_800255C4(void);
void func_8002AE6C(void);
s32 func_80017268(void);
void func_80015C94(u16 playerIndex);
void func_8001596C(u16 playerIndex);
void func_80025840(void);
void advanceCurrentPlayerTaskOnAnimationFlag(void);
void func_80025B08(void);
void func_80025DD8(void);
void func_80025FAC(void);
void func_800260E4(void);
void func_80026500(void);
void func_80026754(void);
void func_80028680(void);
void func_80026BAC(void);
void func_80027624(void);
s16 func_80028234(void);

TaskCallback gCurrentPlayerTaskPhaseCallbacks[8]
    __attribute__((section(".data.player_task_phases"))) = {
    func_80025840, advanceCurrentPlayerTaskOnAnimationFlag, func_80025B08, func_80025DD8,
    func_80025FAC, advanceCurrentPlayerTaskPhaseMovement, func_800260E4, func_80026500
};


TaskCallback gCurrentPlayerState30TaskPhaseCallbacks[3]
    __attribute__((section(".data.player_state30_task_phases"))) = {
    func_80026754, advanceCurrentPlayerState30TaskOnResult, func_80026BAC
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
        playCurrentPlayerSoundEffect(7);
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
    D_801241E8[D_8013AE88->data18].unk16 = 4;
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
    smoothAndScaleCurrentPlayerInput(0, 0);
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
    func_8001EEBC();
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
    D_8013AF88->vx = D_8013B0D8->position8E4[0];
    D_8013AF88->vy = D_8013B0D8->position8E4[1] - 32;
    D_8013AF88->vz = D_8013B0D8->position8E4[2];
    createQuadAnimationTask(D_8013AE88->data18, D_8013AF88);
    setCurrentTaskCallback(func_80025014);
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
    setCurrentTaskCallback(func_800255C4);
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
    func_80014930(&gGameTasks[4], func_80044EC0);
    D_8013AE8C->data18 = D_8013AE88->data18;
    func_8002E5A0(D_8013AE88->data18, 6);
    func_800183FC(D_8013AE88->data18, 1, 0xD00, 0x20);
    setCurrentTaskCallback(func_80024F10);
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

void initializeCurrentPlayerModelAndHeight(void) {
    RacePlayer *player;
    if (D_8013AFE2 == 0) {
        func_80014930(&gGameTasks[4], initializeTaskCoordinateResource);
        D_8013AE8C->data18 = D_8013AE88->data18;
    }
    player = &D_80120E90[D_8013AE88->data18];
    player->unk0B6 = -0x400;
    player->unk0B4 = 0;
    player->unk0B8 = 0;
    player->unk962 = -1;
    D_8013B0D8 = player;
    func_8001E244(((u32 (*)[4096])0x8016B000)[D_8013AE88->data18]);
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
    func_80017268();
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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800205B8);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80021258);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80021B20);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800240D0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024288);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800245C4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80024804);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_8002497C);

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80025014);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800252CC);

void updateCurrentPlayerVerticalMotionUntilLanding(void) {
    RacePlayer *player;
    func_8002AE6C();
    player = D_8013B0D8;
    player->unk062 += 0x1C0;
    player->y += player->unk062;
    player->position8E4[0] = player->x >> 8;
    player->position8E4[1] = player->y >> 8;
    player->position8E4[2] = player->z >> 8;
    if (player->unk062 > 0) {
        if (func_80028234() != 0) {
            D_8013B0D8->unk12C = 12;
            currentPlayerStateCallbacks[12]();
            return;
        }
    } else {
        player->unk946 = 0;
    }
    func_80017268();
}

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
    player->position8E4[0] = player->x >> 8;
    player->position8E4[1] = player->y >> 8;
    player->position8E4[2] = player->z >> 8;
    callCurrentPlayerStateCallbackIfDifferent(30);
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80026754);

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
        func_80017268();
    }
}

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
            playCurrentPlayerSoundEffect(6);
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

void initializeCurrentPlayerAttributes(void) {
    u16 (*variant)[9] = gPlayerVariantAttributes.values;
    u16 (*character)[7] = gPlayerCharacterAttributes;
    RacePlayer *player = currentPlayerAbsolute;
    player->unk130 = player->unk098 = variant[player->unk015[1]][0] + character[player->unk014][0];
    player->unk09A = variant[player->unk015[1]][1] + character[player->unk014][1];
    player->unk09C = variant[player->unk015[1]][2] + character[player->unk014][2];
    player->unk0A0 = variant[player->unk015[1]][3] + character[player->unk014][3];
    player->unk0A6 = variant[player->unk015[1]][4] + character[player->unk014][4];
    player->unk0A2 = variant[player->unk015[1]][5] + character[player->unk014][5];
    player->unk0A4 = variant[player->unk015[1]][6] + character[player->unk014][6];
    player->unk0A8 = variant[player->unk015[1]][7];
    player->unk0AA = variant[player->unk015[1]][8];
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_800293F0);

s16 isPlayerNearSelectedRecord(s16 index) {
    s16 i;
    s32 distance;
    if (D_80120E90[index].unk92E != (s16)D_80123D80[D_8013AFCE].unk00) return 0;
    if (D_80120E90[index].unk128 & 0x40) return 0;
    for (i = 0; i < 4; i++) {
        if (D_80120E90[i].unk024 && D_80120E90[i].playerIndex != index &&
            (D_80120E90[i].unk128 & 0x20)) return 0;
    }
    distance = D_80120E90[index].x - D_80123D80[D_8013AFCE].unk18[0];
    if (distance < 0) distance = -distance;
    if (distance > 0x6000) return 0;
    distance = D_80120E90[index].z - D_80123D80[D_8013AFCE].unk18[2];
    if (distance < 0) distance = -distance;
    return distance <= 0x6000;
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029DD0);

INCLUDE_ASM("asm/nonmatchings/game/unk_8001E63C", func_80029E6C);

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

void computeVectorYawAndPitch(VECTOR *vector, s16 *yaw, s16 *pitch) {
    s32 x, y, z;
    s16 angle;
    x = vector->vx;
    y = vector->vy;
    z = vector->vz;
    angle = func_8004A124(0, 0, x, z);
    if (angle >= 0x800) angle -= 0x1000;
    *yaw = angle;
    x /= 16;
    y /= 16;
    z /= 16;
    angle = func_8004A124(0, 0, SquareRoot0(x*x + z*z), y);
    if (angle >= 0x800) angle -= 0x1000;
    *pitch = angle;
}
