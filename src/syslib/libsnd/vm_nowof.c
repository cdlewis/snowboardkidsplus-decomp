#include "libsnd_private.h"

extern u16 _svm_okof1, _svm_okof2, _svm_okon1, _svm_okon2;
void _SsVmKeyOffNow(int mode) {
    int upper, lower;
    u16 voice = _svm_cur.voice;
    if (voice < 16) {
        lower = 1 << voice;
        upper = 0;
    } else {
        lower = 0;
        upper = 1 << (voice - 16);
    }
    _svm_voice[voice].field1d = 0;
    _svm_voice[voice].field4 = 0;
    _svm_voice[voice].field0 = 0;
    _svm_okof1 |= lower;
    _svm_okof2 |= upper;
    _svm_okon1 &= ~_svm_okof1;
    _svm_okon2 &= ~_svm_okof2;
}
