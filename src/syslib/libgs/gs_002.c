#include "common.h"

extern s16 D_8013F764;

int GsGetActiveBuff(void) { return D_8013F764; }

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 D_8013F766, D_8013F754, D_8013F756;
extern s16 D_8013F6C8[], D_8013F6CC[], D_8013F6D8[];
extern DRAWENV D_8013F6E0;
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern void SetGeomOffset(int, int);
/* Best observed instruction-sequence similarity: 70.59% (48/68 words;
 * GCC 2.7.2; target includes padding). */
void GsSetDrawBuffOffset(void) {
    int x, y;
    if (D_8013F766) {
        D_8013F756 = 0;
        D_8013F754 = 0;
        D_8013F6E0.ofs[0] = D_8013F6D8[0] + D_8013F6C8[D_8013F764];
        D_8013F6E0.ofs[1] = D_8013F6D8[1] + D_8013F6CC[D_8013F764];
        PutDrawEnv(&D_8013F6E0);
    } else {
        x = D_8013F6D8[0] + D_8013F6C8[D_8013F764 ? 0 : 1];
        y = D_8013F6D8[1] + D_8013F6CC[D_8013F764 ? 0 : 1];
        SetGeomOffset(x, y);
        D_8013F754 = x;
        D_8013F756 = y;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_002", GsSetDrawBuffOffset);
#endif
