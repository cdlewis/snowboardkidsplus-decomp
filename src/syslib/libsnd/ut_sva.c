#include "common.h"
#include "vm_g.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 87.07% (101/116 words; target includes padding). */
#include "libsnd_private.h"
extern u8 _svm_vab_used[];

extern VagAtr *volatile _svm_tn;
extern int _SsVmVSetUp(s16, s16);
s16 SsUtSetVagAtr(s16 vab, s16 program, s16 tone, VagAtr *a) {
    s16 i;
    VagAtr *p;
    if (_svm_vab_used[vab] == 1) {
    _SsVmVSetUp(vab, program);
    i = tone + _svm_cur.fake_program * 16;
    _svm_tn[i].prior = a->prior;
    _svm_tn[i].mode = a->mode;
    _svm_tn[i].vol = a->vol;
    _svm_tn[i].pan = a->pan;
    _svm_tn[i].center = a->center;
    _svm_tn[i].shift = a->shift;
    _svm_tn[i].max = a->max;
    _svm_tn[i].min = a->min;
    _svm_tn[i].vibW = a->vibW;
    _svm_tn[i].vibT = a->vibT;
    _svm_tn[i].porW = a->porW;
    _svm_tn[i].porT = a->porT;
    _svm_tn[i].pbmin = a->pbmin;
    _svm_tn[i].pbmax = a->pbmax;
    p = &_svm_tn[i];
    p->adsr1 = a->adsr1;
    p->adsr2 = a->adsr2;
    p->prog = a->prog;
    p->vag = a->vag;
    return 0;
    }
    return -1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_sva", SsUtSetVagAtr);
#endif
