#include "common.h"
#include "game/race/player/race_player.h"
#include "game/engine/angles.h"
#include "game/engine/game_options.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_geometry.h"
#include "game/menu/selected_records.h"
#include "game/engine/random.h"
#include "game/engine/task.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

typedef struct PlayerRankSpeedAdjustment {
    s16 adjustment;
    s16 unk02;
} PlayerRankSpeedAdjustment;

extern SelectedRecord *D_8013B1F0 __attribute__((section(".data")));
extern s16 D_8013B1FE __attribute__((section(".data")));
extern s16 D_8013AFCE __attribute__((section(".data")));
extern VECTOR *D_8013AF88 __attribute__((section(".data")));
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern MATRIX *D_8013AF98 __attribute__((section(".data")));
extern u8 D_8013AFE0 __attribute__((section(".data")));
extern u8 D_8013AFE2 __attribute__((section(".data")));
extern GameTask *D_8013AE88 __attribute__((section(".data")));

const PlayerRankSpeedAdjustment gPlayerRankSpeedAdjustments[4]
    __attribute__((section(".data.player_rank_speed_adjustments"))) = {
    {0, 0},
    {0x200, 0},
    {0x400, 0},
    {0x800, 0},
};

VECTOR gPlayerSlopeVelocityScratch
    __attribute__((section(".data.player_slope_velocity_scratch"))) = {0, 0, 0, 0};

void copySelectedRecordToCurrentPlayer(void) {
    RacePlayer *player = currentPlayerAbsolute;
    player->unk116 = D_8013B1F0[D_8013B1FE].unk8;
    player->unk11A = D_8013B1F0[D_8013B1FE].unkA;
}

s16 smoothAndScaleCurrentPlayerInput(s32 magnitude, s16 input) {
    s16 current;
    s16 factor;
    if (magnitude < 0) {
        magnitude = -magnitude;
    }
    current = currentPlayerAbsolute->unk122;
    input = (input << 4) - current;
    if (input > 128) {
        input = 128;
    }
    if (input < -128) {
        input = -128;
    }
    current += input;
    input = current * currentPlayerAbsolute->unk124 / 63;
    currentPlayerAbsolute->unk122 = current;
    if (magnitude > 4096) {
        factor = 279;
    } else {
        factor = magnitude * 279 / 4096;
    }
    return input * factor / 496;
}

void clampVectorToCurrentPlayerLimit(VECTOR *vector) {
    s32 length = SquareRoot0(vector->vx * vector->vx + vector->vz * vector->vz);
    if (length > currentPlayerAbsolute->unk130) {
        vector->vx = vector->vx * currentPlayerAbsolute->unk130 / length;
        vector->vz = vector->vz * currentPlayerAbsolute->unk130 / length;
    }
}

void clampCurrentPlayerHorizontalVector(VECTOR *vector) {
    s32 magnitude;

    magnitude = SquareRoot0(vector->vx * vector->vx + vector->vz * vector->vz);
    if (magnitude > currentPlayerAbsolute->unk130 / 2) {
        vector->vx = vector->vx * (currentPlayerAbsolute->unk130 / 2) / magnitude;
        vector->vz = vector->vz * (currentPlayerAbsolute->unk130 / 2) / magnitude;
    }
}

void updateCurrentPlayerSlopeVelocity(s32 push, s32 boost, s32 lateralDrag, s32 forwardDrag, s32 reverseDrag) {
    s32 pitchCos, sine, cosine;
    s32 slopeX, slopeZ;
    s32 drag, sideDrag;
    s32 limitReverse;
    s16 heading;

    pitchCos = rcos(currentPlayerAbsolute->unk0C0);
    currentPlayerAbsolute->unk060 = (currentPlayerAbsolute->unk060 << 12) / pitchCos;
    currentPlayerAbsolute->unk064 = (currentPlayerAbsolute->unk064 << 12) / pitchCos;

    sine = rsin(currentPlayerAbsolute->unk120);
    slopeX = rsin(currentPlayerAbsolute->unk11E) * (s16)currentPlayerAbsolute->unk09E / 4096 * sine / 4096;
    slopeZ = rcos(currentPlayerAbsolute->unk11E) * (s16)currentPlayerAbsolute->unk09E / 4096 * sine / 4096;
    currentPlayerAbsolute->unk060 += slopeX;
    currentPlayerAbsolute->unk064 += slopeZ;

    sine = rsin(-(s16)currentPlayerAbsolute->unk110);
    cosine = rcos(-(s16)currentPlayerAbsolute->unk110);
    gPlayerSlopeVelocityScratch.vx = (currentPlayerAbsolute->unk060 * cosine + currentPlayerAbsolute->unk064 * sine) / 4096;
    gPlayerSlopeVelocityScratch.vz = (currentPlayerAbsolute->unk060 * -sine + currentPlayerAbsolute->unk064 * cosine) / 4096;
    gPlayerSlopeVelocityScratch.vy = currentPlayerAbsolute->unk062;

    if (push > 0) {
        if (gPlayerSlopeVelocityScratch.vz < 0xC00) {
            gPlayerSlopeVelocityScratch.vz += push;
        }
    } else if (gPlayerSlopeVelocityScratch.vz >= -0xBFF) {
        gPlayerSlopeVelocityScratch.vz += push;
    }
    limitReverse = 0;
    gPlayerSlopeVelocityScratch.vz += boost;
    if (gPlayerSlopeVelocityScratch.vy > 0) {
        gPlayerSlopeVelocityScratch.vy = 0;
    }
    drag = forwardDrag;
    if (gPlayerSlopeVelocityScratch.vz <= 0) {
        drag = reverseDrag;
        if (forwardDrag != drag) {
            limitReverse = 1;
        }
    }
    sideDrag = lateralDrag;
    if (limitReverse) {
        clampCurrentPlayerHorizontalVector(&gPlayerSlopeVelocityScratch);
    } else {
        clampVectorToCurrentPlayerLimit(&gPlayerSlopeVelocityScratch);
    }

    if (gPlayerSlopeVelocityScratch.vz >= 0) {
        if (gPlayerSlopeVelocityScratch.vz > drag) {
            gPlayerSlopeVelocityScratch.vz -= drag;
        } else {
            gPlayerSlopeVelocityScratch.vz = 0;
        }
    } else if (gPlayerSlopeVelocityScratch.vz < -drag) {
        gPlayerSlopeVelocityScratch.vz += drag;
    } else {
        gPlayerSlopeVelocityScratch.vz = 0;
    }

    if (gPlayerSlopeVelocityScratch.vx >= 0) {
        if (gPlayerSlopeVelocityScratch.vx > sideDrag) {
            gPlayerSlopeVelocityScratch.vx -= sideDrag;
        } else {
            gPlayerSlopeVelocityScratch.vx = 0;
        }
    } else if (gPlayerSlopeVelocityScratch.vx < -sideDrag) {
        gPlayerSlopeVelocityScratch.vx += sideDrag;
    } else {
        gPlayerSlopeVelocityScratch.vx = 0;
    }

    currentPlayerAbsolute->unk096 = gPlayerSlopeVelocityScratch.vx;
    currentPlayerAbsolute->unk094 = gPlayerSlopeVelocityScratch.vz;
    sine = rsin((s16)currentPlayerAbsolute->unk110);
    cosine = rcos((s16)currentPlayerAbsolute->unk110);
    currentPlayerAbsolute->unk060 = (gPlayerSlopeVelocityScratch.vx * cosine + gPlayerSlopeVelocityScratch.vz * sine) / 4096;
    currentPlayerAbsolute->unk064 = (gPlayerSlopeVelocityScratch.vx * -sine + gPlayerSlopeVelocityScratch.vz * cosine) / 4096;
    currentPlayerAbsolute->unk0BC = SquareRoot0(currentPlayerAbsolute->unk060 * currentPlayerAbsolute->unk060 +
                                    currentPlayerAbsolute->unk064 * currentPlayerAbsolute->unk064);
    heading = getXZHeading(0, 0, currentPlayerAbsolute->unk060, currentPlayerAbsolute->unk064);
    currentPlayerAbsolute->unk0BE = heading;
    currentPlayerAbsolute->unk0C0 = rcos(heading - currentPlayerAbsolute->unk11E) * currentPlayerAbsolute->unk120 / 4096;
    sine = rsin(currentPlayerAbsolute->unk0C0);
    currentPlayerAbsolute->unk062 = sine * currentPlayerAbsolute->unk0BC / 4096;
    currentPlayerAbsolute->unk060 = currentPlayerAbsolute->unk060 * pitchCos / 4096;
    currentPlayerAbsolute->unk064 = currentPlayerAbsolute->unk064 * pitchCos / 4096;
}

/* DECOMP_ATTEMPT: archive 12/14 */
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
    currentPlayerAbsolute = initialPlayer;
    initialPlayer->surfaceState.fields.unk137 = initialPlayer->surfaceState.fields.unk136;
    initialPlayer->unk938 = 0;
    if (currentPlayerAbsolute->unk128 & 0x201) {
        currentPlayerAbsolute->surfaceState.fields.unk136 = 0;
    } else {
        currentPlayerAbsolute->surfaceState.fields.unk136 = func_8004AAA0(currentPlayerAbsolute->unk92E, currentPlayerAbsolute->x >> 8, currentPlayerAbsolute->z >> 8);
    }
    if (currentPlayerAbsolute->unk128 & 0x2000000) {
        currentPlayerAbsolute->surfaceState.fields.unk136 = 11;
    }
    if ((u32)((u8)currentPlayerAbsolute->surfaceState.fields.unk136 - 17) < 8 && !(currentPlayerAbsolute->unk128 & 0x200)) {
        getGeometryPlacementTransform(currentPlayerAbsolute->unk92E, &x, &y, &z, &angle);
        surface = currentPlayerAbsolute->surfaceState.fields.unk136;
        D_8013AF90->vx = 0;
        D_8013AF90->vz = 0;
        angle += (surface - 17) * 512;
        D_8013AF90->vy = angle;
        RotMatrix(D_8013AF90, D_8013AF98);

        D_8013AF90->vx = 0;
        D_8013AF90->vy = 0;
        D_8013AF90->vz = 384;
        ApplyMatrix(D_8013AF98, D_8013AF90, D_8013AF88);

        currentPlayerAbsolute->x += D_8013AF88->vx;
        currentPlayerAbsolute->y += D_8013AF88->vy;
        currentPlayerAbsolute->z += D_8013AF88->vz;
    }

    currentPlayerAbsolute->unk060 = currentPlayerAbsolute->x - currentPlayerAbsolute->unk050;
    currentPlayerAbsolute->unk064 = currentPlayerAbsolute->z - currentPlayerAbsolute->unk058;
    /* Copy all four words of the position snapshot, including its fourth word. */
    *(VECTOR *)&currentPlayerAbsolute->unk050 = *(VECTOR *)&currentPlayerAbsolute->x;

    currentPlayerAbsolute->unk96C = 0;
    currentPlayerAbsolute->unk12E = currentPlayerAbsolute->unk098;
    for (i = 0; i < currentPlayerAbsolute->unk0FA; i++) {
        currentPlayerAbsolute->unk12E -= currentPlayerAbsolute->unk12E >> 2;
    }

    if (currentPlayerAbsolute->unk0E8 == 0 && currentPlayerAbsolute->surfaceState.fields.unk136 == 1 && currentPlayerAbsolute->unk12E > 0x1400) {
        currentPlayerAbsolute->unk12E = 0x1400;
    }

    if (currentPlayerAbsolute->unk0FC != 0) {
        if (D_8013AFE2 == 1 && D_8013AFE0 == 0) {
            currentPlayerAbsolute->unk12E += 0x1000;
        } else {
            currentPlayerAbsolute->unk12E += 0xC00;
        }
    }

    player = currentPlayerAbsolute;
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

    if (currentPlayerAbsolute->unk008 && currentPlayerAbsolute->unk934 >= selectedFinishLapCountAbsolute - 1) {
        finishSegment = D_80123D80[D_8013AFCE].unk00;
        if (currentPlayerAbsolute->unk92E > finishSegment - 20 && currentPlayerAbsolute->unk92E < finishSegment) {
            currentPlayerAbsolute->unk12E = currentPlayerAbsolute->unk098;
        }
    }

    delta = currentPlayerAbsolute->unk12E - currentPlayerAbsolute->unk130;
    if (delta > 56) {
        delta = 56;
    }
    if (delta < -56) {
        delta = -56;
    }
    currentPlayerAbsolute->unk130 += delta;
    currentPlayerAbsolute->unk128 &= ~0x800;
    if (currentPlayerAbsolute->unk008) {
        updateCurrentPlayerCourseTrigger();
    }

    currentPlayerAbsolute->modelRotation.vx = currentPlayerAbsolute->modelRotation.vy = currentPlayerAbsolute->modelRotation.vz = 0;
    currentPlayerAbsolute->unk084 = currentPlayerAbsolute->unk086 = currentPlayerAbsolute->unk088 = 0;
    currentPlayerAbsolute->unk128 &= ~2;
}
