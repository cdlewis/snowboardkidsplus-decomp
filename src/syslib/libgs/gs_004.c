#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 D_8013F766, D_8013F764, D_8013F754, D_8013F756;
extern s16 D_8013F6C8[], D_8013F6CC[];
extern DRAWENV D_8013F6E0;
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern void SetGeomOffset(int, int);
/* Best observed instruction-sequence similarity: 65.00% (39/60 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
void GsSetOffset(int x, int y) {
    int ox, oy;
    if (D_8013F766) {
        D_8013F756 = 0;
        D_8013F754 = 0;
        D_8013F6E0.ofs[0] = x + D_8013F6C8[D_8013F764];
        D_8013F6E0.ofs[1] = y + D_8013F6CC[D_8013F764];
        PutDrawEnv(&D_8013F6E0);
    } else {
        ox = x + D_8013F6C8[D_8013F764 ? 0 : 1];
        oy = y + D_8013F6CC[D_8013F764 ? 0 : 1];
        SetGeomOffset(ox, oy);
        D_8013F754 = ox;
        D_8013F756 = oy;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_004", GsSetOffset);
#endif
