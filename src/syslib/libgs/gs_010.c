#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 PSDIDX, PSDOFSX[], PSDOFSY[];
extern u32 PSDCNT;
extern DISPENV GsDISPENV;
extern void PutDispEnv(DISPENV *), SetDispMask(int);
extern void GsSetDrawBuffClip(void), GsSetDrawBuffOffset(void);
/* Best observed instruction-sequence similarity: 70.45% (31/44 words,
 * including target padding). */
void GsSwapDispBuff(void) {
    GsDISPENV.disp.x = PSDOFSX[PSDIDX];
    GsDISPENV.disp.y = PSDOFSY[PSDIDX];
    PutDispEnv(&GsDISPENV);
    SetDispMask(1);
    if (++PSDCNT == 0)
        PSDCNT = 1;
    PSDIDX = !PSDIDX;
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_010", GsSwapDispBuff);
#endif
