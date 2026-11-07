#pragma once

#include "psx/libgs.h"

/* Shared direction scratch vector used by visual effects. */
extern VECTOR D_800EB22C;

extern VECTOR *D_8013AF88 __attribute__((section(".data")));
extern SVECTOR *D_8013AF90 __attribute__((section(".data")));
extern SVECTOR *D_8013AF94 __attribute__((section(".data")));
extern MATRIX *D_8013AF98 __attribute__((section(".data")));

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate);

void adjustDebugPositionAndDisplay(VECTOR *position);
