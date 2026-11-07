#pragma once

#include "common.h"

typedef struct DrawModeSpritePacket {
    u32 tag;
    u16 tpage, drawModeCode;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 w, h;
} DrawModeSpritePacket;

typedef char DrawModeSpritePacketSizeCheck[(sizeof(DrawModeSpritePacket) == 0x18) ? 1 : -1];
