#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 34.17% (41/120 words; GCC 2.8.0, -mno-split-addresses). */
#include "libsnd_private.h"
extern s16 D_8013CFD8, D_8013CFDA, D_8013CFDC[2];
extern s8 D_8013CFC7, D_8013CFCC;
extern u32 D_8013CE80[];
extern u8 *D_8013CFAC;
extern VagAtr *D_8013CFB8;
extern u8 D_8013CCD8[];
extern u16 D_8013CCFE[], D_8013CD00[], D_8013CD02[];
void _SsVmDoAllocate(void) {
    int i;
    int program;
    D_8013CFDC[0] = D_8013CFDA * 8;
    D_8013CFDC[1] = D_8013CFC7 * 16 + D_8013CFCC;
    *(s16 *)((u8 *)&D_8013C7C8[D_8013CFDA] + 6) = 0x7fff;
    for (i = 0; i < 16; i++) D_8013CE80[i] &= ~(1 << D_8013CFDA);
    program = (D_8013CFD8 - 1) / 2;
    if (D_8013CFD8 & 1) D_8013CCFE[D_8013CFDC[0]] = *(u16 *)(D_8013CFAC + program * 16 + 12);
    else D_8013CCFE[D_8013CFDC[0]] = *(u16 *)(D_8013CFAC + program * 16 + 14);
    D_8013CCD8[D_8013CFDA] |= 8;
    D_8013CD00[D_8013CFDC[0]] = D_8013CFB8[D_8013CFC7 * 16 + D_8013CFCC].adsr1;
    D_8013CD02[D_8013CFDC[0]] = D_8013CFB8[D_8013CFC7 * 16 + D_8013CFCC].adsr2 + _svm_damper;
    D_8013CCD8[D_8013CFDA] |= 0x30;
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/vm_aloc2", _SsVmDoAllocate);
#endif
