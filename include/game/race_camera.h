#pragma once

#include "common.h"

/* Per-player camera position and angles; records have a 0x1C-byte stride. */
typedef struct PlayerCamera {
    s32 x, y, z;
    s16 yaw, pitch;
    u8 unk10[4];
    s16 unk14;
    u8 unk16[6];
} PlayerCamera;

extern PlayerCamera D_801241E8[];

void updatePlayerCameraAngles(u16 playerIndex);

void updateCurrentPlayerCameraTarget(void);

void updateCameraRotationMatrices(u16 playerIndex);
