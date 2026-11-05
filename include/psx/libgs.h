#pragma once

#include "decomp/types.h"
#include "psx/libgte.h"

typedef struct GsOT {
    u32 length;
    u32 *org;
    u32 offset, point;
    u32 *tag;
} GsOT;
typedef struct { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct { VECTOR scale; SVECTOR rotate; VECTOR trans; } GsCOORD2PARAM;
typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX;
typedef struct GsCOORDINATE2 {
    u32 flg;
    MATRIX coord, workm;
    GsCOORD2PARAM *param;
    struct GsCOORDINATE2 *super, *sub;
} GsCOORDINATE2;

#ifdef GS_PACKET_ABSOLUTE_ADDRESSING
extern u32 GsOUT_PACKET_P __attribute__((section(".data")));
#else
extern u32 GsOUT_PACKET_P;
#endif

typedef struct GsIMAGE {
    u32 pmode;
    s16 px, py;
    u16 pw, ph;
    u32 *pixel;
    s16 cx, cy;
    u16 cw, ch;
    u32 *clut;
} GsIMAGE;

/* Input begins at the TIM flags word, immediately after the magic word. */
void GsGetTimInfo(u32 *tim, GsIMAGE *image);
void GsSortPoly(void *packet, GsOT *ot, u16 depth);
