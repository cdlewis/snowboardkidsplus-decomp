#include "common.h"

#ifdef NON_MATCHING
extern s32 HWD0, VWD0, gsProjectionShift, GsLIGHT_MODE, gsProjectionLimit;
extern s16 POSITION[2];
extern void GsSetDrawBuffOffset(void);

/* Best observed instruction-sequence similarity: 84.38% (27/32 words,
 * -mno-split-addresses -fno-delayed-branch; target includes three padding words). */
void GsInit3D(void) {
    POSITION[0] = HWD0 / 2;
    POSITION[1] = VWD0 / 2;
    GsSetDrawBuffOffset();
    gsProjectionShift = 10;
    GsLIGHT_MODE = 0;
    gsProjectionLimit = 0x3FFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libgs/gs_104", GsInit3D);
#endif
