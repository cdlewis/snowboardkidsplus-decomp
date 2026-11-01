#include "common.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern u8 D_8013C7D6[], D_8013CFCC[], D_8013CCD8[], D_8013CCFC[];
extern s8 D_8013CFC7;
extern VagAtr *D_8013CFB8;
extern u16 note2pitch2(s16, u16);
/* Best observed instruction-sequence similarity: 40.16% (51/127 words;
 * GCC 2.7.2, assembler reordering). */
s16 _SsVmPBVoice(s16 voice, s16 id, s16 vab, s16 program, u16 bend) {
    u8 *v = D_8013C7D6 + voice * 54;
    int amount, fine, index;
    u16 note;
    s16 d = bend - 64;
    if (*(s16 *)(v + 2) != id || *(s16 *)(v + 10) != vab || *(s16 *)(v + 6) != program) return 0;
    index = (u16)(*(u16 *)(v + 8) + D_8013CFC7 * 16);
    note = *(u16 *)v;
    if (d > 0) {
        amount = d * D_8013CFB8[index].pbmax;
        note += amount / 63;
        fine = (amount % 63) * 2;
    } else if (d < 0) {
        amount = d * D_8013CFB8[index].pbmin;
        note += amount / 64 - 1;
        fine = (amount % 64) * 2 + 127;
    } else fine = 0;
    *(s16 *)(D_8013CFCC + 14) = voice;
    D_8013CFCC[0] = v[8];
    *(u16 *)(D_8013CCFC + voice * 16) = note2pitch2(note, fine);
    D_8013CCD8[voice] |= 4;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_pb", _SsVmPBVoice);
#endif

#ifdef NON_MATCHING
extern s8 D_8013CFBC;
extern s16 D_8013CFD6;
extern int _SsVmVSetUp(s16, s16);
extern s16 _SsVmPBVoice(s16, s16, s16, s16, u16);
/* Best observed instruction-sequence similarity: 48.28% (28/58 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
int _SsVmPitchBend(s16 id, s16 vab, s16 program, u16 bend) {
    s16 i;
    int sum = 0;
    _SsVmVSetUp(vab, program);
    D_8013CFD6 = id;
    for (i = 0; i < D_8013CFBC; i++)
        sum += _SsVmPBVoice(i, id, vab, program, bend);
    return sum;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_pb", _SsVmPitchBend);
#endif
