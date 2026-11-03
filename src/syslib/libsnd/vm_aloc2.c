#include "common.h"
#include "vm_registers.h"
#include "vm_g.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 34.17% (41/120 words; GCC 2.8.0, -mno-split-addresses). */
#include "libsnd_private.h"

extern u32 svmVabVoiceMasks[];
extern u8 *_svm_pg;
extern VagAtr *_svm_tn;
extern u8 _svm_sreg_dirty[];
void _SsVmDoAllocate(void) {
    int i;
    int program;
    _svm_cur.voiceOffset = _svm_cur.voice * 8;
    _svm_cur.field1e = _svm_cur.fake_program * 16 + _svm_cur.tone;
    _svm_voice[_svm_cur.voice].field06 = 0x7fff;
    for (i = 0; i < 16; i++) svmVabVoiceMasks[i] &= ~(1 << _svm_cur.voice);
    program = (_svm_cur.tone_vag_idx - 1) / 2;
    if (_svm_cur.tone_vag_idx & 1) _svm_sreg_buf[_svm_cur.voice].addr = *(u16 *)(_svm_pg + program * 16 + 12);
    else _svm_sreg_buf[_svm_cur.voice].addr = *(u16 *)(_svm_pg + program * 16 + 14);
    _svm_sreg_dirty[_svm_cur.voice] |= 8;
    _svm_sreg_buf[_svm_cur.voice].adsr[0] = _svm_tn[_svm_cur.fake_program * 16 + _svm_cur.tone].adsr1;
    _svm_sreg_buf[_svm_cur.voice].adsr[1] = _svm_tn[_svm_cur.fake_program * 16 + _svm_cur.tone].adsr2 + _svm_damper;
    _svm_sreg_dirty[_svm_cur.voice] |= 0x30;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_aloc2", _SsVmDoAllocate);
#endif
