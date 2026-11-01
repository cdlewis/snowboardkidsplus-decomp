#include "psx/libgpu.h"

void SetDrawTPage(DR_TPAGE *p, s32 dfe, s32 dtd, s32 tpage) {
    setlen(p, 1);
    p->code[0] = 0xe1000000 | (dtd ? 0x200 : 0) |
                 (dfe ? 0x400 : 0) | (tpage & 0x9ff);
}
