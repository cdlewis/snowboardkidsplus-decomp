#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 D_8013F764, D_8013F6C8[], D_8013F6CC[];
extern RECT D_8013F758;
extern DRAWENV D_8013F6E0;
extern DRAWENV *PutDrawEnv(DRAWENV *);
/* Best observed instruction-sequence similarity: 50.00% (16/32 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
void GsSetDrawBuffClip(void) {
    s16 x = D_8013F6C8[D_8013F764];
    s16 y = D_8013F6CC[D_8013F764];
    RECT *src = &D_8013F758;
    RECT *dst = &D_8013F6E0.clip;
    dst->h = src->h;
    dst->w = src->w;
    dst->x = src->x + x;
    dst->y = src->y + y;
    PutDrawEnv(&D_8013F6E0);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_003", GsSetDrawBuffClip);
#endif
