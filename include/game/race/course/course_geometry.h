#pragma once

#include "decomp/types.h"
#include "psx/libgte.h"

typedef struct CourseGeometryVertex {
    s16 x, y, z;
} CourseGeometryVertex;

/* Geometry entries have a 0x1C-byte stride; the origin references a vertex. */
typedef struct GeometryPlacement {
    s16 unk00[8];
    s16 vertexIndex;
    u16 angle;
    s16 unk14[4];
} GeometryPlacement;

typedef struct SelectedRecord {
    u16 vertexIndices[3];
    u8 surfaceType;
    u8 skipFirstEdge;
    s16 unk8;
    s16 unkA;
} SelectedRecord;

extern s16 *D_8013B1EC;
extern SelectedRecord *D_8013B1F0;
extern s16 *D_8013B1F4;
extern s16 D_8013B1F8;
extern s16 D_8013B1FA;
extern s16 D_8013B1FC;
extern s16 D_8013B1FE;
extern s16 *D_8013B200;
extern s16 *D_8013B204;
extern s16 *D_8013B208;


/* Absolute-addressed view of the placement data used by task effects. */
extern s16 *geometryPlacementDataAbsolute __attribute__((section(".data")));

void bindGeometryRecordData(s16 *data);
void selectGeometryRecord(s16 index);
void computeSelectedTriangleNormal(VECTOR *normal);
void computeSelectedTriangleAngles(s16 *pitch, s16 *yaw);

u8 getCourseSurfaceAtPoint(s16 index, s32 x, s32 z);
void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle);

s32 getGeometryPlacementSignedDistance(s16 index, s32 x, s32 z);

s32 func_8004A764(s16 index, s32 x, s32 z);
s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z);

void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit);
void enforceGeometryEdgeDistance(s32 limit);
