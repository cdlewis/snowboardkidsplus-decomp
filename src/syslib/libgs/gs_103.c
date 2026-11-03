#include "common.h"

extern s16 PSDGPU;
extern s16 PSDOFSX[2], PSDOFSY[2], PSDBASEX[2], PSDBASEY[2];
extern void GsSetDrawBuffClip(void), GsSetDrawBuffOffset(void);

void GsDefDispBuff(int x0, int y0, int x1, int y1) {
    PSDOFSX[0] = x0;
    PSDOFSX[1] = x1;
    PSDOFSY[0] = y0;
    PSDOFSY[1] = y1;
    if (PSDGPU) {
        PSDBASEX[0] = 0;
        PSDBASEX[1] = 0;
        PSDBASEY[0] = 0;
        PSDBASEY[1] = 0;
    } else {
        PSDBASEX[0] = x0;
        PSDBASEX[1] = x1;
        PSDBASEY[0] = y0;
        PSDBASEY[1] = y1;
    }
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}
