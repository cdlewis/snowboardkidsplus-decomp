#include "common.h"

extern s16 D_8013F766;
extern s16 D_8013F6C8[2], D_8013F6CC[2], D_8013F6D0[2], D_8013F6D4[2];
extern void GsSetDrawBuffClip(void), GsSetDrawBuffOffset(void);

void GsDefDispBuff(int x0, int y0, int x1, int y1) {
    D_8013F6C8[0] = x0;
    D_8013F6C8[1] = x1;
    D_8013F6CC[0] = y0;
    D_8013F6CC[1] = y1;
    if (D_8013F766) {
        D_8013F6D0[0] = 0;
        D_8013F6D0[1] = 0;
        D_8013F6D4[0] = 0;
        D_8013F6D4[1] = 0;
    } else {
        D_8013F6D0[0] = x0;
        D_8013F6D0[1] = x1;
        D_8013F6D4[0] = y0;
        D_8013F6D4[1] = y1;
    }
    GsSetDrawBuffClip();
    GsSetDrawBuffOffset();
}
