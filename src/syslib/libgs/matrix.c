#include "common.h"

typedef struct { s32 vx, vy, vz, pad; } VECTOR;
typedef struct { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct { VECTOR scale; SVECTOR rotate; VECTOR trans; } GsCOORD2PARAM;
typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX;
typedef struct GsCOORDINATE2 {
    u32 flg;
    MATRIX coord, workm;
    GsCOORD2PARAM *param;
    struct GsCOORDINATE2 *super, *sub;
} GsCOORDINATE2;

extern MATRIX D_8013F870;
extern MATRIX D_8013F800;
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void SetLightMatrix(MATRIX *m);
extern void PushMatrix(void);
extern void PopMatrix(void);
extern MATRIX *MulMatrix(MATRIX *left, MATRIX *right);
extern MATRIX *MulMatrix2(MATRIX *left, MATRIX *right);
extern MATRIX *MulMatrix0(MATRIX *left, MATRIX *right, MATRIX *out);
extern VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *in, VECTOR *out);
extern int printf(const char *, ...);

const char D_80012AB0[36] __attribute__((section(".rodata.gs_matrix_print"), aligned(4))) =
    "(%d %d %d) (%d %d %d) (%d %d %d)\n";
const char D_80012AD4[12] __attribute__((section(".rodata.gs_matrix_print"), aligned(4))) =
    "(%d %d %d)\n";

void GsInitCoordinate2(GsCOORDINATE2 *super, GsCOORDINATE2 *coord) {
    coord->coord = D_8013F870;
    coord->super = super;
    coord->flg = 0;
    if ((u32)super >= 2) {
        coord->super->sub = coord;
    }
}

void GsInitCoord2param(GsCOORD2PARAM *p) {
    p->rotate.vx = p->rotate.vy = p->rotate.vz = 0;
    p->scale.vx = p->scale.vy = p->scale.vz = 0x1000;
    p->trans.vx = p->trans.vy = p->trans.vz = 0;
}

void GsSetLsMatrix(MATRIX *m) {
    SetRotMatrix(m);
    SetTransMatrix(m);
}

void GsSetLightMatrix(MATRIX *m) {
    MATRIX light = D_8013F800;

    PushMatrix();
    MulMatrix(&light, m);
    PopMatrix();
    SetLightMatrix(&light);
}

void GsSetLightMatrix2(MATRIX *m) {
    SetLightMatrix(MulMatrix2(&D_8013F800, m));
}

void GsMulCoord0(MATRIX *a, MATRIX *b, MATRIX *out) {
    ApplyMatrixLV(a, (VECTOR *)b->t, (VECTOR *)out->t);
    MulMatrix0(a, b, out);
    out->t[0] += a->t[0];
    out->t[1] += a->t[1];
    out->t[2] += a->t[2];
}

void GsMulCoord2(MATRIX *a, MATRIX *b) {
    VECTOR translation;

    ApplyMatrixLV(a, (VECTOR *)b->t, &translation);
    MulMatrix2(a, b);
    b->t[0] = translation.vx + a->t[0];
    b->t[1] = translation.vy + a->t[1];
    b->t[2] = translation.vz + a->t[2];
}

void GsMulCoord3(MATRIX *a, MATRIX *b) {
    VECTOR translation;

    ApplyMatrixLV(a, (VECTOR *)b->t, &translation);
    MulMatrix(a, b);
    a->t[0] = translation.vx + a->t[0];
    a->t[1] = translation.vy + a->t[1];
    a->t[2] = translation.vz + a->t[2];
}

void print_matrix(MATRIX *m) {
    printf(D_80012AB0, m->m[0][0], m->m[0][1], m->m[0][2],
           m->m[1][0], m->m[1][1], m->m[1][2],
           m->m[2][0], m->m[2][1], m->m[2][2]);
}

void print_vector(MATRIX *m) {
    printf(D_80012AD4, m->t[0], m->t[1], m->t[2]);
}
