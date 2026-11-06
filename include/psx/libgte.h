#pragma once

#include "decomp/types.h"

typedef struct {
    s32 vx, vy, vz, pad;
} VECTOR;

VECTOR *OuterProduct0(VECTOR *first, VECTOR *second, VECTOR *result);
