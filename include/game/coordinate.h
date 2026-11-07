#pragma once

#include "psx/libgs.h"

#ifdef COORDINATE_GP_ADDRESSING
#define COORDINATE_DATA
#else
#define COORDINATE_DATA __attribute__((section(".data")))
#endif

/* Shared direction scratch vector used by visual effects. */
extern VECTOR D_800EB22C;

extern VECTOR *D_8013AF88 COORDINATE_DATA;
extern SVECTOR *D_8013AF90 COORDINATE_DATA;
extern SVECTOR *D_8013AF94 COORDINATE_DATA;
extern MATRIX *D_8013AF98 COORDINATE_DATA;

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate);

void adjustDebugPositionAndDisplay(VECTOR *position);
void computeVectorYawAndPitch(VECTOR *vector, s16 *yaw, s16 *pitch);

void func_8004BC3C(s16 index, s32 x, s32 z, s32 radius, s32 *displacementX, s32 *displacementZ, s32 *directionX, s32 *directionZ);
void func_8004C968(s16 index, s32 x, s32 z, s32 radius, s32 *displacementX, s32 *displacementZ);

#undef COORDINATE_DATA
