#include "common.h"
#include "libsnd_private.h"

/* SDK wrappers adapted from PsyZ decomp; see THIRD_PARTY.md. */

#ifdef NON_MATCHING
extern s16 D_8013C792;
extern u32 D_8013BF08;
extern void _SsVmSetSeqVol(s16, u16, u16, int), _SsVmSeqKeyOff(s16);
/* Best observed instruction-sequence similarity: 89.47% (85/95 words;
 * GCC 2.7.2). */
void _SsClose(s16 id) {
    int i;
    _SsVmSetSeqVol(id, 0, 0, 1);
    _SsVmSeqKeyOff(id);
    D_8013BF08 &= ~(1 << id);
    for (i = 0; i < D_8013C792; i++) {
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].flags = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unk22 = 0xff;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unk23 = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].vol_l = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].vol_r = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].v_time_l = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].v_time_r = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unk4C = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unkAC = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unkA8 = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unkA4 = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].unk4E = 0;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].voll = 127;
        ((struct SeqStruct *volatile *)D_8013BF10)[id][i].volr = 127;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssclose", _SsClose);
#endif

void SsSeqClose(s16 id) { _SsClose(id); }

void SsSepClose(s16 id) { _SsClose(id); }
