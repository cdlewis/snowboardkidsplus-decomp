#pragma once

#include "game/engine/task.h"
#include "psx/libgs.h"

GameTask *createQuadAnimationTask(u16 unusedId, VECTOR *position);

/* World position copied into the position-effect task resource. */
typedef struct PositionEffectOrigin {
    s32 x, y, z;
    s32 pad;
} PositionEffectOrigin;

void startPositionEffectTask(const PositionEffectOrigin *position);
