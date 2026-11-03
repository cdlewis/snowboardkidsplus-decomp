#include "common.h"
#include "vm_registers.h"
#include "vm_voice.h"
#include "vm_g.h"

#ifdef NON_MATCHING
#include "libsnd_private.h"
extern u8 _svm_sreg_dirty[];

extern VagAtr *_svm_tn;
extern u16 note2pitch2(s16, u16);
/* Best observed instruction-sequence similarity: 40.16% (51/127 words;
 * GCC 2.7.2, assembler reordering). */
s16 _SsVmPBVoice(s16 voice, s16 id, s16 vab, s16 program, u16 bend) {
    SvmVoice *v = &_svm_voice[voice];
    int amount, fine, index;
    u16 note;
    s16 d = bend - 64;
    if (v->seq_id != id || v->vabId != vab || v->prog != program) return 0;
    index = (u16)((u16)v->tone + _svm_cur.fake_program * 16);
    note = (u16)v->note;
    if (d > 0) {
        amount = d * _svm_tn[index].pbmax;
        note += amount / 63;
        fine = (amount % 63) * 2;
    } else if (d < 0) {
        amount = d * _svm_tn[index].pbmin;
        note += amount / 64 - 1;
        fine = (amount % 64) * 2 + 127;
    } else fine = 0;
    _svm_cur.voice = voice;
    _svm_cur.tone = (u8)v->tone;
    _svm_sreg_buf[voice].pitch = note2pitch2(note, fine);
    _svm_sreg_dirty[voice] |= 4;
    return 1;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_pb", _SsVmPBVoice);
#endif

#ifdef NON_MATCHING
extern s8 _SsVmMaxVoice;

extern int _SsVmVSetUp(s16, s16);
extern s16 _SsVmPBVoice(s16, s16, s16, s16, u16);
/* Best observed instruction-sequence similarity: 48.28% (28/58 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
int _SsVmPitchBend(s16 id, s16 vab, s16 program, u16 bend) {
    s16 i;
    int sum = 0;
    _SsVmVSetUp(vab, program);
    _svm_cur.seq_sep_no = id;
    for (i = 0; i < _SsVmMaxVoice; i++)
        sum += _SsVmPBVoice(i, id, vab, program, bend);
    return sum;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_pb", _SsVmPitchBend);
#endif
