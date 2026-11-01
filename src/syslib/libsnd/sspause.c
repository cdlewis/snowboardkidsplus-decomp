#include "common.h"
#include "libsnd_private.h"

extern void _SsSndSetPauseMode(s16 id, s16 sequence);

void _SsSndSetPauseMode(s16 id, s16 sequence) {
    struct SeqStruct *score = &D_8013BF10[id][sequence];

    _SsVmGetSeqVol(id | (sequence << 8), &score->unk5C, &score->unk5E);
    D_8013BF10[id][sequence].flags &= ~1;
    D_8013BF10[id][sequence].flags &= ~8;
    D_8013BF10[id][sequence].flags |= 2;
}

void SsSeqPause(s16 id) { _SsSndSetPauseMode(id, 0); }

void SsSepPause(s16 id, s16 sequence) { _SsSndSetPauseMode(id, sequence); }
