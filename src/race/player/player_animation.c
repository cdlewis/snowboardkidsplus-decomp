#include "../../engine/resource_services_internal.h"
#include "game/engine/random.h"
#include "game/graphics/coordinate.h"
#include "game/io/dat_resources.h"
#include "game/race/player/race_player.h"
#include "psx/libgs.h"

extern VECTOR *gScratchVector0 __attribute__((section(".data")));
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));

u8 D_80120200[16] __attribute__((section(".data.player_model_parts"))) = {
    255, 0, 1, 2, 1, 4, 1, 6, 6, 8, 6, 10, 0, 0, 0, 0
};

u8 D_80120210[16] __attribute__((section(".data.player_model_parts"))) = {
    0, 1, 4, 5, 2, 3, 6, 7, 10, 11, 8, 9, 0, 0, 0, 0
};

s16 D_80120220[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 16, 0, -12, 12, 0, 0, -17, 0, 12, 12, 0, 0, -17, 0, -15, 11, 0, 15, 11, 0
};

s16 D_80120274[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 14, 0, -10, 12, 0, 0, -17, 0, 10, 12, 0, 0, -17, 0, -15, 11, 0, 15, 11, 0
};

s16 D_801202C8[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 50, 0, -8, -8, 0, 0, -16, 0, 8, -8, 0, 0, -16, 0, 0, 5, 0, 0, 31, 0, -24, 24, 0, 0, -28, 0, 24, 24, 0, 0, -28, 0, -17, 13, 0, 17, 13, 0
};

s16 D_8012031C[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 16, 0, -12, 12, 0, 0, -17, 0, 12, 12, 0, 0, -17, 0, -10, 11, 0, 10, 11, 0
};

s16 D_80120370[42] __attribute__((section(".data.player_model_parts"))) = {
    0, 0, 0, 0, 40, 0, -6, -3, 0, 0, -12, 0, 6, -3, 0, 0, -12, 0, 0, 3, 0, 0, 15, 0, -10, 12, 0, 0, -17, 0, 10, 12, 0, 0, -17, 0, -15, 11, 0, 15, 11, 0
};

s16 *D_801203C4[10] __attribute__((section(".data.player_model_parts"))) = {
    D_80120220, D_80120274, D_80120220, D_80120370, D_801202C8, D_80120220, D_80120220, D_801202C8, D_8012031C, D_8012031C
};

RacePlayerVector gPlayerAnimationPose[12] __attribute__((section(".data.player_animation_pose"), aligned(4))) = {0};

void startCurrentPlayerAnimation(u8 animation, u8 mode) {
    RacePlayer *player;
    u32 *offsets = DAT_ANIMATION_ARCHIVE->offsets;
    currentPlayerAbsolute->animationIndex = D_800EB09C[currentPlayerAbsolute->characterId][animation];
    currentPlayerAbsolute->animationFlags = mode;
    currentPlayerAbsolute->animationStep = 0;
    player = currentPlayerAbsolute;
    player->animationStream = (u16 *)(PLAYER_ANIMATION_ARCHIVE + offsets[player->animationIndex]);
    player->animationFrameCount = *player->animationStream;
    player->animationStream++;
    player->animationTicks = *player->animationStream;
    player->animationStream++;
    updateCurrentPlayerAnimation();
    currentPlayerAbsolute->animationFlags |= 0x20;
}

void updateCurrentPlayerModelTransforms(RacePlayer *unused) {
    u16 i;
    u8 parent;
    RacePlayer *player;
    if (currentPlayerAbsolute->unk128 & 0x400) {
        i = 0;
        player = currentPlayerAbsolute;
        player->unk62C[0].x = -player->unk62C[0].x;
        for (; i < 12; i++) {
            player->unk5BC[i].x = gPlayerAnimationPose[player->unk14C[i]].x;
            player->unk5BC[i].y = -gPlayerAnimationPose[player->unk14C[i]].y;
            player->unk5BC[i].z = -gPlayerAnimationPose[player->unk14C[i]].z;
        }
    }
    if (!(currentPlayerAbsolute->animationFlags & 4)) {
        currentPlayerAbsolute->unk92C = currentPlayerAbsolute->unk5BC[0].x;
    } else {
        if ((s16)currentPlayerAbsolute->unk92C >= 0x800)
            currentPlayerAbsolute->unk92C -= 0x1000;
        if ((s16)currentPlayerAbsolute->unk92C > 0x40)
            currentPlayerAbsolute->unk92C -= 0x40;
        else if ((s16)currentPlayerAbsolute->unk92C < -0x40)
            currentPlayerAbsolute->unk92C += 0x40;
        else {
            currentPlayerAbsolute->unk92C = 0;
            currentPlayerAbsolute->animationFlags &= ~4;
        }
        currentPlayerAbsolute->unk5BC[0].x = currentPlayerAbsolute->unk92C;
    }
    for (i = 0; i < 12; i++) {
        parent = currentPlayerAbsolute->unk13E[i];
        RotMatrix((SVECTOR *)&currentPlayerAbsolute->unk5BC[i], &currentPlayerAbsolute->model.modelCoordinates[i].coord);
        if (i == 0) {
            D_8013AF90->vx = 0;
            D_8013AF90->vy = 0;
            D_8013AF90->vz = 0;
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[0].coord, D_8013AF90, gScratchVector0);
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[0] = gScratchVector0->vx + currentPlayerAbsolute->unk62C[0].x;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[1] = gScratchVector0->vy + currentPlayerAbsolute->unk62C[0].y;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[2] = gScratchVector0->vz + currentPlayerAbsolute->unk62C[0].z;
        } else {
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[parent].coord, (SVECTOR *)&currentPlayerAbsolute->unk62C[i], gScratchVector0);
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[0] = gScratchVector0->vx + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[0];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[1] = gScratchVector0->vy + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[1];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[2] = gScratchVector0->vz + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[2];
        }
        setCoordinateRotation((SVECTOR *)&currentPlayerAbsolute->unk5BC[i], &currentPlayerAbsolute->model.modelCoordinates[i]);
    }
}

s32 updateCurrentPlayerAnimation(void) {
    u16 i;
    RacePlayer *player;
    RacePlayer *restart;
    RacePlayerVector *pose;
    if (currentPlayerAbsolute->animationFlags & 0x20) {
        currentPlayerAbsolute->animationFlags &= ~0x20;
        return 1;
    }
    if (currentPlayerAbsolute->animationFlags & 0x10) return 0;
    currentPlayerAbsolute->animationTicks--;
    if (currentPlayerAbsolute->animationTicks != 0) return 1;
    i = 0;
    pose = gPlayerAnimationPose;
    currentPlayerAbsolute->animationFrame = currentPlayerAbsolute->animationStream;
    player = currentPlayerAbsolute;
    for (; i < 12; i++) {
        pose[i].x = player->unk5BC[i].x = *player->animationStream;
        player->animationStream++;
        pose[i].y = player->unk5BC[i].y = *player->animationStream;
        player->animationStream++;
        pose[i].z = player->unk5BC[i].z = *player->animationStream;
        player->animationStream++;
    }
    currentPlayerAbsolute->unk62C[0].x = ((s16)*currentPlayerAbsolute->animationStream + 2) / 4;
    currentPlayerAbsolute->animationStream++;
    currentPlayerAbsolute->unk62C[0].y = ((s16)*currentPlayerAbsolute->animationStream + 2) / 4;
    currentPlayerAbsolute->animationStream++;
    currentPlayerAbsolute->unk62C[0].z = ((s16)*currentPlayerAbsolute->animationStream + 2) / 4;
    currentPlayerAbsolute->animationStream++;
    currentPlayerAbsolute->unk62C[1].z = -((s16)*currentPlayerAbsolute->animationStream + 2) / 4;
    currentPlayerAbsolute->animationStream++;
    currentPlayerAbsolute->unk62C[1].y = ((s16)*currentPlayerAbsolute->animationStream + 2) / 4;
    currentPlayerAbsolute->animationStream++;
    currentPlayerAbsolute->unk62C[1].x = ((s16)*currentPlayerAbsolute->animationStream + 2) / 4;
    currentPlayerAbsolute->animationStream++;
    updateCurrentPlayerModelTransforms(currentPlayerAbsolute);
    currentPlayerAbsolute->animationFrameCount--;
    if (currentPlayerAbsolute->animationFrameCount == 0) {
        switch (currentPlayerAbsolute->animationFlags & 3) {
        case 1:
            currentPlayerAbsolute->animationStep++;
            restart = currentPlayerAbsolute;
            {
                u32 *offsets = DAT_ANIMATION_ARCHIVE->offsets;
                restart->animationStream = (u16 *)(PLAYER_ANIMATION_ARCHIVE + offsets[restart->animationIndex]);
            }
            restart->animationFrameCount = *restart->animationStream;
            restart->animationStream++;
            restart->animationTicks = *restart->animationStream;
            restart->animationStream++;
            return 1;
        case 2:
            currentPlayerAbsolute->animationFlags |= 0x10;
            currentPlayerAbsolute->animationFrameCount = 1;
            currentPlayerAbsolute->animationTicks = 1;
            currentPlayerAbsolute->animationStream = currentPlayerAbsolute->animationFrame;
            return 0;
        }
        return 0;
    }
    currentPlayerAbsolute->animationTicks = *currentPlayerAbsolute->animationStream;
    currentPlayerAbsolute->animationStream++;
    return 1;
}

void blendCurrentPlayerSteeringPose(s16 animation, s32 weight, s32 scale) {
    RacePlayer *initial = currentPlayerAbsolute;
    RacePlayer *player;
    RacePlayer *blendPlayer;
    RacePlayer *output;
    u8 index = D_800EB09C[initial->characterId][animation];
    RacePlayerVector *pose;
    u16 i, j;
    s32 delta;
    s32 rootX, rootY, rootZ, boardX, boardY, boardZ;

    if (initial->animationIndex != index) {
        u32 *offsets = DAT_ANIMATION_ARCHIVE->offsets;
        RacePlayer *streamPlayer;
        OffsetArchive *archive;

        initial->animationIndex = index;
        currentPlayerAbsolute->animationFlags = 0;
        currentPlayerAbsolute->animationStep = 0;
        streamPlayer = currentPlayerAbsolute;
        archive = DAT_ANIMATION_ARCHIVE;
        streamPlayer->animationStream = (u16 *)((u8 *)archive->offsets + offsets[streamPlayer->animationIndex]);
        player = streamPlayer;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 12; j++) {
                player->blendAngles[i][j].x = *player->animationStream;
                player->blendAngles[i][j].y = *++player->animationStream;
                player->blendAngles[i][j].z = *++player->animationStream;
                player->animationStream++;
            }
            player->blendRoot[i].x = ((s16)*player->animationStream + 2) / 4;
            player->blendRoot[i].y = ((s16)*++player->animationStream + 2) / 4;
            player->blendRoot[i].z = ((s16)*++player->animationStream + 2) / 4;
            player->blendBoard[i].z = -((s16)*++player->animationStream + 2) / 4;
            player->blendBoard[i].y = ((s16)*++player->animationStream + 2) / 4;
            player->blendBoard[i].x = ((s16)*++player->animationStream + 2) / 4;
            player->animationStream += 2;
        }
    }

    j = 0;
    blendPlayer = currentPlayerAbsolute;
    pose = gPlayerAnimationPose;
    for (; j < 12; j++) {
        delta = (blendPlayer->blendAngles[1][j].x - blendPlayer->blendAngles[0][j].x) & 0xFFF;
        if (delta > 0x800)
            delta -= 0x1000;
        pose[j].x = blendPlayer->unk5BC[j].x = (blendPlayer->blendAngles[0][j].x + delta * weight / scale) & 0xFFF;
        delta = (blendPlayer->blendAngles[1][j].y - blendPlayer->blendAngles[0][j].y) & 0xFFF;
        if (delta > 0x800)
            delta -= 0x1000;
        pose[j].y = blendPlayer->unk5BC[j].y = (blendPlayer->blendAngles[0][j].y + delta * weight / scale) & 0xFFF;
        delta = (blendPlayer->blendAngles[1][j].z - blendPlayer->blendAngles[0][j].z) & 0xFFF;
        if (delta > 0x800)
            delta -= 0x1000;
        pose[j].z = blendPlayer->unk5BC[j].z = (blendPlayer->blendAngles[0][j].z + delta * weight / scale) & 0xFFF;
    }

    output = currentPlayerAbsolute;
    rootX = (output->blendRoot[1].x - output->blendRoot[0].x) * weight / scale;
    rootY = (output->blendRoot[1].y - output->blendRoot[0].y) * weight / scale;
    rootZ = (output->blendRoot[1].z - output->blendRoot[0].z) * weight / scale;
    boardX = (output->blendBoard[1].x - output->blendBoard[0].x) * weight / scale;
    boardY = (output->blendBoard[1].y - output->blendBoard[0].y) * weight / scale;
    boardZ = (output->blendBoard[1].z - output->blendBoard[0].z) * weight / scale;
    output->unk62C[0].x = output->blendRoot[0].x + rootX;
    output->unk62C[0].y = output->blendRoot[0].y + rootY;
    output->unk62C[0].z = output->blendRoot[0].z + rootZ;
    output->unk62C[1].x = output->blendBoard[0].x + boardX;
    output->unk62C[1].y = output->blendBoard[0].y + boardY;
    output->unk62C[1].z = output->blendBoard[0].z + boardZ;
    updateCurrentPlayerModelTransforms(output);
}
