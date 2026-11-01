#include "psx/libgpu.h"

void SetSemiTrans(void *primitive, s32 enable) {
    P_TAG *p = primitive;

    if (enable) {
        p->code |= 2;
    } else {
        p->code &= ~2;
    }
}
