#include "common.h"
#include "reverb_private.h"

void SsUtSetReverbDepth(s16 left, s16 right) {
    D_8013CEC8.mask = 6;
    D_8013CEC8.depth.left = (left * 0x7FFF) / 127;
    D_8013CEC8.depth.right = (right * 0x7FFF) / 127;
    SpuSetReverbModeParam(&D_8013CEC8);
}
