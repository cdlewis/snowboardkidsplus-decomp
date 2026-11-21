#include "psx/libetc.h"
#include "render_effects_internal.h"
#include "game/graphics/graphics.h"
#include "psx/libapi.h"
#include "psx/libgpu.h"
#include "psx/libgs.h"
#include "psx/libgte.h"

typedef struct SubdivisionScratchpad {
    u32 unk00;
    void (*emitTriangle)(TriangleScratchVertex *);
    s32 depth, depthCue, flags, minDepth, clipping;
    u8 unk1C[0x10];
    volatile s32 vertexDepth[3];
    u8 unk38[0x10];
    struct { u16 x, y; } max, min;
    volatile struct { u16 x, y; } screen[3];
    u8 unk5C[0xC];
    u8 textured, gouraud;
} SubdivisionScratchpad;

typedef struct QuadSubdivisionScratchpad {
    u32 unk00;
    void (*emitQuad)(TriangleScratchVertex *);
    s32 depth, depthCue, flags;
    s32 minDepth; /* Holds the fourth vertex's projection flags before depth sorting. */
    s32 clipping;
    u8 unk1C[0x10];
    volatile s32 vertexDepth[4];
    u8 unk3C[0xC];
    struct { u16 x, y; } max, min;
    volatile struct { u16 x, y; } screen[4];
    u8 unk60[8];
    u8 textured, gouraud;
} QuadSubdivisionScratchpad;

extern u32 GsOUT_PACKET_P __attribute__((section(".data")));

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
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)getScratchAddr(0);
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
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)getScratchAddr(0);
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
    GouraudQuadScratchpad *scratch = (GouraudQuadScratchpad *)getScratchAddr(0);
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
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)getScratchAddr(0);
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
    TexturedQuadScratchpad *scratch = (TexturedQuadScratchpad *)getScratchAddr(0);
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

void drawSubdivisionTriangle(TriangleScratchVertex *first, TriangleScratchVertex *second,
                             TriangleScratchVertex *third) {
    ProjectedModelScratchpad *scratch = (ProjectedModelScratchpad *)getScratchAddr(0);
    if (scratch->textured) {
        if (scratch->gouraud) {
            POLY_GT3 *packet = (POLY_GT3 *)GsOUT_PACKET_P;
            gte_ldv3(&first->position, &second->position, &third->position);
            gte_rtpt();
            gte_stflg(&scratch->flags);
            if (scratch->flags & 0x7FFBE000) {
                return;
            }
            gte_stsxy3_gt3(packet);
            packet->tag = 9 << 24;
            setcode(packet, scratch->code | POLY_GT3_CODE);
            packet->tpage = scratch->tpage;
            packet->clut = scratch->clut;
            gte_avsz3();
            gte_stotz(&scratch->depth);
            if (scratch->depth < 25) {
                return;
            }
            packet->u0 = first->texture.uv.u;
            packet->u1 = second->texture.uv.u;
            packet->u2 = third->texture.uv.u;
            packet->v0 = first->texture.uv.v;
            packet->v1 = second->texture.uv.v;
            packet->v2 = third->texture.uv.v;
            packet->r0 = first->color.r;
            packet->r1 = second->color.r;
            packet->r2 = third->color.r;
            packet->g0 = first->color.g;
            packet->g1 = second->color.g;
            packet->g2 = third->color.g;
            packet->b0 = first->color.b;
            packet->b1 = second->color.b;
            packet->b2 = third->color.b;
            setaddr(packet, getaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6]));
            GsOUT_PACKET_P = (u32)(packet + 1);
            setaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
        } else {
            POLY_FT3 *packet = (POLY_FT3 *)GsOUT_PACKET_P;
            gte_ldv3(&first->position, &second->position, &third->position);
            gte_rtpt();
            gte_stflg(&scratch->flags);
            if (scratch->flags & 0x7FFBE000) {
                return;
            }
            gte_stsxy3_ft3(packet);
            packet->tag = 7 << 24;
            setcode(packet, scratch->code | POLY_FT3_CODE);
            packet->tpage = scratch->tpage;
            packet->clut = scratch->clut;
            gte_avsz3();
            gte_stotz(&scratch->depth);
            if (scratch->depth < 25) {
                return;
            }
            packet->u0 = first->texture.uv.u;
            packet->u1 = second->texture.uv.u;
            packet->u2 = third->texture.uv.u;
            packet->v0 = first->texture.uv.v;
            packet->v1 = second->texture.uv.v;
            packet->v2 = third->texture.uv.v;
            setRGB0(packet, scratch->r, scratch->g, scratch->b);
            setaddr(packet, getaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6]));
            GsOUT_PACKET_P = (u32)(packet + 1);
            setaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
        }
    } else {
        if (scratch->gouraud) {
            POLY_G3 *packet = (POLY_G3 *)GsOUT_PACKET_P;
            gte_ldv3(&first->position, &second->position, &third->position);
            gte_rtpt();
            gte_stflg(&scratch->flags);
            if (scratch->flags & 0x7FFBE000) {
                return;
            }
            gte_stsxy3_g3(packet);
            packet->tag = 6 << 24;
            setcode(packet, scratch->code | POLY_G3_CODE);
            gte_avsz3();
            gte_stotz(&scratch->depth);
            if (scratch->depth < 25) {
                return;
            }
            packet->r0 = first->color.r;
            packet->r1 = second->color.r;
            packet->r2 = third->color.r;
            packet->g0 = first->color.g;
            packet->g1 = second->color.g;
            packet->g2 = third->color.g;
            packet->b0 = first->color.b;
            packet->b1 = second->color.b;
            packet->b2 = third->color.b;
            setaddr(packet, getaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6]));
            GsOUT_PACKET_P = (u32)(packet + 1);
            setaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
        } else {
            POLY_F3 *packet = (POLY_F3 *)GsOUT_PACKET_P;
            gte_ldv3(&first->position, &second->position, &third->position);
            gte_rtpt();
            gte_stflg(&scratch->flags);
            if (scratch->flags & 0x7FFBE000) {
                return;
            }
            gte_stsxy3_f3(packet);
            packet->tag = 4 << 24;
            setcode(packet, scratch->code | POLY_F3_CODE);
            gte_avsz3();
            gte_stotz(&scratch->depth);
            if (scratch->depth < 25) {
                return;
            }
            setRGB0(packet, scratch->r, scratch->g, scratch->b);
            setaddr(packet, getaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6]));
            GsOUT_PACKET_P = (u32)(packet + 1);
            setaddr(&D_8013B670->org[(scratch->depth >> D_8013B69C) + 6], packet);
        }
    }
}

void drawAdaptiveTriangle(TriangleScratchVertex *vertices, s32 level) {
    s16 subdivision = level;
    SubdivisionScratchpad *scratch = (SubdivisionScratchpad *)getScratchAddr(0);
    TriangleScratchVertex *next = vertices + 3;
    u16 a, b;
    s32 z, otherDepth;

    gte_ldv3(&vertices[0].position, &vertices[1].position, &vertices[2].position);
    gte_rtpt();
    if (subdivision == 0) {
        gte_nclip();
        gte_stopz(&scratch->clipping);
        if (scratch->clipping < 0) {
            return;
        }
    }
    gte_stflg(&scratch->flags);
    gte_stsxy3(&scratch->screen[0], &scratch->screen[1], &scratch->screen[2]);

    a = scratch->screen[0].x;
    b = scratch->screen[1].x;
    if ((s16)a > (s16)b) {
        scratch->max.x = a;
        scratch->min.x = b;
    } else {
        scratch->min.x = a;
        scratch->max.x = b;
    }
    a = scratch->screen[2].x;
    if ((s16)a > (s16)scratch->max.x) {
        scratch->max.x = a;
        a = scratch->screen[2].x;
    }
    if ((s16)a < (s16)scratch->min.x) {
        scratch->min.x = a;
    }
    if ((s16)scratch->min.x > 200 || (s16)scratch->max.x < -200) {
        return;
    }

    a = scratch->screen[0].y;
    if ((s16)a > (s16)scratch->screen[1].y) {
        u16 secondY = scratch->screen[1].y;
        scratch->max.y = a;
        scratch->min.y = secondY;
    } else {
        u16 secondY = scratch->screen[1].y;
        scratch->min.y = a;
        scratch->max.y = secondY;
    }
    a = scratch->screen[2].y;
    if ((s16)a > (s16)scratch->max.y) {
        scratch->max.y = a;
        a = scratch->screen[2].y;
    }
    if ((s16)a < (s16)scratch->min.y) {
        scratch->min.y = a;
    }
    if ((s16)scratch->min.y > 160 || (s16)scratch->max.y < -160) {
        return;
    }

    gte_stsz3(&scratch->vertexDepth[0], &scratch->vertexDepth[1], &scratch->vertexDepth[2]);
    z = scratch->vertexDepth[0];
    if (z > scratch->vertexDepth[1]) {
        otherDepth = scratch->vertexDepth[1];
        scratch->depth = z;
        scratch->minDepth = otherDepth;
    } else {
        otherDepth = scratch->vertexDepth[1];
        scratch->minDepth = z;
        scratch->depth = otherDepth;
    }
    z = scratch->vertexDepth[2];
    if (z > scratch->depth) {
        scratch->depth = z;
        z = scratch->vertexDepth[2];
    }
    if (z < scratch->minDepth) {
        scratch->minDepth = z;
    }

    z = scratch->depth >> 2;
    scratch->depth = z;
    if (z < 25) {
        return;
    }
    if (z <= 0 || z >= 4096) {
        return;
    }
    if (subdivision >= 3 ||
        (!(scratch->flags & 0x7FFBE000) &&
         (s16)scratch->max.x - (s16)scratch->min.x < 128 &&
         (s16)scratch->max.y - (s16)scratch->min.y < 128)) {
        scratch->emitTriangle(vertices);
        return;
    }

    next[0].position = vertices[0].position;
    setVector(&next[1].position,
              (vertices[0].position.vx + vertices[1].position.vx) / 2,
              (vertices[0].position.vy + vertices[1].position.vy) / 2,
              (vertices[0].position.vz + vertices[1].position.vz) / 2);
    setVector(&next[2].position,
              (vertices[0].position.vx + vertices[2].position.vx) / 2,
              (vertices[0].position.vy + vertices[2].position.vy) / 2,
              (vertices[0].position.vz + vertices[2].position.vz) / 2);
    subdivision++;
    if (scratch->textured) {
        next[0].texture.coords = vertices[0].texture.coords;
        next[1].texture.uv.u = (vertices[0].texture.uv.u + vertices[1].texture.uv.u) / 2;
        next[1].texture.uv.v = (vertices[0].texture.uv.v + vertices[1].texture.uv.v) / 2;
        next[2].texture.uv.u = (vertices[0].texture.uv.u + vertices[2].texture.uv.u) / 2;
        next[2].texture.uv.v = (vertices[0].texture.uv.v + vertices[2].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color = vertices[0].color;
        next[1].color.r = (vertices[0].color.r + vertices[1].color.r) / 2;
        next[1].color.g = (vertices[0].color.g + vertices[1].color.g) / 2;
        next[1].color.b = (vertices[0].color.b + vertices[1].color.b) / 2;
        next[2].color.r = (vertices[0].color.r + vertices[2].color.r) / 2;
        next[2].color.g = (vertices[0].color.g + vertices[2].color.g) / 2;
        next[2].color.b = (vertices[0].color.b + vertices[2].color.b) / 2;
    }
    drawAdaptiveTriangle(next, subdivision);
    drawSubdivisionTriangle(&next[1], &vertices[0], &vertices[1]);

    next[0].position = vertices[1].position;
    setVector(&next[2].position,
              (vertices[1].position.vx + vertices[2].position.vx) / 2,
              (vertices[1].position.vy + vertices[2].position.vy) / 2,
              (vertices[1].position.vz + vertices[2].position.vz) / 2);
    if (scratch->textured) {
        next[0].texture.coords = vertices[1].texture.coords;
        next[2].texture.uv.u = (vertices[1].texture.uv.u + vertices[2].texture.uv.u) / 2;
        next[2].texture.uv.v = (vertices[1].texture.uv.v + vertices[2].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color = vertices[1].color;
        next[2].color.r = (vertices[1].color.r + vertices[2].color.r) / 2;
        next[2].color.g = (vertices[1].color.g + vertices[2].color.g) / 2;
        next[2].color.b = (vertices[1].color.b + vertices[2].color.b) / 2;
    }
    drawAdaptiveTriangle(next, subdivision);
    drawSubdivisionTriangle(&next[2], &vertices[1], &vertices[2]);

    setVector(&next[0].position,
              (vertices[0].position.vx + vertices[2].position.vx) / 2,
              (vertices[0].position.vy + vertices[2].position.vy) / 2,
              (vertices[0].position.vz + vertices[2].position.vz) / 2);
    if (scratch->textured) {
        next[0].texture.uv.u = (vertices[0].texture.uv.u + vertices[2].texture.uv.u) / 2;
        next[0].texture.uv.v = (vertices[0].texture.uv.v + vertices[2].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color.r = (vertices[0].color.r + vertices[2].color.r) / 2;
        next[0].color.g = (vertices[0].color.g + vertices[2].color.g) / 2;
        next[0].color.b = (vertices[0].color.b + vertices[2].color.b) / 2;
    }
    drawAdaptiveTriangle(next, subdivision);
    drawSubdivisionTriangle(&next[0], &vertices[2], &vertices[0]);

    next[1].position = vertices[2].position;
    if (scratch->textured) {
        next[1].texture.coords = vertices[2].texture.coords;
    }
    if (scratch->gouraud) {
        next[1].color = vertices[2].color;
    }
    drawAdaptiveTriangle(next, subdivision);
}

void drawAdaptiveQuad(TriangleScratchVertex *vertices, s32 level) {
    s16 subdivision = level;
    QuadSubdivisionScratchpad *scratch = (QuadSubdivisionScratchpad *)getScratchAddr(0);
    TriangleScratchVertex *next = vertices + 4;
    u16 a, b;
    s32 z, otherDepth;

    gte_ldv3(&vertices[0].position, &vertices[1].position, &vertices[2].position);
    gte_rtpt();
    if (subdivision == 0) {
        gte_nclip();
        gte_stopz(&scratch->clipping);
        if (scratch->clipping < 0) {
            return;
        }
    }
    gte_stsxy3(&scratch->screen[0], &scratch->screen[1], &scratch->screen[2]);
    gte_stflg(&scratch->flags);
    gte_ldv0(&vertices[3].position);
    gte_rtps();
    gte_stsxy(&scratch->screen[3]);
    gte_stflg(&scratch->minDepth);
    scratch->flags |= scratch->minDepth;

    a = scratch->screen[0].x;
    b = scratch->screen[1].x;
    if ((s16)a > (s16)b) {
        scratch->max.x = a;
        scratch->min.x = b;
    } else {
        scratch->min.x = a;
        scratch->max.x = b;
    }
    a = scratch->screen[2].x;
    if ((s16)a > (s16)scratch->max.x) {
        scratch->max.x = a;
        a = scratch->screen[2].x;
    }
    if ((s16)a < (s16)scratch->min.x) {
        scratch->min.x = a;
    }
    a = scratch->screen[3].x;
    if ((s16)a > (s16)scratch->max.x) {
        scratch->max.x = a;
        a = scratch->screen[3].x;
    }
    if ((s16)a < (s16)scratch->min.x) {
        scratch->min.x = a;
    }
    if ((s16)scratch->min.x > 200 || (s16)scratch->max.x < -200) {
        return;
    }

    a = scratch->screen[0].y;
    if ((s16)a > (s16)scratch->screen[1].y) {
        u16 secondY = scratch->screen[1].y;
        scratch->max.y = a;
        scratch->min.y = secondY;
    } else {
        u16 secondY = scratch->screen[1].y;
        scratch->min.y = a;
        scratch->max.y = secondY;
    }
    a = scratch->screen[2].y;
    if ((s16)a > (s16)scratch->max.y) {
        scratch->max.y = a;
        a = scratch->screen[2].y;
    }
    if ((s16)a < (s16)scratch->min.y) {
        scratch->min.y = a;
    }
    a = scratch->screen[3].y;
    if ((s16)a > (s16)scratch->max.y) {
        scratch->max.y = a;
        a = scratch->screen[3].y;
    }
    if ((s16)a < (s16)scratch->min.y) {
        scratch->min.y = a;
    }
    if ((s16)scratch->min.y > 160 || (s16)scratch->max.y < -160) {
        return;
    }

    gte_stsz4(&scratch->vertexDepth[0],
              &scratch->vertexDepth[1],
              &scratch->vertexDepth[2],
              &scratch->vertexDepth[3]);
    z = scratch->vertexDepth[0];
    if (z > scratch->vertexDepth[1]) {
        otherDepth = scratch->vertexDepth[1];
        scratch->depth = z;
        scratch->minDepth = otherDepth;
    } else {
        otherDepth = scratch->vertexDepth[1];
        scratch->minDepth = z;
        scratch->depth = otherDepth;
    }
    z = scratch->vertexDepth[2];
    if (z > scratch->depth) {
        scratch->depth = z;
        z = scratch->vertexDepth[2];
    }
    if (z < scratch->minDepth) {
        scratch->minDepth = z;
    }

    z = scratch->vertexDepth[3];
    if (z > scratch->depth) {
        scratch->depth = z;
        z = scratch->vertexDepth[3];
    }
    if (z < scratch->minDepth) {
        scratch->minDepth = z;
    }

    z = scratch->depth >> 2;
    scratch->depth = z;
    if (z < 25) {
        return;
    }
    if (z <= 0 || z >= 4096) {
        return;
    }
    if (subdivision >= 3 ||
        (!(scratch->flags & 0x7FFBE000) &&
         (s16)scratch->max.x - (s16)scratch->min.x < 128 &&
         (s16)scratch->max.y - (s16)scratch->min.y < 128)) {
        scratch->emitQuad(vertices);
        return;
    }

    next[0].position = vertices[0].position;
    setVector(&next[1].position,
              (vertices[0].position.vx + vertices[1].position.vx) / 2,
              (vertices[0].position.vy + vertices[1].position.vy) / 2,
              (vertices[0].position.vz + vertices[1].position.vz) / 2);
    setVector(&next[2].position,
              (vertices[0].position.vx + vertices[2].position.vx) / 2,
              (vertices[0].position.vy + vertices[2].position.vy) / 2,
              (vertices[0].position.vz + vertices[2].position.vz) / 2);
    setVector(&next[3].position,
              (vertices[0].position.vx + vertices[3].position.vx) / 2,
              (vertices[0].position.vy + vertices[3].position.vy) / 2,
              (vertices[0].position.vz + vertices[3].position.vz) / 2);
    subdivision++;
    if (scratch->textured) {
        next[0].texture.coords = vertices[0].texture.coords;
        next[1].texture.uv.u = (vertices[0].texture.uv.u + vertices[1].texture.uv.u) / 2;
        next[1].texture.uv.v = (vertices[0].texture.uv.v + vertices[1].texture.uv.v) / 2;
        next[2].texture.uv.u = (vertices[0].texture.uv.u + vertices[2].texture.uv.u) / 2;
        next[2].texture.uv.v = (vertices[0].texture.uv.v + vertices[2].texture.uv.v) / 2;
        next[3].texture.uv.u = (vertices[0].texture.uv.u + vertices[3].texture.uv.u) / 2;
        next[3].texture.uv.v = (vertices[0].texture.uv.v + vertices[3].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color = vertices[0].color;
        next[1].color.r = (vertices[0].color.r + vertices[1].color.r) / 2;
        next[1].color.g = (vertices[0].color.g + vertices[1].color.g) / 2;
        next[1].color.b = (vertices[0].color.b + vertices[1].color.b) / 2;
        next[2].color.r = (vertices[0].color.r + vertices[2].color.r) / 2;
        next[2].color.g = (vertices[0].color.g + vertices[2].color.g) / 2;
        next[2].color.b = (vertices[0].color.b + vertices[2].color.b) / 2;
        next[3].color.r = (vertices[0].color.r + vertices[3].color.r) / 2;
        next[3].color.g = (vertices[0].color.g + vertices[3].color.g) / 2;
        next[3].color.b = (vertices[0].color.b + vertices[3].color.b) / 2;
    }
    drawAdaptiveQuad(next, subdivision);
    drawSubdivisionTriangle(&next[1], &vertices[0], &vertices[1]);

    next[0].position = vertices[1].position;
    setVector(&next[2].position,
              (vertices[1].position.vx + vertices[3].position.vx) / 2,
              (vertices[1].position.vy + vertices[3].position.vy) / 2,
              (vertices[1].position.vz + vertices[3].position.vz) / 2);
    if (scratch->textured) {
        next[0].texture.coords = vertices[1].texture.coords;
        next[2].texture.uv.u = (vertices[1].texture.uv.u + vertices[3].texture.uv.u) / 2;
        next[2].texture.uv.v = (vertices[1].texture.uv.v + vertices[3].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color = vertices[1].color;
        next[2].color.r = (vertices[1].color.r + vertices[3].color.r) / 2;
        next[2].color.g = (vertices[1].color.g + vertices[3].color.g) / 2;
        next[2].color.b = (vertices[1].color.b + vertices[3].color.b) / 2;
    }
    drawAdaptiveQuad(next, subdivision);
    drawSubdivisionTriangle(&next[2], &vertices[1], &vertices[3]);

    next[0].position = vertices[3].position;
    setVector(&next[1].position,
              (vertices[3].position.vx + vertices[2].position.vx) / 2,
              (vertices[3].position.vy + vertices[2].position.vy) / 2,
              (vertices[3].position.vz + vertices[2].position.vz) / 2);
    if (scratch->textured) {
        next[0].texture.coords = vertices[3].texture.coords;
        next[1].texture.uv.u = (vertices[3].texture.uv.u + vertices[2].texture.uv.u) / 2;
        next[1].texture.uv.v = (vertices[3].texture.uv.v + vertices[2].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color = vertices[3].color;
        next[1].color.r = (vertices[3].color.r + vertices[2].color.r) / 2;
        next[1].color.g = (vertices[3].color.g + vertices[2].color.g) / 2;
        next[1].color.b = (vertices[3].color.b + vertices[2].color.b) / 2;
    }
    drawAdaptiveQuad(next, subdivision);
    drawSubdivisionTriangle(&next[1], &vertices[3], &vertices[2]);

    next[0].position = vertices[2].position;
    setVector(&next[2].position,
              (vertices[0].position.vx + vertices[2].position.vx) / 2,
              (vertices[0].position.vy + vertices[2].position.vy) / 2,
              (vertices[0].position.vz + vertices[2].position.vz) / 2);
    if (scratch->textured) {
        next[0].texture.coords = vertices[2].texture.coords;
        next[2].texture.uv.u = (vertices[0].texture.uv.u + vertices[2].texture.uv.u) / 2;
        next[2].texture.uv.v = (vertices[0].texture.uv.v + vertices[2].texture.uv.v) / 2;
    }
    if (scratch->gouraud) {
        next[0].color = vertices[2].color;
        next[2].color.r = (vertices[0].color.r + vertices[2].color.r) / 2;
        next[2].color.g = (vertices[0].color.g + vertices[2].color.g) / 2;
        next[2].color.b = (vertices[0].color.b + vertices[2].color.b) / 2;
    }
    drawAdaptiveQuad(next, subdivision);
    drawSubdivisionTriangle(&next[2], &vertices[2], &vertices[0]);
}

void drawIndexedFlatTriangle(void) {
    ModelFlatTriangle *primitive = D_8013B680;
    TRIANGLE_SCRATCHPAD->emitTriangle = emitProjectedFlatTriangle;
    TRIANGLE_SCRATCHPAD->r = primitive->r;
    TRIANGLE_SCRATCHPAD->g = primitive->g;
    TRIANGLE_SCRATCHPAD->b = primitive->b;
    TRIANGLE_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    TRIANGLE_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    TRIANGLE_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    drawAdaptiveTriangle(TRIANGLE_SCRATCHPAD->vertices, 0);
}

void processGouraudTrianglePrimitive(void) {
    GouraudTrianglePrimitive *primitive = (GouraudTrianglePrimitive *)D_8013B680;
    TRIANGLE_SCRATCHPAD->emitTriangle = emitProjectedGouraudTriangle;
    TRIANGLE_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    TRIANGLE_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    TRIANGLE_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    TRIANGLE_SCRATCHPAD->vertices[0].color = primitive->colors[0];
    TRIANGLE_SCRATCHPAD->vertices[1].color = primitive->colors[1];
    TRIANGLE_SCRATCHPAD->vertices[2].color = primitive->colors[2];
    drawAdaptiveTriangle(TRIANGLE_SCRATCHPAD->vertices, 0);
}

void processTexturedTrianglePrimitive(void) {
    TexturedTrianglePrimitive *primitive = (TexturedTrianglePrimitive *)D_8013B680;
    ((TriangleTextureState *)getScratchAddr(0))->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->r = primitive->r;
    ((TriangleTextureState *)getScratchAddr(0))->g = primitive->g;
    ((TriangleTextureState *)getScratchAddr(0))->b = primitive->b;
    ((TriangleTextureState *)getScratchAddr(0))->callback = emitProjectedTexturedTriangle;
    TRIANGLE_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    TRIANGLE_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    TRIANGLE_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    TRIANGLE_SCRATCHPAD->vertices[0].texture.coords = primitive->uv[0].coords;
    TRIANGLE_SCRATCHPAD->vertices[1].texture.coords = primitive->uv[1].coords;
    TRIANGLE_SCRATCHPAD->vertices[2].texture.coords = primitive->uv[2].coords;
    drawAdaptiveTriangle(TRIANGLE_SCRATCHPAD->vertices, 0);
}

void processTexturedGouraudTrianglePrimitive(void) {
    TexturedGouraudTrianglePrimitive *primitive = (TexturedGouraudTrianglePrimitive *)D_8013B680;
    ((TriangleTextureState *)getScratchAddr(0))->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->callback = emitProjectedGouraudTexturedTriangle;
    TRIANGLE_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    TRIANGLE_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    TRIANGLE_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    TRIANGLE_SCRATCHPAD->vertices[0].texture.coords = primitive->uv[0].coords;
    TRIANGLE_SCRATCHPAD->vertices[1].texture.coords = primitive->uv[1].coords;
    TRIANGLE_SCRATCHPAD->vertices[2].texture.coords = primitive->uv[2].coords;
    TRIANGLE_SCRATCHPAD->vertices[0].color = primitive->colors[0];
    TRIANGLE_SCRATCHPAD->vertices[1].color = primitive->colors[1];
    TRIANGLE_SCRATCHPAD->vertices[2].color = primitive->colors[2];
    drawAdaptiveTriangle(TRIANGLE_SCRATCHPAD->vertices, 0);
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
    drawAdaptiveQuad(QUAD_SCRATCHPAD->vertices, 0);
}

void processGouraudQuadPrimitive(void) {
    GouraudQuadPrimitive *primitive = (GouraudQuadPrimitive *)D_8013B680;
    QUAD_SCRATCHPAD->emitQuad = emitProjectedGouraudQuad;
    QUAD_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    QUAD_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    QUAD_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    QUAD_SCRATCHPAD->vertices[3].position = D_8013B688[primitive->vertexIndices[3]];
    QUAD_SCRATCHPAD->vertices[0].color = primitive->colors[0];
    QUAD_SCRATCHPAD->vertices[1].color = primitive->colors[1];
    QUAD_SCRATCHPAD->vertices[2].color = primitive->colors[2];
    QUAD_SCRATCHPAD->vertices[3].color = primitive->colors[3];
    drawAdaptiveQuad(QUAD_SCRATCHPAD->vertices, 0);
}

void processTexturedQuadPrimitive(void) {
    TexturedQuadPrimitive *primitive = (TexturedQuadPrimitive *)D_8013B680;
    ((TriangleTextureState *)getScratchAddr(0))->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->r = primitive->r;
    ((TriangleTextureState *)getScratchAddr(0))->g = primitive->g;
    ((TriangleTextureState *)getScratchAddr(0))->b = primitive->b;
    ((TriangleTextureState *)getScratchAddr(0))->callback = emitProjectedTexturedQuad;
    QUAD_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    QUAD_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    QUAD_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    QUAD_SCRATCHPAD->vertices[3].position = D_8013B688[primitive->vertexIndices[3]];
    QUAD_SCRATCHPAD->vertices[0].texture.coords = primitive->uv[0].coords;
    QUAD_SCRATCHPAD->vertices[1].texture.coords = primitive->uv[1].coords;
    QUAD_SCRATCHPAD->vertices[2].texture.coords = primitive->uv[2].coords;
    QUAD_SCRATCHPAD->vertices[3].texture.coords = primitive->uv[3].coords;
    drawAdaptiveQuad(QUAD_SCRATCHPAD->vertices, 0);
}

void processTexturedGouraudQuadPrimitive(void) {
    TexturedGouraudQuadPrimitive *primitive = (TexturedGouraudQuadPrimitive *)D_8013B680;
    ((TriangleTextureState *)getScratchAddr(0))->tpage = primitive->uv[1].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->clut = primitive->uv[0].attribute;
    ((TriangleTextureState *)getScratchAddr(0))->callback = emitProjectedGouraudTexturedQuad;
    QUAD_SCRATCHPAD->vertices[0].position = D_8013B688[primitive->vertexIndices[0]];
    QUAD_SCRATCHPAD->vertices[1].position = D_8013B688[primitive->vertexIndices[1]];
    QUAD_SCRATCHPAD->vertices[2].position = D_8013B688[primitive->vertexIndices[2]];
    QUAD_SCRATCHPAD->vertices[3].position = D_8013B688[primitive->vertexIndices[3]];
    QUAD_SCRATCHPAD->vertices[0].texture.coords = primitive->uv[0].coords;
    QUAD_SCRATCHPAD->vertices[1].texture.coords = primitive->uv[1].coords;
    QUAD_SCRATCHPAD->vertices[2].texture.coords = primitive->uv[2].coords;
    QUAD_SCRATCHPAD->vertices[3].texture.coords = primitive->uv[3].coords;
    QUAD_SCRATCHPAD->vertices[0].color = primitive->colors[0];
    QUAD_SCRATCHPAD->vertices[1].color = primitive->colors[1];
    QUAD_SCRATCHPAD->vertices[2].color = primitive->colors[2];
    QUAD_SCRATCHPAD->vertices[3].color = primitive->colors[3];
    drawAdaptiveQuad(QUAD_SCRATCHPAD->vertices, 0);
}

void func_800559F4(TmdObject *object, GsOT *ot, s32 shift, u32 flags) {
    ProjectedModelScratchpad *scratch = (ProjectedModelScratchpad *)getScratchAddr(0);
    u32 i;

    D_8013B670 = ot;
    D_8013B69C = shift;
    D_8013B688 = object->vertices;
    D_8013B680 = (ModelFlatTriangle *)object->primitives;
    D_8013B68C = object->primitiveCount;
    if (flags & 1) scratch->fogEnabled = 1;
    else scratch->fogEnabled = 0;
    scratch->unk6B = (flags >> 1) & 1;
    D_8013B668 = SetSp((u32)getScratchAddr(255));

    for (i = 0; i < D_8013B68C; i++) {
        scratch->header.word = D_8013B680->unk00;
        scratch->code = scratch->header.bytes.mode & 2;
        scratch->textured = (scratch->header.bytes.mode >> 2) & 1;
        scratch->gouraud = (scratch->header.bytes.mode >> 4) & 1;
        switch (scratch->header.word & 0xFDFFFFFF) {
        case ((POLY_F3_CODE | 1) << 24) | 0x00010304: drawIndexedFlatTriangle(); break;
        case ((POLY_FT3_CODE | 1) << 24) | 0x00010607: processTexturedTrianglePrimitive(); break;
        case ((POLY_G3_CODE | 1) << 24) | 0x00010506: processGouraudTrianglePrimitive(); break;
        case ((POLY_GT3_CODE | 1) << 24) | 0x00010809: processTexturedGouraudTrianglePrimitive(); break;
        case ((POLY_F4_CODE | 1) << 24) | 0x00010305: drawIndexedFlatQuad(); break;
        case ((POLY_G4_CODE | 1) << 24) | 0x00010608: processGouraudQuadPrimitive(); break;
        case ((POLY_FT4_CODE | 1) << 24) | 0x00010709: processTexturedQuadPrimitive(); break;
        case ((POLY_GT4_CODE | 1) << 24) | 0x00010A0C: processTexturedGouraudQuadPrimitive(); break;
        }
        D_8013B680 = (ModelFlatTriangle *)((u32 *)D_8013B680 + (scratch->header.bytes.inputLength + 1));
    }
    SetSp(D_8013B668);
}

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
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)getScratchAddr(0);
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
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)getScratchAddr(0);
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
    LitTexturedTriangleScratchpad *scratch = (LitTexturedTriangleScratchpad *)getScratchAddr(0);
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
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)getScratchAddr(0);
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
    TexturedTriangleScratchpad *scratch = (TexturedTriangleScratchpad *)getScratchAddr(0);
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
    IndexedTexturedQuadScratchpad *scratch = (IndexedTexturedQuadScratchpad *)getScratchAddr(0);
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
    IndexedTexturedQuadScratchpad *scratch = (IndexedTexturedQuadScratchpad *)getScratchAddr(0);
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

void drawProjectedModelPrimitives(TmdObject *object, GsOT *ot, s32 shift, u32 flags) {
    ProjectedModelScratchpad *scratch = (ProjectedModelScratchpad *)getScratchAddr(0);
    u16 i;
    D_8013B670 = ot;
    D_8013B69C = shift;
    D_8013B688 = object->vertices;
    D_8013B680 = (ModelFlatTriangle *)object->primitives;
    D_8013B68C = object->primitiveCount;
    if (flags & 1) scratch->fogEnabled = 1;
    else scratch->fogEnabled = 0;
    scratch->unk6B = (flags >> 1) & 1;
    D_8013B66C = SetSp((u32)getScratchAddr(255));
    for (i = 0; i < D_8013B68C; i++) {
        scratch->header.word = D_8013B680->unk00;
        scratch->code = scratch->header.bytes.mode & 2;
        switch (scratch->header.word & 0xFDFFFFFF) {
        case ((POLY_F3_CODE | 1) << 24) | 0x00010304: drawProjectedIndexedFlatTriangle(); break;
        case ((POLY_G3_CODE | 1) << 24) | 0x00010506: drawProjectedIndexedGouraudTriangle(); break;
        case ((POLY_FT3_CODE | 1) << 24) | 0x00010607: drawProjectedIndexedTexturedTriangle(); break;
        case ((POLY_GT3_CODE | 1) << 24) | 0x00010809: drawProjectedIndexedGouraudTexturedTriangle(); break;
        case ((POLY_F4_CODE | 1) << 24) | 0x00010305: drawProjectedIndexedFlatQuad(); break;
        case ((POLY_G4_CODE | 1) << 24) | 0x00010608: drawProjectedIndexedGouraudQuad(); break;
        case ((POLY_FT4_CODE | 1) << 24) | 0x00010709: drawProjectedIndexedTexturedQuad(); break;
        case ((POLY_GT4_CODE | 1) << 24) | 0x00010A0C: drawProjectedIndexedTexturedGouraudQuad(); break;
        }
        D_8013B680 = (ModelFlatTriangle *)((u32 *)D_8013B680 + (scratch->header.bytes.inputLength + 1));
    }
    SetSp(D_8013B66C);
}

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058560);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_800585BC);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058664);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_80058844);

INCLUDE_ASM("asm/nonmatchings/graphics/model_renderer", func_800589F0);
