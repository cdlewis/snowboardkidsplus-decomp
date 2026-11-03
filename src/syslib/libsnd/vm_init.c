#include "common.h"
#include "vm_registers.h"
#include "vm_voice.h"
#include "vm_g.h"

#ifdef NON_MATCHING
#include "psx/libspu.h"
typedef struct { u32 voice, mask; s16 left, right; u8 pad0[8]; s16 pitch; u8 pad1[6]; u32 address; u8 pad2[26]; u16 adsr1, adsr2; u8 pad3[2]; } VoiceAttr;
typedef struct { u32 a, b; s16 c, d; } InitGlobal;
extern InitGlobal _svm_rattr;
extern u16 _svm_vab_count;
extern u8 _svm_sreg_dirty[], _svm_vab_used[], _svm_auto_kof_mode;
extern s8 _SsVmMaxVoice;
extern s16 _svm_damper, _svm_stereo_mono, kMaxPrograms;
extern s16 _svm_okon1, _svm_okon2, _svm_okof1, svmNoiseVoiceMaskLow, svmNoiseVoiceMaskHigh, _svm_orev1, _svm_orev2;
extern char svmSpuAllocationRecords[];
extern void _spu_setInTransfer(int), SpuSetVoiceAttr(VoiceAttr *), _SsVmKeyOffNow(int), _SsVmFlush(void);
/* Best observed instruction-sequence similarity: 44.34% (94/212 words;
 * GCC 2.7.2, assembler reordering; target includes padding). */
void _SsVmInit(s8 count) {
    VoiceAttr attr;
    u16 i;
    _spu_setInTransfer(0);
    _svm_damper = 0;
    SpuInitMalloc(32, svmSpuAllocationRecords);
    for (i = 0; i < NUM_VOICES; i++) {
        _svm_sreg_buf[i].volume.left = 0;
        _svm_sreg_buf[i].volume.right = 0;
        _svm_sreg_buf[i].pitch = 0;
        _svm_sreg_buf[i].addr = 0;
        _svm_sreg_buf[i].adsr[0] = 0;
        _svm_sreg_buf[i].adsr[1] = 0;
        _svm_sreg_buf[i].volumex = 0;
        _svm_sreg_buf[i].loop_addr = 0;
    }
    for (i = 0; i < 24; i++) _svm_sreg_dirty[i] = 0;
    _svm_vab_count = 0;
    for (i = 0; i < 16; i++) _svm_vab_used[i] = 0;
    _SsVmMaxVoice = (u32)count < 24 ? count : 24;
    attr.mask = 0x60093;
    attr.pitch = 0x1000;
    attr.address = 0x1000;
    attr.adsr1 = 0x80ff;
    attr.left = 0;
    attr.right = 0;
    attr.adsr2 = 0x4000;
    for (i = 0; i < _SsVmMaxVoice; i++) {
        SvmVoice *v = &_svm_voice[i];
        v->field02 = 24; v->field0 = 255; v->field1d = 0; v->field4 = 0; v->field06 = 0;
        v->seq_id = -1; v->fake_program = 0; v->prog = 0; v->tone = 255; v->field08 = 0;
        v->field0c = 0; v->field0a = 64; v->auto_vol = 0; v->field20 = 0; v->field22 = 0;
        v->start_vol = 0; v->field2a = 0; v->field2c = 0; v->field2e = 0; v->start_pan = 0; v->end_pan = 0; v->end_vol = 0;
        attr.voice = 1 << i;
        SpuSetVoiceAttr(&attr);
        _svm_cur.voice = i;
        _SsVmKeyOffNow(1);
    }
    _svm_rattr.a = 0; _svm_rattr.c = 0x3fff; _svm_rattr.d = 0x3fff; _svm_rattr.b = 0;
    _svm_okon1 = _svm_okon2 = _svm_okof1 = svmNoiseVoiceMaskLow = svmNoiseVoiceMaskHigh = _svm_orev1 = _svm_orev2 = 0;
    _svm_auto_kof_mode = 0; _svm_stereo_mono = 0; kMaxPrograms = 128;
    _SsVmFlush();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_init", _SsVmInit);
#endif
