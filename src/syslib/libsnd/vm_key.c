#include "common.h"
#include "vm_g.h"

extern s32 _SsVmKeyOff(s16 id, s16 vab, s16 program, u16 note);

INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_key", _SsVmKeyOn);

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 62.22% (56/90 words; GCC 2.8.0, -mno-split-addresses). */
#include "libsnd_private.h"
extern s8 _SsVmMaxVoice;

extern u32 svmReservedVoiceMask;
extern void vmNoiseOff(int), _SsVmKeyOffNow(int);
s32 _SsVmKeyOff(s16 id, s16 vab, s16 program, u16 note) {
    u8 i;
    int count = 0;
    for (i = 0; i < _SsVmMaxVoice; i++) {
        u8 *v = (u8 *)&_svm_voice[i];
        if (!(svmReservedVoiceMask & (1 << i)) && *(s16 *)(v + 14) == note &&
            *(s16 *)(v + 20) == program && *(s16 *)(v + 16) == id && *(s16 *)(v + 24) == vab) {
            if (*(s16 *)v == 255) vmNoiseOff(i);
            else { _svm_cur.voice = i; _SsVmKeyOffNow(0); }
            count++;
        }
    }
    return count;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_key", _SsVmKeyOff);
#endif

#ifdef NON_MATCHING
extern s32 _SsVmKeyOn(s16, s16, s16, u16, u16, u16);
/* Best observed instruction-sequence similarity: 73.44% (47/64 words). */
s32 _SsVmSeKeyOn(s16 vab, s16 program, u16 note, int unused, u16 left, u16 right) {
    u16 volume, pan;
    if (left == right) {
        volume = left;
        pan = 64;
    } else if (left > right) {
        volume = left;
        pan = (right << 6) / left;
    } else {
        volume = right;
        pan = 127 - (left << 6) / right;
    }
    return _SsVmKeyOn(33, vab, program, note, volume, pan);
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_key", _SsVmSeKeyOn);
#endif

s32 _SsVmSeKeyOff(s16 vab, s16 program, u16 note) {
    return _SsVmKeyOff(33, vab, program, note);
}

void KeyOnCheck(void) {}
