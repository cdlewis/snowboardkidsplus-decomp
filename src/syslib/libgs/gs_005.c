#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 D_8013F764, D_8013F6C8[], D_8013F6CC[];
extern DRAWENV D_8013F6E0;
extern DRAWENV *PutDrawEnv(DRAWENV *);
/* Best observed instruction-sequence similarity: 62.50% (20/32 words,
 * -mno-split-addresses; target includes padding). */
void GsSetClip(RECT *rect) {
    D_8013F6E0.clip.x = rect->x + D_8013F6C8[D_8013F764];
    D_8013F6E0.clip.y = rect->y + D_8013F6CC[D_8013F764];
    D_8013F6E0.clip.w = rect->w;
    D_8013F6E0.clip.h = rect->h;
    PutDrawEnv(&D_8013F6E0);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_005", GsSetClip);
#endif
