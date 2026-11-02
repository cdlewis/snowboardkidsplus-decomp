#include "libspu_private.h"

s32 SpuSetNoiseClock(s32 clock) {
    s32 clamped;
    u16 control;

    if (clock < 0) clamped = 0;
    else if (clock > 0x3F) clamped = 0x3F;
    else clamped = clock;
    control = _spu_RXX->rxx.spucnt;
    control &= ~0x3F00;
    control |= (clamped & 0x3F) << 8;
    _spu_RXX->rxx.spucnt = control;
    return clamped;
}
