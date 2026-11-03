#include "common.h"
#include "vm_g.h"

#ifdef NON_MATCHING

extern u16 svmPitchTable[];
/* Best observed instruction-sequence similarity: 84.91% (45/53 words;
 * GCC 2.8.0, -mno-split-addresses). */
u16 note2pitch(void) {
    s16 note = (s8)_svm_cur.note - ((s8)_svm_cur.tone_center - 60);
    u16 fine = (s8)_svm_cur.tone_shift / 8;
    s16 octave = note / 12;
    s16 tone = note % 12;
    u16 pitch;
    if (fine >= 16)
        fine = 15;
    pitch = svmPitchTable[(tone << 4) + fine];
    octave -= 5;
    if (octave > 0)
        pitch <<= octave;
    else if (octave < 0)
        pitch >>= -octave;
    return pitch;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_n2p", note2pitch);
#endif

#ifdef NON_MATCHING
#include "libsnd_private.h"

extern VagAtr *_svm_tn;
extern u16 svmPitchTable[];
/* Best observed instruction-sequence similarity: 80.30% (53/66 words). */
u16 note2pitch2(s16 note, u16 fine) {
    s16 index = _svm_cur.tone + _svm_cur.fake_program * 16;
    VagAtr *attr = &_svm_tn[index];
    s32 f = (fine + attr->shift) / 8;
    s32 carry = 0;
    s16 diff, octave, tone;
    u16 pitch;

    if (f >= 16) {
        carry = 1;
        f -= 16;
    }
    diff = note + 60 - attr->center + carry;
    octave = diff / 12;
    tone = diff % 12;
    pitch = svmPitchTable[(tone << 4) + (s16)f];
    octave -= 5;
    if (octave > 0)
        pitch <<= octave;
    else if (octave < 0)
        pitch = (u16)pitch >> -octave;
    return pitch;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_n2p", note2pitch2);
#endif

#ifdef NON_MATCHING
extern u16 svmPitchTable[];
/* Best observed instruction-sequence similarity: 68.42% (39/57 words,
 * including target padding). */
u16 SsPitchFromNote(s16 note, s16 fine, u8 center, u8 shift) {
    s16 pitch, octave, tone, f;
    s32 n, carry = 0;
    f = ((s16)fine + (u8)shift) / 8;
    if (f >= 16) {
        carry = 1;
        f -= 16;
    }
    n = (s16)(note - ((u8)center - 60) + carry);
    octave = n / 12;
    tone = n % 12;
    pitch = svmPitchTable[(tone << 4) + f];
    octave -= 5;
    if (octave > 0)
        pitch <<= octave;
    else if (octave < 0)
        pitch = (u16)pitch >> -octave;
    return pitch;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_n2p", SsPitchFromNote);
#endif
