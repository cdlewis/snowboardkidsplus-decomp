#include "common.h"
#include "vm_g.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 80.56% (116/144 words; target includes padding). */
#include "libsnd_private.h"
extern u8 _svm_vab_used[];

extern VagAtr *volatile _svm_tn;
extern int _SsVmVSetUp(s16, s16);
s16 SsUtGetVagAtr(s16 vab, s16 program, s16 tone, VagAtr *a) {
    s16 i;
    VagAtr *p;
    if (_svm_vab_used[vab] == 1) {
    _SsVmVSetUp(vab, program);
    i = tone + _svm_cur.fake_program * 16;
    a->prior = _svm_tn[i].prior;
    a->mode = _svm_tn[i].mode;
    a->vol = _svm_tn[i].vol;
    a->pan = _svm_tn[i].pan;
    a->center = _svm_tn[i].center;
    a->shift = _svm_tn[i].shift;
    a->max = _svm_tn[i].max;
    a->min = _svm_tn[i].min;
    a->vibW = _svm_tn[i].vibW;
    a->vibT = _svm_tn[i].vibT;
    a->porW = _svm_tn[i].porW;
    a->porT = _svm_tn[i].porT;
    a->pbmin = _svm_tn[i].pbmin;
    a->pbmax = _svm_tn[i].pbmax;
    p = &_svm_tn[i];
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
