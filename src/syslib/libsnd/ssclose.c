#include "common.h"
#include "libsnd_private.h"


#ifdef NON_MATCHING
extern s16 _snd_seq_t_max;
extern u32 _snd_openflag;
extern void _SsVmSetSeqVol(s16, u16, u16, int), _SsVmSeqKeyOff(s16);
/* Best observed instruction-sequence similarity: 89.47% (85/95 words;
 * GCC 2.7.2). */
void _SsClose(s16 id) {
    int i;
    _SsVmSetSeqVol(id, 0, 0, 1);
    _SsVmSeqKeyOff(id);
    _snd_openflag &= ~(1 << id);
    for (i = 0; i < _snd_seq_t_max; i++) {
        ((struct SeqStruct *volatile *)_ss_score)[id][i].flags = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unk22 = 0xff;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unk23 = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].vol_l = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].vol_r = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].v_time_l = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].v_time_r = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unk4C = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unkAC = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unkA8 = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unkA4 = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].unk4E = 0;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].voll = 127;
        ((struct SeqStruct *volatile *)_ss_score)[id][i].volr = 127;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssclose", _SsClose);
#endif

void SsSeqClose(s16 id) { _SsClose(id); }

void SsSepClose(s16 id) { _SsClose(id); }
