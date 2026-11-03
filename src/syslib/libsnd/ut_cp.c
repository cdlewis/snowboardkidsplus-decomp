#include "common.h"
#include "vm_registers.h"
#include "vm_voice.h"
#include "vm_g.h"

#ifdef NON_MATCHING
extern u8 _svm_sreg_dirty[];
extern int _SsVmVSetUp(s16, s16);
extern u16 note2pitch2(u16, u16);
/* Best observed instruction-sequence similarity: 52.50% (42/80 words;
 * GCC 2.8.0, -mno-split-addresses; target includes padding). */
int SsUtChangePitch(s16 voice, s16 vab, s16 program, s16 tone, u16 note, u16 fine) {
    SvmVoice *v = &_svm_voice[voice];
    if ((u16)voice >= 24 || v->vabId != vab ||
        v->prog != program || v->note != tone)
        return -1;
    _SsVmVSetUp(vab, program);
    _svm_cur.seq_sep_no = 33;
    _svm_cur.voice = voice;
    _svm_cur.tone = (u8)v->tone;
    _svm_sreg_buf[voice].pitch = note2pitch2(note, fine);
    _svm_sreg_dirty[voice] |= 4;
    return 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ut_cp", SsUtChangePitch);
#endif
