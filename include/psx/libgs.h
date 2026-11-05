#pragma once

#include "decomp/types.h"

#ifdef GS_PACKET_ABSOLUTE_ADDRESSING
extern u32 GsOUT_PACKET_P __attribute__((section(".data")));
#else
extern u32 GsOUT_PACKET_P;
#endif

typedef struct GsIMAGE {
    u32 pmode;
    s16 px, py;
    u16 pw, ph;
    u32 *pixel;
    s16 cx, cy;
    u16 cw, ch;
    u32 *clut;
} GsIMAGE;

/* Input begins at the TIM flags word, immediately after the magic word. */
void GsGetTimInfo(u32 *tim, GsIMAGE *image);
