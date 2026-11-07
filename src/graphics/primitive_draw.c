#include "render_effects_internal.h"

void addCopiedPrimitive(u32 *orderingTable, u32 *packet) {
    u32 length = ((P_TAG *)packet)->len;
    u32 *out = (u32 *)GsOUT_PACKET_P;
    u8 remaining = length;

    if (length != 0xFF) {
        do {
            *out++ = *packet++;
        } while (--remaining != 0xFF);
    }
    {
        u32 *start = (u32 *)GsOUT_PACKET_P;

        GsOUT_PACKET_P = (u32)out;
        addPrim(orderingTable, start);
    }
}

void addFlatTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 x2, s32 y2, s32 r, s32 g, s32 b) {
    POLY_F3 *triangle = (POLY_F3 *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(POLY_F3) / sizeof(u32) - 1) << 24;
    setcode(triangle, 0x20);
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r, g, b);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addGouraudTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 r0, s32 g0, s32 b0,
                  s32 r1, s32 g1, s32 b1, s32 r2, s32 g2, s32 b2) {
    POLY_G3 *triangle = (POLY_G3 *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    setcode(triangle, POLY_G3_CODE);
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r0, g0, b0);
    setRGB1(triangle, r1, g1, b1);
    setRGB2(triangle, r2, g2, b2);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addTexturedTriangle(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                   s32 x2, s32 y2, u8 tpage, s32 clut,
                   s32 u0, s32 v0, s32 u1, s32 v1, s32 u2, s32 v2,
                   s32 r, s32 g, s32 b) {
    POLY_FT3 *triangle = (POLY_FT3 *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    setcode(triangle, POLY_FT3_CODE);
    triangle->tpage = tpage;
    triangle->clut = clut;
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    triangle->u0 = u0;
    triangle->v0 = v0;
    triangle->u1 = u1;
    triangle->v1 = v1;
    triangle->u2 = u2;
    triangle->v2 = v2;
    setRGB0(triangle, r, g, b);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

INCLUDE_ASM("asm/nonmatchings/graphics/primitive_draw", func_8005FE7C);

void addSemiTransparentTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                   s32 x1, s32 y1, s32 x2, s32 y2, s32 r, s32 g, s32 b) {
    SemiTransparentTrianglePacket *triangle = (SemiTransparentTrianglePacket *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    triangle->code = 0x22;
    triangle->tpage = tpage & 0xFF;
    triangle->drawModeCode = 0xE100;
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r, g, b);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addSemiTransparentGouraudTriangle(u32 *orderingTable, s32 tpage, s32 x0, s32 y0,
                  s32 x1, s32 y1, s32 x2, s32 y2,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2) {
    SemiTransparentGouraudTrianglePacket *triangle = (SemiTransparentGouraudTrianglePacket *)GsOUT_PACKET_P;
    triangle->tag = (sizeof(*triangle) / sizeof(u32) - 1) << 24;
    triangle->code = POLY_G3_CODE | SEMI_TRANSPARENT_FLAG;
    triangle->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    triangle->tpage = tpage & 0xFF;
    setXY3(triangle, x0, y0, x1, y1, x2, y2);
    setRGB0(triangle, r0, g0, b0);
    setRGB1(triangle, r1, g1, b1);
    setRGB2(triangle, r2, g2, b2);
    addPrim(orderingTable, triangle);
    GsOUT_PACKET_P = (u32)(triangle + 1);
}

void addFlatQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 x2, s32 y2, s32 x3, s32 y3, s32 r, s32 g, s32 b) {
    POLY_F4 *poly = (POLY_F4 *)GsOUT_PACKET_P;

    poly->tag = (sizeof(POLY_F4) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_F4_CODE);
    setXY2(poly, x0, y0, x1, y1);
    poly->x2 = x2;
    poly->y2 = y2;
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r, g, b);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addGouraudQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3) {
    POLY_G4 *poly = (POLY_G4 *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_G4_CODE);
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r0, g0, b0);
    setRGB1(poly, r1, g1, b1);
    setRGB2(poly, r2, g2, b2);
    setRGB3(poly, r3, g3, b3);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addTexturedQuad(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3, s32 flags, u8 tpage, s32 clut,
                  s32 u0, s32 v0, s32 u1, s32 v1, s32 u2, s32 v2,
                  s32 u3, s32 v3, s32 r, s32 g, s32 b) {
    POLY_FT4 *poly = (POLY_FT4 *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_FT4_CODE | flags);
    poly->tpage = tpage;
    poly->clut = clut;
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->x3 = x3;
    poly->y3 = y3;
    poly->u0 = u0;
    poly->v0 = v0;
    poly->u1 = u1;
    poly->v1 = v1;
    poly->u2 = u2;
    poly->v2 = v2;
    poly->u3 = u3;
    poly->v3 = v3;
    setRGB0(poly, r, g, b);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addGouraudTexturedQuad(u32 *orderingTable, s16 x0, s16 y0, s16 x1, s16 y1,
                  s16 x2, s16 y2, s16 x3, s16 y3, u8 tpage, s16 clut,
                  u8 u0, u8 v0, u8 u1, u8 v1, u8 u2, u8 v2,
                  u8 u3, u8 v3, u8 r0, u8 g0, u8 b0,
                  u8 r1, u8 g1, u8 b1, u8 r2, u8 g2, u8 b2,
                  u8 r3, u8 g3, u8 b3) {
    POLY_GT4 *poly = (POLY_GT4 *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    setcode(poly, POLY_GT4_CODE);
    poly->tpage = tpage;
    poly->clut = clut;
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->pad3 = 0;
    poly->x3 = x3;
    poly->y3 = y3;
    poly->pad5 = 0;
    poly->u0 = u0; poly->v0 = v0;
    poly->u1 = u1; poly->v1 = v1;
    poly->u2 = u2; poly->v2 = v2;
    poly->u3 = u3; poly->v3 = v3;
    setRGB0(poly, r0, g0, b0);
    setRGB1(poly, r1, g1, b1);
    poly->pad1 = 0;
    setRGB2(poly, r2, g2, b2);
    poly->pad2 = 0;
    setRGB3(poly, r3, g3, b3);
    poly->pad4 = 0;
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addSemiTransparentFlatQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                                          s32 x0, s32 y0, s32 x1, s32 y1,
                                          s32 x2, s32 y2, s32 x3, s32 y3,
                                          s32 r, s32 g, s32 b) {
    FlatQuadPacket *poly = (FlatQuadPacket *)GsOUT_PACKET_P;
    poly->tag = (sizeof(FlatQuadPacket) / sizeof(u32) - 1) << 24;
    poly->code = POLY_F4_CODE | SEMI_TRANSPARENT_FLAG;
    poly->tpage = tpage & 0xFF;
    poly->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    setXY2(poly, x0, y0, x1, y1);
    poly->x2 = x2;
    poly->y2 = y2;
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r, g, b);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addSemiTransparentGouraudQuadWithDrawMode(u32 *orderingTable, s32 tpage,
                  s32 x0, s32 y0, s32 x1, s32 y1,
                  s32 x2, s32 y2, s32 x3, s32 y3,
                  s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1,
                  s32 r2, s32 g2, s32 b2, s32 r3, s32 g3, s32 b3) {
    GouraudQuadPacket *poly = (GouraudQuadPacket *)GsOUT_PACKET_P;
    poly->tag = (sizeof(*poly) / sizeof(u32) - 1) << 24;
    poly->code = POLY_G4_CODE | SEMI_TRANSPARENT_FLAG;
    poly->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    poly->tpage = tpage & 0xFF;
    setXY3(poly, x0, y0, x1, y1, x2, y2);
    poly->x3 = x3;
    poly->y3 = y3;
    setRGB0(poly, r0, g0, b0);
    setRGB1(poly, r1, g1, b1);
    setRGB2(poly, r2, g2, b2);
    setRGB3(poly, r3, g3, b3);
    addPrim(orderingTable, poly);
    GsOUT_PACKET_P = (u32)(poly + 1);
}

void addFlatLine(u32 *orderingTable, s32 x0, s32 y0, s32 x1, s32 y1,
                 s32 r, s32 g, s32 b) {
    LINE_F2 *line = (LINE_F2 *)GsOUT_PACKET_P;

    line->tag = (sizeof(LINE_F2) / sizeof(u32) - 1) << 24;
    setcode(line, 0x40);
    setXY2(line, x0, y0, x1, y1);
    setRGB0(line, r, g, b);
    addPrim(orderingTable, line);
    GsOUT_PACKET_P = (u32)(line + 1);
}

INCLUDE_ASM("asm/nonmatchings/graphics/primitive_draw", func_80060A04);

void addFlatRectangle(u32 *orderingTable, s32 x, s32 y, s32 w, s32 h,
                   s32 r, s32 g, s32 b) {
    TILE *tile = (TILE *)GsOUT_PACKET_P;
    tile->tag = (sizeof(TILE) / sizeof(u32) - 1) << 24;
    setcode(tile, 0x60);
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    addPrim(orderingTable, tile);
    GsOUT_PACKET_P = (u32)(tile + 1);
}

void addFlatRectangleWithDrawMode(u32 *orderingTable, s32 tpage, s32 flags,
                  s32 x, s32 y, s32 w, s32 h, s32 r, s32 g, s32 b) {
    FlatRectanglePacket *tile = (FlatRectanglePacket *)GsOUT_PACKET_P;
    tile->tag = (sizeof(FlatRectanglePacket) / sizeof(u32) - 1) << 24;
    tile->code = FLAT_RECTANGLE_CODE | flags;
    tile->tpage = tpage;
    tile->drawModeCommand = DRAW_MODE_COMMAND_HIGH;
    setXY0(tile, x, y);
    setWH(tile, w, h);
    setRGB0(tile, r, g, b);
    addPrim(orderingTable, tile);
    GsOUT_PACKET_P = (u32)(tile + 1);
}

void addTexturedRectangle(u32 *orderingTable, s32 clut, s32 x, s32 y,
                   s32 w, s32 h, s32 r, s32 g, s32 b) {
    SPRT *sprite = (SPRT *)GsOUT_PACKET_P;
    sprite->tag = (sizeof(SPRT) / sizeof(u32) - 1) << 24;
    setcode(sprite, 0x64);
    sprite->clut = clut;
    setXY0(sprite, x, y);
    setWH(sprite, w, h);
    setRGB0(sprite, r, g, b);
    addPrim(orderingTable, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

void addTexturedSprite(u32 *orderingTable, s32 tpage, s32 clut, s32 x,
                   s32 y, s32 w, s32 h, u16 u, u16 v,
                   s32 r, s32 g, s32 b) {
    DrawModeSpritePacket *sprite = (DrawModeSpritePacket *)GsOUT_PACKET_P;
    sprite->tag = (sizeof(*sprite) / sizeof(u32) - 1) << 24;
    sprite->code = 0x64;
    sprite->tpage = tpage;
    sprite->drawModeCode = 0xE100;
    sprite->x0 = x;
    sprite->clut = clut;
    sprite->y0 = y;
    setWH(sprite, w, h);
    sprite->u0 = u;
    sprite->v0 = v;
    setRGB0(sprite, r, g, b);
    addPrim(orderingTable, sprite);
    GsOUT_PACKET_P = (u32)(sprite + 1);
}

/* No callers are known; only the variadic argument layout is established. */
