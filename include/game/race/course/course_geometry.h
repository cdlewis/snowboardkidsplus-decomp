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

/* Packed XYZ halfwords, three per vertex. */
extern s16 *gCourseGeometryVertexData;
extern SelectedRecord *gCourseGeometryTriangleRecords;
/* 0x1C-byte region records, also viewed as GeometryPlacement entries. */
extern s16 *gCourseGeometryRegionData;
extern s16 D_8013B1F8;
extern s16 D_8013B1FA;
extern s16 D_8013B1FC;
extern s16 gSelectedGeometryRecordIndex;
extern s16 *gSelectedGeometryVertex0;
extern s16 *gSelectedGeometryVertex1;
extern s16 *gSelectedGeometryVertex2;

extern s32 gGeometryProjectionBasisX __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryProjectionBasisZ __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryProjectionLength __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryProjectionOffsetX __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryProjectionOffsetZOrDistance __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryEdgeDeltaX __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryEdgeDeltaZOrCorrection __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryResolvedX __attribute__((section(".sdata.geometry_edge_projection")));
extern s32 gGeometryResolvedZ __attribute__((section(".sdata.geometry_edge_projection")));


/* Absolute-addressed view of the placement data used by task effects. */
extern s16 *geometryPlacementDataAbsolute __attribute__((section(".data")));

void bindGeometryRecordData(s16 *data);
void selectGeometryRecord(s16 index);
void computeSelectedTriangleNormal(VECTOR *normal);
void computeSelectedTriangleAngles(s16 *pitch, s16 *yaw);

u8 getCourseSurfaceAtPoint(s16 index, s32 x, s32 z);
void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle);

s32 getGeometryPlacementSignedDistance(s16 index, s32 x, s32 z);

s32 getCourseHeightAtPoint(s16 index, s32 x, s32 z);
s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z);

void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit);
void enforceGeometryEdgeDistance(s32 limit);
