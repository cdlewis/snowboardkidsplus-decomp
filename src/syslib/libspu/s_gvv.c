#include "libspu_private.h"

static inline void assign(u16 val, s16* out) {
    u32 offset = 0x8000;
    u32 uval = val;

    if (uval >= 0x4000) {
        *out = uval - offset;
    } else {
        *out = val;
    }
}

void SpuGetVoiceVolume(s32 vNum, s16* volL, s16* volR) {
    u16 left = SPURV(vNum, volume.left);
    u16 right = SPURV(vNum, volume.right);

    assign(left, volL);
    assign(right, volR);
}
