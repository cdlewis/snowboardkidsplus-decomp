#include "common.h"

extern s16 PSDIDX;

int GsGetActiveBuff(void) { return PSDIDX; }

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 PSDGPU, GsORGOFSX, GsORGOFSY;
extern s16 PSDOFSX[], PSDOFSY[], POSITION[];
extern DRAWENV GsDRAWENV;
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern void SetGeomOffset(int, int);
/* Best observed instruction-sequence similarity: 70.59% (48/68 words;
 * GCC 2.7.2; target includes padding). */
void GsSetDrawBuffOffset(void) {
    int x, y;
    if (PSDGPU) {
        GsORGOFSY = 0;
        GsORGOFSX = 0;
        GsDRAWENV.ofs[0] = POSITION[0] + PSDOFSX[PSDIDX];
        GsDRAWENV.ofs[1] = POSITION[1] + PSDOFSY[PSDIDX];
        PutDrawEnv(&GsDRAWENV);
    } else {
        x = POSITION[0] + PSDOFSX[PSDIDX ? 0 : 1];
        y = POSITION[1] + PSDOFSY[PSDIDX ? 0 : 1];
        SetGeomOffset(x, y);
        GsORGOFSX = x;
        GsORGOFSY = y;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_002", GsSetDrawBuffOffset);
#endif
