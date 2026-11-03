#include "../libspu/libspu_private.h"
#include "vm_registers.h"

extern u8 _svm_sreg_dirty[];

s16 SsUtGetDetVVol(s16 voice, s16 *left, s16 *right) {
    if (voice >= 0 && voice < 24) {
        SpuGetVoiceVolume(voice, left, right);
        return 0;
    }
    return -1;
}

s16 SsUtSetDetVVol(s16 voice, s16 left, s16 right) {
    if (voice >= 0 && voice < 24) {
        _svm_sreg_buf[voice].volume.right = right;
        _svm_sreg_buf[voice].volume.left = left;
        _svm_sreg_dirty[voice] |= 3;
        return 0;
    }
    return -1;
}

s16 SsUtGetVVol(s16 voice, s16 *left, s16 *right) {
    s16 l, r;
    if (voice >= 0 && voice < 24) {
        SpuGetVoiceVolume(voice, &l, &r);
        *left = l / 129;
        *right = r / 129;
        return 0;
    }
    return -1;
}

s16 SsUtSetVVol(s16 voice, s16 left, s16 right) {
    if (voice >= 0 && voice < 24) {
        s32 scaled_right = right * 129;
        _svm_sreg_buf[voice].volume.right = scaled_right;
        _svm_sreg_buf[voice].volume.left = left * 129;
        _svm_sreg_dirty[voice] |= 3;
        return 0;
    }
    return -1;
}
