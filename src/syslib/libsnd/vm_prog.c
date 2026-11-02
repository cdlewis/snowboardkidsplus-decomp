#include "libsnd_private.h"

extern ProgAtr *D_8013CFAC;
extern int _SsVmVSetUp(s16 vab, s16 program);

s16 _SsVmSetProgVol(s16 vab, s16 program, s16 value) {
    if (_SsVmVSetUp(vab, program)) return -1;
    D_8013CFAC[program].mvol = value;
    return D_8013CFAC[program].mvol;
}

s16 _SsVmGetProgVol(s16 vab, s16 program) {
    if (_SsVmVSetUp(vab, program)) return -1;
    return D_8013CFAC[program].mvol;
}

s16 _SsVmSetProgPan(s16 vab, s16 program, s16 value) {
    if (_SsVmVSetUp(vab, program)) return -1;
    D_8013CFAC[program].mpan = value;
    return D_8013CFAC[program].mpan;
}

s16 _SsVmGetProgPan(s16 vab, s16 program) {
    if (_SsVmVSetUp(vab, program)) return -1;
    return D_8013CFAC[program].mpan;
}
