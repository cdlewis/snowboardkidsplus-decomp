#include "../../engine/resource_services_internal.h"

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

void updateCurrentPlayerModelTransforms(void) {
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
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[0].coord, D_8013AF90, D_8013AF88);
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[0] = D_8013AF88->vx + currentPlayerAbsolute->unk62C[0].x;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[1] = D_8013AF88->vy + currentPlayerAbsolute->unk62C[0].y;
            currentPlayerAbsolute->model.modelCoordinates[0].coord.t[2] = D_8013AF88->vz + currentPlayerAbsolute->unk62C[0].z;
        } else {
            ApplyMatrix(&currentPlayerAbsolute->model.modelCoordinates[parent].coord, (SVECTOR *)&currentPlayerAbsolute->unk62C[i], D_8013AF88);
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[0] = D_8013AF88->vx + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[0];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[1] = D_8013AF88->vy + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[1];
            currentPlayerAbsolute->model.modelCoordinates[i].coord.t[2] = D_8013AF88->vz + currentPlayerAbsolute->model.modelCoordinates[parent].coord.t[2];
        }
        func_8001E3E4((SVECTOR *)&currentPlayerAbsolute->unk5BC[i], &currentPlayerAbsolute->model.modelCoordinates[i]);
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
    updateCurrentPlayerModelTransforms();
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

INCLUDE_ASM("asm/nonmatchings/race/player/player_animation", func_8001758C);
