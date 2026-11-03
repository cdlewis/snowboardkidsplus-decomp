#include "libsnd_private.h"

extern ProgAtr *_svm_pg;
extern int _SsVmVSetUp(s16 vab, s16 program);

s16 _SsVmSetProgVol(s16 vab, s16 program, s16 value) {
    if (_SsVmVSetUp(vab, program)) return -1;
    _svm_pg[program].mvol = value;
    return _svm_pg[program].mvol;
}

s16 _SsVmGetProgVol(s16 vab, s16 program) {
    if (_SsVmVSetUp(vab, program)) return -1;
    return _svm_pg[program].mvol;
}

s16 _SsVmSetProgPan(s16 vab, s16 program, s16 value) {
    if (_SsVmVSetUp(vab, program)) return -1;
    _svm_pg[program].mpan = value;
    return _svm_pg[program].mpan;
}

s16 _SsVmGetProgPan(s16 vab, s16 program) {
    if (_SsVmVSetUp(vab, program)) return -1;
    return _svm_pg[program].mpan;
}
