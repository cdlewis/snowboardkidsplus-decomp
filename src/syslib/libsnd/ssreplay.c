#include "common.h"
#include "libsnd_private.h"

extern void _SsSndSetReplayMode(s16 id, s16 sequence);

void _SsSndSetReplayMode(s16 id, s16 sequence) {
    struct SeqStruct *score = &D_8013BF10[id][sequence];

    if ((score->flags & 0x204) == 0) {
        if ((score->flags & 0x100) == 0) {
            score->flags &= ~2;
            D_8013BF10[id][sequence].flags |= 8;
            D_8013BF10[id][sequence].flags |= 1;
        }
    }
}

void SsSeqReplay(s16 id) { _SsSndSetReplayMode(id, 0); }

void SsSepReplay(s16 id, s16 sequence) { _SsSndSetReplayMode(id, sequence); }
