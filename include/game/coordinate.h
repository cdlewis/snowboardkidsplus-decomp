#pragma once

#include "psx/libgs.h"

extern MATRIX *D_8013AF98 __attribute__((section(".data")));

void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate);
