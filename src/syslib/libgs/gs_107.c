#include "common.h"

typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX;
extern MATRIX D_8013F820;
extern void SetColorMatrix(MATRIX *m);

INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_107", GsSetFlatLight);

void gte_set_lc(MATRIX *m) {
    D_8013F820 = *m;
    SetColorMatrix(m);
}

void gte_read_lc(MATRIX *m) { *m = D_8013F820; }
