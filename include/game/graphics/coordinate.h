#pragma once

#include "psx/libgs.h"

/* Shared direction scratch vector used by visual effects. */
extern VECTOR D_800EB22C;

extern GsCOORDINATE2 D_80129C60[];
extern SVECTOR D_8012A5C0[];

extern VECTOR *gScratchVector0;
extern SVECTOR *D_8013AF90;
extern SVECTOR *D_8013AF94;
extern MATRIX *gScratchMatrix0;

void setCoordinateRotation(SVECTOR *rotation, GsCOORDINATE2 *coordinate);
void setCoordinateRotationZYX(SVECTOR *rotation, GsCOORDINATE2 *coordinate);

void adjustDebugPositionAndDisplay(VECTOR *position);
void computeVectorYawAndPitch(VECTOR *vector, s16 *yaw, s16 *pitch);

void reflectVelocityAndResolveGeometryBoundary(s16 index, s32 x, s32 z, s32 radius,
                                               s32 *displacementX, s32 *displacementZ,
                                               s32 *directionX, s32 *directionZ);
void resolveGeometryBoundaryCollision(s16 index, s32 x, s32 z, s32 radius, s32 mode,
                                      s32 *displacementX, s32 *displacementZ, s32 *velocityX, s32 *velocityZ);
void getGeometryBoundaryDisplacement(s16 index, s32 x, s32 z, s32 radius, s32 *displacementX, s32 *displacementZ);
