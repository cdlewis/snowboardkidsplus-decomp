#pragma once

#include "decomp/types.h"
#include "psx/libgte.h"
#include "psx/sprite.h"
#include "psx/libgpu.h"

typedef struct { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct { VECTOR scale; SVECTOR rotate; VECTOR trans; } GsCOORD2PARAM;
typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX;
typedef struct GsCOORDINATE2 {
    u32 flg;
    MATRIX coord, workm;
    GsCOORD2PARAM *param;
    struct GsCOORDINATE2 *super, *sub;
} GsCOORDINATE2;

typedef struct GsDOBJ2 {
    u32 attribute;
    GsCOORDINATE2 *coord2;
    u32 *tmd;
    u32 id;
} GsDOBJ2;

void GsGetLw(GsCOORDINATE2 *coordinate, MATRIX *out);
void GsGetLs(GsCOORDINATE2 *coordinate, MATRIX *out);
void GsSetLightMatrix(MATRIX *matrix);
void GsSetLsMatrix(MATRIX *matrix);
typedef struct GsRVIEW2 {
    s32 vpx, vpy, vpz;
    s32 vrx, vry, vrz;
    s32 rz;
    GsCOORDINATE2 *super;
} GsRVIEW2;

void GsSetProjection(s32 projection);
int GsSetRefView2L(GsRVIEW2 *view);

void GsMapModelingData(u32 *data);
void GsLinkObject4(u32 *model, GsDOBJ2 *object, s32 index);

void GsInitCoordinate2(GsCOORDINATE2 *super, GsCOORDINATE2 *coord);
void GsGetLs(GsCOORDINATE2 *coord, MATRIX *matrix);

#ifdef GS_PACKET_ABSOLUTE_ADDRESSING
extern u32 GsOUT_PACKET_P __attribute__((section(".data")));
#else
extern u32 GsOUT_PACKET_P;
#endif

typedef struct GsOT {
    u32 length;
    u32 *org;
    u32 offset, point;
    u32 *tag;
} GsOT;

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
void GsSortObject4(GsDOBJ2 *object, GsOT *ot, s32 shift, u32 *scratch);
void GsSortPoly(void *packet, GsOT *ot, u16 depth);
void GsSortSprite(GsSPRITE *sprite, GsOT *ot, u16 depth);
u32 *GsGetWorkBase(void);
void func_800BF2C8(u32 value);

MATRIX *RotMatrix(SVECTOR *rotation, MATRIX *matrix);
VECTOR *ApplyMatrix(MATRIX *matrix, SVECTOR *vector, VECTOR *result);
MATRIX *RotMatrixZYX(SVECTOR *rotation, MATRIX *matrix);

MATRIX *RotMatrix(SVECTOR *rotation, MATRIX *matrix);
MATRIX *RotMatrixYXZ(SVECTOR *rotation, MATRIX *matrix);

MATRIX *ScaleMatrix(MATRIX *matrix, VECTOR *scale);

VECTOR *ApplyMatrixLV(MATRIX *matrix, VECTOR *in, VECTOR *out);
void GsInitGraph(s32 width, s32 height, s32 mode, s32 interlace, s32 display);
void GsDefDispBuff(s32 x0, s32 y0, s32 x1, s32 y1);
void GsInit3D(void);

extern DISPENV GsDISPENV;
void GsClearOt(u16 offset, u16 point, GsOT *ot);
void GsDefDispBuff2(s32 x0, s32 y0, s32 x1, s32 y1);
void GsDrawOt(GsOT *ot);
s32 GsGetActiveBuff(void);
void GsInitGraph2(s32 width, s32 height, s32 mode, s32 interlace, s32 display);
void GsSetClip(RECT *clip);
void GsSetOffset(s32 x, s32 y);
void GsSortClear(s32 r, s32 g, s32 b, GsOT *ot);
void GsSwapDispBuff(void);
