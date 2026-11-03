#include "common.h"
#include "libsnd_private.h"

#ifdef NON_MATCHING
extern s16 _SsVmSetSeqVol(s16, u16, u16, s16);
/* Best observed instruction-sequence similarity: 83.33% (30/36 words;
 * GCC 2.8.0, -mno-split-addresses). */
void _SsSndSetVol(s16 id, s16 seq, u16 left, u16 right) {
    struct SeqStruct *s = &_ss_score[id][seq];
    if (s->flags != 1) {
        s->voll = left;
        s->volr = right;
    } else {
        _SsVmSetSeqVol(id | (seq << 8), left, right, 1);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssvol", _SsSndSetVol);
#endif

#ifdef NON_MATCHING
extern s16 _SsVmSetSeqVol(s16, u16, u16, s16);

/* Best observed instruction-sequence similarity: 65.38% (17/26 words). */
void SsSeqSetVol(s16 id, int left, int right) {
    struct SeqStruct *score = &_ss_score[id][0];

    if (score->flags != 1) {
        score->voll = left;
        score->volr = right;
    } else {
        _SsVmSetSeqVol(id, left, right, 1);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssvol", SsSeqSetVol);
#endif

#ifdef NON_MATCHING
/* Best observed instruction-sequence similarity: 83.33% (30/36 words;
 * GCC 2.8.0, -mno-split-addresses). */
void SsSepSetVol(s16 id, s16 seq, u16 left, u16 right) {
    struct SeqStruct *s = &_ss_score[id][seq];
    if (s->flags != 1) {
        s->voll = left;
        s->volr = right;
    } else {
        _SsVmSetSeqVol(id | (seq << 8), left, right, 1);
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/syslib/libsnd/ssvol", SsSepSetVol);
#endif

void SsSeqGetVol(s16 id, s16 sequence, s16 *left, s16 *right) {
    _SsVmGetSeqVol(id | (sequence << 8), left, right);
}
