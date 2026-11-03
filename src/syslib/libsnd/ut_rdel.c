#include "common.h"
#include "reverb_private.h"

void SsUtSetReverbDelay(s16 delay) {
    SpuReverbAttr *attr = &_svm_rattr;
    attr->mask = 8;
    attr->delay = delay;
    SpuSetReverbModeParam(attr);
}
