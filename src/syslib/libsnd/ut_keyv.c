#include "common.h"

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_keyv", SsUtKeyOnV);

#ifdef NON_MATCHING
extern s32 D_8013BF04;
extern u16 D_8013CFDA;
extern void _SsVmKeyOffNow(int);
/* Best observed instruction-sequence similarity: 59.38% (19/32 words, including padding). */
s16 SsUtKeyOffV(s16 voice) {
    if (D_8013BF04 == 1)
        return -1;
    D_8013BF04 = 1;
    if (voice < 0 || voice >= 24) {
        D_8013BF04 = 0;
        return -1;
    }
    D_8013CFDA = voice;
    _SsVmKeyOffNow(0);
    D_8013BF04 = 0;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_keyv", SsUtKeyOffV);
#endif
