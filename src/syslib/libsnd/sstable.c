#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 94.12% (128/136 words; target includes padding). */
#include "libsnd_private.h"
extern s16 _snd_seq_s_max, _snd_seq_t_max;
extern u32 _snd_openflag;
void SsSetTableSize(struct SeqStruct *table, s16 sequences, s16 tracks) {
    int i, j;
    _snd_seq_s_max = sequences;
    _snd_seq_t_max = tracks;
    for (i = 0; i < sequences; i++) _ss_score[i] = table + i * tracks;
    for (i = sequences; i < 32; i++) _snd_openflag |= 1 << i;
    for (i = 0; i < _snd_seq_s_max; i++) {
        for (j = 0; j < _snd_seq_t_max; j++) {
            ((struct SeqStruct *volatile *)_ss_score)[i][j].flags = 0;
            ((struct SeqStruct *volatile *)_ss_score)[i][j].unk22 = 0xff;
            ((struct SeqStruct *volatile *)_ss_score)[i][j].unk23 = 0;
            ((struct SeqStruct *volatile *)_ss_score)[i][j].vol_l = 0; ((struct SeqStruct *volatile *)_ss_score)[i][j].vol_r = 0;
            ((struct SeqStruct *volatile *)_ss_score)[i][j].v_time_l = 0; ((struct SeqStruct *volatile *)_ss_score)[i][j].v_time_r = 0;
            ((struct SeqStruct *volatile *)_ss_score)[i][j].unk4C = 0; ((struct SeqStruct *volatile *)_ss_score)[i][j].unkAC = 0; ((struct SeqStruct *volatile *)_ss_score)[i][j].unkA8 = 0; ((struct SeqStruct *volatile *)_ss_score)[i][j].unkA4 = 0; ((struct SeqStruct *volatile *)_ss_score)[i][j].unk4E = 0;
            ((struct SeqStruct *volatile *)_ss_score)[i][j].voll = 127; ((struct SeqStruct *volatile *)_ss_score)[i][j].volr = 127; ((struct SeqStruct *volatile *)_ss_score)[i][j].unk5C = 127; ((struct SeqStruct *volatile *)_ss_score)[i][j].unk5E = 127;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/sstable", SsSetTableSize);
#endif
