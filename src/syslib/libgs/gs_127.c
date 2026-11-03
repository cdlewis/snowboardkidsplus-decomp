#include "common.h"

extern s16 PSDGPU;
extern s16 PSDOFSX[2], PSDOFSY[2], PSDBASEX[2], PSDBASEY[2];

void GsDefDispBuff2(int x0, int y0, int x1, int y1) {
    s16 *p;
    int flags = PSDGPU;
    s16 *displayY;

    p = PSDOFSX;
    *p++ = x0;
    *p = x1;
    displayY = PSDOFSY;
    p = displayY + 1;
    *displayY = y0;
    *p = y1;
    if (flags) {
        p = PSDBASEX;
        *p++ = 0;
        *p = 0;
        p = PSDBASEY;
        *p++ = 0;
        *p = 0;
    } else {
        p = PSDBASEX;
        *p++ = x0;
        *p = x1;
        p = PSDBASEY;
        *p++ = y0;
        *p = y1;
    }
}
