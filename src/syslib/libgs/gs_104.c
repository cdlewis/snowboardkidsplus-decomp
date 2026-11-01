#include "common.h"

#ifdef NON_MATCHING
extern s32 D_8013F7E8, D_8013F7EC, D_8013F7F8, D_8013F7F4, D_8013F7F0;
extern s16 D_8013F6D8[2];
extern void GsSetDrawBuffOffset(void);

/* Best observed instruction-sequence similarity: 84.38% (27/32 words,
 * -mno-split-addresses -fno-delayed-branch; target includes three padding words). */
void GsInit3D(void) {
    D_8013F6D8[0] = D_8013F7E8 / 2;
    D_8013F6D8[1] = D_8013F7EC / 2;
    GsSetDrawBuffOffset();
    D_8013F7F8 = 10;
    D_8013F7F4 = 0;
    D_8013F7F0 = 0x3FFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_104", GsInit3D);
#endif
