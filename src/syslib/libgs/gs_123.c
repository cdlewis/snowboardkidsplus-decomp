#include "common.h"

#ifdef NON_MATCHING
typedef struct { s16 m[3][3]; s16 pad; s32 t[3]; } Matrix;
extern Matrix D_8013F870;
/* Best observed instruction-sequence similarity: 11.54% (6/52 words;
 * GCC 2.8.0, -mno-split-addresses; original uses a jump table). */
void Gssub_make_matrix(Matrix *m, int s, int c, s8 axis) {
    *m = D_8013F870;
    if (axis == 'X' || axis == 'x') {
        m->m[1][1] = c;
        m->m[2][2] = c;
        m->m[1][2] = -s;
        m->m[2][1] = s;
    } else if (axis == 'Y' || axis == 'y') {
        m->m[0][0] = c;
        m->m[2][2] = c;
        m->m[0][2] = s;
        m->m[2][0] = -s;
    } else if (axis == 'Z' || axis == 'z') {
        m->m[0][0] = c;
        m->m[1][1] = c;
        m->m[0][1] = -s;
        m->m[1][0] = s;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_123", Gssub_make_matrix);
#endif
