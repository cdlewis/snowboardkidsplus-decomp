#include "common.h"

extern s16 D_8013F766;
extern s16 D_8013F6C8[2], D_8013F6CC[2], D_8013F6D0[2], D_8013F6D4[2];

void GsDefDispBuff2(int x0, int y0, int x1, int y1) {
    s16 *p;
    int flags = D_8013F766;
    s16 *displayY;

    p = D_8013F6C8;
    *p++ = x0;
    *p = x1;
    displayY = D_8013F6CC;
    p = displayY + 1;
    *displayY = y0;
    *p = y1;
    if (flags) {
        p = D_8013F6D0;
        *p++ = 0;
        *p = 0;
        p = D_8013F6D4;
        *p++ = 0;
        *p = 0;
    } else {
        p = D_8013F6D0;
        *p++ = x0;
        *p = x1;
        p = D_8013F6D4;
        *p++ = y0;
        *p = y1;
    }
}
