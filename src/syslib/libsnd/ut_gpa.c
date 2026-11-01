#include "common.h"

#ifdef NON_MATCHING
typedef struct {
    u8 tones, mvol, prior, mode, mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1, reserved2;
} ProgAtr;
extern u8 D_8013CFE0[];
extern ProgAtr *volatile D_8013CFAC;
extern int _SsVmVSetUp(s16, s16);
/* Best observed instruction-sequence similarity: 86.76% (59/68 words,
 * including target padding). */
s16 SsUtGetProgAtr(s16 vab, s16 program, ProgAtr *attr) {
    if (D_8013CFE0[vab] != 1)
        return -1;
    _SsVmVSetUp(vab, program);
    attr->tones = D_8013CFAC[program].tones;
    attr->mvol = D_8013CFAC[program].mvol;
    attr->prior = D_8013CFAC[program].prior;
    attr->mode = D_8013CFAC[program].mode;
    attr->mpan = D_8013CFAC[program].mpan;
    attr->attr = D_8013CFAC[program].attr;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_gpa", SsUtGetProgAtr);
#endif
