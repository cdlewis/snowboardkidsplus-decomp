/* Adapted from PsyZ decomp; see THIRD_PARTY.md. */
#include "../libspu/libspu_private.h"

extern SPU_VOICE_REG D_8013CCF8[];
extern u8 D_8013CCD8[];

s16 SsUtGetDetVVol(s16 voice, s16 *left, s16 *right) {
    if (voice >= 0 && voice < 24) {
        SpuGetVoiceVolume(voice, left, right);
        return 0;
    }
    return -1;
}

s16 SsUtSetDetVVol(s16 voice, s16 left, s16 right) {
    if (voice >= 0 && voice < 24) {
        D_8013CCF8[voice].volume.right = right;
        D_8013CCF8[voice].volume.left = left;
        D_8013CCD8[voice] |= 3;
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
        D_8013CCF8[voice].volume.right = scaled_right;
        D_8013CCF8[voice].volume.left = left * 129;
        D_8013CCD8[voice] |= 3;
        return 0;
    }
    return -1;
}
