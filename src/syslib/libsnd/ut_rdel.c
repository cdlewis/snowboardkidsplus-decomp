#include "common.h"
#include "reverb_private.h"

void SsUtSetReverbDelay(s16 delay) {
    SpuReverbAttr *attr = &D_8013CEC8;
    attr->mask = 8;
    attr->delay = delay;
    SpuSetReverbModeParam(attr);
}
