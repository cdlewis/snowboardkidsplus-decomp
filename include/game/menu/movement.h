#pragma once

#include "decomp/types.h"

typedef struct Movement2D {
    s32 x;
    s32 y;
    s32 unk08;
    s32 remainingX;
    s32 remainingY;
    s32 stepX;
    s32 stepY;
    s32 delay;
    s32 modeX;
    s32 modeY;
} Movement2D;

void initMovement2D(Movement2D *movement, s32 x, s32 y);

void configureMovement2D(Movement2D *movement, s32 x, s32 y, s32 stepX, s32 stepY, s32 delay);
