#include "common.h"

#ifdef NON_MATCHING
typedef struct { s16 m[3][3]; s32 t[3]; } MATRIX;
/* Best observed instruction-sequence similarity: 72.22% (52/72 words). */
int gte_scale_matrix(MATRIX *m, s16 x, s16 y, s16 z) {
    int changed = 0;
    int a, b, c;
    if (x != 0x1000) {
        a = m->m[0][0] * x; b = m->m[1][0] * x; c = m->m[2][0] * x;
        m->m[0][0] = a >> 12; m->m[1][0] = b >> 12; m->m[2][0] = c >> 12;
        changed = 1;
    }
    if (y != 0x1000) {
        a = m->m[0][1] * y; b = m->m[1][1] * y; c = m->m[2][1] * y;
        m->m[0][1] = a >> 12; m->m[1][1] = b >> 12; m->m[2][1] = c >> 12;
        changed = 1;
    }
    if (z != 0x1000) {
        a = m->m[0][2] * z; b = m->m[1][2] * z; c = m->m[2][2] * z;
        m->m[0][2] = a >> 12; m->m[1][2] = b >> 12; m->m[2][2] = c >> 12;
        changed = 1;
    }
    return changed;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_120", gte_scale_matrix);
#endif
