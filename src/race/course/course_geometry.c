#include "../../graphics/render_effects_internal.h"

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

INCLUDE_ASM("asm/nonmatchings/race/course/course_geometry", func_8004A764);

INCLUDE_ASM("asm/nonmatchings/race/course/course_geometry", func_8004AAA0);

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

INCLUDE_ASM("asm/nonmatchings/race/course/course_geometry", func_8004B044);

INCLUDE_ASM("asm/nonmatchings/race/course/course_geometry", func_8004BC3C);

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
