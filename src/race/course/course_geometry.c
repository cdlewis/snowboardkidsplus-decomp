#include "../../graphics/render_effects_internal.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_geometry.h"
#include "psx/libgte.h"

s32 gGeometryProjectionBasisX __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 gGeometryProjectionBasisZ __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B62C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B630 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B634 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B638 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B63C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B640 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B644 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B648 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B64C __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B650 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

void bindGeometryRecordData(s16 *data) {
    s16 count;

    count = *data++;
    gCourseGeometryVertexData = data;
    /* Section lengths wrap as signed 16-bit halfword counts. */
    data += (s16)(count * 3);
    D_8013B1F8 = count;
    count = *data++;
    D_8013B1F0 = (SelectedRecord *)data;
    data += (s16)(count * 6);
    D_8013B1FA = count;
    D_8013B1FC = *data++;
    gCourseGeometryRegionData = data;
}

s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s16 neighbor;

    x0 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0] * 3] * 5;
    x1 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1] * 3] * 5;
    x2 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2] * 3] * 5;
    x3 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3] * 3] * 5;
    z0 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0] * 3 + 2] * 5;
    z1 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1] * 3 + 2] * 5;
    z2 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2] * 3 + 2] * 5;
    z3 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3] * 3 + 2] * 5;
    neighbor = ((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[0];
    if (neighbor >= 0 && (x0 - x1) * (z - z1) - (z0 - z1) * (x - x1) > 0) return neighbor;
    neighbor = ((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[1];
    if (neighbor >= 0 && (x3 - x2) * (z - z2) - (z3 - z2) * (x - x2) > 0) return neighbor;
    neighbor = ((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[3];
    if (neighbor >= 0 && (x2 - x0) * (z - z0) - (z2 - z0) * (x - x0) > 0) return neighbor;
    neighbor = ((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[2];
    if (neighbor >= 0 && (x1 - x3) * (z - z3) - (z1 - z3) * (x - x3) > 0) return neighbor;
    return index;
}

void selectGeometryRecord(s16 index) {
    s16 *vertices;
    D_8013B1FE = index;
    vertices = gCourseGeometryVertexData;
    D_8013B200 = &vertices[D_8013B1F0[index].vertexIndices[0] * 3];
    D_8013B204 = &vertices[D_8013B1F0[index].vertexIndices[1] * 3];
    D_8013B208 = &vertices[D_8013B1F0[index].vertexIndices[2] * 3];
}

s32 getCourseHeightAtPoint(s16 index, s32 x, s32 z) {
    s16 i;
    CourseGeometryVertex *triangleVertices;

    for (i = ((GeometryRegion *)gCourseGeometryRegionData)[index].firstRecord;
         i < ((GeometryRegion *)gCourseGeometryRegionData)[index].lastRecord + 1; i++) {
        SelectedRecord *records = D_8013B1F0;

        if (records[i].skipFirstEdge == 0) {
            CourseGeometryVertex *vertices = (CourseGeometryVertex *)gCourseGeometryVertexData;
            s32 bi = records[i].vertexIndices[1];
            s32 ai = records[i].vertexIndices[0];

            if ((vertices[bi].x - vertices[ai].x) * (z / 5 - vertices[ai].z) -
                (vertices[bi].z - vertices[ai].z) * (x / 5 - vertices[ai].x) > 0) {
                continue;
            }
        }
        {
            CourseGeometryVertex *vertices = (CourseGeometryVertex *)gCourseGeometryVertexData;
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
            SelectedRecord *selected = D_8013B1F0;

            ci = selected[i].vertexIndices[2];
            triangleVertices = (CourseGeometryVertex *)gCourseGeometryVertexData;
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
    for (i = ((GeometryRegion *)gCourseGeometryRegionData)[index].firstRecord;
         i < ((GeometryRegion *)gCourseGeometryRegionData)[index].lastRecord + 1; i++) {
        vertices = (CourseGeometryVertex *)gCourseGeometryVertexData;
        records = D_8013B1F0;
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
    vertexIndex = ((GeometryPlacement *)gCourseGeometryRegionData)[index].vertexIndex * 3;
    dx = x - gCourseGeometryVertexData[vertexIndex] * 5;
    dz = z - gCourseGeometryVertexData[vertexIndex + 2] * 5;
    sine = rsin((s16)((GeometryPlacement *)gCourseGeometryRegionData)[index].angle);
    cosine = rcos((s16)((GeometryPlacement *)gCourseGeometryRegionData)[index].angle);
    return (-sine * dx + cosine * -dz) / 4096;
}

void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle) {
    *x = gCourseGeometryVertexData[((GeometryPlacement *)gCourseGeometryRegionData)[index].vertexIndex * 3] * 1280;
    *y = gCourseGeometryVertexData[((GeometryPlacement *)gCourseGeometryRegionData)[index].vertexIndex * 3 + 1] * 1280;
    *z = gCourseGeometryVertexData[((GeometryPlacement *)gCourseGeometryRegionData)[index].vertexIndex * 3 + 2] * 1280;
    *angle = ((GeometryPlacement *)gCourseGeometryRegionData)[index].angle;
}

void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit) {
    s32 length, nx, nz, perpendicularX;
    s32 tangent, normal;
    length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
    nx = (D_8013B644 << 12) / length;
    nz = (D_8013B63C << 12) / length;
    D_8013B62C = length;
    gGeometryProjectionBasisX = nx;
    gGeometryProjectionBasisZ = nz;
    perpendicularX = -nx;
    D_8013B638 = (perpendicularX * D_8013B630 + nz * D_8013B638) / 4096;
    if (D_8013B638 < limit) {
        tangent = (nz * *velocityX + nx * *velocityZ) / 4096;
        normal = (perpendicularX * *velocityX + nz * *velocityZ) / 4096;
        if (normal < 0) {
            normal = -normal;
            *velocityX = (nz * tangent - nx * normal) / 4096;
            *velocityZ = (gGeometryProjectionBasisX * tangent + gGeometryProjectionBasisZ * normal) / 4096;
        }
        D_8013B644 = limit - D_8013B638;
        D_8013B648 += -gGeometryProjectionBasisX * D_8013B644 / 4096;
        D_8013B650 += gGeometryProjectionBasisZ * D_8013B644 / 4096;
    }
}

void resolveGeometryBoundaryCollision(s16 index, s32 x, s32 z, s32 radius, s32 mode,
                                      s32 *displacementX, s32 *displacementZ, s32 *velocityX, s32 *velocityZ) {
    /* Boundary types are stored as negative neighbor indices. */
    s8 wallType = -mode;
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s32 nx, nz, tangent, normal, length, edgeLength;

    D_8013B648 = x;
    D_8013B650 = z;
    x0 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0] * 3] * 5;
    x1 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1] * 3] * 5;
    x2 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2] * 3] * 5;
    x3 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3] * 3] * 5;
    z0 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0] * 3 + 2] * 5;
    z1 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1] * 3 + 2] * 5;
    z2 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2] * 3 + 2] * 5;
    z3 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3] * 3 + 2] * 5;

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[0] == wallType) {
        D_8013B63C = x1 - x0;
        D_8013B644 = z1 - z0;
        D_8013B630 = x - x0;
        D_8013B638 = z - z0;
        reflectVelocityAndEnforceGeometryEdgeDistance(velocityX, velocityZ, radius);
    }

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[1] == wallType) {
        D_8013B63C = x2 - x3;
        D_8013B644 = z2 - z3;
        D_8013B630 = D_8013B648 - x3;
        D_8013B638 = D_8013B650 - z3;
        reflectVelocityAndEnforceGeometryEdgeDistance(velocityX, velocityZ, radius);
    }

    /* Flagged side-edge corners use radial distance beyond the endpoints. */
    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[2] == wallType) {
        D_8013B63C = x3 - x1;
        D_8013B644 = z3 - z1;
        D_8013B630 = D_8013B648 - x1;
        D_8013B638 = D_8013B650 - z1;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        nx = (D_8013B644 << 12) / D_8013B62C;
        nz = (D_8013B63C << 12) / D_8013B62C;
        gGeometryProjectionBasisX = nx;
        gGeometryProjectionBasisZ = nz;
        tangent = (nz * D_8013B630 + nx * D_8013B638) / 4096;
        normal = (-nx * D_8013B630 + nz * D_8013B638) / 4096;
        edgeLength = (nz * D_8013B63C + nx * D_8013B644) / 4096;
        if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 2) && tangent < 0) {
            tangent = D_8013B648 - x1;
            normal = D_8013B650 - z1;
            length = SquareRoot0(tangent * tangent + normal * normal);
            D_8013B62C = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x1;
                D_8013B650 = normal + z1;
            }
        } else if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 8) && edgeLength < tangent) {
            tangent = D_8013B648 - x3;
            normal = D_8013B650 - z3;
            length = SquareRoot0(tangent * tangent + normal * normal);
            D_8013B62C = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x3;
                D_8013B650 = normal + z3;
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
            D_8013B644 = radius - normal;
            D_8013B648 += -gGeometryProjectionBasisX * D_8013B644 / 4096;
            D_8013B650 += gGeometryProjectionBasisZ * D_8013B644 / 4096;
        }
    }

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[3] == wallType) {
        D_8013B63C = x0 - x2;
        D_8013B644 = z0 - z2;
        D_8013B630 = D_8013B648 - x2;
        D_8013B638 = D_8013B650 - z2;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        nx = (D_8013B644 << 12) / D_8013B62C;
        nz = (D_8013B63C << 12) / D_8013B62C;
        gGeometryProjectionBasisX = nx;
        gGeometryProjectionBasisZ = nz;
        tangent = (nz * D_8013B630 + nx * D_8013B638) / 4096;
        normal = (-nx * D_8013B630 + nz * D_8013B638) / 4096;
        edgeLength = (nz * D_8013B63C + nx * D_8013B644) / 4096;
        if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 4) && tangent < 0) {
            tangent = D_8013B648 - x2;
            normal = D_8013B650 - z2;
            length = SquareRoot0(tangent * tangent + normal * normal);
            D_8013B62C = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x2;
                D_8013B650 = normal + z2;
            }
        } else if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 1) && edgeLength < tangent) {
            tangent = D_8013B648 - x0;
            normal = D_8013B650 - z0;
            length = SquareRoot0(tangent * tangent + normal * normal);
            D_8013B62C = length;
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x0;
                D_8013B650 = normal + z0;
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
            D_8013B644 = radius - normal;
            D_8013B648 += -gGeometryProjectionBasisX * D_8013B644 / 4096;
            D_8013B650 += gGeometryProjectionBasisZ * D_8013B644 / 4096;
        }
    }

    *displacementX = D_8013B648 - x;
    *displacementZ = D_8013B650 - z;
}

void reflectVelocityAndResolveGeometryBoundary(s16 index, s32 x, s32 z, s32 radius,
                                               s32 *displacementX, s32 *displacementZ,
                                               s32 *directionX, s32 *directionZ) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s32 along, distance, edgeLength, basisX, basisZ;
    s32 cornerLength, tangent, normal;
    CourseGeometryVertex *vertices = (CourseGeometryVertex *)gCourseGeometryVertexData;

    D_8013B648 = x;
    D_8013B650 = z;
    x0 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0]].x * 5;
    x1 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1]].x * 5;
    x2 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2]].x * 5;
    x3 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3]].x * 5;
    z0 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0]].z * 5;
    z1 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1]].z * 5;
    z2 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2]].z * 5;
    z3 = vertices[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3]].z * 5;

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[0] < 0) {
        D_8013B63C = x1 - x0;
        D_8013B644 = z1 - z0;
        D_8013B630 = x - x0;
        D_8013B638 = z - z0;
        reflectVelocityAndEnforceGeometryEdgeDistance(directionX, directionZ, radius);
    }
    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[1] < 0) {
        D_8013B63C = x2 - x3;
        D_8013B644 = z2 - z3;
        D_8013B630 = D_8013B648 - x3;
        D_8013B638 = D_8013B650 - z3;
        reflectVelocityAndEnforceGeometryEdgeDistance(directionX, directionZ, radius);
    }

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[2] < 0) {
        D_8013B63C = x3 - x1;
        D_8013B644 = z3 - z1;
        D_8013B630 = D_8013B648 - x1;
        D_8013B638 = D_8013B650 - z1;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        basisX = (D_8013B644 << 12) / D_8013B62C;
        basisZ = (D_8013B63C << 12) / D_8013B62C;
        gGeometryProjectionBasisX = basisX;
        gGeometryProjectionBasisZ = basisZ;
        along = (basisZ * D_8013B630 + basisX * D_8013B638) / 4096;
        distance = (-basisX * D_8013B630 + basisZ * D_8013B638) / 4096;
        edgeLength = (basisZ * D_8013B63C + basisX * D_8013B644) / 4096;
        if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & (1 << 1)) && along < 0) {
            along = D_8013B648 - x1;
            distance = D_8013B650 - z1;
            cornerLength = SquareRoot0(along * along + distance * distance);
            D_8013B62C = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                D_8013B648 = along + x1;
                D_8013B650 = distance + z1;
            }
        } else if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & (1 << 3)) && edgeLength < along) {
            along = D_8013B648 - x3;
            distance = D_8013B650 - z3;
            cornerLength = SquareRoot0(along * along + distance * distance);
            D_8013B62C = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                D_8013B648 = along + x3;
                D_8013B650 = distance + z3;
            }
        } else if (distance < radius) {
            tangent = (gGeometryProjectionBasisZ * *directionX + gGeometryProjectionBasisX * *directionZ) / 4096;
            normal = (-gGeometryProjectionBasisX * *directionX + gGeometryProjectionBasisZ * *directionZ) / 4096;
            if (normal < 0) {
                normal = -normal;
                *directionX = (gGeometryProjectionBasisZ * tangent - gGeometryProjectionBasisX * normal) / 4096;
                *directionZ = (gGeometryProjectionBasisX * tangent + gGeometryProjectionBasisZ * normal) / 4096;
            }
            D_8013B644 = radius - distance;
            D_8013B648 += -gGeometryProjectionBasisX * D_8013B644 / 4096;
            D_8013B650 += gGeometryProjectionBasisZ * D_8013B644 / 4096;
        }
    }

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[3] < 0) {
        D_8013B63C = x0 - x2;
        D_8013B644 = z0 - z2;
        D_8013B630 = D_8013B648 - x2;
        D_8013B638 = D_8013B650 - z2;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        basisX = (D_8013B644 << 12) / D_8013B62C;
        basisZ = (D_8013B63C << 12) / D_8013B62C;
        gGeometryProjectionBasisX = basisX;
        gGeometryProjectionBasisZ = basisZ;
        along = (basisZ * D_8013B630 + basisX * D_8013B638) / 4096;
        distance = (-basisX * D_8013B630 + basisZ * D_8013B638) / 4096;
        edgeLength = (basisZ * D_8013B63C + basisX * D_8013B644) / 4096;
        if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & (1 << 2)) && along < 0) {
            along = D_8013B648 - x2;
            distance = D_8013B650 - z2;
            cornerLength = SquareRoot0(along * along + distance * distance);
            D_8013B62C = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                D_8013B648 = along + x2;
                D_8013B650 = distance + z2;
            }
        } else if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & (1 << 0)) && edgeLength < along) {
            along = D_8013B648 - x0;
            distance = D_8013B650 - z0;
            cornerLength = SquareRoot0(along * along + distance * distance);
            D_8013B62C = cornerLength;
            if (cornerLength < -radius) {
                along = along * -radius / cornerLength;
                distance = distance * -radius / cornerLength;
                D_8013B648 = along + x0;
                D_8013B650 = distance + z0;
            }
        } else if (distance < radius) {
            tangent = (gGeometryProjectionBasisZ * *directionX + gGeometryProjectionBasisX * *directionZ) / 4096;
            normal = (-gGeometryProjectionBasisX * *directionX + gGeometryProjectionBasisZ * *directionZ) / 4096;
            if (normal < 0) {
                normal = -normal;
                *directionX = (gGeometryProjectionBasisZ * tangent - gGeometryProjectionBasisX * normal) / 4096;
                *directionZ = (gGeometryProjectionBasisX * tangent + gGeometryProjectionBasisZ * normal) / 4096;
            }
            D_8013B644 = radius - distance;
            D_8013B648 += -gGeometryProjectionBasisX * D_8013B644 / 4096;
            D_8013B650 += gGeometryProjectionBasisZ * D_8013B644 / 4096;
        }
    }

    *displacementX = D_8013B648 - x;
    *displacementZ = D_8013B650 - z;
}

void enforceGeometryEdgeDistance(s32 limit) {
    s32 length, nx, nz, perpendicularX;
    length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
    nx = (D_8013B644 << 12) / length;
    nz = (D_8013B63C << 12) / length;
    perpendicularX = -nx;
    D_8013B638 = (perpendicularX * D_8013B630 + nz * D_8013B638) / 4096;
    if (D_8013B638 < limit) {
        D_8013B644 = limit - D_8013B638;
        D_8013B648 += perpendicularX * D_8013B644 / 4096;
        D_8013B650 += nz * D_8013B644 / 4096;
    }
}

void getGeometryBoundaryDisplacement(s16 index, s32 x, s32 z, s32 radius, s32 *displacementX, s32 *displacementZ) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s32 nx, nz, tangent, normal, length, edgeLength;

    D_8013B648 = x;
    D_8013B650 = z;
    x0 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0] * 3] * 5;
    x1 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1] * 3] * 5;
    x2 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2] * 3] * 5;
    x3 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3] * 3] * 5;
    z0 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[0] * 3 + 2] * 5;
    z1 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[1] * 3 + 2] * 5;
    z2 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[2] * 3 + 2] * 5;
    z3 = gCourseGeometryVertexData[((GeometryRegion *)gCourseGeometryRegionData)[index].vertices[3] * 3 + 2] * 5;

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[0] < 0) {
        D_8013B63C = x1 - x0;
        D_8013B644 = z1 - z0;
        D_8013B630 = x - x0;
        D_8013B638 = z - z0;
        enforceGeometryEdgeDistance(radius);
    }

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[1] < 0) {
        D_8013B63C = x2 - x3;
        D_8013B644 = z2 - z3;
        D_8013B630 = D_8013B648 - x3;
        D_8013B638 = D_8013B650 - z3;
        enforceGeometryEdgeDistance(radius);
    }

    /* Flagged side-edge corners use radial distance beyond the endpoints. */
    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[2] < 0) {
        D_8013B63C = x3 - x1;
        D_8013B644 = z3 - z1;
        D_8013B630 = D_8013B648 - x1;
        D_8013B638 = D_8013B650 - z1;
        length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        nx = (D_8013B644 << 12) / length;
        nz = (D_8013B63C << 12) / length;
        tangent = (nz * D_8013B630 + nx * D_8013B638) / 4096;
        normal = (-nx * D_8013B630 + nz * D_8013B638) / 4096;
        edgeLength = (nz * D_8013B63C + nx * D_8013B644) / 4096;
        if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 2) && tangent < 0) {
            tangent = D_8013B648 - x1;
            normal = D_8013B650 - z1;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x1;
                D_8013B650 = normal + z1;
            }
        } else if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 8) && edgeLength < tangent) {
            tangent = D_8013B648 - x3;
            normal = D_8013B650 - z3;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x3;
                D_8013B650 = normal + z3;
            }
        } else if (normal < radius) {
            D_8013B644 = radius - normal;
            D_8013B648 += -nx * D_8013B644 / 4096;
            D_8013B650 += nz * D_8013B644 / 4096;
        }
    }

    if (((GeometryRegion *)gCourseGeometryRegionData)[index].neighbors[3] < 0) {
        D_8013B63C = x0 - x2;
        D_8013B644 = z0 - z2;
        D_8013B630 = D_8013B648 - x2;
        D_8013B638 = D_8013B650 - z2;
        length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        nx = (D_8013B644 << 12) / length;
        nz = (D_8013B63C << 12) / length;
        tangent = (nz * D_8013B630 + nx * D_8013B638) / 4096;
        normal = (-nx * D_8013B630 + nz * D_8013B638) / 4096;
        edgeLength = (nz * D_8013B63C + nx * D_8013B644) / 4096;
        if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 4) && tangent < 0) {
            tangent = D_8013B648 - x2;
            normal = D_8013B650 - z2;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x2;
                D_8013B650 = normal + z2;
            }
        } else if ((((GeometryRegion *)gCourseGeometryRegionData)[index].cornerFlags & 1) && edgeLength < tangent) {
            tangent = D_8013B648 - x0;
            normal = D_8013B650 - z0;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x0;
                D_8013B650 = normal + z0;
            }
        } else if (normal < radius) {
            D_8013B644 = radius - normal;
            D_8013B648 += -nx * D_8013B644 / 4096;
            D_8013B650 += nz * D_8013B644 / 4096;
        }
    }

    *displacementX = D_8013B648 - x;
    *displacementZ = D_8013B650 - z;
}
