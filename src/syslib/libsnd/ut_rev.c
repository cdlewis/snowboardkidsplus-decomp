#include "common.h"
#include "reverb_private.h"

#ifdef NON_MATCHING
extern int SpuSetReverb(int);
/* Best observed instruction-sequence similarity: 74.36% (29/39 words). */
s16 SsUtSetReverbType(s16 type) {
    int clear = 0;
    if (type < 0) {
        clear = 1;
        type = -type;
    }
    if (type < 0 || type >= 10)
        return -1;
    D_8013CEC8.mask = 1;
    D_8013CEC8.mode = clear ? (s16)(type | 0x100) : type;
    if (type == 0)
        SpuSetReverb(0);
    SpuSetReverbModeParam(&D_8013CEC8);
    return type;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_rev", SsUtSetReverbType);
#endif

s16 SsUtGetReverbType(void) { return D_8013CEC8.mode; }
