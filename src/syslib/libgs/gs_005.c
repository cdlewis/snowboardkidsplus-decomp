#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 PSDIDX, PSDOFSX[], PSDOFSY[];
extern DRAWENV GsDRAWENV;
extern DRAWENV *PutDrawEnv(DRAWENV *);
/* Best observed instruction-sequence similarity: 62.50% (20/32 words,
 * -mno-split-addresses; target includes padding). */
void GsSetClip(RECT *rect) {
    GsDRAWENV.clip.x = rect->x + PSDOFSX[PSDIDX];
    GsDRAWENV.clip.y = rect->y + PSDOFSY[PSDIDX];
    GsDRAWENV.clip.w = rect->w;
    GsDRAWENV.clip.h = rect->h;
    PutDrawEnv(&GsDRAWENV);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_005", GsSetClip);
#endif
