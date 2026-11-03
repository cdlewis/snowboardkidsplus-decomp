#include "common.h"
#include "reverb_private.h"

void SsUtSetReverbDepth(s16 left, s16 right) {
    _svm_rattr.mask = 6;
    _svm_rattr.depth.left = (left * 0x7FFF) / 127;
    _svm_rattr.depth.right = (right * 0x7FFF) / 127;
    SpuSetReverbModeParam(&_svm_rattr);
}
