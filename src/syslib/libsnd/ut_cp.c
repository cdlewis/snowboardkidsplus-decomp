#include "common.h"

#ifdef NON_MATCHING
extern u8 D_8013C7D6[], D_8013CFD6[], D_8013CCD8[], D_8013CCFC[];
extern int _SsVmVSetUp(s16, s16);
extern u16 note2pitch2(u16, u16);
/* Best observed instruction-sequence similarity: 52.50% (42/80 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
int SsUtChangePitch(s16 voice, s16 vab, s16 program, s16 tone, u16 note, u16 fine) {
    u8 *v = D_8013C7D6 + voice * 54;
    if ((u16)voice >= 24 || *(s16 *)(v + 10) != vab ||
        *(s16 *)(v + 6) != program || *(s16 *)v != tone)
        return -1;
    _SsVmVSetUp(vab, program);
    *(s16 *)D_8013CFD6 = 33;
    *(s16 *)(D_8013CFD6 + 4) = voice;
    D_8013CFD6[-10] = v[8];
    *(u16 *)(D_8013CCFC + voice * 16) = note2pitch2(note, fine);
    D_8013CCD8[voice] |= 4;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_cp", SsUtChangePitch);
#endif
