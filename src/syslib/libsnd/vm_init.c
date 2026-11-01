#include "common.h"

#ifdef NON_MATCHING
#include "psx/libspu.h"
typedef struct { u32 voice, mask; s16 left, right; u8 pad0[8]; s16 pitch; u8 pad1[6]; u32 address; u8 pad2[26]; u16 adsr1, adsr2; u8 pad3[2]; } VoiceAttr;
typedef struct {
    s16 f00, f02, f04, f06, f08;
    u8 f0a, pad0b;
    s16 f0c, pad0e, f10, f12, f14, f16, pad18, pad1a;
    u8 pad1c, f1d;
    s16 f1e, f20, f22, f24, f26, f28, f2a, f2c, f2e, f30, f32, f34;
} InitVoice;
typedef struct { u32 a, b; s16 c, d; } InitGlobal;
extern InitVoice D_8013C7C8[];
extern InitGlobal D_8013CEC8;
extern u16 D_8013CCF8[], D_8013D038;
extern u8 D_8013CCD8[], D_8013CFE0[], D_8013CFF0;
extern s8 D_8013CFBC;
extern s16 D_8013CFDA, _svm_damper, _svm_stereo_mono, D_8013CFAA;
extern s16 D_8013C7B8, D_8013C7BA, D_8013CEC0, D_8013C7BC, D_8013C7BE, D_8013C7C0, D_8013C7C2;
extern char D_8013D088[];
extern void _spu_setInTransfer(int), SpuSetVoiceAttr(VoiceAttr *), _SsVmKeyOffNow(int), _SsVmFlush(void);
/* Best observed instruction-sequence similarity: 44.34% (94/212 words;
 * GCC 2.7.2, assembler reordering; target includes padding). */
void _SsVmInit(s8 count) {
    VoiceAttr attr;
    u16 i;
    _spu_setInTransfer(0);
    _svm_damper = 0;
    SpuInitMalloc(32, D_8013D088);
    for (i = 0; i < 192; i++) D_8013CCF8[i] = 0;
    for (i = 0; i < 24; i++) D_8013CCD8[i] = 0;
    D_8013D038 = 0;
    for (i = 0; i < 16; i++) D_8013CFE0[i] = 0;
    D_8013CFBC = (u32)count < 24 ? count : 24;
    attr.mask = 0x60093;
    attr.pitch = 0x1000;
    attr.address = 0x1000;
    attr.adsr1 = 0x80ff;
    attr.left = 0;
    attr.right = 0;
    attr.adsr2 = 0x4000;
    for (i = 0; i < D_8013CFBC; i++) {
        InitVoice *v = &D_8013C7C8[i];
        v->f02 = 24; v->f00 = 255; v->f1d = 0; v->f04 = 0; v->f06 = 0;
        v->f10 = -1; v->f12 = 0; v->f14 = 0; v->f16 = 255; v->f08 = 0;
        v->f0c = 0; v->f0a = 64; v->f1e = 0; v->f20 = 0; v->f22 = 0;
        v->f24 = 0; v->f2a = 0; v->f2c = 0; v->f2e = 0; v->f30 = 0; v->f32 = 0; v->f26 = 0;
        attr.voice = 1 << i;
        SpuSetVoiceAttr(&attr);
        D_8013CFDA = i;
        _SsVmKeyOffNow(1);
    }
    D_8013CEC8.a = 0; D_8013CEC8.c = 0x3fff; D_8013CEC8.d = 0x3fff; D_8013CEC8.b = 0;
    D_8013C7B8 = D_8013C7BA = D_8013CEC0 = D_8013C7BC = D_8013C7BE = D_8013C7C0 = D_8013C7C2 = 0;
    D_8013CFF0 = 0; _svm_stereo_mono = 0; D_8013CFAA = 128;
    _SsVmFlush();
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_init", _SsVmInit);
#endif
