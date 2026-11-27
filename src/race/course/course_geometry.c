#include "../../graphics/render_effects_internal.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_geometry.h"
#include "psx/libgte.h"

u32 D_8013B1DC[4] __attribute__((section(".data.geometry_storage"))) = {0, 0, 0, 0};

CourseGeometryVertex *gCourseGeometryVertexData __attribute__((section(".sdata.course_geometry"))) = 0;
SelectedRecord *gCourseGeometryTriangleRecords __attribute__((section(".sdata.course_geometry"))) = 0;
GeometryRegion *gCourseGeometryRegionData __attribute__((section(".sdata.course_geometry"))) = 0;
s16 gCourseGeometryVertexCount __attribute__((section(".sdata.course_geometry"))) = 0;
s16 gCourseGeometryTriangleCount __attribute__((section(".sdata.course_geometry"))) = 0;
s16 gCourseGeometryRegionCount __attribute__((section(".sdata.course_geometry"))) = 0;
s16 gSelectedGeometryRecordIndex __attribute__((section(".sdata.course_geometry"))) = 0;
s16 *gSelectedGeometryVertex0 __attribute__((section(".sdata.course_geometry"))) = 0;
s16 *gSelectedGeometryVertex1 __attribute__((section(".sdata.course_geometry"))) = 0;
s16 *gSelectedGeometryVertex2 __attribute__((section(".sdata.course_geometry"))) = 0;

s32 gGeometryProjectionBasisX __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryProjectionBasisZ __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryProjectionLength __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryProjectionOffsetX __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B634 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryProjectionOffsetZOrDistance __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryEdgeDeltaX __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B640 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryEdgeDeltaZOrCorrection __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryResolvedX __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B64C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryResolvedZ __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

void bindGeometryRecordData(s16 *data) {
    s16 count;

    count = *data++;
    gCourseGeometryVertexData = (CourseGeometryVertex *)data;
    /* Section lengths wrap as signed 16-bit halfword counts. */
    data += (s16)(count * 3);
    gCourseGeometryVertexCount = count;
    count = *data++;
    gCourseGeometryTriangleRecords = (SelectedRecord *)data;
    data += (s16)(count * 6);
    gCourseGeometryTriangleCount = count;
    gCourseGeometryRegionCount = *data++;
    gCourseGeometryRegionData = (GeometryRegion *)data;
}

s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s16 neighbor;

    x0 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[0]].x * 5;
    x1 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[1]].x * 5;
    x2 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[2]].x * 5;
    x3 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[3]].x * 5;
    z0 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[0]].z * 5;
    z1 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[1]].z * 5;
    z2 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[2]].z * 5;
    z3 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[3]].z * 5;
    neighbor = gCourseGeometryRegionData[index].neighbors[0];
    if (neighbor >= 0 && (x0 - x1) * (z - z1) - (z0 - z1) * (x - x1) > 0) return neighbor;
    neighbor = gCourseGeometryRegionData[index].neighbors[1];
    if (neighbor >= 0 && (x3 - x2) * (z - z2) - (z3 - z2) * (x - x2) > 0) return neighbor;
    neighbor = gCourseGeometryRegionData[index].neighbors[3];
    if (neighbor >= 0 && (x2 - x0) * (z - z0) - (z2 - z0) * (x - x0) > 0) return neighbor;
    neighbor = gCourseGeometryRegionData[index].neighbors[2];
    if (neighbor >= 0 && (x1 - x3) * (z - z3) - (z1 - z3) * (x - x3) > 0) return neighbor;
    return index;
}

void selectGeometryRecord(s16 index) {
    CourseGeometryVertex *vertices;
    gSelectedGeometryRecordIndex = index;
    vertices = gCourseGeometryVertexData;
    gSelectedGeometryVertex0 = &vertices[gCourseGeometryTriangleRecords[index].vertexIndices[0]].x;
    gSelectedGeometryVertex1 = &vertices[gCourseGeometryTriangleRecords[index].vertexIndices[1]].x;
    gSelectedGeometryVertex2 = &vertices[gCourseGeometryTriangleRecords[index].vertexIndices[2]].x;
}

s32 getCourseHeightAtPoint(s16 index, s32 x, s32 z) {
    s16 i;
    CourseGeometryVertex *triangleVertices;

    for (i = gCourseGeometryRegionData[index].firstRecord;
         i < gCourseGeometryRegionData[index].lastRecord + 1; i++) {
        SelectedRecord *records = gCourseGeometryTriangleRecords;

        if (records[i].skipFirstEdge == 0) {
            CourseGeometryVertex *vertices = gCourseGeometryVertexData;
            s32 bi = records[i].vertexIndices[1];
            s32 ai = records[i].vertexIndices[0];

            if ((vertices[bi].x - vertices[ai].x) * (z / 5 - vertices[ai].z) -
                (vertices[bi].z - vertices[ai].z) * (x / 5 - vertices[ai].x) > 0) {
                continue;
            }
        }
        {
            CourseGeometryVertex *vertices = gCourseGeometryVertexData;
            s32 bi = records[i].vertexIndices[2];
            s32 ai = records[i].vertexIndices[1];

            if ((vertices[bi].x - vertices[ai].x) * (z / 5 - vertices[ai].z) -
                (vertices[bi].z - vertices[ai].z) * (x / 5 - vertices[ai].x) > 0) {
                continue;
            }
        }
        selectGeometryRecord(i);
        {
            s32 dyC, dzB, dzC, dyB, dxB, dxC;
            s32 yz1, yz2, zx1, xy1, xy2, zx2;
            s32 ci, ai, bi;
            SelectedRecord *selected = gCourseGeometryTriangleRecords;

            ci = selected[i].vertexIndices[2];
            triangleVertices = gCourseGeometryVertexData;
            ai = selected[i].vertexIndices[0];
            bi = selected[i].vertexIndices[1];

            /* Use the triangle normal to solve for the height at (x, z). */
            dyC = triangleVertices[ci].y - triangleVertices[ai].y;
            dzB = triangleVertices[ai].z - triangleVertices[bi].z;
            yz1 = dyC * dzB;
            dzC = triangleVertices[ai].z - triangleVertices[ci].z;
            dyB = triangleVertices[bi].y - triangleVertices[ai].y;
            yz2 = dzC * dyB;
            {
                s32 ax = triangleVertices[ai].x;
                dxB = triangleVertices[bi].x - ax;
            }
            zx1 = dzC * dxB;
            dxC = triangleVertices[ci].x - triangleVertices[ai].x;
            xy1 = dxC * dyB;
            xy2 = dyC * dxB;
            zx2 = dxC * dzB;
            {
                s32 pointZ = z - triangleVertices[ai].z * 5;
                s32 alongZ = (xy1 - xy2) * pointZ;
                s32 pointX = x - triangleVertices[ai].x * 5;
                s32 alongX = (yz1 - yz2) * pointX;
                s32 denominator = zx1 - zx2;
                return -((alongZ - alongX) / denominator + triangleVertices[ai].y * 5);
            }
        }
    }
    return 0x800000;
}

u8 getCourseSurfaceAtPoint(s16 index, s32 x, s32 z) {
    s32 i;
    SelectedRecord *record;
    CourseGeometryVertex *vertices;
    SelectedRecord *records;
    for (i = gCourseGeometryRegionData[index].firstRecord;
         i < gCourseGeometryRegionData[index].lastRecord + 1; i++) {
        vertices = gCourseGeometryVertexData;
        records = gCourseGeometryTriangleRecords;
        record = &records[i];
        if (record->skipFirstEdge == 0) {
            s32 bi = record->vertexIndices[1];
            s32 ai = record->vertexIndices[0];
            s32 bx = vertices[bi].x;
            s32 ax = vertices[ai].x;
            s32 az = vertices[ai].z;
            s32 product = (bx - ax) * (z / 5 - az);
            s32 pointX = x / 5 - ax;
            s32 edgeZ = vertices[bi].z - az;
            if (product - edgeZ * pointX > 0) {
                continue;
            }
        }
        {
            s32 bi = record->vertexIndices[2];
            s32 ai = record->vertexIndices[1];
            s32 bx = vertices[bi].x;
            s32 ax = vertices[ai].x;
            s32 az = vertices[ai].z;
            s32 product = (bx - ax) * (z / 5 - az);
            s32 pointX = x / 5 - ax;
            s32 edgeZ = vertices[bi].z - az;
            if (product - edgeZ * pointX > 0) {
                continue;
            }
        }
        return record->surfaceType;
    }
    return 0;
}

s32 getGeometryPlacementSignedDistance(s16 index, s32 x, s32 z) {
    s32 vertexIndex;
    s32 dx, dz;
    s32 sine, cosine;
    vertexIndex = gCourseGeometryRegionData[index].vertexIndex;
    dx = x - gCourseGeometryVertexData[vertexIndex].x * 5;
    dz = z - gCourseGeometryVertexData[vertexIndex].z * 5;
    sine = rsin((s16)gCourseGeometryRegionData[index].angle);
    cosine = rcos((s16)gCourseGeometryRegionData[index].angle);
    return (-sine * dx + cosine * -dz) / 4096;
}

void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle) {
    *x = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertexIndex].x * 1280;
    *y = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertexIndex].y * 1280;
    *z = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertexIndex].z * 1280;
    *angle = gCourseGeometryRegionData[index].angle;
}

void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit) {
    s32 length, nx, nz, perpendicularX;
    s32 tangent, normal;
    length = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
    nx = (gGeometryEdgeDeltaZOrCorrection << 12) / length;
    nz = (gGeometryEdgeDeltaX << 12) / length;
    gGeometryProjectionLength = length;
    gGeometryProjectionBasisX = nx;
    gGeometryProjectionBasisZ = nz;
    perpendicularX = -nx;
    gGeometryProjectionOffsetZOrDistance = (perpendicularX * gGeometryProjectionOffsetX + nz * gGeometryProjectionOffsetZOrDistance) / 4096;
    if (gGeometryProjectionOffsetZOrDistance < limit) {
        tangent = (nz * *velocityX + nx * *velocityZ) / 4096;
        normal = (perpendicularX * *velocityX + nz * *velocityZ) / 4096;
        if (normal < 0) {
            normal = -normal;
            *velocityX = (nz * tangent - nx * normal) / 4096;
            *velocityZ = (gGeometryProjectionBasisX * tangent + gGeometryProjectionBasisZ * normal) / 4096;
        }
        gGeometryEdgeDeltaZOrCorrection = limit - gGeometryProjectionOffsetZOrDistance;
        gGeometryResolvedX += -gGeometryProjectionBasisX * gGeometryEdgeDeltaZOrCorrection / 4096;
        gGeometryResolvedZ += gGeometryProjectionBasisZ * gGeometryEdgeDeltaZOrCorrection / 4096;
    }
}

void resolveGeometryBoundaryCollision(s16 index, s32 x, s32 z, s32 radius, s32 mode,
                                      s32 *displacementX, s32 *displacementZ, s32 *velocityX, s32 *velocityZ) {
    /* Boundary types are stored as negative neighbor indices. */
    s8 wallType = -mode;
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s32 nx, nz, tangent, normal, length, edgeLength;

    gGeometryResolvedX = x;
    gGeometryResolvedZ = z;
    x0 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[0]].x * 5;
    x1 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[1]].x * 5;
    x2 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[2]].x * 5;
    x3 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[3]].x * 5;
    z0 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[0]].z * 5;
    z1 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[1]].z * 5;
    z2 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[2]].z * 5;
    z3 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[3]].z * 5;

    if (gCourseGeometryRegionData[index].neighbors[0] == wallType) {
        gGeometryEdgeDeltaX = x1 - x0;
        gGeometryEdgeDeltaZOrCorrection = z1 - z0;
        gGeometryProjectionOffsetX = x - x0;
        gGeometryProjectionOffsetZOrDistance = z - z0;
        reflectVelocityAndEnforceGeometryEdgeDistance(velocityX, velocityZ, radius);
    }

    if (gCourseGeometryRegionData[index].neighbors[1] == wallType) {
        gGeometryEdgeDeltaX = x2 - x3;
        gGeometryEdgeDeltaZOrCorrection = z2 - z3;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x3;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z3;
        reflectVelocityAndEnforceGeometryEdgeDistance(velocityX, velocityZ, radius);
    }

    /* Flagged side-edge corners use radial distance beyond the endpoints. */
    if (gCourseGeometryRegionData[index].neighbors[2] == wallType) {
        gGeometryEdgeDeltaX = x3 - x1;
        gGeometryEdgeDeltaZOrCorrection = z3 - z1;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x1;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z1;
        gGeometryProjectionLength = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
        nx = (gGeometryEdgeDeltaZOrCorrection << 12) / gGeometryProjectionLength;
        nz = (gGeometryEdgeDeltaX << 12) / gGeometryProjectionLength;
        gGeometryProjectionBasisX = nx;
        gGeometryProjectionBasisZ = nz;
        tangent = (nz * gGeometryProjectionOffsetX + nx * gGeometryProjectionOffsetZOrDistance) / 4096;
        normal = (-nx * gGeometryProjectionOffsetX + nz * gGeometryProjectionOffsetZOrDistance) / 4096;
        edgeLength = (nz * gGeometryEdgeDeltaX + nx * gGeometryEdgeDeltaZOrCorrection) / 4096;
        if ((gCourseGeometryRegionData[index].cornerFlags & 2) && tangent < 0) {
            tangent = gGeometryResolvedX - x1;
            normal = gGeometryResolvedZ - z1;
            length = SquareRoot0(tangent * tangent + normal * normal);
            gGeometryProjectionLength = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x1;
                gGeometryResolvedZ = normal + z1;
            }
        } else if ((gCourseGeometryRegionData[index].cornerFlags & 8) && edgeLength < tangent) {
            tangent = gGeometryResolvedX - x3;
            normal = gGeometryResolvedZ - z3;
            length = SquareRoot0(tangent * tangent + normal * normal);
            gGeometryProjectionLength = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x3;
                gGeometryResolvedZ = normal + z3;
            }
        } else if (normal < radius) {
            s32 velocityTangent, velocityNormal;
            velocityTangent = (gGeometryProjectionBasisZ * *velocityX + gGeometryProjectionBasisX * *velocityZ) / 4096;
            velocityNormal = (-gGeometryProjectionBasisX * *velocityX + gGeometryProjectionBasisZ * *velocityZ) / 4096;
            if (velocityNormal < 0) {
                velocityNormal = -velocityNormal;
                *velocityX = (gGeometryProjectionBasisZ * velocityTangent - gGeometryProjectionBasisX * velocityNormal) / 4096;
                *velocityZ = (gGeometryProjectionBasisX * velocityTangent + gGeometryProjectionBasisZ * velocityNormal) / 4096;
            }
            gGeometryEdgeDeltaZOrCorrection = radius - normal;
            gGeometryResolvedX += -gGeometryProjectionBasisX * gGeometryEdgeDeltaZOrCorrection / 4096;
            gGeometryResolvedZ += gGeometryProjectionBasisZ * gGeometryEdgeDeltaZOrCorrection / 4096;
        }
    }

    if (gCourseGeometryRegionData[index].neighbors[3] == wallType) {
        gGeometryEdgeDeltaX = x0 - x2;
        gGeometryEdgeDeltaZOrCorrection = z0 - z2;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x2;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z2;
        gGeometryProjectionLength = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
        nx = (gGeometryEdgeDeltaZOrCorrection << 12) / gGeometryProjectionLength;
        nz = (gGeometryEdgeDeltaX << 12) / gGeometryProjectionLength;
        gGeometryProjectionBasisX = nx;
        gGeometryProjectionBasisZ = nz;
        tangent = (nz * gGeometryProjectionOffsetX + nx * gGeometryProjectionOffsetZOrDistance) / 4096;
        normal = (-nx * gGeometryProjectionOffsetX + nz * gGeometryProjectionOffsetZOrDistance) / 4096;
        edgeLength = (nz * gGeometryEdgeDeltaX + nx * gGeometryEdgeDeltaZOrCorrection) / 4096;
        if ((gCourseGeometryRegionData[index].cornerFlags & 4) && tangent < 0) {
            tangent = gGeometryResolvedX - x2;
            normal = gGeometryResolvedZ - z2;
            length = SquareRoot0(tangent * tangent + normal * normal);
            gGeometryProjectionLength = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x2;
                gGeometryResolvedZ = normal + z2;
            }
        } else if ((gCourseGeometryRegionData[index].cornerFlags & 1) && edgeLength < tangent) {
            tangent = gGeometryResolvedX - x0;
            normal = gGeometryResolvedZ - z0;
            length = SquareRoot0(tangent * tangent + normal * normal);
            gGeometryProjectionLength = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x0;
                gGeometryResolvedZ = normal + z0;
            }
        } else if (normal < radius) {
            s32 velocityTangent, velocityNormal;
            velocityTangent = (gGeometryProjectionBasisZ * *velocityX + gGeometryProjectionBasisX * *velocityZ) / 4096;
            velocityNormal = (-gGeometryProjectionBasisX * *velocityX + gGeometryProjectionBasisZ * *velocityZ) / 4096;
            if (velocityNormal < 0) {
                velocityNormal = -velocityNormal;
                *velocityX = (gGeometryProjectionBasisZ * velocityTangent - gGeometryProjectionBasisX * velocityNormal) / 4096;
                *velocityZ = (gGeometryProjectionBasisX * velocityTangent + gGeometryProjectionBasisZ * velocityNormal) / 4096;
            }
            gGeometryEdgeDeltaZOrCorrection = radius - normal;
            gGeometryResolvedX += -gGeometryProjectionBasisX * gGeometryEdgeDeltaZOrCorrection / 4096;
            gGeometryResolvedZ += gGeometryProjectionBasisZ * gGeometryEdgeDeltaZOrCorrection / 4096;
        }
    }

    *displacementX = gGeometryResolvedX - x;
    *displacementZ = gGeometryResolvedZ - z;
}

void reflectVelocityAndResolveGeometryBoundary(s16 index, s32 x, s32 z, s32 radius,
                                               s32 *displacementX, s32 *displacementZ,
                                               s32 *directionX, s32 *directionZ) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s32 along, distance, edgeLength, basisX, basisZ;
    s32 cornerLength, tangent, normal;
    CourseGeometryVertex *vertices = gCourseGeometryVertexData;

    gGeometryResolvedX = x;
    gGeometryResolvedZ = z;
    x0 = vertices[gCourseGeometryRegionData[index].vertices[0]].x * 5;
    x1 = vertices[gCourseGeometryRegionData[index].vertices[1]].x * 5;
    x2 = vertices[gCourseGeometryRegionData[index].vertices[2]].x * 5;
    x3 = vertices[gCourseGeometryRegionData[index].vertices[3]].x * 5;
    z0 = vertices[gCourseGeometryRegionData[index].vertices[0]].z * 5;
    z1 = vertices[gCourseGeometryRegionData[index].vertices[1]].z * 5;
    z2 = vertices[gCourseGeometryRegionData[index].vertices[2]].z * 5;
    z3 = vertices[gCourseGeometryRegionData[index].vertices[3]].z * 5;

    if (gCourseGeometryRegionData[index].neighbors[0] < 0) {
        gGeometryEdgeDeltaX = x1 - x0;
        gGeometryEdgeDeltaZOrCorrection = z1 - z0;
        gGeometryProjectionOffsetX = x - x0;
        gGeometryProjectionOffsetZOrDistance = z - z0;
        reflectVelocityAndEnforceGeometryEdgeDistance(directionX, directionZ, radius);
    }
    if (gCourseGeometryRegionData[index].neighbors[1] < 0) {
        gGeometryEdgeDeltaX = x2 - x3;
        gGeometryEdgeDeltaZOrCorrection = z2 - z3;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x3;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z3;
        reflectVelocityAndEnforceGeometryEdgeDistance(directionX, directionZ, radius);
    }

    if (gCourseGeometryRegionData[index].neighbors[2] < 0) {
        gGeometryEdgeDeltaX = x3 - x1;
        gGeometryEdgeDeltaZOrCorrection = z3 - z1;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x1;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z1;
        gGeometryProjectionLength = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
        basisX = (gGeometryEdgeDeltaZOrCorrection << 12) / gGeometryProjectionLength;
        basisZ = (gGeometryEdgeDeltaX << 12) / gGeometryProjectionLength;
        gGeometryProjectionBasisX = basisX;
        gGeometryProjectionBasisZ = basisZ;
        along = (basisZ * gGeometryProjectionOffsetX + basisX * gGeometryProjectionOffsetZOrDistance) / 4096;
        distance = (-basisX * gGeometryProjectionOffsetX + basisZ * gGeometryProjectionOffsetZOrDistance) / 4096;
        edgeLength = (basisZ * gGeometryEdgeDeltaX + basisX * gGeometryEdgeDeltaZOrCorrection) / 4096;
        if ((gCourseGeometryRegionData[index].cornerFlags & (1 << 1)) && along < 0) {
            along = gGeometryResolvedX - x1;
            distance = gGeometryResolvedZ - z1;
            cornerLength = SquareRoot0(along * along + distance * distance);
            gGeometryProjectionLength = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                gGeometryResolvedX = along + x1;
                gGeometryResolvedZ = distance + z1;
            }
        } else if ((gCourseGeometryRegionData[index].cornerFlags & (1 << 3)) && edgeLength < along) {
            along = gGeometryResolvedX - x3;
            distance = gGeometryResolvedZ - z3;
            cornerLength = SquareRoot0(along * along + distance * distance);
            gGeometryProjectionLength = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                gGeometryResolvedX = along + x3;
                gGeometryResolvedZ = distance + z3;
            }
        } else if (distance < radius) {
            tangent = (gGeometryProjectionBasisZ * *directionX + gGeometryProjectionBasisX * *directionZ) / 4096;
            normal = (-gGeometryProjectionBasisX * *directionX + gGeometryProjectionBasisZ * *directionZ) / 4096;
            if (normal < 0) {
                normal = -normal;
                *directionX = (gGeometryProjectionBasisZ * tangent - gGeometryProjectionBasisX * normal) / 4096;
                *directionZ = (gGeometryProjectionBasisX * tangent + gGeometryProjectionBasisZ * normal) / 4096;
            }
            gGeometryEdgeDeltaZOrCorrection = radius - distance;
            gGeometryResolvedX += -gGeometryProjectionBasisX * gGeometryEdgeDeltaZOrCorrection / 4096;
            gGeometryResolvedZ += gGeometryProjectionBasisZ * gGeometryEdgeDeltaZOrCorrection / 4096;
        }
    }

    if (gCourseGeometryRegionData[index].neighbors[3] < 0) {
        gGeometryEdgeDeltaX = x0 - x2;
        gGeometryEdgeDeltaZOrCorrection = z0 - z2;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x2;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z2;
        gGeometryProjectionLength = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
        basisX = (gGeometryEdgeDeltaZOrCorrection << 12) / gGeometryProjectionLength;
        basisZ = (gGeometryEdgeDeltaX << 12) / gGeometryProjectionLength;
        gGeometryProjectionBasisX = basisX;
        gGeometryProjectionBasisZ = basisZ;
        along = (basisZ * gGeometryProjectionOffsetX + basisX * gGeometryProjectionOffsetZOrDistance) / 4096;
        distance = (-basisX * gGeometryProjectionOffsetX + basisZ * gGeometryProjectionOffsetZOrDistance) / 4096;
        edgeLength = (basisZ * gGeometryEdgeDeltaX + basisX * gGeometryEdgeDeltaZOrCorrection) / 4096;
        if ((gCourseGeometryRegionData[index].cornerFlags & (1 << 2)) && along < 0) {
            along = gGeometryResolvedX - x2;
            distance = gGeometryResolvedZ - z2;
            cornerLength = SquareRoot0(along * along + distance * distance);
            gGeometryProjectionLength = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                gGeometryResolvedX = along + x2;
                gGeometryResolvedZ = distance + z2;
            }
        } else if ((gCourseGeometryRegionData[index].cornerFlags & (1 << 0)) && edgeLength < along) {
            along = gGeometryResolvedX - x0;
            distance = gGeometryResolvedZ - z0;
            cornerLength = SquareRoot0(along * along + distance * distance);
            gGeometryProjectionLength = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                gGeometryResolvedX = along + x0;
                gGeometryResolvedZ = distance + z0;
            }
        } else if (distance < radius) {
            tangent = (gGeometryProjectionBasisZ * *directionX + gGeometryProjectionBasisX * *directionZ) / 4096;
            normal = (-gGeometryProjectionBasisX * *directionX + gGeometryProjectionBasisZ * *directionZ) / 4096;
            if (normal < 0) {
                normal = -normal;
                *directionX = (gGeometryProjectionBasisZ * tangent - gGeometryProjectionBasisX * normal) / 4096;
                *directionZ = (gGeometryProjectionBasisX * tangent + gGeometryProjectionBasisZ * normal) / 4096;
            }
            gGeometryEdgeDeltaZOrCorrection = radius - distance;
            gGeometryResolvedX += -gGeometryProjectionBasisX * gGeometryEdgeDeltaZOrCorrection / 4096;
            gGeometryResolvedZ += gGeometryProjectionBasisZ * gGeometryEdgeDeltaZOrCorrection / 4096;
        }
    }

    *displacementX = gGeometryResolvedX - x;
    *displacementZ = gGeometryResolvedZ - z;
}

void enforceGeometryEdgeDistance(s32 limit) {
    s32 length, nx, nz, perpendicularX;
    length = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
    nx = (gGeometryEdgeDeltaZOrCorrection << 12) / length;
    nz = (gGeometryEdgeDeltaX << 12) / length;
    perpendicularX = -nx;
    gGeometryProjectionOffsetZOrDistance = (perpendicularX * gGeometryProjectionOffsetX + nz * gGeometryProjectionOffsetZOrDistance) / 4096;
    if (gGeometryProjectionOffsetZOrDistance < limit) {
        gGeometryEdgeDeltaZOrCorrection = limit - gGeometryProjectionOffsetZOrDistance;
        gGeometryResolvedX += perpendicularX * gGeometryEdgeDeltaZOrCorrection / 4096;
        gGeometryResolvedZ += nz * gGeometryEdgeDeltaZOrCorrection / 4096;
    }
}

void getGeometryBoundaryDisplacement(s16 index, s32 x, s32 z, s32 radius, s32 *displacementX, s32 *displacementZ) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s32 nx, nz, tangent, normal, length, edgeLength;

    gGeometryResolvedX = x;
    gGeometryResolvedZ = z;
    x0 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[0]].x * 5;
    x1 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[1]].x * 5;
    x2 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[2]].x * 5;
    x3 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[3]].x * 5;
    z0 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[0]].z * 5;
    z1 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[1]].z * 5;
    z2 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[2]].z * 5;
    z3 = gCourseGeometryVertexData[gCourseGeometryRegionData[index].vertices[3]].z * 5;

    if (gCourseGeometryRegionData[index].neighbors[0] < 0) {
        gGeometryEdgeDeltaX = x1 - x0;
        gGeometryEdgeDeltaZOrCorrection = z1 - z0;
        gGeometryProjectionOffsetX = x - x0;
        gGeometryProjectionOffsetZOrDistance = z - z0;
        enforceGeometryEdgeDistance(radius);
    }

    if (gCourseGeometryRegionData[index].neighbors[1] < 0) {
        gGeometryEdgeDeltaX = x2 - x3;
        gGeometryEdgeDeltaZOrCorrection = z2 - z3;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x3;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z3;
        enforceGeometryEdgeDistance(radius);
    }

    /* Flagged side-edge corners use radial distance beyond the endpoints. */
    if (gCourseGeometryRegionData[index].neighbors[2] < 0) {
        gGeometryEdgeDeltaX = x3 - x1;
        gGeometryEdgeDeltaZOrCorrection = z3 - z1;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x1;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z1;
        length = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
        nx = (gGeometryEdgeDeltaZOrCorrection << 12) / length;
        nz = (gGeometryEdgeDeltaX << 12) / length;
        tangent = (nz * gGeometryProjectionOffsetX + nx * gGeometryProjectionOffsetZOrDistance) / 4096;
        normal = (-nx * gGeometryProjectionOffsetX + nz * gGeometryProjectionOffsetZOrDistance) / 4096;
        edgeLength = (nz * gGeometryEdgeDeltaX + nx * gGeometryEdgeDeltaZOrCorrection) / 4096;
        if ((gCourseGeometryRegionData[index].cornerFlags & 2) && tangent < 0) {
            tangent = gGeometryResolvedX - x1;
            normal = gGeometryResolvedZ - z1;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x1;
                gGeometryResolvedZ = normal + z1;
            }
        } else if ((gCourseGeometryRegionData[index].cornerFlags & 8) && edgeLength < tangent) {
            tangent = gGeometryResolvedX - x3;
            normal = gGeometryResolvedZ - z3;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x3;
                gGeometryResolvedZ = normal + z3;
            }
        } else if (normal < radius) {
            gGeometryEdgeDeltaZOrCorrection = radius - normal;
            gGeometryResolvedX += -nx * gGeometryEdgeDeltaZOrCorrection / 4096;
            gGeometryResolvedZ += nz * gGeometryEdgeDeltaZOrCorrection / 4096;
        }
    }

    if (gCourseGeometryRegionData[index].neighbors[3] < 0) {
        gGeometryEdgeDeltaX = x0 - x2;
        gGeometryEdgeDeltaZOrCorrection = z0 - z2;
        gGeometryProjectionOffsetX = gGeometryResolvedX - x2;
        gGeometryProjectionOffsetZOrDistance = gGeometryResolvedZ - z2;
        length = SquareRoot0(gGeometryEdgeDeltaX * gGeometryEdgeDeltaX + gGeometryEdgeDeltaZOrCorrection * gGeometryEdgeDeltaZOrCorrection);
        nx = (gGeometryEdgeDeltaZOrCorrection << 12) / length;
        nz = (gGeometryEdgeDeltaX << 12) / length;
        tangent = (nz * gGeometryProjectionOffsetX + nx * gGeometryProjectionOffsetZOrDistance) / 4096;
        normal = (-nx * gGeometryProjectionOffsetX + nz * gGeometryProjectionOffsetZOrDistance) / 4096;
        edgeLength = (nz * gGeometryEdgeDeltaX + nx * gGeometryEdgeDeltaZOrCorrection) / 4096;
        if ((gCourseGeometryRegionData[index].cornerFlags & 4) && tangent < 0) {
            tangent = gGeometryResolvedX - x2;
            normal = gGeometryResolvedZ - z2;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x2;
                gGeometryResolvedZ = normal + z2;
            }
        } else if ((gCourseGeometryRegionData[index].cornerFlags & 1) && edgeLength < tangent) {
            tangent = gGeometryResolvedX - x0;
            normal = gGeometryResolvedZ - z0;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                gGeometryResolvedX = tangent + x0;
                gGeometryResolvedZ = normal + z0;
            }
        } else if (normal < radius) {
            gGeometryEdgeDeltaZOrCorrection = radius - normal;
            gGeometryResolvedX += -nx * gGeometryEdgeDeltaZOrCorrection / 4096;
            gGeometryResolvedZ += nz * gGeometryEdgeDeltaZOrCorrection / 4096;
        }
    }

    *displacementX = gGeometryResolvedX - x;
    *displacementZ = gGeometryResolvedZ - z;
}
