#pragma once

#include "common.h"

/* Per-player camera position and angles; records have a 0x1C-byte stride. */
typedef struct PlayerCamera {
    s32 x, y, z;
    s16 yaw, pitch;
    s16 unk10, unk12, unk14;
    u8 unk16, unk17;
    s16 unk18;
    s16 unk1A;
} PlayerCamera;

extern PlayerCamera D_801241E8[];

void initPlayerCamera(u16 index);
void initPlayerCameraAndTarget(u16 index);
void func_8003E050(u16 index);
void updatePlayerCameraAngles(u16 playerIndex);

void updateCurrentPlayerCameraTarget(void);

void updateCameraRotationMatrices(u16 playerIndex);
void clampPlayerCameraHeight(u16 playerIndex);

/* Nearest camera distance in whole units, using the sum of axis distances. */
s32 getNearestCameraManhattanDistance(s32 x, s32 y, s32 z);
