#pragma once

#include "decomp/types.h"

typedef struct CVECTOR {
    u8 r, g, b, cd;
} CVECTOR;

/* Psy-Q inline_c.h: store the GTE depth cue interpolation value. */
#define gte_stdp(r0) \
    __asm__ volatile("swc2 $8, 0(%0)" : : "r"(r0) : "memory")

void DpqColor(CVECTOR *color, s32 depthCue, CVECTOR *out);

typedef struct {
    s32 vx, vy, vz, pad;
} VECTOR;

VECTOR *OuterProduct0(VECTOR *first, VECTOR *second, VECTOR *result);
