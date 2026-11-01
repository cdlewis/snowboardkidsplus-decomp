#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 80.56% (116/144 words; target includes padding). */
#include "libsnd_private.h"
extern u8 D_8013CFE0[];
extern s8 D_8013CFC7;
extern VagAtr *volatile D_8013CFB8;
extern int _SsVmVSetUp(s16, s16);
s16 SsUtGetVagAtr(s16 vab, s16 program, s16 tone, VagAtr *a) {
    s16 i;
    VagAtr *p;
    if (D_8013CFE0[vab] == 1) {
    _SsVmVSetUp(vab, program);
    i = tone + D_8013CFC7 * 16;
    a->prior = D_8013CFB8[i].prior;
    a->mode = D_8013CFB8[i].mode;
    a->vol = D_8013CFB8[i].vol;
    a->pan = D_8013CFB8[i].pan;
    a->center = D_8013CFB8[i].center;
    a->shift = D_8013CFB8[i].shift;
    a->max = D_8013CFB8[i].max;
    a->min = D_8013CFB8[i].min;
    a->vibW = D_8013CFB8[i].vibW;
    a->vibT = D_8013CFB8[i].vibT;
    a->porW = D_8013CFB8[i].porW;
    a->porT = D_8013CFB8[i].porT;
    a->pbmin = D_8013CFB8[i].pbmin;
    a->pbmax = D_8013CFB8[i].pbmax;
    p = &D_8013CFB8[i];
    a->adsr1 = p->adsr1;
    a->adsr2 = p->adsr2;
    a->prog = p->prog;
    a->vag = p->vag;
    return 0;
    }
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_gva", SsUtGetVagAtr);
#endif
