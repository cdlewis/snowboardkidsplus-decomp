#include "common.h"
#include "libsnd_private.h"

extern void _SsSndSetPauseMode(s16 id, s16 sequence);

void _SsSndSetPauseMode(s16 id, s16 sequence) {
    struct SeqStruct *score = &_ss_score[id][sequence];

    _SsVmGetSeqVol(id | (sequence << 8), &score->unk5C, &score->unk5E);
    _ss_score[id][sequence].flags &= ~1;
    _ss_score[id][sequence].flags &= ~8;
    _ss_score[id][sequence].flags |= 2;
}

void SsSeqPause(s16 id) { _SsSndSetPauseMode(id, 0); }

void SsSepPause(s16 id, s16 sequence) { _SsSndSetPauseMode(id, sequence); }
