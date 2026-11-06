#include "common.h"
#define RACE_PLAYER_ABSOLUTE_ADDRESSING
#include "game/race_player.h"
#include "game/selected_record.h"
#include "game/selected_records.h"

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

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002A6AC);

void clampCurrentPlayerHorizontalVector(VECTOR *vector) {
    s32 magnitude;

    magnitude = SquareRoot0(vector->vx * vector->vx + vector->vz * vector->vz);
    if (magnitude > D_8013B0D8->unk130 / 2) {
        vector->vx = vector->vx * (D_8013B0D8->unk130 / 2) / magnitude;
        vector->vz = vector->vz * (D_8013B0D8->unk130 / 2) / magnitude;
    }
}

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002A8E4);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002AE38);

INCLUDE_ASM("asm/nonmatchings/game/unk_8002A560", func_8002AE6C);
