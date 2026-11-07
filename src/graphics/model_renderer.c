#include "render_effects_internal.h"

void emitProjectedFlatTriangle(TriangleScratchVertex *vertices) {
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 4 << 24;
    setcode(packet, scratch->code | POLY_F3_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudTriangle(TriangleScratchVertex *vertices) {
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, vertices[0].color.r, vertices[0].color.g, vertices[0].color.b);
        setRGB1(packet, vertices[1].color.r, vertices[1].color.g, vertices[1].color.b);
        setRGB2(packet, vertices[2].color.r, vertices[2].color.g, vertices[2].color.b);
    }
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedTexturedTriangle(TriangleScratchVertex *vertices) {
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 7 << 24;
    setcode(packet, scratch->code | POLY_FT3_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudTexturedTriangle(TriangleScratchVertex *vertices) {
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)0x1F800000;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        packet->r0 = vertices[0].color.r;
        packet->r1 = vertices[1].color.r;
        packet->r2 = vertices[2].color.r;
        packet->g0 = vertices[0].color.g;
        packet->g1 = vertices[1].color.g;
        packet->g2 = vertices[2].color.g;
        packet->b0 = vertices[0].color.b;
        packet->b1 = vertices[1].color.b;
        packet->b2 = vertices[2].color.b;
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedFlatQuad(TriangleScratchVertex *vertices) {
    QuadScratchpad *scratch = QUAD_SCRATCHPAD;
    POLY_F4 *packet = (POLY_F4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 5 << 24;
    setcode(packet, scratch->code | POLY_F4_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudQuad(TriangleScratchVertex *vertices) {
    GouraudQuadScratchpad *scratch = (GouraudQuadScratchpad *)0x1F800000;
    POLY_G4 *packet = (POLY_G4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&vertices[3].color, scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        packet->r0 = vertices[0].color.r;
        packet->g0 = vertices[0].color.g;
        packet->b0 = vertices[0].color.b;
        packet->r1 = vertices[1].color.r;
        packet->g1 = vertices[1].color.g;
        packet->b1 = vertices[1].color.b;
        packet->r2 = vertices[2].color.r;
        packet->g2 = vertices[2].color.g;
        packet->b2 = vertices[2].color.b;
        packet->r3 = vertices[3].color.r;
        packet->g3 = vertices[3].color.g;
        packet->b3 = vertices[3].color.b;
    }
    packet->tag = 8 << 24;
    setcode(packet, scratch->code | POLY_G4_CODE);
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedTexturedQuad(TriangleScratchVertex *vertices) {
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)0x1F800000;
    POLY_FT4 *packet = (POLY_FT4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&scratch->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, scratch->r, scratch->g, scratch->b);
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_FT4_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->u3 = vertices[3].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->v3 = vertices[3].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void emitProjectedGouraudTexturedQuad(TriangleScratchVertex *vertices) {
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)0x1F800000;
    POLY_GT4 *packet = (POLY_GT4 *)GsOUT_PACKET_P;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&vertices[0].color, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&vertices[1].color, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&vertices[2].color, scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&vertices[3].color, scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        packet->r0 = vertices[0].color.r;
        packet->r1 = vertices[1].color.r;
        packet->r2 = vertices[2].color.r;
        packet->r3 = vertices[3].color.r;
        packet->g0 = vertices[0].color.g;
        packet->g1 = vertices[1].color.g;
        packet->g2 = vertices[2].color.g;
        packet->g3 = vertices[3].color.g;
        packet->b0 = vertices[0].color.b;
        packet->b1 = vertices[1].color.b;
        packet->b2 = vertices[2].color.b;
        packet->b3 = vertices[3].color.b;
    }
    packet->tag = 12 << 24;
    setcode(packet, scratch->code | POLY_GT4_CODE);
    packet->tpage = scratch->tpage;
    packet->clut = scratch->clut;
    packet->u0 = vertices[0].texture.uv.u;
    packet->u1 = vertices[1].texture.uv.u;
    packet->u2 = vertices[2].texture.uv.u;
    packet->u3 = vertices[3].texture.uv.u;
    packet->v0 = vertices[0].texture.uv.v;
    packet->v1 = vertices[1].texture.uv.v;
    packet->v2 = vertices[2].texture.uv.v;
    packet->v3 = vertices[3].texture.uv.v;
    packet->x0 = scratch->screen[0].x;
    packet->x1 = scratch->screen[1].x;
    packet->x2 = scratch->screen[2].x;
    packet->x3 = scratch->screen[3].x;
    packet->y0 = scratch->screen[0].y;
    packet->y1 = scratch->screen[1].y;
    packet->y2 = scratch->screen[2].y;
    packet->y3 = scratch->screen[3].y;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80053408);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80053A34);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_800542BC);

void drawIndexedFlatTriangle(void) {
    ModelFlatTriangle *primitive = D_8013B680;
    TRIANGLE_SCRATCHPAD->emitTriangle = emitProjectedFlatTriangle;
    TRIANGLE_SCRATCHPAD->r = primitive->r;
    TRIANGLE_SCRATCHPAD->g = primitive->g;
    TRIANGLE_SCRATCHPAD->b = primitive->b;
    TRIANGLE_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    TRIANGLE_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    TRIANGLE_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    func_80053A34(TRIANGLE_SCRATCHPAD->vertices, 0);
}

void processGouraudTrianglePrimitive(void) {
    GouraudTrianglePrimitive *primitive = (GouraudTrianglePrimitive *)D_8013B680;
    *(void (**)(TriangleScratchVertex *))0x1F800004 = emitProjectedGouraudTriangle;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    func_80053A34((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedTrianglePrimitive(void) {
    TexturedTrianglePrimitive *primitive = (TexturedTrianglePrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->r = primitive->r;
    ((TriangleTextureState *)0x1F800000)->g = primitive->g;
    ((TriangleTextureState *)0x1F800000)->b = primitive->b;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedTexturedTriangle;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    func_80053A34((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedGouraudTrianglePrimitive(void) {
    TexturedGouraudTrianglePrimitive *primitive = (TexturedGouraudTrianglePrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedGouraudTexturedTriangle;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    func_80053A34((TriangleScratchVertex *)0x1F800070, 0);
}

void drawIndexedFlatQuad(void) {
    ModelFlatQuad *primitive = (ModelFlatQuad *)D_8013B680;
    QUAD_SCRATCHPAD->r = primitive->r;
    QUAD_SCRATCHPAD->g = primitive->g;
    QUAD_SCRATCHPAD->b = primitive->b;
    QUAD_SCRATCHPAD->emitQuad = emitProjectedFlatQuad;
    QUAD_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    QUAD_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    QUAD_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    QUAD_SCRATCHPAD->vertices[3].position = D_8013B688[primitive->vertexIndices[3]];
    func_800542BC(QUAD_SCRATCHPAD->vertices, 0);
}

void processGouraudQuadPrimitive(void) {
    GouraudQuadPrimitive *primitive = (GouraudQuadPrimitive *)D_8013B680;
    QUAD_SCRATCHPAD->emitQuad = emitProjectedGouraudQuad;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F8000A0)->position = D_8013B688[primitive->vertexIndices[3]];
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    ((TriangleScratchVertex *)0x1F8000A0)->color = primitive->colors[3];
    func_800542BC((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedQuadPrimitive(void) {
    TexturedQuadPrimitive *primitive = (TexturedQuadPrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->r = primitive->r;
    ((TriangleTextureState *)0x1F800000)->g = primitive->g;
    ((TriangleTextureState *)0x1F800000)->b = primitive->b;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedTexturedQuad;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F8000A0)->position = D_8013B688[primitive->vertexIndices[3]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    *(UVPair *)0x1F8000A8 = primitive->uv[3].coords;
    func_800542BC((TriangleScratchVertex *)0x1F800070, 0);
}

void processTexturedGouraudQuadPrimitive(void) {
    TexturedGouraudQuadPrimitive *primitive = (TexturedGouraudQuadPrimitive *)D_8013B680;
    ((TriangleTextureState *)0x1F800000)->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)0x1F800000)->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)0x1F800000)->callback = emitProjectedGouraudTexturedQuad;
    ((TriangleScratchVertex *)0x1F800070)->position = D_8013B688[primitive->vertexIndices[0]];
    ((TriangleScratchVertex *)0x1F800080)->position = D_8013B688[primitive->vertexIndices[1]];
    ((TriangleScratchVertex *)0x1F800090)->position = D_8013B688[primitive->vertexIndices[2]];
    ((TriangleScratchVertex *)0x1F8000A0)->position = D_8013B688[primitive->vertexIndices[3]];
    *(UVPair *)0x1F800078 = primitive->uv[0].coords;
    *(UVPair *)0x1F800088 = primitive->uv[1].coords;
    *(UVPair *)0x1F800098 = primitive->uv[2].coords;
    *(UVPair *)0x1F8000A8 = primitive->uv[3].coords;
    ((TriangleScratchVertex *)0x1F800070)->color = primitive->colors[0];
    ((TriangleScratchVertex *)0x1F800080)->color = primitive->colors[1];
    ((TriangleScratchVertex *)0x1F800090)->color = primitive->colors[2];
    ((TriangleScratchVertex *)0x1F8000A0)->color = primitive->colors[3];
    func_800542BC((TriangleScratchVertex *)0x1F800070, 0);
}

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_800559F4);

void drawProjectedIndexedFlatTriangle(void) {
    ModelFlatTriangle *primitive = D_8013B680;
    POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 4 << 24;
    setcode(packet, scratch->code | POLY_F3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitFlatTriangle(void) {
    ModelLitFlatTriangle *primitive = (ModelLitFlatTriangle *)D_8013B680;
    POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 4 << 24;
    setcode(packet, scratch->code | POLY_F3_CODE);
    gte_ldv0(&primitive->unk08);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitGouraudTriangle(void) {
    ModelLitGouraudTriangle *primitive = (ModelLitGouraudTriangle *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertices[0].vertexIndex], &D_8013B688[primitive->vertices[1].vertexIndex], &D_8013B688[primitive->vertices[2].vertexIndex]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    gte_ldv0(&primitive->vertices[0].unk00);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->vertices[1].unk00);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->vertices[2].unk00);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedGouraudTriangle(void) {
    GouraudTrianglePrimitive *primitive = (GouraudTrianglePrimitive *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedSingleNormalGouraudTriangle(void) {
    ModelSingleNormalGouraudTriangle *primitive = (ModelSingleNormalGouraudTriangle *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    gte_ldv0(&primitive->unk10);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->unk10);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->unk10);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedColoredLitGouraudTriangle(void) {
    ModelColoredLitGouraudTriangle *primitive = (ModelColoredLitGouraudTriangle *)D_8013B680;
    POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertices[0].vertexIndex], &D_8013B688[primitive->vertices[1].vertexIndex], &D_8013B688[primitive->vertices[2].vertexIndex]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    gte_ldv0(&primitive->vertices[0].unk00);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->vertices[1].unk00);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->vertices[2].unk00);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 6 << 24;
    setcode(packet, scratch->code | POLY_G3_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedTexturedTriangle(void) {
    TexturedTrianglePrimitive *primitive = (TexturedTrianglePrimitive *)D_8013B680;
    POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 7 << 24;
    setcode(packet, scratch->code | POLY_FT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitTexturedTriangle(void) {
    ModelLitTexturedTriangle *primitive = (ModelLitTexturedTriangle *)D_8013B680;
    POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    gte_ldv0(&primitive->unk14);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    packet->tag = 7 << 24;
    setcode(packet, scratch->code | POLY_FT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitTexturedGouraudTriangle(void) {
    ModelLitTexturedGouraudTriangle *primitive = (ModelLitTexturedGouraudTriangle *)D_8013B680;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    LitTexturedTriangleScratchpad *scratch = (LitTexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertices[0].vertexIndex], &D_8013B688[primitive->vertices[1].vertexIndex], &D_8013B688[primitive->vertices[2].vertexIndex]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
        setRGB1(packet, primitive->r, primitive->g, primitive->b);
        setRGB2(packet, primitive->r, primitive->g, primitive->b);
    }
    gte_ldv0(&primitive->vertices[0].unk00);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->vertices[1].unk00);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->vertices[2].unk00);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedGouraudTexturedTriangle(void) {
    TexturedGouraudTrianglePrimitive *primitive = (TexturedGouraudTrianglePrimitive *)D_8013B680;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedLitGouraudTexturedTriangle(void) {
    ModelLitGouraudTexturedTriangle *primitive = (ModelLitGouraudTexturedTriangle *)D_8013B680;
    POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
    }
    gte_ldv0(&primitive->unk1C);
    gte_ldrgb(&packet->r0);
    gte_nccs();
    gte_strgb(&packet->r0);
    gte_ldv0(&primitive->unk1C);
    gte_ldrgb(&packet->r1);
    gte_nccs();
    gte_strgb(&packet->r1);
    gte_ldv0(&primitive->unk1C);
    gte_ldrgb(&packet->r2);
    gte_nccs();
    gte_strgb(&packet->r2);
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_GT3_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->u1 = primitive->uv[1].coords.u;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->v1 = primitive->uv[1].coords.v;
    packet->v2 = primitive->uv[2].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedFlatQuad(void) {
    ModelFlatQuad *primitive = (ModelFlatQuad *)D_8013B680;
    POLY_F4 *packet = (POLY_F4 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 5 << 24;
    setcode(packet, scratch->code | POLY_F4_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedGouraudQuad(void) {
    GouraudQuadPrimitive *primitive = (GouraudQuadPrimitive *)D_8013B680;
    POLY_G4 *packet = (POLY_G4 *)GsOUT_PACKET_P;
    TriangleScratchpad *scratch = TRIANGLE_SCRATCHPAD;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&primitive->colors[3], scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
        setRGB3(packet, primitive->colors[3].r, primitive->colors[3].g, primitive->colors[3].b);
    }
    packet->tag = 8 << 24;
    setcode(packet, scratch->code | POLY_G4_CODE);
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedTexturedQuad(void) {
    TexturedQuadPrimitive *primitive = (TexturedQuadPrimitive *)D_8013B680;
    POLY_FT4 *packet = (POLY_FT4 *)GsOUT_PACKET_P;
    IndexedTexturedQuadScratchpad *scratch = (IndexedTexturedQuadScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->r, scratch->depthCue, (CVECTOR *)&packet->r0);
    } else {
        setRGB0(packet, primitive->r, primitive->g, primitive->b);
    }
    packet->tag = 9 << 24;
    setcode(packet, scratch->code | POLY_FT4_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->u1 = primitive->uv[1].coords.u;
    packet->v1 = primitive->uv[1].coords.v;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v2 = primitive->uv[2].coords.v;
    packet->u3 = primitive->uv[3].coords.u;
    packet->v3 = primitive->uv[3].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

void drawProjectedIndexedTexturedGouraudQuad(void) {
    TexturedGouraudQuadPrimitive *primitive = (TexturedGouraudQuadPrimitive *)D_8013B680;
    POLY_GT4 *packet = (POLY_GT4 *)GsOUT_PACKET_P;
    IndexedTexturedQuadScratchpad *scratch = (IndexedTexturedQuadScratchpad *)0x1F800000;
    gte_ldv3(&D_8013B688[primitive->vertexIndices[0]], &D_8013B688[primitive->vertexIndices[1]], &D_8013B688[primitive->vertexIndices[2]]);
    gte_rtpt();
    gte_stsxy3(&packet->x0, &packet->x1, &packet->x2);
    gte_nclip();
    gte_stopz(&scratch->clipping);
    gte_ldv0(&D_8013B688[primitive->vertexIndices[3]]);
    gte_rtps();
    gte_stsxy(&packet->x3);
    gte_stflg(&scratch->flags);
    gte_stszotz(&scratch->depth);
    if (scratch->clipping < 0 || scratch->flags < 0 || scratch->depth <= 0 || scratch->depth >= 0x1000) return;
    if (scratch->fogEnabled) {
        gte_stdp(&scratch->depthCue);
        DpqColor((CVECTOR *)&primitive->colors[0], scratch->depthCue, (CVECTOR *)&packet->r0);
        DpqColor((CVECTOR *)&primitive->colors[1], scratch->depthCue, (CVECTOR *)&packet->r1);
        DpqColor((CVECTOR *)&primitive->colors[2], scratch->depthCue, (CVECTOR *)&packet->r2);
        DpqColor((CVECTOR *)&primitive->colors[3], scratch->depthCue, (CVECTOR *)&packet->r3);
    } else {
        setRGB0(packet, primitive->colors[0].r, primitive->colors[0].g, primitive->colors[0].b);
        setRGB1(packet, primitive->colors[1].r, primitive->colors[1].g, primitive->colors[1].b);
        setRGB2(packet, primitive->colors[2].r, primitive->colors[2].g, primitive->colors[2].b);
        setRGB3(packet, primitive->colors[3].r, primitive->colors[3].g, primitive->colors[3].b);
    }
    packet->tag = 12 << 24;
    setcode(packet, scratch->code | POLY_GT4_CODE);
    packet->tpage = primitive->uv[1].attribute;
    packet->clut = primitive->uv[0].attribute;
    packet->u0 = primitive->uv[0].coords.u;
    packet->v0 = primitive->uv[0].coords.v;
    packet->u1 = primitive->uv[1].coords.u;
    packet->v1 = primitive->uv[1].coords.v;
    packet->u2 = primitive->uv[2].coords.u;
    packet->v2 = primitive->uv[2].coords.v;
    packet->u3 = primitive->uv[3].coords.u;
    packet->v3 = primitive->uv[3].coords.v;
    addPrim(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
    GsOUT_PACKET_P = (u32)(packet + 1);
}

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058350);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058560);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_800585BC);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058664);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058844);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_800589F0);
