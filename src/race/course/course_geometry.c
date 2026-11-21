#include "../../graphics/render_effects_internal.h"
#include "game/graphics/coordinate.h"
#include "game/race/course/course_geometry.h"
#include "psx/libgte.h"

typedef struct CourseGeometryVertex {
    s16 x, y, z;
} CourseGeometryVertex;

s32 D_8013B624 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

s32 D_8013B628 __attribute__((section(".sdata.geometry_edge_projection"))) = 0;

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
    D_8013B1EC = data;
    /* Section lengths wrap as signed 16-bit halfword counts. */
    data += (s16)(count * 3);
    D_8013B1F8 = count;
    count = *data++;
    D_8013B1F0 = (SelectedRecord *)data;
    data += (s16)(count * 6);
    D_8013B1FA = count;
    D_8013B1FC = *data++;
    D_8013B1F4 = data;
}

s32 getAdjacentGeometryRegionForPoint(s16 index, s32 x, s32 z) {
    s32 x0, x1, x2, x3;
    s32 z0, z1, z2, z3;
    s16 neighbor;

    x0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3] * 5;
    x1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3] * 5;
    x2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3] * 5;
    x3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3] * 5;
    z0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3 + 2] * 5;
    z1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3 + 2] * 5;
    z2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3 + 2] * 5;
    z3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3 + 2] * 5;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[0];
    if (neighbor >= 0 && (x0 - x1) * (z - z1) - (z0 - z1) * (x - x1) > 0) return neighbor;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[1];
    if (neighbor >= 0 && (x3 - x2) * (z - z2) - (z3 - z2) * (x - x2) > 0) return neighbor;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[3];
    if (neighbor >= 0 && (x2 - x0) * (z - z0) - (z2 - z0) * (x - x0) > 0) return neighbor;
    neighbor = ((GeometryRegion *)D_8013B1F4)[index].neighbors[2];
    if (neighbor >= 0 && (x1 - x3) * (z - z3) - (z1 - z3) * (x - x3) > 0) return neighbor;
    return index;
}

void selectGeometryRecord(s16 index) {
    s16 *vertices;
    D_8013B1FE = index;
    vertices = D_8013B1EC;
    D_8013B200 = &vertices[D_8013B1F0[index].vertexIndices[0] * 3];
    D_8013B204 = &vertices[D_8013B1F0[index].vertexIndices[1] * 3];
    D_8013B208 = &vertices[D_8013B1F0[index].vertexIndices[2] * 3];
}

/* DECOMP_ATTEMPT: inline 114/207 */
#ifdef NON_MATCHING
typedef struct Region { s16 unk00[10]; s16 first, last; s16 unk18[2]; } Region;

typedef struct Vertex { s16 x,y,z; } Vertex;

typedef struct Triangle { u16 vertices[3]; u8 unk6, skipFirstEdge; s16 unk8,unkA; } Triangle;

s32 func_8004A764(s16 index, s32 x, s32 z) {
    s16 i;
    Triangle *triangle;
    Vertex *a, *b, *c;
    s32 dyC, dzB, dzC, dyB, dxB, dxC;
    for (i = ((Region *)D_8013B1F4)[index].first; i < ((Region *)D_8013B1F4)[index].last + 1; i++) {
        triangle = &((Triangle *)D_8013B1F0)[i];
        if (!triangle->skipFirstEdge) {
            b = &((Vertex *)D_8013B1EC)[triangle->vertices[1]];
            a = &((Vertex *)D_8013B1EC)[triangle->vertices[0]];
            if ((b->x-a->x)*(z/5-a->z) - (b->z-a->z)*(x/5-a->x) > 0) continue;
        }
        c = &((Vertex *)D_8013B1EC)[triangle->vertices[2]];
        b = &((Vertex *)D_8013B1EC)[triangle->vertices[1]];
        if ((c->x-b->x)*(z/5-b->z) - (c->z-b->z)*(x/5-b->x) > 0) continue;
        selectGeometryRecord(i);
        c = &((Vertex *)D_8013B1EC)[((Triangle *)D_8013B1F0)[i].vertices[2]];
        a = &((Vertex *)D_8013B1EC)[((Triangle *)D_8013B1F0)[i].vertices[0]];
        b = &((Vertex *)D_8013B1EC)[((Triangle *)D_8013B1F0)[i].vertices[1]];
        dyC = c->y-a->y;
        dzB = a->z-b->z;
        dzC = a->z-c->z;
        dyB = b->y-a->y;
        dxB = b->x-a->x;
        dxC = c->x-a->x;
        return -(((dxC*dyB-dyC*dxB)*(z-a->z*5) - (dyC*dzB-dzC*dyB)*(x-a->x*5))/(dzC*dxB-dxC*dzB)+a->y*5);
    }
    return 0x800000;
}
#else
INCLUDE_ASM("asm/nonmatchings/race/course/course_geometry", func_8004A764);
#endif

/* DECOMP_ATTEMPT: inline 68/102 */
#ifdef NON_MATCHING
typedef struct Placement { s16 unk00[8]; s16 vertexIndex; u16 angle; s16 first, last; s16 unk18[2]; } Placement;

typedef struct SurfaceRecord { u16 vertices[3]; u8 surface, flags; s16 unk8, unkA; } SurfaceRecord;

u8 func_8004AAA0(s16 index, s32 x, s32 z) {
    s32 i;
    SurfaceRecord *record;
    s16 *a, *b;
    s32 dx, dz, px, pz;
    s16 *vertices;
    SurfaceRecord *records;
    u32 ai, bi;
    for (i = ((Placement *)D_8013B1F4)[index].first; i < ((Placement *)D_8013B1F4)[index].last + 1; i++) {
        vertices = D_8013B1EC;
        records = (SurfaceRecord *)D_8013B1F0;
        record = &records[i];
        if (record->flags == 0) {
            bi = record->vertices[1];
            ai = record->vertices[0];
            b = vertices + bi * 3;
            a = vertices + ai * 3;
            dx = b[0] - a[0];
        pz = z / 5 - a[2];
        dz = b[2] - a[2];
        px = x / 5 - a[0];
        if (dx * pz - dz * px > 0) continue;
        }
        bi = record->vertices[2];
        ai = record->vertices[1];
        b = vertices + bi * 3;
        a = vertices + ai * 3;
        dx = b[0] - a[0];
        pz = z / 5 - a[2];
        dz = b[2] - a[2];
        px = x / 5 - a[0];
        if (dx * pz - dz * px > 0) continue;
        return record->surface;
    }
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/race/course/course_geometry", func_8004AAA0);
#endif

s32 getGeometryPlacementSignedDistance(s16 index, s32 x, s32 z) {
    s32 vertexIndex;
    s32 dx, dz;
    s32 sine, cosine;
    vertexIndex = ((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3;
    dx = x - D_8013B1EC[vertexIndex] * 5;
    dz = z - D_8013B1EC[vertexIndex + 2] * 5;
    sine = rsin((s16)((GeometryPlacement *)D_8013B1F4)[index].angle);
    cosine = rcos((s16)((GeometryPlacement *)D_8013B1F4)[index].angle);
    return (-sine * dx + cosine * -dz) / 4096;
}

void getGeometryPlacementTransform(s16 index, s32 *x, s32 *y, s32 *z, u16 *angle) {
    *x = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3] * 1280;
    *y = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3 + 1] * 1280;
    *z = D_8013B1EC[((GeometryPlacement *)D_8013B1F4)[index].vertexIndex * 3 + 2] * 1280;
    *angle = ((GeometryPlacement *)D_8013B1F4)[index].angle;
}

void reflectVelocityAndEnforceGeometryEdgeDistance(s32 *velocityX, s32 *velocityZ, s32 limit) {
    s32 length, nx, nz, perpendicularX;
    s32 tangent, normal;
    length = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
    nx = (D_8013B644 << 12) / length;
    nz = (D_8013B63C << 12) / length;
    D_8013B62C = length;
    D_8013B624 = nx;
    D_8013B628 = nz;
    perpendicularX = -nx;
    D_8013B638 = (perpendicularX * D_8013B630 + nz * D_8013B638) / 4096;
    if (D_8013B638 < limit) {
        tangent = (nz * *velocityX + nx * *velocityZ) / 4096;
        normal = (perpendicularX * *velocityX + nz * *velocityZ) / 4096;
        if (normal < 0) {
            normal = -normal;
            *velocityX = (nz * tangent - nx * normal) / 4096;
            *velocityZ = (D_8013B624 * tangent + D_8013B628 * normal) / 4096;
        }
        D_8013B644 = limit - D_8013B638;
        D_8013B648 += -D_8013B624 * D_8013B644 / 4096;
        D_8013B650 += D_8013B628 * D_8013B644 / 4096;
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
    x0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3] * 5;
    x1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3] * 5;
    x2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3] * 5;
    x3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3] * 5;
    z0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3 + 2] * 5;
    z1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3 + 2] * 5;
    z2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3 + 2] * 5;
    z3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3 + 2] * 5;

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[0] == wallType) {
        D_8013B63C = x1 - x0;
        D_8013B644 = z1 - z0;
        D_8013B630 = x - x0;
        D_8013B638 = z - z0;
        reflectVelocityAndEnforceGeometryEdgeDistance(velocityX, velocityZ, radius);
    }

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[1] == wallType) {
        D_8013B63C = x2 - x3;
        D_8013B644 = z2 - z3;
        D_8013B630 = D_8013B648 - x3;
        D_8013B638 = D_8013B650 - z3;
        reflectVelocityAndEnforceGeometryEdgeDistance(velocityX, velocityZ, radius);
    }

    /* Flagged side-edge corners use radial distance beyond the endpoints. */
    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[2] == wallType) {
        D_8013B63C = x3 - x1;
        D_8013B644 = z3 - z1;
        D_8013B630 = D_8013B648 - x1;
        D_8013B638 = D_8013B650 - z1;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        nx = (D_8013B644 << 12) / D_8013B62C;
        nz = (D_8013B63C << 12) / D_8013B62C;
        D_8013B624 = nx;
        D_8013B628 = nz;
        tangent = (nz * D_8013B630 + nx * D_8013B638) / 4096;
        normal = (-nx * D_8013B630 + nz * D_8013B638) / 4096;
        edgeLength = (nz * D_8013B63C + nx * D_8013B644) / 4096;
        if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 2) && tangent < 0) {
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
        } else if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 8) && edgeLength < tangent) {
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
            velocityTangent = (D_8013B628 * *velocityX + D_8013B624 * *velocityZ) / 4096;
            velocityNormal = (-D_8013B624 * *velocityX + D_8013B628 * *velocityZ) / 4096;
            if (velocityNormal < 0) {
                velocityNormal = -velocityNormal;
                *velocityX = (D_8013B628 * velocityTangent - D_8013B624 * velocityNormal) / 4096;
                *velocityZ = (D_8013B624 * velocityTangent + D_8013B628 * velocityNormal) / 4096;
            }
            D_8013B644 = radius - normal;
            D_8013B648 += -D_8013B624 * D_8013B644 / 4096;
            D_8013B650 += D_8013B628 * D_8013B644 / 4096;
        }
    }

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[3] == wallType) {
        D_8013B63C = x0 - x2;
        D_8013B644 = z0 - z2;
        D_8013B630 = D_8013B648 - x2;
        D_8013B638 = D_8013B650 - z2;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        nx = (D_8013B644 << 12) / D_8013B62C;
        nz = (D_8013B63C << 12) / D_8013B62C;
        D_8013B624 = nx;
        D_8013B628 = nz;
        tangent = (nz * D_8013B630 + nx * D_8013B638) / 4096;
        normal = (-nx * D_8013B630 + nz * D_8013B638) / 4096;
        edgeLength = (nz * D_8013B63C + nx * D_8013B644) / 4096;
        if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 4) && tangent < 0) {
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
        } else if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 1) && edgeLength < tangent) {
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
            velocityTangent = (D_8013B628 * *velocityX + D_8013B624 * *velocityZ) / 4096;
            velocityNormal = (-D_8013B624 * *velocityX + D_8013B628 * *velocityZ) / 4096;
            if (velocityNormal < 0) {
                velocityNormal = -velocityNormal;
                *velocityX = (D_8013B628 * velocityTangent - D_8013B624 * velocityNormal) / 4096;
                *velocityZ = (D_8013B624 * velocityTangent + D_8013B628 * velocityNormal) / 4096;
            }
            D_8013B644 = radius - normal;
            D_8013B648 += -D_8013B624 * D_8013B644 / 4096;
            D_8013B650 += D_8013B628 * D_8013B644 / 4096;
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
    CourseGeometryVertex *vertices = (CourseGeometryVertex *)D_8013B1EC;

    D_8013B648 = x;
    D_8013B650 = z;
    x0 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[0]].x * 5;
    x1 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[1]].x * 5;
    x2 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[2]].x * 5;
    x3 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[3]].x * 5;
    z0 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[0]].z * 5;
    z1 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[1]].z * 5;
    z2 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[2]].z * 5;
    z3 = vertices[((GeometryRegion *)D_8013B1F4)[index].vertices[3]].z * 5;

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[0] < 0) {
        D_8013B63C = x1 - x0;
        D_8013B644 = z1 - z0;
        D_8013B630 = x - x0;
        D_8013B638 = z - z0;
        reflectVelocityAndEnforceGeometryEdgeDistance(directionX, directionZ, radius);
    }
    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[1] < 0) {
        D_8013B63C = x2 - x3;
        D_8013B644 = z2 - z3;
        D_8013B630 = D_8013B648 - x3;
        D_8013B638 = D_8013B650 - z3;
        reflectVelocityAndEnforceGeometryEdgeDistance(directionX, directionZ, radius);
    }

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[2] < 0) {
        D_8013B63C = x3 - x1;
        D_8013B644 = z3 - z1;
        D_8013B630 = D_8013B648 - x1;
        D_8013B638 = D_8013B650 - z1;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        basisX = (D_8013B644 << 12) / D_8013B62C;
        basisZ = (D_8013B63C << 12) / D_8013B62C;
        D_8013B624 = basisX;
        D_8013B628 = basisZ;
        along = (basisZ * D_8013B630 + basisX * D_8013B638) / 4096;
        distance = (-basisX * D_8013B630 + basisZ * D_8013B638) / 4096;
        edgeLength = (basisZ * D_8013B63C + basisX * D_8013B644) / 4096;
        if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & (1 << 1)) && along < 0) {
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
        } else if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & (1 << 3)) && edgeLength < along) {
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
            tangent = (D_8013B628 * *directionX + D_8013B624 * *directionZ) / 4096;
            normal = (-D_8013B624 * *directionX + D_8013B628 * *directionZ) / 4096;
            if (normal < 0) {
                normal = -normal;
                *directionX = (D_8013B628 * tangent - D_8013B624 * normal) / 4096;
                *directionZ = (D_8013B624 * tangent + D_8013B628 * normal) / 4096;
            }
            D_8013B644 = radius - distance;
            D_8013B648 += -D_8013B624 * D_8013B644 / 4096;
            D_8013B650 += D_8013B628 * D_8013B644 / 4096;
        }
    }

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[3] < 0) {
        D_8013B63C = x0 - x2;
        D_8013B644 = z0 - z2;
        D_8013B630 = D_8013B648 - x2;
        D_8013B638 = D_8013B650 - z2;
        D_8013B62C = SquareRoot0(D_8013B63C * D_8013B63C + D_8013B644 * D_8013B644);
        basisX = (D_8013B644 << 12) / D_8013B62C;
        basisZ = (D_8013B63C << 12) / D_8013B62C;
        D_8013B624 = basisX;
        D_8013B628 = basisZ;
        along = (basisZ * D_8013B630 + basisX * D_8013B638) / 4096;
        distance = (-basisX * D_8013B630 + basisZ * D_8013B638) / 4096;
        edgeLength = (basisZ * D_8013B63C + basisX * D_8013B644) / 4096;
        if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & (1 << 2)) && along < 0) {
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
        } else if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & (1 << 0)) && edgeLength < along) {
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
            tangent = (D_8013B628 * *directionX + D_8013B624 * *directionZ) / 4096;
            normal = (-D_8013B624 * *directionX + D_8013B628 * *directionZ) / 4096;
            if (normal < 0) {
                normal = -normal;
                *directionX = (D_8013B628 * tangent - D_8013B624 * normal) / 4096;
                *directionZ = (D_8013B624 * tangent + D_8013B628 * normal) / 4096;
            }
            D_8013B644 = radius - distance;
            D_8013B648 += -D_8013B624 * D_8013B644 / 4096;
            D_8013B650 += D_8013B628 * D_8013B644 / 4096;
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
    x0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3] * 5;
    x1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3] * 5;
    x2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3] * 5;
    x3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3] * 5;
    z0 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[0] * 3 + 2] * 5;
    z1 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[1] * 3 + 2] * 5;
    z2 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[2] * 3 + 2] * 5;
    z3 = D_8013B1EC[((GeometryRegion *)D_8013B1F4)[index].vertices[3] * 3 + 2] * 5;

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[0] < 0) {
        D_8013B63C = x1 - x0;
        D_8013B644 = z1 - z0;
        D_8013B630 = x - x0;
        D_8013B638 = z - z0;
        enforceGeometryEdgeDistance(radius);
    }

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[1] < 0) {
        D_8013B63C = x2 - x3;
        D_8013B644 = z2 - z3;
        D_8013B630 = D_8013B648 - x3;
        D_8013B638 = D_8013B650 - z3;
        enforceGeometryEdgeDistance(radius);
    }

    /* Flagged side-edge corners use radial distance beyond the endpoints. */
    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[2] < 0) {
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
        if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 2) && tangent < 0) {
            tangent = D_8013B648 - x1;
            normal = D_8013B650 - z1;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x1;
                D_8013B650 = normal + z1;
            }
        } else if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 8) && edgeLength < tangent) {
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

    if (((GeometryRegion *)D_8013B1F4)[index].neighbors[3] < 0) {
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
        if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 4) && tangent < 0) {
            tangent = D_8013B648 - x2;
            normal = D_8013B650 - z2;
            length = SquareRoot0(tangent * tangent + normal * normal);
            if (length < -radius) {
                tangent = tangent * -radius / length;
                normal = normal * -radius / length;
                D_8013B648 = tangent + x2;
                D_8013B650 = normal + z2;
            }
        } else if ((((GeometryRegion *)D_8013B1F4)[index].cornerFlags & 1) && edgeLength < tangent) {
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
