#pragma once

#include "decomp/types.h"

#define GsALON (1 << 30)

/* PSY-Q sprite layout, verified against the initializer and its consumers. */
typedef struct GsSPRITE {
    u32 attribute;
    s16 x, y;
    u16 w, h, tpage;
    u8 u, v;
    u16 cx, cy;
    u8 r, g, b, pad;
    s16 mx, my, scalex, scaley;
    s32 rotate;
} GsSPRITE;
