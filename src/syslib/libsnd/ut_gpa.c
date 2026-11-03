#include "libsnd_private.h"

extern u8 _svm_vab_used[];
extern ProgAtr *volatile _svm_pg;
extern int _SsVmVSetUp(s16, s16);

s16 SsUtGetProgAtr(s16 vab, s16 program, ProgAtr *attr) {
    if (_svm_vab_used[vab] == 1) {
        _SsVmVSetUp(vab, program);
        attr->tones = _svm_pg[program].tones;
        attr->mvol = _svm_pg[program].mvol;
        attr->prior = _svm_pg[program].prior;
        attr->mode = _svm_pg[program].mode;
        attr->mpan = _svm_pg[program].mpan;
        attr->attr = _svm_pg[program].attr;
        return 0;
    }
    return -1;
}
