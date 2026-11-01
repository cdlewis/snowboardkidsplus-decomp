#include "common.h"

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 94.12% (128/136 words; target includes padding). */
#include "libsnd_private.h"
extern s16 D_8013C790, D_8013C792;
extern u32 D_8013BF08;
void SsSetTableSize(struct SeqStruct *table, s16 sequences, s16 tracks) {
    int i, j;
    D_8013C790 = sequences;
    D_8013C792 = tracks;
    for (i = 0; i < sequences; i++) D_8013BF10[i] = table + i * tracks;
    for (i = sequences; i < 32; i++) D_8013BF08 |= 1 << i;
    for (i = 0; i < D_8013C790; i++) {
        for (j = 0; j < D_8013C792; j++) {
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].flags = 0;
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unk22 = 0xff;
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unk23 = 0;
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].vol_l = 0; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].vol_r = 0;
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].v_time_l = 0; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].v_time_r = 0;
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unk4C = 0; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unkAC = 0; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unkA8 = 0; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unkA4 = 0; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unk4E = 0;
            ((struct SeqStruct *volatile *)D_8013BF10)[i][j].voll = 127; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].volr = 127; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unk5C = 127; ((struct SeqStruct *volatile *)D_8013BF10)[i][j].unk5E = 127;
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/sstable", SsSetTableSize);
#endif
