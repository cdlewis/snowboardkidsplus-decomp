#include "libsnd_private.h"

void vmNoiseOff(u8 voice) {
    _svm_voice[voice].field1d = 0;
    _svm_voice[voice].field0 = 0;
    _svm_voice[voice].field4 = 0;
}
