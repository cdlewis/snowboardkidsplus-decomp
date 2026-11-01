#include "common.h"

#ifdef NON_MATCHING
extern s16 D_8013F766;
extern s16 D_8013F6C8[2], D_8013F6CC[2], D_8013F6D0[2], D_8013F6D4[2];

/* Best observed instruction-sequence similarity: 87.50% (28/32 words,
 * -mno-split-addresses -fno-delayed-branch; target includes two padding words). */
void GsDefDispBuff2(int x0, int y0, int x1, int y1) {
    D_8013F6C8[0] = x0;
    D_8013F6C8[1] = x1;
    D_8013F6CC[0] = y0;
    D_8013F6CC[1] = y1;
    if (D_8013F766) {
        D_8013F6D0[0] = 0;
        D_8013F6D0[1] = 0;
        D_8013F6D4[0] = 0;
        D_8013F6D4[1] = 0;
    } else {
        D_8013F6D0[0] = x0;
        D_8013F6D0[1] = x1;
        D_8013F6D4[0] = y0;
        D_8013F6D4[1] = y1;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_127", GsDefDispBuff2);
#endif
