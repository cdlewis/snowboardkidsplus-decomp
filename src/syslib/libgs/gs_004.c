#include "common.h"

#ifdef NON_MATCHING
#include "psx/libgpu.h"
extern s16 PSDGPU, PSDIDX, GsORGOFSX, GsORGOFSY;
extern s16 PSDOFSX[], PSDOFSY[];
extern DRAWENV GsDRAWENV;
extern DRAWENV *PutDrawEnv(DRAWENV *);
extern void SetGeomOffset(int, int);
/* Best observed instruction-sequence similarity: 65.00% (39/60 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
void GsSetOffset(int x, int y) {
    int ox, oy;
    if (PSDGPU) {
        GsORGOFSY = 0;
        GsORGOFSX = 0;
        GsDRAWENV.ofs[0] = x + PSDOFSX[PSDIDX];
        GsDRAWENV.ofs[1] = y + PSDOFSY[PSDIDX];
        PutDrawEnv(&GsDRAWENV);
    } else {
        ox = x + PSDOFSX[PSDIDX ? 0 : 1];
        oy = y + PSDOFSY[PSDIDX ? 0 : 1];
        SetGeomOffset(ox, oy);
        GsORGOFSX = ox;
        GsORGOFSY = oy;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_004", GsSetOffset);
#endif
