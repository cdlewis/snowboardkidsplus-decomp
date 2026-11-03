#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 PSDIDX, PSDOFSX[], PSDOFSY[];
extern RECT CLIP2;
extern DRAWENV GsDRAWENV;
extern DRAWENV *PutDrawEnv(DRAWENV *);
/* Best observed instruction-sequence similarity: 50.00% (16/32 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
void GsSetDrawBuffClip(void) {
    s16 x = PSDOFSX[PSDIDX];
    s16 y = PSDOFSY[PSDIDX];
    RECT *src = &CLIP2;
    RECT *dst = &GsDRAWENV.clip;
    dst->h = src->h;
    dst->w = src->w;
    dst->x = src->x + x;
    dst->y = src->y + y;
    PutDrawEnv(&GsDRAWENV);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_003", GsSetDrawBuffClip);
#endif
