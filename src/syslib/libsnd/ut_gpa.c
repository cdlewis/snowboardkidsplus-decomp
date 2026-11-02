/* Adapted from PsyZ decomp; see THIRD_PARTY.md. */
#include "libsnd_private.h"

extern u8 D_8013CFE0[];
extern ProgAtr *volatile D_8013CFAC;
extern int _SsVmVSetUp(s16, s16);

s16 SsUtGetProgAtr(s16 vab, s16 program, ProgAtr *attr) {
    if (D_8013CFE0[vab] == 1) {
        _SsVmVSetUp(vab, program);
        attr->tones = D_8013CFAC[program].tones;
        attr->mvol = D_8013CFAC[program].mvol;
        attr->prior = D_8013CFAC[program].prior;
        attr->mode = D_8013CFAC[program].mode;
        attr->mpan = D_8013CFAC[program].mpan;
        attr->attr = D_8013CFAC[program].attr;
        return 0;
    }
    return -1;
}
