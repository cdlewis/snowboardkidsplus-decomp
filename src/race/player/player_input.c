#include "common.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#define TASK_ABSOLUTE_ADDRESSING
#include "game/race/player/race_player.h"
#include "game/engine/game_options.h"
#include "game/graphics/coordinate.h"
#include "game/menu/selected_record.h"
#include "game/menu/selected_records.h"

typedef struct PlayerRankSpeedAdjustment {
    s16 adjustment;
    s16 unk02;
} PlayerRankSpeedAdjustment;

const PlayerRankSpeedAdjustment gPlayerRankSpeedAdjustments[4]
    __attribute__((section(".data.player_rank_speed_adjustments"))) = {
    {0, 0},
    {0x200, 0},
    {0x400, 0},
    {0x800, 0},
};

void copySelectedRecordToCurrentPlayer(void) {
    RacePlayer *player = D_8013B0D8;
    player->unk116 = D_8013B1F0[D_8013B1FE].unk8;
    player->unk11A = D_8013B1F0[D_8013B1FE].unkA;
}

s16 smoothAndScaleCurrentPlayerInput(s32 magnitude, s16 input) {
    s16 current;
    s16 factor;
    if (magnitude < 0) {
        magnitude = -magnitude;
    }
    current = D_8013B0D8->unk122;
    input = (input << 4) - current;
    if (input > 128) {
        input = 128;
    }
    if (input < -128) {
        input = -128;
    }
    current += input;
    input = current * D_8013B0D8->unk124 / 63;
    D_8013B0D8->unk122 = current;
    if (magnitude > 4096) {
        factor = 279;
    } else {
        factor = magnitude * 279 / 4096;
    }
    return input * factor / 496;
}

void clampVectorToCurrentPlayerLimit(VECTOR *vector) {
    s32 length = SquareRoot0(vector->vx * vector->vx + vector->vz * vector->vz);
    if (length > D_8013B0D8->unk130) {
        vector->vx = vector->vx * D_8013B0D8->unk130 / length;
        vector->vz = vector->vz * D_8013B0D8->unk130 / length;
    }
}

void clampCurrentPlayerHorizontalVector(VECTOR *vector) {
    s32 magnitude;

    magnitude = SquareRoot0(vector->vx * vector->vx + vector->vz * vector->vz);
    if (magnitude > D_8013B0D8->unk130 / 2) {
        vector->vx = vector->vx * (D_8013B0D8->unk130 / 2) / magnitude;
        vector->vz = vector->vz * (D_8013B0D8->unk130 / 2) / magnitude;
    }
}

INCLUDE_ASM("asm/nonmatchings/race/player/player_input", func_8002A8E4);

INCLUDE_ASM("asm/nonmatchings/race/player/player_input", func_8002AE38);

void prepareCurrentPlayerFrame(void) {
    s32 x, y, z;
    u16 angle;
    RacePlayer *player;
    RacePlayer *initialPlayer;
    s32 surface;
    u16 i;
    s16 delta;
    s16 finishSegment;

    initialPlayer = &gRacePlayers[D_8013AE88->data18];
    D_8013B0D8 = initialPlayer;
    initialPlayer->unk137 = initialPlayer->unk136;
    initialPlayer->unk938 = 0;
    if (D_8013B0D8->unk128 & 0x201) {
        D_8013B0D8->unk136 = 0;
    } else {
        D_8013B0D8->unk136 = func_8004AAA0(D_8013B0D8->unk92E, D_8013B0D8->x >> 8, D_8013B0D8->z >> 8);
    }
    if (D_8013B0D8->unk128 & 0x2000000) {
        D_8013B0D8->unk136 = 11;
    }
    if ((u32)((u8)D_8013B0D8->unk136 - 17) < 8 && !(D_8013B0D8->unk128 & 0x200)) {
        getGeometryPlacementTransform(D_8013B0D8->unk92E, &x, &y, &z, &angle);
        surface = D_8013B0D8->unk136;
        D_8013AF90->vx = 0;
        D_8013AF90->vz = 0;
        angle += (surface - 17) * 512;
        D_8013AF90->vy = angle;
        RotMatrix(D_8013AF90, D_8013AF98);

        D_8013AF90->vx = 0;
        D_8013AF90->vy = 0;
        D_8013AF90->vz = 384;
        ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);

        D_8013B0D8->x += D_8013AF88->vx;
        D_8013B0D8->y += D_8013AF88->vy;
        D_8013B0D8->z += D_8013AF88->vz;
    }

    D_8013B0D8->unk060 = D_8013B0D8->x - D_8013B0D8->unk050;
    D_8013B0D8->unk064 = D_8013B0D8->z - D_8013B0D8->unk058;
    /* Copy all four words of the position snapshot, including its fourth word. */
    *(VECTOR *)&D_8013B0D8->unk050 = *(VECTOR *)&D_8013B0D8->x;

    D_8013B0D8->unk96C = 0;
    D_8013B0D8->unk12E = D_8013B0D8->unk098;
    for (i = 0; i < D_8013B0D8->unk0FA; i++) {
        D_8013B0D8->unk12E -= D_8013B0D8->unk12E >> 2;
    }

    if (D_8013B0D8->unk0E8 == 0 && D_8013B0D8->unk136 == 1 && D_8013B0D8->unk12E > 0x1400) {
        D_8013B0D8->unk12E = 0x1400;
    }

    if (D_8013B0D8->unk0FC != 0) {
        if (D_8013AFE2 == 1 && D_8013AFE0 == 0) {
            D_8013B0D8->unk12E += 0x1000;
        } else {
            D_8013B0D8->unk12E += 0xC00;
        }
    }

    player = D_8013B0D8;
    if (!player->unk008 || !player->speedControlState) {
        delta = gPlayerRankSpeedAdjustments[player->unk935].adjustment;
        delta -= player->unk132;
        if (delta > 32) {
            delta = 32;
        }
        if (delta < -48) {
            delta = -48;
        }
        player->unk132 += delta;
        player->unk12E += player->unk132;
    } else {
        switch (player->speedControlState) {
        case 1:
            /* Apply the neutral speed multiplier. */
            player->unk12E *= 1;
            break;
        case 2:
            player->unk12E /= 3;
            break;
        case 3:
            if (player->unk935 != 3) {
                player->unk12E -= player->unk12E >> 4;
            }
            break;
        }
    }

    if (D_8013B0D8->unk008 && D_8013B0D8->unk934 >= selectedFinishLapCountAbsolute - 1) {
        finishSegment = D_80123D80[D_8013AFCE].unk00;
        if (D_8013B0D8->unk92E > finishSegment - 20 && D_8013B0D8->unk92E < finishSegment) {
            D_8013B0D8->unk12E = D_8013B0D8->unk098;
        }
    }

    delta = D_8013B0D8->unk12E - D_8013B0D8->unk130;
    if (delta > 56) {
        delta = 56;
    }
    if (delta < -56) {
        delta = -56;
    }
    D_8013B0D8->unk130 += delta;
    D_8013B0D8->unk128 &= ~0x800;
    if (D_8013B0D8->unk008) {
        updateCurrentPlayerCourseTrigger();
    }

    D_8013B0D8->modelRotation.vx = D_8013B0D8->modelRotation.vy = D_8013B0D8->modelRotation.vz = 0;
    D_8013B0D8->unk084 = D_8013B0D8->unk086 = D_8013B0D8->unk088 = 0;
    D_8013B0D8->unk128 &= ~2;
}
