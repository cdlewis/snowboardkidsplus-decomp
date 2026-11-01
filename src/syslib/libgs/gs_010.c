#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 D_8013F764, D_8013F6C8[], D_8013F6CC[];
extern u32 D_8013F760;
extern DISPENV D_8013F740;
extern void PutDispEnv(DISPENV *), SetDispMask(int);
extern void GsSetDrawBuffClip(void), GsSetDrawBuffOffset(void);
/* Best observed instruction-sequence similarity: 70.45% (31/44 words,
 * including target padding). */
void GsSwapDispBuff(void) {
    D_8013F740.disp.x = D_8013F6C8[D_8013F764];
    D_8013F740.disp.y = D_8013F6CC[D_8013F764];
    PutDispEnv(&D_8013F740);
    SetDispMask(1);
    if (++D_8013F760 == 0)
        D_8013F760 = 1;
    D_8013F764 = !D_8013F764;
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_010", GsSwapDispBuff);
#endif
